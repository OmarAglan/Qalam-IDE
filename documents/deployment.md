# Deployment Guide

This document explains how to build and package Qalam IDE on Windows, Linux, and macOS.

## Prerequisites

- Qt 6.x with Widgets / GUI / Core installed
- CMake 3.21+
- C++23 compiler
  - Windows: Qt MinGW kit recommended
  - Linux: GCC 13+ recommended
  - macOS: Clang 16+ recommended

---

## Windows: Build and Package

### Option A: PowerShell scripts (recommended)

From the repository root:

```powershell
# Optional when Qt is not installed under C:\Qt
$env:QALAM_QT_DIR = "C:\Qt\6.10.2\mingw_64"

.\scripts\build-windows.ps1 -Configuration Release
.\scripts\package-windows.ps1 -SkipBuild
```

Outputs:

- Executable: `build/windows-release/qalam/Qalam.exe`
- Portable folder: `dist/Qalam-win64/`
- ZIP package: `dist/Qalam-win64.zip`

The packaging script runs `windeployqt.exe`, which copies the Qt runtime DLLs, platform plugins, styles, and related runtime files into the package folder.

### Option B: CMake presets

When Qt is not in the default `C:\Qt\6.10.2\mingw_64` location:

```powershell
$env:QALAM_QT_DIR = "C:\Qt\6.10.2\mingw_64"
cmake --preset windows-release
cmake --build --preset release
.\scripts\package-windows.ps1 -SkipBuild
```

When Qt is exactly in `C:\Qt\6.10.2\mingw_64` and MinGW is exactly in `C:\Qt\Tools\mingw1310_64`:

```powershell
cmake --preset windows-release-qt6102
cmake --build --preset release-qt6102
.\scripts\package-windows.ps1 -SkipBuild
```

### Optional: Deploy after every build

This is convenient for local manual testing, but slower for normal development:

```powershell
.\scripts\build-windows.ps1 -Configuration Release -DeployAfterBuild
```

### Baa compiler location on Windows

The IDE now resolves the Baa compiler in this order:

1. The configured compiler path from settings.
2. `baa/baa.exe` next to `Qalam.exe`.
3. `baa.exe` next to `Qalam.exe`.
4. `baa.exe` or `baa` from `PATH`.

For portable releases, place the compiler here:

```text
Qalam-win64/
  Qalam.exe
  baa/
    baa.exe
```

---

## Windows Application Icon

The Windows executable icon is configured through `qalam/resources/Qalam.rc` and included by CMake. The `.qrc` file still embeds icons for use inside the running Qt application, but the `.rc` file is what gives `Qalam.exe` its Explorer/taskbar icon.

---

## Linux

### Install dependencies on Ubuntu/Debian

```bash
sudo apt update
sudo apt install -y build-essential cmake qt6-base-dev qt6-base-dev-tools libxcb-cursor0 libxcb-cursor-dev
```

### Build

```bash
./scripts/build-linux.sh Release
```

Or manually:

```bash
cmake -S . -B build/linux-release -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/opt/Qt/6.x/gcc_64
cmake --build build/linux-release --parallel
```

### Runtime compiler layout

Place the Baa compiler beside the executable:

```text
build/linux-release/qalam/
  Qalam
  baa/
    baa
```

Make it executable:

```bash
chmod +x build/linux-release/qalam/baa/baa
```

---

## macOS

```bash
export QALAM_QT_DIR=/Users/$USER/Qt/6.x/macos
cmake --preset macos-release
cmake --build --preset macos-release
macdeployqt build/macos-release/qalam/Qalam.app
```

Runtime compiler layout inside or beside the app should be finalized when macOS packaging is ready.

---

## qmake Fallback

`qalam/Qalam.pro` has been refreshed and includes the current source tree, Windows native libraries, and the Windows icon. However, CMake remains the primary supported build system.

```powershell
cd qalam
qmake Qalam.pro
mingw32-make -j
```

---

## Known Issues / Next Work

1. **Compiler settings UI:** The key already exists (`compilerPath`), but the settings window still needs a proper compiler path picker.
2. **Windows terminal:** The embedded terminal starts `cmd.exe` using UTF-8 code page setup, but a future terminal layer should support PowerShell and better ANSI color rendering.
3. **CI:** GitHub Actions should build Windows, Linux, and macOS artifacts automatically.
4. **Packaging installer:** Current Windows packaging creates a portable ZIP. A proper installer can be added later with Qt Installer Framework or another installer system.
5. **Baa compiler bundling:** `scripts/package-windows.ps1` copies a local `baa/` folder if present. If the compiler lives in another repository, the release process should fetch or build it first.

---

*[← Back to User Guide](USER_GUIDE.md) | [→ Compiler Internals](INTERNALS.md)*
