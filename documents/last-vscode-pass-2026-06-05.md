# Qalam IDE — Last VS Code-style UI/Feature Pass

Date: 2026-06-05

## Goal

Make the startup/workbench feel closer to VS Code while keeping Qalam Arabic-first and focused on Baa development.

## Added

- VS Code-like Command Center in the custom title bar.
- Command Palette opened by the Command Center or `Ctrl+Shift+P`.
- Quick Open for project files with `Ctrl+P`.
- Search in Files shortcut and menu entry with `Ctrl+Shift+F`.
- Sidebar toggle with `Ctrl+B`.
- Bottom terminal/panel toggle with `Ctrl+J`.
- `F5` run shortcut in the menu action.
- Git branch detection from `.git/HEAD` and display in the status bar.
- Breadcrumb visibility for saved/opened files.
- Welcome page walkthrough cards for Command Palette, Quick Open, and running Baa.
- More VS Code-like Activity Bar hover and active indicator styling.

## Updated

- Menu layout now includes File, Edit, View, Run, Terminal, and Help.
- Title bar keeps Windows controls in the expected LTR position while Arabic UI remains RTL inside views.
- Welcome page is wider and more useful for first-time users.

## Test checklist

1. Open Qalam and click the title-bar Command Center.
2. Press `Ctrl+Shift+P` and run each major command.
3. Open a folder and press `Ctrl+P`; confirm files are searchable and open correctly.
4. Press `Ctrl+Shift+F`; confirm the Search sidebar opens and focuses.
5. Press `Ctrl+B`; confirm the sidebar toggles.
6. Press `Ctrl+J`; confirm the bottom panel toggles.
7. Open a Git repository folder; confirm the branch appears in the status bar.
8. Open a saved file; confirm breadcrumb appears above the editor.
9. Press `F5` on a saved `.baa` file; confirm it runs in the terminal panel.
