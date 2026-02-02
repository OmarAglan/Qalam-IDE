#include "TSidebar.h"
#include "../sidebar/TExplorerView.h"
#include "../sidebar/TSearchView.h"
#include "Constants.h"

TSidebar::TSidebar(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    applyStyles();
}

void TSidebar::setupUi()
{
    using namespace Constants;
    
    setMinimumWidth(Layout::SidebarMinWidth);
    setMaximumWidth(Layout::SidebarMaxWidth);
    setLayoutDirection(Qt::RightToLeft);  // RTL for Arabic
    
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // ========== Header ==========
    m_headerWidget = new QWidget();
    m_headerWidget->setObjectName("sidebarHeader");
    m_headerWidget->setFixedHeight(Layout::SidebarHeaderHeight);
    
    QHBoxLayout *headerLayout = new QHBoxLayout(m_headerWidget);
    headerLayout->setContentsMargins(10, 0, 15, 0);  // Swapped for RTL
    headerLayout->setSpacing(0);
    
    m_headerTitle = new QLabel(ExplorerLabel.toUpper());
    m_headerTitle->setObjectName("sidebarHeaderTitle");
    m_headerTitle->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    headerLayout->addStretch();
    headerLayout->addWidget(m_headerTitle);
    
    m_mainLayout->addWidget(m_headerWidget);
    
    // ========== Stacked Widget for Views ==========
    m_stackedWidget = new QStackedWidget();
    
    // Create views
    m_explorerView = new TExplorerView();
    m_searchView = new TSearchView();
    
    m_stackedWidget->addWidget(m_explorerView);  // Index 0
    m_stackedWidget->addWidget(m_searchView);    // Index 1
    
    m_mainLayout->addWidget(m_stackedWidget, 1);
    
    // Connect signals
    connect(m_explorerView, &TExplorerView::fileDoubleClicked, this, &TSidebar::fileSelected);
    connect(m_explorerView, &TExplorerView::openFolderRequested, this, &TSidebar::openFolderRequested);
    connect(m_searchView, &TSearchView::resultClicked, this, [this](const QString &path, int, int) {
        emit fileSelected(path);
    });
    
    // Default view
    setCurrentView(TActivityBar::ViewType::Explorer);
}

void TSidebar::setCurrentView(TActivityBar::ViewType view)
{
    m_currentView = view;
    updateHeader();
    
    switch (view) {
        case TActivityBar::ViewType::Explorer:
            m_stackedWidget->setCurrentWidget(m_explorerView);
            break;
        case TActivityBar::ViewType::Search:
            m_stackedWidget->setCurrentWidget(m_searchView);
            m_searchView->focusSearchInput();
            break;
        case TActivityBar::ViewType::Settings:
            // Settings opens a dialog, doesn't change sidebar
            break;
        default:
            break;
    }
}

void TSidebar::updateHeader()
{
    switch (m_currentView) {
        case TActivityBar::ViewType::Explorer:
            m_headerTitle->setText(Constants::ExplorerLabel.toUpper());
            break;
        case TActivityBar::ViewType::Search:
            m_headerTitle->setText(Constants::SearchLabel.toUpper());
            break;
        default:
            break;
    }
}

void TSidebar::applyStyles()
{
    using namespace Constants;
    
    QString styles = QString(R"(
        TSidebar {
            background-color: %1;
            border-left: 1px solid %2;
        }
        
        #sidebarHeader {
            background-color: %3;
            border-bottom: 1px solid %2;
        }
        
        #sidebarHeaderTitle {
            color: %4;
            font-size: %5px;
            font-weight: 600;
            letter-spacing: 0.5px;
            font-family: 'Segoe UI', 'Tajawal', sans-serif;
        }
    )")
    .arg(Colors::SidebarBackground)           // %1
    .arg(Colors::BorderSubtle)                // %2
    .arg(Colors::SidebarHeaderBackground)     // %3
    .arg(Colors::TextSecondary)               // %4
    .arg(Fonts::SectionHeaderSize);           // %5
    
    setStyleSheet(styles);
}
