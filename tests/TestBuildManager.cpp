#include "BuildManager.h"

#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

class TestBuildManager : public QObject
{
    Q_OBJECT

private slots:
    void buildsStableBaaCheckArguments();
    void buildsValidatedTakweenArguments();
    void classifiesCompilerCliExitCodes();
    void buildsOperationAwareExitDiagnostics();
    void findsNearestTakweenProjectRoot();
    void returnsEmptyRootOutsideTakweenProject();
};

void TestBuildManager::buildsStableBaaCheckArguments()
{
    const QString path = QDir::cleanPath(QDir::temp().filePath("مشروع/الرئيسية.baa"));
    const QStringList arguments = BuildManager::baaCheckArguments(path);

    QCOMPARE(arguments.size(), 3);
    QCOMPARE(arguments[0], QString("--check"));
    QCOMPARE(arguments[1], QString("--diagnostics=json"));
    QCOMPARE(arguments[2], QFileInfo(path).absoluteFilePath());
}

void TestBuildManager::buildsValidatedTakweenArguments()
{
    QCOMPARE(BuildManager::takweenCommandArguments("build"), QStringList{"build"});
    QCOMPARE(BuildManager::takweenCommandArguments(" RUN "), QStringList{"run"});
    QCOMPARE(BuildManager::takweenCommandArguments("test"), QStringList{"test"});
    QCOMPARE(BuildManager::takweenCommandArguments("clean"), QStringList{"clean"});
    QVERIFY(BuildManager::takweenCommandArguments("publish").isEmpty());
    QVERIFY(BuildManager::takweenCommandArguments("build & whoami").isEmpty());
}

void TestBuildManager::classifiesCompilerCliExitCodes()
{
    using ExitClass = BuildManager::CompilerExitClass;
    QCOMPARE(BuildManager::classifyCompilerExitCode(0), ExitClass::Success);
    QCOMPARE(BuildManager::classifyCompilerExitCode(1), ExitClass::SourceError);
    QCOMPARE(BuildManager::classifyCompilerExitCode(2), ExitClass::InvalidInvocation);
    QCOMPARE(BuildManager::classifyCompilerExitCode(3), ExitClass::Unsupported);
    QCOMPARE(BuildManager::classifyCompilerExitCode(4), ExitClass::ToolchainError);
    QCOMPARE(BuildManager::classifyCompilerExitCode(5), ExitClass::InternalError);
    QCOMPARE(BuildManager::classifyCompilerExitCode(-1), ExitClass::ProcessFailure);
    QCOMPARE(BuildManager::classifyCompilerExitCode(42), ExitClass::Unknown);
}

void TestBuildManager::buildsOperationAwareExitDiagnostics()
{
    QCOMPARE(BuildManager::compilerExitCodeId(4), QString("CLI_EXIT_4"));
    QCOMPARE(BuildManager::compilerExitCodeId(-1), QString("PROCESS_FAILURE"));
    QVERIFY(BuildManager::compilerExitSummary(1, "check").contains("1"));
    QVERIFY(BuildManager::compilerExitSummary(4, "build").contains("4"));
    QVERIFY(BuildManager::compilerExitSummary(5, "build").contains("5"));

    const QString runSummary = BuildManager::compilerExitSummary(1, "run");
    QVERIFY(runSummary.contains("run"));
    QVERIFY(runSummary.contains("1"));
}

void TestBuildManager::findsNearestTakweenProjectRoot()
{
    QTemporaryDir temp;
    QVERIFY(temp.isValid());
    QDir root(temp.path());
    QVERIFY(root.mkpath("source/nested"));

    QFile manifest(root.filePath("مشروع.تكوين"));
    QVERIFY(manifest.open(QIODevice::WriteOnly));
    manifest.write("name: test\n");
    manifest.close();

    const QString source = root.filePath("source/nested/main.baa");
    QCOMPARE(BuildManager::findTakweenProjectRoot(source), QDir::cleanPath(temp.path()));
}

void TestBuildManager::returnsEmptyRootOutsideTakweenProject()
{
    QTemporaryDir temp;
    QVERIFY(temp.isValid());
    QVERIFY(BuildManager::findTakweenProjectRoot(QDir(temp.path()).filePath("main.baa")).isEmpty());
}

QTEST_MAIN(TestBuildManager)
#include "TestBuildManager.moc"
