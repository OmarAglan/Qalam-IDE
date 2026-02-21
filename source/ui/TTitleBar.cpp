#include "TTitleBar.h"
#include "QalamTheme.h"
#include "Constants.h"
#include <QPainter>
#include <QStyleOption>

TTitleBar::TTitleBar(QWidget *parent) : QWidget(parent) {
    setFixedHeight(Constants::Layout::TitleBarHeight);
    setupUi();
}

void TTitleBar::setupUi() {
    using namespace Constants;
    
    // Logo
    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(Layout::IconSize, Layout::IconSize);
    m_iconLabel->setPixmap(QPixmap(":/icons/resources/QalamLogo.ico").scaled(
        Layout::IconSize, Layout::IconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    // Title
    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("titleLabel");
    
    // Window Controls
    m_minimizeBtn = createCaptionButton(":/icons/resources/minimize.svg", "captionButton");
    m_maximizeBtn = createCaptionButton(":/icons/resources/maximize.svg", "captionButton");
    m_closeBtn = createCaptionButton(":/icons/resources/close.svg", "closeButton");

    connect(m_minimizeBtn, &QPushButton::clicked, this, &TTitleBar::minimizeClicked);
    connect(m_maximizeBtn, &QPushButton::clicked, this, &TTitleBar::maximizeRestoreClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &TTitleBar::closeClicked);

    // Build layout in LTR style (like VSCode even for RTL apps)
    // Layout: [Icon][Menu]...stretch...[Title]...[Min][Max][Close]
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 0, 0, 0);
    layout->setSpacing(0);
    layout->setDirection(QBoxLayout::LeftToRight);  // LTR for title bar
    
    // Left side: Logo
    layout->addWidget(m_iconLabel);
    layout->addSpacing(8);
    
    // Menu will be inserted at index 2 by addMenuBar()
    
    // Center: stretch + title + stretch (title centered)
    layout->addStretch(1);
    layout->addWidget(m_titleLabel);
    layout->addStretch(1);
    
    // Right side: Window controls (no spacing between caption buttons)
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->setSpacing(0);
    controlsLayout->setContentsMargins(0, 0, 0, 0);
    controlsLayout->addWidget(m_minimizeBtn);
    controlsLayout->addWidget(m_maximizeBtn);
    controlsLayout->addWidget(m_closeBtn);
    
    layout->addLayout(controlsLayout);
}

void TTitleBar::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), QalamTheme::instance().titleBarBackground());
}

void TTitleBar::setTitle(const QString &title) {
    m_titleLabel->setText(title);
}

void TTitleBar::setMaximizedState(bool maximized) {
    if (maximized) {
        m_maximizeBtn->setToolTip("restore");
        m_maximizeBtn->setIcon(QIcon(":/icons/resources/restore.svg"));
    } else {
        m_maximizeBtn->setToolTip("maximize");
        m_maximizeBtn->setIcon(QIcon(":/icons/resources/maximize.svg"));
    }
}

void TTitleBar::addMenuBar(QWidget *menu) {
    if (!menu) return;
    
    // Insert menu after icon (index 2: Icon=0, Spacing=1, then insert at 2)
    if (QHBoxLayout *l = qobject_cast<QHBoxLayout*>(layout())) {
        l->insertWidget(2, menu);
    }

    menu->setFixedHeight(Constants::Layout::TitleBarHeight);
}

QPushButton* TTitleBar::createCaptionButton(const QString &iconPath, const QString &objName) {
    using namespace Constants;
    
    QPushButton *btn = new QPushButton(this);
    btn->setObjectName(objName);
    btn->setFixedSize(Layout::CaptionButtonWidth, Layout::CaptionButtonHeight);
    btn->setIcon(QIcon(iconPath));
    btn->setIconSize(QSize(Layout::CaptionIconSize, Layout::CaptionIconSize));
    
    return btn;
}
