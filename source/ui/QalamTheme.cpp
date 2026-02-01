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
    // Comprehensive Base Stylesheet
    return R"(
        QMainWindow, QDialog {
            background-color: #1e1e2e;
            color: #d4d4d4;
        }

        QWidget {
            background-color: transparent;
            color: #d4d4d4;
        }

        /* --- Scrollbars (VS Code Style) --- */
        QScrollBar:vertical {
            border: none;
            background: #1e1e2e;
            width: 12px;
            margin: 0px 0px 0px 0px;
        }
        QScrollBar::handle:vertical {
            background: #424250;
            min-height: 20px;
            border-radius: 6px;
            margin: 2px;
        }
        QScrollBar::handle:vertical:hover {
            background: #5b5b6e;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            border: none;
            background: none;
            height: 0px;
        }
        
        QScrollBar:horizontal {
            border: none;
            background: #1e1e2e;
            height: 12px;
        }
        QScrollBar::handle:horizontal {
            background: #424250;
            min-width: 20px;
            border-radius: 6px;
            margin: 2px;
        }
        
        /* --- Lists/Trees --- */
        QListWidget, QTreeWidget {
            background-color: #1e1e2e;
            border: none;
            outline: none;
        }
        QListWidget::item, QTreeWidget::item {
            padding: 5px;
            border-radius: 4px;
        }
        QListWidget::item:hover, QTreeWidget::item:hover {
            background-color: #2a2a3a;
        }
        QListWidget::item:selected, QTreeWidget::item:selected {
            background-color: #37374d;
            color: #ffffff;
        }

        /* --- Menus --- */
        QMenu {
            background-color: #252533;
            border: 1px solid #454555;
            border-radius: 6px;
            padding: 5px;
        }
        QMenu::item {
            padding: 6px 20px;
            border-radius: 4px;
        }
        QMenu::item:selected {
            background-color: #007acc;
            color: white;
        }

        /* --- Buttons --- */
        QPushButton {
            background-color: #007acc;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #0062a3;
        }
        QPushButton:pressed {
            background-color: #00538a;
        }
        QPushButton:disabled {
            background-color: #3e3e42;
            color: #888888;
        }
    )";
}
