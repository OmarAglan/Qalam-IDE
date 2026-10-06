#!/usr/bin/env python3
"""Generate Qalam's UI icon set.

Every workbench icon is defined here once, on a 24x24 grid with one stroke
weight and the Qalam Sky palette, so the set stays visually consistent. Run
from the repository root after editing a shape or a color:

    python scripts/generate_ui_icons.py

The script rewrites qalam/resources/<name>.svg; register any new file in
qalam/resources.qrc. Application logos are not generated here.
"""

import math
from pathlib import Path

RESOURCES = Path(__file__).resolve().parent.parent / "qalam" / "resources"

# Palette: mirrors Constants::Colors in qalam/Constants.h.
MUTED = "#82a9c2"      # TextMuted: resting icons
ACTIVE = "#e6f4ff"     # TextPrimary: selected or hovered state
ACCENT = "#38bdf8"     # Accent: Baa identity marks
ERROR = "#f14c4c"      # ErrorForeground
WARNING = "#e2b714"    # WarningForeground, lifted for small sizes
INFO = "#3794ff"       # InfoForeground
SUCCESS = "#4ec9b0"    # SuccessForeground
STATUS = "#ffffff"     # StatusBarForeground

STROKE = 1.75


def gear_path(cx=12.0, cy=12.0, outer=9.2, inner=7.0, teeth=8):
    """Closed gear outline with flat-topped teeth."""
    points = []
    step = 2 * math.pi / teeth
    for tooth in range(teeth):
        base = tooth * step - math.pi / 2
        for angle, radius in ((base - step * 0.30, inner), (base - step * 0.16, outer),
                              (base + step * 0.16, outer), (base + step * 0.30, inner)):
            points.append((cx + radius * math.cos(angle), cy + radius * math.sin(angle)))
    commands = [f"{'M' if index == 0 else 'L'}{x:.2f} {y:.2f}"
                for index, (x, y) in enumerate(points)]
    return " ".join(commands) + "Z"


DOCUMENT = "M14 3H7a1.5 1.5 0 0 0-1.5 1.5v15A1.5 1.5 0 0 0 7 21h10a1.5 1.5 0 0 0 1.5-1.5V7.5z"
DOCUMENT_FOLD = "M14 3v4.5h4.5"
FOLDER = ("M3.5 7.5A1.5 1.5 0 0 1 5 6h4.2l2 2.2H19a1.5 1.5 0 0 1 1.5 1.5v8.3"
          "A1.5 1.5 0 0 1 19 19.5H5a1.5 1.5 0 0 1-1.5-1.5z")
MAGNIFIER = '<circle cx="10.5" cy="10.5" r="6.2"/><path d="M15.2 15.2 20.5 20.5"/>'
BRANCH = ('<circle cx="6.5" cy="5.5" r="2.2"/><circle cx="6.5" cy="18.5" r="2.2"/>'
          '<circle cx="17.5" cy="7.5" r="2.2"/>'
          '<path d="M6.5 7.7v8.6M17.5 9.7c0 4.3-5.6 3.6-9.5 7"/>')

# name -> (body, default color). Bodies may set their own stroke or fill for
# accent details; the outer <svg> supplies the default stroke.
ICONS = {
    # Activity bar and workbench navigation.
    "explorer": (f'<path d="{DOCUMENT}"/><path d="{DOCUMENT_FOLD}"/>'
                 '<path d="M9 12.5h6M9 16h4"/>', MUTED),
    "search": (MAGNIFIER, MUTED),
    "source-control": (BRANCH, MUTED),
    "run": ('<path d="M7.5 4.9v14.2a1 1 0 0 0 1.53.85l11.1-7.1a1 1 0 0 0 0-1.7L9.03 4.05'
            'A1 1 0 0 0 7.5 4.9z"/>', MUTED),
    "extensions": ('<rect x="3.5" y="12.5" width="8" height="8" rx="1.5"/>'
                   '<rect x="12.5" y="12.5" width="8" height="8" rx="1.5"/>'
                   '<rect x="3.5" y="3.5" width="8" height="8" rx="1.5"/>'
                   '<path d="M16.5 2.8 21.2 7.5 16.5 12.2 11.8 7.5z"/>', MUTED),
    "settings": (f'<path d="{gear_path()}"/><circle cx="12" cy="12" r="2.8"/>', MUTED),

    # Window caption.
    "minimize": ('<path d="M6 12h12"/>', MUTED),
    "maximize": ('<rect x="5.5" y="5.5" width="13" height="13" rx="1.8"/>', MUTED),
    "restore": ('<rect x="5" y="8.5" width="10.5" height="10.5" rx="1.6"/>'
                '<path d="M8.5 5.5h8.4A1.6 1.6 0 0 1 18.5 7.1v8.4"/>', MUTED),
    "close": ('<path d="M6.5 6.5 17.5 17.5M17.5 6.5 6.5 17.5"/>', MUTED),

    # Chevrons. "left" is the collapsed direction in a right-to-left tree.
    "up-arrow": ('<path d="M6.5 14.5 12 9l5.5 5.5"/>', MUTED),
    "down-arrow": ('<path d="M6.5 9.5 12 15l5.5-5.5"/>', MUTED),
    "left-arrow": ('<path d="M14.5 6.5 9 12l5.5 5.5"/>', MUTED),
    "right-arrow": ('<path d="M9.5 6.5 15 12l-5.5 5.5"/>', MUTED),

    # Search options.
    "match-case": ('<path d="M2.8 18.5 7 5.5l4.2 13M4.3 14h5.4"/>'
                   '<path d="M14 11.6c.8-1 1.9-1.5 3.3-1.5 2.1 0 3.4 1.1 3.4 3.2v5.2'
                   'M20.7 15h-3.5c-1.9 0-3.2.7-3.2 1.9s1.1 1.9 2.6 1.9c1.7 0 3.1-.9 4.1-2.3"/>',
                   MUTED),
    "match-word": ('<path d="M3 15.5V8M3 13a2.8 2.8 0 1 0 5.6 0 2.8 2.8 0 0 0-5.6 0'
                   'M16 13a2.8 2.8 0 1 0 5.6 0v-2.6M21.6 15.8V10.4M10.8 10.6h3.2M2.5 19.5h19"/>',
                   MUTED),
    "regex": ('<circle cx="5.5" cy="18" r="1.2" fill="currentColor" stroke="none"/>'
              '<path d="M15.5 4.5v13M9.9 7.75l11.2 6.5M21.1 7.75 9.9 14.25"/>', MUTED),
    "match-diacritics": ('<path d="M3.5 11.5c0 3.6 2.6 5.5 8.5 5.5s8.5-1.9 8.5-5.5"/>'
                         '<circle cx="12" cy="20.5" r="1.1" fill="currentColor" stroke="none"/>'
                         '<path d="M9 7.5 15 4.5" stroke-dasharray="2 2.2"/>', MUTED),

    # Files and folders.
    "file": (f'<path d="{DOCUMENT}"/><path d="{DOCUMENT_FOLD}"/>', MUTED),
    "file-new": (f'<path d="{DOCUMENT}"/><path d="{DOCUMENT_FOLD}"/>'
                 '<path d="M12 11v6M9 14h6"/>', MUTED),
    "file-baa": (f'<path d="{DOCUMENT}"/><path d="{DOCUMENT_FOLD}"/>'
                 f'<path d="M8.4 12.2c0 2.3 1.4 3.4 3.6 3.4s3.6-1.1 3.6-3.4" stroke="{ACCENT}"/>'
                 f'<circle cx="12" cy="18" r="1" fill="{ACCENT}" stroke="none"/>', MUTED),
    "file-baa-header": (f'<path d="{DOCUMENT}"/><path d="{DOCUMENT_FOLD}"/>'
                        f'<path d="M8.5 11.5h7M8.5 14.5h7M8.5 17.5h4" stroke="{ACCENT}"/>',
                        MUTED),
    "file-nazm": (f'<path d="{DOCUMENT}"/><path d="{DOCUMENT_FOLD}"/>'
                  f'<path d="M10 11.5 8 14l2 2.5M14 11.5l2 2.5-2 2.5" stroke="{ACCENT}"/>',
                  MUTED),
    "file-open": (f'<path d="{DOCUMENT}"/><path d="{DOCUMENT_FOLD}"/>'
                  '<path d="M12 17.5v-6M9.5 14 12 11.5l2.5 2.5"/>', MUTED),
    "folder": (f'<path d="{FOLDER}"/>', MUTED),
    "folder-new": (f'<path d="{FOLDER}"/><path d="M12 11v5.5M9.25 13.75h5.5"/>', MUTED),
    "folder-open": ('<path d="M3.5 18V7.5A1.5 1.5 0 0 1 5 6h4.2l2 2.2H18a1.5 1.5 0 0 1 1.5 1.5v1.3"/>'
                    '<path d="M3.5 18 6.2 12.2a1.5 1.5 0 0 1 1.36-.87H20.7a1 1 0 0 1 .92 1.4l-2.4 5.6'
                    'a1.5 1.5 0 0 1-1.38.9H5a1.5 1.5 0 0 1-1.5-1.25z"/>', MUTED),
    "git-clone": ('<path d="M12 3.5v11M7.5 10l4.5 4.5 4.5-4.5M4.5 15.5v3A1.5 1.5 0 0 0 6 20h12'
                  'a1.5 1.5 0 0 0 1.5-1.5v-3"/>', MUTED),
    "save": ('<path d="M5.5 4h10.2L19.5 7.8v10.7A1.5 1.5 0 0 1 18 20H6a1.5 1.5 0 0 1-1.5-1.5'
             'V5.5A1.5 1.5 0 0 1 6 4z"/><path d="M8 4v4.5h6.5V4M8 20v-5.5h8V20"/>', MUTED),

    # Editing and navigation.
    "replace": ('<path d="M4 7.5h10M11 4.5l3 3-3 3M20 16.5H10M13 13.5l-3 3 3 3"/>', MUTED),
    "replace-all": ('<path d="M4 6h10M11 3.5l3 2.5-3 2.5M4 10.5h7M20 18H10M13 15.5l-3 2.5 3 2.5'
                    'M20 13.5h-7"/>', MUTED),
    "command-palette": ('<rect x="3" y="4.5" width="18" height="15" rx="2"/>'
                        '<path d="M7 9.5 9.8 12 7 14.5M12.5 14.5h4.5"/>', MUTED),
    "search-files": (f'<path d="{DOCUMENT}"/><path d="{DOCUMENT_FOLD}"/>'
                     '<circle cx="11" cy="13.5" r="2.6"/><path d="M13 15.5l2 2"/>', MUTED),
    "go-to-line": ('<path d="M4 6h16M4 12h9M4 18h16"/><path d="M16.5 9.5 19 12l-2.5 2.5"/>', MUTED),
    "go-to-definition": ('<circle cx="12" cy="12" r="8.5"/><circle cx="12" cy="12" r="4"/>'
                         '<circle cx="12" cy="12" r="0.9" fill="currentColor" stroke="none"/>',
                         MUTED),
    "references": ('<circle cx="6" cy="12" r="2.3"/><circle cx="18" cy="5.5" r="2.3"/>'
                   '<circle cx="18" cy="18.5" r="2.3"/><path d="M8.1 11 15.9 6.6M8.1 13l7.8 4.4"/>',
                   MUTED),
    "comment": ('<path d="M10.5 4 6.5 20M17.5 4l-4 16"/>', MUTED),
    "duplicate": ('<rect x="8.5" y="8.5" width="12" height="12" rx="1.8"/>'
                  '<path d="M15.5 5.2V5A1.5 1.5 0 0 0 14 3.5H5A1.5 1.5 0 0 0 3.5 5v9'
                  'A1.5 1.5 0 0 0 5 15.5h.2"/>', MUTED),
    "format": ('<path d="M4 5.5h16M8 10h12M4 14.5h16M8 19h12"/>', MUTED),
    "quick-fix": ('<path d="M9.5 18.5h5M10.5 21.5h3M12 2.5a6.5 6.5 0 0 0-3.9 11.7c.6.5.9 1.1.9 1.9'
                  'v.4h6v-.4c0-.8.3-1.4.9-1.9A6.5 6.5 0 0 0 12 2.5z"/>', WARNING),
    "rename": ('<path d="M15.8 4.2a2.2 2.2 0 0 1 3.1 3.1L8.3 17.9 4 19.5l1.6-4.3z'
               'M14 6l3.1 3.1"/>', MUTED),
    "trash": ('<path d="M4 6.5h16M9.5 6.5V4.8A1.3 1.3 0 0 1 10.8 3.5h2.4a1.3 1.3 0 0 1 1.3 1.3v1.7'
              'M6 6.5l.9 12.6a1.5 1.5 0 0 0 1.5 1.4h7.2a1.5 1.5 0 0 0 1.5-1.4L18 6.5'
              'M10 10.5v6M14 10.5v6"/>', MUTED),
    "exit": ('<path d="M14 4h4.5A1.5 1.5 0 0 1 20 5.5v13a1.5 1.5 0 0 1-1.5 1.5H14'
             'M9.5 16.5 5 12l4.5-4.5M5 12h10"/>', MUTED),

    # Layout.
    "split-right": ('<rect x="3" y="4" width="18" height="16" rx="2"/><path d="M12 4v16"/>', MUTED),
    "split-down": ('<rect x="3" y="4" width="18" height="16" rx="2"/><path d="M3 12h18"/>', MUTED),
    "move-editor": ('<path d="M4 8.5h14M14.5 5l3.5 3.5-3.5 3.5M20 15.5H6M9.5 12 6 15.5 9.5 19"/>',
                    MUTED),
    "toggle-sidebar": ('<rect x="3" y="4" width="18" height="16" rx="2"/><path d="M15 4v16"/>',
                       MUTED),
    "toggle-panel": ('<rect x="3" y="4" width="18" height="16" rx="2"/><path d="M3 14.5h18"/>',
                     MUTED),
    "panel-right-open": ('<rect x="3" y="4" width="18" height="16" rx="2"/><path d="M15 4v16"/>',
                         MUTED),

    # Tooling.
    "build": ('<path d="M13.4 4.1 16 2.6l5.4 5.4-1.5 2.6-2.2-.4-1.4 1.4-3.9-3.9 1.4-1.4z"/>'
              '<path d="M12.4 9.6 3.6 18.4a1.5 1.5 0 0 0 2.1 2.1l8.8-8.8"/>', MUTED),
    "test": ('<path d="M9 3h6M10 3v6.2L4.9 18.2A1.8 1.8 0 0 0 6.5 21h11a1.8 1.8 0 0 0 1.6-2.8'
             'L14 9.2V3M7.3 15h9.4"/>', MUTED),
    "clean": ('<path d="M15.6 3.9 20.1 8.4a1.5 1.5 0 0 1 0 2.1l-8.3 8.3H7.6l-3.2-3.2'
              'a1.5 1.5 0 0 1 0-2.1l9.1-9.1a1.5 1.5 0 0 1 2.1-.5zM9 9.3l5.7 5.7M11.8 18.8H20"/>',
              MUTED),
    "stop": ('<rect x="6" y="6" width="12" height="12" rx="2.2"/>', MUTED),
    "restart": ('<path d="M19.5 12a7.5 7.5 0 1 1-2.2-5.3M19.5 4v4.5H15"/>', MUTED),
    "terminal": ('<rect x="3" y="4.5" width="18" height="15" rx="2"/>'
                 '<path d="M7 9.5 9.8 12 7 14.5M12.5 14.5h4.5"/>', MUTED),
    "debug": ('<path d="M8.5 7.5a3.5 3.5 0 0 1 7 0M7 10.5a5 5 0 0 1 10 0v3.5a5 5 0 0 1-10 0z'
              'M12 9.5v10M3.5 13H7M17 13h3.5M4.5 7.5l2.8 2M19.5 7.5l-2.8 2M4.5 19l2.8-2M19.5 19l-2.8-2"/>',
              MUTED),

    # Severity markers.
    "error": (f'<circle cx="12" cy="12" r="8.5" stroke="{ERROR}"/>'
              f'<path d="M9.2 9.2 14.8 14.8M14.8 9.2 9.2 14.8" stroke="{ERROR}"/>', ERROR),
    "warning": (f'<path d="M10.6 4.3a1.6 1.6 0 0 1 2.8 0l7.4 13.3a1.6 1.6 0 0 1-1.4 2.4H4.6'
                f'a1.6 1.6 0 0 1-1.4-2.4z" stroke="{WARNING}"/>'
                f'<path d="M12 9.5v4" stroke="{WARNING}"/>'
                f'<circle cx="12" cy="16.6" r="1" fill="{WARNING}" stroke="none"/>', WARNING),
    "info": (f'<circle cx="12" cy="12" r="8.5" stroke="{INFO}"/>'
             f'<path d="M12 11v5.5" stroke="{INFO}"/>'
             f'<circle cx="12" cy="7.9" r="1" fill="{INFO}" stroke="none"/>', INFO),
    "success": (f'<circle cx="12" cy="12" r="8.5" stroke="{SUCCESS}"/>'
                f'<path d="M8.3 12.3 10.8 14.8 15.8 9.4" stroke="{SUCCESS}"/>', SUCCESS),

    # Status bar glyphs sit on the saturated status background, so they use
    # its white foreground rather than severity colors.
    "status-error": ('<circle cx="12" cy="12" r="8.5"/><path d="M9.2 9.2 14.8 14.8M14.8 9.2 9.2 14.8"/>',
                     STATUS),
    "status-warning": ('<path d="M10.6 4.3a1.6 1.6 0 0 1 2.8 0l7.4 13.3a1.6 1.6 0 0 1-1.4 2.4H4.6'
                       'a1.6 1.6 0 0 1-1.4-2.4zM12 9.5v4"/>'
                       '<circle cx="12" cy="16.6" r="1" fill="currentColor" stroke="none"/>', STATUS),
    "status-branch": (BRANCH, STATUS),
}

# Icons the UI swaps to a brighter variant when selected or checked.
ACTIVE_VARIANTS = ["explorer", "search", "source-control", "run", "extensions", "settings",
                   "match-case", "match-word", "regex", "match-diacritics"]


def render(body: str, color: str) -> str:
    # Qt renders SVG Tiny, so currentColor is resolved here rather than by
    # the renderer: fill-only details follow the icon's stroke color.
    body = body.replace("currentColor", color)
    return (
        '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" '
        f'width="24" height="24" fill="none" stroke="{color}" '
        f'stroke-width="{STROKE}" stroke-linecap="round" stroke-linejoin="round">'
        f"{body}</svg>\n"
    )


def main() -> None:
    written = []
    for name, (body, color) in ICONS.items():
        (RESOURCES / f"{name}.svg").write_text(render(body, color), encoding="utf-8")
        written.append(name)
    for name in ACTIVE_VARIANTS:
        body, _ = ICONS[name]
        (RESOURCES / f"{name}-active.svg").write_text(render(body, ACTIVE), encoding="utf-8")
        written.append(f"{name}-active")
    print(f"Wrote {len(written)} icons to {RESOURCES}")


if __name__ == "__main__":
    main()
