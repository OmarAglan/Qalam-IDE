# RTL Layout Correction — 2026-06-05

This pass fixes the broken RTL workbench geometry where the Activity Bar appeared on the far left while the Primary Side Bar stayed on the right.

## Changes

- Keeps the outer workbench shell in explicit left-to-right geometry order so Qt does not auto-mirror the splitter order.
- Places the workbench as: editor/panel area, Primary Side Bar, Activity Bar.
- Pins the Activity Bar to the far right by putting it inside the same horizontal splitter as the sidebar.
- Keeps Arabic-facing widgets RTL internally.
- Tightens the Welcome page width, spacing, title size, and quick-guide cards so it does not look scattered.

## Expected startup layout

From left to right:

1. Editor/welcome area
2. Explorer / Primary Side Bar
3. Activity Bar on the far right

This keeps the UI RTL-first without separating the Activity Bar from the sidebar.
