# AGENTS.md - Qalam IDE

Guidelines for AI agents working on the Qalam IDE codebase.

## Project Overview

Qalam IDE is a Qt 6 C++23 IDE for Arabic-syntax programming languages (Baa). Features RTL-first design, syntax highlighting, auto-completion, and embedded console.

## Build Commands

### Prerequisites
- Qt 6.x (Widgets, GUI, Core), C++23 compiler (MinGW 13+), CMake 3.21+

### Build with CMake (Recommended)
```powershell
cmake --preset windows-mingw    # Configure
cmake --build build             # Build Debug
```

### Build with qmake (Legacy)
```powershell
cd qalam && qmake Qalam.pro && mingw32-make -j
```

## Project Structure

```
├── qalam/              # Main app entry (main.cpp, Qalam.cpp, Constants.h)
├── source/             # Core library (qalam_core)
│   ├── texteditor/     # TEditor, highlighter/, autocomplete/
│   ├── console/        # TConsole, ProcessWorker
│   ├── components/     # TFlatButton, TSearchPanel
│   ├── menubar/        # TMenuBar
│   ├── settings/       # TSettings
│   └── ui/             # QalamWindow, QalamTheme, TTitleBar
└── documents/          # Language spec, user guide
```

## Code Style Guidelines

### Headers & Includes
- Use `#pragma once` for include guards
- Order: Local headers, Qt headers, STL headers
```cpp
#include "TEditor.h"
#include <QPlainTextEdit>
#include <memory>
```

### Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Classes | PascalCase with T/Q prefix | `TEditor`, `QalamWindow` |
| Members | camelCase with `{}` init | `QTabWidget *tabWidget{};` |
| Private members | `m_` prefix | `m_glowIntensity` |
| Signals | Past tense/action | `fontSizeChanged` |
| Slots | `on` prefix or verb | `onInputReturn`, `toggleConsole` |
| Constants | Namespace::PascalCase | `Constants::AppName` |

### Class Declaration Order
```cpp
class TExample : public QWidget {
    Q_OBJECT
public:
    explicit TExample(QWidget *parent = nullptr);
    void publicMethod();
public slots:
    void slotMethod();
protected:
    void paintEvent(QPaintEvent *event) override;
private slots:
    void privateSlot();
signals:
    void signalName();
private:
    QWidget *m_widget{};
};
```

### Memory Management
- Raw pointers for Qt parent-child ownership
- `std::unique_ptr` for non-Qt owned objects
- `std::shared_ptr` for shared config objects
- `QPointer` for weak QObject references

### Qt Patterns
```cpp
// Connect with pointer-to-member
connect(button, &QPushButton::clicked, this, &Qalam::handleClick);

// Lambda for inline handlers
connect(shortcut, &QShortcut::activated, this, [this](){
    if (TEditor* e = currentEditor()) e->toggleComment();
});
```

### Formatting
- 4 spaces indentation
- Same-line opening braces
- Use `and`, `or`, `not` operators (codebase convention)
- ~100 char line length

### Error Handling
```cpp
if (!file.open(QIODevice::ReadOnly)) {
    QMessageBox::warning(this, "خطأ", "لا يمكن فتح الملف");
    return;
}
```

### RTL/Arabic Support
- UI strings in Arabic for user-facing text
- Set RTL on documents: `option.setTextDirection(Qt::RightToLeft);`
- App layout: `app.setLayoutDirection(Qt::RightToLeft);`

## Architecture Patterns

### Lexer (State Machine)
`TLexer` uses states: `NormalState`, `StringState`. Each state parses tokens and transitions.

### AutoComplete (Strategy Pattern)
`ICompletionStrategy` implementations:
- `KeywordStrategy` - language keywords
- `BuiltinStrategy` - built-in functions  
- `SnippetStrategy` - code snippets
- `DynamicWordStrategy` - document words

### Signals & Slots
`TMenuBar` emits signals (`newRequested`, `saveRequested`), `Qalam` connects in constructor.

## Common Tasks

### Adding a Keyword
Update `source/texteditor/highlighter/TSyntaxDefinition.cpp` in `LanguageDefinition` constructor.

### Adding a Snippet
1. Update `SnippetStrategy::getSuggestions()` in `AutoComplete.cpp`
2. Handle navigation in `TEditor::insertSnippet()`

### Adding a Setting
1. Add key to `qalam/Constants.h`
2. Add UI in `TSettings::createAppearancePage()`
3. Connect signal in `Qalam::openSettings()`

## File Extensions
- `.baa` - Baa source files
- `.baahd` - Baa header files
- `.~` - Auto-save backups (temporary)

## Dependencies
- Qt6::Widgets, Qt6::Gui, Qt6::Core
- dwmapi, user32 (Windows frameless window)
