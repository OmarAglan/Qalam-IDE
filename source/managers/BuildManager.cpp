#include "BuildManager.h"
#include "TConsole.h"
#include "Constants.h"

#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFile>

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
    QString program = settings.value(Constants::SettingsKeyCompilerPath).toString();

    if (program.isEmpty()) {
        QString appDir = QCoreApplication::applicationDirPath();
#if defined(Q_OS_WIN)
        QString localBaa = QDir(appDir).filePath("baa/baa.exe");
        if (QFile::exists(localBaa)) {
            program = localBaa;
        } else {
            program = "baa/baa.exe"; // Fallback to PATH or relative
        }
#elif defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
        program = QDir(appDir).filePath("baa/baa");
#endif
    }

    return program;
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

    if (!QFile::exists(program)) {
        console->clear();
        console->appendPlainTextThreadSafe("❌ خطأ: لم يتم العثور على مترجم باء!");
        console->appendPlainTextThreadSafe("المسار المتوقع: " + program);

#if defined(Q_OS_LINUX)
        console->appendPlainTextThreadSafe("تأكد من أن ملف 'alif' موجود ولديه صلاحية التنفيذ (chmod +x).");
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
