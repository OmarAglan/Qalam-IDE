#pragma once
#include <QString>

namespace Constants {
    const QString OrgName = "Alif";
    const QString AppName = "Qalam";
    
    // Settings Keys
    const QString SettingsKeyRecentFiles = "RecentFiles";
    const QString SettingsKeyFontSize = "editorFontSize";
    const QString SettingsKeyFontType = "editorFontType";
    const QString SettingsKeyTheme = "editorCodeTheme";
    const QString SettingsKeyCompilerPath = "compilerPath";

    // File Extensions
    const QString ProjectExtension = ".baa";
    const QString HeaderExtension = ".baahd";
    const QString BackupExtension = ".~";

    // UI Strings
    const QString NewFileLabel = "غير معنون";

    // Defaults
    const int DefaultFontSize = 18;
    const QString DefaultFontType = "Kawkab Mono";

    // ==========================================================================
    // UI Colors - Dark Theme Palette
    // ==========================================================================
    namespace Colors {
        // Primary backgrounds
        constexpr const char* WindowBackground = "#1e202e";
        constexpr const char* EditorBackground = "#141520";
        constexpr const char* SidebarBackground = "#232629";
        constexpr const char* ConsoleBackground = "#03091A";
        constexpr const char* StatusBarBackground = "#333333";
        constexpr const char* MenuBackground = "#252526";
        
        // Tab colors
        constexpr const char* TabBackground = "#2d2d30";
        constexpr const char* TabActiveBackground = "#1e1e1e";
        constexpr const char* TabHoverBackground = "#3e3e42";
        
        // Accent colors
        constexpr const char* Accent = "#007acc";
        constexpr const char* AccentAlt = "#0078d7";
        constexpr const char* Selection = "#094771";
        
        // Button states
        constexpr const char* ButtonHover = "#4f5357";
        constexpr const char* ButtonPressed = "#2a2d31";
        constexpr const char* CaptionButtonHover = "#3e3e42";
        constexpr const char* CaptionButtonPressed = "#2d2d32";
        constexpr const char* CloseButtonHover = "#e81123";
        constexpr const char* CloseButtonPressed = "#f1707a";
        
        // Text colors
        constexpr const char* TextPrimary = "#ffffff";
        constexpr const char* TextSecondary = "#cccccc";
        constexpr const char* TextMuted = "#909090";
        constexpr const char* ConsoleText = "#DEE8FF";
        
        // Borders
        constexpr const char* Border = "#454545";
        constexpr const char* BorderActive = "#444444";
        constexpr const char* LineNumberBorder = "#10a8f4";
    }

    // ==========================================================================
    // Font Sizes
    // ==========================================================================
    namespace Fonts {
        constexpr int ConsoleSize = 15;
        constexpr int UISize = 12;
        constexpr int StatusBarSize = 6;       // pt
        constexpr int TabSize = 9;             // pt
        constexpr int TreeViewSize = 10;       // pt
        constexpr int EditorMinSize = 12;
        constexpr int EditorMaxSize = 36;
    }

    // ==========================================================================
    // Layout Dimensions
    // ==========================================================================
    namespace Layout {
        constexpr int TitleBarHeight = 40;
        constexpr int CaptionButtonWidth = 46;
        constexpr int CaptionButtonHeight = 40;
        constexpr int IconSize = 24;
        constexpr int CaptionIconSize = 16;
        constexpr int ToolButtonSize = 40;
        constexpr int ToolbarIconSize = 30;
        constexpr int ToolbarPadding = 5;
        constexpr int ToolbarSpacing = 10;
        constexpr int BorderRadius = 6;
        constexpr int TabBorderRadius = 4;
        constexpr int SplitterWidth = 1;
        constexpr int MenuItemPadding = 5;
        
        // Autocomplete popup
        constexpr int PopupMinWidth = 295;
        constexpr int PopupMaxWidth = 355;
        constexpr int PopupBasePadding = 250;  // Base padding for text + scrollbar + margins
    }

    // ==========================================================================
    // Timing Constants (milliseconds)
    // ==========================================================================
    namespace Timing {
        constexpr int FlushInterval = 25;
        constexpr int ProcessTerminateTimeout = 500;
        constexpr int ProcessKillTimeout = 200;
        constexpr int AutoSaveInterval = 30000;
    }

    // ==========================================================================
    // Console Limits
    // ==========================================================================
    namespace Console {
        constexpr int MaxBufferLines = 10000;
        constexpr int MaxPendingLines = 5000;
    }
}
