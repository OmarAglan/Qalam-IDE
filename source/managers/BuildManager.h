#pragma once

#include "ProcessWorker.h"
#include <QObject>
#include <QPointer>
#include <QProcess>
#include <QThread>

class TConsole;

class BuildManager : public QObject {
    Q_OBJECT

public:
    enum class CompilerExitClass {
        Success,
        SourceError,
        InvalidInvocation,
        Unsupported,
        ToolchainError,
        InternalError,
        ProcessFailure,
        Unknown
    };
    Q_ENUM(CompilerExitClass)

    explicit BuildManager(QObject *parent = nullptr);
    ~BuildManager();

    /// Run a Baa source file. Outputs go to the given console.
    void runBaa(const QString &filePath, TConsole *console);

    /// Run a supported Takween project command for the project owning filePath.
    bool runTakweenCommand(const QString &filePath, const QString &command, TConsole *console);

    /// Run Baa's non-codegen structured diagnostic check for a saved source file.
    void checkBaa(const QString &filePath);

    /// Build the stable compiler arguments used by the editor check path.
    static QStringList baaCheckArguments(const QString &filePath);

    /// Build argv for the supported Takween project commands, or an empty list.
    static QStringList takweenCommandArguments(const QString &command);

    /// Classify compiler-cli-v1 codes without inspecting human-readable output.
    static CompilerExitClass classifyCompilerExitCode(int exitCode);

    /// Stable diagnostic identifier for a process/compiler exit code.
    static QString compilerExitCodeId(int exitCode);

    /// Arabic, operation-aware fallback used when structured diagnostics are empty.
    static QString compilerExitSummary(int exitCode, const QString &operation);

    /// Find the nearest Takween v0/v1 project root containing مشروع.تكوين.
    static QString findTakweenProjectRoot(const QString &filePath);

    /// Stop the currently running build process, if any.
    void stop();

    /// Whether a build is currently running
    bool isRunning() const;

signals:
    /// Emitted when a build starts
    void buildStarted();
    /// Emitted when a build finishes with the given exit code
    void buildFinished(int exitCode);
    /// Raw output chunks from the compiler/process, for diagnostics parsing.
    void outputChunk(const QString &text);
    /// Complete diagnostics-json-v1 payload emitted by a fast Baa check.
    void diagnosticsReady(const QString &json);
    /// Completion event with an explicit operation and unmodified process exit code.
    void toolingFinished(const QString &operation, int exitCode);

private:
    /// Resolve the compiler path from settings or default locations
    QString resolveCompilerPath() const;
    QString resolveTakweenPath() const;

    /// Clean up existing thread/worker safely
    void cleanupBuild();
    void startProcess(const QString &program,
                      const QStringList &arguments,
                      const QString &workingDirectory,
                      const QString &contextPath,
                      const QString &operation,
                      const QString &heading,
                      TConsole *console);

    QPointer<ProcessWorker> m_worker;
    QPointer<QThread> m_buildThread;
    QPointer<QProcess> m_checkProcess;
    QString m_checkStdout;
};
