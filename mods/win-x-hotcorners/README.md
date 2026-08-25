# Win-X Hot Corners

macOS-style hot corners **and screen edges** for Windows 10 and 11, with full
multi-monitor support. Built as a [Windhawk](https://windhawk.net) mod.

**Version 1.3.0 — published.** Live in the catalogue at
[windhawk.net/mods/win-x-hotcorners](https://windhawk.net/mods/win-x-hotcorners),
accepted via
[ramensoftware/windhawk-mods#5001](https://github.com/ramensoftware/windhawk-mods/pull/5001)
on 22 Aug 2026.

## Where the documentation lives

The full readme — every action, the argument formats, the five trigger styles,
multi-monitor resolution and the reasoning behind the design — is the
`==WindhawkModReadme==` block at the top of
[`win-x-hotcorners.wh.cpp`](win-x-hotcorners.wh.cpp). That block **is** the mod
page Windhawk renders, so it is the single source of truth and the only copy
that is guaranteed current.

This file used to be a second copy of it. It drifted: it went on describing the
4.1.x design, in which there was no settings page and the tray icon held the
configuration. Since 1.0.0 the **Windhawk settings page** is where zones are
configured and the tray dashboard is a read-only view of the result. Rather than
keep a third copy in sync, this file now just points at the real one.

| Where | What it is |
| --- | --- |
| [`win-x-hotcorners.wh.cpp`](win-x-hotcorners.wh.cpp) | Source, and the readme Windhawk renders |
| [windhawk.net/mods/win-x-hotcorners](https://windhawk.net/mods/win-x-hotcorners) | The published mod page |
| [DhakadG/win-x-hotcorners](https://github.com/DhakadG/win-x-hotcorners) | Standalone repo, for people who arrive from a search rather than from Windhawk |
| [`../../docs/per-mod/HANDOVER-win-x-hotcorners.md`](../../docs/per-mod/HANDOVER-win-x-hotcorners.md) | Working notes and review history |
| [`../../docs/per-mod/LAUNCH-win-x-hotcorners.md`](../../docs/per-mod/LAUNCH-win-x-hotcorners.md) | Reach and search-visibility plan |

## Install

Windhawk → **Explore** → search **Win-X Hot Corners** → Install.

## Promoting a change

The mod is published, so there is no standing pull request any more. Each update
needs a fresh branch off a synced `main` in the gallery fork and a **new** PR —
the order is in [`../../docs/HANDOVER.md`](../../docs/HANDOVER.md) §0.
