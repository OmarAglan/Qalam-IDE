#!/usr/bin/env bash
set -euo pipefail

BUILD_TYPE="${1:-Release}"
QT_ROOT="${QALAM_QT_DIR:-${QTDIR:-}}"
BUILD_DIR="build/linux-${BUILD_TYPE,,}"

args=(-S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE")
if [[ -n "$QT_ROOT" ]]; then
  args+=(-DCMAKE_PREFIX_PATH="$QT_ROOT")
fi

cmake "${args[@]}"
cmake --build "$BUILD_DIR" --parallel
