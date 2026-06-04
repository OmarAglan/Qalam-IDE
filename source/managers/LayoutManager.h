#pragma once

#include "TActivityBar.h"
#include <QObject>

class QTabWidget;
class QMainWindow;
class SearchPanel;
class TSidebar;
class TStatusBar;
class TPanelArea;
class TBreadcrumb;
class TEditor;

/**
 * @brief Manages the VSCode-like layout — creates components,
 *        assembles the widget tree, and handles panel/sidebar toggling.
 */
class LayoutManager : public QObject {
    Q_OBJECT

public:
    explicit LayoutManager(QMainWindow *window, QTabWidget *tabWidget,
                           SearchPanel *searchBar, QObject *parent = nullptr);

    /// Build and install the full layout onto the main window
    void setupLayout();

    // --- Component accessors ---
    TActivityBar *activityBar() const { return m_activityBar; }
    TSidebar *sidebar() const { return m_sidebar; }
    TStatusBar *statusBar() const { return m_statusBar; }
    TPanelArea *panelArea() const { return m_panelArea; }
    TBreadcrumb *breadcrumb() const { return m_breadcrumb; }

    // --- Layout actions ---

    /// Toggle the console/panel area visibility
    void toggleConsole(TEditor *currentEditor);

    /// Toggle sidebar visibility
    void toggleSidebar();

    /// Load a folder into the sidebar and update related components
    void loadFolder(const QString &path);

    /// Handle view changes from the activity bar
    void onActivityViewChanged(TActivityBar::ViewType view, const QString &folderPath);

signals:
    /// Emitted when the sidebar file explorer requests opening a folder
    void openFolderRequested();

private:
    QMainWindow *m_window{};
    QTabWidget *m_tabWidget{};
    SearchPanel *m_searchBar{};

    TActivityBar *m_activityBar{};
    TSidebar *m_sidebar{};
    TStatusBar *m_statusBar{};
    TPanelArea *m_panelArea{};
    TBreadcrumb *m_breadcrumb{};
};
