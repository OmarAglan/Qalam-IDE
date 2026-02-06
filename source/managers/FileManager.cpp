#include "FileManager.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QSettings>
#include <QFileInfo>
#include <QDir>

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

FileManager::SaveAction FileManager::needSave()
{
    if (TEditor *editor = currentEditor()) {
        if (editor->document()->isModified()) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("قلم");
            msgBox.setText("تم التعديل على الملف.\n"
                           "هل تريد حفظ التغييرات؟");
            QPushButton *saveButton = msgBox.addButton("حفظ", QMessageBox::AcceptRole);
            QPushButton *discardButton = msgBox.addButton("تجاهل", QMessageBox::DestructiveRole);
            QPushButton *cancelButton = msgBox.addButton("إلغاء", QMessageBox::RejectRole);
            msgBox.setDefaultButton(cancelButton);

            QFont msgFont = m_parentWindow->font();
            msgFont.setPointSize(10);
            saveButton->setFont(msgFont);
            discardButton->setFont(msgFont);
            cancelButton->setFont(msgFont);

            msgBox.exec();

            QAbstractButton *clickedButton = msgBox.clickedButton();
            if (clickedButton == saveButton) {
                return SaveAction::Save;
            } else if (clickedButton == discardButton) {
                return SaveAction::Discard;
            } else if (clickedButton == cancelButton) {
                return SaveAction::Cancel;
            }
        }
    }

    return SaveAction::Discard;
}

void FileManager::newFile()
{
    TEditor *editor = currentEditor();
    if (editor) {
        SaveAction result = needSave();
        if (result == SaveAction::Cancel) return;
        if (result == SaveAction::Save) saveFile();
    }

    TEditor *newEditor = new TEditor(m_parentWindow);
    m_tabWidget->addTab(newEditor, Constants::NewFileLabel);
    m_tabWidget->setCurrentWidget(newEditor);

    connect(newEditor, &TEditor::openRequest, this, [this](QString filePath) {
        openFile(filePath);
    });
    connect(newEditor->document(), &QTextDocument::modificationChanged, this, [this]() {
        emit fileStateChanged();
    });

    emit fileStateChanged();
    emit openEditorsChanged();
}

void FileManager::openFile(QString filePath)
{
    if (currentEditor()) {
        SaveAction result = needSave();
        if (result == SaveAction::Cancel) return;
        if (result == SaveAction::Save) saveFile();
    }

    if (filePath.isEmpty()) {
        filePath = QFileDialog::getOpenFileName(m_parentWindow, "فتح ملف", "",
                                                 "ملف باء (*.baa *.baahd);;All Files (*)");
    }

    if (filePath.isEmpty()) return;

    // Check if file is already open in a tab
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        TEditor *editor = qobject_cast<TEditor*>(m_tabWidget->widget(i));
        if (editor and editor->property("filePath").toString() == filePath) {
            m_tabWidget->setCurrentIndex(i);
            return;
        }
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(m_parentWindow, "خطأ", "لا يمكن فتح الملف");
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    TEditor *newEditor = new TEditor(m_parentWindow);
    connect(newEditor->document(), &QTextDocument::modificationChanged, this, [this]() {
        emit fileStateChanged();
    });
    newEditor->setPlainText(content);
    newEditor->setProperty("filePath", filePath);

    // Check for backup recovery
    QString backupPath = filePath + ".~";
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
                newEditor->setPlainText(backupIn.readAll());
                newEditor->document()->setModified(true);
                backup.close();
            }
        } else {
            QFile::remove(backupPath);
        }
    }

    QFileInfo fileInfo(filePath);
    m_tabWidget->addTab(newEditor, fileInfo.fileName());
    m_tabWidget->setCurrentWidget(newEditor);
    m_tabWidget->setTabToolTip(m_tabWidget->currentIndex(), filePath);

    // Update recent files
    QSettings settings(Constants::OrgName, Constants::AppName);
    QStringList recentFiles = settings.value(Constants::SettingsKeyRecentFiles).toStringList();
    recentFiles.removeAll(filePath);
    recentFiles.prepend(filePath);
    while (recentFiles.size() > 10) {
        recentFiles.removeLast();
    }
    settings.setValue(Constants::SettingsKeyRecentFiles, recentFiles);

    emit fileStateChanged();
    emit openEditorsChanged();
}

void FileManager::saveFile()
{
    TEditor *editor = currentEditor();
    if (!editor) return;

    QString filePath = editor->property("filePath").toString();
    QString content = editor->toPlainText();

    if (filePath.isEmpty()) {
        saveFileAs();
        return;
    }

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&file);
        out << content;
        file.close();
        editor->document()->setModified(false);

        int index = m_tabWidget->indexOf(editor);
        if (index != -1) {
            QFileInfo fileInfo(filePath);
            m_tabWidget->setTabText(index, fileInfo.fileName());
        }
        editor->removeBackupFile();
        emit fileStateChanged();
    } else {
        QMessageBox::warning(m_parentWindow, "خطأ", "لا يمكن حفظ الملف");
    }
}

void FileManager::saveFileAs()
{
    TEditor *editor = currentEditor();
    if (!editor) return;

    QString content = editor->toPlainText();
    QString currentPath = editor->property("filePath").toString();
    QString currentName = currentPath.isEmpty() ? "ملف جديد.baa" : QFileInfo(currentPath).fileName();
    QString fileName = QFileDialog::getSaveFileName(m_parentWindow, "حفظ الملف", currentName,
                                                     "ملف باء (*.baa);;مكتبة باء(*.baahd);;All Files (*)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&file);
        out << content;
        file.close();

        editor->setProperty("filePath", fileName);
        editor->document()->setModified(false);

        int index = m_tabWidget->indexOf(editor);
        if (index != -1) {
            QFileInfo fileInfo(fileName);
            m_tabWidget->setTabText(index, fileInfo.fileName());
        }

        emit fileStateChanged();
    } else {
        QMessageBox::warning(m_parentWindow, "خطأ", "لا يمكن حفظ الملف");
    }
}
