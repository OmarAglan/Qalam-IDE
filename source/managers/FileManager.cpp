#include "FileManager.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QStringConverter>
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QTextDocument>
#include <QPushButton>
#include <QAbstractButton>

FileManager::FileManager(QTabWidget *tabWidget, QWidget *parentWindow, QObject *parent)
    : QObject(parent)
    , m_tabWidget(tabWidget)
    , m_parentWindow(parentWindow)
{
}

TEditor *FileManager::currentEditor() const
{
    return qobject_cast<TEditor*>(m_tabWidget->currentWidget());
}

QString FileManager::normalizePath(const QString &filePath) const
{
    if (filePath.trimmed().isEmpty()) return QString();

    QFileInfo info(filePath);
    const QString canonical = info.canonicalFilePath();
    if (!canonical.isEmpty()) return QDir::cleanPath(canonical);

    return QDir::cleanPath(info.absoluteFilePath());
}

TEditor *FileManager::createEditor(const QString &filePath)
{
    auto *editor = new TEditor(m_parentWindow);
    editor->setFilePath(filePath);

    connect(editor, &TEditor::openRequest, this, [this](const QString &requestedPath) {
        openFile(requestedPath);
    });
    connect(editor->document(), &QTextDocument::modificationChanged, this, [this]() {
        emit fileStateChanged();
        emit openEditorsChanged();
    });

    return editor;
}

void FileManager::addRecentFile(const QString &filePath)
{
    const QString normalizedPath = normalizePath(filePath);
    if (normalizedPath.isEmpty()) return;

    QSettings settings(Constants::OrgName, Constants::AppName);
    QStringList recentFiles = settings.value(Constants::SettingsKeyRecentFiles).toStringList();
    recentFiles.removeAll(normalizedPath);
    recentFiles.prepend(normalizedPath);
    while (recentFiles.size() > 10) {
        recentFiles.removeLast();
    }
    settings.setValue(Constants::SettingsKeyRecentFiles, recentFiles);
}

FileManager::SaveAction FileManager::needSave()
{
    return needSave(currentEditor());
}

FileManager::SaveAction FileManager::needSave(TEditor *editor)
{
    if (editor and editor->document()->isModified()) {
        const QString displayName = editor->currentFilePath().isEmpty()
            ? Constants::NewFileLabel
            : QFileInfo(editor->currentFilePath()).fileName();

        QMessageBox msgBox(m_parentWindow);
        msgBox.setWindowTitle("قلم");
        msgBox.setText(QString("تم تعديل الملف:\n%1\n\nهل تريد حفظ التغييرات؟").arg(displayName));
        QPushButton *saveButton = msgBox.addButton("حفظ", QMessageBox::AcceptRole);
        QPushButton *discardButton = msgBox.addButton("تجاهل", QMessageBox::DestructiveRole);
        QPushButton *cancelButton = msgBox.addButton("إلغاء", QMessageBox::RejectRole);
        msgBox.setDefaultButton(cancelButton);

        QFont msgFont = m_parentWindow ? m_parentWindow->font() : msgBox.font();
        msgFont.setPointSize(10);
        saveButton->setFont(msgFont);
        discardButton->setFont(msgFont);
        cancelButton->setFont(msgFont);

        msgBox.exec();

        QAbstractButton *clickedButton = msgBox.clickedButton();
        if (clickedButton == saveButton) {
            return SaveAction::Save;
        }
        if (clickedButton == discardButton) {
            return SaveAction::Discard;
        }
        return SaveAction::Cancel;
    }

    return SaveAction::Discard;
}

void FileManager::newFile()
{
    TEditor *newEditor = createEditor();
    const int index = m_tabWidget->addTab(newEditor, Constants::NewFileLabel);
    m_tabWidget->setCurrentIndex(index);

    emit fileStateChanged();
    emit openEditorsChanged();
}

void FileManager::openFile(QString filePath)
{
    if (filePath.isEmpty()) {
        filePath = QFileDialog::getOpenFileName(m_parentWindow, "فتح ملف", "",
                                                 "ملف باء (*.baa *.baahd);;Text Files (*.txt);;All Files (*)");
    }

    if (filePath.isEmpty()) return;

    const QString normalizedPath = normalizePath(filePath);
    if (normalizedPath.isEmpty()) return;

    // Check if file is already open in a tab
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        TEditor *editor = qobject_cast<TEditor*>(m_tabWidget->widget(i));
        if (editor and normalizePath(editor->currentFilePath()) == normalizedPath) {
            m_tabWidget->setCurrentIndex(i);
            return;
        }
    }

    // File size safety check
    constexpr qint64 WarnThreshold = 10 * 1024 * 1024;   // 10 MB
    constexpr qint64 RejectThreshold = 50 * 1024 * 1024;  // 50 MB
    QFileInfo fileCheck(normalizedPath);
    qint64 fileSize = fileCheck.size();

    if (fileSize > RejectThreshold) {
        QMessageBox::warning(m_parentWindow, "ملف كبير جداً",
            QString("حجم الملف (%1 MB) يتجاوز الحد المسموح (50 MB).\n"
                    "لا يمكن فتح هذا الملف.")
                .arg(fileSize / (1024 * 1024)));
        return;
    }

    if (fileSize > WarnThreshold) {
        auto reply = QMessageBox::question(m_parentWindow, "ملف كبير",
            QString("حجم الملف (%1 MB) كبير وقد يؤثر على الأداء.\n"
                    "هل تريد المتابعة؟")
                .arg(fileSize / (1024 * 1024)),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (reply != QMessageBox::Yes) return;
    }

    QFile file(normalizedPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(m_parentWindow, "خطأ", "لا يمكن فتح الملف");
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    file.close();

    TEditor *newEditor = createEditor(normalizedPath);
    newEditor->setPlainText(content);
    newEditor->document()->setModified(false);

    // Check for backup recovery
    const QString backupPath = normalizedPath + Constants::BackupExtension;
    if (QFile::exists(backupPath)) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::warning(m_parentWindow, "استعادة ملف",
                                     "يبدو أن البرنامج أُغلق بشكل غير متوقع.\n"
                                     "يوجد نسخة محفوظة تلقائيًا أحدث من الملف الأصلي.\n\n"
                                     "هل تريد استعادتها؟",
                                     QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            QFile backup(backupPath);
            if (backup.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream backupIn(&backup);
                backupIn.setEncoding(QStringConverter::Utf8);
                newEditor->setPlainText(backupIn.readAll());
                newEditor->document()->setModified(true);
                backup.close();
            }
        } else {
            QFile::remove(backupPath);
        }
    }

    QFileInfo fileInfo(normalizedPath);
    const int index = m_tabWidget->addTab(newEditor, fileInfo.fileName());
    m_tabWidget->setCurrentIndex(index);
    m_tabWidget->setTabToolTip(index, normalizedPath);

    addRecentFile(normalizedPath);

    emit fileStateChanged();
    emit openEditorsChanged();
}

bool FileManager::saveEditor(TEditor *editor)
{
    if (!editor) return false;

    QString filePath = editor->currentFilePath();
    QString content = editor->toPlainText();

    if (filePath.isEmpty()) {
        return saveEditorAs(editor);
    }

    filePath = normalizePath(filePath);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << content;
        file.close();

        editor->setFilePath(filePath);
        editor->document()->setModified(false);

        int index = m_tabWidget->indexOf(editor);
        if (index != -1) {
            QFileInfo fileInfo(filePath);
            m_tabWidget->setTabText(index, fileInfo.fileName());
            m_tabWidget->setTabToolTip(index, filePath);
        }

        editor->removeBackupFile();
        addRecentFile(filePath);
        emit fileStateChanged();
        emit openEditorsChanged();
        return true;
    }

    QMessageBox::warning(m_parentWindow, "خطأ", "لا يمكن حفظ الملف");
    return false;
}

void FileManager::saveFile()
{
    (void) saveEditor(currentEditor());
}

bool FileManager::saveEditorAs(TEditor *editor)
{
    if (!editor) return false;

    QString content = editor->toPlainText();
    QString currentPath = editor->currentFilePath();
    QString currentName = currentPath.isEmpty() ? "ملف جديد.baa" : QFileInfo(currentPath).fileName();
    QString fileName = QFileDialog::getSaveFileName(m_parentWindow, "حفظ الملف", currentName,
                                                     "ملف باء (*.baa);;مكتبة باء (*.baahd);;Text Files (*.txt);;All Files (*)");

    if (fileName.isEmpty()) return false;

    const QString normalizedPath = normalizePath(fileName);
    QFile file(normalizedPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << content;
        file.close();

        editor->setFilePath(normalizedPath);
        editor->document()->setModified(false);

        int index = m_tabWidget->indexOf(editor);
        if (index != -1) {
            QFileInfo fileInfo(normalizedPath);
            m_tabWidget->setTabText(index, fileInfo.fileName());
            m_tabWidget->setTabToolTip(index, normalizedPath);
        }

        editor->removeBackupFile();
        addRecentFile(normalizedPath);
        emit fileStateChanged();
        emit openEditorsChanged();
        return true;
    }

    QMessageBox::warning(m_parentWindow, "خطأ", "لا يمكن حفظ الملف");
    return false;
}

void FileManager::saveFileAs()
{
    (void) saveEditorAs(currentEditor());
}
