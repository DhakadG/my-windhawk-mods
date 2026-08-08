# Handover — Win-X Hot Corners

Everything a fresh session needs that the code cannot tell it. Read this
before changing anything.

- **Repo:** https://github.com/DhakadG/win-x-hotcorners (public, MIT)
- **Source of truth:** `win-x-hotcorners.wh.cpp` — the *only* tracked copy
- **Working copy:** `Win-x-HotCorners.cpp` — gitignored, edit this, then copy
  over the tracked one before committing
- **Current version:** 4.1.2 — **not yet run.** 4.1.1 is the last version the
  user confirmed loads and works, and is what **PR #5001 now carries** (title
  updated, body rewritten, all 5 CI checks green, `/ai-review` posted
  2026-08-06 at the user's explicit request — they authorised that comment this
  once; do not assume it for the next one)
- **4.1.2 adds `g_reloadLock`.** Lock order is always `g_reloadLock` then
  `g_settingsLock`, never the reverse. It is recursive on purpose: DashSave,
  the two Reset paths and the tray toggles hold it across `ReloadConfig`'s own
  acquisition. A hang on save or on a tray toggle is the failure mode to watch
  for on first run
- **Do not reintroduce an adaptive poll rate.** Two versions of that idea have
  been written and removed. Any rule for easing off decides from a sample taken
  *before* the user starts moving, so a flick can cross a zone between two
  samples — and on a corner shared with another monitor there is no screen edge
  to stop the pointer, so that is a lost trigger, not a late one. Polling
  cannot close it. `hotcorners_logic_check.ps1` has assertions that fail if a
  third attempt appears
- **4.1.0 removed the Windhawk settings block entirely.** The mod is configured
  only from its tray icon; `Wh_GetIntSetting`/`Wh_GetStringSetting` must never
  come back. Everything persists in the mod's own value store, still under the
  historical `ovr_` key prefix so existing setups are not orphaned
- **This file is gitignored on purpose.** It was tracked until 2026-08-05 and
  is still in the history; it does not belong on a public repo that the
  Windhawk PR links to, because it is a working document about the project and
  the person building it, not part of the mod. Do not re-add it.

---

## 1. Current state

| Area | State |
|---|---|
| Detection engine | Shipped and used daily |
| 36 actions | Shipped; user has verified Task View, Switch to Last Window, Lock+Monitors Off |
| Tray icon and menu | Shipped, verified working |
| Settings dashboard | Compiles and runs; user confirmed 2026-08-05 |
| Per-zone settings (v4.0.0) | Compiles and runs; user confirmed 2026-08-05 |
| v4.0.3 end to end | Loaded in Windhawk 2026-08-05: both monitors named, 4 zones armed, clean uninit |
| v4.0.4 | In the repo, **never run.** Touches uninit ordering, so the thing to test is disabling the mod with the settings window open |
| v4.0.5 | **Never run.** Real dark mode (uxtheme ordinals 133/135), multi-slot dashboard persistence |
| v4.1.0 | Loaded in Windhawk 2026-08-05: 3 configurations, 5 zones, triggers firing on both displays, dashboard confirmed good by the user. Settings page deleted; VerboseLogging deleted; per-monitor fullscreen guard; shared alternation on split edges; Options page grouped |
| v4.1.1 | Run and confirmed working 2026-08-06. Reload is one locked transaction (4.1.0 had a real race: a rebuild landing between the two phases armed no zones); adaptive poll rate removed; Reset button caption fixed |
| v4.1.2 | **Never run.** g_reloadLock serialises write-then-reload; a failed GetCursorPos no longer drops to the idle rate. Third CodeRabbit pass |
| Windhawk PR #5001 | OPEN, carries **v4.1.1**, all 5 checks green 2026-08-06. Do not push 4.1.2 until the user has run it |
| Windhawk PR #5001 | OPEN, carries **v4.0.3**, all 5 checks green since 2026-08-05 |

**The PR now tracks the current version.** It sat at v3.5.0 for four releases
because nothing newer had been compiled; the user confirmed on 2026-08-05 that
v4.0.x builds and runs, which is what released the hold. Keep that bar: do not
push a version to the PR that the user has not run.

**The PR is still in the author-driven review stage.** The repo runs a
two-stage review: the author comments `/ai-review`, then `/ready-for-reviewer`
to hand it to a human. `/ai-review` was posted 2026-08-03 and the bot never
answered; `/ready-for-reviewer` has never been posted. The user posts these
themselves — do not comment on the PR on their behalf.

### Verification status is the thing to be careful about

There is **no C++ toolchain on this machine**. Nothing has ever been compiled
by the assistant — every build error in this project was found by the user
pasting it back. "All checks passed" means the *algorithms* are verified by
simulation, never that the file builds. Say so plainly rather than implying
otherwise. The user compiles in Windhawk and reports back; treat their word as
the only build evidence there is.

---

## 2. How to verify

All three default to the tracked `win-x-hotcorners.wh.cpp`, so they work in a
fresh clone. Pass the working copy while it is still ahead of the tracked file:

```powershell
# algorithm simulation — 79 assertions
scripts\hotcorners_logic_check.ps1 -Path "Win-x-HotCorners.cpp"

# pre-compile static checks — 9 classes of error
scripts\cpp_sanity_check.ps1 -Path "Win-x-HotCorners.cpp"

# proves the sanity checker still catches all 9 known bugs
# (always reads the tracked file — copy over first)
scripts\test_sanity_check.ps1
```

Healthy output ends `ALL CHECKS PASSED` / `SANITY CHECKS PASSED` /
`CHECKER VERIFIED`.

**Is the mod actually loaded?** Compiling is not loading:

```powershell
ls "$env:ProgramData\Windhawk\Engine\Mods\32" -Filter "*hotcorners*"
Get-CimInstance Win32_Process -Filter "Name='windhawk.exe'" | % CommandLine
```

You need a DLL in `Mods\32` **and** a `windhawk.exe -tool-mod "..."` process.
Absence of either means it never loaded, whatever the Windhawk UI shows.

---

## 3. Environment facts

These were expensive to discover. Do not rediscover them.

| Fact | Consequence |
|---|---|
| `windhawk.exe` is **32-bit** on 1.x (with a separate `windhawk-x64-helper.exe`), and **64-bit** from 2.0 alpha 2 onwards | Never declare `@architecture` **at all** on a tool mod. Pinning x86-64 builds only to `Mods\64` and the 1.x host has nothing to load; pinning x86 would now break 2.0. With no declaration Windhawk builds every architecture and loads whichever matches, which is correct on both. It fails **silently** either way — no error anywhere. The sanity checker enforces this |
| The mod must compile as 32-bit *and* 64-bit | cdecl ≠ stdcall, so lambdas cannot be passed to `WNDENUMPROC` etc. (this only breaks the x86 build, which is exactly why it is easy to miss); 64-bit values shared across threads can tear on x86 |
| Tool-mod processes run at **medium** integrity, same as explorer | UIPI is not the cause of input problems. Check something else |
| System is classic **S3** sleep, not Modern Standby | `SC_MONITORPOWER` is expected to work |
| Monitors: `DELL S2725QC` 3840×2160 at (0,0) primary; `BOE0C29` 1920×1080 at (−1920,772) | Mixed DPI, negative coordinates — test both |
| Cursor max is (3839, 2159) | `rcMonitor.right/bottom` are **exclusive** and unreachable. Half-open rects are correct; making them inclusive would overlap adjacent monitors |
| CodeRabbit CLI installed, **Free** plan | Needs a git repo, an explicit `--base`, and allows ~one review per hour (exit 75) |
| `gh` authenticated as DhakadG | Commits must use `73574085+DhakadG@users.noreply.github.com` — GitHub rejects pushes carrying the real email |
| Source file is **LF**, no BOM | PowerShell `Set-Content`/`WriteAllText` must preserve it |
| The mods repo CI compiles against Windhawk **1.6.1, 1.7.3 and 2.0.0-alpha.2** | 2.0 validates the settings block far more strictly than 1.x. A local build proves nothing about the 2.0 job — that is the only compiler you get for it |
| The 2.0 job builds x86, x64 **and ARM64**, and fails on **any compiler warning** ("The mod compiled with warnings"), while 1.6/1.7 do not | A warning is a build failure there. C++20 rules apply: arithmetic between two different enum types is deprecated, so cast one side |

---

## 4. Decisions and why

Do not re-open these without a reason.

**Polled detection, not `WH_MOUSE_LL`.** A low-level hook must return within
`LowLevelHooksTimeout` (300 ms) or Windows skips it and eventually removes it
silently — presenting as "corners work sometimes". It also routes every mouse
event system-wide through the mod. A 16 ms poll on a dedicated thread has a
hard latency bound and costs the rest of the system nothing.

**Tool mod (`@include windhawk.exe`), not injected into explorer.** In
explorer, detection shared the taskbar's UI thread: `WM_TIMER` was starvable,
actions blocked the hook, and the hook died. Its own process removes the whole
class of problem.

**Monitors identified by EDID friendly name, not enumeration order.** Ordinals
shift when you rearrange displays or change which is primary, silently
repointing a configuration at a different screen.

**Half-open zone rectangles.** Verified empirically; inclusive bounds would
make adjacent monitors' zones overlap at the shared edge.

**Edge thickness capped at the smaller of its two adjacent corners.** This one
rule is what keeps all 12 zones disjoint under arbitrary per-zone sizes.
Verified with 3000 randomised size combinations. The hit test is
first-match-wins, so an overlap silently makes a zone unreachable.

**Per-zone overrides use `-1` = inherit.** Existing configurations behave
identically; nothing changes until the user asks.

**Alternating actions are two extra action types, not a second action+args
pair on every zone.** The latter would double a settings block that is already
twelve zones long. Both halves live in the existing Args field split on `|`,
so the zone structure, hit test and detection loop are untouched. The flip flag
rides inside the executor via `shared_ptr`, and each zone gets a freshly built
executor so zones sharing one `*` config alternate independently.

**Dashboard writes to the mod value store, not Windhawk settings.**
`Wh_GetIntSetting` is **read-only** from inside a mod. `Wh_SetIntValue` writes
a *different* store. The dashboard therefore keeps overrides and layers them
over the settings at load; "Reset to Windhawk settings" clears the layer.
This is not a workaround — there is no API to do otherwise.

**Tray runs on its own thread.** `TrackPopupMenu` is modal; hosting it on the
detection thread freezes every zone while the menu is open.

**Global 250 ms floor between any two actions.** Per-zone cooldowns do not stop
a sweep across *different* zones queueing a burst — four Win+Tab in 40 ms was
observed.

---

## 5. Traps — bugs already made once

| Symptom | Cause | Fix |
|---|---|---|
| Mod totally silent, no log at all | `@architecture x86-64` on a tool mod | Remove it |
| `undefined symbol: _CreateDIBSection` | GDI call with no `-lgdi32` | Check every API against `@compilerOptions` |
| `expected expression` on a `RECT{...}` | Parameters named `near`/`far` — `windef.h` `#define`s both to nothing | Rename |
| `redefinition of X` | PowerShell `String.Replace()` replaces **every** occurrence; the anchor was a prefix of both a forward declaration and a definition | Use unique anchors, verify the replacement count |
| `no matching function for EnumChildWindows` | Non-capturing lambda decays to cdecl; `WNDENUMPROC` is stdcall | Free function marked `CALLBACK` |
| Zone fires, nothing visible happens | Corner and adjacent edge both bound to the same *toggle*; crossing fires both and they cancel | Pass-through guard (80 ms) |
| Zones dead while Task View is open | Task View is monitor-sized, so the fullscreen guard caught it. v2 was immune only because it ran *inside* explorer and skipped its own process | Compare against the **shell's** pid |
| Display stays on after Lock+Monitors Off | `WM_SYSCOMMAND` posted to `GetForegroundWindow()`, which is null after `LockWorkStation` switches desktop; the fallback `GetDesktopWindow()` handles nothing | Broadcast via `SendMessageTimeout(HWND_BROADCAST, ...)` |
| Preview highlight lights the wrong thing | Edge and centre rects overlapped in the diagram | Split edges around their centre, hit-test centres first |
| `Failed to parse settings` on Windhawk 2.0, fine on 1.7 | A `$options` dropdown whose value is a number. 2.0 rejects the **whole** settings block, so nothing loads | Values with `$options` must be strings; map them to numbers in one place in the code |

**Meta-trap:** the assistant's own checker was once structurally blind to the
bug it was written for — its regex required whitespace before a function name,
so `wchar_t *Name(` matched zero times. `test_sanity_check.ps1` exists because
of that: it reintroduces all 8 known bugs and asserts each is caught. Run it
after touching the checker.

---

## 6. Open threads

**#11 — GIFs and screenshots.** Only the user can record these. Needed in both
`README.md` and the in-mod readme block (the Windhawk catalog renders that one).

**Deliberately not done, mentioned to the user:**
- Tabs still look like buttons; a real segmented control needs owner-drawing
- "Blank delay after lock" is always editable, not gated on the action being selected
- Genuine full-window Mica: classic Win32 controls paint opaque, so Mica leaves
  solid rectangles floating on translucency. The current approach requests the
  backdrop *and* themes everything dark, which is what actually looks native.
  Real Mica needs owner-drawn controls or XAML Islands — a separate project

**PR updates:** push to the `add-win-x-hotcorners` branch of the
`DhakadG/windhawk-mods` fork; the PR picks it up and re-runs validation. The PR
body must keep the `## Mod authorship` section or `pr_validation.py` fails.
Editing the body alone does **not** re-trigger validation (`on: pull_request`
has no `edited` type) — either push a commit alongside it, or close and reopen.

---

## 7. Working with this user

- **Wants the reasoning, not just the fix.** Explain the mechanism.
- **Corrects scope creep.** "I converted the mod to a tool-mod process, which
  was extra scope beyond the fix" was a fair hit — do what was asked.
- **Will say when something is wrong.** "nothing is working in this mod...
  please do a better work than wasting tokens" followed three failed versions.
  Diagnose with evidence before changing code.
- **Asks for research before implementing.** They explicitly asked for other
  mods to be studied before the tray icon and the dashboard. Doing so found the
  GUID identity and `NOTIFYICON_VERSION_4` patterns that were missing.
- **Wants AI mentions kept out of the product.** Code, README and docs carry
  none. The one exception is the Windhawk PR template, which asks directly how
  the mod was authored — that is answered honestly (`with AI assistance` /
  `Claude`), and the user was told why.

---

## 8. Reference

**Layout metrics** live in `namespace Lay`; the window is sized from them via
`AdjustWindowRectEx`. Eight assertions parse those constants *out of the
source*, so changing one re-derives the test rather than drifting from it.

**Useful mods to crib from** (in `%ProgramData%\Windhawk\ModsSource`):
`audioswap.wh.cpp` and `microswap.wh.cpp` for tray and GUI,
`bt-battery-monitor.wh.cpp` for dark mode, `per-monitor-scale-switcher` for
`QueryDisplayConfig` monitor naming.

**Feature backlog:** `docs/FEATURE-BACKLOG.md` — all 70 upstream
vhanla/winxcorners issues triaged. ~17 are already solved here.
