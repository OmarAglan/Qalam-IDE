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

    QStringList args = { filePath };
    QString workingDir = QFileInfo(filePath).absolutePath();

    // Safely clean up existing thread/worker before creating new ones.
    // The same console hosts an interactive shell, so stop it while Baa owns stdin/stdout.
    cleanupBuild();
    console->stopCmd();

    console->clear();
    console->appendPlainTextThreadSafe("🚀 بدء تشغيل ملف باء...\n");
    console->appendPlainTextThreadSafe("📄 الملف: " + QFileInfo(filePath).fileName() + "\n");

    m_worker = new ProcessWorker(program, args, workingDir);
    m_buildThread = new QThread(this);

    m_worker->moveToThread(m_buildThread);

    connect(m_buildThread, &QThread::started, m_worker, &ProcessWorker::start);

    connect(m_worker, &ProcessWorker::outputReady,
            console, &TConsole::appendPlainTextThreadSafe);
    connect(m_worker, &ProcessWorker::errorReady,
            console, &TConsole::appendPlainTextThreadSafe);

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
