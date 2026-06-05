# Qalam IDE — VS Code-like Layout Pass

Date: 2026-06-05

## Goal

Make the first-launch workbench feel closer to Visual Studio Code while keeping Qalam Arabic-first.

## Changes

- Move the Activity Bar to the far left.
- Show the Primary Side Bar by default on startup.
- Put the Primary Side Bar between the Activity Bar and editor.
- Make the Side Bar resizable with a horizontal splitter.
- Keep the editor/tab chrome left-to-right like VS Code, while keeping Arabic text controls RTL where needed.
- Align the Status Bar with VS Code: workspace/status indicators on the left, editor context indicators on the right.
- Darken the title/activity/sidebar surfaces to reduce the heavy gray frame.
- Make tabs flatter and put the tab close button on the right.
- Move the Welcome page higher, reduce empty vertical space, and make it cleaner and more compact.
- Force the custom title bar and menu strip to stay left-to-right so Windows caption buttons stay in the expected place.

## Manual smoke test

1. Start Qalam with no file and no folder.
2. Confirm the Activity Bar appears on the left.
3. Confirm the Explorer/Primary Side Bar is visible by default.
4. Click Explorer in the Activity Bar; it should hide/show the Side Bar.
5. Click Search; the Side Bar should stay left and change view.
6. Open a file and confirm tabs start from the left with close buttons on the right.
7. Confirm the Status Bar shows workspace items left and file/language info right.
8. Resize the Side Bar with the splitter between the Side Bar and editor.
