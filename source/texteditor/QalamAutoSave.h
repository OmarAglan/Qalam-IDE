#pragma once

#include <QObject>
#include <QTimer>

class QPlainTextEdit;

// Manages periodic auto-save of editor content to a backup file.
// Extracted from QalamEditor to isolate file-backup concerns.
class QalamAutoSave : public QObject {
    Q_OBJECT

public:
    explicit QalamAutoSave(QPlainTextEdit *editor, QObject *parent = nullptr);

    void start();
    void stop();
    void removeBackupFile();

    // The file path the editor is currently editing.
    // Must be kept in sync with QalamEditor::filePath.
    QString filePath{};

public slots:
    void onContentChanged();
    // Writes the backup now; returns false when it could not be written.
    bool performAutoSave();

signals:
    // Emitted once when backups start failing for a path, not on every
    // retry, so the user is told without being flooded.
    void backupFailed(const QString &backupPath, const QString &error);
    void backupRestored(const QString &backupPath);

private:
    QPlainTextEdit *m_editor{};
    QTimer *m_timer{};
    QString m_failingPath{};
};
