# Manual Visual Review: Standalone Packages

This is the human half of Qalam's packaging gate (ROADMAP 7.3.7). CI already
proves that each package installs, launches a native window, starts its
internal Baa-LSP, and leaves nothing behind. CI cannot judge what a person
sees: whether Arabic is shaped and ordered correctly, whether the layout mirrors
properly, and whether text stays crisp when scaled. This review covers that.

Review the **installed packages**, not a development build. A check passes only
when it looks right without a workaround. Fill each cell with ✅ (pass),
❌ (fail, with a note under [Findings](#findings)), or `n/a`.

## 1. Setup

### Packages under review

| | Windows | Linux |
|---|---|---|
| File | `qalam-setup-3.7.0-x64.exe` | `Qalam-3.7.0-x86_64.AppImage` |
| Source | CI artifact `qalam-setup-3.7.0-x64` | CI artifact `Qalam-AppImage-x86_64` |
| CI run | | |
| SHA-256 (matches `.sha256`) | | |

Verify the digest before installing:

```powershell
(Get-FileHash .\qalam-setup-3.7.0-x64.exe -Algorithm SHA256).Hash
Get-Content .\qalam-setup-3.7.0-x64.exe.sha256
```

```sh
sha256sum --check Qalam-3.7.0-x86_64.AppImage.sha256
chmod +x Qalam-3.7.0-x86_64.AppImage
./Qalam-3.7.0-x86_64.AppImage
```

If the AppImage reports a FUSE error, run it once with
`APPIMAGE_EXTRACT_AND_RUN=1 ./Qalam-3.7.0-x86_64.AppImage` and note it as a
finding. Every desktop distribution should run it directly.

### Review environment

| | Windows | Linux |
|---|---|---|
| OS and version | | |
| Desktop / session (X11 or Wayland) | — | |
| Monitor resolution | | |
| Display scale used in §7 | | |
| Baa, Takween, Nazm installed? | | |

### Sample project

Create a folder named `مراجعة بصرية` (Arabic, with a space) and put the following
in `رئيسي.باء` inside it. It mixes Arabic and Latin text, Arabic-Indic digits, a
string, a comment, a call with parameters, and a constant, so most of the
editor's rendering is exercised by a single file.

```baa
#تعريف رسالة "مرحباً بالعالم — Hello, world ١٢٣"

// دالة الجمع: تأخذ عددين وتعيد مجموعهما (sum of a and b)
صحيح جمع(صحيح أ, صحيح ب) {
    إرجع أ + ب.
}

صحيح الرئيسية() {
    ثابت صحيح الحد = ١٠٠.
    صحيح الناتج = جمع(٢٠, الحد).
    إذا (الناتج > ٥٠) {
        اطبع رسالة.
    }
    إرجع ٠.
}
```

## 2. First launch and window

| # | Check | Expected | Win | Linux |
|---|---|---|---|---|
| 2.1 | Launch from the Start menu / app launcher (Linux: from the AppImage) | Qalam opens within a few seconds with the Qalam icon in the taskbar/dock | ☐ | ☐ |
| 2.2 | Window title bar and frame | Title bar is drawn completely; minimize, maximize, and close work and sit on the correct side for an RTL application | ☐ | ☐ |
| 2.3 | Maximize, restore, resize to about 800×600 | Nothing overlaps, clips, or leaves unpainted areas; panels reflow | ☐ | ☐ |
| 2.4 | Welcome screen | Arabic text is joined (not isolated letters), right-aligned, and nothing is truncated | ☐ | ☐ |
| 2.5 | Close and relaunch | Window size and position are restored | ☐ | ☐ |

## 3. RTL layout

| # | Check | Expected | Win | Linux |
|---|---|---|---|---|
| 3.1 | Overall mirroring | Activity bar and explorer on the right; editor to their left; menus open right-to-left | ☐ | ☐ |
| 3.2 | Menu bar and menus | Menu order runs right to left; shortcut hints are on the left side of each item and readable (`Ctrl+S`, not `S+Ctrl`) | ☐ | ☐ |
| 3.3 | Tabs | Open three files: tabs run right to left; the active tab is clear; tab close buttons are on the correct side | ☐ | ☐ |
| 3.4 | File explorer | Folder `مراجعة بصرية` and its files are shown fully; expand arrows point the right way for RTL | ☐ | ☐ |
| 3.5 | Context menus (explorer, tab, editor) | Open toward the left of the cursor; Arabic labels are aligned and not clipped | ☐ | ☐ |
| 3.6 | Settings dialog (File → Settings), including **الأدوات** | Labels on the right, fields on the left; tool paths shown left-to-right and not mirrored | ☐ | ☐ |
| 3.7 | Scrollbars | Vertical scrollbar of the explorer and editor on the side the design intends, and consistent everywhere | ☐ | ☐ |
| 3.8 | Status bar | Items right-aligned; the Arabic reason for a disabled action is readable in full | ☐ | ☐ |

## 4. Arabic text in the editor

Open `رئيسي.باء`.

| # | Check | Expected | Win | Linux |
|---|---|---|---|---|
| 4.1 | Letter shaping | All Arabic words are joined with correct initial/medial/final forms; no boxes (tofu) | ☐ | ☐ |
| 4.2 | Lam-alef ligatures | `إرجع`, `الناتج`, `مرحباً`, `بالعالم` show the لا ligature where it occurs | ☐ | ☐ |
| 4.3 | Diacritics | The tanween in `مرحباً` sits above the correct letter and is not clipped | ☐ | ☐ |
| 4.4 | Mixed direction | In the string and the comment, `Hello, world` and `(sum of a and b)` read left-to-right inside the Arabic line; parentheses face the right way | ☐ | ☐ |
| 4.5 | Arabic-Indic digits | `١٠٠`, `٢٠`, `٥٠`, `١٢٣` render as Arabic-Indic digits, in the correct order | ☐ | ☐ |
| 4.6 | Line numbers and gutter | Gutter on the right; numbers aligned with their lines; folding markers on the correct side | ☐ | ☐ |
| 4.7 | Cursor movement | Right arrow moves backward through Arabic text, Left arrow forward; Home/End go to the visual line start/end; the caret never lands inside a ligature in a confusing place | ☐ | ☐ |
| 4.8 | Selection | Shift+arrows and mouse drag across mixed Arabic/Latin text highlight a single contiguous visual region per direction run, with no gaps | ☐ | ☐ |
| 4.9 | Typing | Type a new line `صحيح س = ٥.`: letters join as you type and the caret stays at the insertion point | ☐ | ☐ |
| 4.10 | Arabic comma and punctuation | Type `جمع(١، ٢)`: `،` renders correctly and the call signature popup appears | ☐ | ☐ |
| 4.11 | Syntax highlighting | Keywords, types, numbers, strings, comments, and `#تعريف` each have a distinct colour, and colour does not break letter joining mid-word | ☐ | ☐ |
| 4.12 | Zoom (`Ctrl++`, `Ctrl+-`, `Ctrl+0`, `Ctrl+Scroll`) | Text stays shaped and crisp at every size; line height grows with the font | ☐ | ☐ |

## 5. Language features (visual only)

These need the internal Baa-LSP only, not an installed compiler.

| # | Check | Expected | Win | Linux |
|---|---|---|---|---|
| 5.1 | Completion (`Ctrl+Space` after typing `ص`) | Popup opens next to the caret on the correct side; Arabic items are right-aligned; the documentation panel is readable | ☐ | ☐ |
| 5.2 | Hover over `جمع` | Tooltip shows the declaration and description in Arabic; not clipped at the screen edge | ☐ | ☐ |
| 5.3 | Diagnostics | Change `أ + ب` to `أ + ج`: a squiggle appears under `ج` (not offset by a character) and the Problems panel shows an Arabic message | ☐ | ☐ |
| 5.4 | Parameter hints | Muted `أ`/`ب` hints appear beside the arguments of `جمع(٢٠, الحد)` without overlapping the text | ☐ | ☐ |
| 5.5 | Outline (**المخطط**) | Shows `جمع` and `الرئيسية` with correct Arabic names | ☐ | ☐ |
| 5.6 | Format (`Shift+Alt+F`) | Result is indented with four spaces; RTL alignment stays correct afterwards | ☐ | ☐ |

## 6. Terminal and output panels

| # | Check | Expected | Win | Linux |
|---|---|---|---|---|
| 6.1 | Toggle the console (`F6`) | Panel opens at the bottom; the session bar shows the shell as ready | ☐ | ☐ |
| 6.2 | Shell prompt and output | Run `echo مرحبا` (Windows cmd) or `echo مرحبا` (bash): Arabic output is shaped and readable, not mojibake | ☐ | ☐ |
| 6.3 | Mixed output | Run `dir` / `ls -la` in `مراجعة بصرية`: Latin columns stay aligned; Arabic file names are readable | ☐ | ☐ |
| 6.4 | Clear, stop, restart controls; `Ctrl+L`; Up/Down history | Each control works and its Arabic label is fully visible | ☐ | ☐ |
| 6.5 | Output tab (**المخرجات**) | Baa-LSP status lines are Arabic plain text, readable, and right-aligned | ☐ | ☐ |
| 6.6 | Missing tools | With Baa/Nazm absent, **تشغيل** (F5) is disabled and its tooltip gives the exact Arabic reason | ☐ | ☐ |
| 6.7 | Run (only if Baa and Nazm are installed) | `F5` builds and runs the sample; `مرحباً بالعالم — Hello, world ١٢٣` appears after **مخرجات البرنامج** correctly shaped | ☐ | ☐ |

## 7. Themes

Switch via **File → Settings → Editor → Theme**, reopening `رئيسي.باء` each time.

| # | Theme | Expected | Win | Linux |
|---|---|---|---|---|
| 7.1 | GitHub Dark (default) | All token colours readable; selection, current line, and squiggles visible | ☐ | ☐ |
| 7.2 | GitHub Light | No dark-on-dark or light-on-light text in the editor, gutter, popups, or tooltips | ☐ | ☐ |
| 7.3 | Monokai | Same as above | ☐ | ☐ |
| 7.4 | Solarized | Same as above | ☐ | ☐ |
| 7.5 | Theme switch while a completion popup and hover are open | Popups adopt the new theme or close cleanly; no stale colours | ☐ | ☐ |

## 8. High DPI

| # | Check | Expected | Win | Linux |
|---|---|---|---|---|
| 8.1 | 100% scale | Baseline: everything above passes | ☐ | ☐ |
| 8.2 | 150% (Windows: Settings → Display → Scale; Linux: fractional scaling) | Text and icons crisp, not blurry or pixelated; no clipped labels in menus, tabs, or settings | ☐ | ☐ |
| 8.3 | 200% | Same as 8.2; the window still fits on screen | ☐ | ☐ |
| 8.4 | Move the window between monitors with different scales (if available) | Qalam re-renders at the new scale without restarting; nothing is blurry after the move | ☐ | ☐ |
| 8.5 | Linux on Wayland | Qalam runs through XWayland; note whether it is crisp at 100% and at fractional scales | — | ☐ |

## 9. Desktop integration

| # | Check | Expected | Win | Linux |
|---|---|---|---|---|
| 9.1 | Icon in launcher, taskbar/dock, and Alt+Tab | Qalam icon at every size; never a generic placeholder | ☐ | ☐ |
| 9.2 | Open a `.باء` file from the file manager (Windows: Open with → Qalam; Linux: after integrating the AppImage, e.g. with AppImageLauncher or Gear Lever) | Qalam opens that file; its Arabic name shows correctly in the tab and title | ☐ | ☐ |
| 9.3 | Paths with Arabic and spaces | Opening `مراجعة بصرية/رئيسي.باء` shows the full path correctly in the title, tab tooltip, and Settings | ☐ | ☐ |
| 9.4 | Uninstall (Windows) / delete the AppImage (Linux) | Windows: Qalam is removed from Apps and the Start menu. Linux: nothing remains besides `~/.config` settings | ☐ | ☐ |

## Findings

Record every ❌ here. A finding blocks sign-off unless it is explicitly
deferred to a ROADMAP item with a reason.

| # | Platform | What was seen | Screenshot | Severity (blocker / minor) | Action |
|---|---|---|---|---|---|
| | | | | | |

## Sign-off

| | Windows | Linux |
|---|---|---|
| Reviewer | | |
| Date | | |
| Verdict (pass / pass with deferred minors / fail) | | |

When both columns pass, mark ROADMAP 7.3.7 complete with this record's commit,
then re-pin Qalam with the Developer Kit in `ecosystem.lock.json` and clear its
"manual visual-review" pending gate.
