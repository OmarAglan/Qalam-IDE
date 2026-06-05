#pragma once

#include <QMenuBar>
#include <QFileSystemModel>
#include <QTreeView>
#include <QAction>
#include <QKeySequence>



class TMenuBar : public QMenuBar {

	Q_OBJECT
public:
    TMenuBar(QWidget* parent = nullptr);

    QAction* newAction;
    QAction* openFileAction;
    QAction* openFolderAction;
    QAction* saveAction;
    QAction* saveAsAction;
    QAction* SettingsAction;
    QAction* exitAction;
    QAction* commandPaletteAction;
    QAction* quickOpenAction;
    QAction* searchInFilesAction;
    QAction* toggleSidebarAction;
    QAction* toggleTerminalAction;
    QAction* runAction;
    QAction* aboutAction;

signals:
    void newRequested();
    void openFileRequested();
    void openFolderRequested();
    void saveRequested();
    void saveAsRequested();
    void settingsRequest();
    void exitRequested();
    void runRequested();
    void commandPaletteRequested();
    void quickOpenRequested();
    void searchInFilesRequested();
    void toggleSidebarRequested();
    void toggleTerminalRequested();
    void aboutRequested();

private slots:

    void onNewAction();
    void onOpenFileAction();
    void onOpenFolderAction();
    void onSaveAction();
    void onSaveAsAction();
    void onSettingsAction();
    void onExitApp();
    void onRunAction();
    void onCommandPaletteAction();
    void onQuickOpenAction();
    void onSearchInFilesAction();
    void onToggleSidebarAction();
    void onToggleTerminalAction();
    void onAboutAction();
};
