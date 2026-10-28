# Handover — Explorer view pair

Two mods tuned to work together in Explorer:

| Mod | Source here | Installed as | Upstream |
| --- | --- | --- | --- |
| Explorer Justified View | `mods/explorer-justified-view/` | `local@explorer-justified-view` | Own mod, derived from DarkionAvey's GPU view 0.9.263 (GPL-2.0) |
| Better file sizes in Explorer details | `mods/explorer-details-better-file-sizes/` | `local@explorer-details-better-file-sizes` | m417z, gallery 1.6 |

The GPU view is a **separate mod** since 2026-10-28 (plan:
`PLAN-explorer-justified-view.md`). The earlier fork, `local@gpu-accelerated-explorer-view`
(0.9.240 based), is disabled and archived as
`archive/gpu-accelerated-explorer-view-v0.9.240-fork.cpp`; DarkionAvey's gallery copy is
installed but disabled. Never enable two GPU views at once: this mod refuses Shell
views another GPU view already marks, but the original has no such guard. The sizes mod
is still a local fork of the gallery copy (also installed, disabled). Change history:
`CHANGELOG-explorer-justified-view.md` (before the split:
`CHANGELOG-gpu-accelerated-explorer-view.md`), `CHANGELOG-explorer-details-better-file-sizes.md`.
Open work: section 6.

Read sections 1–3 before editing. The GPU mod is ~30k lines in one file: grep
for the function names below, don't read it top to bottom.

## 1. Build, test and install

```powershell
.\scripts\check-mod.ps1 mods\explorer-justified-view\explorer-justified-view.wh.cpp -Warnings
.\scripts\build-mod.ps1 mods\explorer-justified-view\explorer-justified-view.wh.cpp
& "C:\Program Files\Windhawk\windhawk-cli.exe" mod install --file <path>   # elevated
```

- **Regression suite:** DarkionAvey's offscreen suite, run on our source. Clone his
  repo outside this one, then
  `pwsh -File scripts/run-upstream-suite.ps1 -Upstream <clone>` (~4 min, PASS lines,
  stops at the first FAIL; `-TestArguments --icon-sizing` etc. run one area). The
  script edits his clone where our product differs on purpose (new settings in
  `options-smoke.h`; our layout features off in `main()`; the 2× icon cap in
  `icon-sizing-smoke.h`) and forces the long `TEMP` path (with the 8.3 `LOST_H~1`
  form his `PlainFilesystemProvider` compare fails and the Office-lock test fails on
  his own release too). New intended differences → add them to the script's table,
  never by loosening an unrelated check. Known flake: `sort discarded a previously
  resolved property before the first paint` (his own 0.9.261 notes report it in older
  builds); it passed 6/6 alone (`background-regression.exe --smart-refresh` from the
  retained build folder) — rerun before suspecting a change.

- Run the install yourself with `Start-Process pwsh -Verb RunAs -Wait`; the user
  approves the UAC prompt. Don't hand them the command.
- Reinstalling a `local@` id keeps its settings. A **first** `--file` install
  creates the `local@` id with **default** settings: copy them from the gallery id
  (`mod settings get` both, `mod settings set local@...`), then disable the gallery id.
- Both mods load into `explorer.exe`; the sizes mod also loads into almost every
  process (`@include *`) so file dialogs get folder sizes.

## 2. How the two mods talk

- The sizes mod hooks `CFSFolder::_GetSize` in `windows.storage.dll` and fills
  `PKEY_Size` for folders (Everything IPC, or a manual walk).
- The GPU mod reads file details from `FindFirstFile` data, which has no folder
  size, so it used to leave folders blank and never reach the hook.
- **Handshake:** while folder sizes are enabled, the sizes mod creates the event
  `Local\WindhawkFolderSizeProvider_<pid>`. The GPU mod's detail worker checks it
  once per batch and, if present, asks the shell property store for folder `Size`
  (worker thread, visible rows only, max 8 per batch). No setting needed.
- GPU-drawn views are detected by the window property
  `WindhawkExplorerGpuOverlayOwnsShellView` on the native view or an ancestor; the
  sizes mod's column auto-fit skips those views (the GPU mod fits its own). In the
  GPU mod this is `kSharedOverlayMarker`, set/removed by `MarkShellView` /
  `UnmarkShellView` next to its private `kOverlayProperty`. Keep the name.

## 3. Map of the GPU mod (grep these)

| Area | Function / symbol |
| --- | --- |
| Detail column values (worker) | the `haveDetail` branch of the directory worker; `FormatShellDetail` |
| Which rows request details | `QueueVisibleDetails` |
| Auto column fit | `AutoRefitReady`, `MaybeRunAutoRefit`, `FitColumnsToVisibleRows`; setting `Columns.autoRefitColumns` |
| Row/sort/group/zoom animation curve | `LayoutTransitionProgress`, `EaseLayout`, `LayoutTransitionMs`; `g_photoMotion` (off = upstream's 180 ms ease-out and view-centre zoom anchor, used by the upstream suite) |
| Other UI animation timing | `kUiTransitionDurationMs` (180 ms, scroll/selection), `kHoverAnimationDurationMs` |
| Grid metrics | `UpdateMetrics` (icon/row size), `LayoutGroupSections` (columns per row) |
| Per-item geometry (draw + hit-test + rename) | `LayoutItem` → `ItemGeometry` |
| Tight photo grid | `PhotoGridIcon`, `PhotoGrid`, `ItemGeometry::photo`; setting `Appearance.photoGridBelow` |
| Justified grid | `JustifiedLayout`, `BuildJustifiedLayout`, `Justified()`, `VisitJustifiedLines`, `ItemWidthAt`, `SectionLineCount`, `RelayoutJustified`; aspects in `ViewState::imageAspects` (filled in `ReceiveIcons`); setting `Appearance.gridLayout` |
| Grid position math (both layouts) | `DocumentRowTop`, `DocumentRowLeft`, `RowAtDocument`, `VisibleItemRows`, `DocumentHeight` |
| Grid keyboard / marquee | `KeyboardNavigationTarget` (`justifiedLines` for Up/Down/PageUp/PageDown), `UpdateMarquee` |
| Icon/thumbnail drawing | `DrawActualIcon` → `DrawCachedIcon` (optional `fill` rect = cover-crop) |
| Icon extraction (worker) | `IconWorkerProc` area: factory for thumbnails only, then `SHGetImageList(SHIL_JUMBO)` → `DecodeShellIcon(fit)` → `FitShellIconPixels` (upstream 0.9.257) |
| Window properties / coexistence | `kOverlayProperty` & co. (derived from `WH_MOD_ID`), `kSharedOverlayMarker`, `ForeignGpuView` (checked before a new `ViewState` attaches) |
| Thumbnail fetch order | `LoadShellItemIcon` (cache-only probe → `E_PENDING`), `IconJob::cacheProbe` (set only by `RequestEntryIcon`; a default job extracts, as upstream), job pick in `IconWorkerProc` |
| HEIF/RAW fast path | `HasEmbeddedPreview`, `LoadEmbeddedPreview` (WIC frame thumbnail + orientation) |
| Thumbnail preloading | `SchedulePreload` (called when names complete), `ThumbnailPreloadProc`, `StopPreload` (both uninit hooks); setting `Appearance.preloadThumbnails` |
| Zoom (Ctrl+wheel / pinch) | `ApplyRowHeight` (grid lower bound 8, list 24) |
| View-mode shortcuts (Ctrl+Shift+1..8) | `NativeViewInput::Owner` — passes through to Explorer |

## 4. Decisions and why

- **Everything connection is reused** (sizes mod, `g_everything3Client` under
  `g_everything3ClientMutex`). Connecting a pipe per folder was the main cause of
  slow opening of folders with many subfolders. A failed query whose
  `GetLastError()` isn't `EVERYTHING3_OK` drops the client and reconnects once.
- **Size cache stays `thread_local`** (upstream design). Explorer's threads and the
  GPU worker each keep their own, so a folder can be queried twice; cheap with the
  reused connection. Make it shared only if profiling says so.
- **Native auto-fit uses `IColumnManager::SetColumnInfo(CM_WIDTH_AUTOSIZE)`** on the
  view's UI thread. The shell browser comes from undocumented `WM_USER+7`
  (CWM_GETISHELLBROWSER), only asked of `ShellTabWindowClass` and `#32770` hosts,
  and validated by `IShellView::GetWindow() == the DefView`. The "Size all columns
  to fit" menu command has no static ID (built at runtime), so it can't be posted.
- **Fit only views showing a folder whose sizes just arrived** (`g_autoFitFolders`,
  last 8), debounced 300 ms on the UI thread, so manual widths in other windows
  aren't overridden.
- **Sort animation:** critically damped spring, 300 ms, ~90 % at 130 ms, no
  overshoot. One curve for every row-layout move so sort/group/refresh feel alike.
  Upstream was 180 ms ease-out-cubic.
- **Photo grid is a layout mode, not a new view.** Uniform cells keep hit-testing,
  marquee, keyboard navigation and animations working. Thumbnails cover-crop; plain
  icons stay fit-centered (max 2× upscale). Selection = accent tint + 3 DIP ring,
  because the tile hides the normal selection background.
- **Justified grid keeps a fixed line height.** Google Photos varies row height;
  here every line is `rowPx` so vertical math, groups, scrolling and zoom stay
  untouched, and the image box cover-crops (row breaks pick the height nearest
  the target, so crops stay small). Positions are fractions of the content width
  inside an immutable `shared_ptr` table, copied to the compositor's
  presentation state via `PRESENTATION_MODEL_FIELDS`; only the owner rebuilds it.
  `itemsPerRow` is set to the widest line so viewport estimates over-cover.
  The table is valid while its size equals `total`; every reorder goes through
  `RebuildGroupSections` → `LayoutGroupSections`, which rebuilds it.
- **Aspect ratios come from delivered thumbnails**, not a property query per
  file, so opening a folder costs nothing extra. Revisiting a folder lays out
  correctly at once (the per-tab map persists; cleared past 50k).
- **Upstream intake = three-way merge.** Base = DarkionAvey's `latest.cpp` at the
  commit last synced (now `e543816`, 0.9.263; recorded in the changelog), ours =
  our file, theirs = his new `latest.cpp`; normalize all three to LF, then
  `git merge-file -L ours -L base -L theirs ours.cpp base.cpp theirs.cpp`. The
  0.9.240 → 0.9.263 sync had 2 conflicts, both in `IconWorkerProc`. Keep our
  additions in contiguous blocks so this stays cheap.
- **Window properties are private to the mod id.** `State()` trusts whatever HWND
  a property names (it only checks the thread), so sharing upstream's names with a
  second GPU-view mod in the same Explorer would cast its windows to our
  `ViewState`. Only `kSharedOverlayMarker` keeps the public name.
- **Justified per-folder memory (Phase 2) goes in Explorer's own view-state bags**,
  bag `WindhawkExplorerJustifiedView`, via `SHGetViewStatePropertyBag`
  (`SHGVSPB_FOLDERNODEFAULTS`). It is exported by **shlwapi.dll**, not shell32, and
  missing from the mingw headers: resolve it with `GetProcAddress`. Probed
  (`scripts/probes/probe-view-mode.cpp`): persists across processes, per folder,
  works for ZIP folders, a `VT_EMPTY` write clears it. Rejected: `FVM_THUMBNAIL`
  (DefView stores it as `FVM_ICON`) and an odd icon size (native Ctrl+wheel itself
  produces odd sizes: 27, 33, 97, 133…).
- **View submenu items have stable verbs**: `viewlogicaliconsextralarge`,
  `…large`, `…medium`, `…small`, `viewlogicallist`, `…details`, `…tiles`,
  `…content`. Find the submenu by verb, never by its localized text.

## 5. Known gaps and gotchas

- Sorting by Size in the GPU view treats folders as 0 bytes (would need every
  folder's size up front, exactly the slowness removed). Explorer's
  `folderPlacement` setting decides where folders go.
- Folder sizes don't appear in search results (the size is keyed under the item's
  real parent; the view's folder is the search).
- Row-detail cache is validated by folder metadata; a deep content change can
  show a stale folder size until refresh.
- Ctrl+wheel in the GPU view zooms item size by design; it does not cycle views
  like native Explorer.
- The GPU mod is `@include explorer.exe` only, so file dialogs (e.g. "Add folder"
  in other apps) show the native view. Folder sizes and auto-fit do work there.
- GPU mod is x86-64 only (`@architecture x86-64`).

## 6. Open work

0. **Phase 2 of `PLAN-explorer-justified-view.md`:** Justified as its own view
   (right-click View menu, Ctrl+Shift+9, status-bar button), per-folder memory in
   the view-state bag, Large icons back to a uniform grid, `Appearance.gridLayout`
   removed. Then 1.0.0.
1. **Thumbnail speed: verify on device.** Cache-first probing, preloading and
   the HEIF/RAW embedded-preview path are in (measurements in the changelog).
   Remaining slow cases: video (MOV/MP4 0.2–0.5 s each, Media Foundation,
   scales badly in parallel) and HEIF/RAW tiles bigger than the embedded preview
   (falls back to the ~0.8 s Shell decode). A progressive path (show the preview
   at once, swap in the sharp render later) would fix the latter; it needs two
   results per job.
   Bench scripts used: `thumbbench.ps1`, `wicbench.ps1`, `orient.ps1` (session
   scratchpad, not kept; easy to rewrite from the changelog numbers).
2. **EXIF popup on hover** (user request): camera, lens, exposure, date, size for
   photos and videos. Read via the property store on a worker, never on hover paint.
3. Verify the justified grid on device: reflow smoothness while thumbnails
   stream in, groups, RTL, rename box position, drag image.
4. **GPU view in file dialogs.** Would mean widening `@include` and handling
   `CFileOpenSave` hosts. Risky (every process); scope before starting.
5. Verify on device: photo grid look at 125/150 % DPI, rename in photo grid
   (edit box sits on the tile's bottom edge), drag image, RTL.
6. The sizes mod still uses the gallery id (`explorer-details-better-file-sizes`);
   rename it to a `-fork` id if gallery updates ever get confused with it. Its
   settings block also fails `scripts/check-settings.py` (numeric `$options` keys,
   inherited from the original).
