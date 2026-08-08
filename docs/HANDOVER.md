# Handover — Windhawk mods

Starting context for a fresh session. Everything here is verified, not assumed. Read §1 and
§2 before touching anything; the rest is per-task.

**Repo:** [DhakadG/my-windhawk-mods](https://github.com/DhakadG/my-windhawk-mods) (public)
**Working folder:** `C:\Users\lost_husky\Downloads\Programs\VS Code Works\WindHawk Mods`

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
- `check-settings.py` — parses the `==WindhawkModSettings==` block with PyYAML. **clang
  cannot catch this class of bug**: a malformed settings block compiles and loads fine, then
  fails later as `Failed to extract previous initial settings for engine`. Its byte/line/column
  is relative to the YAML block, not the `.cpp`, which is why the numbers never match the editor.
- All five mods currently pass with **zero warnings**.
- `scripts/cpp_sanity_check.ps1` is the older heuristic checker. Its brace/paren counts are
  wrong on files containing `LR"(...)"` raw strings (the mangled symbol literals) — compare
  the delta against the unmodified baseline rather than trusting the absolute number. Its
  `@architecture` rule only applies to `windhawk.exe` tool mods.

**Always bump `@version` when you change a file**, even for a one-line fix. A version that
means two different files has already caused one wasted debugging round here.

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

## 5. NEXT TASK — win-x-hotcorners settings + dashboard

The mod retired its settings page in 4.1.0 for a dashboard, because a flat list of ~40
settings was unusable in the old Windhawk UI. **Windhawk v2 removes that constraint**:
settings render as collapsible groups with per-group reset and a settings count on the
collapsed header. `taskbar-fluent-media-player` already uses this shape and is the reference.

The decision reached: **do both, with a clear split.**

- **Windhawk settings page** becomes the place you *configure*. Group the settings the way
  the media player does (Zones, Actions, Timing, Per-monitor, Appearance, Debug). Keep the
  existing setting keys unchanged so nobody's configuration resets.
- **Dashboard** becomes the place you *see*. Not a second config surface — a visual map.

### Dashboard UI draft (to be built)

Needs a draft **before** implementation, and it must be something Windhawk XAML can
actually express. Follow the fluent media player's popup for theme and construction
(`taskbar-fluent-media-player.wh.cpp` — its mini-player flyout is the closest working
reference for a modern surface built from a Windhawk mod).

Intended shape:

- A **tab strip** across the top, one tab per monitor, labelled with the monitor's friendly
  name — the mod already identifies monitors by friendly name rather than list position
  (that was the 4.1.4 fix), so the label and the setting agree. Fall back to the Windows
  monitor number when the friendly name is unavailable or duplicated.
- A **single rectangle centred in the panel** representing that monitor's screen, correctly
  proportioned to its aspect ratio.
- The **8 zones** (4 corners + 4 edges) drawn on the rectangle, each showing its configured
  action. Unconfigured zones visibly inert rather than blank.
- Hover/selection on a zone reveals the full action string; clicking it opens the
  corresponding setting — or, if that is awkward, just tells the user which setting to open.
- Read-only by default. Any editing capability is a later decision, not part of the first draft.

Open questions for the draft: how to represent alternate-action zones (`A | B`), and whether
the rectangle should reflect real monitor arrangement or just be one screen at a time. Do the
draft first, agree it, then implement.

---

## 6. NEXT TASK — bt-battery-monitor-fork

Two asks: **add earbud and Bluetooth-speaker icons**, and **fix battery not showing** for
"OnePlus Nord Buds 4 Pro" and "LotsOfHusky" (a BT speaker). Windows Settings shows 40% and
90% for these, and the mod's own context menu lists both devices with `--`.

### Diagnosis — already done, and it rules out the current approach

The mod reads battery with
`CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Bluetooth_BatteryLevel, ...)` and additionally
requires the devnode to expose an active **HID** or **BLE GATT** interface
(`GUID_DEVINTERFACE_HID` / `GUID_BLUETOOTH_GATT_SERVICE_DEVICE_INTERFACE`, ~line 499-508).
Classic Bluetooth audio devices are neither.

Enumerated on this machine:

- **No property matching `Battery` holds a value on *any* `BTH*` devnode.** Checked every
  present `BTH*` node, and specifically the container nodes
  `BTHENUM\DEV_ACBF711D1BE0` (speaker, 70 props), `BTHENUM\DEV_F8BA98175437` (earbuds, 65),
  `BTHLE\DEV_642428360B1D` (earbuds BLE, 66). None carry it.
- So the failure is **not** a filtering bug that can be fixed by relaxing the HID/GATT gate.
  The data is not in the PnP property store at all for these devices.

The earbuds *do* expose `BTHLEDEVICE\{0000180F-...}` — **0x180F is the GATT Battery Service**,
so a BLE GATT read is a plausible path for them specifically. The speaker appears only as a
classic `BTHENUM` device, so its level almost certainly arrives over HFP battery indicators.

### Where to look next

Windows Settings and Control Center both show these values, so the OS has them. The most
likely source is the **WinRT Association Endpoint layer** rather than SetupAPI —
`Windows.Devices.Enumeration.DeviceInformation.FindAllAsync` requesting the AEP battery
property (`{104EA319-6EE2-4701-BD47-8DDBF425BBE5} 2` as an AEP property, not a devnode one),
and/or `Windows.Devices.Bluetooth` + a GATT read of service 0x180F characteristic 0x2A19 for
the BLE case. **Confirm which one actually returns a value before writing any mod code** —
that is what the last session's tray fix got right and what the guessing got wrong.

Note the mod is `@include windhawk.exe`, i.e. a **tool mod**. Per an earlier finding here,
declaring `@architecture x86-64` silently stops a `windhawk.exe` tool mod from ever loading —
leave the architecture line alone.

Icons are the easy half: the mod already has per-device-type icon presets (Keyboard, Mouse,
Headphones, Controller) with a Browse button for custom `.ico`. Adding Earbuds and Speaker
means extending that enum, its settings dropdown, and the device-type detection.

---

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
