#include "LayoutManager.h"

#include "TActivityBar.h"
#include "TSidebar.h"
#include "TStatusBar.h"
#include "TPanelArea.h"
#include "TBreadcrumb.h"
#include "TExplorerView.h"
#include "TConsole.h"
#include "TSearchPanel.h"
#include "TEditor.h"

#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStatusBar>
#include <QDir>

LayoutManager::LayoutManager(QMainWindow *window, QTabWidget *tabWidget,
                             SearchPanel *searchBar, QObject *parent)
    : QObject(parent)
    , m_window(window)
    , m_tabWidget(tabWidget)
    , m_searchBar(searchBar)
{
}

// ===================================================================
// Build and install the full VSCode-like layout onto the main window
// ===================================================================
void LayoutManager::setupLayout()
{
    // --- Create the UI components ---
    m_activityBar = new TActivityBar(m_window);
    m_sidebar     = new TSidebar(m_window);
    m_statusBar   = new TStatusBar(m_window);
    m_breadcrumb  = new TBreadcrumb(m_window);
    m_panelArea   = new TPanelArea(m_window);

    // =========================================================
    // Build the main layout - RTL for Arabic (Activity Bar on RIGHT)
    // =========================================================

    QWidget *centralContainer = new QWidget(m_window);
    centralContainer->setLayoutDirection(Qt::LeftToRight);

    QVBoxLayout *mainVLayout = new QVBoxLayout(centralContainer);
    mainVLayout->setContentsMargins(0, 0, 0, 0);
    mainVLayout->setSpacing(0);

    // Create horizontal layout for Editor + Sidebar + Activity Bar
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // Create editor + panel vertical splitter
    QSplitter *editorPanelSplitter = new QSplitter(Qt::Vertical);
    editorPanelSplitter->setHandleWidth(1);

    // Create editor area container with breadcrumb
    QWidget *editorContainer = new QWidget();
    QVBoxLayout *editorVLayout = new QVBoxLayout(editorContainer);
    editorVLayout->setContentsMargins(0, 0, 0, 0);
    editorVLayout->setSpacing(0);

    // Add breadcrumb above editor tabs
    editorVLayout->addWidget(m_breadcrumb);

    // Add tabs and search bar to editor container
    editorVLayout->addWidget(m_tabWidget, 1);
    editorVLayout->addWidget(m_searchBar);

    // Add editor container and panel to splitter
    editorPanelSplitter->addWidget(editorContainer);
    editorPanelSplitter->addWidget(m_panelArea);
    editorPanelSplitter->setSizes({700, 200});

    // Add widgets: Editor (left), Sidebar (middle-right), ActivityBar (far right)
    contentLayout->addWidget(editorPanelSplitter, 1);
    contentLayout->addWidget(m_sidebar);
    contentLayout->addWidget(m_activityBar);

    // Add content layout to main vertical layout
    mainVLayout->addLayout(contentLayout, 1);

    // Add status bar at bottom
    mainVLayout->addWidget(m_statusBar);

    // Set the central widget
    m_window->setCentralWidget(centralContainer);

    // --- Initial visibility ---
    m_activityBar->show();
    m_sidebar->hide();       // Start collapsed, like VSCode
    m_statusBar->show();
    m_breadcrumb->hide();    // Hide by default to match VS Code top layout
    m_panelArea->hide();     // Start collapsed, like VSCode

    // --- Set initial status bar values ---
    m_statusBar->setCursorPosition(1, 1);
    m_statusBar->setEncoding("UTF-8");
    m_statusBar->setLineEnding("LF");
    m_statusBar->setLanguage("Baa");
    m_statusBar->setFolderOpen(false);

    // Hide the old QMainWindow status bar
    QStatusBar *oldStatusBar = m_window->statusBar();
    if (oldStatusBar) {
        oldStatusBar->hide();
    }

    // Initialize the panel terminal
    if (m_panelArea->terminal()) {
        m_panelArea->terminal()->setConsoleRTL();
        m_panelArea->terminal()->startCmd();
    }
}

// ===================================================================
// Toggle the console/panel area visibility
// ===================================================================
void LayoutManager::toggleConsole(TEditor *currentEditor)
{
    if (!m_panelArea) return;

    bool isVisible = !m_panelArea->isVisible();
    m_panelArea->setVisible(isVisible);

    if (isVisible) {
        m_panelArea->setCurrentTab(TPanelArea::Tab::Terminal);
        if (m_panelArea->terminal()) {
            m_panelArea->terminal()->setFocus();
        }
    } else {
        if (currentEditor) {
            currentEditor->setFocus();
        }
    }
}

// ===================================================================
// Toggle sidebar visibility
// ===================================================================
void LayoutManager::toggleSidebar()
{
    if (!m_sidebar) return;

    bool shouldBeVisible = !m_sidebar->isVisible();
    m_sidebar->setVisible(shouldBeVisible);

    // Update activity bar state
    if (m_activityBar) {
        if (shouldBeVisible) {
            m_activityBar->setCurrentView(TActivityBar::ViewType::Explorer);
        } else {
            m_activityBar->setCurrentView(TActivityBar::ViewType::None);
        }
    }
}

// ===================================================================
// Load a folder into the sidebar and update related components
// ===================================================================
void LayoutManager::loadFolder(const QString &path)
{
    if (!path.isEmpty() and QDir(path).exists()) {
        // Update sidebar with folder path and show it
        if (m_sidebar and m_sidebar->explorerView()) {
            m_sidebar->explorerView()->setRootPath(path);
            m_sidebar->setCurrentView(TActivityBar::ViewType::Explorer);
            m_sidebar->show();
        }

        // Sync activity bar state
        if (m_activityBar) {
            m_activityBar->setCurrentView(TActivityBar::ViewType::Explorer);
        }

        // Update breadcrumb project root
        if (m_breadcrumb) {
            m_breadcrumb->setProjectRoot(path);
        }

        // Update status bar
        if (m_statusBar) {
            m_statusBar->setFolderOpen(true);
        }
    } else {
        if (m_statusBar) {
            m_statusBar->setFolderOpen(false);
        }
    }
}

// ===================================================================
// Handle view changes from the activity bar
// ===================================================================
void LayoutManager::onActivityViewChanged(TActivityBar::ViewType view,
                                          const QString &folderPath)
{
    m_sidebar->setCurrentView(view);
    m_sidebar->show();

    // Update the sidebar root path when switching to Explorer
    if (view == TActivityBar::ViewType::Explorer and !folderPath.isEmpty()) {
        m_sidebar->explorerView()->setRootPath(folderPath);
    }
}
