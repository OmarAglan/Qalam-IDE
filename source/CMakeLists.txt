add_library(qalam_core STATIC
    # Text Editor
    texteditor/TEditor.cpp
    texteditor/highlighter/TLexer.cpp
    texteditor/highlighter/TSyntaxDefinition.cpp
    texteditor/highlighter/TSyntaxHighlighter.cpp
    texteditor/autocomplete/AutoComplete.cpp
    texteditor/autocomplete/AutoCompleteUI.cpp
    # Components
    components/TFlatButton.cpp
    components/TSearchPanel.cpp
    # Console
    console/TConsole.cpp
    console/ProcessWorker.cpp
    # Menu
    menubar/TMenu.cpp
    # Settings
    settings/TSettings.cpp
    # Sidebar
    sidebar/TExplorerView.cpp
    sidebar/TSearchView.cpp
    # UI Components
    ui/QalamTheme.cpp
    ui/TTitleBar.cpp
    ui/QalamWindow.cpp
    ui/TActivityBar.cpp
    ui/TSidebar.cpp
    ui/TBreadcrumb.cpp
    ui/TPanelArea.cpp
    ui/TStatusBar.cpp
    # Managers
    managers/FileManager.cpp
    managers/BuildManager.cpp
)

target_include_directories(qalam_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../qalam
    texteditor
    texteditor/highlighter
    texteditor/autocomplete
    components
    console
    menubar
    pages
    settings
    sidebar
    ui
    managers
)

target_link_libraries(qalam_core PUBLIC Qt6::Widgets Qt6::Gui Qt6::Core)

if(WIN32)
    target_link_libraries(qalam_core PUBLIC dwmapi user32)
endif()
