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
    
    // Core Colors
    QColor background() const { return QColor("#1e1e2e"); }
    QColor foreground() const { return QColor("#d4d4d4"); }
    QColor accent() const { return QColor("#4fc3f7"); }
    QColor border() const { return QColor("#2d2d3d"); }
    
    // UI Specifics
    QColor titleBarBackground() const { return QColor("#1e1e2e"); } // or slightly darker #181825
    QColor sidebarBackground() const { return QColor("#252533"); }
    QColor editorBackground() const { return QColor("#1e1e2e"); }
    
    // Font
    QFont uiFont() const;
    QFont codeFont() const;

private:
    QalamTheme();
    Type m_currentType = Type::Dark;
};
