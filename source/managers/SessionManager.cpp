#include "SessionManager.h"
#include "TExplorerView.h"

#include <QSettings>
#include <QFileInfo>

SessionManager::SessionManager(QTabWidget *tabWidget, QObject *parent)
    : QObject(parent)
    , m_tabWidget(tabWidget)
{
}

void SessionManager::saveSession(const QString &folderPath, const QByteArray &windowGeometry)
{
    QSettings settings(Constants::OrgName, Constants::AppName);

    // Collect file paths of all open tabs
    QStringList openFiles;
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        TEditor *editor = qobject_cast<TEditor*>(m_tabWidget->widget(i));
        if (editor) {
            QString filePath = editor->property("filePath").toString();
            if (not filePath.isEmpty()) {
                openFiles.append(filePath);
            }
        }
    }

    settings.setValue(Constants::SessionKeyOpenFiles, openFiles);
    settings.setValue(Constants::SessionKeyActiveTab, m_tabWidget->currentIndex());
    settings.setValue(Constants::SessionKeyFolderPath, folderPath);
    settings.setValue(Constants::SessionKeyWindowGeometry, windowGeometry);
    settings.sync();
}

SessionManager::SessionData SessionManager::restoreSession() const
{
    QSettings settings(Constants::OrgName, Constants::AppName);

    SessionData data;
    data.openFiles = settings.value(Constants::SessionKeyOpenFiles).toStringList();
    data.activeTabIndex = settings.value(Constants::SessionKeyActiveTab, -1).toInt();
    data.folderPath = settings.value(Constants::SessionKeyFolderPath).toString();
    data.windowGeometry = settings.value(Constants::SessionKeyWindowGeometry).toByteArray();
    return data;
}

void SessionManager::savePreferences(TEditor *editor, int themeIndex)
{
    if (not editor) return;

    QSettings settings(Constants::OrgName, Constants::AppName);
    settings.setValue(Constants::SettingsKeyFontSize, editor->font().pixelSize());
    settings.setValue(Constants::SettingsKeyFontType, editor->font().family());
    settings.setValue(Constants::SettingsKeyTheme, themeIndex);
    settings.sync();
}

void SessionManager::syncOpenEditors(TExplorerView *explorerView)
{
    if (not explorerView) return;

    explorerView->clearOpenEditors();

    for (int i = 0; i < m_tabWidget->count(); ++i) {
        TEditor *editor = qobject_cast<TEditor*>(m_tabWidget->widget(i));
        if (editor) {
            QString filePath = editor->property("filePath").toString();
            bool modified = editor->document()->isModified();

            // Use tab text if no file path (unsaved file)
            if (filePath.isEmpty()) {
                filePath = m_tabWidget->tabText(i);
            }
            explorerView->addOpenEditor(filePath, modified);
        }
    }
}
