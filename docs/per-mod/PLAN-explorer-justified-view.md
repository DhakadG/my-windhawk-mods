# Plan — new mod: GPU Explorer view with a Justified view

Status: **Phases 0 and 1 done (0.1.0); Phase 2's Justified view in (0.2.0, 2026-10-28);
device check, zoom-out without names and the smoke test left before 1.0.0.** Results: §10.
Id `explorer-justified-view` (decisions in §9). Written 2026-10-28 after reading every file in
[DarkionAvey/windhawk-gpu-accelerated-explorer-view](https://github.com/DarkionAvey/windhawk-gpu-accelerated-explorer-view)
at commit `e543816` (0.9.263) and the local fork in `mods/gpu-accelerated-explorer-view/`.

## 1. Goal

A mod of our own, separate from DarkionAvey's `gpu-accelerated-explorer-view`, that:

- keeps everything the GPU view does (Details, List, icon grids, This PC, animations,
  smooth scrolling) **at upstream's latest quality (0.9.263)**;
- adds **Justified** as its own view **next to** Explorer's eight views (Extra large …
  Content), instead of replacing the uniform icon grid;
- carries every feature of the local fork (thumbnail pipeline, photo grid, zoom anchor,
  spring motion, folder sizes);
- keeps working with Better file sizes (folder sizes) and with upstream's future fixes.

## 2. What exists today

### 2.1 The local fork (`mods/gpu-accelerated-explorer-view/`)

Gallery 0.9.240 plus +824/−77 lines, id and version unchanged, never committed. The
repo copy equals the installed `local@gpu-accelerated-explorer-view` (only line endings
differ). Features, by function:

| Feature | Where |
| --- | --- |
| Justified grid (replaces uniform grid when `Appearance.gridLayout = justified`) | `JustifiedLayout`, `BuildJustifiedLayout`, `VisitJustifiedLines`, `ItemWidthAt`, `SectionLineCount`, `RelayoutJustified`, hit test / marquee / Up-Down keys |
| Aspect ratios learned from delivered thumbnails, debounced reflow (180 ms quiet, 700 ms cap) | `ReceiveIcons`, `kJustifiedRelayoutTimerId` |
| Tight photo grid below `Appearance.photoGridBelow` (64 px) | `PhotoGridIcon`, `PhotoGrid`, `ItemGeometry::photo`, accent tint + ring selection |
| Cover-crop drawing, 2× cap on plain icons, thumbnail fade-in | `DrawCachedIcon(fill)`, `IconRecord::revealQpc` |
| Cache-first thumbnails (`SIIGBF_INCACHEONLY`), miss requeued as extraction | `LoadShellItemIcon`, `IconJob::extract`, `IconWorkerProc` |
| HEIF/RAW embedded previews via WIC, EXIF rotation; DNG retry at 768 px | `HasEmbeddedPreview`, `LoadEmbeddedPreview` |
| Thumbnail preloading into the Windows cache (2 background threads) | `SchedulePreload`, `ThumbnailPreloadProc`, `StopPreload` |
| Dense-grid GPU budget (WDDM 4 KB pages) | `GpuTextureBytes`, `ImageMemoryBudget`, `ImageDemandSlots` |
| Thumbnails survive `SHCNE_UPDATEIMAGE` | `InvalidateShellIcons` |
| Zoom anchored on the item under the pointer; 300 ms critically damped spring for zoom/sort/group | `ApplyRowHeight`, `ZoomScrollTarget`, `EaseLayoutSpring` |
| Folder sizes via the Better file sizes handshake | `DirectoryWorkerProc` (`Local\WindhawkFolderSizeProvider_<pid>`) |
| Centered small icons in jumbo slots | `TrimJumboPadding` |

### 2.2 Upstream since our base (0.9.240 → 0.9.263, 22–28 Oct)

23 releases in four days, +1183/−498 lines. 0.9.246 was released, reverted to 0.9.245,
then redone; 0.9.256 was withdrawn. Grouped by theme:

| Theme | Versions | What changed |
| --- | --- | --- |
| Refresh keeps values | 242, 262, 263 | F5 / menu Refresh keep old metadata until new values arrive; background tab and window refresh on return (`kMsgTabResumeRefresh`) with no repaint when nothing changed |
| This PC storage bars | 243, 244 | Bars aligned across rows, shared label band, no jump after animations |
| Scrollbar | 245 | Rapid track clicks then thumb drag |
| Shell providers | 246–248 | ZIP, network places, delegated folders: enumerates the bound provider, opaque `windhawk:pidl:` row keys, selection by provider identity, native default action |
| Clipboard keys | 249, 250 | Ctrl+V with frame focus, one command per keypress |
| Theme/colors/fonts | 251–254 | Event-driven style refresh (no 2.5 s polling), validated colors/fonts/DPI, polling audit |
| Selection | 255 | Small selections resolved on the selection worker |
| Icon sourcing | 257 | Real HICONs from the image list cropped to their alpha bounds (`FitShellIconPixels`), image factory only for genuine thumbnails, small icons **magnified to fill** the slot |
| Dense grids | 258–261 | Label/glyph cache reuse, SSE2 identity compares, shared DComp curves (commit 187 ms → 2.6 ms in his fixture) |

Settings block: unchanged since 0.9.240. Licence: **GPL-2.0** (`@license GPL-2.0`).

### 2.3 The rest of his repository

- `latest.cpp` = `gpu-accelerated-explorer-view.wh.cpp` = 0.9.263; `previous-versions/`
  keeps the 15 newest; `releases/0.9.240.cpp` = our base.
- `docs/` — one investigation per release (all read; summarized in the table above).
- `tests/` — offscreen regression suite: `background-regression.cpp` includes the mod
  source (`GPU_OVERLAY_SOURCE`) plus ~60 smoke headers (grid, icons, selection,
  providers, RTL, unload…), `run-regressions.ps1 -SourceFile <any path>`, DLL-unload
  checks, an 8 KiB stack budget for message/COM callbacks. Older audits (lifetime,
  RTL, limits, trace findings 0.9.144–0.9.195) document past crash/stall classes.
- `scripts/` + `build.bat` — numbered working versions, a release gate that only
  publishes after the full suite passes, README embedded from `README.md`.
- `AGENTS.md` — his engineering rules (event-driven, per-tab ownership, no guessed
  timers, preserve RTL/DPI/focus, never show both native and GPU lists).

### 2.4 Fork vs upstream: how they combine

A trial three-way merge (base 0.9.240, ours = fork, theirs = 0.9.263) gives **2
textual conflicts, both in `IconWorkerProc`**. Resolution: take upstream's structure
(thumbnail-only factory call, HICON + `FitShellIconPixels`) and keep our `E_PENDING`
requeue. `TrimJumboPadding` is superseded and goes.

Semantic overlaps the merge does not flag:

| Overlap | Action |
| --- | --- |
| Small icons: fork caps at 2×, upstream now magnifies to fill | Decision §9 |
| Provider rows (ZIP, network) now reach the grid | Preload and embedded preview must skip `windhawk:pidl:` keys (already) **and network paths** (new) |
| Tab-resume refresh re-runs `ApplyDirectoryNames` | `SchedulePreload` would restart on every tab switch: schedule only when the names changed |
| `detailsRefreshEpoch` refresh rules | Re-test folder Size cells across F5 / tab resume |
| Text layout cache keyed by text, resized per width | Justified reflow animates widths every frame: check for label re-rasterizing; if so, lay labels out at the destination width during transitions |
| PageUp/PageDown use `VisibleRows × itemsPerRow` | In justified `itemsPerRow` is the widest line, so pages overshoot: move by lines instead (fork bug) |
| Shared DComp curves (0.9.261) | Per-item widths flow through `LayoutRowVisual`; verify the row scene still animates justified moves |

## 3. Product definition

### 3.1 Justified view (the MVP)

- A ninth view. Explorer's own eight keep their own look: Large icons is a uniform
  grid again, as in Explorer. Justified rows: each item's width follows its thumbnail
  shape, rows fill the width, row height = the view's icon size, names underneath.
- **Turning it on**: *View → Justified* in the background right-click
  menu (the GPU view builds that menu itself, so adding a radio item is safe),
  **Ctrl+Shift+9** (unused by Explorer; 1–8 are its views), and a button next to
  Explorer's Details / Large icons status-bar buttons, where the mod's own toggle
  already sits.
- **Turning it off**: choosing any Explorer view, from any place.
- **Per-folder memory**: like Explorer's own views. Preferred mechanism: set the native
  view to `FVM_THUMBNAIL` and treat that mode as "Justified" — Explorer then stores it
  per folder in its own view bags, Explorer's other views switch away from it, and
  with the GPU view off the folder still shows a sensible thumbnail grid. **Must be
  probed first** (§5 phase 0). Fallback: a bounded per-folder list in mod storage.
- **Zoom**: Ctrl+wheel / pinch changes row height, anchored on the photo under the
  pointer (already in the fork). Zooming far out keeps justified rows but drops names
  and gutters (photo-library look) instead of jumping to square tiles.
- **Switching** between Large icons and Justified animates items to their new places
  on the existing layout spring.

### 3.2 Everything else carried over

All fork features in §2.1, on top of 0.9.263. The square tight photo grid stays for
Explorer's icon views. `Appearance.gridLayout` goes away (Justified is now a view).

### 3.3 File and folder sizes

Folder sizes keep coming from Better file sizes through the existing handshake
(decision §9). Improvements on our side, later phase: sort by Size including folders
when Everything answers, sizes in search results, a stale folder size refreshing after
deep changes.

### 3.4 Identity

- New `@id`, `@name`, `@author` lost_husky, `@github` DhakadG, version `0.1.0` for the
  rebased base, `1.0.0` when the Justified view is verified on device.
- `@license GPL-2.0` and a readme line crediting DarkionAvey's mod (required by the
  licence; the repo's MIT licence does not cover this file — say so in README).
- Window class names derive from `WH_MOD_ID`, so they change automatically.
- **Coexistence guard**: if another GPU view (the gallery original) already owns a
  Shell view (`WindhawkExplorerGpuOverlayOwnsShellView` set by a window that isn't
  ours), stand down for that view and log it. Keep that property name: Better file
  sizes uses it to skip GPU views.

## 4. Architecture decisions

1. **Base = upstream 0.9.263 by three-way merge**, not a hand port. Our delta is small
   and fenced; theirs is broad.
2. **Future upstream intake** = the same merge. Record the upstream commit each sync
   used (start: `e543816`); `git merge-file ours base theirs` with base = his
   `latest.cpp` at that commit. Keep our additions in contiguous blocks with short
   section comments so later merges stay small.
3. **View state**: one per-tab `layout` value (`Native` / `Justified`), derived from the
   native mode (or the mod-storage fallback), never a global setting. Justified is
   only valid over an icon mode; Details/List/Tiles/Content always win.
4. **Self-initiated mode changes** (`SetViewModeAndIconSize` when entering Justified)
   are tagged so `NativeViewInput` does not treat them as the user leaving Justified.
5. **No new timers** beyond the existing justified debounce (an interaction policy, as
   upstream's audit allows). Preload stops using `Sleep(400)` and starts when the
   visible icon queue drains.

## 5. Phases

Each phase ends compiled with `scripts/check-mod.ps1 -Warnings`,
`scripts/check-settings.py`, upstream's suite (`run-regressions.ps1 -SourceFile`), and
installed as `local@`.

**Phase 0 — groundwork (no product code)**
1. Run upstream's suite on his `latest.cpp` locally: a known-good baseline.
2. Probe `FVM_THUMBNAIL`: does DefView accept it, report it back, persist it per
   folder, and render it natively? Decides §3.1 per-folder memory.
3. Probe the background menu's View submenu (item ids, radio group) to place
   *Justified*.

**Phase 1 — the new mod, rebased (no new features)**
1. Merge, resolve the two conflicts, rename id/name/author/readme, licence line.
2. Fix what §2.4 lists (network-path skip, preload scheduling, PageUp/PageDown).
3. Run upstream's suite on our source; triage failures (expected: animation timing
   tests, since layout moves are 300 ms spring instead of 180 ms).
4. Repo: `mods/<id>/`, README row + licence note, `CHANGELOG-<id>.md`, handover
   (rework `HANDOVER-explorer-view-pair.md` around the new id), TODO pointer,
   `sync-from-modssource.ps1` mapping (also for the sizes mod), move the old fork to
   `archive/`. CI picks the new folder up by itself.
5. Install `local@<id>`, disable `local@gpu-accelerated-explorer-view`.

**Phase 2 — Justified as its own view (MVP → 1.0.0)**
Entry points, per-folder memory, uniform icon views restored, switch animation,
zoom-out without names, label-cache check. One offscreen smoke test in his style for
`BuildJustifiedLayout` (line breaks, full rows fill the width, groups restart lines,
hit test round-trips). Device check: groups, RTL, rename box, drag image, 125/150 %.

**Phase 3 — sizes improvements** (§3.3).

**Phase 4 — ideas** (§6), one at a time.

## 6. Ideas, ranked

| # | Idea | Why | Size |
| --- | --- | --- | --- |
| 1 | Video tiles: play glyph + duration badge | Photos-style; duration via the detail worker, only in thumbnail views | M |
| 2 | Names on hover in Justified (setting: always / on hover / never) | Clean photo wall, names when needed | M |
| 3 | Progressive thumbnails: embedded preview first, sharp render after | Open item from the handover; fixes big HEIF/RAW tiles | M |
| 4 | EXIF hover card (camera, lens, exposure, date) | Asked for earlier; property store on a worker | M |
| 5 | Relative size bars in the Size column | Reuse upstream's inline storage-bar drawing; WizTree-like at a glance | S–M |
| 6 | Auto-Justified for picture/video folders without a saved choice | Pictures open right by default | S |
| 7 | Date label while dragging the scrollbar in date-sorted photo folders | Photos-style scrubbing | M |
| 8 | Selection check circles on photo tiles | Easier multi-select with the mouse | M |
| 9 | GPU view in file dialogs | Old open item; touches every process — scope first | L |

## 7. Worth adopting from DarkionAvey's way of working

- His rules in `AGENTS.md`: work wired to events, not guessed timers; per-tab state;
  slow Shell calls off the UI thread; RTL/DPI/focus preserved.
- His offscreen suite as our gate before every install, plus our own justified test.
- `PipelineScope` spans for our new paths (justified build, cache probe vs extraction,
  preload) so a log shows where time went.
- Short, user-facing changelog entries per version; investigations in separate docs.

## 8. Risks

- `FVM_THUMBNAIL` may be normalized or not persisted → mod-storage fallback (more code).
- One 30k-line file: merges get harder as our delta grows → fenced blocks, small delta.
- Two GPU views in one Explorer if the gallery original gets enabled → guard (§3.4).
- Upstream ships several releases a day; intake is a merge per release we want, not
  continuous tracking.
- XAML command-bar View dropdown cannot easily get a *Justified* entry; out of scope.

## 9. Decisions (2026-10-28)

1. Id `explorer-justified-view`, name "Explorer Justified View".
2. Justified is selectable from the background right-click View menu, Ctrl+Shift+9 and
   a status-bar button. Not from the XAML command-bar dropdown.
3. Folder sizes stay with Better file sizes (handshake); this mod adds sorting, search
   results and refresh on its side later.
4. Small icons in large tiles keep the fork's 2× cap; upstream's icon sourcing
   (`FitShellIconPixels`) is adopted.

## 10. Results of phases 0 and 1 (2026-10-28)

- **Baseline:** DarkionAvey's suite passes on his 0.9.263 here (131 PASS, ~4 min) —
  but only with `TEMP` set to the long path; with the 8.3 `LOST_H~1` path his
  `PlainFilesystemProvider` compare fails and the Office-lock test fails.
- **Per-folder memory (§3.1) changed:** `FVM_THUMBNAIL` is stored as `FVM_ICON`, so it
  cannot mark Justified. An odd icon size can't either: native Ctrl+wheel produces
  odd sizes. Explorer's view-state bags can: `SHGetViewStatePropertyBag` (shlwapi
  export) with a private bag name persists per folder, across processes, for ZIP
  folders too. That replaces the mod-storage fallback.
- **View submenu:** items carry verbs `viewlogical…`; the background menu is the
  classic `IContextMenu` HMENU the GPU view already tracks itself.
- **Phase 1:** merged, 2 conflicts resolved, renamed, fixes from §2.4 done (Page
  Up/Down, network preload, DNG retry scope); unchanged-refresh preload restarts were
  already prevented by upstream's fast path. Found and fixed a coexistence hazard
  beyond §3.4: all window property names were shared with the original mod while
  `State()` trusts any HWND they name; now derived from the mod id. Details:
  `CHANGELOG-explorer-justified-view.md`.

## 11. Phase 2 so far (0.2.0, 2026-10-28)

- **Done:** Justified per tab (`ViewState::justifiedView`) instead of a global
  setting; entry points (Ctrl+Shift+9, View menu item inserted on
  `WM_INITMENUPOPUP` after the verbs, status-bar button); exits (Explorer view
  verbs, Ctrl+Shift+1–8, native status buttons, any input that lands in a
  non-icon mode); per-folder bag; Large icons uniform again; animated switch;
  `Appearance.gridLayout` removed. Upstream suite: see the changelog.
- **No self-initiated-mode tagging needed (§4.4):** Justified is left only by
  explicit view choices or by a user change to a non-icon mode, so our own
  `SetViewModeAndIconSize(FVM_ICON, 96)` can't end it.
- **Left for 1.0.0:** device check (groups, RTL, rename box, drag image, 125/150 %,
  the button's place and tooltip, menu item in other languages); zoom-out
  keeping justified rows without names (still square tiles); label-cache
  check during the switch; the `BuildJustifiedLayout` smoke test.
