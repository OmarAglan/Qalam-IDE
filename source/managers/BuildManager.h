#pragma once

#include "ProcessWorker.h"
#include <QObject>
#include <QPointer>
#include <QThread>

class TConsole;

class BuildManager : public QObject {
    Q_OBJECT

public:
    explicit BuildManager(QObject *parent = nullptr);
    ~BuildManager();

    /// Run a Baa source file. Outputs go to the given console.
    void runBaa(const QString &filePath, TConsole *console);

    /// Stop the currently running build process, if any.
    void stop();

    /// Whether a build is currently running
    bool isRunning() const;

signals:
    /// Emitted when a build starts
    void buildStarted();
    /// Emitted when a build finishes with the given exit code
    void buildFinished(int exitCode);

private:
    /// Resolve the compiler path from settings or default locations
    QString resolveCompilerPath() const;

    /// Clean up existing thread/worker safely
    void cleanupBuild();

    QPointer<ProcessWorker> m_worker;
    QThread *m_buildThread = nullptr;
};
