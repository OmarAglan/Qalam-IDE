#pragma once

#include "TEditor.h"
#include "TMenu.h"
#include "TSearchPanel.h"
#include "FileManager.h"
#include "BuildManager.h"
#include "../ui/QalamWindow.h"

#include "TActivityBar.h"

// Forward declarations for UI components
class TSidebar;
class TStatusBar;
class TPanelArea;
class TBreadcrumb;

class Qalam : public QalamWindow
{
    Q_OBJECT

public:
    Qalam(const QString &filePath = "", QWidget *parent = nullptr);
    ~Qalam();
    void loadFolder(const QString &folderPath);

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *object, QEvent *event) override;

private slots:
    void handleOpenFolderMenu();
    void openSettings();
    void exitApp();

    void runBaa();
    void aboutQalam();

    void updateWindowTitle();
    void closeTab(int index);
    void toggleSidebar();

    void toggleConsole();

    void updateCursorPosition();
    void onCurrentTabChanged();

    void showFindBar();
    void hideFindBar();
    void findText();
    void findNextText();
    void findPrevText();

    void goToLine();
    
    // VSCode-like component slots
    void onActivityViewChanged(TActivityBar::ViewType view);
    void onSidebarFileSelected(const QString &filePath);

private:
    TEditor *currentEditor();
    void setupNewLayout();
    void connectSignals();
    void syncOpenEditors();

private:
    QTabWidget *tabWidget{};
    TMenuBar *menuBar{};
    TSettings *setting{};
    QString folderPath{};

    FileManager *m_fileManager{};
    BuildManager *m_buildManager{};

    SearchPanel *searchBar{};
    TEditor *m_lastConnectedEditor{}; // Track editor for cursor position disconnect
    
    // VSCode-like UI components
    TActivityBar *m_activityBar{};
    TSidebar *m_sidebar{};
    TStatusBar *m_statusBar{};
    TPanelArea *m_panelArea{};
    TBreadcrumb *m_breadcrumb{};
};
