# Qalam IDE - محرر قلم

[![Qt](https://img.shields.io/badge/Qt-6.x-41CD52?logo=qt&logoColor=white)](https://www.qt.io/)
[![C++](https://img.shields.io/badge/C%2B%2B-23-00599C?logo=c%2B%2B&logoColor=white)](https://en.cppreference.com/w/cpp/23)
[![Platforms](https://img.shields.io/badge/Platforms-Windows%20%7C%20Linux%20%7C%20macOS-2ea44f)](#build--run)

**English:** Qalam IDE is a fast, right-to-left friendly, Qt-powered IDE designed for Arabic-syntax system languages — starting with **Baa (باء)**.  
**العربية:** محرر **قلم** هو بيئة تطوير سريعة تدعم اتجاه الكتابة من اليمين إلى اليسار، ومصممة للغات الأنظمة ذات الصياغة العربية — بدايةً مع **لغة باء**.

> Note: the current codebase still contains legacy naming (Taif/Alif) that will be refactored in the next steps of this rebrand.

---

## Why Qalam?

**English:** “Qalam” (قلم) is the symbol of writing, clarity, and craftsmanship. This project aims to bring a polished developer experience to Arabic-native programming.  
**العربية:** “قلم” يجمع بين المعنى والهوية: كتابة واضحة وتجربة تطوير احترافية للبرمجة بالعربية.

---

## Language: Baa (باء)

- **Spec:** See [`LANGUAGE.md`](LANGUAGE.md) (Baa Language Specification).
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
- **Run integration** (currently wired to run the legacy “Alif” compiler executable; will be aligned to Baa during rebranding)
- **File explorer sidebar** (QTreeView + QFileSystemModel)
- **Welcome screen**
  - Recent files list
  - New file / Open file / Open folder

---

## Project Structure

- `taif/` — Qt application entry, resources, qmake project file
- `source/` — reusable components (editor, console, settings, menu bar, welcome window)

---

## Build & Run

Qalam IDE is a Qt Widgets application written in C++23.

### Requirements (all platforms)

- Qt **6.x** (Widgets / GUI / Core)
- A C++23-capable compiler
- **qmake** (this project currently uses qmake via a `.pro` file)

> The qmake project file is currently located at: `taif/Taif.pro` (will be renamed to Qalam later).

---

## Windows (Qt 6 + MinGW or MSVC)

### Option A — Build with Qt Creator (recommended)
1. Install Qt 6.x using the Qt Online Installer.
2. Open `taif/Taif.pro` in Qt Creator.
3. Select a Qt 6 kit (MinGW 64-bit or MSVC).
4. Build & run.

### Option B — Build with command line (qmake + make)
From the repository root:

```powershell
cd taif
qmake .\Taif.pro
mingw32-make -j
```

> If using MSVC kits, use the matching Qt command prompt and `nmake` instead of `mingw32-make`.

### Deploy (windeployqt)
After building `Taif.exe` (name will change later), run:

```powershell
# Example path from deployment.md (adjust to your Qt install)
~:\Qt\6.*.*\mingw_64\bin\windeployqt6.exe .\Taif.exe
```

### Packaging (Qt Installer Framework)
From [`deployment.md`](deployment.md):

```powershell
~:\Qt\Tools\QtInstallerFramework\4.*\bin\binarycreator.exe -c config/config.xml -p packages TaifInstaller-Win-X64
```

---

## Linux (Ubuntu 22.04+)

> The existing deployment notes explicitly target Ubuntu 22.

### Install dependencies

```bash
sudo apt update
sudo apt install -y build-essential qt6-base-dev qt6-base-dev-tools
sudo apt install -y libxcb-cursor0 libxcb-cursor-dev
```

> Your existing notes mention `libxcb-cusor-dev` (typo). The common package name is `libxcb-cursor-dev`.

### Build with Qt Creator
1. Install Qt 6 (or use distro Qt 6 packages).
2. Open `taif/Taif.pro` in Qt Creator.
3. Configure with a Qt 6 kit and build.

### (Optional) Make Qt binaries available in your PATH
From [`deployment.md`](deployment.md):

```bash
# Add Qt bin folder to PATH (adjust path)
export PATH=/path/to/qt/bin:$PATH
```

### Deploy (linuxdeployqt)
From [`deployment.md`](deployment.md):

```bash
sudo apt install -y libfuse2

# Download linuxdeployqt AppImage (continuous)
chmod a+x linuxdeployqt-continuous-x86_64.AppImage

./linuxdeployqt-continuous-x86_64.AppImage AppNameHere -always-overwrite
```

If linuxdeployqt cannot find qmake:

```bash
./linuxdeployqt-continuous-x86_64.AppImage AppNameHere -always-overwrite -qmake=/home/name/Qt/6.*.*/gcc_64/bin/qmake
```

---

## macOS (Qt 6)

### Build with Qt Creator
1. Install Qt 6.x for macOS using the Qt Online Installer.
2. Open `taif/Taif.pro` in Qt Creator.
3. Select a Desktop Qt 6 kit (clang) and build.

### Deploy (macdeployqt)
After building the `.app`, run (from a Qt terminal with macdeployqt in PATH):

```bash
macdeployqt Qalam.app
```

---

## Keyboard Shortcuts

The following shortcuts are implemented in code:

| Shortcut | Action |
|---|---|
| `Ctrl+S` | Save |
| `Ctrl+F` | Find / Search bar |
| `Ctrl+G` | Go to line |
| `Ctrl+/` | Toggle comment (current implementation uses `#` as the line comment marker) |
| `Ctrl+D` | Duplicate line |
| `Alt+Up` | Move line up |
| `Alt+Down` | Move line down |
| `F6` | Toggle embedded console |
| `Ctrl + Mouse Wheel` | Zoom editor font in/out |
| `Ctrl+L` | Clear embedded console (when focus is in console input) |

---

## Notes & Roadmap (rebrand)

- Rename Taif → Qalam across source, build system, resources, and UI strings.
- Align language-specific UX to Baa:
  - File extensions, highlighting keywords, snippets, built-ins
  - Run/compile integration to Baa toolchain
- Consider migrating from qmake to CMake for smoother cross-platform builds.

---

## Download Qt 6

From [`deployment.md`](deployment.md):

- Qt mirror list: https://download.qt.io/static/mirrorlist/
- Example installer mirror usage:
  - `NameOfQtOnlineInstaller.exe --mirror https://mirrors.ocf.berkeley.edu/qt/`