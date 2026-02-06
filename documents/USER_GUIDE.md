# Qalam IDE User Guide

Welcome to **Qalam IDE**, the professional environment for Arabic-syntax programming.

## Getting Started

### Creating your first Baa file
1. Click **ملف جديد** (New File) on the Welcome Screen or in the menu.
2. Write your code using Baa syntax (see [Language Spec](LANGUAGE.md)).
3. Save the file with a `.baa` extension using `Ctrl+S`.

### Running your code
1. Open your `.baa` file.
2. Ensure you have the Baa compiler installed (default path: `baa/baa.exe` relative to the IDE).
3. Use **تشغيل** (Run) from the build menu to compile and execute your program.

## Interface Overview

### The Editor
The editor supports:
- **Syntax Highlighting:** Automatic coloring for Baa keywords, types, and directives.
- **RTL Support:** Full right-to-left support for both the interface and the code.
- **Auto-completion:** Suggested keywords and snippets appear as you type.
- **Bracket Matching:** Automatic pairing of `()`, `{}`, `[]`, `''`, `""`, and ` `` `.
- **Code Folding:** Collapse/expand function and class blocks.
- **Snippets:** Type triggers like `if` or `main` then press Tab to expand.
- **Auto-save:** Automatic backup every 30 seconds to `.~` files.
- **Zoom:** Use `Ctrl+Scroll` to change font size.

### The Console (`F6`)
The embedded console allows you to interact with the system and view compiler output.
- Use `Ctrl+L` to clear the console.
- Command history is available using the Up/Down keys.
- The console runs your default shell (cmd.exe on Windows, bash on Linux).

### File Explorer
The right sidebar shows your project files. You can double-click a file to open it or drag and drop files from your system.

### Sidebar Search (`Ctrl+Shift+F`)
Search across all files in the opened folder (project-wide search).

## Customization

### Themes
You can change the editor's **syntax highlighting theme** in Settings:
- **GitHub Dark** (default)
- **GitHub Light**
- **Monokai**
- **Solarized**

Access via: **File** → **Settings** → **Editor** → **Theme**.

**Note:** The overall UI theme is currently dark-only. Multiple UI themes are planned (see [ROADMAP.md](ROADMAP.md)).

### Fonts
Adjust the editor font size and family in Settings:
- Font Size: 12–36pt
- Font Family: Select from bundled fonts or system fonts (when enabled)

---

## Keyboard Shortcuts

| Category | Shortcut | Action | Status |
|----------|----------|--------|--------|
| **File** | `Ctrl+S` | Save | ✓ Working |
| | `Ctrl+N` | New File | ✓ Working |
| | `Ctrl+O` | Open File | ✓ Working |
| | `Ctrl+Shift+S` | Save As | ✓ Working |
| | `Ctrl+W` | Close Tab | ✓ Working |
| **Edit** | `Ctrl+F` | Find / Search | ✓ Working |
| | `Ctrl+G` | Go to Line | ✓ Working |
| | `Ctrl+/` | Toggle Comment | ✓ Working |
| | `Ctrl+D` | Duplicate Line | ✓ Working |
| | `Ctrl+Space` | Trigger Autocomplete | ✓ Working |
| **Navigation** | `Alt+Up/Down` | Move line up/down | ✓ Working |
| | `Ctrl+Tab` | Next Tab | ✓ Working |
| | `Ctrl+Shift+Tab` | Previous Tab | ✓ Working |
| **View** | `F6` | Toggle Console | ✓ Working |
| | `Ctrl++/-` | Zoom in/out | ✓ Working |
| | `Ctrl+0` | Reset Zoom | ✓ Working |
| | `Ctrl+Shift+F` | Project Search | ✓ Working |
| **Build** | `F5` | Run / Build | ✓ Working |

---

## Search and Replace

### Editor Search (`Ctrl+F`)
Find text within the current file:
- Find next: Enter or click down arrow
- Find previous: Shift+Enter or click up arrow
- Case-sensitive toggle: Click "Cc" button
- Wrap-around: Automatically wraps to beginning/end

**Note:** Replace functionality is planned for a future release (see [ROADMAP.md](ROADMAP.md)).

### Project Search (`Ctrl+Shift+F`)
Search across all files in the opened project folder.

---

## Snippets

Type the trigger and press **Tab** to expand:

| Trigger | Expands To |
|---------|------------|
| `main` | `صحيح الرئيسية() { ... }` |
| `if` | `إذا (condition) { ... }` |
| `ifelse` | `إذا (condition) { ... } وإلا { ... }` |
| `for` | `لكل (init؛ condition؛ increment) { ... }` |
| `while` | `طالما (condition) { ... }` |
| `func` | Function definition template |
| `class` | Class definition template |

After expanding, press **Tab** to jump between placeholders (e.g., function name, parameters).

---

## Troubleshooting

### "Compiler not found" error
- Ensure the Baa compiler is installed.
- The IDE looks in this order:
  1. Path specified in Settings
  2. `baa/baa.exe` relative to the IDE
  3. System PATH

### Auto-save files
Backup files use the `.~` suffix (e.g., `program.baa.~`). These are automatically cleaned up when you save or close the file.

### File won't open
- Check that the file is UTF-8 encoded.
- Files larger than 50MB are blocked for safety.
- Files larger than 10MB show a warning.

---

## Development Status

Qalam IDE is actively developed. See [ROADMAP.md](ROADMAP.md) for planned features including:
- Multi-cursor editing
- Split editor
- Minimap
- LSP support
- Git integration
- Debugger

---

*[→ Language Specification](LANGUAGE.md) | [→ Compiler Internals](INTERNALS.md) | [→ Development Roadmap](ROADMAP.md)*
