#include "TWelcomePage.h"

#include "Constants.h"

#include <QCheckBox>
#include <QDir>
#include <QFileInfo>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QVBoxLayout>

namespace {
QWidget *createSectionTitle(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setObjectName("welcomeSectionTitle");
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    return label;
}
}

TWelcomePage::TWelcomePage(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("welcomePage");
    setLayoutDirection(Qt::RightToLeft);
    setupUi();
    applyStyles();
    populateRecents();
}

void TWelcomePage::refreshRecents()
{
    populateRecents();
}

void TWelcomePage::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto *scroll = new QScrollArea(this);
    scroll->setObjectName("welcomeScroll");
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    rootLayout->addWidget(scroll);

    auto *scrollContent = new QWidget(scroll);
    scrollContent->setObjectName("welcomeScrollContent");
    scroll->setWidget(scrollContent);

    auto *outerLayout = new QVBoxLayout(scrollContent);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);
    outerLayout->addStretch(1);

    auto *container = new QWidget(scrollContent);
    container->setObjectName("welcomeContainer");
    outerLayout->addWidget(container, 0, Qt::AlignHCenter);

    auto *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(48, 40, 48, 28);
    containerLayout->setSpacing(24);

    // Header
    auto *header = new QWidget(container);
    header->setObjectName("welcomeHeader");
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(18);
    headerLayout->setDirection(QBoxLayout::RightToLeft);

    auto *logoLabel = new QLabel(header);
    logoLabel->setObjectName("welcomeLogo");
    logoLabel->setPixmap(QPixmap(":/icons/resources/QalamLogo.ico")
                             .scaled(72, 72, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setFixedSize(72, 72);

    auto *titleCol = new QVBoxLayout();
    titleCol->setContentsMargins(0, 0, 0, 0);
    titleCol->setSpacing(4);

    auto *title = new QLabel("محرر قلم", header);
    title->setObjectName("welcomeTitle");

    auto *subtitle = new QLabel("بيئة تطوير متكاملة للغة باء", header);
    subtitle->setObjectName("welcomeSubtitle");

    auto *version = new QLabel(QString("الإصدار %1").arg(Constants::AppVersion), header);
    version->setObjectName("welcomeVersion");

    titleCol->addWidget(title);
    titleCol->addWidget(subtitle);
    titleCol->addWidget(version);

    headerLayout->addLayout(titleCol);
    headerLayout->addWidget(logoLabel);
    headerLayout->addStretch(1);

    containerLayout->addWidget(header);

    // Columns (Start / Recent)
    auto *columns = new QHBoxLayout();
    columns->setContentsMargins(0, 0, 0, 0);
    columns->setSpacing(32);
    columns->setDirection(QBoxLayout::RightToLeft);

    // Start column
    auto *startCol = new QVBoxLayout();
    startCol->setContentsMargins(0, 0, 0, 0);
    startCol->setSpacing(10);
    startCol->setAlignment(Qt::AlignTop);

    startCol->addWidget(createSectionTitle("ابدأ", container));

    auto *newFileBtn = createActionButton(":/icons/resources/file-new.svg", "ملف جديد");
    connect(newFileBtn, &QPushButton::clicked, this, &TWelcomePage::newFileRequested);
    startCol->addWidget(newFileBtn);

    auto *openFileBtn = createActionButton(":/icons/resources/folder-open.svg", "فتح ملف...");
    connect(openFileBtn, &QPushButton::clicked, this, &TWelcomePage::openFileRequested);
    startCol->addWidget(openFileBtn);

    auto *openFolderBtn = createActionButton(":/icons/resources/folder.svg", "فتح مجلد...");
    connect(openFolderBtn, &QPushButton::clicked, this, &TWelcomePage::openFolderRequested);
    startCol->addWidget(openFolderBtn);

    auto *cloneBtn = createActionButton(":/icons/resources/git-clone.svg", "استنساخ مستودع...");
    connect(cloneBtn, &QPushButton::clicked, this, &TWelcomePage::cloneRepoRequested);
    startCol->addWidget(cloneBtn);

    startCol->addStretch(1);

    // Recent column
    auto *recentCol = new QVBoxLayout();
    recentCol->setContentsMargins(0, 0, 0, 0);
    recentCol->setSpacing(10);
    recentCol->setAlignment(Qt::AlignTop);

    auto *recentHeader = new QWidget(container);
    auto *recentHeaderLayout = new QHBoxLayout(recentHeader);
    recentHeaderLayout->setContentsMargins(0, 0, 0, 0);
    recentHeaderLayout->setSpacing(8);
    recentHeaderLayout->setDirection(QBoxLayout::RightToLeft);

    auto *recentTitle = qobject_cast<QLabel*>(createSectionTitle("الأخيرة", recentHeader));
    recentHeaderLayout->addWidget(recentTitle);

    recentHeaderLayout->addStretch(1);

    m_clearRecentsBtn = new QPushButton("مسح الكل", recentHeader);
    m_clearRecentsBtn->setObjectName("welcomeClearRecents");
    m_clearRecentsBtn->setFlat(true);
    m_clearRecentsBtn->setCursor(Qt::PointingHandCursor);
    connect(m_clearRecentsBtn, &QPushButton::clicked, this, &TWelcomePage::onClearRecentsRequested);
    recentHeaderLayout->addWidget(m_clearRecentsBtn);

    recentCol->addWidget(recentHeader);

    m_recentList = new QListWidget(container);
    m_recentList->setObjectName("welcomeRecents");
    m_recentList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_recentList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_recentList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_recentList->setFocusPolicy(Qt::NoFocus);
    connect(m_recentList, &QListWidget::itemClicked, this, &TWelcomePage::onRecentItemActivated);
    connect(m_recentList, &QListWidget::itemDoubleClicked, this, &TWelcomePage::onRecentItemActivated);
    recentCol->addWidget(m_recentList, 1);

    columns->addLayout(startCol, 0);
    columns->addLayout(recentCol, 1);

    containerLayout->addLayout(columns, 1);

    // Footer
    auto *footer = new QWidget(container);
    footer->setObjectName("welcomeFooter");
    auto *footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(0, 0, 0, 0);
    footerLayout->setSpacing(12);

    auto *hintLabel = new QLabel("اضغط Ctrl+N لملف جديد • Ctrl+O لفتح ملف", footer);
    hintLabel->setObjectName("welcomeHint");

    m_showOnStartupCheck = new QCheckBox("إظهار صفحة الترحيب عند بدء البرنامج", footer);
    m_showOnStartupCheck->setObjectName("welcomeStartupCheck");
    m_showOnStartupCheck->setChecked(loadShowOnStartup());
    connect(m_showOnStartupCheck, &QCheckBox::toggled, this, &TWelcomePage::onShowOnStartupToggled);

    footerLayout->addWidget(hintLabel);
    footerLayout->addStretch(1);
    footerLayout->addWidget(m_showOnStartupCheck);

    containerLayout->addWidget(footer);

    outerLayout->addStretch(2);
}

void TWelcomePage::applyStyles()
{
    using namespace Constants;

    setStyleSheet(QString(R"(
        #welcomePage {
            background-color: %1;
        }

        #welcomeContainer {
            background-color: transparent;
        }

        #welcomeTitle {
            color: %2;
            font-size: 30px;
            font-weight: 700;
            font-family: 'Tajawal', 'Segoe UI', sans-serif;
        }

        #welcomeSubtitle {
            color: %3;
            font-size: 14px;
            font-family: 'Tajawal', 'Segoe UI', sans-serif;
        }

        #welcomeVersion {
            color: %4;
            font-size: 12px;
        }

        #welcomeSectionTitle {
            color: %2;
            font-size: 16px;
            font-weight: 700;
            padding-bottom: 8px;
            border-bottom: 1px solid %5;
        }

        QPushButton#welcomeActionButton {
            background: transparent;
            border: 1px solid transparent;
            border-radius: 6px;
            padding: 10px 12px;
            text-align: right;
            color: %3;
            font-size: 13px;
        }

        QPushButton#welcomeActionButton:hover {
            background: %6;
        }

        QPushButton#welcomeActionButton:pressed {
            background: %7;
        }

        QPushButton#welcomeClearRecents {
            color: %4;
            font-size: 12px;
            padding: 2px 6px;
        }

        QPushButton#welcomeClearRecents:hover {
            color: %8;
        }

        QListWidget#welcomeRecents {
            background-color: %9;
            border: 1px solid %5;
            border-radius: 8px;
            padding: 6px;
        }

        QListWidget#welcomeRecents::item {
            padding: 0px;
            margin: 2px 0px;
            border-radius: 6px;
        }

        QListWidget#welcomeRecents::item:hover {
            background-color: %6;
        }

        QListWidget#welcomeRecents::item:selected {
            background-color: %10;
        }

        #welcomeHint {
            color: %4;
            font-size: 11px;
        }

        #welcomeStartupCheck {
            color: %3;
            font-size: 12px;
        }

        #welcomeRecentsEmptyTitle {
            color: %4;
            font-size: 14px;
        }

        #welcomeRecentsEmptyHint {
            color: %4;
            font-size: 12px;
        }

        #welcomeRecentName {
            color: %2;
            font-size: 13px;
            font-weight: 600;
        }

        #welcomeRecentPath {
            color: %4;
            font-size: 11px;
        }
    )")
                      .arg(Colors::WindowBackground)          // %1
                      .arg(Colors::TextPrimary)               // %2
                      .arg(Colors::TextSecondary)             // %3
                      .arg(Colors::TextMuted)                 // %4
                      .arg(Colors::BorderSubtle)              // %5
                      .arg(Colors::ListHoverBackground)       // %6
                      .arg(Colors::ButtonPressed)             // %7
                      .arg(Colors::AccentHover)               // %8
                      .arg(Colors::SidebarBackground)         // %9
                      .arg(Colors::ListSelectionBackground)); // %10
}

QPushButton *TWelcomePage::createActionButton(const QString &iconPath, const QString &text)
{
    auto *btn = new QPushButton(text, this);
    btn->setObjectName("welcomeActionButton");
    btn->setCursor(Qt::PointingHandCursor);
    btn->setIcon(QIcon(iconPath));
    btn->setIconSize(QSize(18, 18));
    btn->setLayoutDirection(Qt::RightToLeft);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    return btn;
}

void TWelcomePage::populateRecents()
{
    if (!m_recentList) return;

    m_recentList->clear();

    QSettings settings(Constants::OrgName, Constants::AppName);
    const QStringList recentFiles = settings.value(Constants::SettingsKeyRecentFiles).toStringList();

    if (recentFiles.isEmpty()) {
        showEmptyRecentsState();
        return;
    }

    for (const QString &path : recentFiles) {
        if (path.trimmed().isEmpty()) continue;

        QFileInfo info(path);

        auto *itemWidget = new QWidget(m_recentList);
        itemWidget->setObjectName("welcomeRecentItemWidget");
        auto *layout = new QVBoxLayout(itemWidget);
        layout->setContentsMargins(10, 8, 10, 8);
        layout->setSpacing(4);

        auto *nameRow = new QHBoxLayout();
        nameRow->setContentsMargins(0, 0, 0, 0);
        nameRow->setSpacing(8);
        nameRow->setDirection(QBoxLayout::RightToLeft);

        auto *iconLabel = new QLabel(itemWidget);
        const QString iconPath = info.isDir() ? ":/icons/resources/folder.svg" : ":/icons/resources/file-new.svg";
        iconLabel->setPixmap(QIcon(iconPath).pixmap(16, 16));
        iconLabel->setFixedSize(16, 16);

        auto *nameLabel = new QLabel(info.fileName().isEmpty() ? path : info.fileName(), itemWidget);
        nameLabel->setObjectName("welcomeRecentName");

        nameRow->addWidget(iconLabel);
        nameRow->addWidget(nameLabel, 1);
        nameRow->addStretch(1);

        const QFontMetrics fm(font());
        const QString elidedPath = fm.elidedText(path, Qt::ElideMiddle, 520);

        auto *pathLabel = new QLabel(elidedPath, itemWidget);
        pathLabel->setObjectName("welcomeRecentPath");
        pathLabel->setToolTip(path);

        if (!info.exists()) {
            nameLabel->setStyleSheet(QString("color: %1; text-decoration: line-through;")
                                         .arg(Constants::Colors::TextMuted));
        }

        layout->addLayout(nameRow);
        layout->addWidget(pathLabel);

        auto *item = new QListWidgetItem(m_recentList);
        item->setData(Qt::UserRole, path);
        item->setSizeHint(QSize(0, 56));
        m_recentList->addItem(item);
        m_recentList->setItemWidget(item, itemWidget);
    }
}

void TWelcomePage::showEmptyRecentsState()
{
    if (!m_recentList) return;

    auto *emptyWidget = new QWidget(m_recentList);
    auto *layout = new QVBoxLayout(emptyWidget);
    layout->setContentsMargins(0, 18, 0, 18);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(6);

    auto *icon = new QLabel(emptyWidget);
    icon->setPixmap(QIcon(":/icons/resources/folder-open.svg").pixmap(46, 46));
    icon->setAlignment(Qt::AlignCenter);

    auto *title = new QLabel("لا توجد ملفات حديثة", emptyWidget);
    title->setObjectName("welcomeRecentsEmptyTitle");
    title->setAlignment(Qt::AlignCenter);

    auto *hint = new QLabel("الملفات التي تفتحها ستظهر هنا", emptyWidget);
    hint->setObjectName("welcomeRecentsEmptyHint");
    hint->setAlignment(Qt::AlignCenter);

    layout->addWidget(icon);
    layout->addWidget(title);
    layout->addWidget(hint);

    auto *item = new QListWidgetItem(m_recentList);
    item->setFlags(Qt::NoItemFlags);
    item->setSizeHint(QSize(0, 160));
    m_recentList->addItem(item);
    m_recentList->setItemWidget(item, emptyWidget);
}

void TWelcomePage::onRecentItemActivated(QListWidgetItem *item)
{
    if (!item) return;

    const QString path = item->data(Qt::UserRole).toString();
    if (path.isEmpty()) return;

    QFileInfo info(path);
    if (!info.exists()) {
        const auto reply = QMessageBox::question(
            this,
            "الملف غير موجود",
            QString("الملف التالي غير موجود:\n%1\n\nهل تريد إزالته من القائمة؟").arg(path),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes);

        if (reply == QMessageBox::Yes) {
            removeFromRecents(path);
            populateRecents();
        }
        return;
    }

    emit recentFileRequested(path);
}

void TWelcomePage::removeFromRecents(const QString &path)
{
    QSettings settings(Constants::OrgName, Constants::AppName);
    QStringList recentFiles = settings.value(Constants::SettingsKeyRecentFiles).toStringList();
    recentFiles.removeAll(path);
    settings.setValue(Constants::SettingsKeyRecentFiles, recentFiles);
}

void TWelcomePage::onClearRecentsRequested()
{
    const auto reply = QMessageBox::question(
        this,
        "مسح الملفات الأخيرة",
        "هل تريد مسح قائمة الملفات الأخيرة؟",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    QSettings settings(Constants::OrgName, Constants::AppName);
    settings.remove(Constants::SettingsKeyRecentFiles);
    populateRecents();
}

bool TWelcomePage::loadShowOnStartup() const
{
    QSettings settings(Constants::OrgName, Constants::AppName);
    return settings.value(Constants::SettingsKeyShowWelcome, true).toBool();
}

void TWelcomePage::saveShowOnStartup(bool show)
{
    QSettings settings(Constants::OrgName, Constants::AppName);
    settings.setValue(Constants::SettingsKeyShowWelcome, show);
}

void TWelcomePage::onShowOnStartupToggled(bool show)
{
    saveShowOnStartup(show);
    emit showWelcomeOnStartupChanged(show);
}

