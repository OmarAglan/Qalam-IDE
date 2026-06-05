# Qalam IDE — RTL VS Code Layout + Feature Pass (2026-06-05)

## Goals

- Mirror the VS Code-like workbench for Arabic/RTL developers.
- Keep the Activity Bar and Primary Side Bar on the right.
- Add more VS Code-style workflow features without heavy architecture changes.

## Changes

- Activity Bar moved to the far right.
- Primary Side Bar moved to the right beside the Activity Bar.
- Editor and bottom panel now fill the remaining left side.
- Editor tabs now flow right-to-left.
- Activity indicator moved to the right edge of the Activity Bar.
- Sidebar border moved to the left edge, matching right-side placement.
- Status Bar now flows right-to-left: workspace/problem info starts on the right, editor context sits on the left.
- Added a VS Code-like Command Center button to the custom title bar.
- Added Command Palette (`Ctrl+Shift+P`).
- Added Quick Open (`Ctrl+P`) for project files.
- Added shortcuts: `Ctrl+B`, `Ctrl+J`, `Ctrl+Shift+F`, `Ctrl+Shift+M`, `F5`.
- Expanded top menu into File / Edit / View / Run / Terminal / Help.
- Connected Problems panel clicks to editor navigation.
- Added basic compiler-output diagnostics parsing into the Problems panel.

## Manual test checklist

- Launch app: Activity Bar must be on the far right.
- Explorer must open on the right beside Activity Bar.
- Editor tabs should start from the right.
- `Ctrl+Shift+P` opens Command Palette.
- Clicking the title Command Center opens Command Palette.
- `Ctrl+P` opens project files when a folder is open.
- `Ctrl+B` toggles right sidebar.
- `Ctrl+J` toggles bottom panel.
- `Ctrl+Shift+F` focuses Search view.
- `Ctrl+Shift+M` opens Problems.
- Baa compiler errors like `main.baa:12:4: error: message` appear in Problems.
