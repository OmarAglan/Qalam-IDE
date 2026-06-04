#include "Qalam.h"
#include "TWelcomePage.h"
#include "TConsole.h"
#include "TSearchPanel.h"
#include "Constants.h"

// VSCode-like UI components (needed to call methods on LayoutManager accessors)
#include "TActivityBar.h"
#include "TSidebar.h"
#include "TStatusBar.h"
#include "TPanelArea.h"
#include "TBreadcrumb.h"
#include "TExplorerView.h"

#include <QVBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QShortcut>
#include <QGuiApplication>
#include <QScreen>
#include <QCoreApplication>
#include <QApplication>
#include <QSettings>
#include <QFile>
#include <QKeyEvent>
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
    m_fileManager = new FileManager(tabWidget, this, this);
    m_buildManager = new BuildManager(this);
    m_sessionManager = new SessionManager(tabWidget, this);

    searchBar = new SearchPanel(this);
    searchBar->hide();

    m_layoutManager = new LayoutManager(this, tabWidget, searchBar, this);

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
    setting = new TSettings(this);

    // ===================================================================
    // الخطوة 4: إعداد التخطيط الجديد (VSCode-like)
    // ===================================================================
    m_layoutManager->setupLayout();

    // ===================================================================
    // الخطوة 5: ربط الإشارات والمقابس
    // ===================================================================
    connectSignals();
    onCurrentTabChanged();
    syncOpenEditors();
    
    // ===================================================================
    // الخطوة 6: تحميل الملف المبدئي أو استعادة الجلسة السابقة
    // ===================================================================
    installEventFilter(this);

    if (!filePath.isEmpty()) {
        // Explicit file passed (e.g. command-line argument or double-click)
        m_fileManager->openFile(filePath);
    } else {
        // Try to restore previous session
        auto session = m_sessionManager->restoreSession();

        // Restore window geometry
        if (not session.windowGeometry.isEmpty()) {
            restoreGeometry(session.windowGeometry);
        }

        // Restore folder
        if (not session.folderPath.isEmpty()) {
            loadFolder(session.folderPath);
        }

        // Restore open files
        bool restoredAny = false;
        for (const QString &file : session.openFiles) {
            if (QFile::exists(file)) {
                m_fileManager->openFile(file);
                restoredAny = true;
            }
        }

        // Restore active tab
        if (restoredAny and session.activeTabIndex >= 0
            and session.activeTabIndex < tabWidget->count()) {
            tabWidget->setCurrentIndex(session.activeTabIndex);
        }

        // If nothing was restored, show welcome or create a new empty tab
        if (not restoredAny) {
            if (shouldShowWelcome()) {
                showWelcomeTab();
            } else {
                m_fileManager->newFile();
            }
        }
    }
}

Qalam::~Qalam() {
    // Save session state
    m_sessionManager->saveSession(folderPath, saveGeometry());

    // Save user preferences
    m_sessionManager->savePreferences(currentEditor(), setting->getThemeCombo()->currentIndex());
}

// ===================================================================
// ربط جميع الإشارات والمقابس في مكان واحد
// ===================================================================
void Qalam::connectSignals()
{
    // --- Keyboard shortcuts ---
    auto *findShortcut = new QShortcut(QKeySequence::Find, this);
    connect(findShortcut, &QShortcut::activated, this, &Qalam::showFindBar);

    auto *saveShortcut = new QShortcut(QKeySequence::Save, this);
    connect(saveShortcut, &QShortcut::activated, m_fileManager, &FileManager::saveFile);

    auto *goToLineShortcut = new QShortcut(QKeySequence("Ctrl+G"), this);
    connect(goToLineShortcut, &QShortcut::activated, this, &Qalam::goToLine);

    auto *commentShortcut = new QShortcut(QKeySequence("Ctrl+/"), this);
    connect(commentShortcut, &QShortcut::activated, this, [this](){
        if (TEditor* editor = currentEditor()) editor->toggleComment();
    });

    auto *duplicateShortcut = new QShortcut(QKeySequence("Ctrl+D"), this);
    connect(duplicateShortcut, &QShortcut::activated, this, [this](){
        if (TEditor* editor = currentEditor()) editor->duplicateLine();
    });

    auto *moveUpShortcut = new QShortcut(QKeySequence("Alt+Up"), this);
    connect(moveUpShortcut, &QShortcut::activated, this, [this](){
        if (TEditor* editor = currentEditor()) editor->moveLineUp();
    });

    auto *moveDownShortcut = new QShortcut(QKeySequence("Alt+Down"), this);
    connect(moveDownShortcut, &QShortcut::activated, this, [this](){
        if (TEditor* editor = currentEditor()) editor->moveLineDown();
    });

    // --- Menu bar signals ---
    connect(menuBar, &TMenuBar::newRequested, this, &Qalam::newFileFromUi);
    connect(menuBar, &TMenuBar::openFileRequested, this, [this]() { openFileFromUi(QString()); });
    connect(menuBar, &TMenuBar::saveRequested, m_fileManager, &FileManager::saveFile);
    connect(menuBar, &TMenuBar::saveAsRequested, m_fileManager, &FileManager::saveFileAs);
    connect(menuBar, &TMenuBar::settingsRequest, this, &Qalam::openSettings);
    connect(menuBar, &TMenuBar::exitRequested, this, &Qalam::exitApp);
    connect(menuBar, &TMenuBar::runRequested, this, &Qalam::runBaa);
    connect(menuBar, &TMenuBar::aboutRequested, this, &Qalam::aboutQalam);
    connect(menuBar, &TMenuBar::openFolderRequested, this, &Qalam::handleOpenFolderMenu);

    // --- Tab widget signals ---
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &Qalam::closeTab);
    connect(tabWidget, &QTabWidget::currentChanged, this, &Qalam::updateWindowTitle);
    connect(tabWidget, &QTabWidget::currentChanged, this, &Qalam::onCurrentTabChanged);

    // --- Search bar signals ---
    // Search logic is now handled internally by SearchPanel.
    // We only need to connect the close signal and sync the active editor.
    connect(searchBar, &SearchPanel::closed, this, &Qalam::hideFindBar);

    // --- FileManager signals ---
    connect(m_fileManager, &FileManager::fileStateChanged, this, &Qalam::updateWindowTitle);
    connect(m_fileManager, &FileManager::fileStateChanged, this, [this]() {
        // Update modification indicator on tab text
        TEditor *editor = currentEditor();
        if (editor) {
            int index = tabWidget->indexOf(editor);
            if (index != -1) {
                bool modified = editor->document()->isModified();
                QString currentText = tabWidget->tabText(index);
                if (modified and not currentText.endsWith("[*]")) {
                    tabWidget->setTabText(index, currentText + "[*]");
                } else if (not modified and currentText.endsWith("[*]")) {
                    tabWidget->setTabText(index, currentText.left(currentText.length() - 3));
                }
            }
        }
    });
    connect(m_fileManager, &FileManager::openEditorsChanged, this, &Qalam::syncOpenEditors);

    // --- Layout component signals ---
    auto *activityBar = m_layoutManager->activityBar();
    auto *sidebar = m_layoutManager->sidebar();
    auto *panelArea = m_layoutManager->panelArea();
    auto *statusBar = m_layoutManager->statusBar();

    connect(activityBar, &TActivityBar::viewChanged, this, &Qalam::onActivityViewChanged);
    connect(activityBar, &TActivityBar::runRequested, this, &Qalam::runBaa);

    connect(activityBar, &TActivityBar::viewToggled, this, [this](TActivityBar::ViewType view, bool visible) {
        if (view == TActivityBar::ViewType::Settings) {
            openSettings();
            return;
        }
        if (!visible) {
            m_layoutManager->sidebar()->hide();
        } else {
            m_layoutManager->sidebar()->show();
            m_layoutManager->sidebar()->setCurrentView(view);
        }
    });

    connect(sidebar, &TSidebar::fileSelected, this, &Qalam::onSidebarFileSelected);
    connect(sidebar, &TSidebar::openFolderRequested, this, &Qalam::handleOpenFolderMenu);

    connect(statusBar, &TStatusBar::problemsClicked, this, [this]() {
        m_layoutManager->panelArea()->setCurrentTab(TPanelArea::Tab::Problems);
        m_layoutManager->panelArea()->show();
    });

    connect(panelArea, &TPanelArea::closeRequested, this, [this]() {
        m_layoutManager->panelArea()->hide();
    });

    connect(panelArea, &TPanelArea::tabChanged, this, [this](TPanelArea::Tab tab) {
        if (tab == TPanelArea::Tab::Terminal and m_layoutManager->panelArea()->terminal()) {
            m_layoutManager->panelArea()->terminal()->setFocus();
        }
    });
}

void Qalam::closeEvent(QCloseEvent *event) {
    auto saveResult = m_fileManager->needSave();

    if (saveResult == FileManager::SaveAction::Save) {
        m_fileManager->saveFile();
    } else if (saveResult == FileManager::SaveAction::Cancel) {
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
    return QalamWindow::eventFilter(object, event);
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
    searchBar->setEditor(currentEditor());
    searchBar->show();
    searchBar->setFocusToInput();
}

void Qalam::hideFindBar() {
    searchBar->hide();
    if (TEditor* editor = currentEditor()) {
        editor->setFocus();
    }
}

void Qalam::toggleConsole()
{
    m_layoutManager->toggleConsole(currentEditor());
}

void Qalam::loadFolder(const QString &path)
{
    this->folderPath = path;
    m_layoutManager->loadFolder(path);
}

void Qalam::handleOpenFolderMenu()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, "اختر مجلد", QDir::homePath());
    if (folderPath.isEmpty()) return;

    loadFolder(folderPath);

    if (not hasAnyEditorTabs()) {
        m_fileManager->newFile();
    }

    removeWelcomeTabIfPresent();
}

void Qalam::toggleSidebar()
{
    m_layoutManager->toggleSidebar();
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
    auto result = m_fileManager->needSave();
    if (result == FileManager::SaveAction::Cancel) {
        return;
    }
    else if (result == FileManager::SaveAction::Save) {
        m_fileManager->saveFile();
    }
    close();
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

        // Keep search panel pointing at the active editor
        searchBar->setEditor(editor);
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
        if (m_layoutManager->statusBar()) {
            m_layoutManager->statusBar()->setCursorPosition(line, column);
        }
        
        // Update breadcrumb with current file
        if (m_layoutManager->breadcrumb()) {
            QString filePath = editor->property("filePath").toString();
            m_layoutManager->breadcrumb()->setFilePath(filePath);
        }
    }
}


/* ----------------------------------- Run Menu Button ----------------------------------- */

void Qalam::runBaa() {
    TEditor *editor = currentEditor();
    if (!editor) return;

    QString filePath = editor->property("filePath").toString();
    if (filePath.isEmpty() or editor->document()->isModified()) {
        QMessageBox::warning(this, "تنبيه", "يجب حفظ الملف قبل التشغيل.");
        m_fileManager->saveFile();
        filePath = editor->property("filePath").toString();
        if (filePath.isEmpty() or editor->document()->isModified()) return;
    }

    // Show the panel area with output tab
    auto *panelArea = m_layoutManager->panelArea();
    if (!panelArea) return;
    panelArea->setCurrentTab(TPanelArea::Tab::Output);
    panelArea->show();
    panelArea->setCollapsed(false);

    // Delegate build to BuildManager
    TConsole *console = panelArea->terminal();
    m_buildManager->runBaa(filePath, console);
}

//----------------

TEditor* Qalam::currentEditor() {
    return qobject_cast<TEditor*>(tabWidget->currentWidget());
}

bool Qalam::shouldShowWelcome() const
{
    QSettings settings(Constants::OrgName, Constants::AppName);
    return settings.value(Constants::SettingsKeyShowWelcome, true).toBool();
}

bool Qalam::hasAnyEditorTabs() const
{
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (qobject_cast<TEditor*>(tabWidget->widget(i))) {
            return true;
        }
    }
    return false;
}

void Qalam::showWelcomeTab()
{
    if (m_welcomePage) {
        const int index = tabWidget->indexOf(m_welcomePage);
        if (index != -1) {
            m_welcomePage->refreshRecents();
            tabWidget->setCurrentIndex(index);
            return;
        }

        m_welcomePage->deleteLater();
        m_welcomePage = nullptr;
    }

    m_welcomePage = new TWelcomePage(tabWidget);

    connect(m_welcomePage, &TWelcomePage::newFileRequested, this, &Qalam::newFileFromUi);
    connect(m_welcomePage, &TWelcomePage::openFileRequested, this, [this]() { openFileFromUi(QString()); });
    connect(m_welcomePage, &TWelcomePage::openFolderRequested, this, &Qalam::handleOpenFolderMenu);
    connect(m_welcomePage, &TWelcomePage::recentFileRequested, this, &Qalam::openFileFromUi);
    connect(m_welcomePage, &TWelcomePage::cloneRepoRequested, this, [this]() {
        QMessageBox::information(this, "استنساخ", "هذه الميزة قيد التطوير.");
    });

    const int index = tabWidget->addTab(m_welcomePage, "الترحيب");
    tabWidget->setCurrentIndex(index);
}

void Qalam::removeWelcomeTabIfPresent()
{
    if (!m_welcomePage) return;

    const int index = tabWidget->indexOf(m_welcomePage);
    if (index == -1) return;

    tabWidget->removeTab(index);
    m_welcomePage->deleteLater();
    m_welcomePage = nullptr;
}

void Qalam::newFileFromUi()
{
    m_fileManager->newFile();
    if (hasAnyEditorTabs()) {
        removeWelcomeTabIfPresent();
    }
}

void Qalam::openFileFromUi(const QString &filePathOrEmpty)
{
    m_fileManager->openFile(filePathOrEmpty);
    if (hasAnyEditorTabs()) {
        removeWelcomeTabIfPresent();
    }
}

void Qalam::closeTab(int index)
{
    QWidget *tab = tabWidget->widget(index);

    if (!tab) return;

    if (auto *welcome = qobject_cast<TWelcomePage*>(tab)) {
        tabWidget->removeTab(index);
        if (welcome == m_welcomePage) {
            m_welcomePage->deleteLater();
            m_welcomePage = nullptr;
        } else {
            welcome->deleteLater();
        }

        if (tabWidget->count() == 0) {
            m_fileManager->newFile();
        }
        return;
    }

    TEditor* editor = qobject_cast<TEditor*>(tab);
    if (!editor) {
        tabWidget->removeTab(index);
        tab->deleteLater();
        return;
    }

    if (editor and editor->document()->isModified()) {
        auto saveResult = m_fileManager->needSave();

        if (saveResult == FileManager::SaveAction::Cancel) {
            return;
        }
        else if (saveResult == FileManager::SaveAction::Save) {
            m_fileManager->saveFile();
            return;
        }

    }
    tabWidget->removeTab(index);
    editor->deleteLater();
    syncOpenEditors();

    if (not hasAnyEditorTabs()) {
        if (shouldShowWelcome()) {
            showWelcomeTab();
        } else {
            m_fileManager->newFile();
        }
    }
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

void Qalam::onActivityViewChanged(TActivityBar::ViewType view)
{
    m_layoutManager->onActivityViewChanged(view, folderPath);
}

void Qalam::onSidebarFileSelected(const QString &filePath)
{
    m_fileManager->openFile(filePath);
}

void Qalam::syncOpenEditors()
{
    auto *sidebar = m_layoutManager->sidebar();
    if (not sidebar or not sidebar->explorerView()) return;
    m_sessionManager->syncOpenEditors(sidebar->explorerView());
}
