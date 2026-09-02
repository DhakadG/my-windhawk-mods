# tray-hover-expand-plus

Fork of [Tray hover expand](https://windhawk.net/mods/tray-hover-expand) by
[wygodad](https://github.com/wygodad).

The original opens the hidden-icons chevron flyout on hover. This adds the same for **any
tray icon you configure** — EarTrumpet, the volume button (which opens Quick Settings), a
hidden icon inside the flyout — and makes the cursor's trip from the icon to the popup
forgiving, since the two are usually nowhere near each other.

The chevron's own settings, defaults and behaviour are unchanged from the original.

The full user guide lives in the mod's readme block at the top of
[`tray-hover-expand-plus.wh.cpp`](tray-hover-expand-plus.wh.cpp): how to find an icon's
match values, how popup detection works, recommended delays, and a troubleshooting table.

Engineering notes, the live measurements this was built against, and the bugs that only
showed up when running it: [`docs/HANDOVER.md` §15](../../docs/HANDOVER.md).

## Verifying a change

```bash
pwsh -File scripts/check-mod.ps1 -Warnings && python scripts/check-settings.py && pwsh -File scripts/build-mod.ps1 && python scripts/check-gallery.py
```

Then the two probes, which include the mod itself and exercise the real functions:

```bash
scripts/probes/probe-travel-grace.cpp   24 assertions on ShouldClose / ClampCandidates
scripts/probes/probe-resolve.cpp        runs the real matcher against the live taskbar
```

Build them with Windhawk's own clang, `-static` (see `docs/HANDOVER.md` §2).
