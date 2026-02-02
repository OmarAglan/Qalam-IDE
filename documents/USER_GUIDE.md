# Qalam IDE User Guide

Welcome to **Qalam IDE**, the professional environment for Arabic-syntax programming.

## Getting Started

### Creating your first Baa file
1. Click **ملف جديد** (New File) on the Welcome Screen or press `Ctrl+N`.
2. Write your code using Baa syntax (see [Language Spec](LANGUAGE.md)).
3. Save the file with a `.baa` extension using `Ctrl+S`.

### Running your code
1. Open your `.baa` file.
2. Ensure you have the Baa compiler installed (default path: `baa/baa.exe` relative to the IDE).
3. The IDE is designed to integrate with the Baa toolchain for compilation and execution.

## Interface Overview

### The Editor
The editor supports:
- **Syntax Highlighting:** Automatic coloring for Baa keywords, types, and directives.
- **RTL Support:** Full right-to-left support for both the interface and the code.
- **Auto-completion:** Suggested keywords and snippets appear as you type.

### The Console (`F6`)
The embedded console allows you to interact with the system and view compiler output.
- Use `Ctrl+L` to clear the console.
- Command history is available using the Up/Down keys.

### File Explorer
The left sidebar shows your project files. You can double-click a file to open it or drag and drop files from your system.

## Customization

### Themes
You can change the editor theme in the settings. Qalam comes with several built-in themes optimized for dark and light environments.

## Keyboard Shortcuts

| Category | Shortcut | Action |
|---|---|---|
| **File** | `Ctrl+S` | Save |
| | `Ctrl+N` | New File |
| | `Ctrl+O` | Open File |
| **Edit** | `Ctrl+F` | Find / Search |
| | `Ctrl+G` | Go to Line |
| | `Ctrl+/` | Toggle Comment |
| | `Ctrl+D` | Duplicate Line |
| **Logic**| `Alt+Up/Down` | Move line up/down |
| **View** | `F6` | Toggle Console |
| | `Ctrl++/-` | Zoom in/out |
| | `Ctrl+0` | Reset Zoom |
