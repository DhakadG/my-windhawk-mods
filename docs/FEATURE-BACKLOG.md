# Feature backlog — from vhanla/winxcorners issues

All 70 issues (57 open, 13 closed) reviewed. Grouped by whether they apply to
this mod, and what each would cost to build.

Pick from **§2** and **§3**; §1 needs nothing, §4 and §5 explain what I'd skip
and why.

---

## 1. Already solved here

Roughly half the open issues on that tracker are things this mod already does.
Worth knowing before choosing what to build.

| Issue | Request | How it's covered |
|-------|---------|------------------|
| #5 | Start menu hot corner | `Start Menu` action |
| #6, #13, #39 | Pause while a fullscreen app / DX12 game is running | **Disable on fullscreen apps**, incl. non-exclusive fullscreen and Win11 shell surfaces |
| #9 | Allow 0 ms delay | Activation delay defaults to 0 |
| #12 | "Do nothing" option | `Nothing` action |
| #14 | Lock screen action | `Lock Computer` |
| #10, #63 | Task Switcher (Ctrl+Alt+Tab) | `Task Switcher` — **added in v3.4.0** |
| #81 | Alt+Tab swap with last window | `Switch to Last Window` — **added in v3.4.0** |
| #85 | Minimize focused window | `Minimize Active Window` — **added in v3.4.0** |
| #20 | Virtual desktop actions | Next / Previous / New — **added in v3.4.0** |
| #7, #52 | Custom commands / arbitrary hotkeys | `Custom Command`, `Virtual Key Press` |
| #30, #70 | Multi-monitor, extended screens | Per-monitor config keyed by display name |
| #73 | Hot edges, not just corners | Four edge zones per monitor |
| #75 | Disable in certain applications | Excluded processes list |
| #54 | Short delay on activation | Activation delay setting |
| #37 | Corner stops triggering after first use | Enter/leave state machine; re-arms on exit |
| #1 | HiDPI issues | Detection is per-monitor DPI aware |
| #55 | Corner randomly won't trigger | Polled detection — no hook to be silently dropped |

---

## 2. Small — self-contained, low risk

### 2a. Lock session when turning off displays — *#34*
macOS asks for a password when the display sleeps; this doesn't.
**Build:** one new action that calls `LockWorkStation()` then posts
`SC_MONITORPOWER`. Ordering matters — lock first, or the display wakes.
**Cost:** ~15 lines + one dropdown entry.

### 2b. Keep-awake toggle — *#31, #60*
A zone that suspends the screensaver and sleep timers, and a way to release it.
**Build:** `SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED)` to
arm, `ES_CONTINUOUS` alone to release. Needs to be a toggle, so it depends on
§3a or a dedicated on/off pair.
**Cost:** ~30 lines. Note the state is per-thread — it must be set on a thread
that stays alive, i.e. the worker.

### 2c. Bottom-centre / edge-centre zones — *#76, #82*
On ultrawides the bottom-centre is easier to hit than any corner.
**Build:** split each edge strip into left/centre/right thirds, or add four new
centre zones. The second is less disruptive to existing configs.
**Cost:** ~40 lines + 4 more zones × 8 dropdown blocks in settings. The
settings YAML is the bulk of it.

### 2d. Close virtual desktop
Rounds out the virtual-desktop set added in v3.4.0 (Win+Ctrl+F4).
**Cost:** 3 lines + one dropdown entry.

---

## 3. Medium — genuinely useful, some design needed

### 3a. Second-trigger action — *#53, #69*  ← the one you flagged
Two distinct requests that share machinery:

- **#53** wants the *same* action undone on the second bump (GNOME behaviour).
  For toggles like Task View this is already what happens.
- **#69** wants a *different* action on the second trigger — e.g. `Alt+S` to
  show, `Alt+H` to hide.

**Build:** give each zone a second action + args, and a per-zone `armed` flag
that flips on every fire. Zone config grows from 2 fields to 4.
**Design questions worth deciding first:**
- When does the flag reset? Never (pure alternation), on a timeout, or when the
  foreground window changes? Pure alternation desynchronises the moment the
  user does the same thing by keyboard.
- Does it survive a settings reload or display change? Currently all per-zone
  state resets on rebuild.

**Cost:** ~80 lines + doubling the per-zone settings block. The settings YAML
is again most of the work.
**My take:** the most valuable item on this list, but decide the reset rule
before any code — that choice is the whole feature.

### 3b. Knock-knock gesture — *#19*
Trigger only when the cursor hits an edge twice in quick succession, like
knocking. Sharply reduces accidental activation, which is the main complaint
about hot corners generally.
**Build:** the detection loop already tracks enter/leave per zone; add a
"previous exit time for this zone" and require a second entry inside a window.
**Cost:** ~40 lines, one setting. Fits the existing state machine cleanly.
**My take:** strong candidate. Cheaper than 3a and solves a real annoyance.

### 3c. Modifier-gated zones
Not in their tracker, but it addresses the same accidental-trigger problem:
only fire when e.g. Ctrl is held.
**Build:** one `GetAsyncKeyState` check in `DetectTick`, one setting per zone
or one global.
**Cost:** ~20 lines global, more if per-zone.

---

## 4. Large — possible, but a different mod

### 4a. Clickable hot corners — *#18*
Draw a visible clickable button in the corner rather than triggering on hover.
Needs a layered always-on-top window per corner per monitor, hit-testing,
theming, and DPI handling. That's a UI mod, not a detection mod.

### 4b. Drag-and-drop aware overview — *#21*
Windows' own Task View doesn't accept dropped files, so upstream is writing a
custom window switcher. Enormous scope: enumerate windows, thumbnails, drop
targets.

### 4c. Hover actions on application windows — *#23*
Buttons in the corners of *every* window for centre/resize/move. Different
product entirely.

### 4d. Pascal script support — *#22*
Upstream embeds a script engine. Not appropriate here — `Custom Command`
already runs any script interpreter you like.

---

## 5. Not applicable

These are artefacts of WinXCorners being a standalone tray app. Windhawk
handles them, so there is nothing to build.

| Issue | Why it doesn't apply |
|-------|----------------------|
| #11, #67 | Tray icon appearance / tray menu — this mod has no tray icon |
| #27, #79 | `settings.ini` location and write errors — Windhawk stores settings |
| #57, #72 | Autostart — Windhawk starts the mod |
| #59, #15 | winget / Microsoft Store packaging — install via Windhawk |
| #16, #32, #25, #33, #35 | Install, extract, build, release questions |
| #26, #66, #71 | Main-window layout with the taskbar on top — no window |
| #48 | Malware false positive on their binary |
| #2, #3, #24, #40, #58, #64 | Crashes in the Delphi implementation |
| #8, #28 | "Does it work on Windows 11" — yes |
| #84 | ARM64 build — the mod declares no architecture, so Windhawk builds whatever the host needs |

### Two worth a second look

- **#77** — bottom-right corner conflicts with the taskbar's own "peek at
  desktop" strip. Real on Windows 10. Our zones are ~4 px and the peek strip
  overlaps them. Mitigation today: use `Nothing` for bottom-right, or shrink
  corner size. A proper fix means excluding the taskbar's rect from zone
  computation — feasible, ~30 lines, but only matters if you hit it.
- **#78** — a triggered context menu doesn't close by itself. Only affects
  actions that open menus; our cooldown and pass-through guard make the
  reported double-fire much less likely.

---

## Suggested order

1. **§3b knock-knock** — best annoyance-reduction per line of code
2. **§2a lock on display sleep** — trivial, clear win
3. **§3a second-trigger action** — highest value, but settle the reset rule first
4. **§2c bottom-centre zones** — only if you use an ultrawide
