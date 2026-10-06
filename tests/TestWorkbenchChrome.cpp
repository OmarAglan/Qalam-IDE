#include "Qalam.h"
#include "Constants.h"
#include "QalamActivityBar.h"
#include "QalamMenuBar.h"
#include "QalamPanelArea.h"
#include "QalamSettings.h"

#include <QAbstractButton>
#include <QDir>
#include <QFile>
#include <QMenu>
#include <QMenuBar>
#include <QScopeGuard>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>

// Workbench chrome contract: every button that shows no text carries an icon
// and an accessible name, so no control is an unlabeled blank square for
// sighted or screen-reader users. With QALAM_TEST_SCREENSHOTS set, the test
// also saves each main surface for visual review.
class TestWorkbenchChrome : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void iconOnlyButtonsHaveIconsAndNames();
    void primaryMenuActionsHaveIcons();
    void menuBarShowsEveryTopLevelMenu();
    void capturesSurfaces();

private:
    void capture(QWidget *widget, const QString &name);
    QStringList unlabeledButtons(QWidget *root) const;

    QTemporaryDir m_directory;
    QString m_previousPath;
    Qalam *m_window{};
};

void TestWorkbenchChrome::initTestCase()
{
    QVERIFY(m_directory.isValid());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_directory.path());
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, m_directory.path());
    QSettings settings(Constants::OrgName, Constants::AppName);
    // Missing tools keep the test hermetic: nothing external is launched.
    const QString missing = m_directory.filePath(QStringLiteral("missing-tool"));
    settings.setValue(Constants::SettingsKeyCompilerPath, missing);
    settings.setValue(Constants::SettingsKeyLanguageServerPath, missing);
    settings.setValue(Constants::SettingsKeyTakweenPath, missing);
    settings.setValue(Constants::SettingsKeyNazmPath, missing);
    settings.setValue(Constants::SettingsKeyShowWelcome, false);
    settings.sync();

    const QString project = m_directory.filePath(QStringLiteral("مشروعي"));
    QVERIFY(QDir().mkpath(project + QStringLiteral("/المصدر")));
    QFile source(project + QStringLiteral("/المصدر/رئيسي.باء"));
    QVERIFY(source.open(QIODevice::WriteOnly));
    source.write(QStringLiteral(
        "#تضمين \"مكتبة.رأسباء\"\n\n"
        "// نقطة البداية\n"
        "صحيح الرئيسية() {\n"
        "    صحيح مجموع = ٠.\n"
        "    لكل (صحيح ع = ١؛ ع <= ١٠؛ ع++) {\n"
        "        مجموع = مجموع + ع.\n"
        "    }\n"
        "    اطبع(\"المجموع: \" + مجموع).\n"
        "    إرجع ٠.\n"
        "}\n").toUtf8());
    source.close();

    m_window = new Qalam(source.fileName(), nullptr,
                         m_directory.filePath(QStringLiteral("session.ini")));
    m_window->resize(1280, 800);
    m_window->loadFolder(project);
    m_window->show();
    // The frameless window is not reported as exposed on every platform;
    // waiting for visibility and a settled layout is enough for these checks.
    QTRY_VERIFY(m_window->isVisible());
    QTest::qWait(300);
}

void TestWorkbenchChrome::cleanupTestCase()
{
    if (m_window) {
        for (QalamEditor *editor : m_window->findChildren<QalamEditor *>())
            editor->document()->setModified(false);
        delete m_window;
    }
}

QStringList TestWorkbenchChrome::unlabeledButtons(QWidget *root) const
{
    QStringList failures;
    for (QAbstractButton *button : root->findChildren<QAbstractButton *>()) {
        if (not button->isVisibleTo(root) or not button->text().trimmed().isEmpty())
            continue;
        // Qt's internal scroll/tab arrows are not workbench controls.
        if (button->objectName().startsWith(QLatin1String("qt_"))) continue;
        if (QString(button->metaObject()->className()) == QLatin1String("QToolBarExtension"))
            continue;
        const QString id = QStringLiteral("%1 '%2' in %3")
            .arg(button->metaObject()->className(), button->objectName(),
                 button->parentWidget() ? button->parentWidget()->metaObject()->className()
                                        : "none");
        if (button->icon().isNull()) failures << QStringLiteral("no icon: ") + id;
        if (button->accessibleName().isEmpty() and button->toolTip().isEmpty())
            failures << QStringLiteral("no name: ") + id;
    }
    return failures;
}

void TestWorkbenchChrome::iconOnlyButtonsHaveIconsAndNames()
{
    auto *panel = m_window->findChild<QalamPanelArea *>();
    QVERIFY(panel);
    QStringList failures;
    for (const auto tab : {QalamPanelArea::Tab::Problems, QalamPanelArea::Tab::Terminal,
                           QalamPanelArea::Tab::Debug}) {
        panel->show();
        panel->setCollapsed(false);
        panel->setCurrentTab(tab);
        QCoreApplication::processEvents();
        failures << unlabeledButtons(m_window);
    }
    failures.removeDuplicates();
    QVERIFY2(failures.isEmpty(), qPrintable(failures.join(QLatin1Char('\n'))));
}

void TestWorkbenchChrome::primaryMenuActionsHaveIcons()
{
    // Commands a user reaches for repeatedly are recognizable by shape.
    const QStringList required{
        QStringLiteral("جديد"), QStringLiteral("فتح ملف"), QStringLiteral("فتح مجلد"),
        QStringLiteral("حفظ"), QStringLiteral("الإعدادات"), QStringLiteral("بحث في الملف"),
        QStringLiteral("بحث في الملفات"), QStringLiteral("لوحة الأوامر"),
        QStringLiteral("بناء المشروع أو ملف نظم"), QStringLiteral("تشغيل"),
        QStringLiteral("اختبار مشروع تكوين"), QStringLiteral("تنظيف مشروع تكوين"),
        QStringLiteral("تقسيم المحرر إلى اليمين"), QStringLiteral("تقسيم المحرر إلى الأسفل")};
    auto *menuBar = m_window->findChild<QalamMenuBar *>();
    QVERIFY(menuBar);
    QList<QAction *> menuActions;
    QList<QAction *> pending = menuBar->actions();
    while (not pending.isEmpty()) {
        QAction *action = pending.takeFirst();
        if (action->menu()) pending << action->menu()->actions();
        else menuActions << action;
    }
    QStringList missing;
    for (const QString &label : required) {
        bool found = false;
        for (QAction *action : menuActions) {
            QString text = action->text();
            text.remove(QLatin1Char('&'));
            if (text.section(QLatin1Char('\t'), 0, 0).trimmed() != label) continue;
            found = true;
            if (action->icon().isNull()) missing << label;
            break;
        }
        if (not found) missing << label + QStringLiteral(" (no action)");
    }
    QVERIFY2(missing.isEmpty(), qPrintable(missing.join(QStringLiteral(", "))));
}

void TestWorkbenchChrome::menuBarShowsEveryTopLevelMenu()
{
    // A menu that falls into the overflow chevron is invisible to users who
    // do not know to look for it. QMenuBar hides any item whose rectangle,
    // height included, is not fully inside the bar.
    auto *menuBar = m_window->findChild<QalamMenuBar *>();
    QVERIFY(menuBar);
    QVERIFY(menuBar->isVisible());
    QStringList hidden;
    for (QAction *action : menuBar->actions()) {
        const QRect geometry = menuBar->actionGeometry(action);
        if (not geometry.isValid() or not menuBar->rect().contains(geometry))
            hidden << action->text();
    }
    QVERIFY2(hidden.isEmpty(), qPrintable(hidden.join(QStringLiteral(", "))));
    capture(menuBar, QStringLiteral("chrome-menubar"));
    if (QWidget *titleBar = menuBar->parentWidget())
        capture(titleBar, QStringLiteral("chrome-titlebar"));
}

void TestWorkbenchChrome::capture(QWidget *widget, const QString &name)
{
    const QString directory = qEnvironmentVariable("QALAM_TEST_SCREENSHOTS");
    if (directory.isEmpty() or not widget) return;
    QCoreApplication::processEvents();
    QVERIFY(widget->grab().save(directory + QLatin1Char('/') + name + QStringLiteral(".png")));
}

void TestWorkbenchChrome::capturesSurfaces()
{
    if (qEnvironmentVariableIsEmpty("QALAM_TEST_SCREENSHOTS"))
        QSKIP("Set QALAM_TEST_SCREENSHOTS to a directory to save workbench surfaces.");

    auto *panel = m_window->findChild<QalamPanelArea *>();
    QVERIFY(panel);
    panel->show();
    panel->setCollapsed(false);
    panel->addProblem(QStringLiteral("[B2001] رمز غير معرف: مجموعة"),
                      QStringLiteral("رئيسي.باء"), 7, 9, QStringLiteral("error"));
    panel->addProblem(QStringLiteral("[B3002] متغير غير مستخدم"),
                      QStringLiteral("رئيسي.باء"), 5, 10, QStringLiteral("warning"));
    panel->setCurrentTab(QalamPanelArea::Tab::Problems);
    capture(m_window, QStringLiteral("chrome-problems"));
    panel->setCurrentTab(QalamPanelArea::Tab::Terminal);
    capture(m_window, QStringLiteral("chrome-terminal"));
    panel->setCurrentTab(QalamPanelArea::Tab::Debug);
    capture(m_window, QStringLiteral("chrome-debug"));

    if (auto *activity = m_window->findChild<QalamActivityBar *>()) {
        for (const auto view : {QalamActivityBar::ViewType::Search,
                                QalamActivityBar::ViewType::Run}) {
            activity->setCurrentView(view);
            capture(m_window, view == QalamActivityBar::ViewType::Search
                                  ? QStringLiteral("chrome-search")
                                  : QStringLiteral("chrome-run"));
        }
        activity->setCurrentView(QalamActivityBar::ViewType::Explorer);
    }

    for (QMenu *menu : m_window->findChildren<QMenu *>()) {
        if (menu->title().isEmpty() or menu->actions().isEmpty()) continue;
        menu->adjustSize();
        capture(menu, QStringLiteral("menu-") + QString::number(qHash(menu->title())));
    }

    if (auto *settings = m_window->findChild<QalamSettings *>()) {
        settings->show();
        capture(settings, QStringLiteral("chrome-settings"));
        settings->hide();
    }
}

QTEST_MAIN(TestWorkbenchChrome)
#include "TestWorkbenchChrome.moc"
