#pragma once

#include "TEditor.h"
#include "Constants.h"
#include <QTabWidget>

class FileManager : public QObject {
    Q_OBJECT

public:
    /// Return values for the save confirmation dialog
    enum class SaveAction {
        Cancel,   ///< User cancelled the operation
        Save,     ///< User chose to save
        Discard   ///< User chose to discard changes
    };

    explicit FileManager(QTabWidget *tabWidget, QWidget *parentWindow, QObject *parent = nullptr);

    /// Get the currently active editor from the tab widget
    TEditor *currentEditor() const;

    /// Show save confirmation dialog if the current document is modified
    SaveAction needSave();

public slots:
    void newFile();
    void openFile(QString filePath);
    void saveFile();
    void saveFileAs();

signals:
    /// Emitted after any file operation that changes window state
    void fileStateChanged();
    /// Emitted when the set of open editors changes (tab added/removed)
    void openEditorsChanged();

private:
    QTabWidget *m_tabWidget{};
    QWidget *m_parentWindow{};
};
