#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QTreeView>
#include <QFileSystemModel>
#include <QLabel>
#include <QPushButton>

/**
 * @brief Explorer View - File tree with collapsible sections
 * 
 * Shows:
 * - Open Editors section (collapsible)
 * - Folder tree section (collapsible)
 */
class TExplorerView : public QWidget
{
    Q_OBJECT

public:
    explicit TExplorerView(QWidget *parent = nullptr);
    ~TExplorerView() = default;

    void setRootPath(const QString &path);
    QString rootPath() const { return m_rootPath; }
    
    void addOpenEditor(const QString &filePath, bool modified = false);
    void removeOpenEditor(const QString &filePath);
    void updateOpenEditor(const QString &filePath, bool modified);
    void clearOpenEditors();

    QTreeView* treeView() const { return m_treeView; }
    QFileSystemModel* fileSystemModel() const { return m_fileSystemModel; }

signals:
    void fileDoubleClicked(const QString &filePath);
    void openEditorClicked(const QString &filePath);

private:
    void setupUi();
    void applyStyles();
    QWidget* createSectionHeader(const QString &title, bool expanded = true);
    
    QString m_rootPath;
    
    QVBoxLayout *m_mainLayout = nullptr;
    
    // Open Editors section
    QWidget *m_openEditorsHeader = nullptr;
    QWidget *m_openEditorsContent = nullptr;
    QVBoxLayout *m_openEditorsLayout = nullptr;
    bool m_openEditorsExpanded = true;
    
    // Folder section
    QWidget *m_folderHeader = nullptr;
    QLabel *m_folderNameLabel = nullptr;
    QTreeView *m_treeView = nullptr;
    QFileSystemModel *m_fileSystemModel = nullptr;
    bool m_folderExpanded = true;
    
    // No folder open state
    QWidget *m_noFolderWidget = nullptr;
};
