#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include "TActivityBar.h"

class TExplorerView;
class TSearchView;

/**
 * @brief Sidebar container that shows different views based on Activity Bar selection
 * 
 * Contains a header with title and a stacked widget for different views:
 * - Explorer View (file tree)
 * - Search View (find in files)
 * - Settings will open a dialog, not shown in sidebar
 */
class TSidebar : public QWidget
{
    Q_OBJECT

public:
    explicit TSidebar(QWidget *parent = nullptr);
    ~TSidebar() = default;

    void setCurrentView(TActivityBar::ViewType view);
    TActivityBar::ViewType currentView() const { return m_currentView; }
    
    TExplorerView* explorerView() const { return m_explorerView; }
    TSearchView* searchView() const { return m_searchView; }

signals:
    void fileSelected(const QString &filePath);
    void searchRequested(const QString &query);
    void openFolderRequested();  // Forward from explorer view

private:
    void setupUi();
    void applyStyles();
    void updateHeader();
    QWidget* createPlaceholderView(const QString &title);

    TActivityBar::ViewType m_currentView = TActivityBar::ViewType::Explorer;
    
    QVBoxLayout *m_mainLayout = nullptr;
    QWidget *m_headerWidget = nullptr;
    QLabel *m_headerTitle = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;
    
    TExplorerView *m_explorerView = nullptr;
    TSearchView *m_searchView = nullptr;
    QWidget *m_sourceControlView = nullptr;
    QWidget *m_extensionsView = nullptr;
};
