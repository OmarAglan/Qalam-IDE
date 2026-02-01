#include "TTitleBar.h"
#include "QalamTheme.h"
#include <QPainter>
#include <QStyleOption>

TTitleBar::TTitleBar(QWidget *parent) : QWidget(parent) {
    setFixedHeight(40); // Standard title bar height
    setupUi();
}

void TTitleBar::setupUi() {
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 0, 0); // Left margin for icon
    layout->setSpacing(10);
    layout->setDirection(QBoxLayout::RightToLeft); // Arabic RTL standard

    // Logo
    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(24, 24);
    m_iconLabel->setPixmap(QPixmap(":/icons/resources/QalamLogo.ico").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    // Title
    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet("color: #cccccc; font-weight: bold; font-family: 'Segoe UI', 'Tajawal';");
    
    // Window Controls
    m_minimizeBtn = createCaptionButton(":/icons/resources/minimize.svg", "minimizeBtn");
    m_maximizeBtn = createCaptionButton(":/icons/resources/maximize.svg", "maximizeBtn");
    m_closeBtn = createCaptionButton(":/icons/resources/close.svg", "closeBtn");
    
    // Hover style for close button (red)
    m_closeBtn->setStyleSheet(m_closeBtn->styleSheet() + 
        "QPushButton#closeBtn:hover { background-color: #e81123; border: none; } "
        "QPushButton#closeBtn:pressed { background-color: #f1707a; border: none; }");

    connect(m_minimizeBtn, &QPushButton::clicked, this, &TTitleBar::minimizeClicked);
    connect(m_maximizeBtn, &QPushButton::clicked, this, &TTitleBar::maximizeRestoreClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &TTitleBar::closeClicked);

    // Layout
    layout->addWidget(m_minimizeBtn);
    layout->addWidget(m_maximizeBtn);
    layout->addWidget(m_closeBtn);
    layout->addSpacing(10);
    
    // In RTL: Controls are Far Left (added first)? 
    // Wait, Standard Window controls in Arabic Windows are on the Top-Left?
    // Yes, usually.
    // So if layout RTL:
    // add(min) -> Rightmost? No.
    // RTL Layout:
    // Element 0: Rightmost. Element N: Leftmost.
    // We want Close/Max/Min on the LEFT side (if mirroring standard Arabic UI).
    // Standard Windows (English): Controls Right.
    // Standard Windows (Arabic): Controls Left.
    // Layout RTL: addWidget adds from Right to Left.
    // So to put items on Left, we add them LAST or add a Stretch first.
    
    // Let's reset layout and do it carefully.
    // We want: [Controls] [Stretch] [Title] [Logo] (Visual Left to Right)
    // Or rather standard Arabic Header:
    // [Close][Max][Min] ........ [Title] [Logo]
    
    // If RTL Layout:
    // add(Logo) -> Rightmost.
    // add(Title) -> Right of Logo (wait, to the Left of Logo).
    // add(Stretch) -> Fills middle.
    // add(Min) -> Left.
    // add(Max) -> Left of Min.
    // add(Close) -> Leftmost.
    
    // Let's clear and re-add in logical connection order for RTL.
    // 1. Logo (Start/Right)
    // 2. Title
    // 3. Stretch
    // 4. Min
    // 5. Max
    // 6. Close (End/Left)
    
    // Rebuild layout
    delete layout;
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 0, 0); // Margins might need swap for RTL
    layout->setSpacing(10);
    layout->setDirection(QBoxLayout::RightToLeft);
    
    layout->addWidget(m_iconLabel);   // Rightmost
    layout->addWidget(m_titleLabel);
    layout->addStretch();
    
    // Controls
    // We want spacing 0 between caption buttons
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->setSpacing(0);
    controlsLayout->setContentsMargins(0,0,0,0);
    controlsLayout->setDirection(QBoxLayout::RightToLeft); // Min, Max, Close (Right to Left visually? No. Min is inner.)
    
    // Order: Min [Max] [Close] (Leftmost)
    // RTL add order:
    // add(Min) -> Right side of block
    // add(Max)
    // add(Close) -> Left side of block
    
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

QPushButton* TTitleBar::createCaptionButton(const QString &iconPath, const QString &objName) {
    QPushButton *btn = new QPushButton(this);
    btn->setObjectName(objName);
    btn->setFixedSize(46, 40); // Standard windows caption button width
    btn->setIcon(QIcon(iconPath));
    btn->setIconSize(QSize(16, 16));
    
    // Base style for buttons
    btn->setStyleSheet(
        "QPushButton { background: transparent; border: none; border-radius: 0px; }"
        "QPushButton:hover { background-color: #3e3e42; }"
        "QPushButton:pressed { background-color: #2d2d32; }"
    );
    
    return btn;
}
