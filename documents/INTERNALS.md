# Qalam IDE Internals

This document describes the internal architecture and component relationships of the Qalam IDE.

## High-Level Architecture

Qalam IDE is built using **Qt 6 (C++23)** and follows a modular design. The project is split into two main parts:

1. **`qalam_core` (Static Library):** Contains all the reusable logic and UI components (Editor, Console, Settings, etc.).
2. **`Qalam` (Application):** The main entry point that assembles the components into the final IDE.

## Directory Structure

```
/documents              # Documentation files
/qalam                  # Main application entry (main.cpp, Qalam.cpp, Qalam.h, resources.qrc)
/source                 # qalam_core static library
├── texteditor          # Custom editor components
│   ├── highlighter     # TLexer (state-machine), TSyntaxHighlighter, LanguageDefinition
│   └── autocomplete    # AutoComplete strategies and UI
├── console             # TConsole, ProcessWorker
├── components          # TFlatButton, TSearchPanel
├── menubar             # TMenuBar
├── settings            # TSettings
├── sidebar             # TExplorerView, TSearchView
├── ui                  # QalamWindow, QalamTheme, TTitleBar, TActivityBar, TSidebar, TPanelArea, TStatusBar, TBreadcrumb
├── managers            # FileManager, BuildManager, SessionManager, LayoutManager
└── pages               # TWelcomeWindow
```

### Key Components

#### 1. Text Editor (`source/texteditor`)
The heart of Qalam is `TEditor`, a custom `QPlainTextEdit` subclass.
- **`TLexer`:** A state-based lexer using `QStringView` for zero-copy tokenization. Tokens are used for syntax highlighting and auto-completion.
- **`TSyntaxHighlighter`:** Integrates `TLexer` with Qt's `QSyntaxHighlighter` for real-time coloring.
- **`AutoComplete`:** Provides context-aware suggestions using the Strategy pattern with 5 strategies: `KeywordStrategy`, `BuiltinStrategy`, `SnippetStrategy`, `PreprocessorStrategy`, `DynamicWordStrategy`.
- **`TBracketHandler`:** Handles bracket/quote auto-pairing, skip-over, and selection wrapping.
- **`TSnippetManager`:** Manages code snippets with Tab/Enter placeholder navigation.
- **`TAutoSave`:** Handles automatic backup to `.~` files.

#### 2. Console (`source/console`)
`TConsole` provides an interactive terminal.
- **`ProcessWorker`:** Runs in a background `QThread` with mutex-protected buffers for the compiler or external scripts, keeping the UI responsive.

#### 3. Framing & Theme (`source/ui`)
- **`QalamWindow`:** Handles the frameless window implementation with native Windows snap/shadow and RTL layout.
- **`QalamTheme`:** Singleton managing CSS-based themes for consistent styling across all components.
- **`TTitleBar`:** Custom title bar with embedded menu and system buttons.
- **`TSidebar`:** Stacked widget for Explorer and Search views.
- **`TPanelArea`:** Bottom panel hosting Problems, Output, and Terminal tabs.

#### 4. Managers (`source/managers`)
- **`FileManager`:** Handles file open/save, recent files, drag-and-drop, and file size safety checks.
- **`SessionManager`:** Saves and restores open files, active tab, folder path, and window geometry.
- **`BuildManager`:** Integrates with Baa compiler, runs builds in background, streams output to console.
- **`LayoutManager`:** Manages sidebar and panel visibility states.

## Development Workflow

### Adding a new keyword

Keywords are now data-driven from `qalam/resources/baa-language.json`:

1. Add the keyword to the `keywords` array in `baa-language.json`.
2. The IDE will automatically pick it up for syntax highlighting and auto-completion.
3. (Optional) Add a snippet to `TSnippetManager::setupNavigation()` if needed.

### Adding a new source file

1. Create the `.h` and `.cpp` files in the appropriate `source/` subdirectory.
2. Add the files to `source/CMakeLists.txt` target sources.
3. Run `cmake --preset windows-mingw` (or your platform's preset) to reconfigure.

### Adding a setting

1. Add the key constant to `Constants.h` in the appropriate `Settings` namespace.
2. Add UI widget creation in `TSettings::createAppearancePage()` (or a new page method).
3. Wire up the load/save logic in `TSettings::loadSettings()` and `applySettings()`.
4. Connect the change signal in `Qalam::openSettings()` to propagate changes.

## Build System

**CMake is the only supported build system.** The `.pro` file is stale and cannot build the current codebase.

- Root `CMakeLists.txt`: Manages project configuration and subdirectories.
- `source/CMakeLists.txt`: Defines the `qalam_core` static library.
- `qalam/CMakeLists.txt`: Defines the final `Qalam` executable.
- `CMakePresets.json`: Contains build presets. Currently Windows-only; Linux/macOS presets need to be added (see Phase 7 of ROADMAP.md).

### Prerequisites

- Qt 6.10.2+
- MinGW 13.1+ (Windows) or GCC 13+ (Linux) or Clang 16+ (macOS)
- CMake 3.21+

### Build Commands

```sh
# Configure (Windows)
cmake --preset windows-mingw

# Build (Debug)
cmake --build build

# Build (Release)
cmake --preset windows-release
cmake --build build --preset release
```

Output: `build/qalam/Qalam.exe`

**Note:** Reconfigure CMake when adding new `.cpp`/`.h` files to `source/CMakeLists.txt` or editing `.qrc`.

---

*For planned improvements, see [ROADMAP.md](ROADMAP.md)*
