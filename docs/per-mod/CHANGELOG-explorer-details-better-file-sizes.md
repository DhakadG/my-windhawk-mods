# Changelog — explorer-details-better-file-sizes (local fork)

Base: gallery 1.6 by m417z. Version string not bumped yet; newest first.
Context: `HANDOVER-explorer-view-pair.md`.

## 2026-10-27

- **Faster Everything lookups.** One SDK3 pipe connection is reused for all
  folder queries (was connect + destroy per folder). Reconnects once on a stale
  pipe; falls back to the 1.4 window IPC as before.
- **New setting `autoFitColumns`** (Disabled / Size column (default) / All
  columns): after folder sizes load, fit columns like double-clicking a divider.
  Runs on the view's UI thread via `IColumnManager`, debounced 300 ms, only for
  views showing the folder that got sizes, skipping GPU-drawn views.
- **Provider signal** `Local\WindhawkFolderSizeProvider_<pid>` while folder sizes
  are enabled, read by the GPU view mod.
- Added `-lcomctl32` (window subclassing).
