#pragma once

#include "TEditor.h"
#include "TMenu.h"
#include "TSearchPanel.h"
#include "ProcessWorker.h"
#include <QSplitter>
#include <QStatusBar>
#include "../ui/QalamWindow.h"

// ...

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

private:
    int needSave();
    TEditor *currentEditor();

private:
    QTabWidget *tabWidget{};
    TMenuBar *menuBar{};
    TSettings *setting{};
    QAction *toggleSidebarAction{};
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
    ProcessWorker *worker{};
    QThread *buildThread{};

    QLabel *cursorPositionLabel{};
    QLabel *encodingLabel{};
    QProcess *alifProcess{};
    QProcess *currentBaaProcess{};
    SearchPanel *searchBar{};
};
