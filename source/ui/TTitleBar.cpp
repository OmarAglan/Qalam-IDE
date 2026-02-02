#include "TTitleBar.h"
#include "QalamTheme.h"
#include <QPainter>
#include <QStyleOption>

TTitleBar::TTitleBar(QWidget *parent) : QWidget(parent) {
    setFixedHeight(40); // Standard title bar height
    setupUi();
}

void TTitleBar::setupUi() {
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

    // Build layout correctly once (no delete/recreate)
    // RTL Layout: Elements added first appear on the right
    // We want: [Close][Max][Min] ....stretch.... [Title] [Logo] (visual left to right)
    // In RTL mode, addWidget adds from right to left visually
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(10);
    layout->setDirection(QBoxLayout::RightToLeft);
    
    // Add logo and title (will appear on the right in RTL)
    layout->addWidget(m_iconLabel);
    layout->addWidget(m_titleLabel);
    layout->addStretch();
    
    // Controls layout (no spacing between caption buttons)
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->setSpacing(0);
    controlsLayout->setContentsMargins(0, 0, 0, 0);
    // In RTL: Min, Max, Close order means Close will be leftmost
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
    
    // Insert menu after title (index 2: Icon=0, Title=1)
    // Layout is QHBoxLayout.
    // We access the layout() of this widget.
    if (QHBoxLayout *l = qobject_cast<QHBoxLayout*>(layout())) {
        l->insertWidget(2, menu);
    }
    
    // Style adjustments for embedded menu
    menu->setStyleSheet(menu->styleSheet() + "QMenuBar { background: transparent; border: none; }");
    menu->setFixedHeight(30);
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
