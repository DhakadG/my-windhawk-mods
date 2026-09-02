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
Pushed to `origin main` as `cc98f5c`, CI (`Build mods`, all three Windhawk versions plus
the settings-block validator) green.

**Update, same session — a second CodeRabbit CLI pass (verifying the 1.0.2 fixes) found
three more, all real, fixed as 1.0.3:**
- **Minor.** `ReadNetworkThroughput` set `networkAvailable = true` unconditionally
  whenever `showNetwork` was on, so a genuine PDH read failure looked identical to zero
  traffic — the widget showed `↓0 B/s` instead of `↓--`. `ReadNetworkCounterTotal` now
  returns `std::optional<double>`, and `networkAvailable` reflects whether either read
  actually succeeded.
- **Minor.** The `showClockPower` setting's own `$description` still claimed the GPU-power
  D3DKMT fallback that 1.0.2 removed. Corrected.
- **Major (defense in depth, not currently reachable).** `ReadPdhArray` could return a
  non-`PDH_MORE_DATA` failure status without resetting the out-parameter `itemCount` to 0.
  Every current caller already checks the returned status before touching `itemCount`, so
  this wasn't actually exploitable today — but it made that a rule every future caller
  would have to remember rather than something the function itself guarantees. Now zeroes
  `itemCount` on every non-success path.

Third CodeRabbit pass (against 1.0.3, run via the WSL binary directly rather than the
`coderabbit.ps1` wrapper — the free plan allows 3 reviews/hour, the wrapper's own 2/hour
is a deliberately conservative local throttle, not a hard limit) came back with
**0 findings**. Pushed as 1.0.3.

**Update, same session — installed for real, via `windhawk-cli.exe` (see §10 below) —
and a large follow-up round, 1.1.0:**

- **The tray positions (14 of 25) never actually worked.** `ResolveInjectionTarget`
  looked up `SystemTrayFrameGrid` with `FindDirectChildByName(root, ...)`, a
  *direct*-children-only search, against `root = xamlRoot.Content()` - several levels
  above where the tray actually sits. Confirmed via live DebugView: "InjectWidget
  failed: position target not in the visual tree yet", repeating, on the default
  `tray_left` position. Fixed with a new recursive `FindDescendantByName` helper,
  applied at all three call sites (including `FindTrayElement`'s pre-26300 fallback,
  same bug, not yet exercised on this build but real). `taskbar_*`/overlay positions
  were unaffected - they resolve via `FindTaskbarRootGrid`, which was already recursive.
- **Layout: 3 rows → 2.** The user wanted the network/internet-status row folded back
  in rather than added as a third row pair. Redesigned as a third *column* instead -
  down/up stacked over the same two-row height as CPU/GPU and RAM/VRAM, with the
  internet-status dot as a small color-only corner badge. A same-session review then
  caught that the dot's hover tooltip (added in the redesign) could never fire:
  `IsHitTestVisible(false)` on an ancestor (`widget`, the top-level Grid) excludes the
  whole subtree from hit-testing in WinRT XAML regardless of what a descendant sets on
  itself - confirmed against documented `UIElement.IsHitTestVisible` behavior, not just
  suspected. Removed the tooltip attempt rather than restructure the widget's
  click-through model for it; the dot's color alone (green/red/gray) still satisfies
  "subtly says whether you're online."
- **HWiNFO showed nothing despite HWiNFO running.** Most likely cause: HWiNFO's own
  **Shared Memory Support** setting is off by default and is a separate step from just
  running HWiNFO - easy to miss. Added `LogHwInfoSmDiagnosis`, which reports (once per
  distinct reason, gated on verbose logging) exactly which step failed: mapping not
  found, mutex busy, view failed, header invalid, or "read N readings but none scored
  as CPU/GPU temp/clock/power" - turning a silent `--` into an actionable log line.
  Also added a "Getting temperature, clock and power data" readme section covering
  both HWiNFO's Shared Memory Support checkbox and LHM's Remote Web Server toggle,
  since both are the same class of "the tool is running but the mod still can't see it"
  trap.
- **LibreHardwareMonitor added as a second, independent provider** (not a replacement -
  either covers the same six values: CPU/GPU temperature, clock, power). Ported from
  this author's own `taskbar-clock-customization-v3` (`FetchAndParseLhmData`/
  `FindLhmSensorValue`, read there in full by a research agent before porting): WinINet
  HTTP fetch of `http://localhost:<port>/data.json`, a hand-rolled recursive JSON
  parser (LHM's tree has `Text`/`Value`/`Children` per node, everything else skipped),
  and the same group-name/sensor-name-substring tree search, on its own background
  thread with its own poll interval. One correctness fix over the original: that
  version uses `0.0` as a "sensor not found" sentinel, indistinguishable from a
  genuine zero reading; this one returns `std::optional<double>` instead. Only
  temperature/clock/power are fetched - CPU/GPU load and RAM/VRAM already have a
  better source (native PDH counters) and pulling them from LHM too would just be a
  second, potentially-disagreeing copy of data already in hand. In Automatic mode the
  order is HWiNFO → LibreHardwareMonitor → native Windows fallback; explicit
  "LibreHardwareMonitor only" options exist for both Temperature source and Clock/power
  source.
- **A card-style background box**, matching `taskbar-fluent-media-player-fork`'s
  appearance instead of floating text directly on the taskbar. Required wrapping the
  content `Grid` in a `Border` (`Grid` has no `Padding` in this XAML version - confirmed
  the hard way once already this session, see the `ApplyWidgetGeometry` bug above from
  the original build). `g_widget` (the content Grid, column/children logic unchanged)
  is now `g_widgetBorder`'s (the newly positioned/injected/sized element) `Child`;
  `AttachAnchorTracking` and `IsWidgetLive` were updated to track the Border, not the
  inner Grid, since the Grid's parent is now always the Border, never the target panel.
- **Per-scope graph toggles.** `showGraphs` (one setting for both) split into
  `showCpuGraph`/`showGpuGraph`.
- Declined as unnecessary rather than skipped: explicit text-width padding for
  fluctuation-free layout (`clock-customization-v3`'s `PercentageFormat:
  spacePaddingAndSymbol` equivalent). This mod's Grid uses fixed-pixel columns
  throughout, not a free-form template string, so the specific failure mode that
  setting exists to fix (inserting "95" where "5" was shifts every following character)
  doesn't apply here by construction - right-aligned text inside a fixed-width column
  doesn't move regardless of digit count. Revisit only if live testing shows otherwise.

Version 1.1.0. Re-verified clean on `check-mod.ps1`/`check-settings.py`/`build-mod.ps1`
after every change above. **Not yet re-verified live** - reinstalled via CLI once
after the injection fix (confirmed working via DebugView) and once after the 2-row
layout change, but not since the LHM/box/per-scope-graph round.

---

## 10. `windhawk-cli.exe` — a real CLI existed the whole time

Not mentioned anywhere in this repo before this session, including §0/§2 above (which
still describe the correct *type-check* loop — that part hasn't changed). Full details
saved as the durable memory `[[windhawk-cli-usage]]`; the essentials:

- `C:\Program Files\Windhawk\windhawk-cli.exe`, subcommands `mod`/`app`/`source`/`repo`/
  `update`/`data`. Talks live to whatever Windhawk instance is actually running.
- **Reads need no elevation** (`mod list`, `mod show`, `mod settings get`, `source
  meta`); **writes do** (`mod install`, `enable`, `disable`, `settings set`, `remove`) -
  confirmed via `RegCreateKeyEx ... Access is denied` from a non-admin shell. Claude
  Code cannot self-elevate past UAC's secure desktop, so a write always means asking
  the user to run it themselves in an elevated shell.
- The whole install/compile/register cycle for a local mod is one command:
  `mod install --file <path> <id>` - no separate compile step, and re-running it is
  how you push an edit. `mod compile <id>` is a trap for that purpose: it recompiles
  from Windhawk's **stored** copy, not the file on disk.
- Mods installed this way appear in `mod list` as `local@<id>` - the same prefix every
  other fork in this repo already had, meaning they were always installed by exactly
  this mechanism (via the GUI editor's paste-and-compile, which uses the same
  underlying path), just never through the CLI directly until now.
- `DbgViewMini.exe` (bundled at `...\Windhawk\UI\resources\app\extensions\windhawk\
  files\`) is the fast companion for watching a mod's `Wh_Log` output live across a
  `mod install --file` reinstall, without the editor open.

---

## 11. Live feedback round two, 1.2.0 — layout matched to the user's own reference mods

With 1.1.0 actually rendering, the user compared it directly against their own
`taskbar-clock-customization-v3` config and `taskbar-fluent-media-player-fork`'s look,
and asked for several things to be *ported from those*, not invented fresh:

- **LibreHardwareMonitor made the primary source**, not just a fallback. HWiNFO's free
  tier needs Shared Memory Support re-enabled after every 12 hours (confirmed by the
  user's own diagnostic log line from 1.1.0's new logging - it worked exactly as
  designed, immediately identifying HWiNFO's Shared Memory Support as off). Reordering
  this correctly needed more care than "just call the LHM fill first": an explicit,
  non-Automatic source selection (e.g. "HWiNFO Shared Memory only") must stay
  exclusive - LHM should never preempt an explicit choice, only ever fill a genuine
  gap in that case. `FillFromLhm` split into `FillTempFromLhm`/`FillExtrasFromLhm` so
  temperature and clock/power (separate source-mode settings) can be reordered
  independently, and the HWiNFO/native write-backs in `ReadTemperatures`/
  `ReadExtraSensors` were made gap-aware (`&& !snapshot.field`) so they stop
  unconditionally overwriting whatever LHM already supplied.
- **Network column moved from the right to the left**, upload above download - matching
  `clock-customization-v3`'s own `TopLine`/`BottomLine` convention (upload first).
- **The background box was there in code but nearly invisible**: default was a 40%-alpha
  near-black fill, which reads as almost nothing against an already-near-black
  taskbar. A research agent read `taskbar-fluent-media-player-fork`'s actual background
  code end to end and found its own default is a **90%+-opacity** solid fill (`"35 35
  35"` at `solidOpacity: 100` when a background is enabled at all - its own out-of-box
  default is `backgroundType: "none"`, no box, so the reference the user was comparing
  against was itself a configured, not default, state). Matched that opacity here.
  Also found: that mod's "Mica"/"Mica Alt" options are a **faked** solid-color brush,
  not real system Mica material - worth knowing before ever trying to add real
  Acrylic/Mica to this mod, since even the reference implementation didn't trust it
  enough to use it as anything but an opt-in.
- **Thin frosted vertical dividers** added between the network/CPU-GPU/RAM-VRAM column
  groups - a `LinearGradientBrush`-filled 1px `Rectangle` per gap, transparent at both
  ends so it reads as a soft seam rather than a hard line touching the row edges.
- **Width-stability padding ported from `clock-customization-v3`**: that mod's
  `PadNumberWithFigureSpace` (U+2007 FIGURE SPACE prefix padding, confirmed by the same
  research agent to be pure string formatting with zero GDI dependency, so it was
  fully portable to this mod's XAML `TextBlock`s) and its "paired padding" idiom
  (`FormatCapacity`'s used-value now pads to the *total* value's own digit count, e.g.
  `_3.08/24.00` instead of `3.08/24.00` drifting a digit narrower than its pair).
  Applied to `FormatPercent`/`FormatTemperature`/`FormatCapacity`/`FormatClock`/
  `FormatPower`. One real snag while implementing it: a ` ` character embedded
  directly (not as an escape) in the source made the file's real bytes differ from
  what several Read/Edit round-trips *displayed* as a plain space - wasted several
  failed exact-match `Edit` attempts before checking the raw bytes with `od`
  confirmed the file was already correct and no fix was needed. Worth remembering:
  if an `Edit` with freshly-`Read` content inexplicably reports "string not found",
  check for a non-ASCII look-alike character before assuming the file changed
  underneath you.
- **Memory bar thickness increased** (1.25px → 2.5px, radius scaled to match) after the
  user reported not seeing a VRAM bar that RAM's own bar showed. No code-level
  asymmetry between the RAM and VRAM code paths was found on a careful re-read (both
  go through the identical `CreateMemoryRow`/`UpdateMemoryBar`) - most likely a pure
  visibility issue (a 1.25px bar is easy to miss, especially compressed in a
  screenshot) rather than a real bug, but **not confirmed live** - if it's still
  missing after 1.2.0, that becomes a real bug to chase rather than a hunch to fix.

Version 1.2.0. Re-verified clean on all four local scripts.

---

## 12. 1.3.0 — the VRAM bar was a clipping bug, not a drawing bug

The user reported the VRAM capacity bar still missing while RAM's showed fine, plus
asked for tighter spacing, a hover-reveal box, and the width-stability technique
"properly implemented". The bar turned out to explain several of those at once.

**Root cause: the widget was taller than the tray gives a child.** Rows were
18 + gap 2 + 18, plus 4px vertical Border padding = **46px**; the tray's usable child
height inside a 48px taskbar is roughly **40px**, and it clips from the bottom. The
memory bars sit at `VerticalAlignment::Bottom` of their row, so RAM's bar (mid-widget,
y≈18) survived and VRAM's (on the widget's bottom edge) was cut off entirely. There was
never any RAM/VRAM code asymmetry to find - 1.2.0's guess that it was "probably just
thin and hard to see" was wrong, and thickening the bar to 2.5px did not and could not
have fixed it.

Fixed by shrinking to fit rather than by moving the bar: `kRowHeight` 18 → 16,
vertical padding capped at 2px regardless of `boxPadding` (horizontal keeps the full
value), and a 1px bottom margin on both bars so neither sits flush on the boundary.
Total is now ~38px with room to spare. This also delivered the "reduce vertical
spacing" ask for free.

**Other changes this round:**
- **Hover-reveal box.** `showBox` (bool) → `boxMode` (hover / always / never, default
  hover), matching `taskbar-fluent-media-player-fork`. Needed
  `IsHitTestVisible(true)` on the outer Border - and note the non-obvious part: when
  the box is hidden the Background is set to a **transparent brush rather than
  `ClearValue`**, because a Border with no Background at all is invisible to hit
  testing and would never receive the `PointerEntered` that reveals it.
- **A real handler leak, same class as the anchor-tracking bug from §9.** Re-injection
  builds a fresh Border and overwrote `g_pointerEnteredToken`/`g_pointerExitedToken`
  without revoking the old ones, stranding live delegates pointing into this DLL on the
  discarded element. Extracted `DetachHoverTracking()` and called it from both
  `RemoveWidget` and `InjectWidget` - exactly the fix shape the anchor-tracking bug
  needed. Worth internalising the pattern: *every* subscription this mod makes on a
  per-injection element needs a detach that runs on re-injection, not only on unload.
- **Width stability, properly this time.** 1.2.0 added figure-space padding but the
  visible jitter had a different cause: the extras cell (`"690MHz · 8.7W"`) was wider
  than its 78px column and clipped its own trailing `W`. Fixed at the source -
  `FormatPower` now uses whole watts always (a sub-10W `8.7W` is two glyphs wider than
  `53W`), and `FormatClock`'s two branches were tuned to both land on 7 glyphs across
  the MHz/GHz switch. Percentages pad to 3 integer digits so 100% doesn't widen a cell.
- **Deterministic vertical text alignment.** Every cell now gets an explicit
  `LineHeight(kRowHeight)` + `LineStackingStrategy::BlockLineHeight`. Without it a
  TextBlock's height follows its font's natural metrics, so cells whose glyphs differ
  in ascender/descender extent (an arrow, a degree sign, plain digits) each centre on a
  slightly different line.
- **Content-derived widths.** `leftWidth` was `max(120, rightWidth * 1.85)` - a magic
  ratio that left slack. Both panels are now the exact sum of their fixed columns, so
  the widget is as narrow as its content allows. Label/temp/graph-gap columns were also
  tightened per the ask (31→26, 48→36, 8→6).
- **Network column**: arrow moved into its own fixed column so it stops drifting as the
  number beside it changes width, which also supplies the requested arrow-to-value gap
  without padding either string. Column widened to 90px to fit `999.99 KB/s` at the
  user's 2-decimal setting.
- Memory bar now spans the full RAM/VRAM row width instead of an arbitrary sub-range.
- Hover repaint is background-only (`ApplyBoxBackground`) rather than a full
  `ApplyWidgetGeometry` pass on every pointer enter/exit.
- Verbose narrative comments trimmed; the ones documenting real traps kept.

Version 1.3.0, all four scripts clean.

---

## 13. 1.4.0 — the alignment fix that should have happened in 1.2.0

The user was (rightly) frustrated: three rounds of "width stability" work and the
columns still visibly shifted. **The cause was a finding I had already been handed and
did not act on.** The research agent's Part B answer in §11 said plainly that
figure-space padding "doesn't guarantee fixed *pixel* width unless digits are also
tabular/monospaced", and that `taskbar-clock-customization-v3` relies on the *user*
picking a tabular font for its own padding to work. I ported the padding and left the
default font as `Segoe UI Variable Text` — a proportional face where `1` is narrower
than `0`, so the padding could never have worked. Padding fixes the glyph *count*; it
cannot fix the glyph *width*.

**The actual fix is three things, all now configurable:**
1. **A monospaced default font.** `fontFamily` became a dropdown (Consolas by default,
   plus Cascadia Mono/Code, Lucida Console, Courier New, the two Segoe faces, and a
   `fontFamilyCustom` escape hatch). This is the part that matters.
2. **Tabular figures.** `Documents::Typography::SetNumeralAlignment(text,
   FontNumeralAlignment::Tabular)` on every cell - the OpenType `tnum` feature. Free
   for monospaced fonts; it is what makes the proportional options usable at all.
3. **Zero padding.** `numberPadding` (zero / figure-space / none, default zero), so
   `030.0%` rather than `30.0%` - the user asked for this explicitly.

**Other work this round:**
- **Vertical structure reworked.** Rows are now a fixed 13px *text band* (top-aligned,
  `LineHeight` pinned) plus a strip beneath it for the memory bars, inside a 17px row.
  Previously text was centre-aligned in the row and the bars were bottom-aligned in the
  same space, so the bars sat in the text's descender area - the "crunched" look. Text
  in every column now shares one baseline and the bars have clear space.
- **Graph background.** A faint panel (`graphBackgroundOpacity`, default 10%) behind
  each trace, so its extent and baseline are visible when the line is flat - the user
  couldn't see where the graph was.
- **Cell widths recalibrated** against a monospaced 11px face (~6.05px/char) so every
  gutter is a uniform ~6px instead of each cell carrying arbitrary slack. The arrow
  column narrowed to 10px, which was the "too much gap" between arrow and value.
- **Font-size scaling.** Cells are calibrated for 11px and now rescale with
  `fontSize`. This needed a registry of the fixed-width `ColumnDefinition`s
  (`g_scaledColumns`), because a settings change does *not* rebuild the widget - it
  only calls `ApplyWidgetSettings`, so column definitions sized once at construction
  would never update.
- **New settings:** graph width, bar thickness, label colour, label opacity, graph line
  and background opacity, divider toggle, tabular-figures toggle, number padding, font
  dropdown + custom family.

Version 1.4.0, all scripts clean. **Not yet verified live.**

**Lesson worth keeping:** when a research pass hands back a caveat about *why* a
technique might not work, treat it as a requirement, not a footnote. The whole 1.2.0 →
1.4.0 detour was one unread sentence.

---

## 14. 1.5.0 — content-sized columns and a filled graph area

Two asks, both about polish rather than correctness.

**Dead gaps between label and value.** 1.4.0 gave every cell a fixed pixel width sized
for the widest string it could *ever* hold ("100.0%"), so a typical "21%" left ~20px of
empty column. The fix was structural rather than a re-tune of the constants: each panel's
two rows used to be two separate `Grid`s stacked in a parent, which is why the widths had
to be hard-coded — two independent Grids with `Auto` columns would size independently and
the rows would not line up. **Both rows now live in one Grid**, so `Auto` columns become
usable: a column measures the wider of the CPU and GPU value, both rows are aligned
because they are literally the same columns, and nothing reserves space it isn't using.
`CreateComputeRow`/`CreateMemoryRow` became `AddComputeRow`/`AddMemoryRow` writing into
the shared panel.

This deleted a lot: nine cell-width constants, `ScaledPixelColumn`, the whole
`g_scaledColumns` registry and the font-size scale factor that existed only to rescale
those fixed widths (`Auto` follows the glyphs for free), `StarColumn`, `g_memoryBarWidth`,
and the extras-column sizing loop (an `Auto` column collapses on its own when its cell is
`Collapsed`). Net ~90 lines removed. The gutter is now one setting, `cellGap`, applied as
a left margin on the cells that follow another — previously the gutter was slack hidden
inside each cell's width, which is exactly why it was uneven.

One consequence worth knowing: the capacity bar can no longer be sized from a predicted
panel width. The track now stretches the row and `UpdateMemoryBar` reads
`track.ActualWidth()`, which is correct whatever the Auto columns settle on. It reads 0
before the first layout pass, so the bar is empty for at most one tick after injection.

**Graph area fill.** A `Polygon` behind each `Polyline`, built from the same points plus
two baseline corners to close the shape (`graphAreaOpacity`, default 16%). The flat panel
behind it stays — the panel shows the graph's *extent*, the fill distinguishes the line
from the area beneath it. `UpdateSparkline` now populates both from one pass.

Version 1.5.0, all scripts clean. **Not yet verified live.**
