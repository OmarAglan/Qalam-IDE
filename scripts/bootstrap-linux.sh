#!/usr/bin/env bash
set -euo pipefail

CONFIGURATION="${CONFIGURATION:-Release}"
QT_VERSION="${QT_VERSION:-6.10.2}"
QT_ARCH="${QT_ARCH:-gcc_64}"
QT_ROOT="${QT_ROOT:-$HOME/Qt}"

cd "$(dirname "${BASH_SOURCE[0]}")/.."

log() { printf '\n==> %s\n' "$1"; }

ensure_command() {
  command -v "$1" >/dev/null 2>&1
}

install_apt_packages() {
  if ensure_command apt-get; then
    log "Installing Linux build dependencies with apt"
    sudo apt-get update
    sudo apt-get install -y build-essential cmake python3 python3-pip libgl1-mesa-dev libxkbcommon-dev libxcb-cursor0 libxcb-cursor-dev libxcb-xinerama0 libxcb-xinerama0-dev
  elif ensure_command dnf; then
    log "Installing Linux build dependencies with dnf"
    sudo dnf install -y gcc-c++ cmake python3 python3-pip mesa-libGL-devel libxkbcommon-devel xcb-util-cursor-devel libxcb-devel
  elif ensure_command pacman; then
    log "Installing Linux build dependencies with pacman"
    sudo pacman -Sy --needed base-devel cmake python python-pip mesa libxkbcommon xcb-util-cursor libxcb
  else
    echo "Unsupported package manager. Install C++ compiler, CMake, Python 3, pip, OpenGL/XCB Qt runtime packages manually." >&2
  fi
}

find_qt_root() {
  local match
  match="$(find "$QT_ROOT" -path "*/$QT_VERSION/*/lib/cmake/Qt6/Qt6Config.cmake" -print -quit 2>/dev/null || true)"
  if [[ -n "$match" ]]; then
    dirname "$(dirname "$(dirname "$(dirname "$match")")")"
  fi
}

install_apt_packages

log "Installing aqtinstall"
python3 -m pip install --user --upgrade pip aqtinstall
export PATH="$HOME/.local/bin:$PATH"

RESOLVED_QT_ROOT="$(find_qt_root || true)"
if [[ -z "$RESOLVED_QT_ROOT" ]]; then
  log "Installing Qt $QT_VERSION ($QT_ARCH) to $QT_ROOT"
  python3 -m aqt install-qt -O "$QT_ROOT" linux desktop "$QT_VERSION" "$QT_ARCH"
  RESOLVED_QT_ROOT="$(find_qt_root || true)"
fi

if [[ -z "$RESOLVED_QT_ROOT" ]]; then
  echo "Qt was installed, but Qt6Config.cmake was not found under $QT_ROOT" >&2
  exit 1
fi

export QALAM_QT_DIR="$RESOLVED_QT_ROOT"

log "Building Qalam IDE"
cmake -S . -B build/linux-release -DCMAKE_BUILD_TYPE="$CONFIGURATION" -DCMAKE_PREFIX_PATH="$QALAM_QT_DIR"
cmake --build build/linux-release --parallel

log "Qalam IDE is ready"
echo "Executable: build/linux-release/qalam/Qalam"
