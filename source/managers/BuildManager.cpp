#include "BuildManager.h"
#include "QalamConsole.h"
#include "Constants.h"
#include "ToolchainDiscovery.h"

#include <QSettings>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QStandardPaths>
#include <QMetaObject>
#include <QPointer>
#include <QTimer>
#include <QUuid>

namespace {

QString standaloneRunExecutablePath(const QString &sourcePath)
{
    QString cacheRoot = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (cacheRoot.isEmpty()) {
        cacheRoot = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    }
    if (cacheRoot.isEmpty()) cacheRoot = QDir::tempPath();

    QDir runDirectory(QDir(cacheRoot).filePath(QStringLiteral("تشغيل")));
    if (not runDirectory.exists() and not QDir().mkpath(runDirectory.absolutePath())) {
        return QString();
    }

    QString identity = QFileInfo(sourcePath).canonicalFilePath();
    if (identity.isEmpty()) identity = QFileInfo(sourcePath).absoluteFilePath();
    const QString digest = QString::fromLatin1(
        QCryptographicHash::hash(identity.toUtf8(), QCryptographicHash::Sha256)
            .toHex()
            .left(16));
#if defined(Q_OS_WIN)
    const QString executableName = QStringLiteral("برنامج-%1.exe").arg(digest);
#else
    const QString executableName = QStringLiteral("برنامج-%1").arg(digest);
#endif
    return runDirectory.filePath(executableName);
}

}

BuildManager::BuildManager(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<TakweenBuildEvent>();
    qRegisterMetaType<QVector<TakweenTarget>>();
}

BuildManager::~BuildManager()
{
    cleanupBuild();
}

bool BuildManager::isRunning() const
{
    return (m_buildThread and m_buildThread->isRunning()) or m_checkProcess;
}

QString BuildManager::resolveCompilerProgram()
{
    return ToolchainDiscovery::resolve(QalamToolKind::Baa).program;
}

QString BuildManager::resolveTakweenProgram()
{
    return ToolchainDiscovery::resolve(QalamToolKind::Takween).program;
}

QString BuildManager::resolveNazmProgram()
{
    return ToolchainDiscovery::resolve(QalamToolKind::Nazm).program;
}

bool BuildManager::isNazmSourcePath(const QString &filePath)
{
    return filePath.endsWith(QStringLiteral(".نظم"), Qt::CaseInsensitive);
}

QString BuildManager::nazmObjectPath(const QString &filePath)
{
    QFileInfo source(filePath);
#if defined(Q_OS_WIN)
    const QString suffix = QStringLiteral(".obj");
#else
    const QString suffix = QStringLiteral(".o");
#endif
    return source.dir().filePath(source.completeBaseName() + suffix);
}

QStringList BuildManager::nazmCommandArguments(const QString &filePath,
                                                const QString &objectPath)
{
#if defined(Q_OS_WIN)
    const QString format = QStringLiteral("كوف");
#else
    const QString format = QStringLiteral("إلف64");
#endif
    return {filePath, QStringLiteral("--خرج"), objectPath,
            QStringLiteral("--صيغة"), format};
}

BuildManager::ToolActionState BuildManager::toolActionState(
    const QString &filePath,
    const QString &operation,
    bool baaAvailable,
    bool takweenAvailable,
    bool nazmAvailable)
{
    const QString normalized = operation.trimmed().toLower();
    if (filePath.trimmed().isEmpty()) {
        return {false, QStringLiteral("افتح ملف باء أو نظم محفوظًا أولًا.")};
    }

    const bool baaSource = Constants::isBaaImplementationPath(filePath);
    const bool nazmSource = isNazmSourcePath(filePath);
    if (not baaSource and not nazmSource) {
        return {false, QStringLiteral("الأمر متاح لملفات باء أو نظم فقط.")};
    }

    QStringList requiredTools;
    const bool project = not findTakweenProjectRoot(filePath).isEmpty();
    if (project) {
        requiredTools << QStringLiteral("تكوين");
        if (normalized != QStringLiteral("clean")) {
            requiredTools << QStringLiteral("باء") << QStringLiteral("نظم");
        }
    } else if (normalized == QStringLiteral("build") and nazmSource) {
        requiredTools << QStringLiteral("نظم");
    } else if (normalized == QStringLiteral("run")) {
        requiredTools << QStringLiteral("باء") << QStringLiteral("نظم");
    } else if (normalized == QStringLiteral("build") and baaSource) {
        return {false, QStringLiteral(
            "بناء ملف باء المفرد يتم عند التشغيل؛ استخدم F5.")};
    } else {
        return {false, QStringLiteral(
            "تتطلب هذه العملية ملفًا داخل مشروع تكوين.")};
    }

    QStringList missing;
    for (const QString &tool : requiredTools) {
        if ((tool == QStringLiteral("باء") and not baaAvailable) or
            (tool == QStringLiteral("تكوين") and not takweenAvailable) or
            (tool == QStringLiteral("نظم") and not nazmAvailable)) {
            missing << tool;
        }
    }
    if (not missing.isEmpty()) {
        return {false, QStringLiteral(
            "الأدوات المطلوبة غير متاحة: %1. ثبّتها في PATH أو اضبط مساراتها من الإعدادات.")
            .arg(missing.join(QStringLiteral("، ")))};
    }
    return {true, QString()};
}

QStringList BuildManager::takweenCommandArguments(const QString &command,
                                                   const QString &targetName,
                                                   const QString &profileName)
{
    const QString normalized = command.trimmed().toLower();
    QString canonical;
    if (normalized == "build") canonical = "بناء";
    else if (normalized == "check") canonical = "فحص";
    else if (normalized == "run") canonical = "تشغيل";
    else if (normalized == "test") canonical = "اختبار";
    else if (normalized == "clean") canonical = "تنظيف";
    else return {};

    const QString target = targetName.trimmed();
    const QString profile = profileName.trimmed();
    // A leading dash would turn a remembered name into a Takween option.
    if (target.startsWith(QLatin1Char('-')) or profile.startsWith(QLatin1Char('-'))) return {};

    QStringList arguments = {canonical};
    if (normalized == "clean") {
        if (not target.isEmpty() or not profile.isEmpty()) return {};
        return arguments;
    }
    if (not target.isEmpty()) arguments.push_back(target);
    if (not profile.isEmpty()) arguments << QStringLiteral("--نمط") << profile;
    return arguments;
}

QStringList BuildManager::builtInTakweenProfiles()
{
    return {QStringLiteral("تطوير"), QStringLiteral("إصدار")};
}

QString BuildManager::takweenSelectionKey(const QString &projectRoot)
{
    QString identity = QDir::cleanPath(QFileInfo(projectRoot).absoluteFilePath());
#if defined(Q_OS_WIN)
    identity = identity.toLower();
#endif
    return QStringLiteral("takweenProjects/") + QString::fromLatin1(
        QCryptographicHash::hash(identity.toUtf8(), QCryptographicHash::Sha256)
            .toHex()
            .left(16));
}

BuildManager::TakweenSelection BuildManager::takweenSelection(const QString &projectRoot)
{
    if (projectRoot.isEmpty()) return {};
    QSettings settings = Constants::settings();
    settings.beginGroup(takweenSelectionKey(projectRoot));
    return {settings.value(QStringLiteral("target")).toString(),
            settings.value(QStringLiteral("profile")).toString()};
}

void BuildManager::setTakweenSelection(const QString &projectRoot,
                                       const TakweenSelection &selection)
{
    if (projectRoot.isEmpty()) return;
    QSettings settings = Constants::settings();
    settings.beginGroup(takweenSelectionKey(projectRoot));
    const QString target = selection.target.trimmed();
    const QString profile = selection.profile.trimmed();
    if (target.isEmpty() and profile.isEmpty()) {
        settings.remove(QString());
        return;
    }
    // The key is a digest; keep the readable root beside it for humans.
    settings.setValue(QStringLiteral("root"), QDir::cleanPath(projectRoot));
    auto store = [&settings](const QString &key, const QString &value) {
        if (value.isEmpty()) settings.remove(key);
        else settings.setValue(key, value);
    };
    store(QStringLiteral("target"), target);
    store(QStringLiteral("profile"), profile);
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
    case -2: return CompilerExitClass::Cancelled;
    default:
        return exitCode < 0 ? CompilerExitClass::ProcessFailure : CompilerExitClass::Unknown;
    }
}

QString BuildManager::compilerExitCodeId(int exitCode)
{
    if (exitCode >= 0 and exitCode <= 5) {
        return QString("CLI_EXIT_%1").arg(exitCode);
    }
    if (exitCode == -2) return "TOOLING_CANCELLED";
    if (exitCode < 0) return "PROCESS_FAILURE";
    return QString("UNKNOWN_EXIT_%1").arg(exitCode);
}

QString BuildManager::compilerExitSummary(int exitCode, const QString &operation)
{
    if (exitCode == -2) return "أُلغيت عملية البناء بطلب المستخدم.";
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
    case CompilerExitClass::Cancelled:
        return "أُلغيت عملية البناء بطلب المستخدم.";
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

QVector<TakweenTarget> BuildManager::discoverTakweenTargets(const QString &filePath,
                                                            QString *error) const
{
    const QString projectRoot = findTakweenProjectRoot(filePath);
    if (projectRoot.isEmpty()) {
        if (error) *error = "لم يُعثر على مشروع.تكوين.";
        return {};
    }

    QByteArray output;
    if (not runTakweenQuery(projectRoot, {"أهداف", "--جسون"}, &output, error)) return {};

    QVector<TakweenTarget> targets;
    QString parseError;
    if (not TakweenProtocol::parseTargets(output, &targets, &parseError)) {
        if (error) *error = parseError;
        return {};
    }
    return targets;
}

bool BuildManager::loadTakweenBuildPlan(const QString &filePath,
                                        const TakweenSelection &selection,
                                        TakweenBuildPlan *plan,
                                        QString *error) const
{
    const QString projectRoot = findTakweenProjectRoot(filePath);
    if (projectRoot.isEmpty()) {
        if (error) *error = "لم يُعثر على مشروع.تكوين.";
        return false;
    }
    // `خطة` shares the target/profile grammar of `بناء`.
    QStringList arguments = takweenCommandArguments("build", selection.target, selection.profile);
    if (arguments.isEmpty()) {
        if (error) *error = "اسم الهدف أو النمط غير صالح.";
        return false;
    }
    arguments[0] = QStringLiteral("خطة");
    arguments.insert(1, QStringLiteral("--جسون"));

    QByteArray output;
    if (not runTakweenQuery(projectRoot, arguments, &output, error)) return false;
    QString parseError;
    if (not TakweenProtocol::parseBuildPlan(output, plan, &parseError)) {
        if (error) *error = parseError;
        return false;
    }
    return true;
}

bool BuildManager::runTakweenQuery(const QString &projectRoot,
                                   const QStringList &arguments,
                                   QByteArray *output,
                                   QString *error) const
{
    const QString takween = resolveTakweenProgram();
    if (takween.isEmpty()) {
        if (error) *error = "لم يُعثر على برنامج تكوين القابل للتنفيذ.";
        return false;
    }

    QProcess process;
    process.setProgram(takween);
    process.setArguments(arguments);
    process.setWorkingDirectory(projectRoot);
    process.setProcessEnvironment(ToolchainDiscovery::processEnvironment());
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start();
    if (not process.waitForStarted(3000)) {
        if (error) *error = "تعذر بدء تكوين: " + process.errorString();
        return false;
    }
    // Planning may resolve dependencies, so it gets more time than a listing.
    if (not process.waitForFinished(15000)) {
        process.kill();
        process.waitForFinished(500);
        if (error) *error = "انتهت مهلة انتظار تكوين.";
        return false;
    }
    if (process.exitStatus() != QProcess::NormalExit or process.exitCode() != 0) {
        if (error) {
            const QString detail = QString::fromUtf8(process.readAllStandardError()).trimmed();
            *error = detail.isEmpty()
                ? QString("فشل تكوين بكود %1.").arg(process.exitCode())
                : detail;
        }
        return false;
    }
    *output = process.readAllStandardOutput();
    return true;
}

QVector<TakweenTarget> BuildManager::selectableTakweenTargets(
    const QVector<TakweenTarget> &targets,
    const QString &command)
{
    const QString normalized = command.trimmed().toLower();
    QVector<TakweenTarget> selected;
    for (const TakweenTarget &target : targets) {
        const bool selectable =
            (normalized == "build" and target.buildable) or
            (normalized == "run" and target.runnable) or
            (normalized == "test" and target.test);
        if (selectable) selected.push_back(target);
    }
    return selected;
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
    if (isRunning()) m_cancelRequested = true;
    if (m_checkProcess) {
        m_checkProcess->setProperty("qalam.cancelled", true);
        stopTakweenCheck();
    }
    cleanupBuild();
}

void BuildManager::stopTakweenCheck()
{
    QProcess *process = m_checkProcess.data();
    if (not process) return;
    process->kill();
    process->waitForFinished(1000);
}

void BuildManager::startTakweenCheck(const std::function<void()> &finish)
{
    const QString target = m_takweenFailedTarget.isEmpty()
        ? m_takweenRunSelection.target : m_takweenFailedTarget;
    const QStringList arguments =
        takweenCommandArguments("check", target, m_takweenRunSelection.profile);
    const QString takween = resolveTakweenProgram();
    if (arguments.isEmpty() or takween.isEmpty() or m_takweenProjectRoot.isEmpty()) {
        finish();
        return;
    }

    auto *process = new QProcess(this);
    m_checkProcess = process;
    process->setProgram(takween);
    process->setArguments(arguments);
    process->setWorkingDirectory(m_takweenProjectRoot);
    process->setProcessEnvironment(ToolchainDiscovery::processEnvironment());
    process->setProcessChannelMode(QProcess::SeparateChannels);
    QTimer::singleShot(60000, process, [process]() { process->kill(); });

    const QString root = m_takweenProjectRoot;
    auto complete = [this, process, root, finish](bool exited) {
        if (m_checkProcess != process) return;
        m_checkProcess = nullptr;
        process->deleteLater();
        // A newer operation superseded this check; its result is stale.
        if (process->property("qalam.superseded").toBool()) return;
        const QByteArray output = exited ? process->readAllStandardOutput() : QByteArray();
        if (not process->property("qalam.cancelled").toBool() and
            not output.trimmed().isEmpty()) {
            emit takweenDiagnosticsReady(root, output);
        }
        finish();
    };
    connect(process, &QProcess::finished, this,
            [complete](int, QProcess::ExitStatus status) {
        complete(status == QProcess::NormalExit);
    });
    connect(process, &QProcess::errorOccurred, this,
            [complete](QProcess::ProcessError processError) {
        if (processError == QProcess::FailedToStart) complete(false);
    });
    emit toolingProgress(QStringLiteral("جمع تشخيصات تكوين…"));
    process->start();
}

void BuildManager::runBaa(const QString &filePath, QalamConsole *console)
{
    if (!console) return;

    QString program = resolveCompilerProgram();
    QStringList args = { filePath };
    QString workingDir = QFileInfo(filePath).absolutePath();
    bool usingTakween = false;
    QString followUpProgram;

    const QString projectRoot = findTakweenProjectRoot(filePath);
    if (!projectRoot.isEmpty()) {
        const QString takween = resolveTakweenProgram();
        if (!takween.isEmpty()) {
            program = takween;
            args = takweenCommandArguments("run");
            workingDir = projectRoot;
            usingTakween = true;
            m_takweenRunSelection = {};
        }
    }

    if (not usingTakween) {
        followUpProgram = standaloneRunExecutablePath(filePath);
        if (followUpProgram.isEmpty()) {
            console->clear();
            console->appendPlainTextThreadSafe(
                QStringLiteral("❌ تعذر إنشاء مجلد تشغيل مؤقت لبرنامج باء.\n"));
            emit toolingFinished(QStringLiteral("run"), -1);
            return;
        }
        QFile::remove(followUpProgram);
        const QString nazm = resolveNazmProgram();
        if (not nazm.isEmpty()) {
            args << (QStringLiteral("--nazm-path=") + nazm);
        }
        args << QStringLiteral("-o") << followUpProgram;
    }

    startProcess(program,
                 args,
                 workingDir,
                 filePath,
                 QStringLiteral("run"),
                 usingTakween
                     ? QStringLiteral("🚀 تشغيل مشروع تكوين...\n")
                     : (isNazmSourcePath(filePath)
                            ? QStringLiteral("🚀 ربط وتشغيل ملف نظم...\n")
                            : QStringLiteral("🚀 بدء تشغيل ملف باء...\n")),
                 console,
                 usingTakween,
                 followUpProgram,
                 {},
                 workingDir,
                 usingTakween ? QString() : QStringLiteral("\n▶ مخرجات البرنامج:\n"));
}

void BuildManager::buildNazm(const QString &filePath, QalamConsole *console)
{
    if (not console or not isNazmSourcePath(filePath)) return;
    const QString output = nazmObjectPath(filePath);
    QFile::remove(output);
    startProcess(resolveNazmProgram(),
                 nazmCommandArguments(filePath, output),
                 QFileInfo(filePath).absolutePath(),
                 filePath,
                 QStringLiteral("assemble"),
                 QStringLiteral("🛠️ تجميع ملف نظم إلى %1...\n")
                     .arg(QFileInfo(output).fileName()),
                 console);
}

bool BuildManager::runTakweenCommand(const QString &filePath,
                                     const QString &command,
                                     QalamConsole *console,
                                     const QString &targetName,
                                     const QString &profileName)
{
    if (!console) return false;

    const QStringList arguments = takweenCommandArguments(command, targetName, profileName);
    const QString projectRoot = findTakweenProjectRoot(filePath);
    const QString takween = resolveTakweenProgram();
    if (arguments.isEmpty() or projectRoot.isEmpty() or takween.isEmpty()) return false;

    const QString normalized = command.trimmed().toLower();
    QString heading = "🚀 تنفيذ أمر مشروع تكوين...\n";
    if (normalized == "build") heading = "🛠️ بناء مشروع تكوين...\n";
    else if (normalized == "run") heading = "🚀 تشغيل مشروع تكوين...\n";
    else if (normalized == "test") heading = "🧪 اختبار مشروع تكوين...\n";
    else if (normalized == "clean") heading = "🧹 تنظيف مشروع تكوين...\n";

    m_takweenRunSelection = {targetName.trimmed(), profileName.trimmed()};
    startProcess(
        takween, arguments, projectRoot, filePath, normalized, heading, console, true);
    return true;
}

void BuildManager::startProcess(const QString &requestedProgram,
                                const QStringList &arguments,
                                const QString &workingDirectory,
                                const QString &contextPath,
                                const QString &operation,
                                const QString &heading,
                                QalamConsole *console,
                                bool usesTakweenEvents,
                                const QString &followUpProgram,
                                const QStringList &followUpArguments,
                                const QString &followUpWorkingDirectory,
                                const QString &followUpHeading)
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
        console->appendPlainTextThreadSafe("❌ خطأ: لم يتم العثور على أداة البناء المطلوبة!\n");
        console->appendPlainTextThreadSafe("المسار المتوقع: " + program + "\n");
        console->appendPlainTextThreadSafe("ثبّت الأداة في PATH أو اضبط مسارها من الإعدادات.\n");

#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
        console->appendPlainTextThreadSafe("تأكد من أن ملف baa لديه صلاحية التنفيذ (chmod +x).\n");
#endif
        emit toolingFinished(operation, -1);
        return;
    }

    // Safely clean up existing thread/worker before creating new ones.
    // The same console hosts an interactive shell, so stop it while Baa owns stdin/stdout.
    if (m_checkProcess) {
        m_checkProcess->setProperty("qalam.superseded", true);
        stopTakweenCheck();
    }
    cleanupBuild();
    console->stopCmd();
    console->beginTask(heading.section(QLatin1Char('\n'), 0, 0));

    console->clear();
    console->appendPlainTextThreadSafe(heading);
    console->appendPlainTextThreadSafe("📄 السياق: " + QFileInfo(contextPath).fileName() + "\n");

    QStringList processArguments = arguments;
    QString eventFilePath;
    if (usesTakweenEvents) {
        QString temporaryRoot = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        if (temporaryRoot.isEmpty()) temporaryRoot = QDir::tempPath();
        eventFilePath = QDir(temporaryRoot).filePath(
            "قلم-أحداث-تكوين-" + QUuid::createUuid().toString(QUuid::WithoutBraces) + ".jsonl");
        QFile::remove(eventFilePath);
        processArguments << "--ملف_أحداث" << eventFilePath;
    }

    m_lastEventSequence = 0;
    m_terminalEventSeen = false;
    m_eventProtocolFailed = false;
    m_terminalEventExitCode = 0;
    m_cancelRequested = false;
    m_takweenCompilerFailed = false;
    m_takweenFailedTarget.clear();
    m_takweenProjectRoot = usesTakweenEvents ? workingDirectory : QString();

    m_worker = new ProcessWorker(
        program,
        processArguments,
        workingDirectory,
        eventFilePath,
        followUpProgram,
        followUpArguments,
        followUpWorkingDirectory,
        followUpHeading,
        ToolchainDiscovery::processEnvironment());
    m_buildThread = new QThread(this);

    m_worker->moveToThread(m_buildThread);

    connect(m_buildThread, &QThread::started, m_worker, &ProcessWorker::start);

    // Takween failures are diagnosed through `فحص` JSON, never by scraping text.
    connect(m_worker, &ProcessWorker::outputReady, this,
            [this, console, usesTakweenEvents](const QString &text) {
        console->appendPlainTextThreadSafe(text);
        if (not usesTakweenEvents) emit outputChunk(text);
    });
    connect(m_worker, &ProcessWorker::errorReady, this,
            [this, console, usesTakweenEvents](const QString &text) {
        console->appendPlainTextThreadSafe(text);
        if (not usesTakweenEvents) emit outputChunk(text);
    });
    connect(m_worker, &ProcessWorker::eventLineReady, this,
            [this, console, operation](const QByteArray &line) {
                auto reject = [this, console](const QString &message) {
                    if (m_eventProtocolFailed) return;
                    m_eventProtocolFailed = true;
                    const QString diagnostic = "❌ خرق عقد أحداث تكوين: " + message;
                    console->appendPlainTextThreadSafe(diagnostic + "\n");
                    emit toolingProtocolError(diagnostic);
                };

                TakweenBuildEvent event;
                QString error;
                if (not TakweenProtocol::parseBuildEvent(line, &event, &error)) {
                    reject(error);
                    return;
                }
                if (not TakweenProtocol::validateTransition(
                        event, operation, m_lastEventSequence, m_terminalEventSeen, &error)) {
                    reject(error);
                    return;
                }

                m_lastEventSequence = event.sequence;
                if (event.event == "target_started" and not m_takweenCompilerFailed) {
                    m_takweenFailedTarget = event.target;
                }
                if (event.event == "phase_finished" and event.status == "failed" and
                    (event.phase == "compiler" or event.phase == "compiler_check")) {
                    m_takweenCompilerFailed = true;
                }
                if (event.event == "operation_finished") {
                    m_terminalEventSeen = true;
                    m_terminalEventExitCode = event.exitCode;
                }
                emit takweenEventReady(event);
                const QString progress = TakweenProtocol::progressText(event);
                if (not progress.isEmpty()) emit toolingProgress(progress);
            });

    QThread *thread = m_buildThread.data();
    ProcessWorker *worker = m_worker.data();
    QPointer<QalamConsole> safeConsole(console);

    connect(m_worker, &ProcessWorker::finished, this,
            [this, safeConsole, thread, operation, eventFilePath](int code) {
        int effectiveCode = code;
        if (m_cancelRequested or code == -2) {
            effectiveCode = -2;
        } else if (not eventFilePath.isEmpty()) {
            QString completionError;
            if (not TakweenProtocol::validateCompletion(
                    code, false, m_eventProtocolFailed, m_terminalEventSeen,
                    m_terminalEventExitCode, &completionError)) {
                effectiveCode = -1;
                if (not m_eventProtocolFailed) {
                    const QString message = "❌ خرق عقد أحداث تكوين: " + completionError;
                    if (safeConsole) safeConsole->appendPlainTextThreadSafe(message + "\n");
                    emit toolingProtocolError(message);
                }
            }
        }
        if (safeConsole) {
            QString result;
            if (effectiveCode == -2) {
                result = "\n──────────────────────────────\n⏹ أُلغيت العملية.\n";
            } else if (effectiveCode == 0) {
                result = "\n──────────────────────────────\n✅ اكتمل الأمر بنجاح.\n";
            } else {
                result = "\n──────────────────────────────\n❌ فشل الأمر (Exit code = "
                    + QString::number(effectiveCode) + ")\n";
            }
            safeConsole->appendPlainTextThreadSafe(result);
            safeConsole->startCmd();
        }
        if (thread) {
            thread->quit();
        }
        m_cancelRequested = false;
        auto finish = [this, operation, effectiveCode]() {
            emit buildFinished(effectiveCode);
            emit toolingFinished(operation, effectiveCode);
        };
        if (not eventFilePath.isEmpty() and effectiveCode != -2 and m_takweenCompilerFailed) {
            startTakweenCheck(finish);
        } else {
            finish();
        }
    });

    // Cleanup logic: ensure pointers are cleared after the worker thread finishes.
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(thread, &QThread::finished, this, [this, thread, eventFilePath]() {
        if (not eventFilePath.isEmpty()) QFile::remove(eventFilePath);
        if (m_buildThread == thread) {
            m_buildThread = nullptr;
        }
        m_worker = nullptr;
    });

    connect(console, &QalamConsole::commandEntered,
            m_worker, &ProcessWorker::sendInput);

    emit buildStarted();
    m_buildThread->start();
}
