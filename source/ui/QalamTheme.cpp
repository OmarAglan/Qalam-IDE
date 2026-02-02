#include "QalamTheme.h"
#include <QFontDatabase>

QalamTheme& QalamTheme::instance() {
    static QalamTheme instance;
    return instance;
}

QalamTheme::QalamTheme() {}

void QalamTheme::apply(QApplication* app) {
    app->setStyleSheet(styleSheet());
    app->setFont(uiFont());
}

QFont QalamTheme::uiFont() const {
    // Priority: Tajawal (Arabic) -> Segoe UI -> Sans Serif
    QStringList families;
    families << "Tajawal" << "Segoe UI" << "Roboto" << "sans-serif";
    
    QFont font;
    font.setFamilies(families);
    font.setPointSize(10);
    return font;
}

QFont QalamTheme::codeFont() const {
    QStringList families;
    families << "Consolas" << "JetBrains Mono" << "Courier New" << "monospace";
    
    QFont font;
    font.setFamilies(families);
    font.setPointSize(11);
    font.setFixedPitch(true);
    return font;
}

QString QalamTheme::styleSheet() const {
    // VSCode Dark Theme Stylesheet
    return R"(
        QMainWindow, QDialog {
            background-color: #1e1e1e;
            color: #cccccc;
        }

        QWidget {
            background-color: transparent;
            color: #cccccc;
            font-family: 'Segoe UI', 'Tajawal', sans-serif;
        }

        /* --- Scrollbars (VS Code Style) --- */
        QScrollBar:vertical {
            border: none;
            background: transparent;
            width: 14px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: #79797966;
            min-height: 20px;
            border-radius: 7px;
            margin: 3px;
        }
        QScrollBar::handle:vertical:hover {
            background: #646464b3;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            border: none;
            background: none;
            height: 0px;
        }
        
        QScrollBar:horizontal {
            border: none;
            background: transparent;
            height: 14px;
        }
        QScrollBar::handle:horizontal {
            background: #79797966;
            min-width: 20px;
            border-radius: 7px;
            margin: 3px;
        }
        QScrollBar::handle:horizontal:hover {
            background: #646464b3;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            border: none;
            background: none;
            width: 0px;
        }
        
        /* --- Lists/Trees --- */
        QListWidget, QTreeWidget, QTreeView {
            background-color: #252526;
            border: none;
            outline: none;
            color: #cccccc;
        }
        QListWidget::item, QTreeWidget::item, QTreeView::item {
            padding: 4px 8px;
        }
        QListWidget::item:hover, QTreeWidget::item:hover, QTreeView::item:hover {
            background-color: #2a2d2e;
        }
        QListWidget::item:selected, QTreeWidget::item:selected, QTreeView::item:selected {
            background-color: #094771;
            color: #ffffff;
        }

        /* --- Menus --- */
        QMenu {
            background-color: #252526;
            border: 1px solid #454545;
            border-radius: 4px;
            padding: 4px 0px;
        }
        QMenu::item {
            padding: 6px 30px 6px 20px;
        }
        QMenu::item:selected {
            background-color: #094771;
            color: #ffffff;
        }
        QMenu::separator {
            height: 1px;
            background: #454545;
            margin: 4px 10px;
        }

        /* --- Buttons --- */
        QPushButton {
            background-color: #0e639c;
            color: white;
            border: none;
            padding: 6px 14px;
            border-radius: 2px;
        }
        QPushButton:hover {
            background-color: #1177bb;
        }
        QPushButton:pressed {
            background-color: #0d5289;
        }
        QPushButton:disabled {
            background-color: #3e3e42;
            color: #6e6e6e;
        }
        
        /* --- Line Edits --- */
        QLineEdit {
            background-color: #3c3c3c;
            border: 1px solid #3c3c3c;
            border-radius: 2px;
            padding: 4px 8px;
            color: #cccccc;
            selection-background-color: #264f78;
        }
        QLineEdit:focus {
            border: 1px solid #007acc;
        }
        
        /* --- Tool Tips --- */
        QToolTip {
            background-color: #252526;
            border: 1px solid #454545;
            color: #cccccc;
            padding: 4px 8px;
        }
    )";
}
