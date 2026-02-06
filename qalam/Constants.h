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

    // Session Keys
    const QString SessionKeyOpenFiles = "session/openFiles";
    const QString SessionKeyActiveTab = "session/activeTabIndex";
    const QString SessionKeyFolderPath = "session/folderPath";
    const QString SessionKeyWindowGeometry = "session/windowGeometry";

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
    // UI Colors - Premium Modern Dark (GitHub Dark / VSCode-ish, high-contrast)
    // ==========================================================================
    namespace Colors {
        // Surfaces
        constexpr const char* WindowBackground = "#0d1117";
        constexpr const char* EditorBackground = "#0d1117";
        constexpr const char* SidebarBackground = "#161b22";
        constexpr const char* SidebarHeaderBackground = "#161b22";
        constexpr const char* ConsoleBackground = "#0d1117";
        constexpr const char* MenuBackground = "#161b22";

        // Activity Bar
        constexpr const char* ActivityBarBackground = "#0d1117";
        constexpr const char* ActivityBarBorder = "#21262d";
        constexpr const char* IconInactive = "#8b949e";
        constexpr const char* IconActive = "#c9d1d9";
        constexpr const char* ActivityIndicator = "#1f6feb";
        constexpr const char* ActivityBarBadge = "#f85149";

        // Tabs
        constexpr const char* TabBackground = "#161b22";
        constexpr const char* TabActiveBackground = "#0d1117";
        constexpr const char* TabHoverBackground = "#1f2630";
        constexpr const char* TabBorder = "#21262d";

        // Accent
        constexpr const char* Accent = "#1f6feb";
        constexpr const char* AccentHover = "#388bfd";
        constexpr const char* AccentAlt = "#0969da";

        // Selection
        constexpr const char* Selection = "#1f6feb55";
        constexpr const char* SelectionHighlight = "#1f6feb26";

        // Inputs
        constexpr const char* InputBackground = "#0d1117";
        constexpr const char* Border = "#30363d";
        constexpr const char* BorderSubtle = "#21262d";
        constexpr const char* BorderFocus = "#58a6ff";

        // Text
        constexpr const char* TextPrimary = "#c9d1d9";
        constexpr const char* TextSecondary = "#adbac7";
        constexpr const char* TextMuted = "#8b949e";
        constexpr const char* TextDisabled = "#5c6370";
        constexpr const char* ConsoleText = "#adbac7";

        // Buttons / caption buttons
        constexpr const char* ButtonHover = "#1f2630";
        constexpr const char* ButtonPressed = "#21262d";
        constexpr const char* CaptionButtonHover = "#1f2630";
        constexpr const char* CaptionButtonPressed = "#21262d";
        constexpr const char* CloseButtonHover = "#da3633";
        constexpr const char* CloseButtonPressed = "#f85149";

        // Status Bar
        constexpr const char* StatusBarBackground = "#1f6feb";
        constexpr const char* StatusBarForeground = "#ffffff";
        constexpr const char* StatusBarHover = "#388bfd";
        constexpr const char* StatusBarNoFolder = "#8957e5";

        // Breadcrumb
        constexpr const char* BreadcrumbBackground = "#0d1117";
        constexpr const char* BreadcrumbForeground = "#8b949e";
        constexpr const char* BreadcrumbFocusForeground = "#c9d1d9";

        // Panel / Console Area
        constexpr const char* PanelBackground = "#0d1117";
        constexpr const char* PanelBorder = "#21262d";
        constexpr const char* PanelTabActive = "#0d1117";
        constexpr const char* PanelTabInactive = "transparent";

        // Problems colors
        constexpr const char* ErrorForeground = "#f85149";
        constexpr const char* WarningForeground = "#d29922";
        constexpr const char* InfoForeground = "#58a6ff";

        // Borders (semantic aliases)
        constexpr const char* BorderActive = "#1f6feb";
        constexpr const char* LineNumberBorder = "#1f6feb";

        // Scrollbar
        constexpr const char* ScrollbarBackground = "transparent";
        constexpr const char* ScrollbarThumb = "#6e768166";
        constexpr const char* ScrollbarThumbHover = "#6e7681b3";

        // List / Tree
        constexpr const char* ListHoverBackground = "#1f2630";
        constexpr const char* ListSelectionBackground = "#1f6feb33";
        constexpr const char* ListActiveBackground = "#1f6feb55";
        constexpr const char* ListInactiveBackground = "#2d333b";

        // Title Bar
        constexpr const char* TitleBarBackground = "#0d1117";
        constexpr const char* TitleBarActiveBackground = "#161b22";
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
        constexpr int TitleBarHeight = 38;
        constexpr int CaptionButtonWidth = 46;
        constexpr int CaptionButtonHeight = 38;
        constexpr int CaptionIconSize = 16;

        // Activity Bar (more breathing room)
        constexpr int ActivityBarWidth = 56;
        constexpr int ActivityBarIconSize = 26;
        constexpr int ActivityBarButtonSize = 56;
        constexpr int ActivityIndicatorWidth = 3;

        // Sidebar
        constexpr int SidebarDefaultWidth = 260;
        constexpr int SidebarMinWidth = 180;
        constexpr int SidebarMaxWidth = 520;
        constexpr int SidebarHeaderHeight = 32;
        constexpr int SidebarSectionHeaderHeight = 22;

        // Editor Area
        constexpr int TabBarHeight = 36;
        constexpr int BreadcrumbHeight = 22;
        constexpr int IconSize = 16;

        // Panel Area
        constexpr int PanelDefaultHeight = 210;
        constexpr int PanelMinHeight = 110;
        constexpr int PanelTabHeight = 30;
        
        // Status Bar
        constexpr int StatusBarHeight = 22;
        constexpr int StatusBarItemPadding = 8;
        
        // General
        constexpr int BorderRadius = 6;
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
