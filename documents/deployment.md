# Deployment Guide

This document provides instructions for deploying and packaging Qalam IDE on different platforms.

## Windows (MinGW / MSVC)

### Prerequisites
- Qt 6.x installed via Qt Online Installer.
- CMake or qmake.

### 1. Build
Use CMake (recommended) or qmake to build the `Qalam.exe` executable in Release mode.

### 2. Deployment (windeployqt)
Run `windeployqt` to copy necessary DLLs to the build folder:
```powershell
# Adjust path to your Qt installation
~:\Qt\6.x.x\mingw_64\bin\windeployqt6.exe .\Qalam.exe
```

### 3. Packaging (Qt Installer Framework)
```powershell
# Adjust path to your QtIFW installation
~:\Qt\Tools\QtInstallerFramework\4.x\bin\binarycreator.exe -c config/config.xml -p packages QalamInstaller-Win-X64
```

## Linux (Ubuntu 22.04+)

> **Note:** Official support is currently tested on Ubuntu 22.04.

### 1. Install Dependencies
```bash
sudo apt update
sudo apt install -y build-essential qt6-base-dev qt6-base-dev-tools cmake
sudo apt install -y libxcb-cursor0 libxcb-cursor-dev
```

### 2. Environment Setup
Add the Qt bin directory to your `PATH`:
```bash
export PATH=/path/to/qt/bin:$PATH
# To make it permanent:
echo 'export PATH=/path/to/qt/bin:$PATH' >> ~/.bashrc
source ~/.bashrc
```

### 3. Deploy (linuxdeployqt)
```bash
sudo apt install -y libfuse2
# Download linuxdeployqt-continuous-x86_64.AppImage from GitHub
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
./linuxdeployqt-continuous-x86_64.AppImage Qalam -always-overwrite
```
*Note: If building on Ubuntu 24.04+, you may need `-unsupported-bundle-everything -unsupported-allow-new-glibc`.*

## macOS

### 1. Build
Build the `Qalam.app` bundle using CMake or qmake in Qt Creator.

### 2. Deploy (macdeployqt)
```bash
macdeployqt Qalam.app
```

---

## Download Qt 6
- **Mirrors:** [https://download.qt.io/static/mirrorlist/](https://download.qt.io/static/mirrorlist/)
- **CLI Example:** `NameOfQtOnlineInstaller.exe --mirror https://mirrors.ocf.berkeley.edu/qt/`
