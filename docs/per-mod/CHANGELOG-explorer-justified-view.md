# Changelog — explorer-justified-view

Separate mod derived from DarkionAvey's
[GPU-accelerated explorer.exe list and grid views](https://github.com/DarkionAvey/windhawk-gpu-accelerated-explorer-view)
(GPL-2.0). Upstream base: **0.9.263**, commit `e543816` (2026-10-28). Newest first.
Context: `HANDOVER-explorer-view-pair.md`; plan: `PLAN-explorer-justified-view.md`.
History before the split: `CHANGELOG-gpu-accelerated-explorer-view.md`.

## 0.1.0 — 2026-10-28

The local fork (0.9.240 + justified grid, photo grid, thumbnail pipeline, folder
sizes) rebased onto upstream 0.9.263 by a three-way merge. No new features.

- **From upstream 0.9.241–0.9.263:** refresh keeps values and refreshes a tab or
  window on return; ZIP, network and delegated Shell folders; Ctrl+V with frame
  focus; event-driven theme/colour/font refresh; selection on the existing worker;
  image-list icons cropped to their artwork; dense-grid drawing and compositor
  speedups. See upstream's `CHANGELOG.md`.
- **Icons:** upstream's sourcing (real HICONs, `FitShellIconPixels`) replaces
  `TrimJumboPadding`; the cache-first thumbnail probe and its requeue stay; small
  icons keep the 2× upscale cap.
- **Separate identity:** id, name, author, readme. Every window property name is
  now derived from the mod id, so this mod and DarkionAvey's can never read each
  other's window state in one Explorer. The shared marker Better file sizes reads
  (`WindhawkExplorerGpuOverlayOwnsShellView`) is still set; a Shell view already
  carrying it from another GPU-view mod is left alone.
- **Fix:** Page Up / Page Down in justified rows stepped by the widest line's item
  count and overshot; they now move by lines, keeping the horizontal position.
- **Fix:** the DNG retry at 768 px ran for every file type without a thumbnail;
  now only for types with embedded previews.
- **Change:** no thumbnail preloading for folders on network paths.
- **Change (internal):** the cache-first flag is now opt-in (`IconJob::cacheProbe`,
  set only for viewport thumbnail requests); any other job extracts, as upstream.
  Found by upstream's icon-alpha test, which calls `LoadShellItemIcon` directly.
- **Change (internal):** the fork's spring timing and pointer-anchored zoom sit
  behind one switch, `g_photoMotion` (on; off = upstream's 180 ms ease-out and
  view-centre anchor). No behaviour change.
- **Testing:** `scripts/run-upstream-suite.ps1` runs DarkionAvey's suite on this
  source with our layout features and Photos-style motion off, so his tests check
  his paths: **131/131 pass**, same as his 0.9.263 here.
- Upstream's `PageLength` unused-constant warning silenced (CI fails on warnings).
