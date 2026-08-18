# Scripts

## Verification — run these before pushing

The order they belong in, and what each one catches that the others do not:

| Script | Catches |
|---|---|
| `check-mod.ps1` | Compiler errors and warnings. A real clang type check with the mod's own flags, per declared architecture. |
| `check-settings.py` | A malformed `==WindhawkModSettings==` block. These compile and load fine, then fail at runtime as *"Failed to extract previous initial settings"* — clang cannot see them. |
| `build-mod.ps1` | Link errors. `check-mod.ps1` is `-fsyntax-only`, so a missing library reaches CI without this. |
| `check-gallery.py` | The Windhawk catalogue's own submission rules — metadata, readme and settings conventions. Downloads and runs their `pr_validation.py`. |

```powershell
pwsh -File scripts/check-mod.ps1 -Warnings
python scripts/check-settings.py
pwsh -File scripts/build-mod.ps1
python scripts/check-gallery.py
```

`sync-from-modssource.ps1` copies mods out of `C:\ProgramData\Windhawk\ModsSource`
(which needs admin to write) into this repo, printing versions as it goes.

## `capture/`

The demo-recording rig for the readme GIFs: DPI-aware pointer control, a
Desktop Duplication recorder, per-clip GIF conversion, and a dashboard
screenshotter. `capture/README.md` has the details and the traps.

## `probes/`

Standalone programs written to settle a specific question against the real
machine rather than by reasoning about the API. Each one is disposable; they are
kept because the answers they produced are cited in `docs/HANDOVER.md`.

- `probe-sendkeys.cpp` — slices the mod's real `SendKeys` out of the source and
  drives a live window with it. Proved that a physically-held modifier
  contaminated injected combinations.
- `probe-vtext.cpp` — vertical text rendering candidates for the dashboard.
- `zonegeom.cpp` — asserts the 16 zones tile the border with no overlap or gap,
  at six aspect ratios including portrait.

## `legacy/`

Superseded heuristic checkers from before the clang-based tooling existed. Kept
because `docs/per-mod/HANDOVER-win-x-hotcorners.md` still cites them, but the
four scripts at the top of this folder replace them — prefer those.
