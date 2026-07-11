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
            this, [this, checkProcess](int, QProcess::ExitStatus) {
                if (m_checkProcess != checkProcess) return;
                m_checkStdout += QString::fromUtf8(checkProcess->readAllStandardOutput());
                const QString payload = m_checkStdout;
                m_checkStdout.clear();
                checkProcess->deleteLater();
                m_checkProcess = nullptr;
                if (!payload.trimmed().isEmpty()) emit diagnosticsReady(payload);
            });
    connect(checkProcess, &QProcess::errorOccurred, this,
            [this, checkProcess](QProcess::ProcessError error) {
                if (error != QProcess::FailedToStart or m_checkProcess != checkProcess) return;
                m_checkStdout.clear();
                checkProcess->deleteLater();
                m_checkProcess = nullptr;
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
            args = { "run" };
            workingDir = projectRoot;
            usingTakween = true;
        }
    }

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
        return;
    }

    // Safely clean up existing thread/worker before creating new ones.
    // The same console hosts an interactive shell, so stop it while Baa owns stdin/stdout.
    cleanupBuild();
    console->stopCmd();

    console->clear();
    console->appendPlainTextThreadSafe(usingTakween
        ? "🚀 تشغيل مشروع تكوين...\n"
        : "🚀 بدء تشغيل ملف باء...\n");
    console->appendPlainTextThreadSafe("📄 الملف: " + QFileInfo(filePath).fileName() + "\n");

    m_worker = new ProcessWorker(program, args, workingDir);
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

    connect(m_worker, &ProcessWorker::finished, this, [this, safeConsole, thread](int code) {
        if (safeConsole) {
            safeConsole->appendPlainTextThreadSafe(
                "\n──────────────────────────────\n✅ انتهى التنفيذ (Exit code = "
                + QString::number(code) + ")\n"
                );
            safeConsole->startCmd();
        }
        if (thread) {
            thread->quit();
        }
        emit buildFinished(code);
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
