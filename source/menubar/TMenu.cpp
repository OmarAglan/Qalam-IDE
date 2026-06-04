#include "TMenu.h"

TMenuBar::TMenuBar(QWidget* parent) : QMenuBar(parent) {
    QMenu* fileMenu = addMenu("ملف");
    QMenu* runMenu = addMenu("تشغيل");
    QMenu* helpMenu = addMenu("مساعدة");

    fileMenu->setMinimumWidth(200);
    runMenu->setMinimumWidth(200);
    helpMenu->setMinimumWidth(200);
    newAction = new QAction("جديد", parent);
    openFileAction = new QAction("فتح ملف", parent);
    openFolderAction = new QAction("فتح مجلد", parent);
    saveAction = new QAction("حفظ", parent);
    saveAsAction = new QAction("حفظ باسم", parent);
    SettingsAction = new QAction("الإعدادات", parent);
    exitAction = new QAction("خروج", parent);

    runAction = new QAction("تشغيل", parent);

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

    runMenu->addAction(runAction);

    helpMenu->addAction(aboutAction);


    connect(newAction, &QAction::triggered, this, &TMenuBar::onNewAction);
    connect(openFileAction, &QAction::triggered, this, &TMenuBar::onOpenFileAction);
    connect(openFolderAction, &QAction::triggered, this, &TMenuBar::onOpenFolderAction);
    connect(saveAction, &QAction::triggered, this, &TMenuBar::onSaveAction);
    connect(saveAsAction, &QAction::triggered, this, &TMenuBar::onSaveAsAction);
    connect(SettingsAction, &QAction::triggered, this, &TMenuBar::onSettingsAction);
    connect(exitAction, &QAction::triggered, this, &TMenuBar::onExitApp);

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
void TMenuBar::onAboutAction() { emit aboutRequested(); }

