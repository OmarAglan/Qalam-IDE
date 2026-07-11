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
#include <QFileInfo>
#include <QTextStream>
#include <QStringConverter>
#include <QDirIterator>
#include <QRegularExpression>
#include <QKeyEvent>
#include <QInputDialog>
#include <QSet>
#include <QVector>
#include <QTextBlock>
#include "TSearchView.h"
#include "CommandRegistry.h"
#include "DiagnosticParser.h"
#include "DiagnosticsModel.h"
#include "WorkspaceIndexer.h"
#include "BreakpointModel.h"
#include "TCommandPalette.h"

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
    m_commandRegistry = new CommandRegistry(this);
    for (const auto &command : CommandRegistry::defaultCommands()) {
        m_commandRegistry->registerCommand(command);
    }
    m_diagnosticsModel = new DiagnosticsModel(this);
    m_workspaceIndexer = new WorkspaceIndexer(this);
    m_breakpointModel = new BreakpointModel(this);

    searchBar = new SearchPanel(this);
    searchBar->hide();

    m_layoutManager = new LayoutManager(this, tabWidget, searchBar, this);

    // ===================================================================
    // الخطوة 2: إعداد النافذة وشريط القوائم
    // ===================================================================
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeo = screen ? screen->availableGeometry() : QRect(0, 0, 1280, 800);
    const int margin = 100;
    const int widthFixedNum = 6;
    const int width = qMax(900, screenGeo.size().width() - margin * widthFixedNum);
    const int height = qMax(600, screenGeo.size().height() - margin);
    const int x = screenGeo.left() + qMax(0, (screenGeo.width() - width) / 2);
    const int y = screenGeo.top() + qMax(0, (screenGeo.height() - height) / 2);
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
    // Global workbench shortcuts are owned by TMenuBar QActions so they show
    // in the menus and avoid duplicate Qt shortcut ambiguity. Editor-only
    // shortcuts stay here.

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
    connect(menuBar, &TMenuBar::buildRequested, this, &Qalam::buildTakweenProject);
    connect(menuBar, &TMenuBar::runRequested, this, &Qalam::runBaa);
    connect(menuBar, &TMenuBar::cleanRequested, this, &Qalam::cleanTakweenProject);
    connect(menuBar, &TMenuBar::aboutRequested, this, &Qalam::aboutQalam);
    connect(menuBar, &TMenuBar::openFolderRequested, this, &Qalam::handleOpenFolderMenu);
    connect(menuBar, &TMenuBar::commandPaletteRequested, this, &Qalam::showCommandPalette);
    connect(menuBar, &TMenuBar::quickOpenRequested, this, &Qalam::showQuickOpen);
    connect(menuBar, &TMenuBar::findRequested, this, &Qalam::showFindBar);
    connect(menuBar, &TMenuBar::findInFilesRequested, this, &Qalam::focusSearchInFiles);
    connect(menuBar, &TMenuBar::goToLineRequested, this, &Qalam::goToLine);
    connect(menuBar, &TMenuBar::toggleSidebarRequested, this, &Qalam::toggleSidebar);
    connect(menuBar, &TMenuBar::togglePanelRequested, this, &Qalam::toggleConsole);
    connect(menuBar, &TMenuBar::problemsRequested, this, &Qalam::openProblemsPanel);
    connect(menuBar, &TMenuBar::debugPanelRequested, this, &Qalam::openDebugPanel);
    connect(menuBar, &TMenuBar::goToDefinitionRequested, this, &Qalam::goToDefinition);
    connect(menuBar, &TMenuBar::findReferencesRequested, this, &Qalam::findReferences);
    connect(this, &QalamWindow::commandCenterClicked, this, &Qalam::showCommandPalette);

    connect(m_buildManager, &BuildManager::outputChunk, this, &Qalam::handleBuildOutput);
    connect(m_buildManager, &BuildManager::diagnosticsReady, this, [this](const QString &json) {
        if (!m_diagnosticsModel) return;
        QString fallbackFile;
        if (TEditor *editor = currentEditor()) fallbackFile = editor->currentFilePath();
        m_diagnosticsModel->setDiagnostics(
            DiagnosticParser::parseCompilerOutput(json, fallbackFile, folderPath));
    });
    connect(m_buildManager, &BuildManager::buildFinished, this, [this](int exitCode) {
        if (exitCode != 0 and m_diagnosticsModel and m_diagnosticsModel->count() == 0) {
            QString filePath;
            if (TEditor *editor = currentEditor()) {
                filePath = editor->currentFilePath();
            }
            QVector<Diagnostic> runnerDiagnostics;
            Diagnostic diagnostic;
            diagnostic.file = filePath;
            diagnostic.severity = "error";
            diagnostic.message = "فشل التشغيل أو البناء. راجع الطرفية للمزيد من التفاصيل.";
            diagnostic.source = "runner";
            runnerDiagnostics.push_back(diagnostic);
            m_diagnosticsModel->addDiagnostics(runnerDiagnostics);
        }
        updateProblemsStatusBar();
    });

    connect(m_diagnosticsModel, &DiagnosticsModel::diagnosticsChanged, this, [this]() {
        rebuildProblemsPanel();
        applyDiagnosticsToEditors();
        updateProblemsStatusBar();
    });

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
        // Update modification indicators for every editor, not only the active tab.
        for (int index = 0; index < tabWidget->count(); ++index) {
            TEditor *editor = qobject_cast<TEditor*>(tabWidget->widget(index));
            if (!editor) continue;

            const bool modified = editor->document()->isModified();
            QString tabText = tabWidget->tabText(index);
            if (modified and not tabText.endsWith("[*]")) {
                tabWidget->setTabText(index, tabText + "[*]");
            } else if (not modified and tabText.endsWith("[*]")) {
                tabWidget->setTabText(index, tabText.left(tabText.length() - 3));
            }
        }

        TEditor *editor = currentEditor();
        if (editor and not editor->document()->isModified() and
            !editor->currentFilePath().isEmpty()) {
            m_buildManager->checkBaa(editor->currentFilePath());
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
    connect(sidebar, &TSidebar::openEditorCloseRequested, this, &Qalam::closeEditorByPath);
    connect(sidebar, &TSidebar::searchRequested, this, &Qalam::performProjectSearch);
    if (sidebar->searchView()) {
        connect(sidebar->searchView(), &TSearchView::resultClicked, this, &Qalam::goToLocation);
    }

    connect(statusBar, &TStatusBar::problemsClicked, this, [this]() {
        m_layoutManager->panelArea()->setCurrentTab(TPanelArea::Tab::Problems);
        m_layoutManager->panelArea()->show();
    });

    connect(panelArea, &TPanelArea::problemClicked, this, &Qalam::goToLocation);

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
    if (!maybeSaveAllModified()) {
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
    if (m_workspaceIndexer) {
        m_workspaceIndexer->setRootPath(path);
    }
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
    close();
}

void Qalam::onCurrentTabChanged()
{
    updateWindowTitle();
    updateCursorPosition();

    TEditor* editor = currentEditor();

    // Disconnect the previous editor to avoid accumulating connections.
    // Also clear the raw pointer so closing the last editor does not leave a dangling reference.
    if (m_lastConnectedEditor) {
        disconnect(m_lastConnectedEditor, &QPlainTextEdit::cursorPositionChanged, this, &Qalam::updateCursorPosition);
        m_lastConnectedEditor = nullptr;
    }

    if (editor) {
        connect(editor, &QPlainTextEdit::cursorPositionChanged, this, &Qalam::updateCursorPosition);
        connect(editor, &QObject::destroyed, this, [this, editor]() {
            if (m_lastConnectedEditor == editor) {
                m_lastConnectedEditor = nullptr;
            }
        }, Qt::UniqueConnection);
        m_lastConnectedEditor = editor;

        // Keep search panel pointing at the active editor
        searchBar->setEditor(editor);
        if (m_layoutManager->breadcrumb()) {
            m_layoutManager->breadcrumb()->setVisible(!editor->currentFilePath().isEmpty());
        }
        applyDiagnosticsToEditors();
    } else {
        searchBar->setEditor(nullptr);
        if (m_layoutManager->breadcrumb()) {
            m_layoutManager->breadcrumb()->hide();
        }
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
            QString filePath = editor->currentFilePath();
            m_layoutManager->breadcrumb()->setFilePath(filePath);
        }
    }
}


/* ----------------------------------- Run Menu Button ----------------------------------- */

void Qalam::runBaa() {
    TEditor *editor = currentEditor();
    if (!editor) return;

    QString filePath = editor->currentFilePath();
    if (filePath.isEmpty() or editor->document()->isModified()) {
        QMessageBox::warning(this, "تنبيه", "يجب حفظ الملف قبل التشغيل.");
        m_fileManager->saveFile();
        filePath = editor->currentFilePath();
        if (filePath.isEmpty() or editor->document()->isModified()) return;
    }

    // Show the terminal tab because Baa programs may ask for input.
    auto *panelArea = m_layoutManager->panelArea();
    if (!panelArea) return;
    panelArea->clearProblems();
    if (m_diagnosticsModel) {
        m_diagnosticsModel->clear();
    } else {
        applyDiagnosticsToEditors();
        updateProblemsStatusBar();
    }
    panelArea->setCurrentTab(TPanelArea::Tab::Terminal);
    panelArea->show();
    panelArea->setCollapsed(false);

    // Delegate build to BuildManager
    TConsole *console = panelArea->terminal();
    m_buildManager->runBaa(filePath, console);
}

void Qalam::buildTakweenProject()
{
    runTakweenProjectCommand("build");
}

void Qalam::cleanTakweenProject()
{
    runTakweenProjectCommand("clean");
}

void Qalam::runTakweenProjectCommand(const QString &command)
{
    TEditor *editor = currentEditor();
    if (!editor or editor->currentFilePath().isEmpty()) {
        QMessageBox::information(this, "مشروع تكوين", "افتح ملفًا محفوظًا داخل مشروع تكوين أولًا.");
        return;
    }

    if (editor->document()->isModified()) {
        m_fileManager->saveFile();
        if (editor->document()->isModified()) return;
    }

    auto *panelArea = m_layoutManager ? m_layoutManager->panelArea() : nullptr;
    if (!panelArea) return;
    panelArea->setCurrentTab(TPanelArea::Tab::Terminal);
    panelArea->show();
    panelArea->setCollapsed(false);

    if (!m_buildManager->runTakweenCommand(
            editor->currentFilePath(), command, panelArea->terminal())) {
        QMessageBox::warning(
            this,
            "مشروع تكوين",
            "لم يُعثر على مشروع.تكوين أو على برنامج تكوين القابل للتنفيذ.");
    }
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

    if (editor->document()->isModified()) {
        const int previousIndex = tabWidget->currentIndex();
        tabWidget->setCurrentIndex(index);
        auto saveResult = m_fileManager->needSave(editor);

        if (saveResult == FileManager::SaveAction::Cancel) {
            tabWidget->setCurrentIndex(previousIndex);
            return;
        }
        if (saveResult == FileManager::SaveAction::Save and !m_fileManager->saveEditor(editor)) {
            tabWidget->setCurrentIndex(previousIndex);
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

bool Qalam::maybeSaveAllModified()
{
    const int previousIndex = tabWidget->currentIndex();

    for (int i = 0; i < tabWidget->count(); ++i) {
        TEditor *editor = qobject_cast<TEditor*>(tabWidget->widget(i));
        if (!editor or !editor->document()->isModified()) {
            continue;
        }

        tabWidget->setCurrentIndex(i);
        auto result = m_fileManager->needSave(editor);
        if (result == FileManager::SaveAction::Cancel) {
            tabWidget->setCurrentIndex(previousIndex);
            return false;
        }
        if (result == FileManager::SaveAction::Save and !m_fileManager->saveEditor(editor)) {
            tabWidget->setCurrentIndex(previousIndex);
            return false;
        }
    }

    if (previousIndex >= 0 and previousIndex < tabWidget->count()) {
        tabWidget->setCurrentIndex(previousIndex);
    }
    return true;
}

void Qalam::goToLocation(const QString &filePath, int line, int column)
{
    if (filePath.isEmpty()) return;

    openFileFromUi(filePath);
    TEditor *editor = currentEditor();
    if (!editor) return;

    QTextCursor cursor(editor->document());
    const int targetLine = qMax(1, line);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, targetLine - 1);
    const int targetColumn = qMax(1, column);
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, targetColumn - 1);
    editor->setTextCursor(cursor);
    editor->centerCursor();
    editor->setFocus();
}

void Qalam::closeEditorByPath(const QString &filePath)
{
    const QString targetCanonical = QFileInfo(filePath).canonicalFilePath();
    const QString targetClean = QDir::cleanPath(filePath);

    for (int i = 0; i < tabWidget->count(); ++i) {
        TEditor *editor = qobject_cast<TEditor*>(tabWidget->widget(i));
        if (!editor) continue;

        const QString editorPath = editor->currentFilePath();
        const QString editorCanonical = QFileInfo(editorPath).canonicalFilePath();
        const QString editorClean = QDir::cleanPath(editorPath);

        const bool sameRealFile = !targetCanonical.isEmpty() and editorCanonical == targetCanonical;
        const bool sameUnsavedLabel = targetCanonical.isEmpty() and editorPath.isEmpty() and tabWidget->tabText(i) == filePath;
        const bool sameCleanPath = !targetClean.isEmpty() and !editorClean.isEmpty() and editorClean == targetClean;

        if (sameRealFile or sameCleanPath or sameUnsavedLabel) {
            closeTab(i);
            return;
        }
    }
}

void Qalam::performProjectSearch(const QString &query, bool caseSensitive, bool wholeWord, bool regex)
{
    auto *sidebar = m_layoutManager ? m_layoutManager->sidebar() : nullptr;
    auto *searchView = sidebar ? sidebar->searchView() : nullptr;
    if (!searchView) return;

    searchView->clearResults();

    const QString trimmedQuery = query.trimmed();
    if (trimmedQuery.isEmpty() or folderPath.isEmpty()) {
        searchView->setResultCount(0, 0);
        return;
    }

    QRegularExpression::PatternOptions options = QRegularExpression::UseUnicodePropertiesOption;
    if (!caseSensitive) {
        options |= QRegularExpression::CaseInsensitiveOption;
    }

    QString pattern = regex ? trimmedQuery : QRegularExpression::escape(trimmedQuery);
    if (wholeWord) {
        pattern = QStringLiteral("(?<![\\p{L}\\p{N}_])(?:%1)(?![\\p{L}\\p{N}_])").arg(pattern);
    }

    QRegularExpression expression(pattern, options);
    if (!expression.isValid()) {
        searchView->setResultCount(0, 0);
        return;
    }

    int fileCount = 0;
    int matchCount = 0;

    const QStringList indexedFiles = collectProjectFiles();
    for (const QString &path : indexedFiles) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        QTextStream stream(&file);
        stream.setEncoding(QStringConverter::Utf8);

        bool fileHadMatch = false;
        int lineNumber = 0;
        while (!stream.atEnd()) {
            const QString lineText = stream.readLine();
            ++lineNumber;

            QRegularExpressionMatchIterator matches = expression.globalMatch(lineText);
            while (matches.hasNext()) {
                const QRegularExpressionMatch match = matches.next();
                if (!match.hasMatch()) continue;

                if (!fileHadMatch) {
                    fileHadMatch = true;
                    ++fileCount;
                }
                ++matchCount;
                searchView->addResult(path, lineNumber, match.capturedStart() + 1,
                                      lineText, match.captured(0));
            }
        }
    }

    searchView->setResultCount(fileCount, matchCount);
}


void Qalam::focusSearchInFiles()
{
    if (!m_layoutManager or !m_layoutManager->sidebar()) return;

    m_layoutManager->sidebar()->show();
    m_layoutManager->sidebar()->setCurrentView(TActivityBar::ViewType::Search);
    if (m_layoutManager->activityBar()) {
        m_layoutManager->activityBar()->setCurrentView(TActivityBar::ViewType::Search);
    }
    if (m_layoutManager->sidebar()->searchView()) {
        m_layoutManager->sidebar()->searchView()->focusSearchInput();
    }
}

void Qalam::openProblemsPanel()
{
    auto *panel = m_layoutManager ? m_layoutManager->panelArea() : nullptr;
    if (!panel) return;

    panel->setCurrentTab(TPanelArea::Tab::Problems);
    panel->show();
    panel->setCollapsed(false);
}


void Qalam::openDebugPanel()
{
    auto *panel = m_layoutManager ? m_layoutManager->panelArea() : nullptr;
    if (!panel) return;

    panel->setCurrentTab(TPanelArea::Tab::Debug);
    panel->show();
    panel->setCollapsed(false);
}

QStringList Qalam::collectProjectFiles() const
{
    if (m_workspaceIndexer) {
        return m_workspaceIndexer->quickOpenFiles();
    }
    return {};
}

bool Qalam::runCommandById(const QString &commandId)
{
    if (commandId == "file.new") { newFileFromUi(); return true; }
    if (commandId == "file.open") { openFileFromUi(QString()); return true; }
    if (commandId == "folder.open") { handleOpenFolderMenu(); return true; }
    if (commandId == "file.save") { m_fileManager->saveFile(); return true; }
    if (commandId == "file.saveAs") { m_fileManager->saveFileAs(); return true; }
    if (commandId == "view.search") { focusSearchInFiles(); return true; }
    if (commandId == "view.sidebar") { toggleSidebar(); return true; }
    if (commandId == "view.panel") { toggleConsole(); return true; }
    if (commandId == "view.problems") { openProblemsPanel(); return true; }
    if (commandId == "view.debug") { openDebugPanel(); return true; }
    if (commandId == "code.definition") { goToDefinition(); return true; }
    if (commandId == "code.references") { findReferences(); return true; }
    if (commandId == "project.build") { buildTakweenProject(); return true; }
    if (commandId == "run.baa") { runBaa(); return true; }
    if (commandId == "project.clean") { cleanTakweenProject(); return true; }
    if (commandId == "quick.open") { showQuickOpen(); return true; }
    if (commandId == "go.line") { goToLine(); return true; }
    if (commandId == "settings.open") { openSettings(); return true; }
    if (commandId == "help.about") { aboutQalam(); return true; }
    return false;
}

void Qalam::showCommandPalette()
{
    QVector<TCommandPalette::Entry> entries;
    if (m_commandRegistry) {
        for (const CommandRegistry::Command &command : m_commandRegistry->commands()) {
            entries.push_back({command.id, command.title, command.description, command.shortcut, QString()});
        }
    }

    auto *palette = new TCommandPalette(this);
    palette->setAttribute(Qt::WA_DeleteOnClose);
    palette->setWindowTitle("لوحة الأوامر");
    palette->setPlaceholderText("اكتب اسم الأمر...");
    palette->setEmptyText("لا يوجد أمر مطابق");
    palette->setEntries(entries);
    connect(palette, &TCommandPalette::entryActivated, this, [this](const QString &id, const QString &) {
        runCommandById(id);
    });
    palette->show();
}

void Qalam::showQuickOpen()
{
    const QStringList files = collectProjectFiles();
    if (files.isEmpty()) {
        if (folderPath.isEmpty()) {
            QMessageBox::information(this, "فتح سريع", "افتح مجلدًا أولًا لاستخدام الفتح السريع.");
        } else {
            QMessageBox::information(this, "فتح سريع", "لا توجد ملفات مناسبة داخل المجلد الحالي.");
        }
        return;
    }

    QVector<TCommandPalette::Entry> entries;
    entries.reserve(qMin(files.size(), 600));
    for (const QString &file : files) {
        const QString relative = QDir(folderPath).relativeFilePath(file);
        entries.push_back({"file.open.path", relative, QFileInfo(file).absolutePath(), QString(), file});
        if (entries.size() >= 600) break;
    }

    auto *palette = new TCommandPalette(this);
    palette->setAttribute(Qt::WA_DeleteOnClose);
    palette->setWindowTitle("فتح سريع");
    palette->setPlaceholderText("اكتب اسم الملف...");
    palette->setEmptyText("لا يوجد ملف مطابق");
    palette->setEntries(entries);
    connect(palette, &TCommandPalette::entryActivated, this, [this](const QString &id, const QString &payload) {
        if (id == "file.open.path" and !payload.isEmpty()) {
            openFileFromUi(payload);
        }
    });
    palette->show();
}

void Qalam::updateProblemsStatusBar()
{
    auto *status = m_layoutManager ? m_layoutManager->statusBar() : nullptr;
    if (!status || !m_diagnosticsModel) return;
    status->setProblemsCount(m_diagnosticsModel->errorCount(), m_diagnosticsModel->warningCount());
}

void Qalam::rebuildProblemsPanel()
{
    auto *panel = m_layoutManager ? m_layoutManager->panelArea() : nullptr;
    if (!panel || !m_diagnosticsModel) return;

    panel->clearProblems();
    for (const Diagnostic &diagnostic : m_diagnosticsModel->diagnostics()) {
        panel->addProblem(diagnostic.displayMessage(), diagnostic.file,
                          diagnostic.line, diagnostic.column,
                          diagnostic.severity);
    }
}

void Qalam::handleBuildOutput(const QString &text)
{
    if (!m_diagnosticsModel) return;

    QString fallbackFile;
    if (TEditor *editor = currentEditor()) {
        fallbackFile = editor->currentFilePath();
    }

    const QVector<Diagnostic> diagnostics = DiagnosticParser::parseCompilerOutput(text, fallbackFile, folderPath);
    m_diagnosticsModel->addDiagnostics(diagnostics);
}

void Qalam::applyDiagnosticsToEditors()
{
    for (int index = 0; index < tabWidget->count(); ++index) {
        auto *editor = qobject_cast<TEditor*>(tabWidget->widget(index));
        if (!editor) continue;

        QVector<TEditor::Diagnostic> editorDiagnostics;
        if (m_diagnosticsModel) {
            for (const Diagnostic &diagnostic : m_diagnosticsModel->diagnosticsForFile(editor->currentFilePath())) {
                editorDiagnostics.push_back({diagnostic.file, diagnostic.line, diagnostic.column,
                                             diagnostic.severity, diagnostic.message});
            }
        }
        editor->setDiagnostics(editorDiagnostics);
    }
}

QString Qalam::symbolUnderCursor() const
{
    const TEditor *editor = const_cast<Qalam*>(this)->currentEditor();
    if (!editor) return QString();

    QTextCursor cursor = editor->textCursor();
    const QString blockText = cursor.block().text();
    int end = qBound(0, cursor.positionInBlock(), static_cast<int>(blockText.length()));
    int start = end;
    while (start > 0) {
        const QChar ch = blockText.at(start - 1);
        if (!(ch.isLetterOrNumber() || ch == '_' || ch == '#')) break;
        --start;
    }
    while (end < blockText.length()) {
        const QChar ch = blockText.at(end);
        if (!(ch.isLetterOrNumber() || ch == '_' || ch == '#')) break;
        ++end;
    }
    return blockText.mid(start, end - start).trimmed();
}

bool Qalam::findDefinitionLocation(const QString &symbol, QString *filePath, int *line, int *column) const
{
    if (symbol.trimmed().isEmpty() || !m_workspaceIndexer) return false;

    WorkspaceIndexer::SymbolLocation location;
    if (!m_workspaceIndexer->findDefinition(symbol, &location)) {
        return false;
    }

    if (filePath) *filePath = location.file;
    if (line) *line = location.line;
    if (column) *column = location.column;
    return true;
}

void Qalam::goToDefinition()
{
    const QString symbol = symbolUnderCursor();
    if (symbol.isEmpty()) {
        if (m_layoutManager && m_layoutManager->statusBar()) {
            m_layoutManager->statusBar()->showMessage("لا يوجد رمز تحت المؤشر");
        }
        return;
    }

    QString file;
    int line = 1;
    int column = 1;
    if (findDefinitionLocation(symbol, &file, &line, &column)) {
        goToLocation(file, line, column);
    } else if (m_layoutManager && m_layoutManager->statusBar()) {
        m_layoutManager->statusBar()->showMessage("لم يتم العثور على تعريف: " + symbol);
    }
}

void Qalam::findReferences()
{
    const QString symbol = symbolUnderCursor();
    if (symbol.isEmpty()) {
        if (m_layoutManager && m_layoutManager->statusBar()) {
            m_layoutManager->statusBar()->showMessage("لا يوجد رمز تحت المؤشر");
        }
        return;
    }

    focusSearchInFiles();
    performProjectSearch(symbol, false, true, false);
    if (m_workspaceIndexer) {
        const auto references = m_workspaceIndexer->findReferences(symbol);
        if (m_layoutManager && m_layoutManager->statusBar()) {
            m_layoutManager->statusBar()->showMessage(
                QString("تم العثور على %1 مرجع/مراجع لـ: %2").arg(references.size()).arg(symbol));
        }
    } else if (m_layoutManager && m_layoutManager->statusBar()) {
        m_layoutManager->statusBar()->showMessage("تم البحث عن المراجع: " + symbol);
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
        QString filePath = editor->currentFilePath();

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
