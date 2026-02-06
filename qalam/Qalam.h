#pragma once

#include "TEditor.h"
#include "TMenu.h"
#include "TSearchPanel.h"
#include "ProcessWorker.h"
#include <QPointer>
#include "../ui/QalamWindow.h"

// Forward declarations for UI components
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

    /// Return values for the save confirmation dialog
    enum class SaveAction {
        Cancel,   ///< User cancelled the operation
        Save,     ///< User chose to save
        Discard   ///< User chose to discard changes
    };

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
    void onActivityViewChanged(int viewType);
    void onSidebarFileSelected(const QString &filePath);

private:
    SaveAction needSave();
    TEditor *currentEditor();
    void setupNewLayout();
    void syncOpenEditors();

private:
    QTabWidget *tabWidget{};
    TMenuBar *menuBar{};
    TSettings *setting{};
    QString folderPath{};

    QPointer<ProcessWorker> worker;
    QThread* buildThread = nullptr;

    SearchPanel *searchBar{};
    TEditor *m_lastConnectedEditor{}; // Track editor for cursor position disconnect
    
    // VSCode-like UI components
    TActivityBar *m_activityBar{};
    TSidebar *m_sidebar{};
    TStatusBar *m_statusBar{};
    TPanelArea *m_panelArea{};
    TBreadcrumb *m_breadcrumb{};
};
