#include "TAutoSave.h"

#include <QPlainTextEdit>
#include <QFile>
#include <QTextStream>
#include "Constants.h"

TAutoSave::TAutoSave(QPlainTextEdit *editor, QObject *parent)
    : QObject(parent), m_editor(editor) {
    m_timer = new QTimer(this);
    m_timer->setInterval(Constants::Timing::AutoSaveInterval);
    connect(m_timer, &QTimer::timeout, this, &TAutoSave::performAutoSave);
}

void TAutoSave::start() {
    if (!m_timer->isActive()) {
        m_timer->start();
    }
}

void TAutoSave::stop() {
    m_timer->stop();
}

void TAutoSave::onContentChanged() {
    start();
}

void TAutoSave::performAutoSave() {
    if (filePath.isEmpty() or !m_editor->document()->isModified()) return;

    QString backupPath = filePath + ".~";

    QFile file(backupPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << m_editor->toPlainText();
        file.close();
    }
}

void TAutoSave::removeBackupFile() {
    if (filePath.isEmpty()) return;

    QString backupPath = filePath + ".~";
    if (QFile::exists(backupPath)) {
        QFile::remove(backupPath);
    }
    stop();
}
