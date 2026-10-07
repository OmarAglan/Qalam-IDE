#include "QalamSettings.h"
#include "../../qalam/Constants.h"
#include "ToolchainDiscovery.h"
#include "texteditor/autocomplete/QalamCompletionHistory.h"
#include "core/CommandRegistry.h"
#include "core/QalamKeybindings.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHeaderView>
#include <QKeySequenceEdit>
#include <QScrollArea>
#include <QStandardPaths>
#include <QSet>
#include <QToolButton>

namespace {

// A titled card. QGroupBox measures its title without Arabic shaping and
// clips the last glyph in RTL, so the title is a separate label that
// addSection() places above the card.
QFrame *createGroup(const QString &title)
{
    auto *card = new QFrame;
    card->setObjectName(QStringLiteral("settingsCard"));
    card->setProperty("sectionTitle", title);
    card->setAccessibleName(title);
    card->setStyleSheet(QStringLiteral(
        "QFrame#settingsCard { border: 1px solid %1; border-radius: 6px; }")
        .arg(Constants::Colors::Border));
    return card;
}

void addSection(QVBoxLayout *layout, QFrame *card)
{
    auto *title = new QLabel(card->property("sectionTitle").toString());
    title->setStyleSheet(QStringLiteral("font-weight: 600; color: %1; margin-top: 6px;")
                             .arg(Constants::Colors::TextSecondary));
    title->setBuddy(card);
    layout->addWidget(title);
    layout->addWidget(card);
}

QLabel *createHint(const QString &text)
{
    auto *hint = new QLabel(text);
    hint->setWordWrap(true);
    hint->setStyleSheet(QStringLiteral("color: %1;").arg(Constants::Colors::TextMuted));
    return hint;
}

} // namespace

QalamSettings::QalamSettings(QWidget* parent) : QWidget(parent) {
    setWindowTitle("الإعدادات");
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
    setMinimumSize(860, 620);
    setStyleSheet(QStringLiteral("color: %1; background-color: %2;")
                      .arg(Constants::Colors::TextPrimary,
                           Constants::Colors::WindowBackground));

    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    categoryList = new QListWidget;
    categoryList->setObjectName(QStringLiteral("settingsCategories"));
    categoryList->setAccessibleName(QStringLiteral("أقسام الإعدادات"));
    categoryList->setIconSize(QSize(18, 18));
    categoryList->setFixedWidth(220);
    categoryList->setFocusPolicy(Qt::StrongFocus);
    categoryList->setStyleSheet(QStringLiteral(
        "QListWidget { background: %1; border: none; border-left: 1px solid %2;"
        " outline: 0; padding-top: 10px; }"
        " QListWidget::item { padding: 9px 14px; margin: 1px 6px; border-radius: 4px;"
        " color: %3; }"
        " QListWidget::item:hover { background: %4; }"
        " QListWidget::item:selected { background: %5; color: %6; }")
        .arg(Constants::Colors::SidebarBackground, Constants::Colors::BorderSubtle,
             Constants::Colors::TextSecondary, Constants::Colors::TabHoverBackground,
             Constants::Colors::Selection, Constants::Colors::TextPrimary));

    stackedWidget = new QStackedWidget;
    connect(categoryList, &QListWidget::currentRowChanged,
            stackedWidget, &QStackedWidget::setCurrentIndex);

    createCategory(QStringLiteral("المحرر"), QStringLiteral("format"),
                   QStringLiteral("الخط والمظهر وتوقيت التحليل أثناء الكتابة"));
    createCategory(QStringLiteral("الأدوات"), QStringLiteral("build"),
                   QStringLiteral("مسارات أدوات منظومة باء وخادم اللغة وحالة اكتشافها"));
    createCategory(QStringLiteral("الاختصارات"), QStringLiteral("command-palette"),
                   QStringLiteral("مفاتيح الأوامر. تُطبَّق التغييرات فوراً على القوائم ولوحة الأوامر."));
    categoryList->setCurrentRow(0);

    mainLayout->addWidget(categoryList);
    mainLayout->addWidget(stackedWidget, 1);
}


void QalamSettings::closeEvent(QCloseEvent* event) {
    Q_UNUSED(event)
    QSettings settings = Constants::settings();
    settings.setValue(Constants::SettingsKeyFontSize, fontSpin->value());
    settings.setValue(Constants::SettingsKeyFontType, fontCombo->currentText());
    settings.setValue(Constants::SettingsKeyTheme, themeCombo->currentIndex());
    saveToolPaths();
    settings.sync();
    emit toolPathsChanged();
}

void QalamSettings::createCategory(const QString &name, const QString &iconName,
                                   const QString &description) {
    auto *item = new QListWidgetItem(
        QIcon(QStringLiteral(":/icons/resources/%1.svg").arg(iconName)), name);
    item->setToolTip(description);
    categoryList->addItem(item);

    auto *page = new QWidget;
    auto *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(28, 22, 28, 22);
    pageLayout->setSpacing(14);
    pageLayout->setAlignment(Qt::AlignTop);

    auto *titleLabel = new QLabel(name);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 17pt; font-weight: 600;"));
    titleLabel->setAccessibleName(name);
    pageLayout->addWidget(titleLabel);
    pageLayout->addWidget(createHint(description));

    const qsizetype index = categoryList->count() - 1;
    if (index == 0) {
        createAppearancePage(pageLayout);
    } else if (index == 1) {
        createToolsPage(pageLayout);
    } else {
        createShortcutsPage(pageLayout);
        // The table scrolls itself; a page scroll area would nest scrollbars.
        stackedWidget->addWidget(page);
        return;
    }

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(page);
    stackedWidget->addWidget(scroll);
}

void QalamSettings::createShortcutsPage(QVBoxLayout *layout)
{
    auto *filter = new QLineEdit;
    filter->setObjectName(QStringLiteral("settingsShortcutFilter"));
    filter->setAccessibleName(QStringLiteral("تصفية الاختصارات"));
    filter->setPlaceholderText(QStringLiteral("ابحث باسم الأمر أو المفتاح"));
    filter->setClearButtonEnabled(true);
    filter->setMinimumHeight(34);
    filter->addAction(QIcon(QStringLiteral(":/icons/resources/search.svg")),
                      QLineEdit::LeadingPosition);
    connect(filter, &QLineEdit::textChanged, this, &QalamSettings::filterShortcuts);
    layout->addWidget(filter);

    QVector<CommandRegistry::Command> commands;
    for (const CommandRegistry::Command &command : CommandRegistry::defaultCommands()) {
        if (QalamKeybindings::isRebindable(command.id)) commands.append(command);
    }

    shortcutTable = new QTableWidget(commands.size(), 3);
    shortcutTable->setObjectName(QStringLiteral("settingsShortcutTable"));
    shortcutTable->setAccessibleName(QStringLiteral("جدول الاختصارات"));
    shortcutTable->setHorizontalHeaderLabels(
        {QStringLiteral("الأمر"), QStringLiteral("الاختصار"), QString()});
    shortcutTable->verticalHeader()->hide();
    shortcutTable->setSelectionMode(QAbstractItemView::NoSelection);
    shortcutTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    shortcutTable->setShowGrid(false);
    shortcutTable->setFocusPolicy(Qt::NoFocus);
    shortcutTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    shortcutTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    shortcutTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    shortcutTable->setColumnWidth(1, 220);
    shortcutTable->setColumnWidth(2, 44);
    shortcutTable->verticalHeader()->setDefaultSectionSize(40);
    shortcutTable->setStyleSheet(QStringLiteral(
        "QTableWidget { background: %1; border: 1px solid %2; border-radius: 6px; }"
        " QTableWidget::item { padding: 0 8px; border-bottom: 1px solid %2; }"
        " QHeaderView::section { background: %3; color: %4; border: none;"
        " border-bottom: 1px solid %2; padding: 6px 8px; }")
        .arg(Constants::Colors::EditorBackground, Constants::Colors::BorderSubtle,
             Constants::Colors::SidebarHeaderBackground, Constants::Colors::TextMuted));

    QSettings settings = Constants::settings();
    for (int row = 0; row < commands.size(); ++row) {
        const CommandRegistry::Command &command = commands.at(row);

        auto *titleItem = new QTableWidgetItem(command.title);
        titleItem->setData(Qt::UserRole, command.id);
        titleItem->setData(Qt::UserRole + 1, command.shortcut);
        titleItem->setToolTip(command.description);
        shortcutTable->setItem(row, 0, titleItem);

        auto *edit = new QKeySequenceEdit(QalamKeybindings::effectiveShortcut(command, settings));
        edit->setMaximumSequenceLength(1);
        edit->setClearButtonEnabled(true);
        edit->setLayoutDirection(Qt::LeftToRight);
        // QKeySequenceEdit has no placeholder API; its inner line edit does.
        if (auto *line = edit->findChild<QLineEdit *>())
            line->setPlaceholderText(QStringLiteral("بلا اختصار"));
        edit->setAccessibleName(QStringLiteral("اختصار %1").arg(command.title));
        edit->setToolTip(QStringLiteral("انقر ثم اضغط المفاتيح الجديدة. امسح الحقل لإلغاء الاختصار."));
        shortcutTable->setCellWidget(row, 1, edit);

        auto *reset = new QToolButton;
        reset->setIcon(QIcon(QStringLiteral(":/icons/resources/restart.svg")));
        reset->setIconSize(QSize(16, 16));
        reset->setAutoRaise(true);
        reset->setToolTip(QStringLiteral("استعادة الافتراضي: %1")
                              .arg(command.shortcut.isEmpty() ? QStringLiteral("بلا اختصار")
                                                              : command.shortcut));
        reset->setAccessibleName(QStringLiteral("استعادة اختصار %1").arg(command.title));
        reset->setEnabled(settings.contains(QalamKeybindings::settingsKey(command.id)));
        shortcutTable->setCellWidget(row, 2, reset);

        connect(edit, &QKeySequenceEdit::keySequenceChanged, this,
                [this, row]() { saveShortcut(row); });
        connect(reset, &QToolButton::clicked, this, [this, edit, command]() {
            edit->setKeySequence(QKeySequence::fromString(command.shortcut,
                                                          QKeySequence::PortableText));
        });
    }
    layout->addWidget(shortcutTable, 1);

    shortcutConflictLabel = new QLabel;
    shortcutConflictLabel->setObjectName(QStringLiteral("settingsShortcutConflicts"));
    shortcutConflictLabel->setWordWrap(true);
    shortcutConflictLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    shortcutConflictLabel->setStyleSheet(QStringLiteral("color: %1;")
                                             .arg(Constants::Colors::WarningForeground));
    layout->addWidget(shortcutConflictLabel);

    auto *resetAll = new QPushButton(QIcon(QStringLiteral(":/icons/resources/restart.svg")),
                                     QStringLiteral("استعادة كل الاختصارات الافتراضية"));
    resetAll->setMinimumHeight(34);
    connect(resetAll, &QPushButton::clicked, this, [this]() {
        for (int row = 0; row < shortcutTable->rowCount(); ++row) {
            auto *edit = qobject_cast<QKeySequenceEdit *>(shortcutTable->cellWidget(row, 1));
            const QString fallback = shortcutTable->item(row, 0)->data(Qt::UserRole + 1).toString();
            if (edit) edit->setKeySequence(QKeySequence::fromString(fallback,
                                                                    QKeySequence::PortableText));
        }
    });
    layout->addWidget(resetAll, 0, Qt::AlignLeft);

    refreshShortcutConflicts();
}

void QalamSettings::saveShortcut(int row)
{
    if (not shortcutTable or row < 0 or row >= shortcutTable->rowCount()) return;
    const QTableWidgetItem *titleItem = shortcutTable->item(row, 0);
    auto *edit = qobject_cast<QKeySequenceEdit *>(shortcutTable->cellWidget(row, 1));
    auto *reset = qobject_cast<QToolButton *>(shortcutTable->cellWidget(row, 2));
    if (not titleItem or not edit) return;

    const QString id = titleItem->data(Qt::UserRole).toString();
    const QKeySequence fallback = QKeySequence::fromString(
        titleItem->data(Qt::UserRole + 1).toString(), QKeySequence::PortableText);
    QSettings settings = Constants::settings();
    const QString key = QalamKeybindings::settingsKey(id);
    if (edit->keySequence() == fallback) {
        settings.remove(key);
    } else {
        // Stored even when empty: an empty override unbinds the default.
        settings.setValue(key, edit->keySequence().toString(QKeySequence::PortableText));
    }
    settings.sync();
    if (reset) reset->setEnabled(settings.contains(key));

    refreshShortcutConflicts();
    emit shortcutsChanged();
}

void QalamSettings::refreshShortcutConflicts()
{
    if (not shortcutTable or not shortcutConflictLabel) return;

    QHash<QString, QKeySequence> bindings;
    QHash<QString, QString> titles;
    for (const CommandRegistry::Command &command : CommandRegistry::defaultCommands()) {
        titles.insert(command.id, command.title);
        // Fixed keys still occupy their sequence.
        if (not QalamKeybindings::isRebindable(command.id))
            bindings.insert(command.id, QKeySequence::fromString(command.shortcut,
                                                                 QKeySequence::PortableText));
    }
    for (int row = 0; row < shortcutTable->rowCount(); ++row) {
        const auto *edit = qobject_cast<QKeySequenceEdit *>(shortcutTable->cellWidget(row, 1));
        if (edit) bindings.insert(shortcutTable->item(row, 0)->data(Qt::UserRole).toString(),
                                  edit->keySequence());
    }

    const QHash<QString, QStringList> conflicts = QalamKeybindings::conflicts(bindings);
    QSet<QString> conflicted;
    QStringList lines;
    QStringList keys = conflicts.keys();
    keys.sort();
    for (const QString &key : keys) {
        QStringList names;
        for (const QString &id : conflicts.value(key)) {
            conflicted.insert(id);
            names << QStringLiteral("«%1»").arg(titles.value(id, id));
        }
        lines << QStringLiteral("⚠ %1 مستخدم في: %2")
                     .arg(QKeySequence::fromString(key, QKeySequence::PortableText)
                              .toString(QKeySequence::NativeText),
                          names.join(QStringLiteral("، ")));
    }
    shortcutConflictLabel->setText(lines.join(QLatin1Char('\n')));
    shortcutConflictLabel->setVisible(not lines.isEmpty());

    for (int row = 0; row < shortcutTable->rowCount(); ++row) {
        QTableWidgetItem *item = shortcutTable->item(row, 0);
        const bool clash = conflicted.contains(item->data(Qt::UserRole).toString());
        item->setForeground(QColor(clash ? Constants::Colors::WarningForeground
                                         : Constants::Colors::TextPrimary));
        item->setIcon(clash ? QIcon(QStringLiteral(":/icons/resources/warning.svg")) : QIcon());
    }
}

void QalamSettings::filterShortcuts(const QString &text)
{
    if (not shortcutTable) return;
    const QString needle = text.trimmed();
    for (int row = 0; row < shortcutTable->rowCount(); ++row) {
        const QTableWidgetItem *item = shortcutTable->item(row, 0);
        const auto *edit = qobject_cast<QKeySequenceEdit *>(shortcutTable->cellWidget(row, 1));
        const QString keys = edit ? edit->keySequence().toString(QKeySequence::NativeText)
                                  : QString();
        const bool match = needle.isEmpty()
            or item->text().contains(needle, Qt::CaseInsensitive)
            or item->data(Qt::UserRole).toString().contains(needle, Qt::CaseInsensitive)
            or keys.contains(needle, Qt::CaseInsensitive);
        shortcutTable->setRowHidden(row, not match);
    }
}

void QalamSettings::createToolsPage(QVBoxLayout *layout)
{
    auto *group = createGroup(QStringLiteral("أدوات البناء"));
    auto *grid = new QGridLayout(group);
    grid->setColumnStretch(1, 1);

    QSettings settings = Constants::settings();
    auto addToolRow = [this, grid, &settings](
        int row,
        const QString &label,
        QalamToolKind kind,
        QLineEdit **editor,
        QLabel **status) {
        auto *pathEdit = new QLineEdit;
        pathEdit->setMinimumHeight(36);
        pathEdit->setClearButtonEnabled(true);
        pathEdit->setLayoutDirection(Qt::LeftToRight);
        pathEdit->setText(settings.value(ToolchainDiscovery::settingsKey(kind)).toString());
        pathEdit->setPlaceholderText(QStringLiteral("اكتشاف تلقائي من متغير البيئة ثم PATH"));

        pathEdit->setAccessibleName(label);
        auto *browse = new QPushButton(QIcon(QStringLiteral(":/icons/resources/folder-open.svg")),
                                       QStringLiteral("اختيار…"));
        browse->setMinimumHeight(36);
        connect(browse, &QPushButton::clicked, this,
                [this, kind, pathEdit]() { chooseToolPath(kind, pathEdit); });
        connect(pathEdit, &QLineEdit::textChanged, this,
                [this]() { refreshToolHealth(); });

        auto *statusLabel = new QLabel;
        statusLabel->setWordWrap(true);
        statusLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

        grid->addWidget(new QLabel(label), row * 2, 0);
        grid->addWidget(pathEdit, row * 2, 1);
        grid->addWidget(browse, row * 2, 2);
        grid->addWidget(statusLabel, row * 2 + 1, 1, 1, 2);
        *editor = pathEdit;
        *status = statusLabel;
    };

    addToolRow(0, QStringLiteral("مصرّف باء:"), QalamToolKind::Baa,
               &baaPathEdit, &baaStatusLabel);
    addToolRow(1, QStringLiteral("نظام تكوين:"), QalamToolKind::Takween,
               &takweenPathEdit, &takweenStatusLabel);
    addToolRow(2, QStringLiteral("مجمّع نظم:"), QalamToolKind::Nazm,
               &nazmPathEdit, &nazmStatusLabel);

    auto *hint = createHint(QStringLiteral(
        "اترك الحقل فارغاً للاكتشاف التلقائي. الترتيب: متغير QALAM_*_PATH، "
        "ثم PATH، ثم حزمة محمولة قديمة. يحفظ قلم المسار المختار فقط ولا ينسخ الأدوات."));

    auto *refresh = new QPushButton(QIcon(QStringLiteral(":/icons/resources/restart.svg")),
                                    QStringLiteral("إعادة فحص الأدوات"));
    refresh->setMinimumHeight(36);
    connect(refresh, &QPushButton::clicked, this, &QalamSettings::refreshToolHealth);
    connect(refresh, &QPushButton::clicked, this, &QalamSettings::refreshLanguageServerHealth);

    // Baa-LSP is not a build tool and resolves differently, so it gets its
    // own group instead of a ToolchainDiscovery row.
    auto *serverGroup = createGroup(QStringLiteral("خادم اللغة"));
    auto *serverGrid = new QGridLayout(serverGroup);
    serverGrid->setColumnStretch(1, 1);
    languageServerPathEdit = new QLineEdit;
    languageServerPathEdit->setObjectName(QStringLiteral("settingsLanguageServerPath"));
    languageServerPathEdit->setAccessibleName(QStringLiteral("مسار خادم لغة باء"));
    languageServerPathEdit->setMinimumHeight(36);
    languageServerPathEdit->setClearButtonEnabled(true);
    languageServerPathEdit->setLayoutDirection(Qt::LeftToRight);
    languageServerPathEdit->setText(
        settings.value(Constants::SettingsKeyLanguageServerPath).toString());
    languageServerPathEdit->setPlaceholderText(
        QStringLiteral("اكتشاف تلقائي: BAA_LSP ثم الخادم المرفق مع قلم ثم PATH"));
    auto *serverBrowse = new QPushButton(
        QIcon(QStringLiteral(":/icons/resources/folder-open.svg")), QStringLiteral("اختيار…"));
    serverBrowse->setMinimumHeight(36);
    connect(serverBrowse, &QPushButton::clicked, this, [this]() {
        QString initial = languageServerPathEdit->text().trimmed();
        if (not initial.isEmpty()) initial = QFileInfo(initial).absolutePath();
        const QString chosen = QFileDialog::getOpenFileName(
            this, QStringLiteral("اختيار خادم لغة باء"), initial,
#if defined(Q_OS_WIN)
            QStringLiteral("ملف تنفيذي (*.exe);;كل الملفات (*)")
#else
            QStringLiteral("كل الملفات (*)")
#endif
        );
        if (not chosen.isEmpty())
            languageServerPathEdit->setText(QDir::toNativeSeparators(chosen));
    });
    connect(languageServerPathEdit, &QLineEdit::textChanged,
            this, &QalamSettings::refreshLanguageServerHealth);
    languageServerStatusLabel = new QLabel;
    languageServerStatusLabel->setWordWrap(true);
    languageServerStatusLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    serverGrid->addWidget(new QLabel(QStringLiteral("خادم لغة باء:")), 0, 0);
    serverGrid->addWidget(languageServerPathEdit, 0, 1);
    serverGrid->addWidget(serverBrowse, 0, 2);
    serverGrid->addWidget(languageServerStatusLabel, 1, 1, 1, 2);

    addSection(layout, group);
    layout->addWidget(hint);
    layout->addWidget(refresh, 0, Qt::AlignLeft);
    addSection(layout, serverGroup);
    layout->addWidget(createHint(QStringLiteral(
        "يُعاد تشغيل الخادم بالمسار الجديد عند إغلاق الإعدادات. "
        "المسار المحدد هنا يتقدّم على الخادم المرفق مع قلم.")));
    refreshToolHealth();
    refreshLanguageServerHealth();
}

void QalamSettings::refreshLanguageServerHealth()
{
    if (not languageServerPathEdit or not languageServerStatusLabel) return;
    const QString requested = languageServerPathEdit->text().trimmed();
    if (requested.isEmpty()) {
        languageServerStatusLabel->setText(QStringLiteral("اكتشاف تلقائي عند بدء الخادم"));
        languageServerStatusLabel->setStyleSheet(
            QStringLiteral("color: %1;").arg(Constants::Colors::TextMuted));
        return;
    }
    QString program = QStandardPaths::findExecutable(requested);
    if (program.isEmpty() and QFileInfo(requested).isExecutable()) program = requested;
    if (program.isEmpty()) {
        languageServerStatusLabel->setText(QStringLiteral("✗ غير جاهز: %1")
                                               .arg(QDir::toNativeSeparators(requested)));
        languageServerStatusLabel->setStyleSheet(
            QStringLiteral("color: %1;").arg(Constants::Colors::ErrorForeground));
    } else {
        languageServerStatusLabel->setText(QStringLiteral("✓ جاهز: %1")
                                               .arg(QDir::toNativeSeparators(program)));
        languageServerStatusLabel->setStyleSheet(
            QStringLiteral("color: %1;").arg(Constants::Colors::SuccessForeground));
    }
}

void QalamSettings::chooseToolPath(QalamToolKind kind, QLineEdit *editor)
{
    if (not editor) return;
    QString initial = editor->text().trimmed();
    if (not initial.isEmpty()) initial = QFileInfo(initial).absolutePath();
    const QString chosen = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("اختيار %1").arg(QalamToolResolution{kind}.toolLabel()),
        initial,
#if defined(Q_OS_WIN)
        QStringLiteral("ملف تنفيذي (*.exe);;كل الملفات (*)")
#else
        QStringLiteral("كل الملفات (*)")
#endif
    );
    if (not chosen.isEmpty()) editor->setText(QDir::toNativeSeparators(chosen));
}

void QalamSettings::saveToolPaths()
{
    if (not baaPathEdit or not takweenPathEdit or not nazmPathEdit) return;
    QSettings settings = Constants::settings();
    // An empty field means automatic discovery, which is the absent key.
    const auto store = [&settings](const QString &key, const QLineEdit *edit) {
        const QString path = edit ? edit->text().trimmed() : QString();
        if (path.isEmpty()) settings.remove(key);
        else settings.setValue(key, path);
    };
    store(Constants::SettingsKeyCompilerPath, baaPathEdit);
    store(Constants::SettingsKeyTakweenPath, takweenPathEdit);
    store(Constants::SettingsKeyNazmPath, nazmPathEdit);
    if (languageServerPathEdit)
        store(Constants::SettingsKeyLanguageServerPath, languageServerPathEdit);
    settings.sync();
}

void QalamSettings::refreshToolHealth()
{
    if (not baaPathEdit or not takweenPathEdit or not nazmPathEdit) return;

    // Discovery reads settings, so the edited paths are written temporarily
    // and the previous state is restored afterwards. Absent keys stay absent.
    QSettings settings = Constants::settings();
    QHash<QString, QVariant> previous;
    for (const QString &key : {Constants::SettingsKeyCompilerPath,
                               Constants::SettingsKeyTakweenPath,
                               Constants::SettingsKeyNazmPath}) {
        if (settings.contains(key)) previous.insert(key, settings.value(key));
        else previous.insert(key, QVariant());
    }
    settings.setValue(Constants::SettingsKeyCompilerPath, baaPathEdit->text().trimmed());
    settings.setValue(Constants::SettingsKeyTakweenPath, takweenPathEdit->text().trimmed());
    settings.setValue(Constants::SettingsKeyNazmPath, nazmPathEdit->text().trimmed());
    settings.sync();

    const QList<QalamToolResolution> resolutions = ToolchainDiscovery::resolveAll();
    const QList<QLabel *> labels = {baaStatusLabel, takweenStatusLabel, nazmStatusLabel};
    for (qsizetype index = 0; index < resolutions.size(); ++index) {
        const QalamToolResolution &resolution = resolutions.at(index);
        QLabel *label = labels.at(index);
        if (resolution.isAvailable()) {
            label->setText(QStringLiteral("✓ جاهز من %1: %2")
                               .arg(resolution.sourceLabel(),
                                    QDir::toNativeSeparators(resolution.program)));
            label->setStyleSheet(QStringLiteral("color: #73c991;"));
        } else {
            const QString requested = resolution.requestedProgram.isEmpty()
                ? QStringLiteral("لم يُعثر على ملف تنفيذي")
                : QDir::toNativeSeparators(resolution.requestedProgram);
            label->setText(QStringLiteral("✗ غير جاهز: %1").arg(requested));
            label->setStyleSheet(QStringLiteral("color: #f14c4c;"));
        }
    }

    for (auto it = previous.cbegin(); it != previous.cend(); ++it) {
        if (it.value().isValid()) settings.setValue(it.key(), it.value());
        else settings.remove(it.key());
    }
    settings.sync();
}

void QalamSettings::createAppearancePage(QVBoxLayout* layout) {

    // ================== Font selection ==================
    auto *fontGroup = createGroup(QStringLiteral("الخط"));
    QVBoxLayout* fontLayout = new QVBoxLayout(fontGroup);
    QFormLayout* fontSizeLayout = new QFormLayout();
    QFormLayout* fontFamilyLayout = new QFormLayout();

    fontSpin = new QSpinBox;
    fontSpin->setRange(12, 36);
    fontSpin->setMinimumHeight(40);
    fontSpin->setMaximumWidth(80);

    QSettings settingsVal = Constants::settings();
    int savedSize = settingsVal.value(Constants::SettingsKeyFontSize).toInt();
    savedSize ? fontSpin->setValue(savedSize) : fontSpin->setValue(Constants::DefaultFontSize);

    fontSizeLayout->addRow("حجم الخط: ", fontSpin);
    connect(fontSpin, &QSpinBox::valueChanged, this, &QalamSettings::fontSizeChanged);


    fontCombo = new QComboBox();
    fontCombo->setEditable(true);
    fontCombo->setInsertPolicy(QComboBox::NoInsert);
    fontCombo->setMinimumHeight(40);
    fontCombo->setMaximumWidth(200);

    QStringList fontFamilies = QFontDatabase::families();
    fontFamilies.sort(Qt::CaseInsensitive);

    const QStringList preferredFonts = {
        Constants::DefaultFontType,
        "Noto Kufi Arabic",
        "Tajawal",
        "Kawkab Mono"
    };
    for (const QString &family : preferredFonts) {
        if (!family.isEmpty() && !fontFamilies.contains(family, Qt::CaseInsensitive)) {
            fontFamilies.prepend(family);
        }
    }

    fontCombo->addItems(fontFamilies);
    QString savedFont = settingsVal.value(Constants::SettingsKeyFontType).toString();
    !savedFont.isEmpty() ? fontCombo->setCurrentText(savedFont) : fontCombo->setCurrentText(Constants::DefaultFontType);

    fontFamilyLayout->addRow("نوع الخط: ", fontCombo);
    connect(fontCombo, &QComboBox::currentTextChanged, this, &QalamSettings::fontTypeChanged);

    fontLayout->addLayout(fontSizeLayout);
    fontLayout->addLayout(fontFamilyLayout);

    // ================== Themes ==================
    auto *themeGroup = createGroup(QStringLiteral("المظهر"));
    QVBoxLayout* themeLayout = new QVBoxLayout(themeGroup);
    QFormLayout* comboLayout = new QFormLayout();

    themeCombo = new QComboBox();
    themeCombo->setInsertPolicy(QComboBox::NoInsert);
    themeCombo->setMinimumHeight(40);
    themeCombo->setMaximumWidth(250);

    // Populate UI
    auto availableThemes = ThemeManager::getAvailableThemes();
    for (const auto& theme : availableThemes) {
        themeCombo->addItem(theme->name());
    }

    int savedTheme = settingsVal.value(Constants::SettingsKeyTheme).toInt();
    if (savedTheme < 0 || savedTheme >= themeCombo->count()) savedTheme = 0;
    themeCombo->setCurrentIndex(savedTheme);

    comboLayout->addRow("مظهر الشيفرة: ", themeCombo);
    connect(themeCombo, &QComboBox::currentIndexChanged, this, &QalamSettings::highlighterThemeChanged);

    themeLayout->addLayout(comboLayout);

    auto *analysisGroup = createGroup(QStringLiteral("التحليل أثناء الكتابة"));
    auto *analysisLayout = new QFormLayout(analysisGroup);
    analysisDelaySpin = new QSpinBox;
    analysisDelaySpin->setObjectName(QStringLiteral("settingsAnalysisDelay"));
    analysisDelaySpin->setAccessibleName(QStringLiteral("مهلة التحليل بالمللي ثانية"));
    analysisDelaySpin->setRange(0, Constants::Timing::AnalysisDelayMax);
    analysisDelaySpin->setSingleStep(25);
    analysisDelaySpin->setSuffix(QStringLiteral(" مللي ثانية"));
    analysisDelaySpin->setMinimumHeight(40);
    analysisDelaySpin->setMaximumWidth(180);
    analysisDelaySpin->setValue(settingsVal.value(Constants::SettingsKeyAnalysisDelay,
                                                  Constants::Timing::AnalysisDelay).toInt());
    analysisLayout->addRow(QStringLiteral("المهلة قبل التحليل: "), analysisDelaySpin);
    analysisLayout->addRow(createHint(QStringLiteral(
        "المدة التي ينتظرها قلم بعد آخر ضغطة قبل أن يطلب من خادم اللغة تحليل الملف. "
        "قيمة أكبر تخفف الحمل على الأجهزة البطيئة، وقيمة أصغر تظهر الأخطاء أسرع.")));
    connect(analysisDelaySpin, &QSpinBox::valueChanged, this, [this](int milliseconds) {
        QSettings settings = Constants::settings();
        settings.setValue(Constants::SettingsKeyAnalysisDelay, milliseconds);
        emit analysisDelayChanged(milliseconds);
    });

    auto *completionGroup = createGroup(QStringLiteral("ترتيب الإكمال"));
    auto *completionLayout = new QVBoxLayout(completionGroup);
    auto *completionHint = createHint(QStringLiteral(
        "يرتب قلم النتائج المتساوية دلالياً بحسب الاقتراحات التي اخترتها "
        "فعلياً داخل السياق نفسه. لا تُرسل هذه البيانات خارج الجهاز."));
    auto *clearCompletionHistory = new QPushButton(
        QIcon(QStringLiteral(":/icons/resources/trash.svg")),
        QStringLiteral("مسح سجل ترتيب الاقتراحات"));
    clearCompletionHistory->setMinimumHeight(36);
    connect(clearCompletionHistory, &QPushButton::clicked, this,
            [clearCompletionHistory]() {
        QSettings settings = Constants::settings();
        QalamCompletionHistory::clear(settings);
        clearCompletionHistory->setText(QStringLiteral("تم مسح السجل"));
    });
    completionLayout->addWidget(completionHint);
    completionLayout->addWidget(clearCompletionHistory, 0, Qt::AlignLeft);

    addSection(layout, fontGroup);
    addSection(layout, themeGroup);
    addSection(layout, analysisGroup);
    addSection(layout, completionGroup);
}

QComboBox *QalamSettings::getThemeCombo() const {
    return themeCombo;
}
