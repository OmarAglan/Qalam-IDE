#include "BuildManager.h"
#include "TConsole.h"
#include "Constants.h"

#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QStandardPaths>
#include <QMetaObject>
#include <QPointer>

BuildManager::BuildManager(QObject *parent)
    : QObject(parent)
{
}

BuildManager::~BuildManager()
{
    if (m_checkProcess and m_checkProcess->state() != QProcess::NotRunning) {
        m_checkProcess->kill();
        m_checkProcess->waitForFinished(500);
    }
    cleanupBuild();
}

bool BuildManager::isRunning() const
{
    return m_buildThread and m_buildThread->isRunning();
}

QString BuildManager::resolveCompilerPath() const
{
    QSettings settings(Constants::OrgName, Constants::AppName);
    const QString configuredProgram = settings.value(Constants::SettingsKeyCompilerPath).toString().trimmed();

    if (!configuredProgram.isEmpty()) {
        return configuredProgram;
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
#if defined(Q_OS_WIN)
        QDir(appDir).filePath("baa/baa.exe"),
        QDir(appDir).filePath("baa.exe"),
        QStandardPaths::findExecutable("baa.exe"),
        QStandardPaths::findExecutable("baa")
#else
        QDir(appDir).filePath("baa/baa"),
        QDir(appDir).filePath("baa"),
        QStandardPaths::findExecutable("baa")
#endif
    };

    for (const QString &candidate : candidates) {
        if (!candidate.isEmpty() and QFileInfo(candidate).isExecutable()) {
            return candidate;
        }
    }

#if defined(Q_OS_WIN)
    return QDir(appDir).filePath("baa/baa.exe");
#else
    return QDir(appDir).filePath("baa/baa");
#endif
}

QString BuildManager::resolveTakweenPath() const
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
#if defined(Q_OS_WIN)
        QDir(appDir).filePath("takween/تكوين.exe"),
        QDir(appDir).filePath("takween/takween.exe"),
        QDir(appDir).filePath("تكوين.exe"),
        QDir(appDir).filePath("takween.exe"),
        QStandardPaths::findExecutable("تكوين.exe"),
        QStandardPaths::findExecutable("takween.exe"),
        QStandardPaths::findExecutable("takween")
#else
        QDir(appDir).filePath("takween/تكوين"),
        QDir(appDir).filePath("takween/takween"),
        QStandardPaths::findExecutable("تكوين"),
        QStandardPaths::findExecutable("takween")
#endif
    };

    for (const QString &candidate : candidates) {
        if (!candidate.isEmpty() and QFileInfo(candidate).isExecutable()) {
            return candidate;
        }
    }
    return QString();
}

QStringList BuildManager::baaCheckArguments(const QString &filePath)
{
    return {"--check", "--diagnostics=json", QFileInfo(filePath).absoluteFilePath()};
}

QStringList BuildManager::takweenCommandArguments(const QString &command)
{
    const QString normalized = command.trimmed().toLower();
    if (normalized == "build" or normalized == "run" or normalized == "test" or
        normalized == "clean") {
        return {normalized};
    }
    return {};
}

BuildManager::CompilerExitClass BuildManager::classifyCompilerExitCode(int exitCode)
{
    switch (exitCode) {
    case 0: return CompilerExitClass::Success;
    case 1: return CompilerExitClass::SourceError;
    case 2: return CompilerExitClass::InvalidInvocation;
    case 3: return CompilerExitClass::Unsupported;
    case 4: return CompilerExitClass::ToolchainError;
    case 5: return CompilerExitClass::InternalError;
    default:
        return exitCode < 0 ? CompilerExitClass::ProcessFailure : CompilerExitClass::Unknown;
    }
}

QString BuildManager::compilerExitCodeId(int exitCode)
{
    if (exitCode >= 0 and exitCode <= 5) {
        return QString("CLI_EXIT_%1").arg(exitCode);
    }
    if (exitCode < 0) return "PROCESS_FAILURE";
    return QString("UNKNOWN_EXIT_%1").arg(exitCode);
}

QString BuildManager::compilerExitSummary(int exitCode, const QString &operation)
{
    const QString normalized = operation.trimmed().toLower();
    const bool mayBeProgramExit = normalized == "run" or normalized == "test";
    if (mayBeProgramExit) {
        return QString("انتهى أمر %1 بكود خروج %2. بعد نجاح البناء قد يكون هذا كود البرنامج الناتج.")
            .arg(normalized, QString::number(exitCode));
    }

    switch (classifyCompilerExitCode(exitCode)) {
    case CompilerExitClass::Success:
        return QString();
    case CompilerExitClass::SourceError:
        return "رفضت الأداة المصدر أو المشروع (كود الخروج 1).";
    case CompilerExitClass::InvalidInvocation:
        return "استدعاء أداة البناء غير صالح (كود الخروج 2).";
    case CompilerExitClass::Unsupported:
        return "الهدف أو الميزة المطلوبة غير مدعومة (كود الخروج 3).";
    case CompilerExitClass::ToolchainError:
        return "فشلت أداة البناء أو backend أو إنشاء المخرجات (كود الخروج 4).";
    case CompilerExitClass::InternalError:
        return "حدث خطأ داخلي في المصرّف أو نظام البناء (كود الخروج 5).";
    case CompilerExitClass::ProcessFailure:
        return "تعذر بدء أداة البناء أو انقطعت العملية قبل اكتمالها.";
    case CompilerExitClass::Unknown:
        return QString("انتهت أداة البناء بكود خروج غير معروف: %1.").arg(exitCode);
    }
    return QString();
}

QString BuildManager::findTakweenProjectRoot(const QString &filePath)
{
    QDir directory = QFileInfo(filePath).isDir()
        ? QDir(filePath)
        : QFileInfo(filePath).absoluteDir();

    while (directory.exists()) {
        if (QFileInfo(directory.filePath("مشروع.تكوين")).isFile()) {
            return QDir::cleanPath(directory.absolutePath());
        }
        if (!directory.cdUp()) break;
    }
    return QString();
}

void BuildManager::checkBaa(const QString &filePath)
{
    const QFileInfo source(filePath);
    if (!source.isFile() or (source.suffix() != "baa" and source.suffix() != "baahd")) return;

    QString program = resolveCompilerPath();
    if (!QFileInfo(program).isExecutable()) {
        const QString pathProgram = QStandardPaths::findExecutable(program);
        if (!pathProgram.isEmpty()) program = pathProgram;
    }
    if (!QFileInfo(program).isExecutable()) return;

    if (m_checkProcess) {
        QProcess *previous = m_checkProcess.data();
        disconnect(previous, nullptr, this, nullptr);
        if (previous->state() != QProcess::NotRunning) {
            previous->kill();
            previous->waitForFinished(250);
        }
        previous->deleteLater();
        m_checkProcess = nullptr;
    }

    m_checkStdout.clear();
    m_checkProcess = new QProcess(this);
    m_checkProcess->setProgram(program);
    m_checkProcess->setArguments(baaCheckArguments(filePath));
    m_checkProcess->setWorkingDirectory(source.absolutePath());
    m_checkProcess->setProcessChannelMode(QProcess::SeparateChannels);

    QProcess *checkProcess = m_checkProcess.data();
    connect(checkProcess, &QProcess::readyReadStandardOutput, this, [this, checkProcess]() {
        if (m_checkProcess == checkProcess) {
            m_checkStdout += QString::fromUtf8(checkProcess->readAllStandardOutput());
        }
    });
    connect(checkProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, checkProcess](int exitCode, QProcess::ExitStatus exitStatus) {
                if (m_checkProcess != checkProcess) return;
                m_checkStdout += QString::fromUtf8(checkProcess->readAllStandardOutput());
                const QString payload = m_checkStdout;
                m_checkStdout.clear();
                checkProcess->deleteLater();
                m_checkProcess = nullptr;
                if (!payload.trimmed().isEmpty()) emit diagnosticsReady(payload);
                emit toolingFinished("check", exitStatus == QProcess::NormalExit ? exitCode : -1);
            });
    connect(checkProcess, &QProcess::errorOccurred, this,
            [this, checkProcess](QProcess::ProcessError error) {
                if (error != QProcess::FailedToStart or m_checkProcess != checkProcess) return;
                m_checkStdout.clear();
                checkProcess->deleteLater();
                m_checkProcess = nullptr;
                emit toolingFinished("check", -1);
            });
    checkProcess->start();
}

void BuildManager::cleanupBuild()
{
    QThread *thread = m_buildThread.data();
    ProcessWorker *worker = m_worker.data();

    m_worker = nullptr;
    m_buildThread = nullptr;

    if (worker) {
        if (thread and thread->isRunning() and QThread::currentThread() != thread) {
            QMetaObject::invokeMethod(worker, "stop", Qt::BlockingQueuedConnection);
        } else {
            worker->stop();
        }
    }

    if (thread) {
        thread->quit();
        if (!thread->wait(3000)) {
            thread->terminate();
            thread->wait();
        }
    }
}

void BuildManager::stop()
{
    cleanupBuild();
}

void BuildManager::runBaa(const QString &filePath, TConsole *console)
{
    if (!console) return;

    QString program = resolveCompilerPath();
    QStringList args = { filePath };
    QString workingDir = QFileInfo(filePath).absolutePath();
    bool usingTakween = false;

    const QString projectRoot = findTakweenProjectRoot(filePath);
    if (!projectRoot.isEmpty()) {
        const QString takween = resolveTakweenPath();
        if (!takween.isEmpty()) {
            program = takween;
            args = takweenCommandArguments("run");
            workingDir = projectRoot;
            usingTakween = true;
        }
    }

    startProcess(program,
                 args,
                 workingDir,
                 filePath,
                 usingTakween ? "run" : "baa",
                 usingTakween ? "🚀 تشغيل مشروع تكوين...\n" : "🚀 بدء تشغيل ملف باء...\n",
                 console);
}

bool BuildManager::runTakweenCommand(const QString &filePath,
                                     const QString &command,
                                     TConsole *console)
{
    if (!console) return false;

    const QStringList arguments = takweenCommandArguments(command);
    const QString projectRoot = findTakweenProjectRoot(filePath);
    const QString takween = resolveTakweenPath();
    if (arguments.isEmpty() or projectRoot.isEmpty() or takween.isEmpty()) return false;

    const QString normalized = arguments.first();
    QString heading = "🚀 تنفيذ أمر مشروع تكوين...\n";
    if (normalized == "build") heading = "🛠️ بناء مشروع تكوين...\n";
    else if (normalized == "run") heading = "🚀 تشغيل مشروع تكوين...\n";
    else if (normalized == "test") heading = "🧪 اختبار مشروع تكوين...\n";
    else if (normalized == "clean") heading = "🧹 تنظيف مشروع تكوين...\n";

    startProcess(takween, arguments, projectRoot, filePath, normalized, heading, console);
    return true;
}

void BuildManager::startProcess(const QString &requestedProgram,
                                const QStringList &arguments,
                                const QString &workingDirectory,
                                const QString &contextPath,
                                const QString &operation,
                                const QString &heading,
                                TConsole *console)
{
    if (!console) return;
    QString program = requestedProgram;

    if (!QFileInfo(program).isExecutable()) {
        const QString pathProgram = QStandardPaths::findExecutable(program);
        if (!pathProgram.isEmpty()) {
            program = pathProgram;
        }
    }

    if (!QFileInfo(program).isExecutable()) {
        console->clear();
        console->appendPlainTextThreadSafe("❌ خطأ: لم يتم العثور على مترجم باء!\n");
        console->appendPlainTextThreadSafe("المسار المتوقع: " + program + "\n");
        console->appendPlainTextThreadSafe("يمكنك وضع المترجم بجانب التطبيق داخل baa/ أو ضبط مساره من الإعدادات.\n");

#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
        console->appendPlainTextThreadSafe("تأكد من أن ملف baa لديه صلاحية التنفيذ (chmod +x).\n");
#endif
        emit toolingFinished(operation, -1);
        return;
    }

    // Safely clean up existing thread/worker before creating new ones.
    // The same console hosts an interactive shell, so stop it while Baa owns stdin/stdout.
    cleanupBuild();
    console->stopCmd();

    console->clear();
    console->appendPlainTextThreadSafe(heading);
    console->appendPlainTextThreadSafe("📄 السياق: " + QFileInfo(contextPath).fileName() + "\n");

    m_worker = new ProcessWorker(program, arguments, workingDirectory);
    m_buildThread = new QThread(this);

    m_worker->moveToThread(m_buildThread);

    connect(m_buildThread, &QThread::started, m_worker, &ProcessWorker::start);

    connect(m_worker, &ProcessWorker::outputReady, this, [this, console](const QString &text) {
        console->appendPlainTextThreadSafe(text);
        emit outputChunk(text);
    });
    connect(m_worker, &ProcessWorker::errorReady, this, [this, console](const QString &text) {
        console->appendPlainTextThreadSafe(text);
        emit outputChunk(text);
    });

    QThread *thread = m_buildThread.data();
    ProcessWorker *worker = m_worker.data();
    QPointer<TConsole> safeConsole(console);

    connect(m_worker, &ProcessWorker::finished, this, [this, safeConsole, thread, operation](int code) {
        if (safeConsole) {
            safeConsole->appendPlainTextThreadSafe(
                "\n──────────────────────────────\n✅ انتهى الأمر (Exit code = "
                + QString::number(code) + ")\n"
                );
            safeConsole->startCmd();
        }
        if (thread) {
            thread->quit();
        }
        emit buildFinished(code);
        emit toolingFinished(operation, code);
    });

    // Cleanup logic: ensure pointers are cleared after the worker thread finishes.
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(thread, &QThread::finished, this, [this, thread]() {
        if (m_buildThread == thread) {
            m_buildThread = nullptr;
        }
        m_worker = nullptr;
    });

    connect(console, &TConsole::commandEntered,
            m_worker, &ProcessWorker::sendInput);

    emit buildStarted();
    m_buildThread->start();
}
