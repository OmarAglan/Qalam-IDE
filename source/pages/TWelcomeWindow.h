#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QVariant>

class QPushButton;
class QCheckBox;
class QLabel;

class WelcomeWindow : public QMainWindow
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
    QWidget* createActionLink(const QString &iconPath, const QString &text, void (WelcomeWindow::*slot)());
    QVBoxLayout* createStartColumn();
    QVBoxLayout* createRecentColumn();
    void populateRecentProjects();

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
