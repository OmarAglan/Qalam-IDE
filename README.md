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

### Requirements (all platforms)

- Qt **6.x** (Widgets / GUI / Core)
- A C++23-capable compiler
- **CMake** 3.21+ (recommended) or qmake

---

## Building with CMake (Recommended)

From the repository root:

```powershell
mkdir build
cd build
cmake ..
cmake --build .
```

### Build with Qt Creator
1. Open `CMakeLists.txt` from the root directory.
2. Select a Qt 6 kit.
3. Build & run.

---

## Building with qmake (Legacy)

The qmake project file is located at `qalam/Qalam.pro`.

### Build with Qt Creator
1. Open `qalam/Qalam.pro`.
2. Select a Qt 6 kit.
3. Build & run.

### Build with command line
```powershell
cd qalam
qmake .\Qalam.pro
mingw32-make -j
```

---

## Deployment & Packaging

For detailed instructions on how to deploy Qalam IDE on Windows, Linux, and macOS, see [`documents/deployment.md`](documents/deployment.md).

---

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
