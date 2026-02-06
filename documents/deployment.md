# Deployment Guide

This document provides instructions for deploying and packaging Qalam IDE on different platforms.

## Prerequisites

Before deploying, ensure you have:
- Qt 6.10.2 or later installed via [Qt Online Installer](https://download.qt.io/static/mirrorlist/)
- CMake 3.21+
- MinGW 13.1+ (Windows), GCC 13+ (Linux), or Clang 16+ (macOS)

## Windows (MinGW)

### 1. Build

Use CMake to build the `Qalam.exe` executable in Release mode:

```powershell
# Configure
cmake --preset windows-release

# Build
cmake --build build --preset release
```

Output: `build/qalam/Qalam.exe`

**Note:** The `.pro` file (qmake) is deprecated and cannot build the current codebase. Use CMake only.

### 2. Deploy with windeployqt

Run `windeployqt` to copy necessary DLLs to the build folder:

```powershell
# Adjust path to your Qt installation
~:\Qt\6.10.2\mingw_64\bin\windeployqt6.exe .\build\qalam\Qalam.exe
```

This will copy:
- Qt runtime DLLs
- Platform plugins (`platforms/`, `styles/`)
- Required image format plugins

### 3. Package (Optional)

For distribution, you can:
- Zip the entire `qalam/` folder containing the executable and DLLs
- Or create an installer using Qt Installer Framework (if you have config files set up)

```powershell
# Zip for distribution
Compress-Archive -Path .\build\qalam\* -DestinationPath Qalam-v3.3.0-win64.zip
```

### 4. Single-Instance Check

Qalam uses `QLockFile` to ensure only one instance runs. If the app fails to start, check for a stale lock file at `%TEMP%/qalam.lock`.

---

## Linux (Ubuntu 22.04+)

### 1. Install Dependencies

```bash
sudo apt update
sudo apt install -y build-essential qt6-base-dev qt6-base-dev-tools cmake
sudo apt install -y libxcb-cursor0 libxcb-cursor-dev
```

### 2. Environment Setup

Add the Qt bin directory to your `PATH` if you installed Qt in a non-standard location:

```bash
export PATH=/opt/Qt/6.10.2/gcc_64/bin:$PATH
```

### 3. Build

```bash
# Configure (Note: Linux presets need to be added - see ROADMAP.md Phase 7)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/opt/Qt/6.10.2/gcc_64

# Build
cmake --build build -j$(nproc)
```

### 4. Deploy with linuxdeployqt

```bash
sudo apt install -y libfuse2

# Download linuxdeployqt
curl -LO https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
chmod a+x linuxdeployqt-continuous-x86_64.AppImage

# Deploy
./linuxdeployqt-continuous-x86_64.AppImage build/qalam/Qalam -appimage -always-overwrite
```

**Note for Ubuntu 24.04+:** You may need `-unsupported-bundle-everything -unsupported-allow-new-glibc` flags.

---

## macOS

### 1. Build

```bash
# Configure (Note: macOS presets need to be added - see ROADMAP.md Phase 7)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/usr/local/Qt/6.10.2/macos

# Build
cmake --build build -j$(sysctl -n hw.ncpu)
```

### 2. Deploy with macdeployqt

```bash
macdeployqt build/qalam/Qalam.app
```

This bundles Qt frameworks into the .app bundle for distribution.

---

## CI/CD Pipeline (Future)

Automated builds are planned. See [ROADMAP.md](ROADMAP.md) Phase 7 for details on:
- GitHub Actions workflows
- Multi-platform build matrix
- Automated testing with `ctest`
- Release artifact publishing

---

## Known Issues

1. **Vulkan DLL Warning:** During Windows linking, you may see a warning about Vulkan DLL. This is harmless and can be ignored.

2. **RTL on Windows:** The frameless window implementation uses native Windows APIs (`dwmapi`, `user32`) which work best on Windows 10/11.

3. **Console Colors:** ANSI color codes in the embedded console are planned but not yet fully functional (see ROADMAP.md Phase 5.4).

---

## Additional Resources

- **Qt Downloads:** https://download.qt.io/static/mirrorlist/
- **Qt Installer CLI:** `NameOfQtOnlineInstaller.exe --mirror https://mirrors.ocf.berkeley.edu/qt/`
- **Project Documentation:** See `documents/` folder for [INTERNALS.md](INTERNALS.md), [LANGUAGE.md](LANGUAGE.md), and [ROADMAP.md](ROADMAP.md)

---

*[← Back to User Guide](USER_GUIDE.md) | [→ Compiler Internals](INTERNALS.md)*
