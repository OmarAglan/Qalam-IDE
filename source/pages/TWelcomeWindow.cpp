#include "TWelcomeWindow.h"
#include "Qalam.h"
#include "Constants.h"
#include <QtWidgets>
#include <QFontDatabase>

WelcomeWindow::WelcomeWindow(QWidget *parent)
    : QalamWindow(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setLayoutDirection(Qt::RightToLeft);  
    setWindowTitle("صفحة الترحيب - محرر قلم");

    // Apply welcome window specific styles
    applyStyles();

    // Main central widget
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setObjectName("welcomeCentral");
    setCentralWidget(centralWidget);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 20);
    mainLayout->setSpacing(0);

    // 1. Header (Logo + Title)
    QWidget *headerWidget = new QWidget();
    headerWidget->setObjectName("welcomeHeader");
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(40, 40, 40, 20);
    headerLayout->setDirection(QBoxLayout::RightToLeft); 

    QLabel *logoLabel = new QLabel();
    logoLabel->setPixmap(QPixmap(":/icons/resources/QalamLogo.ico").scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(5);
    
    QLabel *titleLabel = new QLabel("محرر قلم");
    titleLabel->setObjectName("mainTitle");
    
    QLabel *subtitleLabel = new QLabel("بيئة تطوير متكاملة للغة باء");
    subtitleLabel->setObjectName("subtitle");

    QLabel *versionLabel = new QLabel("الإصدار 3.3.0");
    versionLabel->setObjectName("versionLabel");
    
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);
    titleLayout->addWidget(versionLabel);

    headerLayout->addLayout(titleLayout);
    headerLayout->addSpacing(20);
    headerLayout->addWidget(logoLabel);
    headerLayout->addStretch();
    
    mainLayout->addWidget(headerWidget);

    // 2. Content Area (Two Columns)
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(60, 20, 60, 20);
    contentLayout->setSpacing(60);

    // Start Actions (will be on Right due to RTL)
    QVBoxLayout *startCol = createStartColumn();
    
    // Recent Files (will be on Left due to RTL)
    QVBoxLayout *recentCol = createRecentColumn();

    // RTL: first added goes Right
    contentLayout->addLayout(startCol, 0);
    contentLayout->addLayout(recentCol, 1);

    mainLayout->addLayout(contentLayout, 1);

    // 3. Footer (Checkbox + Keyboard Hint)
    QWidget *footerWidget = new QWidget();
    footerWidget->setObjectName("welcomeFooter");
    QHBoxLayout *footerLayout = new QHBoxLayout(footerWidget);
    footerLayout->setContentsMargins(40, 10, 40, 0);
    
    // Keyboard shortcut hint
    QLabel *hintLabel = new QLabel("اضغط Ctrl+N لملف جديد • Ctrl+O لفتح ملف");
    hintLabel->setObjectName("hintLabel");
    
    showOnStartupCheck = new QCheckBox("إظهار صفحة الترحيب عند بدء البرنامج");
    showOnStartupCheck->setObjectName("startupCheck");
    showOnStartupCheck->setChecked(loadShowOnStartup());
    connect(showOnStartupCheck, &QCheckBox::toggled, this, &WelcomeWindow::saveShowOnStartup);
    
    footerLayout->addWidget(hintLabel);
    footerLayout->addStretch();
    footerLayout->addWidget(showOnStartupCheck);
    
    mainLayout->addWidget(footerWidget);
    
    // Geometry
    resize(1000, 700);
}

WelcomeWindow::~WelcomeWindow()
{
}

void WelcomeWindow::applyStyles()
{
    QString styles = QString(R"(
        /* Main backgrounds */
        #welcomeCentral {
            background-color: %1;
        }
        
        /* Header styling */
        #welcomeHeader {
            background-color: transparent;
        }
        
        #mainTitle {
            font-size: 32px;
            font-weight: bold;
            color: %2;
            font-family: 'Tajawal', 'Segoe UI';
        }
        
        #subtitle {
            font-size: 16px;
            color: %3;
            font-family: 'Tajawal', 'Segoe UI';
        }
        
        #versionLabel {
            font-size: 12px;
            color: %4;
            margin-top: 5px;
        }
        
        /* Section titles */
        #sectionTitle {
            font-size: 18px;
            font-weight: bold;
            color: %2;
            padding-bottom: 10px;
            border-bottom: 1px solid %5;
            margin-bottom: 10px;
        }
        
        /* Action links */
        .actionContainer {
            padding: 12px 15px;
            border-radius: 6px;
            background-color: transparent;
        }
        
        .actionContainer:hover {
            background-color: %6;
        }
        
        #actionLink {
            font-size: 14px;
            color: %7;
            font-family: 'Tajawal', 'Segoe UI';
        }
        
        #actionLink:hover {
            color: %8;
        }
        
        /* Recent files list */
        QListWidget {
            background-color: %9;
            border: 1px solid %5;
            border-radius: 8px;
            padding: 5px;
            outline: none;
        }
        
        QListWidget::item {
            padding: 8px;
            border-radius: 6px;
            margin: 2px;
        }
        
        QListWidget::item:hover {
            background-color: %6;
        }
        
        QListWidget::item:selected {
            background-color: %10;
        }
        
        /* Footer */
        #welcomeFooter {
            background-color: transparent;
        }
        
        #hintLabel {
            color: %4;
            font-size: 11px;
        }
        
        #startupCheck {
            color: %3;
            font-size: 12px;
        }
        
        #startupCheck::indicator {
            width: 16px;
            height: 16px;
        }
        
        /* Empty state */
        #emptyStateLabel {
            color: %4;
            font-size: 14px;
            padding: 40px;
        }

        /* Scrollbar styling */
        QScrollBar:vertical {
            background: %9;
            width: 8px;
            border-radius: 4px;
        }
        
        QScrollBar::handle:vertical {
            background: %5;
            border-radius: 4px;
            min-height: 30px;
        }
        
        QScrollBar::handle:vertical:hover {
            background: %4;
        }
        
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )")
    .arg(Constants::Colors::WindowBackground)      // %1 - main background
    .arg(Constants::Colors::TextPrimary)           // %2 - primary text (white)
    .arg(Constants::Colors::TextSecondary)         // %3 - secondary text
    .arg(Constants::Colors::TextMuted)             // %4 - muted text
    .arg(Constants::Colors::Border)                // %5 - borders
    .arg(Constants::Colors::TabHoverBackground)    // %6 - hover background
    .arg(Constants::Colors::Accent)                // %7 - accent (link color)
    .arg("#5cb8ff")                                // %8 - accent hover (lighter blue)
    .arg(Constants::Colors::SidebarBackground)     // %9 - list background
    .arg(Constants::Colors::Selection);            // %10 - selection

    setStyleSheet(styles);
}

QVBoxLayout* WelcomeWindow::createStartColumn()
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignTop);
    
    QLabel *title = new QLabel("ابدأ");
    title->setObjectName("sectionTitle");
    title->setAlignment(Qt::AlignRight);
    layout->addWidget(title);
    layout->addSpacing(10);
    
    layout->addWidget(createActionLink(":/icons/resources/file-new.svg", "ملف جديد", "new"));
    layout->addWidget(createActionLink(":/icons/resources/folder-open.svg", "فتح ملف...", "open_file"));
    layout->addWidget(createActionLink(":/icons/resources/folder.svg", "فتح مجلد...", "open_folder"));
    layout->addWidget(createActionLink(":/icons/resources/git-clone.svg", "استنساخ مستودع...", "clone"));
    
    layout->addStretch();
    return layout;
}

QVBoxLayout* WelcomeWindow::createRecentColumn()
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignTop);
    
    // Header with title and clear button
    QHBoxLayout *headerLayout = new QHBoxLayout();
    
    QLabel *title = new QLabel("الأخيرة");
    title->setObjectName("sectionTitle");
    title->setAlignment(Qt::AlignRight);
    
    QPushButton *clearBtn = new QPushButton("مسح الكل");
    clearBtn->setObjectName("clearRecentBtn");
    clearBtn->setCursor(Qt::PointingHandCursor);
    clearBtn->setStyleSheet(QString(
        "QPushButton { background: transparent; color: %1; border: none; font-size: 12px; }"
        "QPushButton:hover { color: %2; }"
    ).arg(Constants::Colors::TextMuted).arg(Constants::Colors::Accent));
    connect(clearBtn, &QPushButton::clicked, this, &WelcomeWindow::clearRecentFiles);
    
    headerLayout->addWidget(title);
    headerLayout->addStretch();
    headerLayout->addWidget(clearBtn);
    
    layout->addLayout(headerLayout);
    layout->addSpacing(10);
    
    recentProjectsList = new QListWidget();
    recentProjectsList->setFocusPolicy(Qt::NoFocus);
    recentProjectsList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    recentProjectsList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    recentProjectsList->setSelectionMode(QAbstractItemView::SingleSelection);
    
    connect(recentProjectsList, &QListWidget::itemClicked, this, &WelcomeWindow::onRecentFileClicked);
    connect(recentProjectsList, &QListWidget::itemDoubleClicked, this, &WelcomeWindow::onRecentFileClicked);
    
    populateRecentProjects();
    
    layout->addWidget(recentProjectsList);
    return layout;
}

QWidget* WelcomeWindow::createActionLink(const QString &iconPath, const QString &text, const QString &actionId)
{
    QWidget *container = new QWidget();
    container->setObjectName("actionContainer");
    container->setCursor(Qt::PointingHandCursor);
    container->setProperty("action", actionId);
    container->installEventFilter(this);
    
    QHBoxLayout *layout = new QHBoxLayout(container);
    layout->setContentsMargins(15, 12, 15, 12);
    layout->setSpacing(12);
    
    QLabel *icon = new QLabel();
    QPixmap pixmap = QIcon(iconPath).pixmap(22, 22);
    icon->setPixmap(pixmap);
    icon->setFixedSize(22, 22);
    
    QLabel *label = new QLabel(text);
    label->setObjectName("actionLink");
    
    // RTL order: Icon (Right) -> Text (Left) -> Stretch (Far Left)
    layout->addWidget(icon);
    layout->addWidget(label);
    layout->addStretch();
    
    return container;
}

bool WelcomeWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QString action = obj->property("action").toString();
        if (!action.isEmpty()) {
            if (action == "new") handleNewFileRequest();
            else if (action == "open_file") handleOpenFileRequest();
            else if (action == "open_folder") handleOpenFolderRequest();
            else if (action == "clone") handleCloneRepo();
            return true;
        }
    }
    // Hover effect for action containers
    else if (event->type() == QEvent::Enter) {
        if (QWidget *w = qobject_cast<QWidget*>(obj)) {
            if (w->property("action").isValid()) {
                w->setStyleSheet(QString("background-color: %1; border-radius: 6px;")
                    .arg(Constants::Colors::TabHoverBackground));
            }
        }
    }
    else if (event->type() == QEvent::Leave) {
        if (QWidget *w = qobject_cast<QWidget*>(obj)) {
            if (w->property("action").isValid()) {
                w->setStyleSheet("");
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void WelcomeWindow::populateRecentProjects()
{
    // FIX: Use correct organization name from Constants
    QSettings settings(Constants::OrgName, Constants::AppName);
    QStringList recentFiles = settings.value(Constants::SettingsKeyRecentFiles).toStringList();
    
    recentProjectsList->clear();
    
    if (recentFiles.isEmpty()) {
        // Show empty state with icon
        QWidget *emptyWidget = new QWidget();
        QVBoxLayout *emptyLayout = new QVBoxLayout(emptyWidget);
        emptyLayout->setAlignment(Qt::AlignCenter);
        
        QLabel *emptyIcon = new QLabel();
        emptyIcon->setPixmap(QIcon(":/icons/resources/folder-open.svg").pixmap(48, 48));
        emptyIcon->setAlignment(Qt::AlignCenter);
        emptyIcon->setStyleSheet("opacity: 0.5;");
        
        QLabel *emptyLabel = new QLabel("لا توجد ملفات حديثة");
        emptyLabel->setObjectName("emptyStateLabel");
        emptyLabel->setAlignment(Qt::AlignCenter);
        
        QLabel *hintLabel = new QLabel("الملفات التي تفتحها ستظهر هنا");
        hintLabel->setStyleSheet(QString("color: %1; font-size: 12px;").arg(Constants::Colors::TextMuted));
        hintLabel->setAlignment(Qt::AlignCenter);
        
        emptyLayout->addStretch();
        emptyLayout->addWidget(emptyIcon);
        emptyLayout->addWidget(emptyLabel);
        emptyLayout->addWidget(hintLabel);
        emptyLayout->addStretch();
        
        QListWidgetItem *item = new QListWidgetItem(recentProjectsList);
        item->setFlags(Qt::NoItemFlags);
        item->setSizeHint(QSize(100, 150));
        recentProjectsList->setItemWidget(item, emptyWidget);
        return;
    }

    for (const QString &path : recentFiles) {
        QFileInfo info(path);
        
        // Create custom widget for each item
        QWidget *itemWidget = new QWidget();
        itemWidget->setStyleSheet("background: transparent;");
        
        QVBoxLayout *layout = new QVBoxLayout(itemWidget);
        layout->setContentsMargins(10, 8, 10, 8);
        layout->setSpacing(4);
        
        // File/folder name with icon
        QHBoxLayout *nameLayout = new QHBoxLayout();
        nameLayout->setSpacing(8);
        
        QLabel *iconLabel = new QLabel();
        QString iconPath = info.isDir() ? ":/icons/resources/folder.svg" : ":/icons/resources/file-new.svg";
        iconLabel->setPixmap(QIcon(iconPath).pixmap(16, 16));
        iconLabel->setFixedSize(16, 16);
        
        QLabel *nameLabel = new QLabel(info.fileName());
        nameLabel->setStyleSheet(QString("font-weight: bold; color: %1; font-size: 14px;")
            .arg(Constants::Colors::Accent));
        
        // Check if file exists
        if (!info.exists()) {
            nameLabel->setStyleSheet(QString("font-weight: bold; color: %1; font-size: 14px; text-decoration: line-through;")
                .arg(Constants::Colors::TextMuted));
        }
        
        nameLayout->addWidget(iconLabel);
        nameLayout->addWidget(nameLabel);
        nameLayout->addStretch();
        
        // Path (truncated if too long)
        QString displayPath = path;
        QFontMetrics fm(font());
        if (fm.horizontalAdvance(displayPath) > 350) {
            displayPath = fm.elidedText(displayPath, Qt::ElideMiddle, 350);
        }
        
        QLabel *pathLabel = new QLabel(displayPath);
        pathLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(Constants::Colors::TextMuted));
        pathLabel->setToolTip(path);  // Full path on hover
        
        layout->addLayout(nameLayout);
        layout->addWidget(pathLabel);
        
        QListWidgetItem *item = new QListWidgetItem(recentProjectsList);
        item->setSizeHint(QSize(0, 55));
        item->setData(Qt::UserRole, path);
        
        recentProjectsList->setItemWidget(item, itemWidget);
    }
}

void WelcomeWindow::clearRecentFiles()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "مسح الملفات الأخيرة",
        "هل تريد مسح قائمة الملفات الأخيرة؟",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        QSettings settings(Constants::OrgName, Constants::AppName);
        settings.remove(Constants::SettingsKeyRecentFiles);
        populateRecentProjects();
    }
}

bool WelcomeWindow::loadShowOnStartup()
{
    QSettings settings(Constants::OrgName, Constants::AppName);
    return settings.value("ShowWelcomeOnStartup", true).toBool();
}

void WelcomeWindow::saveShowOnStartup(bool show)
{
    QSettings settings(Constants::OrgName, Constants::AppName);
    settings.setValue("ShowWelcomeOnStartup", show);
}

// Slots
void WelcomeWindow::handleNewFileRequest()
{
    Qalam *editor = new Qalam();
    editor->show();
    this->close();
}

void WelcomeWindow::handleOpenFileRequest()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, 
        "فتح ملف",
        QDir::homePath(),
        "ملفات باء (*.baa *.baahd);;جميع الملفات (*.*)"
    );
    if (!filePath.isEmpty()) {
        Qalam *editor = new Qalam(filePath);
        editor->show();
        this->close();
    }
}

void WelcomeWindow::handleOpenFolderRequest()
{
    QString folderPath = QFileDialog::getExistingDirectory(
        this, 
        "فتح مجلد",
        QDir::homePath()
    );
    if (!folderPath.isEmpty()) {
        Qalam *editor = new Qalam();
        editor->loadFolder(folderPath);
        editor->show();
        this->close();
    }
}

void WelcomeWindow::handleCloneRepo()
{
    // Simple clone dialog
    bool ok;
    QString repoUrl = QInputDialog::getText(
        this,
        "استنساخ مستودع",
        "أدخل رابط المستودع:",
        QLineEdit::Normal,
        "https://github.com/",
        &ok
    );
    
    if (ok && !repoUrl.isEmpty()) {
        QString destPath = QFileDialog::getExistingDirectory(
            this,
            "اختر مجلد الوجهة",
            QDir::homePath()
        );
        
        if (!destPath.isEmpty()) {
            // TODO: Implement actual git clone functionality
            QMessageBox::information(this, "استنساخ", 
                QString("سيتم استنساخ:\n%1\nإلى:\n%2\n\n(هذه الميزة قيد التطوير)")
                .arg(repoUrl).arg(destPath));
        }
    }
}

void WelcomeWindow::onRecentFileClicked(QListWidgetItem *item)
{
    QString filePath = item->data(Qt::UserRole).toString();
    if (filePath.isEmpty()) return;
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, 
            "الملف غير موجود",
            QString("الملف التالي غير موجود:\n%1\n\nهل تريد إزالته من القائمة؟").arg(filePath),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes
        );
        
        if (reply == QMessageBox::Yes) {
            removeFromRecentFiles(filePath);
            populateRecentProjects();
        }
        return;
    }
    
    if (fileInfo.isDir()) {
        Qalam *editor = new Qalam();
        editor->loadFolder(filePath);
        editor->show();
    } else {
        Qalam *editor = new Qalam(filePath);
        editor->show();
    }
    this->close();
}

void WelcomeWindow::removeFromRecentFiles(const QString &filePath)
{
    QSettings settings(Constants::OrgName, Constants::AppName);
    QStringList recentFiles = settings.value(Constants::SettingsKeyRecentFiles).toStringList();
    recentFiles.removeAll(filePath);
    settings.setValue(Constants::SettingsKeyRecentFiles, recentFiles);
}

void WelcomeWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}
