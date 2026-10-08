#!/usr/bin/env bash
# Package Qalam as a Linux AppImage: Qalam, its Qt runtime, and the internal
# Baa-LSP only. Like the Windows installer, it carries no Baa, Takween, Nazm,
# or linker and changes nothing outside its own file; Qalam discovers those
# tools as documented in the user guide.
set -euo pipefail

usage() {
  cat >&2 <<'EOF'
usage: scripts/package-linux.sh --build-dir DIR --baa-lsp FILE [--qt-dir DIR]
                                [--output-dir DIR] [--tools-dir DIR]

  --build-dir   Qalam Release build directory (contains qalam/Qalam)
  --baa-lsp     Baa-LSP executable to bundle as Qalam's internal server
  --qt-dir      Qt 6 root such as ~/Qt/6.10.2/gcc_64 (default: $QALAM_QT_DIR)
  --output-dir  where the AppImage and its .sha256 go (default: dist/linux)
  --tools-dir   cache for the pinned packaging tools (default: build/appimage-tools)
EOF
  exit 2
}

build_dir=""
baa_lsp=""
qt_dir="${QALAM_QT_DIR:-}"
output_dir="dist/linux"
tools_dir="build/appimage-tools"
while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-dir) build_dir="$2"; shift 2 ;;
    --baa-lsp) baa_lsp="$2"; shift 2 ;;
    --qt-dir) qt_dir="$2"; shift 2 ;;
    --output-dir) output_dir="$2"; shift 2 ;;
    --tools-dir) tools_dir="$2"; shift 2 ;;
    *) usage ;;
  esac
done
[[ -n "$build_dir" && -n "$baa_lsp" && -n "$qt_dir" ]] || usage

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
qalam="$build_dir/qalam/Qalam"
[[ -x "$qalam" ]] || { echo "Qalam executable not found: $qalam" >&2; exit 1; }
[[ -x "$baa_lsp" ]] || { echo "Baa-LSP executable not found: $baa_lsp" >&2; exit 1; }
[[ -x "$qt_dir/bin/qmake" ]] || { echo "qmake not found under Qt root: $qt_dir" >&2; exit 1; }

version="$(sed -n 's/^project(QalamIDE VERSION \([0-9.]*\).*/\1/p' "$root/CMakeLists.txt")"
[[ -n "$version" ]] || { echo "Could not read Qalam's version from CMakeLists.txt" >&2; exit 1; }

# Packaging tools are pinned by release and SHA-256 and verified before they run.
mkdir -p "$tools_dir"
fetch() {
  local name="$1" url="$2" sha="$3" path="$tools_dir/$1"
  if ! echo "$sha  $path" | sha256sum --check --status 2>/dev/null; then
    curl -fsSL --retry 3 -o "$path.part" "$url"
    if ! echo "$sha  $path.part" | sha256sum --check --status; then
      rm -f "$path.part"
      echo "Checksum mismatch for $name from $url" >&2
      exit 1
    fi
    mv "$path.part" "$path"
  fi
  chmod +x "$path"
}
fetch linuxdeploy-x86_64.AppImage \
  https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20251107-1/linuxdeploy-x86_64.AppImage \
  c20cd71e3a4e3b80c3483cef793cda3f4e990aca14014d23c544ca3ce1270b4d
fetch linuxdeploy-plugin-qt-x86_64.AppImage \
  https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/1-alpha-20250213-1/linuxdeploy-plugin-qt-x86_64.AppImage \
  15106be885c1c48a021198e7e1e9a48ce9d02a86dd0a1848f00bdbf3c1c92724
fetch appimagetool-x86_64.AppImage \
  https://github.com/AppImage/appimagetool/releases/download/1.9.1/appimagetool-x86_64.AppImage \
  ed4ce84f0d9caff66f50bcca6ff6f35aae54ce8135408b3fa33abfc3cb384eb0
# appimagetool would otherwise download whatever runtime is current.
fetch runtime-x86_64 \
  https://github.com/AppImage/type2-runtime/releases/download/20251108/runtime-x86_64 \
  2fca8b443c92510f1483a883f60061ad09b46b978b2631c807cd873a47ec260d
tools_dir="$(cd "$tools_dir" && pwd)"

work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
appdir="$work/Qalam.AppDir"

# linuxdeploy names the icon after the desktop file's Icon= key.
icon_args=()
for size in 64 128 256 512; do
  mkdir -p "$work/icons/$size"
  cp "$root/qalam/resources/branding/icons/qalam-$size.png" "$work/icons/$size/qalam.png"
  icon_args+=(--icon-file "$work/icons/$size/qalam.png")
done

# The tools are AppImages themselves; CI runners and containers lack FUSE.
export APPIMAGE_EXTRACT_AND_RUN=1
export QMAKE="$qt_dir/bin/qmake"
export LD_LIBRARY_PATH="$qt_dir/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export PATH="$tools_dir:$PATH"
# Qalam finds the internal server as usr/bin/baa-lsp, next to itself.
"$tools_dir/linuxdeploy-x86_64.AppImage" \
  --appdir "$appdir" \
  --executable "$qalam" \
  --executable "$baa_lsp" \
  --desktop-file "$root/packaging/linux/qalam.desktop" \
  "${icon_args[@]}" \
  --plugin qt

install -Dm644 "$root/packaging/linux/qalam-mime.xml" \
  "$appdir/usr/share/mime/packages/qalam.xml"
licenses="$appdir/usr/share/licenses/qalam"
mkdir -p "$licenses"
window_kit_license="$build_dir/_deps/qwindowkit-src/LICENSE"
[[ -f "$window_kit_license" ]] || {
  echo "QWindowKit license missing from the selected build: $window_kit_license" >&2
  exit 1
}
cp "$window_kit_license" "$licenses/QWindowKit-LICENSE.txt"
printf '%s\n' \
  'QWindowKit 1.5.0, commit 35e88f3655720ed0537c7dd1dde243bf4a70c94c' \
  'https://github.com/stdware/qwindowkit' \
  'Statically linked into Qalam; upstream source is unmodified.' \
  > "$licenses/QWindowKit-NOTICE.txt"
printf '%s\n' \
  "Qt $("$QMAKE" -query QT_VERSION) is used under the GNU LGPL v3." \
  'Its libraries are dynamically linked from usr/lib and usr/plugins and' \
  'may be replaced; source is available from https://download.qt.io/.' \
  > "$licenses/Qt-NOTICE.txt"

mkdir -p "$output_dir"
output_dir="$(cd "$output_dir" && pwd)"
appimage="$output_dir/Qalam-$version-x86_64.AppImage"
rm -f "$appimage" "$appimage.sha256"
ARCH=x86_64 "$tools_dir/appimagetool-x86_64.AppImage" \
  --runtime-file "$tools_dir/runtime-x86_64" \
  --no-appstream \
  "$appdir" "$appimage"
(cd "$output_dir" && sha256sum "$(basename "$appimage")" > "$(basename "$appimage").sha256")
echo "Packaged $appimage"
