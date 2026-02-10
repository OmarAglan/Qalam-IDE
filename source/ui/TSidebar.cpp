#include "TSidebar.h"
#include "QalamTheme.h"
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
    m_sourceControlView = createPlaceholderView("قيد التطوير");
    m_extensionsView = createPlaceholderView("قيد التطوير");
    
    m_stackedWidget->addWidget(m_explorerView);  // Index 0
    m_stackedWidget->addWidget(m_searchView);    // Index 1
    m_stackedWidget->addWidget(m_sourceControlView); // Index 2
    m_stackedWidget->addWidget(m_extensionsView);    // Index 3
    
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
        case TActivityBar::ViewType::SourceControl:
            m_stackedWidget->setCurrentWidget(m_sourceControlView);
            break;
        case TActivityBar::ViewType::Extensions:
            m_stackedWidget->setCurrentWidget(m_extensionsView);
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
        case TActivityBar::ViewType::SourceControl:
            m_headerTitle->setText(Constants::SourceControlLabel.toUpper());
            break;
        case TActivityBar::ViewType::Extensions:
            m_headerTitle->setText(Constants::ExtensionsLabel.toUpper());
            break;
        default:
            break;
    }
}

QWidget* TSidebar::createPlaceholderView(const QString &title)
{
    auto *root = new QWidget(this);
    root->setObjectName("sidebarPlaceholder");
    auto *layout = new QVBoxLayout(root);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setAlignment(Qt::AlignTop);

    auto *label = new QLabel(title, root);
    label->setObjectName("sidebarPlaceholderLabel");
    label->setAlignment(Qt::AlignRight | Qt::AlignTop);
    label->setStyleSheet(QString("color: %1;").arg(Constants::Colors::TextMuted));

    layout->addWidget(label);
    layout->addStretch(1);
    return root;
}

void TSidebar::applyStyles()
{
    setStyleSheet(QalamTheme::sidebarStyleSheet());
}
