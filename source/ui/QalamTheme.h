#pragma once

#include <QString>
#include <QColor>
#include <QFont>
#include <QApplication>

/**
 * @class QalamTheme
 * @brief Centralized theme manager for Qalam IDE
 * 
 * This singleton class provides a single source of truth for all UI styling.
 * It reads color and layout values from Constants.h and generates consistent
 * stylesheets for all components.
 * 
 * Usage:
 *   - Call QalamTheme::instance().apply(app) in main() to apply global styles
 *   - Components can call QalamTheme::getXXXStylesheet() for component-specific styles
 */
class QalamTheme {
public:
    enum class Type {
        Dark,       // VSCode Dark+ (current default)
        Light,      // Future support
        HighContrast
    };

    static QalamTheme& instance();

    // Apply theme to entire application
    void apply(QApplication* app);
    
    // Get the current theme type
    Type currentType() const { return m_currentType; }
    
    // Set theme type (for future theme switching support)
    void setType(Type type) { m_currentType = type; }
    
    // ==========================================================================
    // Global Stylesheet
    // ==========================================================================
    QString globalStyleSheet() const;
    
    // ==========================================================================
    // Component-Specific Stylesheets
    // These are called by individual components in their applyStyles() methods
    // ==========================================================================
    static QString activityBarStyleSheet();
    static QString sidebarStyleSheet();
    static QString explorerViewStyleSheet();
    static QString searchViewStyleSheet();
    static QString breadcrumbStyleSheet();
    static QString panelAreaStyleSheet();
    static QString statusBarStyleSheet();
    static QString titleBarStyleSheet();
    static QString editorStyleSheet();
    static QString tabBarStyleSheet();
    static QString consoleStyleSheet();
    
    // ==========================================================================
    // Color Accessors (for code that needs individual colors)
    // ==========================================================================
    QColor background() const;
    QColor foreground() const;
    QColor accent() const;
    QColor border() const;
    QColor sidebarBackground() const;
    QColor editorBackground() const;
    QColor statusBarBackground() const;
    QColor titleBarBackground() const;
    
    // ==========================================================================
    // Font Accessors
    // ==========================================================================
    QFont uiFont() const;
    QFont codeFont() const;
    QFont consoleFont() const;

private:
    QalamTheme();
    Type m_currentType = Type::Dark;
    
    // Internal stylesheet generation helpers
    static QString scrollbarStyles();
    static QString menuStyles();
    static QString buttonStyles();
    static QString inputStyles();
    static QString listStyles();
    static QString tooltipStyles();
};
