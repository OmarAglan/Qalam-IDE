#include "ProcessWorker.h"
#include <QDebug>
#include <QMutexLocker>

ProcessWorker::ProcessWorker(const QString &program, const QStringList &args, const QString &workingDir)
    : program(program), args(args), workingDir(workingDir), process(nullptr), flushTimer(nullptr)
{
    // Do NOT create timer here - it must be created in the worker thread
    // Timer will be created in start() which runs in the correct thread
}

void ProcessWorker::start() {
    // Create timer in the worker thread (correct thread affinity)
    if (!flushTimer) {
        flushTimer = new QTimer(this);
        flushTimer->setInterval(20);
        connect(flushTimer, &QTimer::timeout, this, &ProcessWorker::flushBuffers);
    }

    // Clean up any existing process
    if (process) {
        disconnect(process, nullptr, this, nullptr);
        process->deleteLater();
        process = nullptr;
    }
    process = new QProcess(this);

    process->setProgram(program);
    process->setArguments(args);
    process->setWorkingDirectory(workingDir);
    process->setProcessChannelMode(QProcess::SeparateChannels);

    // Connect read signals
    connect(process, &QProcess::readyReadStandardOutput,
            this, &ProcessWorker::onReadyReadOutput);
    connect(process, &QProcess::readyReadStandardError,
            this, &ProcessWorker::onReadyReadError);

    // Start timer when process starts
    connect(process, &QProcess::started, this, [this]() {
        if (flushTimer && !flushTimer->isActive())
            flushTimer->start();
    });

    // Stop timer and emit finished when process ends
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int code, QProcess::ExitStatus /*status*/) {
                if (flushTimer && flushTimer->isActive())
                    flushTimer->stop();

                // Flush any remaining buffered data
                flushBuffers();

                emit finished(code);
            });

    // Start the process (non-blocking)
    process->start();
}
void ProcessWorker::onReadyReadOutput() {
    QMutexLocker locker(&bufferMutex);
    QString s = QString::fromLocal8Bit(process->readAllStandardOutput());
    outputBuffer.append(s);
}

void ProcessWorker::onReadyReadError() {
    QMutexLocker locker(&bufferMutex);
    QString s = QString::fromLocal8Bit(process->readAllStandardError());
    errorBuffer.append(s);
}

void ProcessWorker::flushBuffers() {
    QString outCopy, errCopy;
    {
        QMutexLocker locker(&bufferMutex);
        outCopy = outputBuffer;
        errCopy = errorBuffer;
        outputBuffer.clear();
        errorBuffer.clear();
    }
    if (!outCopy.isEmpty()) {
        emit outputReady(outCopy);
    }
    if (!errCopy.isEmpty()) {
        emit errorReady(errCopy);
    }
}

void ProcessWorker::stop() {
    if (process && process->state() == QProcess::Running) {
        process->terminate();
        if (!process->waitForFinished(3000)) {
            process->kill();
        }
    }
}

void ProcessWorker::sendInput(const QString &text) {
    if (process && process->state() == QProcess::Running) {
#if defined(Q_OS_WIN)
        process->write((text + "\r\n").toLocal8Bit());
#else
        process->write((text + "\n").toLocal8Bit());
#endif
    }
}
