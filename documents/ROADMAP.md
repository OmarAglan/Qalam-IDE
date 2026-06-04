# Qalam IDE - Improvement Roadmap

**Date:** February 2026
**Version:** 3.3.0
**Codebase:** ~9,750 lines C++ across 67 files (Qt 6 + C++23)

---

## Completed Work (Archive)

Phases 1-3 have been completed and are archived here for reference.

| Phase | Description | Status |
|-------|-------------|--------|
| Phase 1: Stability | Fixed 14 critical/functional bugs (comment toggle, cursor direction, multi-line strings, crash guards, exit flow) | **Complete** |
| Phase 2: Memory & Performance | Fixed leaks, hot-path copies, O(n^2) trimming, cached themes, dynamic word shrinking | **Complete** |
| Phase 3: Architecture Refactoring | Single keyword source, TEditor decomposition, QalamTheme adoption, data-driven language definitions | **Complete** |

One deferred item from Phase 2:
- 2.7 Remove redundant `QString value` from `TToken` *(low risk/reward -- carry forward to Phase 4)*

---

## Code Quality Assessment

### Current Strengths

- **Well-factored Manager pattern** -- `FileManager`, `BuildManager`, `SessionManager`, `LayoutManager` cleanly separate concerns
- **Clean Strategy pattern** for autocomplete (`ICompletionStrategy`) and themes (`SyntaxTheme`)
- **State-machine lexer** with solid `LexerState` interface and `QStringView` zero-copy tokenization
- **RTL/Arabic-first design** throughout -- UI strings, layout direction, icon positioning
- **Comprehensive `Constants.h`** -- organized namespaces for colors, fonts, layout, timing
- **Centralized theme engine** (`QalamTheme`, 805 lines) providing consistent styling
- **Session persistence** -- restores tabs, geometry, folder, preferences
- **Modern signal/slot connections** -- 100% pointer-to-member syntax, zero string-based connections
- **Smart pointer usage** -- `std::unique_ptr` for strategies/states, `std::shared_ptr` for themes, `QPointer` for cross-thread refs
- **Thread safety** -- `QMutex` in `ProcessWorker` and `TConsole`, proper thread lifecycle in `BuildManager`

### Remaining Issues by Severity

#### Hardcoded Values & Theme Leaks (P2)

| File | Line(s) | Issue |
|------|---------|-------|
| `AutoCompleteUI.cpp` | 49-57, 62-71, 130 | Hardcoded colors (`#1e202e`, `#4b5263`, `#abb2bf`, `#3e4451`) instead of `QalamTheme`/`Constants` |
| `TMenu.cpp` | (inline CSS) | Menu bar/dropdown CSS hardcoded, not using `QalamTheme` |
| `TSettings.cpp` | 8 | Hardcoded `#1e202e` |
| `TWelcomeWindow.cpp` | 247 | Hardcoded `#5cb8ff` |
| `LayoutManager.cpp` | 61 | Hardcoded `#007acc` splitter color |
| `LayoutManager.cpp` | 79 | Hardcoded splitter sizes `{700, 200}` |
| `TEditor.cpp` | 113-114 | Zoom limits `5.0`/`50` not in Constants |
| `TEditor.cpp` | 133-134, 322, 372 | Font fallback `10`/`18`, line number width `30`, paint offset `12` |
| `TConsole.h` | 56 | `m_maxLines = 2000` vs `Constants::Console::MaxBufferLines = 10000` |
| `ProcessWorker.cpp` | 16 | Flush timer `20`ms vs `Constants::Timing::FlushInterval = 25`ms |
| `Qalam.cpp` | 56-61 | Window geometry magic numbers `100`, `6`, `30` |

#### Code Hygiene (P3)

- Inconsistent `m_` prefix on private members across classes
- Missing `explicit` on some single-argument constructors
- Pass-by-value where `const&` is appropriate (`TEditor::updateFontType(QString)`, `FileManager::openFile(QString)`)
- Dead code: `TConsole::appendOutput()`, `StateType` enum
- `Constants.h` lines 5-6 use `const QString` at namespace scope (per-TU copies) -- should be `inline const`
- `QalamWindow.h:23` declares `void initFrameless()` but it's never defined or called
- `TEditor.h:88` -- QCompleter member named `c` -- poor naming
- `TWelcomeWindow.cpp` includes `<QtWidgets>` (kitchen-sink include)
- Mixed logical operators: some files use `and`/`or`/`not`, others use `&&`/`||`/`!`
- `.pro` file is stale and can't build the current codebase

#### Functional Gaps Discovered

| Area | Issue |
|------|-------|
| Editor Search | `TSearchPanel` has find but **no replace** functionality; `isWholeWord()` hardcoded to `false` |
| Sidebar Search | `TSearchView` UI exists but search is **not wired** -- `searchRequested` signal emitted but nothing connects to scan files; `addResult()` never called; replace buttons nonfunctional |
| Build Error Parsing | Output goes to console raw; `TPanelArea::addProblem()` exists but `BuildManager` never parses compiler errors into it -- Problems panel is empty |
| ANSI Colors | `TConsole::appendOutput()` has ANSI parsing but is dead code; active path `flushPending()` does NOT render colors |
| Auto-save Error | `TAutoSave.cpp:35` -- `file.open()` failure is silently ignored |
| Settings | Only editor appearance (font size/family/theme); no compiler path UI, no keybinding config, no auto-save interval config; "Advanced" category commented out |
| File Explorer | No rename/delete files, no new file/folder from context menu, no close-all tabs |
| Keyboard Shortcuts | `Ctrl+N` (new file) and `Ctrl+O` (open file) mentioned in USER_GUIDE.md but not wired as shortcuts |
| Welcome Page | "Clone repo" button is a stub (shows "under development" message) |
| Bracket Highlighting | Auto-pairing exists but no visual highlight of matching bracket at cursor position |

#### Architecture Concerns

- `TEditor` still 927 lines with broad responsibilities despite decomposition
- `QalamTheme` at 805 lines -- growing, may need sub-theme modules
- `TWelcomeWindow::applyStyles()` is 144 lines of inline CSS -- should use `QalamTheme`
- `QSettings` construction repeated ~15 times instead of a helper
- Tab iteration pattern (`for i in tabWidget.count, qobject_cast<TEditor*>`) repeated ~5 times across `Qalam.cpp` and `SessionManager.cpp`
- ANSI color switch blocks duplicated in `TConsole.cpp` (lines 264-277 and 278-292)
- No `[[nodiscard]]`, `std::optional`, structured bindings, or `consteval` usage

---

## Phase 4: Code Quality & Tooling

**Goal:** Enforce consistency, remove dead code, and catch issues early
**Effort:** 2-3 days | **Impact:** Medium

### 4.1 Formatting & Linting Setup

- [ ] 4.1.1 Add `.clang-format` matching AGENTS.md style rules (K&R braces, 4-space indent, pointer left-aligned)
- [ ] 4.1.2 Add `.clang-tidy` with `modernize-*`, `readability-*`, `bugprone-*` checks
- [ ] 4.1.3 Add `.editorconfig` (4 spaces, UTF-8, LF line endings, trim trailing whitespace)
- [ ] 4.1.4 Run clang-format on entire codebase and commit as a single formatting pass

### 4.2 Naming & Consistency

- [ ] 4.2.1 Apply `m_` prefix consistently to all private members across all classes
- [ ] 4.2.2 Rename `TEditor.h:88` member `c` to `m_completer`
- [ ] 4.2.3 Add `explicit` to all single-argument constructors missing it
- [ ] 4.2.4 Standardize logical operators: use `and`/`or`/`not` consistently per AGENTS.md
- [ ] 4.2.5 Fix pass-by-value to `const&` where appropriate (`updateFontType`, `openFile`, etc.)

### 4.3 Constants & Dead Code

- [ ] 4.3.1 Fix `Constants.h` to use `inline const` or `constexpr` instead of `const QString`
- [ ] 4.3.2 Extract remaining magic numbers to `Constants.h` (zoom limits, font fallbacks, geometry offsets, splitter sizes)
- [ ] 4.3.3 Align `TConsole::m_maxLines` with `Constants::Console::MaxBufferLines`
- [ ] 4.3.4 Align `ProcessWorker` flush timer with `Constants::Timing::FlushInterval`
- [ ] 4.3.5 Remove dead code: `TConsole::appendOutput()`, `StateType` enum
- [ ] 4.3.6 Remove undeclared `QalamWindow::initFrameless()` declaration
- [ ] 4.3.7 Remove redundant `QString value` from `TToken` (carried from Phase 2.7)
- [ ] 4.3.8 Delete or update stale `.pro` file
- [ ] 4.3.9 Remove redundant Arabic digit checks in `TLexer` (`QChar::isDigit()` already covers them)
- [ ] 4.3.10 Replace `<QtWidgets>` include in `TWelcomeWindow.cpp` with specific headers

### 4.4 Theme Consistency

- [ ] 4.4.1 Migrate `AutoCompleteUI.cpp` hardcoded colors to `QalamTheme`/`Constants`
- [ ] 4.4.2 Migrate `TMenu.cpp` inline CSS to `QalamTheme::menuBarStyleSheet()`
- [ ] 4.4.3 Migrate `TSettings.cpp` hardcoded color to `Constants`
- [ ] 4.4.4 Migrate `TWelcomeWindow.cpp` hardcoded color to `Constants`
- [ ] 4.4.5 Migrate `LayoutManager.cpp` hardcoded splitter color to `Constants`
- [ ] 4.4.6 Move `TWelcomeWindow::applyStyles()` (144 lines) to use `QalamTheme`

### 4.5 Code Deduplication

- [ ] 4.5.1 Create `QSettings` helper (e.g. `AppSettings::instance()` or free function) to eliminate ~15 repeated constructions
- [ ] 4.5.2 Extract tab iteration helper (`forEachEditor(callback)`) to reduce 5+ duplicated loops
- [ ] 4.5.3 Deduplicate ANSI color switch blocks in `TConsole.cpp` into a shared color map

### 4.6 Modern C++ Improvements

- [ ] 4.6.1 Add `[[nodiscard]]` to functions with important return values
- [ ] 4.6.2 Use `std::optional` where nullable return values are appropriate
- [ ] 4.6.3 Apply structured bindings where map/pair iteration occurs
- [ ] 4.6.4 Add forward declarations where full includes are unnecessary

---

## Phase 5: Functional Completeness

**Goal:** Wire up existing UI that doesn't work yet and fill critical feature gaps
**Effort:** 1-2 weeks | **Impact:** High

### 5.1 Editor Search & Replace

- [ ] 5.1.1 Add replace input field and replace/replace-all buttons to `TSearchPanel`
- [ ] 5.1.2 Implement `isWholeWord()` properly (currently hardcoded `false`)
- [ ] 5.1.3 Add regex search toggle to `TSearchPanel`
- [ ] 5.1.4 Add match count display (e.g. "3 of 12")
- [ ] 5.1.5 Add highlight-all-matches visual indicator

### 5.2 Project-Wide Search

- [ ] 5.2.1 Implement file scanner that connects to `TSearchView::searchRequested` signal
- [ ] 5.2.2 Scan all files in open folder recursively, respecting `.gitignore` patterns
- [ ] 5.2.3 Populate `TSearchView` results tree via `addResult()`
- [ ] 5.2.4 Wire up project-wide replace functionality
- [ ] 5.2.5 Add search progress indicator
- [ ] 5.2.6 Implement result caching and incremental update

### 5.3 Build Error Integration

- [ ] 5.3.1 Parse Baa compiler error output (line number, column, message, severity)
- [ ] 5.3.2 Feed parsed errors to `TPanelArea::addProblem()`
- [ ] 5.3.3 Add inline error indicators in editor (squiggly underlines or margin marks)
- [ ] 5.3.4 Click-to-navigate from Problems panel to error location in editor
- [ ] 5.3.5 Clear problems on successful build

### 5.4 Console ANSI Colors

- [ ] 5.4.1 Integrate ANSI color parsing into the active `flushPending()` code path
- [ ] 5.4.2 Support 256-color and true-color ANSI codes (extended SGR)
- [ ] 5.4.3 Remove the dead `appendOutput()` method after migration

### 5.5 File Explorer Enhancements

- [ ] 5.5.1 Add context menu to explorer: New File, New Folder, Rename, Delete
- [ ] 5.5.2 Implement file/folder rename with validation
- [ ] 5.5.3 Implement file/folder delete with confirmation dialog
- [ ] 5.5.4 Add "Close All Tabs" action
- [ ] 5.5.5 Add "Close Other Tabs" action
- [ ] 5.5.6 Add file icons by extension in explorer tree

### 5.6 Keyboard Shortcuts

- [ ] 5.6.1 Wire `Ctrl+N` to new file action
- [ ] 5.6.2 Wire `Ctrl+O` to open file action
- [ ] 5.6.3 Wire `Ctrl+Shift+S` to save-as action
- [ ] 5.6.4 Wire `Ctrl+W` to close current tab
- [ ] 5.6.5 Wire `Ctrl+Tab` / `Ctrl+Shift+Tab` for tab switching
- [ ] 5.6.6 Wire `Ctrl+Shift+F` for project-wide search (focus sidebar search)
- [ ] 5.6.7 Add keyboard shortcut reference panel / cheat sheet

### 5.7 Settings Expansion

- [ ] 5.7.1 Add compiler path configuration UI in settings
- [ ] 5.7.2 Add auto-save interval configuration
- [ ] 5.7.3 Add keybinding customization panel
- [ ] 5.7.4 Show system fonts alongside bundled fonts
- [ ] 5.7.5 Add tab size / indentation configuration
- [ ] 5.7.6 Add "Reset to Defaults" button

### 5.8 Missing Editor Features

- [ ] 5.8.1 Add matching bracket highlight at cursor position (colored box/underline)
- [ ] 5.8.2 Fix auto-save silent failure -- show warning on `file.open()` failure (`TAutoSave.cpp:35`)
- [ ] 5.8.3 Add Go-to-Definition for Baa symbols (local scope, same file)
- [ ] 5.8.4 Add matching tag/bracket highlight on hover

---

## Phase 6: Testing Infrastructure

**Goal:** Build confidence for future changes with automated tests
**Effort:** 1-2 weeks | **Impact:** High

### 6.1 Framework Setup

- [ ] 6.1.1 Add Qt Test framework to CMake (or Catch2 if preferred for non-Qt classes)
- [ ] 6.1.2 Create `tests/` directory structure mirroring `source/`
- [ ] 6.1.3 Add test CMake target (`ctest` integration)
- [ ] 6.1.4 Create test utilities/helpers for common patterns

### 6.2 Core Unit Tests

- [ ] 6.2.1 `TLexer` tests: single-line tokens, multi-line strings, edge cases, escape sequences, Arabic digits, operators
- [ ] 6.2.2 `TSyntaxDefinition` tests: keyword loading from JSON, fallback defaults
- [ ] 6.2.3 `LanguageDefinition` tests: singleton access, keyword/builtin lists
- [ ] 6.2.4 `AutoComplete` strategy tests: each strategy returns correct completions, prefix matching, deduplication
- [ ] 6.2.5 `TBracketHandler` tests: pairing, skip-over, wrapping, edge cases
- [ ] 6.2.6 `TSnippetManager` tests: insertion, placeholder navigation, indentation

### 6.3 Manager Unit Tests

- [ ] 6.3.1 `FileManager` tests: new/open/save/save-as workflows, size limits, duplicate detection
- [ ] 6.3.2 `SessionManager` tests: save/restore round-trip, missing files, empty session
- [ ] 6.3.3 `BuildManager` tests: compiler resolution, build lifecycle
- [ ] 6.3.4 `LayoutManager` tests: sidebar toggle, panel toggle, state persistence

### 6.4 Integration Tests

- [ ] 6.4.1 Editor integration: type text -> verify highlighting -> verify autocomplete suggestions
- [ ] 6.4.2 Bracket pairing integration: type opening bracket -> verify closing inserted -> verify cursor position
- [ ] 6.4.3 Snippet integration: trigger snippet -> verify expansion -> Tab through placeholders
- [ ] 6.4.4 Build integration: trigger build -> verify console output -> verify exit code display

---

## Phase 7: CI/CD Pipeline

**Goal:** Automate quality gates and releases
**Effort:** 2-3 days | **Impact:** High

### 7.1 Build Automation

- [ ] 7.1.1 Add GitHub Actions workflow for Windows build (MinGW 13.1+ + Qt 6.10.2)
- [ ] 7.1.2 Add CMakePresets for Linux (GCC 13+)
- [ ] 7.1.3 Add CMakePresets for macOS (Clang 16+)
- [ ] 7.1.4 Cache Qt installation in CI for faster builds

### 7.2 Quality Gates

- [ ] 7.2.1 Add test execution step (`ctest --output-on-failure`)
- [ ] 7.2.2 Add `clang-format --dry-run --Werror` check step
- [ ] 7.2.3 Add `clang-tidy` static analysis step
- [ ] 7.2.4 Add build warnings as errors (`-Werror`) for CI builds

### 7.3 Release Automation

- [ ] 7.3.1 Add release workflow: build Release preset -> package with `windeployqt`
- [ ] 7.3.2 Create installer (NSIS or WiX)
- [ ] 7.3.3 Auto-generate changelog from git commits
- [ ] 7.3.4 Publish to GitHub Releases with artifacts

---

## Phase 8: Core Feature Enhancements

**Goal:** Bring Qalam closer to modern IDE capabilities
**Effort:** 2-4 weeks | **Impact:** High

### 8.1 Multi-Cursor Editing

- [ ] 8.1.1 Implement `Ctrl+D` to select next occurrence of current selection
- [ ] 8.1.2 Implement `Ctrl+Shift+L` to select all occurrences
- [ ] 8.1.3 Implement `Alt+Click` to add cursor at click position
- [ ] 8.1.4 Support typing, deleting, and pasting at all cursors simultaneously
- [ ] 8.1.5 Visual indicators for each cursor position

### 8.2 Split Editor

- [ ] 8.2.1 Implement horizontal split (`Ctrl+\`)
- [ ] 8.2.2 Implement vertical split
- [ ] 8.2.3 Synchronized scrolling option for same-file splits
- [ ] 8.2.4 Independent scrolling for different-file splits
- [ ] 8.2.5 Drag tabs between split groups

### 8.3 Minimap

- [ ] 8.3.1 Render miniature document overview on the left margin (RTL)
- [ ] 8.3.2 Show viewport indicator (current visible region)
- [ ] 8.3.3 Click-to-scroll on minimap
- [ ] 8.3.4 Highlight search results on minimap
- [ ] 8.3.5 Show git diff indicators on minimap

### 8.4 Multiple UI Themes

- [ ] 8.4.1 Refactor `QalamTheme` to support loadable theme definitions (JSON/TOML)
- [ ] 8.4.2 Create "Qalam Light" theme
- [ ] 8.4.3 Create "Qalam Solarized" theme
- [ ] 8.4.4 Add theme selector in Settings for overall UI theme (not just syntax)
- [ ] 8.4.5 Live theme preview in settings

### 8.5 Undo/Redo & Toolbar

- [ ] 8.5.1 Add undo/redo buttons in toolbar
- [ ] 8.5.2 Add undo history dropdown (list of recent changes)
- [ ] 8.5.3 Add customizable toolbar with common actions

---

## Phase 9: Advanced Features

**Goal:** Professional-grade IDE capabilities
**Effort:** Months | **Impact:** Very High

### 9.1 LSP Client Integration

- [ ] 9.1.1 Implement LSP client protocol handler (JSON-RPC over stdio)
- [ ] 9.1.2 Support `textDocument/completion` for intelligent autocomplete
- [ ] 9.1.3 Support `textDocument/hover` for documentation tooltips
- [ ] 9.1.4 Support `textDocument/definition` for go-to-definition
- [ ] 9.1.5 Support `textDocument/references` for find-all-references
- [ ] 9.1.6 Support `textDocument/diagnostic` for real-time error reporting
- [ ] 9.1.7 Support `textDocument/formatting` for auto-format
- [ ] 9.1.8 Support `textDocument/rename` for symbol rename

### 9.2 Baa Language Server

- [ ] 9.2.1 Design language server architecture for Baa
- [ ] 9.2.2 Implement Baa parser for semantic analysis
- [ ] 9.2.3 Implement symbol table and scope resolution
- [ ] 9.2.4 Implement completion provider with context awareness
- [ ] 9.2.5 Implement diagnostic provider (type errors, undefined variables)
- [ ] 9.2.6 Package as standalone LSP binary

### 9.3 Git Integration

- [ ] 9.3.1 Add Git status indicators in file explorer (modified, added, untracked)
- [ ] 9.3.2 Add inline diff gutter marks in editor (green/red bars for additions/deletions)
- [ ] 9.3.3 Add diff view panel (side-by-side or inline)
- [ ] 9.3.4 Add blame annotations (inline or sidebar)
- [ ] 9.3.5 Add stage/unstage/commit UI in sidebar
- [ ] 9.3.6 Add branch switching UI
- [ ] 9.3.7 Wire up Welcome page "Clone repo" button to actual `git clone`

### 9.4 Integrated Debugger

- [ ] 9.4.1 Design debugger interface for Baa runtime
- [ ] 9.4.2 Implement breakpoint UI (click gutter to toggle)
- [ ] 9.4.3 Implement step-over, step-into, step-out, continue controls
- [ ] 9.4.4 Add variable inspector panel
- [ ] 9.4.5 Add call stack panel
- [ ] 9.4.6 Add watch expressions

### 9.5 Plugin/Extension System

- [ ] 9.5.1 Design plugin API (C++ shared library or scripted)
- [ ] 9.5.2 Implement plugin loader and lifecycle management
- [ ] 9.5.3 Expose editor, UI, and build APIs to plugins
- [ ] 9.5.4 Create sample plugin (e.g. word count, Markdown preview)
- [ ] 9.5.5 Add plugin manager UI in settings

---

## Priority Summary

| Phase | Effort | Impact | Status | Dependencies |
|-------|--------|--------|--------|--------------|
| Phase 4: Code Quality & Tooling | Low (2-3 days) | Medium | **Pending** | None |
| Phase 5: Functional Completeness | Medium (1-2 weeks) | High | **Pending** | None |
| Phase 6: Testing Infrastructure | Medium (1-2 weeks) | High | **Pending** | None |
| Phase 7: CI/CD Pipeline | Low-Medium (2-3 days) | High | **Pending** | Phase 6 |
| Phase 8: Core Feature Enhancements | Medium-High (2-4 weeks) | High | **Pending** | Phase 5 |
| Phase 9: Advanced Features | Very High (months) | Very High | **Pending** | Phases 6, 8 |

### Recommended Execution Order

**Parallel Track A (Foundation):** Phase 4 -> Phase 6 -> Phase 7
**Parallel Track B (Features):** Phase 5 -> Phase 8 -> Phase 9

Phases 4 and 5 can start simultaneously. Phase 6 (testing) benefits from Phase 4's cleanup. Phase 7 (CI/CD) requires Phase 6's test framework. Phase 8 builds on Phase 5's foundation. Phase 9 requires stable architecture from all prior phases.
