#include "BuildManager.h"
#include "Constants.h"
#include "DiagnosticParser.h"
#include "QalamConsole.h"
#include "QalamMenuBar.h"
#include "QalamTitleBar.h"

#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>

class TestBuildManager : public QObject
{
    Q_OBJECT

private slots:
    void buildsValidatedTakweenArguments();
    void filtersTakweenTargetsByCapability();
    void passesProfilesAndRejectsOptionLikeNames();
    void remembersTakweenSelectionPerProject();
    void diagnosesTakweenCompileFailureFromCheckJson();
    void classifiesCompilerCliExitCodes();
    void buildsOperationAwareExitDiagnostics();
    void findsNearestTakweenProjectRoot();
    void returnsEmptyRootOutsideTakweenProject();
    void recognizesNazmSourcesAndObjects();
    void buildsCanonicalNazmArguments();
    void explainsUnavailableToolActions();
    void keepsMenuBarRightToLeft();
    void keepsFullMenuVisibleInArabicTitleBar();
};

void TestBuildManager::keepsMenuBarRightToLeft()
{
    QalamMenuBar menu;
    QCOMPARE(menu.layoutDirection(), Qt::RightToLeft);
    for (QAction *action : menu.actions()) {
        QVERIFY(action->menu());
        QCOMPARE(action->menu()->layoutDirection(), Qt::RightToLeft);
    }
}

void TestBuildManager::keepsFullMenuVisibleInArabicTitleBar()
{
    QWidget owner;
    QalamTitleBar titleBar;
    QalamMenuBar menu(&owner);
    menu.setStyleSheet(QStringLiteral(
        "QMenuBar::item { padding: 4px 10px; }"));
    titleBar.resize(1400, titleBar.height());
    titleBar.addMenuBar(&menu);
    titleBar.show();
    QTest::qWait(20);

    QCOMPARE(menu.layoutDirection(), Qt::RightToLeft);
    QVERIFY(menu.width() >= menu.sizeHint().width());
    auto *commandCenter = titleBar.findChild<QPushButton *>(
        QStringLiteral("commandCenterButton"));
    QVERIFY(commandCenter);
    const int commandCenterX = commandCenter->geometry().center().x();
    const int titleCenterX = titleBar.rect().center().x();
    QVERIFY2(qAbs(commandCenterX - titleCenterX) <= 1,
             qPrintable(QStringLiteral(
                 "مركز الأوامر %1 لا يطابق مركز النافذة %2؛ هندسة المركز: %3")
                 .arg(commandCenterX)
                 .arg(titleCenterX)
                 .arg(commandCenter->geometry().x())));
    for (QAction *action : menu.actions()) {
        const QRect actionRect = menu.actionGeometry(action);
        QVERIFY2(actionRect.isValid(), qPrintable(action->text()));
        QVERIFY2(actionRect.width()
                     >= menu.fontMetrics().horizontalAdvance(action->text()),
                 qPrintable(action->text()));
        QVERIFY2(menu.rect().contains(actionRect.center()),
                 qPrintable(action->text()));
    }
}

void TestBuildManager::buildsValidatedTakweenArguments()
{
    QCOMPARE(BuildManager::takweenCommandArguments("build"), QStringList{"بناء"});
    QCOMPARE(BuildManager::takweenCommandArguments(" RUN "), QStringList{"تشغيل"});
    QCOMPARE(BuildManager::takweenCommandArguments("test", "اختبار_أ"),
             (QStringList{"اختبار", "اختبار_أ"}));
    QCOMPARE(BuildManager::takweenCommandArguments("clean"), QStringList{"تنظيف"});
    QVERIFY(BuildManager::takweenCommandArguments("clean", "تطبيق").isEmpty());
    QVERIFY(BuildManager::takweenCommandArguments("publish").isEmpty());
    QVERIFY(BuildManager::takweenCommandArguments("build & whoami").isEmpty());
}

void TestBuildManager::passesProfilesAndRejectsOptionLikeNames()
{
    QCOMPARE(BuildManager::takweenCommandArguments("build", "تطبيق", "إصدار"),
             (QStringList{"بناء", "تطبيق", "--نمط", "إصدار"}));
    QCOMPARE(BuildManager::takweenCommandArguments("run", QString(), "تطوير"),
             (QStringList{"تشغيل", "--نمط", "تطوير"}));
    QCOMPARE(BuildManager::takweenCommandArguments("check", "تطبيق"),
             (QStringList{"فحص", "تطبيق"}));
    // Takween's clean takes neither a target nor a profile.
    QVERIFY(BuildManager::takweenCommandArguments("clean", QString(), "إصدار").isEmpty());
    QVERIFY(BuildManager::takweenCommandArguments("build", "--مقفل").isEmpty());
    QVERIFY(BuildManager::takweenCommandArguments("build", "تطبيق", "-O3").isEmpty());
    QCOMPARE(BuildManager::builtInTakweenProfiles(), (QStringList{"تطوير", "إصدار"}));
}

void TestBuildManager::remembersTakweenSelectionPerProject()
{
    QTemporaryDir first;
    QTemporaryDir second;
    QVERIFY(first.isValid() and second.isValid());
    QVERIFY(BuildManager::takweenSelection(first.path()).target.isEmpty());

    BuildManager::setTakweenSelection(first.path(), {"تطبيق", "إصدار"});
    BuildManager::setTakweenSelection(second.path(), {"أداة", QString()});
    BuildManager::TakweenSelection selection = BuildManager::takweenSelection(first.path());
    QCOMPARE(selection.target, QStringLiteral("تطبيق"));
    QCOMPARE(selection.profile, QStringLiteral("إصدار"));
    selection = BuildManager::takweenSelection(second.path());
    QCOMPARE(selection.target, QStringLiteral("أداة"));
    QVERIFY(selection.profile.isEmpty());

    // The same project reached through a non-canonical path shares its choice.
    const QString dotted = QDir(first.path()).filePath(QStringLiteral("مجلد/.."));
    QCOMPARE(BuildManager::takweenSelectionKey(dotted),
             BuildManager::takweenSelectionKey(first.path()));
#if defined(Q_OS_WIN)
    QCOMPARE(BuildManager::takweenSelectionKey(first.path().toUpper()),
             BuildManager::takweenSelectionKey(first.path()));
#endif

    // Clearing both values forgets the project entirely.
    BuildManager::setTakweenSelection(first.path(), {});
    QSettings settings = Constants::settings();
    QVERIFY(not settings.childGroups().contains(QStringLiteral("takweenProjects")) or
            not settings.value(BuildManager::takweenSelectionKey(first.path())
                               + QStringLiteral("/root")).isValid());
    QVERIFY(BuildManager::takweenSelection(first.path()).target.isEmpty());
    BuildManager::setTakweenSelection(second.path(), {});
}

void TestBuildManager::diagnosesTakweenCompileFailureFromCheckJson()
{
    if (BuildManager::resolveTakweenProgram().isEmpty() or
        BuildManager::resolveCompilerProgram().isEmpty()) {
        QSKIP("Takween and Baa are not installed.");
    }
    QTemporaryDir project;
    QVERIFY(project.isValid());
    const QString root = QDir::cleanPath(project.path());
    QVERIFY(QDir(root).mkpath(QStringLiteral("المصدر")));
    auto write = [&root](const QString &relative, const QByteArray &content) {
        QFile file(QDir(root).filePath(relative));
        return file.open(QIODevice::WriteOnly | QIODevice::Truncate) and
               file.write(content) == content.size();
    };
    QVERIFY(write(QStringLiteral("مشروع.تكوين"),
                  "[المشروع]\nالاسم = \"تجربة\"\nالإصدار = \"1.0.0\"\n\n"
                  "[الأهداف.تطبيق]\nالنوع = \"تنفيذي\"\nالمدخل = \"المصدر/الرئيسية.baa\"\n"
                  "يعتمد_على = [\"حساب\"]\n\n"
                  "[الأهداف.حساب]\nالنوع = \"مكتبة\"\nالمدخل = \"المصدر/حساب.baa\"\n\n"
                  "[البناء]\nالمخرج = \"بناء\"\n\n[الأنماط.سريع]\nالتحسين = ٢\n"));
    QVERIFY(write(QStringLiteral("المصدر/حساب.baa"),
                  "صحيح جمع(صحيح أ، صحيح ب) {\n    إرجع أ + ب.\n}\n"));
    const QString source = QDir(root).filePath(QStringLiteral("المصدر/الرئيسية.baa"));
    QVERIFY(write(QStringLiteral("المصدر/الرئيسية.baa"),
                  "صحيح الرئيسية() {\n    صحيح س = غير_معرف.\n    إرجع ٠.\n}\n"));

    BuildManager manager;
    TakweenBuildPlan plan;
    QString error;
    QVERIFY2(manager.loadTakweenBuildPlan(source, {"تطبيق", "سريع"}, &plan, &error),
             qPrintable(error));
    QCOMPARE(plan.profileName, QStringLiteral("سريع"));
    QCOMPARE(plan.optimization, 2);
    QCOMPARE(plan.targetOrder, (QStringList{"حساب", "تطبيق"}));
    QVERIFY(not manager.loadTakweenBuildPlan(source, {"تطبيق", "مجهول"}, &plan, &error));
    QVERIFY(not error.isEmpty());

    QalamConsole console;
    QSignalSpy diagnostics(&manager, &BuildManager::takweenDiagnosticsReady);
    QSignalSpy finished(&manager, &BuildManager::toolingFinished);
    QSignalSpy scraped(&manager, &BuildManager::outputChunk);
    QVERIFY(manager.runTakweenCommand(source, "build", &console, "تطبيق", "سريع"));
    QTRY_COMPARE_WITH_TIMEOUT(finished.count(), 1, 60000);
    QCOMPARE(finished.first().at(1).toInt(), 1);
    QVERIFY(not manager.isRunning());
    // Human output stays in the terminal; problems come only from `فحص` JSON.
    QCOMPARE(scraped.count(), 0);
    QCOMPARE(diagnostics.count(), 1);
    QCOMPARE(diagnostics.first().at(0).toString(), root);
    const QVector<Diagnostic> parsed = DiagnosticParser::parseCompilerOutput(
        QString::fromUtf8(diagnostics.first().at(1).toByteArray()), QString(), root);
    QCOMPARE(parsed.size(), 1);
    QCOMPARE(parsed.first().code, QStringLiteral("B1000"));
    QCOMPARE(parsed.first().file, source);
    QCOMPARE(parsed.first().line, 2);
}

void TestBuildManager::filtersTakweenTargetsByCapability()
{
    const QVector<TakweenTarget> targets = {
        {"تطبيق", "executable", "ready", true, true, false},
        {"اختبار_أ", "test", "ready", true, true, true},
        {"مكتبة", "library", "unsupported", false, false, false}
    };
    QCOMPARE(BuildManager::selectableTakweenTargets(targets, "build").size(), 2);
    QCOMPARE(BuildManager::selectableTakweenTargets(targets, "run").size(), 2);
    const auto tests = BuildManager::selectableTakweenTargets(targets, "test");
    QCOMPARE(tests.size(), 1);
    QCOMPARE(tests.first().name, QString("اختبار_أ"));
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
    QCOMPARE(BuildManager::classifyCompilerExitCode(-2), ExitClass::Cancelled);
    QCOMPARE(BuildManager::classifyCompilerExitCode(-1), ExitClass::ProcessFailure);
    QCOMPARE(BuildManager::classifyCompilerExitCode(42), ExitClass::Unknown);
}

void TestBuildManager::buildsOperationAwareExitDiagnostics()
{
    QCOMPARE(BuildManager::compilerExitCodeId(4), QString("CLI_EXIT_4"));
    QCOMPARE(BuildManager::compilerExitCodeId(-1), QString("PROCESS_FAILURE"));
    QCOMPARE(BuildManager::compilerExitCodeId(-2), QString("TOOLING_CANCELLED"));
    QVERIFY(BuildManager::compilerExitSummary(1, "check").contains("1"));
    QVERIFY(BuildManager::compilerExitSummary(4, "build").contains("4"));
    QVERIFY(BuildManager::compilerExitSummary(5, "build").contains("5"));

    const QString runSummary = BuildManager::compilerExitSummary(1, "run");
    QVERIFY(runSummary.contains("run"));
    QVERIFY(runSummary.contains("1"));
    QVERIFY(BuildManager::compilerExitSummary(-2, "run").contains("أُلغيت"));
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

void TestBuildManager::recognizesNazmSourcesAndObjects()
{
    QVERIFY(BuildManager::isNazmSourcePath(QStringLiteral("مصدر/بداية.نظم")));
    QVERIFY(not BuildManager::isNazmSourcePath(QStringLiteral("مصدر/بداية.baa")));
#if defined(Q_OS_WIN)
    QVERIFY(BuildManager::nazmObjectPath(QStringLiteral("C:/مشروع/بداية.نظم"))
                .endsWith(QStringLiteral("بداية.obj")));
#else
    QVERIFY(BuildManager::nazmObjectPath(QStringLiteral("/tmp/مشروع/بداية.نظم"))
                .endsWith(QStringLiteral("بداية.o")));
#endif
}

void TestBuildManager::buildsCanonicalNazmArguments()
{
    const QString source = QStringLiteral("C:/مشروع/بداية.نظم");
    const QString object = QStringLiteral("C:/مشروع/بداية.obj");
#if defined(Q_OS_WIN)
    const QString format = QStringLiteral("كوف");
#else
    const QString format = QStringLiteral("إلف64");
#endif
    QCOMPARE(BuildManager::nazmCommandArguments(source, object),
             (QStringList{source, QStringLiteral("--خرج"), object,
                          QStringLiteral("--صيغة"), format}));
}

void TestBuildManager::explainsUnavailableToolActions()
{
    QTemporaryDir temp;
    QVERIFY(temp.isValid());
    const QString standalone = QDir(temp.path()).filePath(QStringLiteral("بدء.نظم"));

    auto state = BuildManager::toolActionState(
        standalone, QStringLiteral("build"), true, true, false);
    QVERIFY(not state.enabled);
    QCOMPARE(state.explanation, QStringLiteral(
        "الأدوات المطلوبة غير متاحة: نظم. ثبّتها في PATH أو اضبط مساراتها من الإعدادات."));

    state = BuildManager::toolActionState(
        standalone, QStringLiteral("build"), false, false, true);
    QVERIFY(state.enabled);

    state = BuildManager::toolActionState(
        standalone, QStringLiteral("run"), false, true, true);
    QVERIFY(not state.enabled);
    QVERIFY(state.explanation.contains(QStringLiteral("باء")));

    const QString canonicalBaa = QDir(temp.path()).filePath(
        QStringLiteral("رئيسية.باء"));
    state = BuildManager::toolActionState(
        canonicalBaa, QStringLiteral("run"), true, true, true);
    QVERIFY(state.enabled);

    QDir root(temp.path());
    QVERIFY(root.mkpath(QStringLiteral("مشروع/مصدر")));
    QFile manifest(root.filePath(QStringLiteral("مشروع/مشروع.تكوين")));
    QVERIFY(manifest.open(QIODevice::WriteOnly));
    manifest.close();
    const QString projectSource = root.filePath(QStringLiteral("مشروع/مصدر/رئيسية.baa"));
    state = BuildManager::toolActionState(
        projectSource, QStringLiteral("build"), true, false, true);
    QVERIFY(not state.enabled);
    QVERIFY(state.explanation.contains(QStringLiteral("تكوين")));
}

QTEST_MAIN(TestBuildManager)
#include "TestBuildManager.moc"
