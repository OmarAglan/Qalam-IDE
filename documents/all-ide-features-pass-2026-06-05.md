# Qalam IDE — Full IDE Features Pass

Date: 2026-06-05

## Added in this pass

- Problems diagnostics now also flow into editor decorations.
- Compiler errors/warnings are underlined in the editor.
- Hovering a diagnostic line shows the diagnostic message.
- Hovering Baa keywords, builtins, and preprocessor directives shows simple language help.
- Problems status bar count is refreshed after diagnostic parsing.
- Added a Debug panel tab with Run/Stop placeholder controls, call stack placeholder, and variables placeholder.
- Added Command Palette entries for Debug, Go to Definition, and Find References.
- Added top-menu entries and shortcuts:
  - `Ctrl+Shift+D` Debug panel
  - `F12` Go to Definition
  - `Shift+F12` Find References
- Added simple Go to Definition based on Baa definition patterns.
- Added Find References using the project search pipeline.
- Fixed malformed Problems-panel cleanup from earlier edits.

## Notes

The Debug panel is intentionally protocol-shaped but not connected to a real debugger yet. It is ready for a future Baa debug adapter or direct debugger integration.

## Smoke tests

1. Build the app on Windows.
2. Open a `.baa` file and intentionally trigger a compiler error.
3. Confirm the error appears in Problems.
4. Confirm the matching line/word is underlined in the editor.
5. Hover the underlined area and confirm the tooltip appears.
6. Press `Ctrl+Shift+M` and confirm Problems opens.
7. Press `Ctrl+Shift+D` and confirm Debug panel opens.
8. Put the cursor on a function/symbol and press `F12`.
9. Put the cursor on a symbol and press `Shift+F12`.
10. Hover over Baa keywords such as `إذا`, `صحيح`, `اطبع`, or `#تضمين`.
