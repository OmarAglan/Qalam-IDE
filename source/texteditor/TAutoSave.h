#pragma once

#include <QObject>
#include <QTimer>

class QPlainTextEdit;

// Manages periodic auto-save of editor content to a backup file.
// Extracted from TEditor to isolate file-backup concerns.
class TAutoSave : public QObject {
    Q_OBJECT

public:
    explicit TAutoSave(QPlainTextEdit *editor, QObject *parent = nullptr);

    void start();
    void stop();
    void removeBackupFile();

    // The file path the editor is currently editing.
    // Must be kept in sync with TEditor::filePath.
    QString filePath{};

public slots:
    void onContentChanged();

private slots:
    void performAutoSave();

private:
    QPlainTextEdit *m_editor{};
    QTimer *m_timer{};
};
