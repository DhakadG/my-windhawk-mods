# Handover — Windhawk mods

Starting context for a fresh session. Everything here is verified, not assumed. Read §1 and
§2 before touching anything; the rest is per-task.

**Repo:** [DhakadG/my-windhawk-mods](https://github.com/DhakadG/my-windhawk-mods) (public)
**Working folder:** `C:\Users\lost_husky\Downloads\Programs\VS Code Works\WindHawk Mods`

## 0. How a change reaches the gallery — do not skip a step

1. Edit locally, then `scripts/check-mod.ps1`, `scripts/build-mod.ps1`,
   `scripts/check-settings.py`.
2. Push to **this repo** and wait for its **Build mods** workflow to go green on all
   three Windhawk versions.
3. **Only then** copy the file into the gallery fork (`DhakadG/windhawk-mods`) on a
   **fresh branch cut from an up-to-date `main`**, and open a **new** PR.

> **Changed after #5001 merged (2026-08-22).** There is no standing PR branch any
> more. `add-win-x-hotcorners` was deleted post-merge; its tip is kept as the tag
> `pr-5001-merged` because the PR was **squash**-merged, so those 23 commits are in
> no upstream history. Before each new submission:
> `gh api repos/DhakadG/windhawk-mods/merge-upstream -f branch=main`, then branch
> from `main`. Do not resurrect the old branch — it is 31 commits behind and would
> reopen settled review.

Never push a mod change straight to a live PR branch, however small. The gallery's check
only runs on a PR, only for the changed file, and a fork run can sit waiting for a
maintainer to approve it — the `-luuid` link error reached them exactly that way.

---

## 1. Environment facts

| What | Value |
| --- | --- |
| OS | Windows 11 Insider, **26H2 build 26300.9032**, Experimental channel |
| Windhawk | **v2.0-alpha.3** |
| `taskbar.dll` | 10.0.26100.9032 (in `System32`, not SystemApps) |
| `SystemTray.dll` | 2607.2000.0.0 |
| `Taskbar.View.dll` | 2607.10000.0.6000 |

- Windhawk compiles from `C:\ProgramData\Windhawk\ModsSource\` — **admin-only for writes**.
  Edit in this repo, then paste into the Windhawk editor and compile there.
- Rolling Windows back is off the table. Don't suggest it.
- Closing the Windhawk UI does **not** unload mods; the `Windhawk` service keeps the engine
  injected. A real control test means disabling the mod.

## 2. Verify locally — never ask the user to compile just to find errors

Windhawk ships **clang 20.1.3 + mingw-w64** at `C:\Program Files\Windhawk\Compiler`, driven
by its own `compile_flags.txt`. That is the same setup clangd uses for the squiggles in the
Windhawk editor, so it reproduces those diagnostics offline.

```bash
pwsh -File scripts/check-mod.ps1 -Warnings && python scripts/check-settings.py   && pwsh -File scripts/build-mod.ps1 && python scripts/check-gallery.py
```

- `check-mod.ps1` — real type check (`-fsyntax-only -Wall`). Not a link; it will not produce
  a loadable DLL. Every error worth catching before a paste is a front-end error.
- `check-settings.py` — parses the `==WindhawkModSettings==` block with PyYAML **and checks
  Windhawk's schema rules on top**. **clang cannot catch this class of bug**: a malformed
  settings block compiles and loads fine, then fails later as `Failed to extract previous
  initial settings for engine`. Its byte/line/column is relative to the YAML block, not the
  `.cpp`, which is why the numbers never match the editor.
  - Valid YAML is **necessary but not sufficient**. `$options` works only on a string, or on
    a list of strings for a multi-select — an integer default is rejected at load with
    `must be a string or array of strings to use $options`, and the path it reports
    (`instance[0].displays[0][1].zones[0][8].modifier`) is not a line number. The checker now
    reproduces that path. Use `INHERIT`/`NONE`/`CTRL`-style string enums and map them in code.
  - When adding a rule here, run it against the **other** mods first. The first version of the
    `$options` rule flagged `taskbar-fluent-media-player-fork`, which works — the rule was
    wrong, not the mod.
- `check-gallery.py` — downloads the **gallery's own** `pr_validation.py` and runs it over
  the mod. This exists because 1.1.8 went red on PR #5001 for something no local check
  looked at: the three "Mod compatibility check" jobs only *compile* the mod, while
  "PR mod validation" is a separate job enforcing metadata/readme/settings rules. Run it
  before every promotion.
  - It cannot check the PR **description** rules (the required `## Mod authorship`
    section), because those read the `pull_request` event payload. That one is only
    verifiable on the PR itself — so when editing the PR body, keep the template sections.
  - Set UTF-8 output or a warning containing a zero-width character dies in cp1252 and
    looks like a crash. The gallery's workflow sets `PYTHONUTF8=1` for the same reason.
- **`gh pr checks` without `--json` lists every check.** With `--json name,state` it
  returned a single row here, which is how a failing required check was reported as green
  for a whole round. Read the plain output.
- All five mods currently pass with **zero warnings**.
- `scripts/legacy/cpp_sanity_check.ps1` is the older heuristic checker. Its brace/paren counts are
  wrong on files containing `LR"(...)"` raw strings (the mangled symbol literals) — compare
  the delta against the unmodified baseline rather than trusting the absolute number. Its
  `@architecture` rule only applies to `windhawk.exe` tool mods.

**Always bump `@version` when you change a file**, even for a one-line fix. A version that
means two different files has already caused one wasted debugging round here.

### Build a runnable probe, don't reason about the API

The same clang **links**, not just syntax-checks, so a suspect code path can be lifted into a
standalone exe and run against the real machine. This is what found the bt-battery-monitor
bug after two rounds of plausible-but-wrong reasoning:

```bash
"/c/Program Files/Windhawk/Compiler/bin/clang++.exe" --target=i686-w64-mingw32 -std=c++20 -static -Wl,--subsystem,console probe.cpp -o probe.exe -lcfgmgr32
```

- `--target=i686-w64-mingw32` matches `windhawk.exe`'s bitness; drop it for a 64-bit host.
- `-static` matters — without it the exe dies with `0xC0000135` (missing libc++ DLLs) unless
  `Compiler\bin` is on `PATH`.
- In PowerShell, quote `'-Wl,--subsystem,console'` or the commas are parsed as an argument list.
- Use `%ls` in `wprintf`, not `%s` — mingw's printf reads `%s` as `char*` and prints one byte.
- `devpkey.h` constants need `INITGUID` to link; it is easier to declare the `DEVPROPKEY`
  literal locally, which is what the mods do anyway.

## 3. The 26300 tray change (root cause behind three separate "broken" mods)

Build 26300 rebuilt the tray for the movable-taskbar work. **`SystemTrayFrameGrid` kept its
name but changed type from `Grid` to `StackPanel`.** Any mod resolving its tray target with
`try_as<Grid>()` gets null and silently never injects — while still reporting a successful
load, which is what makes it look like a symbol problem.

Verified tree on 26300.9032:

```
[SystemTrayFrame]
  [StackPanel] name='SystemTrayFrameGrid'
    0 NotifyIconStack   1 NotificationAreaIcons   2 MainStack   3 NonActivatableStack
    4 SecondaryClockStack   5 ControlCenterButton   6 NotificationCenterButton   7 ShowDesktopStack
```

Children still carry stale `Grid.Column` values — **do not read them**, they no longer drive
layout. Order is the index in `Children()`.

Fixed here in `taskbar-ai-quota-fork` (0.12.0) and `taskbar-fluent-media-player-fork` (1.6.1).
The pattern: hold a `Panel`, pick strategy from the real type, insert by child index on the
StackPanel path, keep the Grid path for older builds. **Re-resolve the insert index after
removing the old element** — it is computed from a tree that still contains it, so on
re-injection everything after it has shifted down by one.

Upstream is aware: issues [#5049](https://github.com/ramensoftware/windhawk-mods/issues/5049)
(quota, @Cleroth), [#5050](https://github.com/ramensoftware/windhawk-mods/issues/5050)
(media player, @Salyts), plus [#5018](https://github.com/ramensoftware/windhawk-mods/issues/5018)
(Taskbar Styler, @m417z, filed by someone else). Each carries the full fix description; PRs
offered but not yet opened. **Check for replies before doing more work on the forks.**

## 4. Mods

| Mod | Version | Original author | State |
| --- | --- | --- | --- |
| win-x-hotcorners | 1.3.0 | — (lost_husky's own) | **Published.** Merged via #5001, live at windhawk.net/mods/win-x-hotcorners |
| taskbar-ai-quota-fork | 0.12.0 | Cleroth | Working |
| taskbar-clock-customization-v3 | 3.1.71 | m417z | Working |
| taskbar-fluent-media-player-fork | 1.6.1 | Salyts | Working |
| taskbar-system-info-fork | 1.0.0 | Yevhenii Starychenko | **New — see §9. Verified by script only, not yet installed.** |
| mac-magnifying-cursor | 1.5.0 | Jaali | Working; suggestion filed as [#5051](https://github.com/ramensoftware/windhawk-mods/issues/5051) |
| bt-battery-monitor-fork | 1.0.1 | BlackPaw | **Skeleton only — see §6** |

`@author` is `lost_husky` on all of them, with the original author credited in a blockquote
at the top of each fork's readme. Windhawk has one `@author` field, so origin lives in the
readme; that is deliberate, don't "tidy" it away.

**No AI attribution anywhere** — not in commits, PR/issue bodies, code comments, or readmes.
This is a hard rule, not a preference.

---

## 5. DONE — win-x-hotcorners settings + dashboard

**4.2.1** — the Windhawk settings page (`displays[]` → `zones[]`, plus all globals),
`ReadSettingsZones()` / `ApplySettingsGlobals()`, and the precedence rule in `ReloadConfig`:
settings win; else a 4.1.x layout still runs untouched; else globals only. Confirmed working
on the real machine — the legacy layout kept firing with its own globals (corner 8 / edge 2).

**4.3.0** — the dashboard is a read-only picture. Tab per display with a configured-zone
count and a dot when absent; screen box at the display's real aspect ratio; action name drawn
in each zone (vertical text on the side strips via `lfEscapement`/`lfOrientation` 900); a
detail panel showing all six timings with each marked *global* or *this zone*; wildcard-
inherited zones say so. Editing is gone, and so are `DashSave`, `ClearStoredConfig` and the
whole slot/store-writing layer — the file got **275 lines shorter** despite gaining a
settings page.

Key design point: **the dashboard reads `g_settings`, not the value store.** That is what
makes it correct for both configuration sources without knowing which is live, and it is why
it can no longer drift from what actually fires. `DashFillZones` mirrors `ResolveZone`
exactly — own configuration first, then wildcard, decided *per zone*.

Geometry is verified rather than eyeballed: `scripts/probes/zonegeom.cpp` copies
`ZoneRectInDiagram` verbatim and asserts the 16 pieces tile the border ring with no overlap,
no gap and nothing outside the box, at six aspect ratios including portrait. Re-run it if you
touch the proportions.

Reference: [docs/design/hotcorners-dashboard-draft.html](design/hotcorners-dashboard-draft.html).

**Not done, deliberately:** the tray's "skip while fullscreen" / "skip while dragging"
toggles still write `ovr_*`. They change the live configuration immediately, but in
settings mode the next reload restores the settings value. They are quick toggles, so
reverting is arguably right — but if it ever annoys, that is the reason.

---

## 5b. Original brief (kept for the reasoning)

The mod retired its settings page in 4.1.0 for a dashboard, because a flat list of ~40
settings was unusable in the old Windhawk UI. **Windhawk v2 removes that constraint**:
settings render as collapsible groups with per-group reset and a settings count on the
collapsed header. `taskbar-fluent-media-player` already uses this shape and is the reference.

The decision reached: **do both, with a clear split.**

- **Windhawk settings page** becomes the place you *configure*. Group the settings the way
  the media player does (Zones, Actions, Timing, Per-monitor, Appearance, Debug). Keep the
  existing setting keys unchanged so nobody's configuration resets.
- **Dashboard** becomes the place you *see*. Not a second config surface — a visual map.

### Dashboard UI draft — DONE, awaiting sign-off

**[docs/design/hotcorners-dashboard-draft.html](design/hotcorners-dashboard-draft.html)** (also published at
<https://claude.ai/code/artifact/5f3b97ba-c5ed-4db6-9b4f-18256f336b78>). Interactive: tabs
switch, zones respond to hover, and it follows the viewer's theme so both of the mod's
palettes are visible.

Two corrections to what this section used to say:

- There are **12 zones, not 8** — 4 corners, 4 edges, 4 edge-centres (`ZONE_COUNT = 12`).
- The dashboard **already draws the diagram**. `ZoneRectInDiagram()` lays out all twelve
  today, and the mod already has a light/dark `Palette` and requests Mica. This is a
  redesign of a working surface, not a new build — and it is plain **GDI, not XAML**.

Proposed changes, all of them buildable with what the mod already owns:

| Element | Today | Proposed |
| --- | --- | --- |
| Display picker | combo box (`IDC_MONITOR`) | tab strip, one tab per display, with a configured-zone count badge |
| Disconnected display | listed identically | dimmed dot, still selectable |
| Screen box | fixed proportions | derived from `MonitorInfo::rcMonitor` |
| Zone labels | an action combo per zone | the action name drawn in the zone |
| Editing | in the dashboard | in Windhawk settings |

The two open questions are **answered in the draft** — overrule them if you disagree:

1. **Alternate actions** → show the primary action plus a `⇄` badge, both spelled out in the
   detail row. `Alt+S | Alt+H` cannot fit in a corner block, and truncating it misleads.
2. **Arrangement view vs one screen at a time** → one screen at a time. An arrangement view
   shrinks every screen to fit the widest span; at three displays the corner blocks are too
   small to label, which defeats the diagram's only job.

**Trap:** the centre zones sit *inside* the edge strips. The current code splits each edge
into two segments around its centre block (the `secondHalf` parameter) precisely because they
used to overlap — hovering a centre highlighted the whole edge. Preserve that split.

Verdict on the dashboard: **keep it, read-only.** Drop the zone action combos, numeric fields,
Save and Reset. Two surfaces writing the same store is the exact shape that produced the 4.1.3
and 4.1.4 bugs. Deep-linking a zone into the settings page can come later.

---

## 6. DONE — bt-battery-monitor-fork 1.1.0

Both asks are implemented and verified. **An earlier version of this section stated the
opposite diagnosis and was wrong** — it is kept below only as a warning about how it went
wrong, because the same trap is easy to fall into again.

### Root cause: one wrong GUID

The mod declared

```cpp
DEVPKEY_Bluetooth_DeviceAddress = {E57A6B4A-21B8-4B8A-B4B4-73B9F358ED60}, 1
```

That key **is not published by any devnode on this machine.** The real one is
`{2BD67D8B-8BEB-48D5-87E0-6CDA3428040A}, 1`. In `GetBatteryFromMediaClass` the address read
came *before* the battery could be assigned, so it hit `continue` on every node and no
classic Bluetooth device ever received a battery value. BLE peripherals (keyboard, mouse)
get theirs from `EnumerateBthleDevices` / the registry fallback and were unaffected — which
is exactly why it looked like a hardware limitation of audio devices.

The battery itself lives on the **Hands-Free AG service node**,
`BTHENUM\{0000111e-...}_..._C00000000`, not on the `DEV_` container node — verified reading
`{104EA319-...},2` = 20 (earbuds) and 100 (speaker).

Also fixed: `DEVPKEY_Bluetooth_IsConnected` was pointed at that same address key, so its
`DEVPROP_TYPE_BOOLEAN` check could never pass and the branch was dead. Removed; the
child-interface fallback that was already doing the work now runs directly.

### Why the first diagnosis was wrong

It enumerated only the `BTHENUM\DEV_*` container nodes and searched by *property name*. The
battery key has no friendly name, so it renders as a raw `{GUID} PID` string and a name match
silently misses it. It also read `BTHLEDEVICE\{0000180F-...}` as belonging to the earbuds —
that node is the **EvoFox keyboard's**. Neither audio device exposes GATT `0x180F`.

Lesson: query the exact key, and confirm against a device that already works.

### Also in 1.1.0

- **Duplicate merge.** A device's BLE endpoint has a different MAC from its classic one, so
  address matching listed the earbuds twice — once with a battery, once with `--`.
  `DEVPKEY_Device_ContainerId` is what Windows groups them by, and now the mod does too.
  Verified: the Nord Buds' BLE endpoint merges; `LE-LotsOfHusky` has a genuinely different
  container ID and correctly stays separate.
- **Earbuds + Speaker icons**, from the Class of Device minor field (`minor >> 2`): `0x01`
  Wearable Headset → earbuds, `0x05` Loudspeaker → speaker, everything else → headphones.
  Nord Buds report `0x240404`, LotsOfHusky `0x240414`. `ddores.dll` index **6** is earbuds,
  **93** is a loudspeaker (picked by rendering a contact sheet of all 151 icons, not guessed).
- Dashboard rows are now driven by `kRowCount = ARRAYSIZE(kStorageKeys)` instead of a
  hardcoded `6` in a dozen places. **Careful:** there is an unrelated `for (i = 0; i < 6; i++)`
  that parses the 6 MAC bytes — do not fold that into `kRowCount`.

Note the mod is `@include windhawk.exe`, i.e. a **tool mod**. Per an earlier finding here,
declaring `@architecture x86-64` silently stops a `windhawk.exe` tool mod from ever loading —
leave the architecture line alone.

### Still to do

Install and confirm in the real tray. The pipeline was verified out-of-process by compiling
the patched logic standalone (32-bit, matching `windhawk.exe`) — it printed
`OnePlus Nord Buds 4 Pro 20% [Earbuds]` and `LotsOfHusky 100% [Speaker]`.

---

## 6b. Hold to peek (1.1.0) and the capture rig

**Hold zones.** A zone with `releaseAction` set to anything but Nothing runs its action
on arrival and the release action when the pointer leaves — the old Show Desktop button.
`ACTION_SAME` ("the same action again") covers every toggle.

The trap worth remembering: `g_holdEngaged` is deliberately *not* `g_firedThisEntry`.
Releasing keeps the visit spent, so re-enabling the mod while the pointer rests in a hold
zone cannot re-fire it, and leaving afterwards cannot fire a second release. One flag did
both and got both wrong. Disable, suspend and rebuild all call `ReleaseHeldZone` first —
a peeked desktop that cannot be restored is worse than a missed trigger.

**A required modifier vs. any key-based action (fixed in 1.1.1).** `SendInput`
cannot mask a key the user is physically holding — Windows merges it into the
injected combination. So a Ctrl-guarded zone running *Snap left* sent
Ctrl+Win+Left (switch virtual desktop, or nothing at all on a single desktop),
and it read as "the zone never fires" when the zone was firing perfectly.

`SendKeys` now releases held modifiers that are not already in the batch, and
**does not re-press them**. Do not "finish" this by restoring the key: a
re-press is only valid if the key is still down when the restore runs, and the
version that did this left Ctrl and Win logically stuck for whole sessions. A
stray key-up has no equivalent failure mode. Both halves of that reasoning are
in the comment above `SendKeys`.

Verified by slicing the real `SendKeys` out of the `.cpp` into
`scripts/probes/probe-sendkeys.cpp` and driving a live window with it — the pattern
from §2, and the reason this was not shipped on reasoning alone.

**Show Desktop.** Uses `Win+D` via `SendKeys`. Do not "improve" this to
`IShellDispatch4::ToggleDesktop`: on Windows 11 26300 it returns **S_OK and does
nothing**, so there is no error to fall back on. `WM_COMMAND 407` to `Shell_TrayWnd` is
the other dead end (undocumented, and a window the mod does not own).

**Capture rig** lives in the session scratchpad (`capture/`): `ctl.ps1` (DPI-aware mouse,
staging), `rec.ps1` (gdigrab, lossless RGB), `runall.ps1`, `mkgif.ps1`. Lessons:

- Make the process **per-monitor DPI aware** or `SetCursorPos` lands in the 3072×1728
  logical space while gdigrab records 3840×2160.
- Shell animations take **~1.3 s** after the trigger. Sampling one frame at t+2.9 s showed
  "nothing fired" for actions that fired perfectly — always trace several frames before
  concluding the mod is broken. The user was right that it was the harness, not the mod.
- Stage the windows before **every** clip, or one clip inherits what the last left behind.
- Dull colour is not the palette (128-colour bayer vs 256 undithered: 18.1% → 17.9%
  saturation). The desktop is genuinely near-greyscale — 1.9% measured. `mkgif.ps1`
  applies `eq=saturation=1.25:contrast=1.05`, which measured 17.9% → 26.1%.
- **Use `ddagrab`, not `gdigrab`, at 4K.** gdigrab sustained ~17 fps on this panel,
  which reads as stepped pointer motion in a GIF; ddagrab holds ~57 fps. It also
  captures one *output* rather than a region of the virtual desktop, so a second
  monitor cannot leak in by getting `-offset_x`/`-video_size` wrong. Needs
  `hwdownload,format=bgra` or ffmpeg cannot convert its D3D11 frames.
- **Snap Assist previews every switchable window**, not just staged ones — a folder of
  personal documents reached a recording that way. `runall.ps1` now refuses to start
  when a File Explorer window is open, matched on window **class**
  (`CabinetWClass`/`ExploreWClass`); a title allowlist is useless because browsers and
  players retitle themselves constantly. Also press Esc right after a snap to dismiss
  the assist, and only *after* releasing Ctrl — Ctrl+Esc opens Start.
- **"Switch to last window" follows the system MRU**, which contains every window on the
  machine. The knock clip alt-tabbed to a window on the *other* monitor, so it recorded
  as "nothing happened". `Set-Stage` now focuses both staged windows in order to pin
  MRU[0] and MRU[1]. Stage them overlapping too (`Set-Stage -Overlap`) or a focus change
  is invisible.
- **PowerShell timing:** a loop of `Start-Sleep -Milliseconds 8` actually steps at ~21 ms,
  so scripted moves ran **2.7x** their requested duration (200 ms → 547 ms). That silently
  broke the knock demo, whose whole point is re-entering inside a 400 ms window. Drive
  animation from a `Stopwatch` instead of counting sleeps.
- **Dot-sourced scripts share the caller's scope**, and PowerShell names are
  case-insensitive: a `foreach ($name in ...)` inside `logger.ps1` overwrote `rec.ps1`'s
  `-Name` parameter and sent all seven clips to one filename. Keep loop variables in
  dot-sourced files long and specific.
- **`Set-StrictMode` plus `$global:` defaults do not mix.** Reading a never-assigned
  global is a terminating error, so `if ($global:X)` fallbacks blow up on load. Declare
  them `$null` first.
- GIF trims are **per clip** (`$TRIMS` in `mkgif.ps1`). One global window either opened
  mid-staging or ran past the end, because the clips are 6.0-8.5 s and the moment that
  matters sits at a different offset in each.
- `gdigrab` does capture Task View, Start and Quick Settings, so it needs no replacing.
- Synthetic right-clicks on the tray icon never open the menu (likely UIPI against the
  elevated process); `Shell_NotifyIconGetRect` with the mod's GUID locates the icon fine.
  The dashboard can be opened by posting `WM_COMMAND` `IDM_SETTINGS` (100) to the
  `WindhawkHotCornersTray` window. `scripts/capture/dashshot.ps1` then polls for it by
  class (`WindhawkHotCornersDash`) and falls back to the window title. **An earlier
  version of this note claimed `FindWindowW` cannot match that class — that was never
  verified.** Both lookups return 0 when the window simply is not open, which is easy to
  misread as "the class does not match", and the reopen bug below makes that the common
  case. Confirm against an actually-open dashboard before concluding anything.

## 7. Smaller open items

- **Media is published and verified** — all six `trigger-*.gif`, `hot-corners.gif` and
  `dashboard.png` return 200 from `raw.githubusercontent.com` on `main`. Any *new* readme
  image must be pushed here before the readme reaches PR #5001, since the links resolve
  against this repo's `main`. That ordering is not covered by the promotion rule in §0.
- **1.3.0 is published.** PR #5001 merged 2026-08-22 (squash, upstream `ed03744`); the
  mod is live at <https://windhawk.net/mods/win-x-hotcorners>. The standalone repo
  `DhakadG/win-x-hotcorners` is synced to 1.3.0 and tagged `v1.3.0`; it had been stale
  at 1.2.1 with an install section still telling people to await the merge. Reach and
  SEO plan, with ready-to-post drafts, is in
  [`docs/per-mod/LAUNCH-win-x-hotcorners.md`](per-mod/LAUNCH-win-x-hotcorners.md).
- **1.1.8 was the first promotion.** On `main` with CI green and on PR #5001, gallery check passing.
  The PR title carries the version, so it needs updating on every promotion, and the PR
  **body** was rewritten at 1.1.7 - it still described the 4.1.x tray-only design ("there
  is no settings page") and counted twelve zones instead of sixteen.
- **Review round of 1.1.3-1.1.8, for context on why that code looks the way it does.**
  The gallery reviewer, CodeRabbit and three local agents between them found: a held zone
  was never released on unload; a solo Win key-up opened the Start menu; `g_holdEngaged`
  armed even when the queue dropped the entry; `SameZoneConfig` ignored the release fields;
  `ProcessNameOfWindow` truncated at `MAX_PATH` so long-path processes never matched the
  exclusion list; `g_hDashWnd`/`g_hDetectWnd`/`g_hTrayWnd` were non-atomic across threads;
  `TrayThread` did not pin its DPI context; "the same action again" used a second executor
  so Alternate zones drifted across displays; and a dashboard opened at the instant of
  unload could strand its thread. All fixed. An over-engineering audit of the whole
  non-dashboard file found nothing worth cutting.
- **The Start-menu behaviour behind the Win-key mask was never reproduced locally.** The
  detector could not see the Start window at all - `EnumWindows` does not return it on
  26300 - and a control case proved the detector broken rather than the finding absent. The
  fix is the documented AutoHotkey masking trick, taken on reasoning. If it ever needs
  revisiting, test visually; window enumeration is a dead end here.
- **Donation links** — Windhawk's `@donateUrl` renders a Donate button but takes only one
  URL; Ko-fi is set on `win-x-hotcorners` and the fork. The rest are plain links in the
  readme's `## Support the mod` section. Note the gallery only serves images from
  `i.imgur.com` and `raw.githubusercontent.com`.
- **Clock mod noise** — the File Explorer process (no tray) logs two `HookSymbols failed`
  lines per launch. Harmless since 3.1.71 releases the latch instead of locking out the retry,
  but it could be gated on the process actually owning a taskbar window.
- **`archive/`** — four superseded working copies, tracked so nothing was lost during the
  reorganisation. `win-x-hotcorners-v4.1.4-duplicate.cpp` is byte-identical to the tracked
  source and can go now.
- **`DhakadG/win-x-hotcorners`** — the old single-mod repo. History lives here now; it is
  still on GitHub as remote `old-win-x-hotcorners`. Archive it or keep it as the standalone
  home. **Do not delete `DhakadG/windhawk-mods`** — that is the gallery fork backing open PR
  [#5001](https://github.com/ramensoftware/windhawk-mods/pull/5001).

## 8. Working preferences

- Concise, direct answers. Explain *why*, not just *what* — the user reads and builds this code.
- Verify before asserting. Check the binary, the registry, the log, the actual tree. Several
  confident-sounding hypotheses in this project turned out wrong and were only caught by
  looking. The tray fix came from a XAML dump, not from reasoning about symbol names.
- Correct the record when a premise turns out false, including your own from earlier in the
  session.
- Don't file issues, open PRs, or change repo visibility without an explicit go-ahead.

---

## 9. NEW — taskbar-system-info-fork 1.0.0

Combines three things the user already had working in separate mods into one. Built by
reading all three sources in full first, not by reasoning about their APIs from memory.

**Base engine, from `taskbar-system-info` (GPL-3.0, Yevhenii Starychenko, pulled from
`ModsSource` at 1.3.3, 3328 lines):** PDH-based CPU/GPU/RAM/VRAM collection, D3DKMT GPU
adapter enumeration/identity, and the HWiNFO shared-memory (`Global\HWiNFO_SENS_SM2`) and
Gadget Registry (`HKCU\Software\HWiNFO64\VSB`) temperature readers, including the
sensor-name scoring heuristics. Kept close to verbatim — that engine is proven.

**What was replaced, and why.** The base mod's placement was the actual problem statement:
`InjectWidget` appended a full-`ColumnSpan` `Grid` to the taskbar's `RootGrid` at
`Canvas.ZIndex 10000` with `IsHitTestVisible(false)`, then faked "reserved space" by
shrinking `TaskbarFrameRepeater`'s left margin by the widget's own width — a click-through
overlay, not a tray citizen. Replaced with the `InjectionTarget`/`ResolveInjectionTarget`
pattern from this author's own `taskbar-ai-quota-fork` (0.12.0, the cleanest of the three
placement implementations in this repo): a real `StackPanel.Children().InsertAt()` on the
26300+ tray, a real `Grid.ColumnDefinitions().InsertAt()` on the pre-26300 tray, or a
tracked taskbar-button margin for the eight anchored positions, with three overlay
positions kept as an explicit opt-out. Also ported: the permanent watchdog thread
(`TrayUI::StartTaskbar` hook + poll loop) from the same fork, since the base mod only
re-injects on `TaskbarFrame::Loaded` and has no recovery if Explorer rebuilds the tray
without that firing.

**What was added.** Network throughput (two more PDH counters,
`\Network Interface(*)\Bytes Received|Sent/sec`, following
`taskbar-clock-customization-v3`'s dynamic KB/s-MB/s formatter) and an internet-status dot
(ICMP ping via `IcmpSendEcho`, ported near-verbatim from that same mod's `NetStatusThread`).
CPU/GPU clock and power: HWiNFO shared memory extended to read power (type 4) and clock
(type 5) readings in the *same pass* as temperature (type 1) rather than a second scan,
with native fallbacks where one actually exists — `NtPowerInformation`'s per-core
`CurrentMhz` for CPU clock, and `D3DKMT_ADAPTER_PERFDATA.Power` (already being queried for
GPU temperature, previously read and discarded) for GPU power. GPU clock and CPU power have
no native Windows equivalent and stay HWiNFO-only, shown as `--` otherwise — deliberate, not
an oversight.

**Three bugs the compiler and a manual review pass caught, worth remembering:**
- A ternary passed as a `Wh_Log` format argument — the exact trap `HANDOVER.md §2` already
  documents (`Wh_Log` is a permissive stub under `-DWH_EDITING` but concatenates onto a
  literal in a real build). `check-mod.ps1` reproduced it immediately.
- Switching the `position` setting between tray and taskbar-anchored positions left the old
  widget stranded: removal only swept the *newly resolved* panel, not wherever the widget
  currently lived. Fixed by sweeping `g_injectionParent` first, then the new target
  defensively.
- Tray sub-positions (`tray_left`, `tray_before_clock`, ...) all resolve to the same
  `SystemTrayFrameGrid` object, so the ai-quota-fork's own `panel == injectionParent`
  liveness check can't tell "still in the right spot" from "position changed to another
  spot in the same panel" — it would silently skip re-injection. Fixed by tracking the
  position string alongside the panel and requiring both to match.

**Verified:** `check-mod.ps1 -Warnings` clean, `check-settings.py` clean (10 groups),
`build-mod.ps1` links on x86-64, `check-gallery.py` clean.

**Not verified — next session or the user should do this before trusting it:** never pasted
into the Windhawk editor or run against a live taskbar. Compiling clean proves the C++ is
sound; it says nothing about whether the two-column layout looks right at the computed
default width, whether HWiNFO's shared-memory struct offsets still match a current HWiNFO
build, or whether every one of the 25 placement positions (14 tray + 8 anchored + 3
overlay) actually lands where its label says. Install it, and eyeball every position in
the dropdown once.

**Update, same session:** a second review (this time by an external CodeRabbit CLI pass,
see below) caught a real HIGH-severity bug the first pass missed: `AttachAnchorTracking`'s
`LayoutUpdated` subscription and pushed-aside anchor margin were only ever torn down in
`RemoveWidget()` (final unload), never in `InjectWidget()` (every position change and every
watchdog-triggered re-injection). Fixed by factoring the cleanup into
`DetachAnchorTracking()` and calling it from both places. Also fixed: `ReadHwInfoSharedMemory`/
`ReadHwInfoGadgetRegistry` now stamp which of the two actually supplied each clock/power/temp
value (`HwInfoExtras`'s new `*Provider` fields) instead of `ReadTemperatures`/`ReadExtraSensors`
hardcoding `HwInfoSharedMemory` regardless of source; a misleading comment in
`RunFromWindowThread` that claimed the hook stays installed for a late reply when the code
actually unhooks immediately (rewritten to state the small bounded leak honestly); and the
`fontSize` clamp widened to 9-20 while its own setting description still said "9 to 13"
(tightened the clamp back to match).

**Update, same session — CodeRabbit CLI (see `[[coderabbit-cli-setup]]`) found two more,
both real:**
- **Critical.** `RemoveWidget()` only nulled the C++-side `winrt::Grid` references
  (`g_widget`, `g_injectionParent`, ...) and never actually removed the widget from its
  parent panel's `Children()` collection. Since the panel holds its own reference, the
  widget stayed visible in the live taskbar after unload, with event handlers still
  pointing into the about-to-be-unmapped DLL. Fixed: `RemoveWidget()` now calls
  `RemoveWidgetFromPanel` (plus the same reserved-column cleanup `InjectWidget` already
  did) before dropping its own references.
- **Major.** `StopWatchdogThread()` waited only 2000ms for `WatchdogThreadProc` to exit
  before closing its thread handle and wake event. Since `RunFromWindowThread`'s own
  `SendMessageTimeoutW` call can legitimately take up to 2000ms, a bad race let the wait
  give up while the thread was still mid-iteration - closing the wake event out from
  under a thread that might still call `WaitForSingleObject` on it, and risking the DLL
  being unmapped while that thread was still executing. Changed to `INFINITE`; the inner
  call is already bounded, so this doesn't introduce a real deadlock risk.
- **Also removed, not fixed:** the GPU-power native fallback via
  `D3DKMT_ADAPTER_PERFDATA::Power`. CodeRabbit flagged that this field's actual unit has
  no authoritative source - the original `taskbar-system-info` mod already had this
  field available and never used it, which in hindsight was a signal worth taking more
  seriously the first time. Rather than guess a conversion, GPU power is now HWiNFO-only
  like CPU power and GPU clock already were; the mod's own readme/comments are updated to
  match. The `WindowsGpuPerfData::temperature` field this same query also returns is kept
  - that one *is* well-established across multiple public tools.

Version bumped to 1.0.2. Re-verified clean on all four scripts after every fix above.
