#pragma once

#include "TEditor.h"
#include "TMenu.h"
#include "TSearchPanel.h"
#include "ProcessWorker.h"
#include <QSplitter>
#include <QStatusBar>
#include <QPointer>
#include "../ui/QalamWindow.h"

// Forward declarations for new UI components
class TActivityBar;
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
    void newFile();
    void openFile(QString);
    void saveFile();
    void saveFileAs();
    void handleOpenFolderMenu();
    void openSettings();
    void exitApp();

    void runBaa();
    void aboutQalam();

    void updateWindowTitle();
    void onModificationChanged(bool modified);
    void onFileTreeDoubleClicked(const QModelIndex &index);
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
    
    // New component slots
    void onActivityViewChanged(int viewType);
    void onSidebarFileSelected(const QString &filePath);

private:
    int needSave();
    TEditor *currentEditor();
    void setupNewLayout();  // New method for VSCode-like layout

private:
    QTabWidget *tabWidget{};
    TMenuBar *menuBar{};
    TSettings *setting{};
    QAction *toggleSidebarAction{};
    QAction *runToolbarAction{};
    QAction *stopToolbarAction{};
    QString folderPath{};
    QAbstractItemModel *model{};

    QSplitter *mainSplitter{};
    QTreeView *fileTreeView{};
    QFileSystemModel *fileSystemModel{};

    QSplitter *editorSplitter{};
    // TConsole *console;
    QTabWidget *consoleTabWidget{};

    // TConsole *cmdConsole;
    // TConsole *alifConsole;
    QPointer<ProcessWorker> worker;
    QThread* buildThread = nullptr;

    QLabel *cursorPositionLabel{};
    QLabel *encodingLabel{};
    QProcess *alifProcess{};
    QProcess *currentBaaProcess{};
    SearchPanel *searchBar{};
    
    // New VSCode-like UI components
    TActivityBar *m_activityBar{};
    TSidebar *m_sidebar{};
    TStatusBar *m_statusBar{};
    TPanelArea *m_panelArea{};
    TBreadcrumb *m_breadcrumb{};
};
