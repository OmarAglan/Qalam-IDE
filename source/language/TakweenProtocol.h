#pragma once

#include <QByteArray>
#include <QMetaType>
#include <QString>
#include <QVector>

struct TakweenTarget {
    QString name;
    QString kind;
    QString status;
    bool buildable{};
    bool runnable{};
    bool test{};
};

struct TakweenBuildEvent {
    QString schemaVersion;
    qint64 sequence{};
    QString event;
    QString operation;
    QString phase;
    QString status;
    QString target;
    QString package;
    bool hasExitCode{};
    int exitCode{};
    QString artifactKind;
    QString artifactPath;
};

class TakweenProtocol
{
public:
    static bool parseTargets(const QByteArray &json,
                             QVector<TakweenTarget> *targets,
                             QString *error = nullptr);
    static bool parseBuildEvent(const QByteArray &line,
                                TakweenBuildEvent *event,
                                QString *error = nullptr);
    static bool validateTransition(const TakweenBuildEvent &event,
                                   const QString &expectedOperation,
                                   qint64 previousSequence,
                                   bool terminalSeen,
                                   QString *error = nullptr);
    static bool validateCompletion(int processExitCode,
                                   bool cancellationRequested,
                                   bool protocolFailed,
                                   bool terminalSeen,
                                   int terminalExitCode,
                                   QString *error = nullptr);
    static QString progressText(const TakweenBuildEvent &event);
};

Q_DECLARE_METATYPE(TakweenTarget)
Q_DECLARE_METATYPE(QVector<TakweenTarget>)
Q_DECLARE_METATYPE(TakweenBuildEvent)
