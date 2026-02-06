#include "Qalam.h"
#include "TWelcomeWindow.h"
#include "TConsole.h"
#include "ProcessWorker.h"
#include "TSearchPanel.h"
#include "Constants.h"

// VSCode-like UI components
#include "TActivityBar.h"
#include "TSidebar.h"
#include "TStatusBar.h"
#include "TPanelArea.h"
#include "TBreadcrumb.h"
#include "TExplorerView.h"
#include "TSearchView.h"

#include <QThread>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QShortcut>
#include <QGuiApplication>
#include <QScreen>
#include <QCoreApplication>
#include <QTextStream>
#include <QApplication>
#include <QHeaderView>
#include <QSettings>
#include <QProcess>
#include <QKeyEvent>
#include <QTimer>
#include <QInputDialog>

Qalam::Qalam(const QString& filePath, QWidget *parent)
    : QalamWindow(parent)
{

    setAttribute(Qt::WA_DeleteOnClose);

    // ===================================================================
    // الخطوة 1: إنشاء المكونات الرئيسية
    // ===================================================================
    tabWidget = new QTabWidget(this);
    tabWidget->setObjectName("MainTabs");
    tabWidget->setDocumentMode(true);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    menuBar = new TMenuBar(this);

    searchBar = new SearchPanel(this);
    searchBar->hide();

    QShortcut *findShortcut = new QShortcut(QKeySequence::Find, this);
    connect(findShortcut, &QShortcut::activated, this, &Qalam::showFindBar);

    // ===================================================================
    // الخطوة 2: إعداد النافذة وشريط القوائم
    // ===================================================================
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeo = screen->availableGeometry();
    int margin = 100;
    int widthFixedNum = 6;
    int x = screenGeo.right() - screenGeo.size().width() + margin * widthFixedNum / 2;
    int y = screenGeo.top() + 30 + margin / 2; // 30 is top system bar height
    int width = screenGeo.size().width() - margin * widthFixedNum;
    int height = screenGeo.size().height() - margin;
    this->setGeometry(x, y, width, height);
    this->setCustomMenuBar(menuBar);

    // ===================================================================
    // الخطوة 3: إعداد الإعدادات
    // ===================================================================
    setting = new TSettings();

    // ===================================================================
    // الخطوة 4: ربط الإشارات والمقابس
    // ===================================================================
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &Qalam::closeTab);
    QShortcut* saveShortcut = new QShortcut(QKeySequence::Save, this);
    connect(saveShortcut, &QShortcut::activated, this, &Qalam::saveFile);
    connect(menuBar, &TMenuBar::newRequested, this, &Qalam::newFile);
    connect(menuBar, &TMenuBar::openFileRequested, this, [this](){this->openFile("");});
    connect(menuBar, &TMenuBar::saveRequested, this, &Qalam::saveFile);
    connect(menuBar, &TMenuBar::saveAsRequested, this, &Qalam::saveFileAs);
    connect(menuBar, &TMenuBar::settingsRequest, this, &Qalam::openSettings);
    connect(menuBar, &TMenuBar::exitRequested, this, &Qalam::exitApp);
    connect(menuBar, &TMenuBar::runRequested, this, &Qalam::runBaa);
    connect(menuBar, &TMenuBar::aboutRequested, this, &Qalam::aboutQalam);
    connect(menuBar, &TMenuBar::openFolderRequested, this, &Qalam::handleOpenFolderMenu);
    connect(tabWidget, &QTabWidget::currentChanged, this, &Qalam::updateWindowTitle);
    connect(tabWidget, &QTabWidget::currentChanged, this, &Qalam::onCurrentTabChanged);
    connect(searchBar, &SearchPanel::findNext, this, &Qalam::findNextText);
    connect(searchBar, &SearchPanel::findText, this, &Qalam::findText);
    connect(searchBar, &SearchPanel::findPrevious, this, &Qalam::findPrevText);
    connect(searchBar, &SearchPanel::closed, this, &Qalam::hideFindBar);
    onCurrentTabChanged();

    QShortcut *goToLineShortcut = new QShortcut(QKeySequence("Ctrl+G"), this);
    connect(goToLineShortcut, &QShortcut::activated, this, &Qalam::goToLine);

    QShortcut *commentShortcut = new QShortcut(QKeySequence("Ctrl+/"), this);
    connect(commentShortcut, &QShortcut::activated, this, [this](){
        if (TEditor* editor = currentEditor()) editor->toggleComment();
    });

    QShortcut *duplicateShortcut = new QShortcut(QKeySequence("Ctrl+D"), this);
    connect(duplicateShortcut, &QShortcut::activated, this, [this](){
        if (TEditor* editor = currentEditor()) editor->duplicateLine();
    });

    QShortcut *moveUpShortcut = new QShortcut(QKeySequence("Alt+Up"), this);
    connect(moveUpShortcut, &QShortcut::activated, this, [this](){
        if (TEditor* editor = currentEditor()) editor->moveLineUp();
    });

    QShortcut *moveDownShortcut = new QShortcut(QKeySequence("Alt+Down"), this);
    connect(moveDownShortcut, &QShortcut::activated, this, [this](){
        if (TEditor* editor = currentEditor()) editor->moveLineDown();
    });

    // ===================================================================
    // الخطوة 5: إعداد التخطيط الجديد (VSCode-like)
    // ===================================================================
    setupNewLayout();
    
    // ===================================================================
    // الخطوة 6: تحميل الملف المبدئي أو إنشاء تبويب جديد
    // ===================================================================
    installEventFilter(this);

    if (!filePath.isEmpty()) {
        openFile(filePath);
    } else {
        newFile();
    }
}

Qalam::~Qalam() {

    if (TEditor* editor = currentEditor()) {
        QSettings settings(Constants::OrgName, Constants::AppName);
        settings.setValue(Constants::SettingsKeyFontSize, editor->font().pixelSize());
        settings.setValue(Constants::SettingsKeyFontType, editor->font().family());
        settings.setValue(Constants::SettingsKeyTheme, setting->getThemeCombo()->currentIndex());
        settings.sync();
    }
}

void Qalam::closeEvent(QCloseEvent *event) {
    SaveAction saveResult = needSave();

    if (saveResult == SaveAction::Save) {
        saveFile();
    } else if (saveResult == SaveAction::Cancel) {
        event->ignore();
        return;
    }

    event->accept();
}

bool Qalam::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_F6) {
            toggleConsole();
            return true;
        }
    }
    return QMainWindow::eventFilter(object, event);
}

void Qalam::goToLine()
{
    TEditor *editor = currentEditor();
    if (!editor) return;

    bool ok;
    int maxLine = editor->blockCount();

    int lineNumber = QInputDialog::getInt(this, "الذهاب إلى سطر",
                                          QString("أدخل رقم السطر (1 - %1):").arg(maxLine),
                                          1, 1, maxLine, 1, &ok);

    if (ok) {
        QTextCursor cursor = editor->textCursor();
        cursor.setPosition(0);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1);
        editor->setTextCursor(cursor);
        editor->centerCursor();
        editor->setFocus();
    }
}

void Qalam::showFindBar() {
    searchBar->show();
    searchBar->setFocusToInput();
}

void Qalam::hideFindBar() {
    searchBar->hide();
    if (TEditor* editor = currentEditor()) {
        editor->setFocus();
    }
}

void Qalam::findText() {
    TEditor* editor = currentEditor();
    if (!editor) return;

    QString text = searchBar->getText();
    if (text.isEmpty()) return;

    QTextDocument::FindFlags flags;
    if (searchBar->isCaseSensitive()) flags |= QTextDocument::FindCaseSensitively;

    editor->moveCursor(QTextCursor::Start);
    bool found = editor->find(text, flags);

    if (!found) {
        QApplication::beep();
    }
}

void Qalam::findNextText() {
    TEditor* editor = currentEditor();
    if (!editor) return;

    QString text = searchBar->getText();
    if (text.isEmpty()) return;

    QTextDocument::FindFlags flags;
    if (searchBar->isCaseSensitive()) flags |= QTextDocument::FindCaseSensitively;

    bool found = editor->find(text, flags);

    if (!found) {
        editor->moveCursor(QTextCursor::Start);
        found = editor->find(text, flags);
        if (!found) {
            QApplication::beep();
        }
    }
}

void Qalam::findPrevText() {
    TEditor* editor = currentEditor();
    if (!editor) return;

    QString text = searchBar->getText();
    if (text.isEmpty()) return;

    QTextDocument::FindFlags flags = QTextDocument::FindBackward;
    if (searchBar->isCaseSensitive()) flags |= QTextDocument::FindCaseSensitively;

    bool found = editor->find(text, flags);

    if (!found) {
        editor->moveCursor(QTextCursor::End);
        found = editor->find(text, flags);
        if (!found) QApplication::beep();
    }
}

void Qalam::toggleConsole()
{
    if (!m_panelArea) return;

    bool isVisible = !m_panelArea->isVisible();
    m_panelArea->setVisible(isVisible);

    if (isVisible) {
        m_panelArea->setCurrentTab(TPanelArea::Tab::Terminal);
        if (m_panelArea->terminal()) {
            m_panelArea->terminal()->setFocus();
        }
    } else {
        if (TEditor* editor = currentEditor()) {
            editor->setFocus();
        }
    }
}

/* ----------------------------------- File Menu Button ----------------------------------- */

Qalam::SaveAction Qalam::needSave() {
    if (TEditor* editor = currentEditor()) {
        if (editor->document()->isModified()) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("قلم");
            msgBox.setText("تم التعديل على الملف.\n"    \
                           "هل تريد حفظ التغييرات؟");
            QPushButton *saveButton = msgBox.addButton("حفظ", QMessageBox::AcceptRole);
            QPushButton *discardButton = msgBox.addButton("تجاهل", QMessageBox::DestructiveRole);
            QPushButton *cancelButton = msgBox.addButton("إلغاء", QMessageBox::RejectRole);
            msgBox.setDefaultButton(cancelButton);

            QFont msgFont = this->font();
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

void Qalam::newFile() {

    TEditor* editor = currentEditor();
    if (editor) {
        SaveAction result = needSave();
        if (result == SaveAction::Cancel) return;
        if (result == SaveAction::Save) this->saveFile();
    }

    TEditor *newEditor = new TEditor(this);
    tabWidget->addTab(newEditor, Constants::NewFileLabel);
    tabWidget->setCurrentWidget(newEditor);

    connect(newEditor, &TEditor::openRequest, this, [this](QString filePath){this->openFile(filePath);});
    connect(newEditor->document(), &QTextDocument::modificationChanged, this, &Qalam::onModificationChanged);
    updateWindowTitle();
    syncOpenEditors();
}

void Qalam::openFile(QString filePath) {
    if (currentEditor()) {
        SaveAction result = needSave();
        if (result == SaveAction::Cancel) return;
        if (result == SaveAction::Save) this->saveFile();
    }

    if (filePath.isEmpty()) {
        filePath = QFileDialog::getOpenFileName(this, "فتح ملف", "", "ملف باء (*.baa *.baahd);;All Files (*)");
    }

    if (!filePath.isEmpty()) {
        for (int i = 0; i < tabWidget->count(); ++i) {
            TEditor* editor = qobject_cast<TEditor*>(tabWidget->widget(i));
            if (editor && editor->property("filePath").toString() == filePath) {
                tabWidget->setCurrentIndex(i);
                return;
            }
        }

        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString content = in.readAll();
            file.close();

            TEditor *newEditor = new TEditor(this);
            connect(newEditor->document(), &QTextDocument::modificationChanged, this, &Qalam::onModificationChanged);
            newEditor->setPlainText(content);
            newEditor->setProperty("filePath", filePath);


            QString backupPath = filePath + ".~";
            if (QFile::exists(backupPath)) {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::warning(this, "استعادة ملف",
                                             "يبدو أن البرنامج أُغلق بشكل غير متوقع.\n"
                                             "يوجد نسخة محفوظة تلقائيًا أحدث من الملف الأصلي.\n\n"
                                             "هل تريد استعادتها؟",
                                             QMessageBox::Yes | QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                    QFile backup(backupPath);
                    if (backup.open(QIODevice::ReadOnly | QIODevice::Text)) {
                        QTextStream in(&backup);
                        newEditor->setPlainText(in.readAll());
                        newEditor->document()->setModified(true);
                        backup.close();
                    }
                } else {
                    QFile::remove(backupPath);
                }
            }

            connect(newEditor, &QPlainTextEdit::cursorPositionChanged, this, &Qalam::updateCursorPosition);

            QFileInfo fileInfo(filePath);
            tabWidget->addTab(newEditor, fileInfo.fileName());
            tabWidget->setCurrentWidget(newEditor);
            tabWidget->setTabToolTip(tabWidget->currentIndex(), filePath);
            updateWindowTitle();
            syncOpenEditors();


            QSettings settings(Constants::OrgName, Constants::AppName);
            QStringList recentFiles = settings.value(Constants::SettingsKeyRecentFiles).toStringList();
            recentFiles.removeAll(filePath);
            recentFiles.prepend(filePath);
            while (recentFiles.size() > 10) {
                recentFiles.removeLast();
            }
            settings.setValue(Constants::SettingsKeyRecentFiles, recentFiles);
        } else {
            QMessageBox::warning(this, "خطأ", "لا يمكن فتح الملف");
        }
    }
}

void Qalam::loadFolder(const QString &path)
{
    this->folderPath = path;
    
    if (!path.isEmpty() && QDir(path).exists()) {
        // Update sidebar with folder path and show it
        if (m_sidebar && m_sidebar->explorerView()) {
            m_sidebar->explorerView()->setRootPath(path);
            m_sidebar->setCurrentView(TActivityBar::ViewType::Explorer);
            m_sidebar->show();
        }
        
        // Sync activity bar state
        if (m_activityBar) {
            m_activityBar->setCurrentView(TActivityBar::ViewType::Explorer);
        }
        
        // Update breadcrumb project root
        if (m_breadcrumb) {
            m_breadcrumb->setProjectRoot(path);
        }
        
        // Update status bar
        if (m_statusBar) {
            m_statusBar->setFolderOpen(true);
        }
    } else {
        if (m_statusBar) {
            m_statusBar->setFolderOpen(false);
        }
    }
}

void Qalam::handleOpenFolderMenu()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, "اختر مجلد", QDir::homePath());
    if (folderPath.isEmpty()) return;

    loadFolder(folderPath);
}

void Qalam::toggleSidebar()
{
    if (!m_sidebar) return;

    bool shouldBeVisible = !m_sidebar->isVisible();
    m_sidebar->setVisible(shouldBeVisible);
    
    // Update activity bar state
    if (m_activityBar) {
        if (shouldBeVisible) {
            m_activityBar->setCurrentView(TActivityBar::ViewType::Explorer);
        } else {
            m_activityBar->setCurrentView(TActivityBar::ViewType::None);
        }
    }
}

void Qalam::saveFile() {
    TEditor *editor = currentEditor();
    if (!editor) return;

    QString filePath = editor->property("filePath").toString();

    QString content = editor->toPlainText();

    if (filePath.isEmpty()) {
        saveFileAs();
        return;
    } else {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&file);
            out << content;
            file.close();
            editor->document()->setModified(false);

            int index = tabWidget->indexOf(editor);
            if (index != -1) {
                QFileInfo fileInfo(filePath);
                tabWidget->setTabText(index, fileInfo.fileName());
            }
            editor->removeBackupFile();
            updateWindowTitle();
            return ;
        } else {
            QMessageBox::warning(this, "خطأ", "لا يمكن حفظ الملف");
            return;
        }
    }
}

void Qalam::saveFileAs() {
    TEditor *editor = currentEditor();
    if (!editor) return ;

    QString content = editor->toPlainText();
    QString currentPath = editor->property("filePath").toString();
    QString currentName = currentPath.isEmpty() ? "ملف جديد.baa" : QFileInfo(currentPath).fileName();
    QString fileName = QFileDialog::getSaveFileName(this, "حفظ الملف", currentName, "ملف باء (*.baa);;مكتبة باء(*.baahd);;All Files (*)");

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&file);
            out << content;
            file.close();

            editor->setProperty("filePath", fileName);

            editor->document()->setModified(false);

            int index = tabWidget->indexOf(editor);
            if (index != -1) {
                QFileInfo fileInfo(fileName);
                tabWidget->setTabText(index, fileInfo.fileName());
            }

            updateWindowTitle();
            return ;
        } else {
            QMessageBox::warning(this, "خطأ", "لا يمكن حفظ الملف");
            return ;
        }
    }
    return ;
}

void Qalam::openSettings() {
    if (setting and setting->isVisible()) return;
 
    connect(setting, &TSettings::fontSizeChanged, this, [this](int size){
        for (int i = 0; i < tabWidget->count(); ++i) {
            TEditor* editor = qobject_cast<TEditor*>(tabWidget->widget(i));
            if (editor) editor->updateFontSize(size);
        }
    }, Qt::UniqueConnection);
    connect(setting, &TSettings::fontTypeChanged, this, [this](QString font){
        for (int i = 0; i < tabWidget->count(); ++i) {
            TEditor* editor = qobject_cast<TEditor*>(tabWidget->widget(i));
            if (editor) editor->updateFontType(font);
        }
    }, Qt::UniqueConnection);
    connect(setting, &TSettings::highlighterThemeChanged, this, [this](int themeIdx){
        auto theme = ThemeManager::getThemeByIndex(themeIdx);
        for (int i = 0; i < tabWidget->count(); ++i) {
            TEditor* editor = qobject_cast<TEditor*>(tabWidget->widget(i));
            if (editor) editor->updateHighlighterTheme(theme);
        }
    }, Qt::UniqueConnection);
 
    setting->show();
}


void Qalam::exitApp() {
    SaveAction result = needSave();
    if (result == SaveAction::Cancel) {
        return;
    }
    else if (result == SaveAction::Save) {
        this->saveFile();
        return;
    }

    WelcomeWindow *welcome = new WelcomeWindow();
    welcome->show();
    this->close();
}

void Qalam::onCurrentTabChanged()
{
    updateWindowTitle();
    updateCursorPosition();

    TEditor* editor = currentEditor();

    // Disconnect the previous editor to avoid accumulating connections
    if (m_lastConnectedEditor) {
        disconnect(m_lastConnectedEditor, &QPlainTextEdit::cursorPositionChanged, this, &Qalam::updateCursorPosition);
    }

    if (editor) {
        connect(editor, &QPlainTextEdit::cursorPositionChanged, this, &Qalam::updateCursorPosition);
        m_lastConnectedEditor = editor;
    }
}

void Qalam::updateCursorPosition()
{
    TEditor* editor = currentEditor();
    if (editor) {
        const QTextCursor cursor = editor->textCursor();
        int line = cursor.blockNumber() + 1;
        int column = cursor.columnNumber() + 1;

        // Update status bar
        if (m_statusBar) {
            m_statusBar->setCursorPosition(line, column);
        }
        
        // Update breadcrumb with current file
        if (m_breadcrumb) {
            QString filePath = editor->property("filePath").toString();
            m_breadcrumb->setFilePath(filePath);
        }
    }
}


/* ----------------------------------- Run Menu Button ----------------------------------- */

void Qalam::runBaa() {
    TEditor *editor = currentEditor();
    if (!editor) return;

    QString filePath = editor->property("filePath").toString();
    if (filePath.isEmpty() || editor->document()->isModified()) {
        QMessageBox::warning(this, "تنبيه", "يجب حفظ الملف قبل التشغيل.");
        saveFile();
        filePath = editor->property("filePath").toString();
        if (filePath.isEmpty() || editor->document()->isModified()) return;
    }

    // Use TPanelArea's terminal
    if (!m_panelArea) return;

    TConsole *console = m_panelArea->terminal();
    m_panelArea->setCurrentTab(TPanelArea::Tab::Output);
    m_panelArea->show();
    m_panelArea->setCollapsed(false);

    // Dynamic Compiler Path Logic
    QSettings settings(Constants::OrgName, Constants::AppName);
    QString program = settings.value(Constants::SettingsKeyCompilerPath).toString();

    if (program.isEmpty()) {
        QString appDir = QCoreApplication::applicationDirPath();
#if defined(Q_OS_WIN)
        QString localBaa = QDir(appDir).filePath("baa/baa.exe");
        if (QFile::exists(localBaa)) {
            program = localBaa;
        } else {
            program = "baa/baa.exe"; // Fallback to PATH or relative
        }
#elif defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
        program = QDir(appDir).filePath("baa/baa");
#endif
    }

    if (!QFile::exists(program)) {
        console->clear();
        console->appendPlainTextThreadSafe("❌ خطأ: لم يتم العثور على مترجم باء!");
        console->appendPlainTextThreadSafe("المسار المتوقع: " + program);

#if defined(Q_OS_LINUX)
        console->appendPlainTextThreadSafe("تأكد من أن ملف 'alif' موجود ولديه صلاحية التنفيذ (chmod +x).");
#endif
        return;
    }

    QStringList args = { filePath };
    QString workingDir = QFileInfo(filePath).absolutePath();

    // Safely clean up existing thread/worker before creating new ones
    if (buildThread) {
        if (worker) {
            worker->stop();
            worker = nullptr;
        }
        buildThread->quit();
        if (!buildThread->wait(3000)) {
            buildThread->terminate();
            buildThread->wait();
        }
        if (buildThread) {
            buildThread->deleteLater();
            buildThread = nullptr;
        }
    }

    console->clear();
    console->appendPlainTextThreadSafe("🚀 بدء تشغيل ملف باء...");
    console->appendPlainTextThreadSafe("📄 الملف: " + QFileInfo(filePath).fileName());

    worker = new ProcessWorker(program, args, workingDir);
    buildThread = new QThread(this);

    worker->moveToThread(buildThread);
 
    connect(buildThread, &QThread::started, worker, &ProcessWorker::start);

    connect(worker, &ProcessWorker::outputReady,
            console, &TConsole::appendPlainTextThreadSafe);
    connect(worker, &ProcessWorker::errorReady,
            console, &TConsole::appendPlainTextThreadSafe);
 
    connect(worker, &ProcessWorker::finished, this, [this, console](int code){
        console->appendPlainTextThreadSafe(
            "\n──────────────────────────────\n✅ انتهى التنفيذ (Exit code = "
            + QString::number(code) + ")\n"
            );
        buildThread->quit();
    });

    // Cleanup logic: Ensure thread and worker are deleted after the thread finishes
    connect(buildThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(buildThread, &QThread::finished, buildThread, &QObject::deleteLater);

    connect(console, &TConsole::commandEntered,
            worker, &ProcessWorker::sendInput);

    buildThread->start();
}

//----------------

TEditor* Qalam::currentEditor() {
    return qobject_cast<TEditor*>(tabWidget->currentWidget());
}

void Qalam::closeTab(int index)
{

    if (tabWidget->count() <= 1) {
        return;
    }

    QWidget *tab = tabWidget->widget(index);

    if (!tab) return;

    TEditor* editor = qobject_cast<TEditor*>(tabWidget->widget(index));
    if (!editor) return;

    if (editor && editor->document()->isModified()) {
        SaveAction saveResult = needSave();

        if (saveResult == SaveAction::Cancel) {
            return;
        }
        else if (saveResult == SaveAction::Save) {
            this->saveFile();
            return;
        }

    }
    tabWidget->removeTab(index);
    syncOpenEditors();
}

/* ----------------------------------- Help Menu Button ----------------------------------- */

void Qalam::aboutQalam() {
    QMessageBox messageDialog{};
    messageDialog.setWindowTitle("عن محرر قلم");
    messageDialog.setText(R"(
        محرر قلم (Qalam IDE)

        بيئة تطوير مبنية بـ Qt و C++ موجهة لدعم البرمجة ذات الصياغة العربية.

        • يدعم اتجاه الكتابة من اليمين إلى اليسار (RTL)
        • تلوين شيفرة (Syntax Highlighting) ومحرك ثيمات
        • إكمال تلقائي (Auto-complete) وحفظ تلقائي واستعادة النسخ الاحتياطية

        ملاحظة:
        المشروع كان يُعرف سابقاً باسم "طيف" ويتم حالياً تحديث الهوية والواجهة.

        © Qalam IDE
                                    )"
                          );

    messageDialog.setStyleSheet("background: #03091A; color: white");

    messageDialog.exec();
}

/* ----------------------------------- Other Functions ----------------------------------- */

void Qalam::updateWindowTitle() {
    TEditor* editor = currentEditor();
    QString title{};

    if (!editor) {
        title = "قلم";
    } else {
        QString filePath = editor->property("filePath").toString();

        if (filePath.isEmpty()) {
            title = "غير معنون";
        } else {
            title = QFileInfo(filePath).fileName();
        }
        if (editor->document()->isModified()) {
            title += "[*]";
        }
        title += " - قلم";
    }
    setWindowTitle(title);
    setWindowModified(editor && editor->document()->isModified());
}

void Qalam::onModificationChanged(bool modified) {
    updateWindowTitle();
    TEditor* editor = currentEditor();
    if (editor) {
        int index = tabWidget->indexOf(editor);
        if (index != -1) {
            QString currentText = tabWidget->tabText(index);
            if (modified && !currentText.endsWith("[*]")) {
                tabWidget->setTabText(index, currentText + "[*]");
            } else if (!modified && currentText.endsWith("[*]")) {
                tabWidget->setTabText(index, currentText.left(currentText.length() - 3));
            }
        }
    }
}

// ===================================================================
// VSCode-like Layout Methods
// ===================================================================

void Qalam::setupNewLayout()
{
    // Create the UI components
    m_activityBar = new TActivityBar(this);
    m_sidebar = new TSidebar(this);
    m_statusBar = new TStatusBar(this);
    m_breadcrumb = new TBreadcrumb(this);
    m_panelArea = new TPanelArea(this);
    
    // Connect Activity Bar signals
    connect(m_activityBar, &TActivityBar::viewChanged, this, [this](TActivityBar::ViewType view) {
        onActivityViewChanged(static_cast<int>(view));
    });
    
    connect(m_activityBar, &TActivityBar::viewToggled, this, [this](TActivityBar::ViewType view, bool visible) {
        if (view == TActivityBar::ViewType::Settings) {
            openSettings();
            return;
        }
        if (!visible) {
            m_sidebar->hide();
        } else {
            m_sidebar->show();
            m_sidebar->setCurrentView(view);
        }
    });
    
    // Connect Sidebar signals
    connect(m_sidebar, &TSidebar::fileSelected, this, &Qalam::onSidebarFileSelected);
    connect(m_sidebar, &TSidebar::openFolderRequested, this, &Qalam::handleOpenFolderMenu);
    
    // Connect Status Bar signals
    connect(m_statusBar, &TStatusBar::problemsClicked, this, [this]() {
        m_panelArea->setCurrentTab(TPanelArea::Tab::Problems);
        m_panelArea->show();
    });
    
    // Connect Panel Area signals
    connect(m_panelArea, &TPanelArea::closeRequested, this, [this]() {
        m_panelArea->hide();
    });
    
    connect(m_panelArea, &TPanelArea::tabChanged, this, [this](TPanelArea::Tab tab) {
        if (tab == TPanelArea::Tab::Terminal && m_panelArea->terminal()) {
            m_panelArea->terminal()->setFocus();
        }
    });
    
    // =========================================================
    // Build the main layout - RTL for Arabic (Activity Bar on RIGHT)
    // =========================================================
    
    QWidget *centralContainer = new QWidget(this);
    centralContainer->setLayoutDirection(Qt::LeftToRight);
    
    QVBoxLayout *mainVLayout = new QVBoxLayout(centralContainer);
    mainVLayout->setContentsMargins(0, 0, 0, 0);
    mainVLayout->setSpacing(0);
    
    // Create horizontal layout for Editor + Sidebar + Activity Bar
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    
    // Create editor + panel vertical splitter
    QSplitter *editorPanelSplitter = new QSplitter(Qt::Vertical);
    editorPanelSplitter->setHandleWidth(1);
    editorPanelSplitter->setStyleSheet("QSplitter::handle { background: #007acc; }");
    
    // Create editor area container with breadcrumb
    QWidget *editorContainer = new QWidget();
    QVBoxLayout *editorVLayout = new QVBoxLayout(editorContainer);
    editorVLayout->setContentsMargins(0, 0, 0, 0);
    editorVLayout->setSpacing(0);
    
    // Add breadcrumb above editor tabs
    editorVLayout->addWidget(m_breadcrumb);
    
    // Add tabs and search bar to editor container
    editorVLayout->addWidget(tabWidget, 1);
    editorVLayout->addWidget(searchBar);
    
    // Add editor container and panel to splitter
    editorPanelSplitter->addWidget(editorContainer);
    editorPanelSplitter->addWidget(m_panelArea);
    editorPanelSplitter->setSizes({700, 200});
    
    // Add widgets: Editor (left), Sidebar (middle-right), ActivityBar (far right)
    contentLayout->addWidget(editorPanelSplitter, 1);
    contentLayout->addWidget(m_sidebar);
    contentLayout->addWidget(m_activityBar);
    
    // Add content layout to main vertical layout
    mainVLayout->addLayout(contentLayout, 1);
    
    // Add status bar at bottom
    mainVLayout->addWidget(m_statusBar);
    
    // Set the central widget
    this->setCentralWidget(centralContainer);
    
    // Initial visibility
    m_activityBar->show();
    m_sidebar->hide();  // Start collapsed, like VSCode
    m_statusBar->show();
    m_breadcrumb->show();
    m_panelArea->hide();  // Start collapsed, like VSCode
    
    // Set initial status bar values
    m_statusBar->setCursorPosition(1, 1);
    m_statusBar->setEncoding("UTF-8");
    m_statusBar->setLineEnding("LF");
    m_statusBar->setLanguage("Baa");
    m_statusBar->setFolderOpen(false);
    
    // Hide the old QMainWindow status bar
    QStatusBar* oldStatusBar = this->statusBar();
    if (oldStatusBar) {
        oldStatusBar->hide();
    }
    
    // Initialize the panel terminal
    if (m_panelArea->terminal()) {
        m_panelArea->terminal()->setConsoleRTL();
        m_panelArea->terminal()->startCmd();
    }
    
    // Sync when tabs are added - populate initial open editors
    syncOpenEditors();
}

void Qalam::onActivityViewChanged(int viewType)
{
    auto view = static_cast<TActivityBar::ViewType>(viewType);
    m_sidebar->setCurrentView(view);
    m_sidebar->show();
    
    // Update the sidebar root path when switching to Explorer
    if (view == TActivityBar::ViewType::Explorer && !folderPath.isEmpty()) {
        m_sidebar->explorerView()->setRootPath(folderPath);
    }
}

void Qalam::onSidebarFileSelected(const QString &filePath)
{
    openFile(filePath);
}

void Qalam::syncOpenEditors()
{
    if (!m_sidebar || !m_sidebar->explorerView()) return;
    
    // Clear existing open editors
    m_sidebar->explorerView()->clearOpenEditors();
    
    // Add all open tabs
    for (int i = 0; i < tabWidget->count(); ++i) {
        TEditor *editor = qobject_cast<TEditor*>(tabWidget->widget(i));
        if (editor) {
            QString filePath = editor->property("filePath").toString();
            bool modified = editor->document()->isModified();
            
            // Use tab text if no file path (unsaved file)
            if (filePath.isEmpty()) {
                filePath = tabWidget->tabText(i);
            }
            m_sidebar->explorerView()->addOpenEditor(filePath, modified);
        }
    }
}
