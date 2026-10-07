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
    void parsesBuildPlanContract();
    void rejectsInvalidBuildPlan();
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

namespace {
QJsonObject validPlan()
{
    return QJsonObject{
        {"schema_version", "takween-build-plan-v1"},
        {"operation", "build"},
        {"project", "تجربة"},
        {"target", "تطبيق"},
        {"profile", QJsonObject{{"name", "سريع"}, {"optimization", 2}, {"verify", false}}},
        {"target_order", QJsonArray{"حساب", "تطبيق"}},
        {"working_directory", "."},
        {"source_files", QJsonArray{"././المصدر/الرئيسية.baa", "././المصدر/حساب.baa"}},
        {"include_paths", QJsonArray{}},
        {"argv", QJsonArray{"baa", "././المصدر/الرئيسية.baa", "-O2", "-o", "بناء/تطبيق.exe"}},
    };
}

QByteArray planBytes(const QJsonObject &plan)
{
    return QJsonDocument(plan).toJson(QJsonDocument::Compact);
}
}

void TestTakweenProtocol::parsesBuildPlanContract()
{
    TakweenBuildPlan plan;
    QString error;
    QVERIFY2(TakweenProtocol::parseBuildPlan(planBytes(validPlan()), &plan, &error),
             qPrintable(error));
    QCOMPARE(plan.project, QStringLiteral("تجربة"));
    QCOMPARE(plan.target, QStringLiteral("تطبيق"));
    QCOMPARE(plan.profileName, QStringLiteral("سريع"));
    QCOMPARE(plan.optimization, 2);
    QVERIFY(not plan.verify);
    QCOMPARE(plan.targetOrder, (QStringList{"حساب", "تطبيق"}));
    QCOMPARE(plan.sourceFiles.size(), 2);
    QVERIFY(plan.includePaths.isEmpty());
    QCOMPARE(plan.argv.last(), QStringLiteral("بناء/تطبيق.exe"));

    // Manifest v0.1 projects report an unnamed profile.
    QJsonObject legacy = validPlan();
    legacy["profile"] = QJsonObject{{"name", ""}, {"optimization", 1}, {"verify", false}};
    QVERIFY(TakweenProtocol::parseBuildPlan(planBytes(legacy), &plan, &error));
    QVERIFY(plan.profileName.isEmpty());
}

void TestTakweenProtocol::rejectsInvalidBuildPlan()
{
    TakweenBuildPlan plan;
    QString error;
    auto rejects = [&](QJsonObject document) {
        error.clear();
        const bool parsed = TakweenProtocol::parseBuildPlan(planBytes(document), &plan, &error);
        return not parsed and not error.isEmpty() and plan.target.isEmpty();
    };

    QJsonObject wrongSchema = validPlan();
    wrongSchema["schema_version"] = "takween-build-plan-v2";
    QVERIFY(rejects(wrongSchema));

    QJsonObject badOptimization = validPlan();
    badOptimization["profile"] = QJsonObject{{"name", "س"}, {"optimization", 3}, {"verify", true}};
    QVERIFY(rejects(badOptimization));

    QJsonObject orderWithoutTarget = validPlan();
    orderWithoutTarget["target_order"] = QJsonArray{"تطبيق", "حساب"};
    QVERIFY(rejects(orderWithoutTarget));

    QJsonObject duplicateOrder = validPlan();
    duplicateOrder["target_order"] = QJsonArray{"تطبيق", "تطبيق"};
    QVERIFY(rejects(duplicateOrder));

    QJsonObject noSources = validPlan();
    noSources["source_files"] = QJsonArray{};
    QVERIFY(rejects(noSources));

    QJsonObject numericArgv = validPlan();
    numericArgv["argv"] = QJsonArray{"baa", 2};
    QVERIFY(rejects(numericArgv));

    QVERIFY(not TakweenProtocol::parseBuildPlan("not json", &plan, &error));
}

QTEST_MAIN(TestTakweenProtocol)
#include "TestTakweenProtocol.moc"
