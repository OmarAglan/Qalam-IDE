#!/usr/bin/env bash
set -euo pipefail

CONFIGURATION="${CONFIGURATION:-Release}"
QT_VERSION="${QT_VERSION:-6.10.2}"
QT_ARCH="${QT_ARCH:-clang_64}"
QT_ROOT="${QT_ROOT:-$HOME/Qt}"

cd "$(dirname "${BASH_SOURCE[0]}")/.."

log() { printf '\n==> %s\n' "$1"; }

if ! command -v xcode-select >/dev/null 2>&1 || ! xcode-select -p >/dev/null 2>&1; then
  log "Installing Xcode Command Line Tools"
  xcode-select --install
  echo "Run this script again after the Xcode Command Line Tools installer finishes."
  exit 1
fi

if ! command -v brew >/dev/null 2>&1; then
  echo "Homebrew is required for the low-hassle macOS bootstrap. Install Homebrew first: https://brew.sh" >&2
  exit 1
fi

log "Installing macOS build dependencies with Homebrew"
brew install cmake python aqtinstall || true

find_qt_root() {
  local match
  match="$(find "$QT_ROOT" -path "*/$QT_VERSION/*/lib/cmake/Qt6/Qt6Config.cmake" -print -quit 2>/dev/null || true)"
  if [[ -n "$match" ]]; then
    dirname "$(dirname "$(dirname "$(dirname "$match")")")"
  fi
}

RESOLVED_QT_ROOT="$(find_qt_root || true)"
if [[ -z "$RESOLVED_QT_ROOT" ]]; then
  log "Installing Qt $QT_VERSION ($QT_ARCH) to $QT_ROOT"
  python3 -m aqt install-qt -O "$QT_ROOT" mac desktop "$QT_VERSION" "$QT_ARCH"
  RESOLVED_QT_ROOT="$(find_qt_root || true)"
fi

if [[ -z "$RESOLVED_QT_ROOT" ]]; then
  echo "Qt was installed, but Qt6Config.cmake was not found under $QT_ROOT" >&2
  exit 1
fi

export QALAM_QT_DIR="$RESOLVED_QT_ROOT"

log "Building Qalam IDE"
cmake -S . -B build/macos-release -DCMAKE_BUILD_TYPE="$CONFIGURATION" -DCMAKE_PREFIX_PATH="$QALAM_QT_DIR"
cmake --build build/macos-release --parallel

log "Qalam IDE is ready"
echo "Bundle: build/macos-release/qalam/Qalam.app"
