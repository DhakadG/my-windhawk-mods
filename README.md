# Windhawk Mods

My personal [Windhawk](https://windhawk.net) mods — sources, working notes and revision history
in one place.

> This is **not** my fork of the official mod gallery. That lives at
> [DhakadG/windhawk-mods](https://github.com/DhakadG/windhawk-mods) and is only used to open PRs
> against [ramensoftware/windhawk-mods](https://github.com/ramensoftware/windhawk-mods).

## Layout

```
mods/<mod-id>/<mod-id>.wh.cpp   the mod source, mirrored from ModsSource
mods/<mod-id>/README.md         per-mod pointer, where one exists
docs/HANDOVER.md                the working notes that matter across sessions
docs/per-mod/                   per-mod handovers, backlogs, launch plans
docs/media/                     readme GIFs and screenshots, referenced by raw URL
docs/design/                    design drafts
scripts/                        the four verification tools + the ModsSource sync
scripts/capture/                the demo-recording rig
scripts/probes/                 standalone C++ probes for verifying Windows behaviour
scripts/legacy/                 superseded checkers, kept for reference
archive/                        superseded working copies, kept for reference only
```

## Mods

All are authored or maintained by **lost_husky**; the forks credit their original author at
the top of each mod's readme.

| Mod | Version | Original author | Notes |
| --- | --- | --- | --- |
| [win-x-hotcorners](mods/win-x-hotcorners/) | 1.3.0 | — | macOS-style hot corners and edges. **Published** — [in the gallery](https://windhawk.net/mods/win-x-hotcorners) via [#5001](https://github.com/ramensoftware/windhawk-mods/pull/5001), merged 22 Aug 2026. |
| [taskbar-ai-quota-fork](mods/taskbar-ai-quota-fork/) | 0.12.0 | [Cleroth](https://github.com/Cleroth) | AI quota bars. Fixed for the 26300 StackPanel tray. |
| [taskbar-clock-customization-v3](mods/taskbar-clock-customization-v3/) | 3.1.71 | [m417z](https://github.com/m417z) | Clock with sensors, weather and network throughput. |
| [taskbar-fluent-media-player-fork](mods/taskbar-fluent-media-player-fork/) | 1.6.1 | [Salyts](https://github.com/Salyts) | Fluent media player. Fixed for the 26300 StackPanel tray. |
| [taskbar-system-info-fork](mods/taskbar-system-info-fork/) | 1.5.0 | [Yevhenii Starychenko](https://github.com/starychenko) | CPU/GPU/RAM/VRAM monitor with network throughput, an internet-status dot, HWiNFO or LibreHardwareMonitor sensors, and a card-style background. Real tray/taskbar insertion instead of the original's overlay. |
| [mac-magnifying-cursor](mods/mac-magnifying-cursor/) | 1.5.0 | [Jaali](https://github.com/alivca) | Shake-to-magnify cursor. |
| [bt-battery-monitor-fork](mods/bt-battery-monitor-fork/) | 1.1.1 | — | Bluetooth battery levels. |
| [spicetify-guardian](mods/spicetify-guardian/) | 1.1.0 | — | Keeps a Spicetify install from being undone by a Spotify update. |

Only `win-x-hotcorners` is in the public gallery. The rest are personal builds and forks,
installed from source.

### Windows 11 26H2 (build 26300)

Build 26300 changed `SystemTrayFrameGrid` from a `Grid` to a `StackPanel` while keeping the
name. Any mod that resolves its tray target with `try_as<Grid>()` gets null and silently
never injects. Both taskbar forks here now branch on the real type and insert by child index
on the new shape. Same root cause as
[ramensoftware/windhawk-mods#5018](https://github.com/ramensoftware/windhawk-mods/issues/5018).

## Working on a mod

Windhawk compiles from `C:\ProgramData\Windhawk\ModsSource\`, which needs admin rights to write.
So that directory stays the source of truth and this repo mirrors it:

1. Edit in the repo, then verify **before** pasting anything into the editor:

```bash
pwsh -File scripts/check-mod.ps1 -Warnings && python scripts/check-settings.py
```

`check-mod.ps1` drives Windhawk's own bundled clang 20 (`C:\Program Files\Windhawk\Compiler`)
with Windhawk's own `compile_flags.txt`, so it reproduces exactly the diagnostics the
Windhawk editor shows. `check-settings.py` parses the `==WindhawkModSettings==` block,
which clang cannot check — a malformed settings block still compiles and loads, and only
fails later as `Failed to extract previous initial settings for engine`.

2. Paste into the Windhawk editor and compile.
3. Pull the result back into the repo:

```bash
pwsh -File scripts/sync-from-modssource.ps1
```

4. `git diff` to review, then commit.

Installed versions can lag behind this repo. The sync script prints the version of
everything it copies, which is the quickest way to spot a mismatch.

## Licence

[MIT](LICENSE).
