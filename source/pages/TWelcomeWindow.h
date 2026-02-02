#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QVariant>

#include "../ui/QalamWindow.h"

class QPushButton;
class QCheckBox;
class QLabel;
class QVBoxLayout;
class QWidget;

class WelcomeWindow : public QalamWindow
{
    Q_OBJECT

public:
    WelcomeWindow(QWidget *parent = nullptr);
    ~WelcomeWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    // UI Creation Helpers
    void applyStyles();
    QWidget* createActionLink(const QString &iconPath, const QString &text, const QString &actionId);
    QVBoxLayout* createStartColumn();
    QVBoxLayout* createRecentColumn();
    void populateRecentProjects();
    
    // Settings helpers
    bool loadShowOnStartup();
    void saveShowOnStartup(bool show);
    void clearRecentFiles();
    void removeFromRecentFiles(const QString &filePath);

    // UI Elements
    QListWidget *recentProjectsList;
    QCheckBox *showOnStartupCheck;

private slots:
    void handleNewFileRequest();
    void handleOpenFileRequest();
    void handleOpenFolderRequest();
    void handleCloneRepo();
    void onRecentFileClicked(QListWidgetItem *item);
};
