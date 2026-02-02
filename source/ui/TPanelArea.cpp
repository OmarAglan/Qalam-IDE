#include "TPanelArea.h"
#include "QalamTheme.h"
#include "Constants.h"
#include "TConsole.h"
#include <QScrollArea>
#include <QTextEdit>

TPanelArea::TPanelArea(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    applyStyles();
}

void TPanelArea::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Header bar with tabs and controls
    m_mainLayout->addWidget(createHeaderBar());
    
    // Stacked widget for tab contents
    m_stackedWidget = new QStackedWidget(this);
    m_mainLayout->addWidget(m_stackedWidget, 1);
    
    // Setup individual views
    setupProblemsView();
    setupOutputView();
    setupTerminal();
    
    // Add to stacked widget
    m_stackedWidget->addWidget(m_problemsView);
    m_stackedWidget->addWidget(m_outputView);
    m_stackedWidget->addWidget(m_terminal);
    
    // Connect tab bar
    connect(m_tabBar, &QTabBar::currentChanged, this, [this](int index) {
        m_stackedWidget->setCurrentIndex(index);
        emit tabChanged(static_cast<Tab>(index));
    });
}

QWidget* TPanelArea::createHeaderBar()
{
    QWidget* header = new QWidget(this);
    header->setFixedHeight(Constants::Layout::PanelTabHeight);
    header->setObjectName("panelHeader");
    
    QHBoxLayout* layout = new QHBoxLayout(header);
    layout->setContentsMargins(0, 0, 4, 0);
    layout->setSpacing(0);
    layout->setDirection(QBoxLayout::RightToLeft);  // RTL
    
    // Tab bar on the right
    setupTabBar();
    layout->addWidget(m_tabBar);
    
    // Stretch in the middle
    layout->addStretch();
    
    // Control buttons on the left (in RTL, added last)
    m_maximizeBtn = new QPushButton(this);
    m_maximizeBtn->setFixedSize(20, 20);
    m_maximizeBtn->setText("□");
    m_maximizeBtn->setCursor(Qt::PointingHandCursor);
    m_maximizeBtn->setToolTip("تكبير");
    connect(m_maximizeBtn, &QPushButton::clicked, this, &TPanelArea::maximizeRequested);
    layout->addWidget(m_maximizeBtn);
    
    m_closeBtn = new QPushButton(this);
    m_closeBtn->setFixedSize(20, 20);
    m_closeBtn->setText("×");
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    m_closeBtn->setToolTip("إغلاق");
    connect(m_closeBtn, &QPushButton::clicked, this, &TPanelArea::closeRequested);
    layout->addWidget(m_closeBtn);
    
    return header;
}

void TPanelArea::setupTabBar()
{
    m_tabBar = new QTabBar(this);
    m_tabBar->setLayoutDirection(Qt::RightToLeft);
    m_tabBar->setDocumentMode(true);
    m_tabBar->setExpanding(false);
    
    // Add tabs - RTL order (rightmost first)
    m_tabBar->addTab(Constants::ProblemsLabel);
    m_tabBar->addTab(Constants::OutputLabel);
    m_tabBar->addTab(Constants::TerminalLabel);
    
    // Create problems badge
    m_problemsBadge = new QLabel(this);
    m_problemsBadge->setFixedSize(18, 14);
    m_problemsBadge->setAlignment(Qt::AlignCenter);
    m_problemsBadge->hide();
}

void TPanelArea::setupProblemsView()
{
    m_problemsView = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(m_problemsView);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    // Scroll area for problems
    QScrollArea* scroll = new QScrollArea(m_problemsView);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setFrameShape(QFrame::NoFrame);
    
    m_problemsContent = new QWidget(scroll);
    m_problemsLayout = new QVBoxLayout(m_problemsContent);
    m_problemsLayout->setContentsMargins(8, 4, 8, 4);
    m_problemsLayout->setSpacing(2);
    m_problemsLayout->setAlignment(Qt::AlignTop);
    
    scroll->setWidget(m_problemsContent);
    layout->addWidget(scroll);
}

void TPanelArea::setupOutputView()
{
    m_outputView = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(m_outputView);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QScrollArea* scroll = new QScrollArea(m_outputView);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    
    m_outputLabel = new QLabel(scroll);
    m_outputLabel->setAlignment(Qt::AlignTop | Qt::AlignRight);
    m_outputLabel->setWordWrap(true);
    m_outputLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_outputLabel->setStyleSheet(QString("QLabel { padding: 8px; color: %1; }")
                                 .arg(Constants::Colors::TextSecondary));
    
    scroll->setWidget(m_outputLabel);
    layout->addWidget(scroll);
}

void TPanelArea::setupTerminal()
{
    m_terminal = new TConsole(this);
}

void TPanelArea::setCurrentTab(Tab tab)
{
    m_tabBar->setCurrentIndex(static_cast<int>(tab));
}

TPanelArea::Tab TPanelArea::currentTab() const
{
    return static_cast<Tab>(m_tabBar->currentIndex());
}

void TPanelArea::addProblem(const QString& message, const QString& file, 
                            int line, int column, const QString& severity)
{
    QWidget* item = new QWidget(m_problemsContent);
    QHBoxLayout* layout = new QHBoxLayout(item);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(8);
    layout->setDirection(QBoxLayout::RightToLeft);
    
    // Severity icon
    QLabel* icon = new QLabel(item);
    QString iconColor;
    QString iconText;
    if (severity == "error") {
        iconColor = Constants::Colors::ErrorForeground;
        iconText = "✕";
        m_errorCount++;
    } else if (severity == "warning") {
        iconColor = Constants::Colors::WarningForeground;
        iconText = "⚠";
        m_warningCount++;
    } else {
        iconColor = Constants::Colors::InfoForeground;
        iconText = "ℹ";
    }
    icon->setText(iconText);
    icon->setStyleSheet(QString("color: %1;").arg(iconColor));
    layout->addWidget(icon);
    
    // Message
    QLabel* msgLabel = new QLabel(message, item);
    msgLabel->setStyleSheet(QString("color: %1;").arg(Constants::Colors::TextPrimary));
    layout->addWidget(msgLabel, 1);
    
    // File location
    QString location = QString("%1:%2:%3").arg(file).arg(line).arg(column);
    QLabel* locLabel = new QLabel(location, item);
    locLabel->setStyleSheet(QString("color: %1;").arg(Constants::Colors::TextMuted));
    layout->addWidget(locLabel);
    
    // Make clickable
    item->setCursor(Qt::PointingHandCursor);
    item->setProperty("file", file);
    item->setProperty("line", line);
    item->setProperty("column", column);
    
    item->installEventFilter(this);
    
    m_problemsLayout->addWidget(item);
    updateProblemsBadge();
}

void TPanelArea::clearProblems()
{
    while (m_problemsLayout->count() > 0) {
        QLayoutItem* item = m_problemsLayout->takeAt(0);
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
    m_errorCount = 0;
    m_warningCount = 0;
    updateProblemsBadge();
}

int TPanelArea::problemCount() const
{
    return m_errorCount + m_warningCount;
}

void TPanelArea::updateProblemsBadge()
{
    int count = problemCount();
    if (count > 0) {
        m_problemsBadge->setText(QString::number(count > 99 ? 99 : count));
        m_problemsBadge->setStyleSheet(QString(R"(
            QLabel {
                background: %1;
                color: white;
                border-radius: 7px;
                font-size: 10px;
                font-weight: bold;
            }
        )").arg(m_errorCount > 0 ? Constants::Colors::ErrorForeground 
                                 : Constants::Colors::WarningForeground));
        m_problemsBadge->show();
    } else {
        m_problemsBadge->hide();
    }
}

void TPanelArea::appendOutput(const QString& text)
{
    m_outputBuffer += text;
    m_outputLabel->setText(m_outputBuffer);
}

void TPanelArea::clearOutput()
{
    m_outputBuffer.clear();
    m_outputLabel->clear();
}

void TPanelArea::setCollapsed(bool collapsed)
{
    if (m_collapsed == collapsed) {
        return;
    }
    m_collapsed = collapsed;
    m_stackedWidget->setVisible(!collapsed);
    emit collapseToggled(collapsed);
}

void TPanelArea::applyStyles()
{
    setStyleSheet(QString(R"(
        TPanelArea {
            background-color: %1;
            border-top: 1px solid %2;
        }
        
        #panelHeader {
            background-color: %1;
            border-bottom: 1px solid %2;
        }
        
        QTabBar::tab {
            background: transparent;
            color: %3;
            padding: 8px 12px;
            border: none;
            border-bottom: 2px solid transparent;
        }
        
        QTabBar::tab:hover {
            color: %4;
        }
        
        QTabBar::tab:selected {
            color: %4;
            border-bottom: 2px solid %5;
        }
        
        QPushButton {
            background: transparent;
            border: none;
            color: %3;
            font-size: 14px;
        }
        
        QPushButton:hover {
            color: %4;
            background: %6;
        }
        
        QScrollArea {
            background: %1;
            border: none;
        }
        
        QScrollArea > QWidget > QWidget {
            background: %1;
        }
    )").arg(Constants::Colors::PanelBackground)
      .arg(Constants::Colors::PanelBorder)
      .arg(Constants::Colors::TextMuted)
      .arg(Constants::Colors::TextPrimary)
      .arg(Constants::Colors::ActivityIndicator)
      .arg(Constants::Colors::ListHoverBackground));
}
