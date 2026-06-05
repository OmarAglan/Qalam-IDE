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
#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QSet>
#include <QVector>
#include "TSearchView.h"

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
    connect(menuBar, &TMenuBar::runRequested, this, &Qalam::runBaa);
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
    connect(this, &QalamWindow::commandCenterClicked, this, &Qalam::showCommandPalette);

    connect(m_buildManager, &BuildManager::outputChunk, this, &Qalam::handleBuildOutput);
    connect(m_buildManager, &BuildManager::buildFinished, this, [this](int exitCode) {
        if (exitCode != 0 and m_layoutManager and m_layoutManager->panelArea()
            and m_layoutManager->panelArea()->problemCount() == 0) {
            QString filePath;
            if (TEditor *editor = currentEditor()) {
                filePath = editor->currentFilePath();
            }
            m_layoutManager->panelArea()->addProblem("فشل التشغيل أو البناء. راجع الطرفية للمزيد من التفاصيل.",
                                                      filePath, 1, 1, "error");
        }
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
    m_seenDiagnostics.clear();
    panelArea->clearProblems();
    updateProblemsStatusBar();
    panelArea->setCurrentTab(TPanelArea::Tab::Terminal);
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

    const QStringList allowedExtensions = {"baa", "baahd", "txt"};
    int fileCount = 0;
    int matchCount = 0;

    QDirIterator it(folderPath, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString path = it.next();
        const QFileInfo info(path);
        const QString normalizedPath = QDir::fromNativeSeparators(path);
        if (normalizedPath.contains("/.git/")
            or normalizedPath.contains("/build/")
            or normalizedPath.contains("/dist/")
            or normalizedPath.contains("/node_modules/")
            or normalizedPath.contains("/.cache/")) {
            continue;
        }
        if (!allowedExtensions.contains(info.suffix().toLower())) {
            continue;
        }
        if (info.size() > 5 * 1024 * 1024) {
            continue;
        }

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

QStringList Qalam::collectProjectFiles() const
{
    QStringList files;
    if (folderPath.isEmpty() or !QDir(folderPath).exists()) {
        return files;
    }

    const QStringList allowedExtensions = {"baa", "baahd", "txt", "md", "json", "cmake", "cpp", "h", "hpp"};
    QDirIterator it(folderPath, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString path = it.next();
        const QString normalized = QDir::fromNativeSeparators(path);
        if (normalized.contains("/.git/")
            or normalized.contains("/build/")
            or normalized.contains("/dist/")
            or normalized.contains("/node_modules/")
            or normalized.contains("/.cache/")) {
            continue;
        }

        QFileInfo info(path);
        if (info.size() > 5 * 1024 * 1024) continue;
        if (!allowedExtensions.contains(info.suffix().toLower())) continue;
        files << QDir::cleanPath(path);
    }

    files.sort(Qt::CaseInsensitive);
    return files;
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
    if (commandId == "run.baa") { runBaa(); return true; }
    if (commandId == "quick.open") { showQuickOpen(); return true; }
    if (commandId == "go.line") { goToLine(); return true; }
    if (commandId == "settings.open") { openSettings(); return true; }
    if (commandId == "help.about") { aboutQalam(); return true; }
    return false;
}

void Qalam::showCommandPalette()
{
    struct CommandItem {
        QString id;
        QString title;
        QString shortcut;
    };

    const QVector<CommandItem> commands = {
        {"file.new", "ملف: ملف جديد", "Ctrl+N"},
        {"file.open", "ملف: فتح ملف", "Ctrl+O"},
        {"folder.open", "ملف: فتح مجلد", ""},
        {"file.save", "ملف: حفظ", "Ctrl+S"},
        {"file.saveAs", "ملف: حفظ باسم", "Ctrl+Shift+S"},
        {"quick.open", "انتقال: فتح سريع للملفات", "Ctrl+P"},
        {"go.line", "انتقال: الذهاب إلى سطر", "Ctrl+G"},
        {"view.search", "عرض: البحث في الملفات", "Ctrl+Shift+F"},
        {"view.sidebar", "عرض: إظهار/إخفاء الشريط الجانبي", "Ctrl+B"},
        {"view.panel", "عرض: إظهار/إخفاء اللوحة السفلية", "Ctrl+J"},
        {"view.problems", "عرض: المشاكل", "Ctrl+Shift+M"},
        {"run.baa", "تشغيل: تشغيل ملف باء", "F5"},
        {"settings.open", "إعدادات: فتح الإعدادات", ""},
        {"help.about", "مساعدة: عن قلم", ""},
    };

    QDialog dialog(this);
    dialog.setWindowTitle("لوحة الأوامر");
    dialog.setLayoutDirection(Qt::RightToLeft);
    dialog.resize(620, 430);

    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto *input = new QLineEdit(&dialog);
    input->setPlaceholderText("اكتب اسم الأمر...");
    input->setClearButtonEnabled(true);
    layout->addWidget(input);

    auto *list = new QListWidget(&dialog);
    list->setLayoutDirection(Qt::RightToLeft);
    layout->addWidget(list, 1);

    auto refill = [list, &commands](const QString &filterText) {
        list->clear();
        const QString filter = filterText.trimmed();
        for (const auto &cmd : commands) {
            const QString searchable = (cmd.title + " " + cmd.shortcut).toLower();
            bool visible = filter.isEmpty();
            if (!visible) {
                visible = true;
                for (const QString &part : filter.toLower().split(' ', Qt::SkipEmptyParts)) {
                    if (!searchable.contains(part)) {
                        visible = false;
                        break;
                    }
                }
            }
            if (!visible) continue;

            auto *item = new QListWidgetItem(cmd.shortcut.isEmpty()
                                             ? cmd.title
                                             : QString("%1\t%2").arg(cmd.title, cmd.shortcut));
            item->setData(Qt::UserRole, cmd.id);
            list->addItem(item);
        }
        if (list->count() > 0) {
            list->setCurrentRow(0);
        }
    };

    refill(QString());
    connect(input, &QLineEdit::textChanged, &dialog, refill);
    connect(input, &QLineEdit::returnPressed, &dialog, [&dialog, list]() {
        if (list->currentItem()) dialog.accept();
    });
    connect(list, &QListWidget::itemActivated, &dialog, [&dialog](QListWidgetItem *) {
        dialog.accept();
    });

    input->setFocus();
    if (dialog.exec() == QDialog::Accepted and list->currentItem()) {
        runCommandById(list->currentItem()->data(Qt::UserRole).toString());
    }
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

    QDialog dialog(this);
    dialog.setWindowTitle("فتح سريع");
    dialog.setLayoutDirection(Qt::RightToLeft);
    dialog.resize(640, 460);

    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto *input = new QLineEdit(&dialog);
    input->setPlaceholderText("اكتب اسم الملف...");
    input->setClearButtonEnabled(true);
    layout->addWidget(input);

    auto *list = new QListWidget(&dialog);
    list->setLayoutDirection(Qt::RightToLeft);
    layout->addWidget(list, 1);

    auto refill = [this, list, files](const QString &filterText) {
        list->clear();
        const QString filter = filterText.trimmed().toLower();
        int added = 0;
        for (const QString &file : files) {
            const QString relative = QDir(folderPath).relativeFilePath(file);
            const QString searchable = relative.toLower();
            bool visible = filter.isEmpty();
            if (!visible) {
                visible = true;
                for (const QString &part : filter.split(' ', Qt::SkipEmptyParts)) {
                    if (!searchable.contains(part)) {
                        visible = false;
                        break;
                    }
                }
            }
            if (!visible) continue;

            auto *item = new QListWidgetItem(relative);
            item->setData(Qt::UserRole, file);
            item->setToolTip(file);
            list->addItem(item);
            if (++added >= 250) break;
        }
        if (list->count() > 0) {
            list->setCurrentRow(0);
        }
    };

    refill(QString());
    connect(input, &QLineEdit::textChanged, &dialog, refill);
    connect(input, &QLineEdit::returnPressed, &dialog, [&dialog, list]() {
        if (list->currentItem()) dialog.accept();
    });
    connect(list, &QListWidget::itemActivated, &dialog, [&dialog](QListWidgetItem *) {
        dialog.accept();
    });

    input->setFocus();
    if (dialog.exec() == QDialog::Accepted and list->currentItem()) {
        openFileFromUi(list->currentItem()->data(Qt::UserRole).toString());
    }
}

void Qalam::updateProblemsStatusBar()
{
    auto *panel = m_layoutManager ? m_layoutManager->panelArea() : nullptr;
    auto *status = m_layoutManager ? m_layoutManager->statusBar() : nullptr;
    if (!panel or !status) return;
    status->setProblemsCount(panel->errorCount(), panel->warningCount());
}

void Qalam::handleBuildOutput(const QString &text)
{
    auto *panel = m_layoutManager ? m_layoutManager->panelArea() : nullptr;
    if (!panel) return;

    QString fallbackFile;
    if (TEditor *editor = currentEditor()) {
        fallbackFile = editor->currentFilePath();
    }

    const QList<QRegularExpression> patterns = {
        QRegularExpression(R"(([^:\n]+):(\d+):(\d+):\s*(error|warning|خطأ|تحذير)[:：]?\s*(.*))",
                           QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption),
        QRegularExpression(R"(([^:\n]+):(\d+):\s*(error|warning|خطأ|تحذير)[:：]?\s*(.*))",
                           QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption),
        QRegularExpression(R"((?:line|السطر)\s+(\d+)(?:[,،]\s*(?:column|العمود)\s+(\d+))?.*(error|warning|خطأ|تحذير)[:：]?\s*(.*))",
                           QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption)
    };

    for (const QString &rawLine : text.split('\n')) {
        const QString lineText = rawLine.trimmed();
        if (lineText.isEmpty()) continue;

        QString file = fallbackFile;
        int line = 1;
        int column = 1;
        QString severity = "error";
        QString message;
        bool matched = false;

        QRegularExpressionMatch match = patterns[0].match(lineText);
        if (match.hasMatch()) {
            file = match.captured(1).trimmed();
            line = match.captured(2).toInt();
            column = qMax(1, match.captured(3).toInt());
            severity = match.captured(4).contains("warning", Qt::CaseInsensitive)
                       or match.captured(4).contains("تحذير") ? "warning" : "error";
            message = match.captured(5).trimmed();
            matched = true;
        }

        if (!matched) {
            match = patterns[1].match(lineText);
            if (match.hasMatch()) {
                file = match.captured(1).trimmed();
                line = match.captured(2).toInt();
                severity = match.captured(3).contains("warning", Qt::CaseInsensitive)
                           or match.captured(3).contains("تحذير") ? "warning" : "error";
                message = match.captured(4).trimmed();
                matched = true;
            }
        }

        if (!matched) {
            match = patterns[2].match(lineText);
            if (match.hasMatch()) {
                line = match.captured(1).toInt();
                column = qMax(1, match.captured(2).toInt());
                severity = match.captured(3).contains("warning", Qt::CaseInsensitive)
                           or match.captured(3).contains("تحذير") ? "warning" : "error";
                message = match.captured(4).trimmed();
                matched = true;
            }
        }

        if (!matched) continue;
        if (message.isEmpty()) message = lineText;

        if (!file.isEmpty()) {
            QFileInfo fileInfo(file);
            if (fileInfo.isRelative()) {
                if (!fallbackFile.isEmpty()) {
                    file = QFileInfo(fallbackFile).absoluteDir().filePath(file);
                } else if (!folderPath.isEmpty()) {
                    file = QDir(folderPath).filePath(file);
                }
            }
            file = QDir::cleanPath(file);
        }

        const QString key = QString("%1|%2|%3|%4|%5").arg(file).arg(line).arg(column).arg(severity, message);
        if (m_seenDiagnostics.contains(key)) continue;
        m_seenDiagnostics.insert(key);

        panel->addProblem(message, file, qMax(1, line), qMax(1, column), severity);
    }

    updateProblemsStatusBar();
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
