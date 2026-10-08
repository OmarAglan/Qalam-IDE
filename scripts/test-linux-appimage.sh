#!/usr/bin/env bash
# Verify a Qalam AppImage the way the Windows installer test verifies the
# installer: digest, contents, no bundled external tools, an LSP handshake with
# the internal server, a native window, and no process left behind.
#
# Run it on a machine without Qt. CI runs it in a fresh ubuntu:24.04 container
# with --install-desktop-baseline, which installs only libraries a desktop
# session already has, so anything else the AppImage needs must be inside it.
set -euo pipefail

usage() {
  echo "usage: scripts/test-linux-appimage.sh [--install-desktop-baseline] Qalam-<version>-x86_64.AppImage" >&2
  exit 2
}

install_baseline=0
appimage=""
for argument in "$@"; do
  case "$argument" in
    --install-desktop-baseline) install_baseline=1 ;;
    -*) usage ;;
    *) [[ -z "$appimage" ]] || usage; appimage="$argument" ;;
  esac
done
[[ -n "$appimage" && -f "$appimage" ]] || usage
appimage="$(cd "$(dirname "$appimage")" && pwd)/$(basename "$appimage")"

fail() { echo "FAIL: $*" >&2; exit 1; }

if [[ "$install_baseline" == 1 ]]; then
  # Deliberately absent: Qt, libxcb-cursor0, and the other xcb-util libraries
  # Qt's xcb plugin needs; the AppImage must carry those itself.
  export DEBIAN_FRONTEND=noninteractive
  apt-get update -qq
  apt-get install -y -qq --no-install-recommends \
    xvfb xdotool procps ca-certificates \
    libgl1 libegl1 libopengl0 libfontconfig1 libfreetype6 \
    libx11-6 libx11-xcb1 libxcb1 libxkbcommon0 libxkbcommon-x11-0 \
    libdbus-1-3 libglib2.0-0t64 >/dev/null
fi

echo "== digest"
(cd "$(dirname "$appimage")" && sha256sum --check "$(basename "$appimage").sha256")

work="$(mktemp -d)"
xvfb_pid=""
qalam_pid=""
cleanup() {
  [[ -n "$qalam_pid" ]] && kill "$qalam_pid" 2>/dev/null || true
  [[ -n "$xvfb_pid" ]] && kill "$xvfb_pid" 2>/dev/null || true
  rm -rf "$work"
}
trap cleanup EXIT

echo "== contents"
extract_root="$work/مسار عربي مع مسافات"
mkdir -p "$extract_root"
(cd "$extract_root" && "$appimage" --appimage-extract >/dev/null)
bundle="$extract_root/squashfs-root"
for required in \
  usr/bin/Qalam usr/bin/baa-lsp \
  usr/lib/libQt6Core.so.6 usr/lib/libQt6Widgets.so.6 \
  usr/plugins/platforms/libqxcb.so \
  usr/share/mime/packages/qalam.xml \
  usr/share/licenses/qalam/QWindowKit-LICENSE.txt \
  qalam.desktop qalam.png; do
  [[ -e "$bundle/$required" ]] || fail "AppImage is missing $required"
done
[[ -x "$bundle/usr/bin/baa-lsp" ]] || fail "internal Baa-LSP is not executable"
external="$(find "$bundle" \( -name baa -o -name 'نظم' -o -name nazm \
  -o -name takween -o -name 'تكوين' -o -name ld -o -name gcc \) -print)"
[[ -z "$external" ]] || fail "AppImage bundles an external tool: $external"

echo "== dynamic dependencies"
while IFS= read -r -d '' binary; do
  missing="$(env -u LD_LIBRARY_PATH ldd "$binary" 2>/dev/null | grep 'not found' || true)"
  [[ -z "$missing" ]] || fail "$binary has unresolved libraries:"$'\n'"$missing"
done < <(find "$bundle/usr/bin" "$bundle/usr/lib" "$bundle/usr/plugins" \
  -type f \( -perm -u+x -o -name '*.so*' \) -print0)
outside_qt="$(env -u LD_LIBRARY_PATH ldd "$bundle/usr/bin/Qalam" |
  grep 'libQt6' | grep -vF "$bundle/usr/lib/" || true)"
[[ -z "$outside_qt" ]] || fail "Qalam resolves Qt outside the AppImage:"$'\n'"$outside_qt"

echo "== internal Baa-LSP handshake"
"$bundle/usr/bin/baa-lsp" --version
request='{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"processId":null,"rootUri":null,"capabilities":{}}}'
response="$(printf 'Content-Length: %d\r\n\r\n%s' "${#request}" "$request" |
  timeout 15 "$bundle/usr/bin/baa-lsp" 2>/dev/null || true)"
grep -q '"capabilities"' <<<"$response" ||
  fail "internal Baa-LSP did not answer initialize: $response"

echo "== native window and internal server from the AppImage"
export HOME="$work/home"
export XDG_RUNTIME_DIR="$work/runtime"
mkdir -p "$HOME" "$XDG_RUNTIME_DIR"
chmod 700 "$XDG_RUNTIME_DIR"
project="$work/مشروع تجريبي"
mkdir -p "$project"
printf 'صحيح الرئيسية() {\n    إرجع ٠.\n}\n' > "$project/رئيسي.باء"

Xvfb :97 -screen 0 1600x1000x24 -nolisten tcp >/dev/null 2>&1 &
xvfb_pid=$!
export DISPLAY=:97
for _ in $(seq 50); do xdotool getdisplaygeometry >/dev/null 2>&1 && break; sleep 0.1; done

# A container has no FUSE; the runtime extracts itself instead of mounting.
APPIMAGE_EXTRACT_AND_RUN=1 QT_QPA_PLATFORM=xcb \
  "$appimage" "$project/رئيسي.باء" >"$work/qalam.log" 2>&1 &
qalam_pid=$!

window=""
server=""
for _ in $(seq 300); do
  kill -0 "$qalam_pid" 2>/dev/null || { cat "$work/qalam.log" >&2; fail "Qalam exited during startup"; }
  if [[ -z "$window" ]]; then
    for candidate in $(xdotool search --onlyvisible --name '.' 2>/dev/null || true); do
      eval "$(xdotool getwindowgeometry --shell "$candidate")"
      if (( WIDTH >= 640 && HEIGHT >= 480 )); then window="$candidate"; break; fi
    done
  fi
  [[ -n "$server" ]] || server="$(pgrep -f '/usr/bin/baa-lsp' || true)"
  [[ -n "$window" && -n "$server" ]] && break
  sleep 0.1
done
[[ -n "$window" ]] || { cat "$work/qalam.log" >&2; fail "Qalam created no usable native window"; }
[[ -n "$server" ]] || { cat "$work/qalam.log" >&2; fail "Qalam did not start its internal Baa-LSP"; }
echo "window $window: $(xdotool getwindowname "$window")"

# Stop Qalam itself, as a session logout would; the AppImage runtime waits for
# it and then removes its extraction.
pkill -TERM -f '/usr/bin/Qalam' || true
wait "$qalam_pid" 2>/dev/null || true
qalam_pid=""
for _ in $(seq 100); do
  pgrep -f '/usr/bin/baa-lsp' >/dev/null || break
  sleep 0.1
done
if pgrep -f '/usr/bin/baa-lsp' >/dev/null; then
  fail "internal Baa-LSP outlived Qalam"
fi

echo "Qalam AppImage verification passed."
