# Windhawk Mods

My personal [Windhawk](https://windhawk.net) mods — sources, working notes and revision history
in one place.

> This is **not** my fork of the official mod gallery. That lives at
> [DhakadG/windhawk-mods](https://github.com/DhakadG/windhawk-mods) and is only used to open PRs
> against [ramensoftware/windhawk-mods](https://github.com/ramensoftware/windhawk-mods).

## Layout

```
mods/<mod-id>/<mod-id>.wh.cpp   the mod source, mirrored from ModsSource
mods/<mod-id>/README.md         per-mod docs, where they exist
docs/                           handover notes and feature backlogs
scripts/                        sanity checks + the ModsSource sync script
archive/                        superseded working copies, kept for reference only
```

## Mods

| Mod | Version | Notes |
| --- | --- | --- |
| [win-x-hotcorners](mods/win-x-hotcorners/) | 4.1.4 | macOS-style hot corners. Also published standalone; PR [#5001](https://github.com/ramensoftware/windhawk-mods/pull/5001) is open against the gallery. |
| [taskbar-ai-quota-fork](mods/taskbar-ai-quota-fork/) | 0.11.0 | Fork of Cleroth's AI quota bars. **Not rendering on Windows 11 26H2 build 26300** — see [the handover](docs/HANDOVER-taskbar-ai-quota-fork.md). |
| [taskbar-clock-customization-v3](mods/taskbar-clock-customization-v3/) | 3.1.69 | Fork of the clock mod with sensors, weather and network throughput. |
| [mac-magnifying-cursor](mods/mac-magnifying-cursor/) | 1.5.0 | Shake-to-magnify cursor. |

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

Installed versions can lag behind this repo — `win-x-hotcorners` here is 4.1.4 while 4.1.2 is
what is currently installed. The sync script prints the version of everything it copies.

## Licence

[MIT](LICENSE).
