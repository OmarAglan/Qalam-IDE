#include "TTitleBar.h"
#include "QalamTheme.h"
#include "Constants.h"
#include <QPainter>
#include <QStyleOption>
#include <QMenuBar>

TTitleBar::TTitleBar(QWidget *parent) : QWidget(parent) {
    setLayoutDirection(Qt::LeftToRight);
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

    // Title kept for accessibility/tooltips; the visible center is the command center.
    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("titleLabel");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->hide();

    // VS Code-like command center / command palette entry point.
    m_commandCenterBtn = new QPushButton("قلم  •  Ctrl+Shift+P", this);
    m_commandCenterBtn->setObjectName("commandCenterButton");
    m_commandCenterBtn->setCursor(Qt::PointingHandCursor);
    m_commandCenterBtn->setToolTip("لوحة الأوامر");
    m_commandCenterBtn->setFixedHeight(22);
    m_commandCenterBtn->setMinimumWidth(260);
    m_commandCenterBtn->setMaximumWidth(430);
    connect(m_commandCenterBtn, &QPushButton::clicked, this, &TTitleBar::commandCenterClicked);

    // Window Controls
    m_minimizeBtn = createCaptionButton(":/icons/resources/minimize.svg", "captionButton");
    m_maximizeBtn = createCaptionButton(":/icons/resources/maximize.svg", "captionButton");
    m_closeBtn = createCaptionButton(":/icons/resources/close.svg", "closeButton");

    connect(m_minimizeBtn, &QPushButton::clicked, this, &TTitleBar::minimizeClicked);
    connect(m_maximizeBtn, &QPushButton::clicked, this, &TTitleBar::maximizeRestoreClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &TTitleBar::closeClicked);

    // Build layout in LTR style (like VSCode even for RTL apps)
    // Layout: [Icon][Menu]...stretch...[Command Center]...[Min][Max][Close]

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 0, 0, 0);
    layout->setSpacing(0);
    layout->setDirection(QBoxLayout::LeftToRight);  // LTR for title bar

    // Left side: Logo
    layout->addWidget(m_iconLabel);
    layout->addSpacing(8);

    // Menu will be inserted at index 2 by addMenuBar()

    // Center: VS Code-like command center.
    layout->addStretch(1);
    layout->addWidget(m_commandCenterBtn, 0, Qt::AlignCenter);
    layout->addStretch(1);

    // Right side: Window controls (no spacing between caption buttons)
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->setSpacing(0);
    controlsLayout->setContentsMargins(0, 0, 0, 0);
    controlsLayout->addWidget(m_minimizeBtn);
    controlsLayout->addWidget(m_maximizeBtn);
    controlsLayout->addWidget(m_closeBtn);

    layout->addLayout(controlsLayout);

    setStyleSheet(QalamTheme::titleBarStyleSheet());
}

void TTitleBar::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), QalamTheme::instance().titleBarBackground());
}

void TTitleBar::setTitle(const QString &title) {
    m_titleLabel->setText(title);
    if (m_commandCenterBtn) {
        m_commandCenterBtn->setToolTip(QString("%1 — لوحة الأوامر").arg(title));
    }
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

    menu->setLayoutDirection(Qt::LeftToRight);

    // Insert menu after icon (index 2: Icon=0, Spacing=1, then insert at 2)
    if (QHBoxLayout *l = qobject_cast<QHBoxLayout*>(layout())) {
        l->insertWidget(2, menu);
    }

    menu->setFixedHeight(Constants::Layout::TitleBarHeight);

    // Keep the left menu compact so the command center remains visually centered.
    if (auto *menuBar = qobject_cast<QMenuBar*>(menu)) {
        menuBar->setMaximumWidth(360);
    }

    setStyleSheet(QalamTheme::titleBarStyleSheet());
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
