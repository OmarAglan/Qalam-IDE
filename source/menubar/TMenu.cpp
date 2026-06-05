#include "TMenu.h"

TMenuBar::TMenuBar(QWidget* parent) : QMenuBar(parent) {
    setLayoutDirection(Qt::LeftToRight);

    QMenu* fileMenu = addMenu("ملف");
    QMenu* editMenu = addMenu("تحرير");
    QMenu* viewMenu = addMenu("عرض");
    QMenu* runMenu = addMenu("تشغيل");
    QMenu* terminalMenu = addMenu("طرفية");
    QMenu* helpMenu = addMenu("مساعدة");

    fileMenu->setMinimumWidth(210);
    editMenu->setMinimumWidth(210);
    viewMenu->setMinimumWidth(230);
    runMenu->setMinimumWidth(200);
    terminalMenu->setMinimumWidth(200);
    helpMenu->setMinimumWidth(200);
    newAction = new QAction("جديد", parent);
    openFileAction = new QAction("فتح ملف", parent);
    openFolderAction = new QAction("فتح مجلد", parent);
    saveAction = new QAction("حفظ", parent);
    saveAsAction = new QAction("حفظ باسم", parent);
    SettingsAction = new QAction("الإعدادات", parent);
    exitAction = new QAction("خروج", parent);

    commandPaletteAction = new QAction("لوحة الأوامر...", parent);
    quickOpenAction = new QAction("فتح سريع للملفات...", parent);
    searchInFilesAction = new QAction("بحث في الملفات", parent);
    toggleSidebarAction = new QAction("إظهار/إخفاء الشريط الجانبي", parent);
    toggleTerminalAction = new QAction("إظهار/إخفاء الطرفية", parent);

    runAction = new QAction("تشغيل", parent);

    aboutAction = new QAction("عن المحرر", parent);

    newAction->setShortcut(QKeySequence("Ctrl+N"));
    openFileAction->setShortcut(QKeySequence("Ctrl+O"));
    openFolderAction->setShortcut(QKeySequence("Ctrl+K, Ctrl+O"));
    saveAction->setShortcut(QKeySequence("Ctrl+S"));
    saveAsAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
    commandPaletteAction->setShortcut(QKeySequence("Ctrl+Shift+P"));
    quickOpenAction->setShortcut(QKeySequence("Ctrl+P"));
    searchInFilesAction->setShortcut(QKeySequence("Ctrl+Shift+F"));
    toggleSidebarAction->setShortcut(QKeySequence("Ctrl+B"));
    toggleTerminalAction->setShortcut(QKeySequence("Ctrl+J"));
    runAction->setShortcut(QKeySequence("F5"));


    fileMenu->addAction(newAction);
    fileMenu->addAction(openFileAction);
    fileMenu->addAction(openFolderAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(SettingsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    editMenu->addAction(commandPaletteAction);
    editMenu->addAction(quickOpenAction);

    viewMenu->addAction(searchInFilesAction);
    viewMenu->addAction(toggleSidebarAction);

    runMenu->addAction(runAction);

    terminalMenu->addAction(toggleTerminalAction);

    helpMenu->addAction(aboutAction);


    connect(newAction, &QAction::triggered, this, &TMenuBar::onNewAction);
    connect(openFileAction, &QAction::triggered, this, &TMenuBar::onOpenFileAction);
    connect(openFolderAction, &QAction::triggered, this, &TMenuBar::onOpenFolderAction);
    connect(saveAction, &QAction::triggered, this, &TMenuBar::onSaveAction);
    connect(saveAsAction, &QAction::triggered, this, &TMenuBar::onSaveAsAction);
    connect(SettingsAction, &QAction::triggered, this, &TMenuBar::onSettingsAction);
    connect(exitAction, &QAction::triggered, this, &TMenuBar::onExitApp);

    connect(commandPaletteAction, &QAction::triggered, this, &TMenuBar::onCommandPaletteAction);
    connect(quickOpenAction, &QAction::triggered, this, &TMenuBar::onQuickOpenAction);
    connect(searchInFilesAction, &QAction::triggered, this, &TMenuBar::onSearchInFilesAction);
    connect(toggleSidebarAction, &QAction::triggered, this, &TMenuBar::onToggleSidebarAction);
    connect(toggleTerminalAction, &QAction::triggered, this, &TMenuBar::onToggleTerminalAction);

    connect(runAction, &QAction::triggered, this, &TMenuBar::onRunAction);

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
void TMenuBar::onCommandPaletteAction() { emit commandPaletteRequested(); }
void TMenuBar::onQuickOpenAction() { emit quickOpenRequested(); }
void TMenuBar::onSearchInFilesAction() { emit searchInFilesRequested(); }
void TMenuBar::onToggleSidebarAction() { emit toggleSidebarRequested(); }
void TMenuBar::onToggleTerminalAction() { emit toggleTerminalRequested(); }
void TMenuBar::onAboutAction() { emit aboutRequested(); }

