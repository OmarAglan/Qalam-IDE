#pragma once
#include <QString>

namespace Constants {
    const QString OrgName = "Alif";
    const QString AppName = "Qalam";
    const QString AppVersion = "3.3.0";
    
    // Settings Keys
    const QString SettingsKeyRecentFiles = "RecentFiles";
    const QString SettingsKeyFontSize = "editorFontSize";
    const QString SettingsKeyFontType = "editorFontType";
    const QString SettingsKeyTheme = "editorCodeTheme";
    const QString SettingsKeyCompilerPath = "compilerPath";
    const QString SettingsKeySidebarWidth = "sidebarWidth";
    const QString SettingsKeyPanelHeight = "panelHeight";
    const QString SettingsKeyShowWelcome = "ShowWelcomeOnStartup";

    // File Extensions
    const QString ProjectExtension = ".baa";
    const QString HeaderExtension = ".baahd";
    const QString BackupExtension = ".~";

    // UI Strings (Arabic)
    const QString NewFileLabel = "غير معنون";
    const QString ExplorerLabel = "المستكشف";
    const QString SearchLabel = "البحث";
    const QString SettingsLabel = "الإعدادات";
    const QString ProblemsLabel = "المشاكل";
    const QString OutputLabel = "المخرجات";
    const QString TerminalLabel = "الطرفية";
    const QString OpenEditorsLabel = "الملفات المفتوحة";
    const QString NoFolderOpenLabel = "لم يتم فتح مجلد";

    // Defaults
    const int DefaultFontSize = 18;
    const QString DefaultFontType = "Kawkab Mono";

    // ==========================================================================
    // UI Colors - VSCode Dark Theme Palette
    // ==========================================================================
    namespace Colors {
        // Primary backgrounds (VSCode Dark+ theme)
        constexpr const char* WindowBackground = "#1e1e1e";
        constexpr const char* EditorBackground = "#1e1e1e";
        constexpr const char* SidebarBackground = "#252526";
        constexpr const char* SidebarHeaderBackground = "#252526";
        constexpr const char* ConsoleBackground = "#1e1e1e";
        constexpr const char* MenuBackground = "#252526";
        
        // Activity Bar (VSCode style)
        constexpr const char* ActivityBarBackground = "#333333";
        constexpr const char* ActivityBarBorder = "#252526";
        constexpr const char* IconInactive = "#858585";
        constexpr const char* IconActive = "#ffffff";
        constexpr const char* ActivityIndicator = "#007acc";
        
        // Tab colors
        constexpr const char* TabBackground = "#2d2d2d";
        constexpr const char* TabActiveBackground = "#1e1e1e";
        constexpr const char* TabHoverBackground = "#2a2d2e";
        constexpr const char* TabBorder = "#252526";
        
        // Accent colors
        constexpr const char* Accent = "#007acc";
        constexpr const char* AccentHover = "#1c97ea";
        constexpr const char* AccentAlt = "#0078d7";
        constexpr const char* Selection = "#264f78";
        constexpr const char* SelectionHighlight = "#add6ff26";
        
        // Button states
        constexpr const char* ButtonHover = "#3e3e42";
        constexpr const char* ButtonPressed = "#2d2d30";
        constexpr const char* CaptionButtonHover = "#3e3e42";
        constexpr const char* CaptionButtonPressed = "#2d2d32";
        constexpr const char* CloseButtonHover = "#e81123";
        constexpr const char* CloseButtonPressed = "#f1707a";
        
        // Text colors
        constexpr const char* TextPrimary = "#ffffff";
        constexpr const char* TextSecondary = "#cccccc";
        constexpr const char* TextMuted = "#858585";
        constexpr const char* TextDisabled = "#5a5a5a";
        constexpr const char* ConsoleText = "#cccccc";
        
        // Status Bar (Blue like VSCode)
        constexpr const char* StatusBarBackground = "#007acc";
        constexpr const char* StatusBarForeground = "#ffffff";
        constexpr const char* StatusBarHover = "#1c97ea";
        constexpr const char* StatusBarNoFolder = "#68217a";  // Purple when no folder open
        
        // Breadcrumb
        constexpr const char* BreadcrumbBackground = "#1e1e1e";
        constexpr const char* BreadcrumbForeground = "#cccccc";
        constexpr const char* BreadcrumbFocusForeground = "#e0e0e0";
        
        // Panel/Console Area
        constexpr const char* PanelBackground = "#1e1e1e";
        constexpr const char* PanelBorder = "#80808059";
        constexpr const char* PanelTabActive = "#1e1e1e";
        constexpr const char* PanelTabInactive = "transparent";
        
        // Problems colors
        constexpr const char* ErrorForeground = "#f14c4c";
        constexpr const char* WarningForeground = "#cca700";
        constexpr const char* InfoForeground = "#3794ff";
        
        // Borders
        constexpr const char* Border = "#3c3c3c";
        constexpr const char* BorderActive = "#007acc";
        constexpr const char* BorderSubtle = "#3c3c3c";
        constexpr const char* LineNumberBorder = "#007acc";
        
        // Scrollbar
        constexpr const char* ScrollbarBackground = "transparent";
        constexpr const char* ScrollbarThumb = "#79797966";
        constexpr const char* ScrollbarThumbHover = "#646464b3";
        
        // List/Tree
        constexpr const char* ListHoverBackground = "#2a2d2e";
        constexpr const char* ListActiveBackground = "#094771";
        constexpr const char* ListInactiveBackground = "#37373d";
        
        // Title Bar
        constexpr const char* TitleBarBackground = "#323233";
        constexpr const char* TitleBarActiveBackground = "#3c3c3c";
    }

    // ==========================================================================
    // Font Sizes
    // ==========================================================================
    namespace Fonts {
        constexpr int ConsoleSize = 13;
        constexpr int UISize = 13;
        constexpr int StatusBarSize = 12;
        constexpr int TabSize = 13;
        constexpr int TreeViewSize = 13;
        constexpr int BreadcrumbSize = 12;
        constexpr int SectionHeaderSize = 11;
        constexpr int EditorMinSize = 12;
        constexpr int EditorMaxSize = 36;
    }

    // ==========================================================================
    // Layout Dimensions
    // ==========================================================================
    namespace Layout {
        // Title Bar
        constexpr int TitleBarHeight = 40;
        constexpr int CaptionButtonWidth = 46;
        constexpr int CaptionButtonHeight = 40;
        constexpr int CaptionIconSize = 16;
        
        // Activity Bar
        constexpr int ActivityBarWidth = 48;
        constexpr int ActivityBarIconSize = 24;
        constexpr int ActivityBarButtonSize = 48;
        constexpr int ActivityIndicatorWidth = 2;
        
        // Sidebar
        constexpr int SidebarDefaultWidth = 250;
        constexpr int SidebarMinWidth = 170;
        constexpr int SidebarMaxWidth = 500;
        constexpr int SidebarHeaderHeight = 35;
        constexpr int SidebarSectionHeaderHeight = 22;
        
        // Editor Area
        constexpr int TabBarHeight = 35;
        constexpr int BreadcrumbHeight = 22;
        constexpr int IconSize = 16;
        
        // Panel Area
        constexpr int PanelDefaultHeight = 200;
        constexpr int PanelMinHeight = 100;
        constexpr int PanelTabHeight = 35;
        
        // Status Bar
        constexpr int StatusBarHeight = 22;
        constexpr int StatusBarItemPadding = 8;
        
        // General
        constexpr int BorderRadius = 4;
        constexpr int SplitterWidth = 1;
        constexpr int ScrollbarWidth = 10;
        
        // Autocomplete popup
        constexpr int PopupMinWidth = 295;
        constexpr int PopupMaxWidth = 355;
        constexpr int PopupBasePadding = 250;
    }

    // ==========================================================================
    // Timing Constants (milliseconds)
    // ==========================================================================
    namespace Timing {
        constexpr int FlushInterval = 25;
        constexpr int ProcessTerminateTimeout = 500;
        constexpr int ProcessKillTimeout = 200;
        constexpr int AutoSaveInterval = 30000;
        constexpr int SearchDebounce = 300;
        constexpr int HoverDelay = 500;
    }

    // ==========================================================================
    // Console Limits
    // ==========================================================================
    namespace Console {
        constexpr int MaxBufferLines = 10000;
        constexpr int MaxPendingLines = 5000;
    }
}
