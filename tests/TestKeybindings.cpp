#include "CommandRegistry.h"
#include "QalamKeybindings.h"
#include "QalamSettings.h"
#include "Constants.h"

#include <QAction>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QShortcut>
#include <QSettings>
#include <QSpinBox>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTest>
#include <QWidget>

class TestKeybindings : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void defaultsHaveNoConflicts();
    void conflictsGroupCommandsByKey();
    void overrideRebindsActionShortcutAndRegistry();
    void emptyOverrideUnbinds();
    void removingOverrideRestoresDefault();
    void semanticSelectionIsNotRebindable();
    void settingsTableSavesOverridesAndReportsConflicts();
    void settingsAnalysisDelayIsPersisted();
    void settingsPageLeavesAbsentToolPathsAbsent();

private:
    static int rowFor(QTableWidget *table, const QString &id);

    QTemporaryDir m_directory;
};

void TestKeybindings::initTestCase()
{
    QVERIFY(m_directory.isValid());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_directory.path());
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, m_directory.path());
    // init() clears settings; that must never reach the user's real ones.
    QVERIFY(Constants::settings().fileName().startsWith(m_directory.path()));
}

void TestKeybindings::init()
{
    QSettings settings = Constants::settings();
    settings.clear();
    // Keep the tools page hermetic: no discovery hits a real toolchain.
    const QString missing = m_directory.filePath(QStringLiteral("missing-tool"));
    settings.setValue(Constants::SettingsKeyCompilerPath, missing);
    settings.setValue(Constants::SettingsKeyTakweenPath, missing);
    settings.setValue(Constants::SettingsKeyNazmPath, missing);
    settings.sync();
}

int TestKeybindings::rowFor(QTableWidget *table, const QString &id)
{
    for (int row = 0; row < table->rowCount(); ++row) {
        if (table->item(row, 0)->data(Qt::UserRole).toString() == id) return row;
    }
    return -1;
}

void TestKeybindings::defaultsHaveNoConflicts()
{
    QHash<QString, QKeySequence> bindings;
    for (const CommandRegistry::Command &command : CommandRegistry::defaultCommands())
        bindings.insert(command.id, QKeySequence::fromString(command.shortcut,
                                                             QKeySequence::PortableText));
    const auto conflicts = QalamKeybindings::conflicts(bindings);
    QStringList lines;
    for (auto it = conflicts.cbegin(); it != conflicts.cend(); ++it)
        lines << it.key() + QStringLiteral(": ") + it.value().join(QStringLiteral(", "));
    QVERIFY2(conflicts.isEmpty(), qPrintable(lines.join(QLatin1Char('\n'))));
}

void TestKeybindings::conflictsGroupCommandsByKey()
{
    const QHash<QString, QKeySequence> bindings{
        {QStringLiteral("b"), QKeySequence(QStringLiteral("Ctrl+K"))},
        {QStringLiteral("a"), QKeySequence(QStringLiteral("Ctrl+K"))},
        {QStringLiteral("c"), QKeySequence(QStringLiteral("Ctrl+L"))},
        {QStringLiteral("d"), QKeySequence()},
        {QStringLiteral("e"), QKeySequence()}};
    const auto conflicts = QalamKeybindings::conflicts(bindings);
    QCOMPARE(conflicts.size(), 1);
    QCOMPARE(conflicts.value(QStringLiteral("Ctrl+K")),
             (QStringList{QStringLiteral("a"), QStringLiteral("b")}));
}

void TestKeybindings::overrideRebindsActionShortcutAndRegistry()
{
    CommandRegistry registry;
    for (const auto &command : CommandRegistry::defaultCommands()) registry.registerCommand(command);
    QWidget host;
    QAction save;
    QShortcut comment(&host);
    QalamKeybindings bindings(&registry);
    bindings.bind(QStringLiteral("file.save"), &save);
    bindings.bind(QStringLiteral("editor.comment"), &comment);

    QSettings settings = Constants::settings();
    bindings.apply(settings);
    QCOMPARE(save.shortcut(), QKeySequence(QStringLiteral("Ctrl+S")));
    QCOMPARE(comment.key(), QKeySequence(QStringLiteral("Ctrl+/")));

    settings.setValue(QalamKeybindings::settingsKey(QStringLiteral("file.save")),
                      QStringLiteral("Ctrl+Alt+S"));
    bindings.apply(settings);
    QCOMPARE(save.shortcut(), QKeySequence(QStringLiteral("Ctrl+Alt+S")));
    // The palette and menus read the registry, so it must agree with the key.
    QCOMPARE(registry.command(QStringLiteral("file.save")).shortcut, QStringLiteral("Ctrl+Alt+S"));
}

void TestKeybindings::emptyOverrideUnbinds()
{
    CommandRegistry registry;
    for (const auto &command : CommandRegistry::defaultCommands()) registry.registerCommand(command);
    QAction save;
    QalamKeybindings bindings(&registry);
    bindings.bind(QStringLiteral("file.save"), &save);

    QSettings settings = Constants::settings();
    settings.setValue(QalamKeybindings::settingsKey(QStringLiteral("file.save")), QString());
    bindings.apply(settings);
    QVERIFY(save.shortcut().isEmpty());
    QVERIFY(registry.command(QStringLiteral("file.save")).shortcut.isEmpty());
}

void TestKeybindings::removingOverrideRestoresDefault()
{
    CommandRegistry registry;
    for (const auto &command : CommandRegistry::defaultCommands()) registry.registerCommand(command);
    QAction save;
    QalamKeybindings bindings(&registry);
    bindings.bind(QStringLiteral("file.save"), &save);

    QSettings settings = Constants::settings();
    const QString key = QalamKeybindings::settingsKey(QStringLiteral("file.save"));
    settings.setValue(key, QStringLiteral("F9"));
    bindings.apply(settings);
    QCOMPARE(save.shortcut(), QKeySequence(QStringLiteral("F9")));

    settings.remove(key);
    bindings.apply(settings);
    QCOMPARE(save.shortcut(), QKeySequence(QStringLiteral("Ctrl+S")));
    QCOMPARE(registry.command(QStringLiteral("file.save")).shortcut, QStringLiteral("Ctrl+S"));
}

void TestKeybindings::semanticSelectionIsNotRebindable()
{
    QVERIFY(not QalamKeybindings::isRebindable(QStringLiteral("code.expandSelection")));
    QVERIFY(not QalamKeybindings::isRebindable(QStringLiteral("code.shrinkSelection")));
    QVERIFY(QalamKeybindings::isRebindable(QStringLiteral("file.save")));
}

void TestKeybindings::settingsTableSavesOverridesAndReportsConflicts()
{
    QalamSettings page;
    auto *table = page.findChild<QTableWidget *>(QStringLiteral("settingsShortcutTable"));
    auto *conflictLabel = page.findChild<QLabel *>(QStringLiteral("settingsShortcutConflicts"));
    QVERIFY(table);
    QVERIFY(conflictLabel);
    QVERIFY(rowFor(table, QStringLiteral("code.expandSelection")) < 0);
    QVERIFY(conflictLabel->text().isEmpty());

    const int saveRow = rowFor(table, QStringLiteral("file.save"));
    QVERIFY(saveRow >= 0);
    auto *edit = qobject_cast<QKeySequenceEdit *>(table->cellWidget(saveRow, 1));
    QVERIFY(edit);
    QCOMPARE(edit->keySequence(), QKeySequence(QStringLiteral("Ctrl+S")));

    int changes = 0;
    connect(&page, &QalamSettings::shortcutsChanged, this, [&changes]() { ++changes; });

    // Ctrl+O already opens a file: the page must say so, not silently accept.
    edit->setKeySequence(QKeySequence(QStringLiteral("Ctrl+O")));
    QCOMPARE(changes, 1);
    QSettings settings = Constants::settings();
    QCOMPARE(settings.value(QalamKeybindings::settingsKey(QStringLiteral("file.save"))).toString(),
             QStringLiteral("Ctrl+O"));
    QVERIFY(not conflictLabel->text().isEmpty());
    QVERIFY(conflictLabel->text().contains(QStringLiteral("فتح ملف")));

    // Returning to the default drops the stored override entirely.
    edit->setKeySequence(QKeySequence(QStringLiteral("Ctrl+S")));
    QCOMPARE(changes, 2);
    settings.sync();
    QVERIFY(not settings.contains(QalamKeybindings::settingsKey(QStringLiteral("file.save"))));
    QVERIFY(conflictLabel->text().isEmpty());
}

void TestKeybindings::settingsAnalysisDelayIsPersisted()
{
    QalamSettings page;
    auto *spin = page.findChild<QSpinBox *>(QStringLiteral("settingsAnalysisDelay"));
    QVERIFY(spin);
    QCOMPARE(spin->value(), Constants::Timing::AnalysisDelay);

    int emitted = -1;
    connect(&page, &QalamSettings::analysisDelayChanged, this,
            [&emitted](int milliseconds) { emitted = milliseconds; });
    spin->setValue(400);
    QCOMPARE(emitted, 400);
    QSettings settings = Constants::settings();
    QCOMPARE(settings.value(Constants::SettingsKeyAnalysisDelay).toInt(), 400);
}

void TestKeybindings::settingsPageLeavesAbsentToolPathsAbsent()
{
    QSettings settings = Constants::settings();
    settings.clear();
    settings.sync();
    {
        // Building the page runs tool discovery, which used to "restore"
        // missing keys as empty strings on every launch.
        QalamSettings page;
        page.close();
    }
    settings.sync();
    for (const QString &key : {Constants::SettingsKeyCompilerPath,
                               Constants::SettingsKeyTakweenPath,
                               Constants::SettingsKeyNazmPath,
                               Constants::SettingsKeyLanguageServerPath}) {
        QVERIFY2(not settings.contains(key), qPrintable(key));
    }
}

QTEST_MAIN(TestKeybindings)
#include "TestKeybindings.moc"
