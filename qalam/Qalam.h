#pragma once

#include "TEditor.h"
#include "TMenu.h"
#include "TSearchPanel.h"
#include "FileManager.h"
#include "BuildManager.h"
#include "SessionManager.h"
#include "LayoutManager.h"
#include "../ui/QalamWindow.h"

#include "TActivityBar.h"

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

    void goToLine();
    
    // VSCode-like component slots
    void onActivityViewChanged(TActivityBar::ViewType view);
    void onSidebarFileSelected(const QString &filePath);

private:
    TEditor *currentEditor();
    void connectSignals();
    void syncOpenEditors();

private:
    QTabWidget *tabWidget{};
    TMenuBar *menuBar{};
    TSettings *setting{};
    QString folderPath{};

    FileManager *m_fileManager{};
    BuildManager *m_buildManager{};
    SessionManager *m_sessionManager{};
    LayoutManager *m_layoutManager{};

    SearchPanel *searchBar{};
    TEditor *m_lastConnectedEditor{}; // Track editor for cursor position disconnect
};
