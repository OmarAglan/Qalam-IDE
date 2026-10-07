#include "QalamAutoSave.h"

#include <QPlainTextEdit>
#include <QFile>
#include <QSaveFile>
#include "Constants.h"

QalamAutoSave::QalamAutoSave(QPlainTextEdit *editor, QObject *parent)
    : QObject(parent), m_editor(editor) {
    m_timer = new QTimer(this);
    m_timer->setInterval(Constants::Timing::AutoSaveInterval);
    connect(m_timer, &QTimer::timeout, this, &QalamAutoSave::performAutoSave);
}

void QalamAutoSave::start() {
    if (!m_timer->isActive()) {
        m_timer->start();
    }
}

void QalamAutoSave::stop() {
    m_timer->stop();
}

void QalamAutoSave::onContentChanged() {
    start();
}

bool QalamAutoSave::performAutoSave() {
    if (filePath.isEmpty() or not m_editor->document()->isModified()) return true;

    const QString backupPath = filePath + Constants::BackupExtension;

    // QSaveFile never leaves a truncated backup behind: a half-written
    // recovery file is worse than the previous complete one.
    QSaveFile file(backupPath);
    const bool saved = file.open(QIODevice::WriteOnly | QIODevice::Text)
        and file.write(m_editor->toPlainText().toUtf8()) >= 0
        and file.commit();
    if (not saved) {
        if (m_failingPath != backupPath) {
            m_failingPath = backupPath;
            emit backupFailed(backupPath, file.errorString());
        }
        return false;
    }
    if (m_failingPath == backupPath) {
        m_failingPath.clear();
        emit backupRestored(backupPath);
    }
    return true;
}

void QalamAutoSave::removeBackupFile() {
    if (filePath.isEmpty()) return;

    QString backupPath = filePath + Constants::BackupExtension;
    if (QFile::exists(backupPath)) {
        QFile::remove(backupPath);
    }
    stop();
}
