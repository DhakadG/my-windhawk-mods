# Changelog — gpu-accelerated-explorer-view (local fork)

Base: gallery 0.9.240 by DarkionAvey. Version string not bumped yet; changes are
listed here newest first. Context: `HANDOVER-explorer-view-pair.md`.

## 2026-10-27

- **Dense photo grids load every tile and keep them.** Each icon was charged a
  full 64 KB GPU page, so a tiny-tile grid (~1,700 visible) overflowed the image
  budget after a few hundred: the rest stayed on the type icon and loaded ones
  were evicted to make room. GPU cost now follows WDDM paging (4 KB pages below
  64 KB, `GpuTextureBytes`), and `ImageMemoryBudget` always fits the visible
  tiles plus overscan at a 48 px tier (capped at 4x the RAM allowance).
- **Thumbnails survive `SHCNE_UPDATEIMAGE`.** That broadcast (sync-client
  overlays fire it often) cleared the whole icon cache, blanking photo folders
  and reloading them tile by tile. `InvalidateShellIcons` now keeps thumbnails.
- **Thumbnails fade in.** A thumbnail's first upload crossfades over what the
  tile showed before (a smaller size of the same image, else its type icon) on
  the 180 ms UI curve (`IconRecord::revealQpc`).
- **HEIF and camera RAW thumbnails from the file's embedded preview.**
  Measured on the user's trip folder at 384 px (Shell `GetImage`, cold cache):
  JPEG 65 ms, MP4 365 ms, MOV 197 ms, HEIC 837 ms — and HEIC got no faster on 8
  threads (916 ms/file), so more workers can't fix it. WIC reads the preview
  stored in the file instead: HEIC 416x312 in 1–20 ms, NEF 1024x682 in 1–2 ms.
  Extractions for HEIF/AVIF/RAW (`HasEmbeddedPreview`) now try
  `LoadEmbeddedPreview` first and use it when it covers the tile (>= 3/4 of the
  requested size) and has the photo's own shape (no letterbox), rotated per
  `System.Photo.Orientation` (NEF portraits report 8 with a landscape preview;
  HEIF previews arrive upright). Otherwise the Shell renders it as before.
  JPEG stays on the Shell path (fast; its previews are tiny and letterboxed).
  The preloader skips these types, since warming the Shell cache for them would
  cost ~0.8 s each for nothing.
- **RAW thumbnails that never loaded (DNG).** Some RAW handlers have no small
  preview: `IMG_4278.DNG` fails at 256 px (`0x80070490`) yet renders at 768 px
  (NEF works at both). A failed thumbnail extraction below 768 px now retries at
  768 and scales down.
- **Blurry thumbnails.** (1) The cache-only probe accepted whatever tier the
  cache held, including the ~160 px letterboxed preview cameras embed in JPEGs;
  results smaller than 3/4 of the requested size are now treated as a miss and
  rendered properly. (2) Cover-cropped tiles (justified, photo grid) request
  1.5x the tile size, since a 3:2 photo fills its box with its short side.
- **Zoom keeps the photo under the pointer still** (Photos style). The anchor
  is the item under the cursor (or the view's center) and the exact point
  within it, instead of a fractional row count that was wrong for justified
  rows. Zoom, sort and reflow now share one clock and curve (300 ms critically
  damped spring); zoom used 180 ms ease-out-cubic while items moved on the
  spring, which looked disjointed.
- **Smoother reflow while thumbnails stream in.** Justified relayout waits for a
  burst of new aspect ratios to go quiet (180 ms, at most 700 ms) instead of
  restarting the spring from rest every 150 ms; changes that only affect rows
  below the screen reflow silently, with the top visible item kept in place.
  Frame pacing was already native: a DWM-clock pump (`DwmFlush`) drives frames at
  the monitor's own rate.
- **Fix:** in the justified grid the filename selection pill and the rename box
  were measured against the uniform cell width, so they sat off-center under
  wide thumbnails. `SelectionRect` and `RenameEditorRect` now use `ItemWidthAt`.
- **Cache-first thumbnails.** Every thumbnail is first requested from the
  Windows thumbnail cache only (`SIIGBF_INCACHEONLY`, about 1 ms on a hit). A miss
  requeues the job as an extraction (`IconJob::extract`), and provider workers
  always take cache probes before extractions, so slow RAW/video decodes never
  hold up thumbnails that are already cached.
- **Thumbnail preloading**, setting `Appearance.preloadThumbnails` (default on).
  When a folder finishes loading, two background-priority threads ask
  `IThumbnailCache` to extract the rest of the folder into the Windows cache
  (no pixels kept), starting just past the viewport and fanning out both ways.
  A new folder cancels the old one between items; unload waits for the current
  item. RAW and video need no code of their own: this PC has the Raw Image,
  HEIF, HEVC, AV1, VP9 and Web Media extensions, and every tested RAW type
  (.cr2 .cr3 .nef .arw .dng .raf .orf .rw2) and video type (.mp4 .mov .mkv .mts
  .avi) already has a Shell thumbnail handler.
- **Justified grid (Google Photos style)**, new setting `Appearance.gridLayout`
  (default `justified`, `uniform` restores equal cells). Rows keep the grid's row
  height; each item's width follows its thumbnail aspect ratio (clamped
  0.45–2.8, unknown = square) and full rows fill the width. Thumbnails
  cover-crop into the box, names stay underneath. Aspect ratios are learned as
  thumbnails arrive; the grid reflows at most every 150 ms, keeps the top visible
  item still, and animates the rest on the layout spring. Keyboard up/down picks
  the nearest item on the next line; marquee tests each item's own x-span.
  Algorithm ported from husky-drop `computeJustifiedRows`.
- **Tight photo grid.** Zooming a grid below `Appearance.photoGridBelow` (default
  64 px at 100 %) switches to an edge-to-edge thumbnail grid: 2 DIP gutters, no
  names, cover-cropped thumbnails, accent tint + ring for selection. Grid zoom-out
  lower bound lowered from 24 to 8 so Large-icon grids can reach it.
- **Loading placeholders centered.** Types without 256 px icon art came back from
  `SHIL_JUMBO` as a small icon in the corner of a transparent 256 canvas and drew
  pinned to the tile's top-left. `TrimJumboPadding` crops them; plain icons are
  capped at 2× upscale so they stay crisp and centered.
- **Spring layout animation.** Sort, group, refresh reorder and view-mode morphs
  use a critically damped spring over 300 ms (was 180 ms ease-out-cubic).
- **Folder sizes.** When `Local\WindhawkFolderSizeProvider_<pid>` exists (set by
  Better file sizes), folder rows ask the shell for `Size` on the detail worker
  instead of staying blank. Existing auto-fit waits for these values.
