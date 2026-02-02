#pragma once

#include <QString>
#include <QColor>
#include <QFont>
#include <QApplication>

class QalamTheme {
public:
    enum class Type {
        Dark,
        Light, // Future support
        HighContrast
    };

    static QalamTheme& instance();

    void apply(QApplication* app);
    QString styleSheet() const;
    
    // Core Colors (VSCode Dark Theme)
    QColor background() const { return QColor("#1e1e1e"); }
    QColor foreground() const { return QColor("#cccccc"); }
    QColor accent() const { return QColor("#007acc"); }
    QColor border() const { return QColor("#3c3c3c"); }
    
    // UI Specifics
    QColor titleBarBackground() const { return QColor("#323233"); }
    QColor sidebarBackground() const { return QColor("#252526"); }
    QColor editorBackground() const { return QColor("#1e1e1e"); }
    
    // Font
    QFont uiFont() const;
    QFont codeFont() const;

private:
    QalamTheme();
    Type m_currentType = Type::Dark;
};
