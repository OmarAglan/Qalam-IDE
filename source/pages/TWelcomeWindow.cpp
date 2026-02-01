#include "TWelcomeWindow.h"
#include "Qalam.h"
#include <QtWidgets>
#include <QFontDatabase>

WelcomeWindow::WelcomeWindow(QWidget *parent)
    : QalamWindow(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setLayoutDirection(Qt::RightToLeft);  
    setWindowTitle("صفحة الترحيب - محرر قلم");

    // Main central widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 20);
    mainLayout->setSpacing(0);

    // 1. Header (Logo + Title)
    // ... (Keep existing layout logic, but remove hardcoded colors/styles where possible and rely on QalamTheme)
    
    // For now, keeping layout structure same.
    // QalamWindow handles the TitleBar.
    // We just provide content.

    QWidget *headerWidget = new QWidget();
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(40, 40, 40, 20);
    headerLayout->setDirection(QBoxLayout::RightToLeft); 

    QLabel *logoLabel = new QLabel();
    logoLabel->setPixmap(QPixmap(":/icons/resources/QalamLogo.ico").scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    QVBoxLayout *titleLayout = new QVBoxLayout();
    QLabel *titleLabel = new QLabel("محرر قلم");
    titleLabel->setObjectName("mainTitle");
    // Styling is handled by global stylesheet now? 
    // Yes, QalamTheme defines QLabel#mainTitle
    
    QLabel *subtitleLabel = new QLabel("محرر لغة باء");
    subtitleLabel->setObjectName("subtitle");
    
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);

    headerLayout->addLayout(titleLayout);
    headerLayout->addSpacing(20);
    headerLayout->addWidget(logoLabel);
    headerLayout->addStretch();
    
    mainLayout->addWidget(headerWidget);

    // 2. Content Area (Two Columns)
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(60, 20, 60, 20);
    contentLayout->setSpacing(60);
    // With setLayoutDirection(RTL), first added = Right side

    // Start Actions (will be on Right due to RTL)
    QVBoxLayout *startCol = createStartColumn();
    
    // Recent Files (will be on Left due to RTL)
    QVBoxLayout *recentCol = createRecentColumn();

    // RTL: first added goes Right
    contentLayout->addLayout(startCol, 0);  // Right side
    contentLayout->addLayout(recentCol, 1); // Left side

    mainLayout->addLayout(contentLayout, 1);

    // 3. Footer (Checkbox)
    showOnStartupCheck = new QCheckBox("إظهار صفحة الترحيب عند بدء البرنامج");
    showOnStartupCheck->setChecked(true);
    showOnStartupCheck->setDisabled(true); 
    
    QHBoxLayout *footerLayout = new QHBoxLayout();
    footerLayout->addStretch();
    footerLayout->addWidget(showOnStartupCheck);
    footerLayout->addSpacing(40); 
    
    mainLayout->addLayout(footerLayout);
    
    // Inherit QalamTheme styles (applied globally via main.cpp preferably, preventing local override)
    // Remove local setStyleSheet calls.
    
    // Geometry
    resize(1000, 700);
}

WelcomeWindow::~WelcomeWindow()
{
}

QVBoxLayout* WelcomeWindow::createStartColumn()
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignTop);
    
    QLabel *title = new QLabel("ابدأ");
    title->setObjectName("sectionTitle");
    title->setAlignment(Qt::AlignRight);  // RTL alignment
    layout->addWidget(title);
    layout->addSpacing(10);
    
    layout->addWidget(createActionLink(":/icons/resources/file-new.svg", "ملف جديد", &WelcomeWindow::handleNewFileRequest));
    layout->addWidget(createActionLink(":/icons/resources/folder-open.svg", "فتح ملف...", &WelcomeWindow::handleOpenFileRequest));
    layout->addWidget(createActionLink(":/icons/resources/folder.svg", "فتح مجلد...", &WelcomeWindow::handleOpenFolderRequest));
    layout->addWidget(createActionLink(":/icons/resources/git-clone.svg", "استنساخ مستودع...", &WelcomeWindow::handleCloneRepo));
    
    layout->addStretch();
    return layout;
}

QVBoxLayout* WelcomeWindow::createRecentColumn()
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignTop);
    
    QLabel *title = new QLabel("الأخيرة");
    title->setObjectName("sectionTitle");
    title->setAlignment(Qt::AlignRight);  // RTL alignment
    layout->addWidget(title);
    layout->addSpacing(10);
    
    recentProjectsList = new QListWidget();
    recentProjectsList->setFocusPolicy(Qt::NoFocus);
    recentProjectsList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    connect(recentProjectsList, &QListWidget::itemClicked, this, &WelcomeWindow::onRecentFileClicked);
    
    populateRecentProjects();
    
    layout->addWidget(recentProjectsList);
    return layout;
}

QWidget* WelcomeWindow::createActionLink(const QString &iconPath, const QString &text, void (WelcomeWindow::*slot)())
{
    QWidget *container = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 8, 0, 8);
    layout->setSpacing(10);
    // RTL: Icon on Right, Text on Left (icon added first = Right)
    
    QLabel *icon = new QLabel();
    icon->setPixmap(QIcon(iconPath).pixmap(20, 20));
    
    QLabel *label = new QLabel(text);
    label->setObjectName("actionLink");
    label->setCursor(Qt::PointingHandCursor);
    
    // RTL order: Icon (Right) -> Text (Left) -> Stretch (Far Left)
    layout->addWidget(icon);
    layout->addWidget(label);
    layout->addStretch();
    
    // Interactivity
    container->setCursor(Qt::PointingHandCursor);
    container->installEventFilter(this);
    
    // Store slot pointer securely? 
    // Method pointer storage in QVariant is non-standard. 
    // We'll map widgets to actions nicely or use Property.
    // However, pointer to member function is not QVariant compatible directly.
    // Hack: Store index or handle via unique mapping. 
    // Better: Connect a custom signal from this container or use a transparent button.
    
    // Simplest: Use an invisible button over it OR event filter click.
    // Let's use internal mapping logic for now via property name/ID if needed, 
    // OR just use a QToolButton styled to look like this.
    
    // Actually, making a ClickableWidget is cleaner, but let's stick to EventFilter for speed.
    // To identify WHICH slot, we can subclass or just hardcode for these 4 items?
    // Let's use a dynamic property with method name string, and invokeMethod?
    // No, invokeMethod only works for slots.
    
    // Revised: Just use a custom QToolButton with text and icon.
    // It is much easier and accessible.
    
    // But for the specific design (Link look), QToolButton needs heavily styling.
    // Let's go with the EventFilter and a simple map or dynamic property check.
    
    // I will use `container` pointer to map to slot.
    // But I can't store member function pointer easily.
    // Let's just create a small lambda connection if I made it a button.
    // Let's turn the container into a QToolButton actually for native click handling.
    // But QToolButton layout is rigid.
    
    // Reverting to EventFilter but handling click:
    // We need to know WHICH action.
    if (text == "ملف جديد") container->setProperty("action", "new");
    else if (text.contains("فتح ملف")) container->setProperty("action", "open_file");
    else if (text.contains("فتح مجلد")) container->setProperty("action", "open_folder");
    else if (text.contains("استنساخ")) container->setProperty("action", "clone");
    
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
    return QMainWindow::eventFilter(obj, event);
}

void WelcomeWindow::populateRecentProjects()
{
    QSettings settings("Baa", "Qalam");
    QStringList recentFiles = settings.value("RecentFiles").toStringList();
    
    recentProjectsList->clear();
    
    if (recentFiles.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem("لا توجد ملفات حديثة");
        item->setFlags(Qt::NoItemFlags);
        item->setForeground(QBrush(QColor("#666666")));
        recentProjectsList->addItem(item);
        return;
    }

    for (const QString &path : recentFiles) {
        QFileInfo info(path);
        
        // Custom widget for item (Name ... Path)
        QWidget *itemWidget = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(itemWidget);
        layout->setContentsMargins(5, 5, 5, 5);
        layout->setSpacing(2);
        
        QLabel *nameLabel = new QLabel(info.fileName());
        nameLabel->setStyleSheet("font-weight: bold; color: #4fc3f7; font-size: 14px;");
        
        QLabel *pathLabel = new QLabel(path);
        pathLabel->setStyleSheet("color: #888888; font-size: 12px;");
        
        layout->addWidget(nameLabel);
        layout->addWidget(pathLabel);
        
        QListWidgetItem *item = new QListWidgetItem(recentProjectsList);
        item->setSizeHint(itemWidget->sizeHint());
        item->setData(Qt::UserRole, path); // Store full path
        
        recentProjectsList->setItemWidget(item, itemWidget);
    }
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
    QString filePath = QFileDialog::getOpenFileName(this, "Open File");
    if (!filePath.isEmpty()) {
        Qalam *editor = new Qalam(filePath);
        editor->show();
        this->close();
    }
}

void WelcomeWindow::handleOpenFolderRequest()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, "Open Folder");
    if (!folderPath.isEmpty()) {
        Qalam *editor = new Qalam();
        editor->loadFolder(folderPath);
        editor->show();
        this->close();
    }
}

void WelcomeWindow::handleCloneRepo()
{
    QMessageBox::information(this, "استنساخ", "ميزة استنساخ المستودع قادمة قريباً!");
}

void WelcomeWindow::onRecentFileClicked(QListWidgetItem *item)
{
    QString filePath = item->data(Qt::UserRole).toString();
    if (filePath.isEmpty()) return;
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, "خطأ", "الملف غير موجود:\n" + filePath);
        return;
    }
    
    Qalam *editor = new Qalam(filePath);
    editor->show();
    this->close();
}

void WelcomeWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}
