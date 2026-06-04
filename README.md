# Qalam IDE - محرر قلم

[![Qt](https://img.shields.io/badge/Qt-6.x-41CD52?logo=qt&logoColor=white)](https://www.qt.io/)
[![C++](https://img.shields.io/badge/C%2B%2B-23-00599C?logo=c%2B%2B&logoColor=white)](https://en.cppreference.com/w/cpp/23)
[![Platforms](https://img.shields.io/badge/Platforms-Windows%20%7C%20Linux%20%7C%20macOS-2ea44f)](#build--run)

**English:** Qalam IDE is a fast, right-to-left friendly, Qt-powered IDE designed for Arabic-syntax system languages — starting with **Baa (باء)**.  
**العربية:** محرر **قلم** هو بيئة تطوير سريعة تدعم اتجاه الكتابة من اليمين إلى اليسار، ومصممة للغات الأنظمة ذات الصياغة العربية — بدايةً مع **لغة باء**.

> Note: Qalam IDE is the evolution of the Taif editor, now fully aligned with the Baa programming language.

---

## Why Qalam?

**English:** “Qalam” (قلم) is the symbol of writing, clarity, and craftsmanship. This project aims to bring a polished developer experience to Arabic-native programming.  
**العربية:** “قلم” يجمع بين المعنى والهوية: كتابة واضحة وتجربة تطوير احترافية للبرمجة بالعربية.

---

## Language: Baa (باء)

- **Spec:** See [`documents/LANGUAGE.md`](documents/LANGUAGE.md) (Baa Language Specification).
- **File extension (spec):** `.baa`
- **Entry point (spec):** `الرئيسية`

---

## Key Features (from the current implementation)

### Editor experience
- **Custom syntax highlighting engine** (lexer + Qt highlighter) with stateful multi-line handling.
- **Theme engine** (multiple built-in code themes).
- **Auto-completion framework** with multiple strategies:
  - Snippets
  - Keywords
  - Built-ins
  - Dynamic words from the current document
- **Auto-save & crash recovery**
  - Periodic auto-save to `file.~`
  - Recovery prompt if a newer backup exists
- **Code folding (lightweight)** and **line numbers**
- **Smart editing**
  - Auto-pairing for brackets and quotes
  - Indentation helper on Enter
- **Drag & drop** open for supported files
- **Right-to-left (RTL) first**
  - RTL UI layout direction
  - RTL editor text direction & alignment

### IDE features
- **Embedded interactive console** with command history and fast flush buffering
- **Run integration** (executes `baa/baa.exe` for the current file)
- **File explorer sidebar** (QTreeView + QFileSystemModel)
- **Welcome screen**
  - Recent files list
  - New file / Open file / Open folder

---

## Project Structure

- `qalam/` — Qt application entry, main window, and resources.
- `source/` — Reusable components (editor, console, settings, menu bar, welcome window).
- `documents/` — Language specifications, user guide, and deployment notes.

---

## Build & Run

Qalam IDE is a Qt Widgets application written in C++23.

### Requirements

- Qt 6.x with the Widgets / GUI / Core modules
- CMake 3.21+
- A C++23-capable compiler
  - Windows: Qt MinGW kit is the easiest path
  - Linux: GCC 13+ recommended
  - macOS: Clang 16+ recommended

### Windows: one-command bootstrap recommended

On a fresh Windows machine, open PowerShell, Command Prompt, or Windows Terminal in the repository root and run:

```bat
build-qalam-windows.cmd
```

You can also run the PowerShell script directly:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\bootstrap-windows.ps1
```

The `.cmd` launcher uses `-ExecutionPolicy Bypass` only for the current PowerShell process, so users do not need to permanently change their Windows script policy.

The bootstrap script attempts to install the missing base tools with `winget`, installs Qt + MinGW with `aqtinstall`, builds Qalam, and creates the portable ZIP.

Output:

- Build: `build/windows-release/qalam/Qalam.exe`
- Portable package: `dist/Qalam-win64.zip`

Useful options:

```bat
REM Build only, do not create dist/Qalam-win64.zip
build-qalam-windows.cmd -NoPackage

REM Use a custom Qt install root
build-qalam-windows.cmd -QtRoot "D:\Qt"

REM Use an already installed Qt and skip downloading Qt
build-qalam-windows.cmd -SkipQtInstall -QtRoot "C:\Qt"
```

If Windows still blocks scripts after using the launcher, unblock the downloaded project files once after verifying the source:

```powershell
Get-ChildItem -Recurse -File | Unblock-File
```

### Windows: manual quick path

If Qt 6 + MinGW is already installed, run PowerShell from the repository root:

```powershell
# Optional when Qt is not installed under C:\Qt
$env:QALAM_QT_DIR = "C:\Qt\6.10.2\mingw_64"

.\scripts\build-windows.ps1 -Configuration Release
.\scripts\package-windows.ps1 -SkipBuild
```

### Windows: manual CMake path

```powershell
$env:QALAM_QT_DIR = "C:\Qt\6.10.2\mingw_64"
cmake --preset windows-release
cmake --build --preset release
```

If your Qt is exactly at `C:\Qt\6.10.2\mingw_64`, you can also use:

```powershell
cmake --preset windows-release-qt6102
cmake --build --preset release-qt6102
```

### Linux

On Ubuntu/Fedora/Arch-like systems, the low-hassle path is:

```bash
./scripts/bootstrap-linux.sh
```

If Qt is already installed:

```bash
./scripts/build-linux.sh Release
```

Or manually:

```bash
cmake -S . -B build/linux-release -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/opt/Qt/6.x/gcc_64
cmake --build build/linux-release --parallel
```

### macOS

On macOS with Homebrew available:

```bash
./scripts/bootstrap-macos.sh
```

### Build with Qt Creator

1. Open the root `CMakeLists.txt`.
2. Select a Qt 6 kit.
3. Configure, build, and run.

### qmake fallback

`qalam/Qalam.pro` has been refreshed for the current source tree, but CMake is still the main supported build system. Use qmake only as a fallback.

---

## Deployment & Packaging

For detailed instructions on how to deploy Qalam IDE on Windows, Linux, and macOS, see [`documents/deployment.md`](documents/deployment.md).


## Keyboard Shortcuts

The following shortcuts are implemented:

| Shortcut | Action |
|---|---|
| `Ctrl+S` | Save |
| `Ctrl+F` | Find / Search bar |
| `Ctrl+G` | Go to line |
| `Ctrl+/` | Toggle comment |
| `Ctrl+D` | Duplicate line |
| `Alt+Up` | Move line up |
| `Alt+Down` | Move line down |
| `F6` | Toggle embedded console |
| `Ctrl + Mouse Wheel` | Zoom editor font in/out |
| `Ctrl+L` | Clear embedded console (focus in console) |

---

## More Documentation

- [**Baa Language Specification**](documents/LANGUAGE.md)
- [**User Guide**](documents/USER_GUIDE.md) (How to use the IDE)
- [**Internal Architecture**](documents/INTERNALS.md) (For contributors)

---

## Download Qt 6

- Qt mirror list: https://download.qt.io/static/mirrorlist/
- Example installer mirror usage:
  - `NameOfQtOnlineInstaller.exe --mirror https://mirrors.ocf.berkeley.edu/qt/`
