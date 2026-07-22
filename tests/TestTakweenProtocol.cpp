#include "TakweenProtocol.h"

#include <QtTest/QtTest>

class TestTakweenProtocol : public QObject
{
    Q_OBJECT

private slots:
    void parsesTargetsContract();
    void rejectsInvalidTargetsContract();
    void parsesBuildLifecycleEvents();
    void rejectsInvalidBuildEvents();
    void validatesStreamOrderingAndCompletion();
    void rendersArabicProgress();
};

void TestTakweenProtocol::parsesTargetsContract()
{
    const QByteArray json = R"json({
        "schema_version": "takween-targets-v1",
        "targets": [
            {"name":"تطبيق","kind":"executable","status":"ready","buildable":true,"runnable":true,"test":false,"future":7},
            {"name":"اختبار_أ","kind":"test","status":"ready","buildable":true,"runnable":true,"test":true}
        ],
        "future_root": true
    })json";
    QVector<TakweenTarget> targets;
    QString error;
    QVERIFY2(TakweenProtocol::parseTargets(json, &targets, &error), qPrintable(error));
    QCOMPARE(targets.size(), 2);
    QCOMPARE(targets[0].name, QString("تطبيق"));
    QVERIFY(targets[0].buildable);
    QVERIFY(targets[1].test);
}

void TestTakweenProtocol::rejectsInvalidTargetsContract()
{
    QVector<TakweenTarget> targets;
    QString error;
    QVERIFY(not TakweenProtocol::parseTargets(
        R"json({"schema_version":"other","targets":[]})json", &targets, &error));
    QVERIFY(not error.isEmpty());
    QVERIFY(not TakweenProtocol::parseTargets(
        R"json({"schema_version":"takween-targets-v1","targets":[{"name":"أ","kind":"test","status":"ready","buildable":"yes","runnable":true,"test":true}]})json",
        &targets, &error));
    QVERIFY(not TakweenProtocol::parseTargets(
        R"json({"schema_version":"takween-targets-v1","targets":[{"name":"أ","kind":"test","status":"ready","buildable":true,"runnable":true,"test":true},{"name":"أ","kind":"test","status":"ready","buildable":true,"runnable":true,"test":true}]})json",
        &targets, &error));
}

void TestTakweenProtocol::parsesBuildLifecycleEvents()
{
    TakweenBuildEvent event;
    QString error;
    QVERIFY2(TakweenProtocol::parseBuildEvent(
        R"json({"schema_version":"takween-build-events-v1","sequence":1,"event":"operation_started","operation":"build","phase":"operation","status":"started","future":true})json",
        &event, &error), qPrintable(error));
    QCOMPARE(event.sequence, 1);
    QCOMPARE(event.operation, QString("build"));
    QVERIFY(not event.hasExitCode);

    QVERIFY2(TakweenProtocol::parseBuildEvent(
        R"json({"schema_version":"takween-build-events-v1","sequence":2,"event":"phase_finished","operation":"build","phase":"compiler","status":"failed","exit_code":4})json",
        &event, &error), qPrintable(error));
    QVERIFY(event.hasExitCode);
    QCOMPARE(event.exitCode, 4);

    QVERIFY2(TakweenProtocol::parseBuildEvent(
        R"json({"schema_version":"takween-build-events-v1","sequence":3,"event":"artifact","operation":"build","artifact":{"kind":"executable","path":"بناء/تطبيق.exe"}})json",
        &event, &error), qPrintable(error));
    QCOMPARE(event.artifactKind, QString("executable"));
    QCOMPARE(event.artifactPath, QString("بناء/تطبيق.exe"));
}

void TestTakweenProtocol::rejectsInvalidBuildEvents()
{
    TakweenBuildEvent event;
    QString error;
    QVERIFY(not TakweenProtocol::parseBuildEvent(
        R"json({"schema_version":"takween-build-events-v1","sequence":0,"event":"operation_started","operation":"build","phase":"operation","status":"started"})json",
        &event, &error));
    QVERIFY(not TakweenProtocol::parseBuildEvent(
        R"json({"schema_version":"takween-build-events-v1","sequence":1.5,"event":"operation_started","operation":"build","phase":"operation","status":"started"})json",
        &event, &error));
    QVERIFY(not TakweenProtocol::parseBuildEvent(
        R"json({"schema_version":"takween-build-events-v1","sequence":1,"event":"phase_finished","operation":"build","phase":"compiler","status":"failed"})json",
        &event, &error));
    QVERIFY(not TakweenProtocol::parseBuildEvent(
        R"json({"schema_version":"takween-build-events-v1","sequence":1,"event":"unknown","operation":"build"})json",
        &event, &error));
}

void TestTakweenProtocol::validatesStreamOrderingAndCompletion()
{
    QString error;
    TakweenBuildEvent started;
    started.sequence = 1;
    started.event = "operation_started";
    started.operation = "build";
    QVERIFY(TakweenProtocol::validateTransition(started, "build", 0, false, &error));

    TakweenBuildEvent phase;
    phase.sequence = 2;
    phase.event = "phase_started";
    phase.operation = "build";
    QVERIFY(TakweenProtocol::validateTransition(phase, "build", 1, false, &error));
    QVERIFY(not TakweenProtocol::validateTransition(phase, "build", 0, false, &error));
    QVERIFY(not TakweenProtocol::validateTransition(phase, "run", 1, false, &error));
    QVERIFY(not TakweenProtocol::validateTransition(phase, "build", 1, true, &error));

    QVERIFY(TakweenProtocol::validateCompletion(0, false, false, true, 0, &error));
    QVERIFY(not TakweenProtocol::validateCompletion(0, false, false, false, 0, &error));
    QVERIFY(not TakweenProtocol::validateCompletion(0, false, false, true, 4, &error));
    QVERIFY(TakweenProtocol::validateCompletion(-2, true, false, false, 0, &error));
}

void TestTakweenProtocol::rendersArabicProgress()
{
    TakweenBuildEvent event;
    event.event = "phase_started";
    event.phase = "compiler";
    QVERIFY(TakweenProtocol::progressText(event).contains("الترجمة"));
}

QTEST_MAIN(TestTakweenProtocol)
#include "TestTakweenProtocol.moc"
