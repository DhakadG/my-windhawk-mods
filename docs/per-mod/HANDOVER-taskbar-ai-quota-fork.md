# Handover — Taskbar AI Quota Bars (Windhawk mod fork)

**Purpose of this file.** Starting context for a fresh Claude Cowork instance. Everything below is
carried over from a prior chat session. Read it before touching code — several root causes are
already diagnosed and should not be re-derived.

**Status:** v0.11.0 was written and delivered but **does not render on the taskbar** after a recent
Windows Insider update. The mod worked before the update. Priority 1 is making it render again.

---

## 1. Environment and file map

| What | Where |
| --- | --- |
| Live mod source (Windhawk reads this) | `C:\ProgramData\Windhawk\ModsSource\local@taskbar-ai-quota-fork.wh.cpp` |
| Working folder / deliverables | `C:\Users\lost_husky\Downloads\Programs\VS Code Works\WindHawk Mods\` |
| Latest build (broken on current OS) | `…\WindHawk Mods\taskbar-ai-quota-fork-v0.11.0.wh.cpp` |
| Reference mod — placement/layout | `C:\ProgramData\Windhawk\ModsSource\taskbar-fluent-media-player.wh.cpp` |
| Reference mod — user's own, resilient | `C:\ProgramData\Windhawk\ModsSource\local@taskbar-clock-customization-v3.wh.cpp` |
| Upstream unforked original | `C:\ProgramData\Windhawk\ModsSource\taskbar-ai-quota.wh.cpp` |

- OS: **Windows 11, Insider channel.** Rollback is off the table — do not propose it.
- Fixes must stay compatible with older/stable Windows 11 builds, not just the Insider build.
- Mod id `taskbar-ai-quota-fork`, currently `@version 0.11.0`, author field still `Cleroth` (upstream).
- Workflow: the user pastes the `.wh.cpp` into the Windhawk editor and compiles there. **There is no
  compiler in the Cowork sandbox** — WinRT headers are unavailable, so builds cannot be verified
  locally. Static checks only (see §7).

---

## 2. Priority 1 — mod does not render after the Windows update

### Verified facts about the failure surface

Line numbers refer to `taskbar-ai-quota-fork-v0.11.0.wh.cpp`.

`HookTaskbarDllSymbols()` (~line 4598) requests **7 symbols from `taskbar.dll`**:

| Symbol | Purpose |
| --- | --- |
| ``const CTaskBand::`vftable'{for `ITaskListWndSite'}`` | address only |
| ``const CSecondaryTaskBand::`vftable'{for `ITaskListWndSite'}`` | address only |
| `CTaskBand::GetTaskbarHost` | address only |
| `CSecondaryTaskBand::GetTaskbarHost` | address only |
| `TaskbarHost::FrameHeight` | address only (its **bytes are pattern-scanned**, lines ~2981 / ~2995) |
| `std::_Ref_count_base::_Decref` | address only |
| `TrayUI::StartTaskbar` | the only actual **hook** |

Two structural weaknesses, both load-bearing:

1. **All-or-nothing.** `WindhawkUtils::HookSymbols(...)` returns false if *any single* entry fails.
   `Wh_ModInit` (~line 4770) returns `FALSE` on that → **the mod never loads at all**.
2. **No symbol alternates.** Every entry supplies exactly one mangled string. Compare the user's own
   `taskbar-clock-customization-v3`, which supplies **multiple alternate mangled strings per hook**
   (see its `symbolHooks[]` around line 7210) and version-gates hook sets via a `WinVersion` enum.
   That is precisely why the clock mod survived the update and this one did not.

Those 6 address-only symbols all feed `TryGetTaskbarElementAbi()` (~line 2940), which is the **only**
path to `GetTaskbarXamlRoot()`. There is already a partial guard at lines ~3025–3026 that returns
failure if three of them are null — but failure there just means no XamlRoot, so injection silently
never happens and the watchdog spins forever.

Also note `TaskbarHost::FrameHeight` is used as a **byte-pattern scan target**, not called. Any
codegen change in that function (inlining, register allocation, a new prologue) breaks it even when
the symbol still resolves.

### Ranked hypotheses

1. **`Wh_ModInit` returns FALSE — mod never loads.** One of the 7 symbols was renamed/removed/inlined
   in the Insider build. *Distinguishing signature:* Windhawk shows the mod as failed/not loaded, and
   the log contains `HookTaskbarDllSymbols failed` with **no** subsequent `InjectQuotaGrid` lines.
2. **Mod loads, but `TryGetTaskbarElementAbi` fails → no XamlRoot.** Symbols resolve but the
   `FrameHeight` byte pattern or the vftable offset walk no longer matches. *Signature:* repeating
   `InjectQuotaGrid failed: … reason=no XamlRoot`.
3. **XAML tree renamed.** `SystemTrayFrameGrid` / `SystemTray.SystemTrayFrame` restructured or renamed
   in the new shell. *Signature:* repeating `reason=position target not in the visual tree yet`.
   Note `taskbar-fluent-media-player` would be broken in the same way — **ask the user whether that
   mod still works**; it is a free A/B test that isolates this hypothesis.
4. **v0.11.0 regression unrelated to the OS.** Possible but unlikely to present as "nothing renders";
   v0.11.0 was never confirmed working on the pre-update OS either. Worth ruling out by testing the
   pre-fork `taskbar-ai-quota.wh.cpp` on the current build.

### Diagnostic sequence (do this before writing any fix)

1. Ask the user to enable mod logging in Windhawk (Advanced → debug logging) and capture
   **DbgView/DbgViewMini** output across an Explorer restart. The log line that appears decides
   between hypotheses 1/2/3 outright. The user has supplied DbgView captures before and is
   comfortable doing so.
2. Get `winver` / the exact Insider build number.
3. Ask whether `taskbar-fluent-media-player` and `taskbar-clock-customization-v3` still work right now.
4. Only then choose the fix.

### Fix direction (regardless of which hypothesis lands)

- **Make symbol resolution non-fatal and individually optional.** Split the 7 into "required" vs
  "optional", supply alternate mangled strings per entry (mirror the clock mod's pattern), and let
  `Wh_ModInit` succeed with degraded capability rather than returning `FALSE`.
- **Add a symbol-free fallback path to the XamlRoot.** Do not depend solely on the
  `CTaskBand`/`TaskbarHost` walk. Diff `GetTaskbarXamlRoot` against the media player mod's version and
  consider locating the XAML island by window class instead
  (`Windows.UI.Composition.DesktopWindowXamlSource` / `XamlExplorerHostIslandWindow` children of the
  `Shell_TrayWnd` tree).
- **Add version detection** like the clock mod's `WinVersion` enum so newer shells can take different
  code paths without breaking older ones. This is what keeps cross-version compatibility.
- **Log loudly on each symbol miss** (name + which one), so the next OS update is a 30-second diagnosis.
- Consider porting the media player's `DumpXamlTree` behind a debug setting — invaluable when element
  names change.

---

## 3. Priority 2 — occasional vanishing (both mods)

The quota mod's vanishing was **already root-caused and fixed in v0.11.0**. Do not re-diagnose:

> The old `InjectQuotaGrid` early-returned `true` whenever `state->quotaGrid` was non-null. A WinRT
> reference stays perfectly valid after Explorer detaches the element from the visual tree, so every
> re-injection attempt silently succeeded at doing nothing. `RetryInjectThreadProc` also `break`'d
> permanently on first success, so nothing was watching afterwards.

v0.11.0 replaced this with `EnsureQuotaGrid()` + a permanent watchdog (100 ms while missing, 3 s
heartbeat once healthy) and a real liveness test: visual parent non-null **and** still parented to the
grid the current position setting resolves to (catches Explorer swapping the whole tray frame).

**`taskbar-clock-customization-v3` vanishing is a separate, unsolved problem.** Key insight from
recon: that mod barely touches XAML at all — it hooks `GetTimeFormatEx` / `GetDateFormatEx` /
`GetLocalTime` / `ClockButton::*` / `ClockSystemTrayIconDataModel::*` and rewrites the clock's *text*.
Its only XAML touch is `FindChildByName(dateTimeIconContentElement, L"ContainerGrid")` (~line 6296).
So its "vanishing" is a different failure mode from the quota mod's — likely the hooked data model
being re-instantiated, or `OnApplyTemplate` firing on a fresh element the mod never re-decorates.
Investigate `DateTimeIconContent::OnApplyTemplate` (hooked, ~line 7248) as the re-attach point.

Handle this **only after** Priority 1 is resolved. The user was explicit about that ordering.

---

## 4. What v0.11.0 changed (all of it is intentional — preserve unless it is the cause)

### 4a. Bar text rendering — a real bug was fixed

The pre-v0.11.0 inversion used `percentFilled.Clip(fillClip)`. A XAML `Clip` rectangle is in **that
element's own coordinate space**, and the TextBlock was centered — so its origin sat mid-bar and the
clip landed in the wrong place entirely. What the user described as *"black with a subtle white 1px
background"* was the white base layer showing through a misaligned dark copy.

Fix: both text layers now live in **full-bleed wrapper Grids**, and the *wrapper* is clipped, so the
rectangle maps exactly onto the filled pixels. Plus `TextLineBounds::Tight` (drops the padded font
line box so glyphs center on whole pixels), `UseLayoutRounding` throughout, no opacity on text,
`OpticalMarginAlignment::TrimSideBearings`.

> Deliberately **not** used: explicit `LineHeight` + `LineStackingStrategy::BlockLineHeight`. They were
> tried and removed — with `Tight` bounds they risk clipping cap heights. Do not reintroduce.

Text color over the fill is computed at runtime: WCAG relative luminance of the current fill picks
near-black (`#111417`) or white, whichever has more contrast (crossover at L = 0.179). Works for
custom thresholds and colorblind mode. Track/chrome colors follow the system light/dark theme
(`IsSystemLightTheme()` reads `HKCU\…\Themes\Personalize\SystemUsesLightTheme`) and repaint on flip.

### 4b. Reset countdown + strip

Percent at the leading end of the bar, countdown (`4h 52m`) at the trailing end, both inside the bar.
Optional thin reset-progress strip stacked above each bar, driven by `resetUnixMs` + `durationSeconds`.
A 30 s `DispatcherTimer` per instance keeps the countdown live between polls without extra API calls.

### 4c. Sizing detangled

`barLength` / `barThickness` were **removed**. Primary, secondary, and reset bars each have
independent length + thickness. Per-account overrides via `"<length> <thickness>"` strings, plus a
per-account stacked/vertical layout override. Single resolution point: `ResolveBar()` — both
`BuildQuotaGrid` and `UpdateQuotaUi` call it, so sizes cannot drift apart. Bars too thin for legible
text drop it automatically (`fitFont` returns 0 below 7 px).

Defaults: primary 110×15, secondary 110×8, reset strip 3 px.

> **Migration note:** users' saved `barLength`/`barThickness` values do not carry over.

### 4d. Placement

Ported `ResolveInjectionTarget` from `taskbar-fluent-media-player` — 25 anchors (tray far left/right,
either side of clock / Network-Volume / language / tray icons / hidden-icons chevron / Show Desktop,
plus taskbar-side anchors on Start / Search / Task View / Widgets with `LayoutUpdated` margin
tracking, plus three overlay edge positions). Setting key: `position`, default `tray_left`.

Also hardened: column definitions are only removed when our own child is actually found. The old
fallback to a stale `quotaColumn` index could delete a column Explorer owns — a plausible cause of
the tray-icon overlap the user hit in an earlier session.

---

## 5. Architecture map (v0.11.0)

| Area | Notes |
| --- | --- |
| `Settings` / `AccountConfig` / `BarSpec` | struct block ~line 340–460 |
| `ResolvedBar` + `ResolveBar()` | single source of truth for bar geometry |
| `BarPalette` / `OnColorFor` / `RelativeLuminance` | theme + contrast helpers |
| `FormatCountdownCompact` / `ResetWindowProgress` | in-bar countdown + strip math |
| `BuildQuotaGrid()` | constructs the XAML subtree; tooltip/menu/pointer code inherited unchanged |
| `UpdateQuotaUi()` | diffed against `AppliedState` — only touches XAML on actual change |
| `ResolveInjectionTarget` / `ResolveAnchorElement` | placement resolution |
| `InjectQuotaGrid` / `EnsureQuotaGrid` / `IsQuotaUiLive` | injection + liveness |
| `RetryInjectThreadProc` | permanent watchdog |
| `HookTaskbarDllSymbols` | **the suspect for the current breakage** |

Data model is unchanged from upstream: `WindowUsage{pct, resetUnixMs, durationSeconds}` × 2 windows
(primary/5h, secondary/weekly) per account. A previous diagnostic run confirmed the user's **OpenAI
Free account returns a single 30-day window**, not 5h+weekly — the UI already collapses the
unavailable bar and labels windows from `durationSeconds` rather than hardcoding "5h".

---

## 6. Constraints and preferences

- **Concise, direct responses.** Minimal preamble, no padding.
- Do not suggest rolling back Windows.
- The user compiles in Windhawk and pastes error output back — expect iteration on compile errors.
  Historical trap: **C++/WinRT projected types must be `{nullptr}`-initialized** as struct members
  (`FrameworkElement x{nullptr}`, not `FrameworkElement x;`). This has bitten twice.
- Give **new filenames per revision** — the user has previously compiled a stale copy and reported a
  fixed error as still present.
- The user builds these mods themselves and reads the code. Explain *why*, not just *what*.

---

## 7. Static checks to run before delivering any build

No compiler available, so run these (they caught a real `Grid slot` vs `StackPanel` type mismatch
last time):

1. Brace/paren balance with a proper tokenizer that skips comments and strings. *Naive regex stripping
   fails* — `//` inside the readme's `/* … */` block eats the terminator.
2. Struct members of WinRT projected types lacking `{nullptr}`.
3. Settings-key parity both directions: every YAML key is read in `LoadSettings`, every key read
   exists in YAML. Remember keys are read via the `getIntSetting` / `getBoolSetting` /
   `getSettingText` / `getAccountText` lambdas, not only via raw `Wh_Get*Setting`.
4. No duplicate function definitions (forward declarations legitimately appear twice).
5. Parse the `==WindhawkModSettings==` block as YAML to confirm it is well-formed.

---

## 8. Suggested first message in the new session

> Priority 1: the AI quota bars mod stopped rendering after a Windows Insider update. Read
> `plan-handover.md` §2 first. Before proposing a fix, I need: (a) the Windhawk debug log across an
> Explorer restart with mod logging on, (b) my exact Insider build number, (c) whether
> `taskbar-fluent-media-player` still works right now.
