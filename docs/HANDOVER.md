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
3. **Only then** copy the file into the gallery fork (`DhakadG/windhawk-mods`, branch
   `add-win-x-hotcorners`) and push, which updates
   [ramensoftware/windhawk-mods#5001](https://github.com/ramensoftware/windhawk-mods/pull/5001).

Never push a mod change straight to the PR branch, however small. The gallery's check
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
pwsh -File scripts/check-mod.ps1 -Warnings && python scripts/check-settings.py
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
- All five mods currently pass with **zero warnings**.
- `scripts/cpp_sanity_check.ps1` is the older heuristic checker. Its brace/paren counts are
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
| win-x-hotcorners | 4.1.4 | — (lost_husky's own) | Repo is ahead of installed 4.1.2 |
| taskbar-ai-quota-fork | 0.12.0 | Cleroth | Working |
| taskbar-clock-customization-v3 | 3.1.71 | m417z | Working |
| taskbar-fluent-media-player-fork | 1.6.1 | Salyts | Working |
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

Geometry is verified rather than eyeballed: `scratchpad/zonegeom.cpp` copies
`ZoneRectInDiagram` verbatim and asserts the 16 pieces tile the border ring with no overlap,
no gap and nothing outside the box, at six aspect ratios including portrait. Re-run it if you
touch the proportions.

Reference: [docs/hotcorners-dashboard-draft.html](hotcorners-dashboard-draft.html).

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

**[docs/hotcorners-dashboard-draft.html](hotcorners-dashboard-draft.html)** (also published at
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
- `gdigrab` does capture Task View, Start and Quick Settings, so it needs no replacing.
- Synthetic right-clicks on the tray icon never open the menu (likely UIPI against the
  elevated process); `Shell_NotifyIconGetRect` with the mod's GUID locates the icon fine.
  The dashboard can be opened by posting `WM_COMMAND` `IDM_SETTINGS` (100) to the
  `WindhawkHotCornersTray` window — but `FindWindowW` does not match the dashboard class,
  so find it by title instead.

## 7. Smaller open items

- **`win-x-hotcorners` 4.1.4 is not installed** — 4.1.2 is. Either compile 4.1.4 or work out
  why it was skipped. `scripts/sync-from-modssource.ps1` prints versions as it copies.
- **Donation links** — Windhawk's `@donateUrl` metadata field renders a Donate button but
  takes only one URL; Ko-fi is set on `win-x-hotcorners` and the fork. The rest are badges in
  a `## Support` readme section, with a comment explaining the shields.io escaping (doubled
  underscores, doubled dashes). Placeholders noted for OnlyChai and BuyMeChai.
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
