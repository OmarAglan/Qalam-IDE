#include "TMenu.h"
#include <QMenu>

TMenuBar::TMenuBar(QWidget* parent) : QMenuBar(parent) {
    setLayoutDirection(Qt::LeftToRight);

    QMenu* fileMenu = addMenu("ملف");
    QMenu* editMenu = addMenu("تحرير");
    QMenu* viewMenu = addMenu("عرض");
    QMenu* runMenu = addMenu("تشغيل");
    QMenu* terminalMenu = addMenu("الطرفية");
    QMenu* helpMenu = addMenu("مساعدة");

    fileMenu->setMinimumWidth(220);
    editMenu->setMinimumWidth(220);
    viewMenu->setMinimumWidth(240);
    runMenu->setMinimumWidth(200);
    terminalMenu->setMinimumWidth(220);
    helpMenu->setMinimumWidth(200);
    newAction = new QAction("جديد", parent);
    openFileAction = new QAction("فتح ملف", parent);
    openFolderAction = new QAction("فتح مجلد", parent);
    saveAction = new QAction("حفظ", parent);
    saveAsAction = new QAction("حفظ باسم", parent);
    SettingsAction = new QAction("الإعدادات", parent);
    exitAction = new QAction("خروج", parent);

    runAction = new QAction("تشغيل", parent);

    commandPaletteAction = new QAction("لوحة الأوامر", parent);
    quickOpenAction = new QAction("فتح سريع", parent);
    findAction = new QAction("بحث في الملف", parent);
    findInFilesAction = new QAction("بحث في الملفات", parent);
    goToLineAction = new QAction("الذهاب إلى سطر", parent);
    toggleSidebarAction = new QAction("إظهار/إخفاء الشريط الجانبي", parent);
    togglePanelAction = new QAction("إظهار/إخفاء اللوحة", parent);
    problemsAction = new QAction("المشاكل", parent);

    newAction->setShortcut(QKeySequence("Ctrl+N"));
    openFileAction->setShortcut(QKeySequence("Ctrl+O"));
    saveAction->setShortcut(QKeySequence("Ctrl+S"));
    saveAsAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
    runAction->setShortcut(QKeySequence("F5"));
    commandPaletteAction->setShortcut(QKeySequence("Ctrl+Shift+P"));
    quickOpenAction->setShortcut(QKeySequence("Ctrl+P"));
    findAction->setShortcut(QKeySequence("Ctrl+F"));
    findInFilesAction->setShortcut(QKeySequence("Ctrl+Shift+F"));
    goToLineAction->setShortcut(QKeySequence("Ctrl+G"));
    toggleSidebarAction->setShortcut(QKeySequence("Ctrl+B"));
    togglePanelAction->setShortcut(QKeySequence("Ctrl+J"));
    problemsAction->setShortcut(QKeySequence("Ctrl+Shift+M"));

    aboutAction = new QAction("عن المحرر", parent);


    fileMenu->addAction(newAction);
    fileMenu->addAction(openFileAction);
    fileMenu->addAction(openFolderAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(SettingsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    editMenu->addAction(findAction);
    editMenu->addAction(findInFilesAction);
    editMenu->addSeparator();
    editMenu->addAction(goToLineAction);

    viewMenu->addAction(commandPaletteAction);
    viewMenu->addAction(quickOpenAction);
    viewMenu->addSeparator();
    viewMenu->addAction(toggleSidebarAction);
    viewMenu->addAction(togglePanelAction);
    viewMenu->addAction(problemsAction);

    runMenu->addAction(runAction);

    terminalMenu->addAction(togglePanelAction);

    helpMenu->addAction(aboutAction);


    connect(newAction, &QAction::triggered, this, &TMenuBar::onNewAction);
    connect(openFileAction, &QAction::triggered, this, &TMenuBar::onOpenFileAction);
    connect(openFolderAction, &QAction::triggered, this, &TMenuBar::onOpenFolderAction);
    connect(saveAction, &QAction::triggered, this, &TMenuBar::onSaveAction);
    connect(saveAsAction, &QAction::triggered, this, &TMenuBar::onSaveAsAction);
    connect(SettingsAction, &QAction::triggered, this, &TMenuBar::onSettingsAction);
    connect(exitAction, &QAction::triggered, this, &TMenuBar::onExitApp);

    connect(runAction, &QAction::triggered, this, &TMenuBar::onRunAction);
    connect(commandPaletteAction, &QAction::triggered, this, &TMenuBar::onCommandPaletteAction);
    connect(quickOpenAction, &QAction::triggered, this, &TMenuBar::onQuickOpenAction);
    connect(findAction, &QAction::triggered, this, &TMenuBar::onFindAction);
    connect(findInFilesAction, &QAction::triggered, this, &TMenuBar::onFindInFilesAction);
    connect(goToLineAction, &QAction::triggered, this, &TMenuBar::onGoToLineAction);
    connect(toggleSidebarAction, &QAction::triggered, this, &TMenuBar::onToggleSidebarAction);
    connect(togglePanelAction, &QAction::triggered, this, &TMenuBar::onTogglePanelAction);
    connect(problemsAction, &QAction::triggered, this, &TMenuBar::onProblemsAction);

    connect(aboutAction, &QAction::triggered, this, &TMenuBar::onAboutAction);
}

void TMenuBar::onNewAction() { emit newRequested(); }
void TMenuBar::onOpenFileAction() { emit openFileRequested(); }
void TMenuBar::onOpenFolderAction() { emit openFolderRequested(); }
void TMenuBar::onSaveAction() { emit saveRequested(); }
void TMenuBar::onSaveAsAction() { emit saveAsRequested(); }
void TMenuBar::onSettingsAction() { emit settingsRequest(); }
void TMenuBar::onExitApp() { emit exitRequested(); }
void TMenuBar::onRunAction() { emit runRequested(); }
void TMenuBar::onAboutAction() { emit aboutRequested(); }


void TMenuBar::onCommandPaletteAction() { emit commandPaletteRequested(); }
void TMenuBar::onQuickOpenAction() { emit quickOpenRequested(); }
void TMenuBar::onFindAction() { emit findRequested(); }
void TMenuBar::onFindInFilesAction() { emit findInFilesRequested(); }
void TMenuBar::onGoToLineAction() { emit goToLineRequested(); }
void TMenuBar::onToggleSidebarAction() { emit toggleSidebarRequested(); }
void TMenuBar::onTogglePanelAction() { emit togglePanelRequested(); }
void TMenuBar::onProblemsAction() { emit problemsRequested(); }
