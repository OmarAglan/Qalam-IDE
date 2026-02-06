# Qalam IDE - Improvement Roadmap

**Date:** February 2026  
**Version:** 3.3.0  
**Codebase:** ~9,500 lines C++ across 61 files (Qt 6 + C++23)

---

## Code Quality Assessment

### Strengths

- **Well-factored Manager pattern** -- `FileManager`, `BuildManager`, `SessionManager`, `LayoutManager` cleanly separate concerns
- **Clean Strategy pattern** for autocomplete (`ICompletionStrategy`) and themes (`SyntaxTheme`)
- **State-machine lexer** with solid `LexerState` interface
- **RTL/Arabic-first design** throughout -- UI strings, layout direction, icon positioning
- **Comprehensive `Constants.h`** -- organized namespaces for colors, fonts, layout, timing
- **Centralized theme engine** (`QalamTheme`, 805 lines)
- **Session persistence** -- restores tabs, geometry, folder, preferences

### Critical Bugs (P0)

| # | File | Issue |
|---|------|-------|
| 1 | `TEditor.cpp:161,171` | Toggle comment uses `#` instead of `//` -- inserts preprocessor directives instead of comments |
| 2 | `TEditor.cpp:1054-1056, 1100-1101` | Bracket/quote auto-pair cursor wrong direction -- `Right` instead of `Left` after insert |
| 3 | `TEditor.cpp:1088, 1117` | Bracket skip moves `Left` instead of `Right` -- cursor goes backwards |
| 4 | `TEditor.cpp:912` | Builtin function insert cursor off -- lands past `)` instead of between `()` |
| 5 | `TLexer.cpp:144-146` | Multi-line strings broken -- `StringState::nextState()` always returns `NormalState` |
| 6 | `TSettings.cpp:136-139` | Crash risk -- `font.at(0)` on potentially empty list |
| 7 | `TPanelArea.cpp:208` | `eventFilter` never overridden -- problem item clicks silently ignored |
| 8 | `Qalam.cpp:427` | `exitApp()` returns after save -- never actually exits |

### Functional Bugs (P1)

| # | File | Issue |
|---|------|-------|
| 9 | `TMenu.cpp:3` | `TMenuBar` doesn't pass `parent` to `QMenuBar` base class |
| 10 | `TEditor.cpp:654` | Auto-save reads `this->property("filePath")` but path is in member `filePath` |
| 11 | `TEditor.cpp:59` | Timer hardcoded to 60000ms, ignoring `Constants::Timing::AutoSaveInterval` (30000ms) |
| 12 | `TEditor.cpp:950-991` | Snippet navigation targets don't match actual snippet content |
| 13 | `Qalam.cpp:273` | `eventFilter` calls `QMainWindow::eventFilter` instead of `QalamWindow::eventFilter` |
| 14 | `TLexer.cpp:134` | Escape at end of string can overshoot `pos` by 1 |

### Architecture & Performance Issues (P2)

| # | Issue | Impact |
|---|-------|--------|
| 15 | `TEditor.cpp:832` -- `toPlainText()` on every keystroke | O(n) document copy per keypress |
| 16 | Keyword lists duplicated in `AutoComplete.cpp` and `TSyntaxDefinition.cpp` | Maintenance risk |
| 17 | `QalamTheme` exists but 5+ classes use hardcoded inline CSS | Theme inconsistency |
| 18 | `TConsole.cpp:213` -- `QStringList::pop_front()` in loop | O(n^2) performance |
| 19 | Dynamic word index grows forever, never shrinks | Memory leak over time |
| 20 | `ThemeManager::getAvailableThemes()` allocates 4 themes per call | Wasteful |
| 21 | `TEditor.cpp` is 1,126 lines with 8+ responsibilities | God-class risk |
| 22 | `TSettings` created without parent -- memory leak (`Qalam.cpp:68`) | Resource leak |
| 23 | `WelcomeWindow` in `exitApp()` created without parent (`Qalam.cpp:429`) | Resource leak |

### Code Hygiene Issues (P3)

- Inconsistent `m_` prefix (some classes use it, others don't)
- Missing `explicit` on single-argument constructors
- Pass-by-value where `const&` is appropriate
- Dead code: `TConsole::appendOutput()`, `StateType` enum
- Magic numbers in geometry calculations
- `Constants.h` uses `const QString` instead of `inline const` or `constexpr`
- Redundant Arabic digit checks (`QChar::isDigit()` already covers them)
- `.pro` file is stale and can't build the current codebase
- No `.clang-format`, `.editorconfig`, or `.clang-tidy`

### Infrastructure Gaps

- **Zero tests** -- no test framework, files, or directories
- **Zero CI/CD** -- no GitHub Actions or automated builds
- **No code formatting config** despite detailed style rules in AGENTS.md
- **No linting/static analysis** configuration
- **CMakePresets.json** only defines Windows presets

---

## Phase 1: Stability (Fix Critical Bugs)

**Goal:** Make existing features actually work correctly  
**Effort:** 1-2 days | **Impact:** Critical

- [x] 1.1 Fix toggle comment: `#` -> `//` (`TEditor.cpp`)
- [x] 1.2 Fix bracket/quote auto-pair cursor direction Right->Left (`TEditor.cpp`)
- [x] 1.3 Fix bracket skip direction Left->Right (`TEditor.cpp`)
- [x] 1.4 Fix builtin function insert cursor Right->Left (`TEditor.cpp`)
- [x] 1.5 Fix multi-line string lexing -- `StringState::nextState()` persistence (`TLexer.cpp`)
- [x] 1.6 Fix `TSettings` font crash -- guard `.at(0)` on empty list (`TSettings.cpp`)
- [x] 1.7 Fix `TPanelArea` -- implement `eventFilter()` override (`TPanelArea.cpp`)
- [x] 1.8 Fix `exitApp()` -- remove early `return` after save (`Qalam.cpp`)
- [x] 1.9 Fix `TMenuBar` parent passing (`TMenu.cpp`)
- [x] 1.10 Fix auto-save: use member `filePath` instead of `property()` (`TEditor.cpp`)
- [x] 1.11 Fix auto-save timer to use `Constants::Timing::AutoSaveInterval` (`TEditor.cpp`)
- [x] 1.12 Fix snippet navigation targets to match actual snippet content (`TEditor.cpp`)
- [x] 1.13 Fix `eventFilter` base class call: `QalamWindow` not `QMainWindow` (`Qalam.cpp`)
- [x] 1.14 Fix escape at end of string bounds issue (`TLexer.cpp`)

## Phase 2: Memory & Performance

**Goal:** Eliminate leaks and fix performance bottlenecks  
**Effort:** 2-3 days | **Impact:** High

- [ ] 2.1 Remove `toPlainText()` from `performCompletion()` hot path (`TEditor.cpp`)
- [ ] 2.2 Fix `TSettings` memory leak -- pass `this` as parent or use `unique_ptr` (`Qalam.cpp`)
- [ ] 2.3 Fix `WelcomeWindow` leak -- set `WA_DeleteOnClose` or parent (`Qalam.cpp`, `main.cpp`)
- [ ] 2.4 Replace `QStringList::pop_front()` with `QQueue`/`std::deque` in `TConsole`
- [ ] 2.5 Cache themes in `ThemeManager` (static local)
- [ ] 2.6 Implement proper shrink logic in `DynamicWordStrategy`
- [ ] 2.7 Remove redundant `QString value` from `TToken`
- [ ] 2.8 Fix double hash lookup in `highlightBlock` (use `find()`)

## Phase 3: Architecture Refactoring

**Goal:** Improve maintainability and reduce coupling  
**Effort:** 1-2 weeks | **Impact:** High

- [ ] 3.1 Single source of truth for keywords -- `AutoComplete` strategies read from `LanguageDefinition`
- [ ] 3.2 Decompose `TEditor` into: `BracketPairing`, `CodeFolding`, `AutoSaveManager`, `SnippetManager`, `EditorAutoComplete`
- [ ] 3.3 Complete `QalamTheme` adoption -- migrate all hardcoded CSS to centralized theme
- [ ] 3.4 Move find/replace logic out of `Qalam` into `SearchPanel` or `SearchController`
- [ ] 3.5 Make `LanguageDefinition` data-driven (load from config files)
- [ ] 3.6 Fix snippet matching -- use IDs/enum instead of `startsWith()` on content
- [ ] 3.7 Improve lexer multi-char operators -- lex `==`, `!=`, `<=`, `>=` as single tokens

## Phase 4: Code Quality & Tooling

**Goal:** Enforce consistency and catch issues early  
**Effort:** 1-2 days | **Impact:** Medium

- [ ] 4.1 Add `.clang-format` matching AGENTS.md style rules
- [ ] 4.2 Add `.clang-tidy` with modernize checks
- [ ] 4.3 Add `.editorconfig`
- [ ] 4.4 Apply `m_` prefix consistently to all private members
- [ ] 4.5 Add `explicit` to all single-argument constructors
- [ ] 4.6 Fix `Constants.h` to use `inline const` or `constexpr`
- [ ] 4.7 Remove dead code (`TConsole::appendOutput`, `StateType` enum)
- [ ] 4.8 Delete or update stale `.pro` file
- [ ] 4.9 Remove redundant Arabic digit checks in `TLexer`
- [ ] 4.10 Fix `SnippetStrategy` `.contains()` -> `.startsWith()` for consistent matching

## Phase 5: Testing Infrastructure

**Goal:** Build confidence for future changes  
**Effort:** 1-2 weeks | **Impact:** High

- [ ] 5.1 Add Qt Test or Catch2 framework to CMake
- [ ] 5.2 Write lexer unit tests -- normal tokens, multi-line strings, edge cases
- [ ] 5.3 Write autocomplete strategy tests -- each strategy, prefix matching
- [ ] 5.4 Write `FileManager` tests -- new/open/save/close workflows
- [ ] 5.5 Write `SessionManager` tests -- save/restore round-trip
- [ ] 5.6 Write `TEditor` integration tests -- bracket pairing, commenting, snippets

## Phase 6: CI/CD Pipeline

**Goal:** Automate quality gates  
**Effort:** 2-3 days | **Impact:** High

- [ ] 6.1 Add GitHub Actions workflow for Windows build (MinGW + Qt 6)
- [ ] 6.2 Add test execution step
- [ ] 6.3 Add `clang-format` check step
- [ ] 6.4 Add `clang-tidy` static analysis step
- [ ] 6.5 Add CMakePresets for Linux/macOS
- [ ] 6.6 Add release workflow (build + package + GitHub Release)

## Phase 7: Feature Enhancements

**Goal:** Reach feature parity with modern editors  
**Effort:** Months | **Impact:** High

- [ ] 7.1 Add `Ctrl+N` (new file) and `Ctrl+O` (open file) keyboard shortcuts
- [ ] 7.2 Add undo/redo buttons in toolbar
- [ ] 7.3 Add multi-cursor editing
- [ ] 7.4 Add minimap
- [ ] 7.5 Implement LSP client (the `LSP-Core` branch)
- [ ] 7.6 Add Git integration panel (diff view, blame, stage/commit)
- [ ] 7.7 Add plugin/extension system
- [ ] 7.8 Add integrated debugger support
- [ ] 7.9 Add split editor (horizontal/vertical)
- [ ] 7.10 Add Baa language server

---

## Priority Summary

| Phase | Effort | Impact | Status |
|-------|--------|--------|--------|
| Phase 1: Stability | Low (1-2 days) | Critical | **Complete** |
| Phase 2: Memory/Perf | Low-Medium (2-3 days) | High | Pending |
| Phase 3: Architecture | Medium (1-2 weeks) | High | Pending |
| Phase 4: Code Quality | Low (1-2 days) | Medium | Pending |
| Phase 5: Testing | Medium (1-2 weeks) | High | Pending |
| Phase 6: CI/CD | Low-Medium (2-3 days) | High | Pending |
| Phase 7: Features | High (months) | High | Pending |
