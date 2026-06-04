#include "BuildManager.h"
#include "TConsole.h"
#include "Constants.h"

#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QStandardPaths>

BuildManager::BuildManager(QObject *parent)
    : QObject(parent)
{
}

BuildManager::~BuildManager()
{
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

void BuildManager::cleanupBuild()
{
    if (m_buildThread) {
        if (m_worker) {
            m_worker->stop();
            m_worker = nullptr;
        }
        m_buildThread->quit();
        if (!m_buildThread->wait(3000)) {
            m_buildThread->terminate();
            m_buildThread->wait();
        }
        if (m_buildThread) {
            m_buildThread->deleteLater();
            m_buildThread = nullptr;
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

    if (!QFileInfo(program).isExecutable()) {
        const QString pathProgram = QStandardPaths::findExecutable(program);
        if (!pathProgram.isEmpty()) {
            program = pathProgram;
        }
    }

    if (!QFileInfo(program).isExecutable()) {
        console->clear();
        console->appendPlainTextThreadSafe("❌ خطأ: لم يتم العثور على مترجم باء!");
        console->appendPlainTextThreadSafe("المسار المتوقع: " + program);
        console->appendPlainTextThreadSafe("يمكنك وضع المترجم بجانب التطبيق داخل baa/ أو ضبط مساره من الإعدادات.");

#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
        console->appendPlainTextThreadSafe("تأكد من أن ملف baa لديه صلاحية التنفيذ (chmod +x).");
#endif
        return;
    }

    QStringList args = { filePath };
    QString workingDir = QFileInfo(filePath).absolutePath();

    // Safely clean up existing thread/worker before creating new ones
    cleanupBuild();

    console->clear();
    console->appendPlainTextThreadSafe("🚀 بدء تشغيل ملف باء...");
    console->appendPlainTextThreadSafe("📄 الملف: " + QFileInfo(filePath).fileName());

    m_worker = new ProcessWorker(program, args, workingDir);
    m_buildThread = new QThread(this);

    m_worker->moveToThread(m_buildThread);

    connect(m_buildThread, &QThread::started, m_worker, &ProcessWorker::start);

    connect(m_worker, &ProcessWorker::outputReady,
            console, &TConsole::appendPlainTextThreadSafe);
    connect(m_worker, &ProcessWorker::errorReady,
            console, &TConsole::appendPlainTextThreadSafe);

    connect(m_worker, &ProcessWorker::finished, this, [this, console](int code) {
        console->appendPlainTextThreadSafe(
            "\n──────────────────────────────\n✅ انتهى التنفيذ (Exit code = "
            + QString::number(code) + ")\n"
            );
        m_buildThread->quit();
        emit buildFinished(code);
    });

    // Cleanup logic: Ensure thread and worker are deleted after the thread finishes
    connect(m_buildThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_buildThread, &QThread::finished, m_buildThread, &QObject::deleteLater);

    connect(console, &TConsole::commandEntered,
            m_worker, &ProcessWorker::sendInput);

    emit buildStarted();
    m_buildThread->start();
}
