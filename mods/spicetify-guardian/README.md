# Spicetify Guardian

Keeps Spicetify alive across Spotify updates. Detects the moment Spotify wipes it,
checks compatibility, and re-applies — with a tray dashboard for every other Spicetify
chore.

The full write-up, plus standalone PowerShell scripts for machines without Windhawk,
lives at **<https://github.com/DhakadG/spicetify-guardian>**.

## What it is

Spotify auto-updates itself, overwrites `xpui.spa`, and Spicetify disappears. The usual
fix is to type `spicetify update` again. This does that for you, the moment it happens,
and refuses to when doing it would be a bad idea.

- **Exact detection, no polling** — when Spicetify is applied `Spotify\Apps\xpui` is a
  folder; when Spotify updates it becomes an `xpui.spa` file again. `ReadDirectoryChangesW`
  on that one folder is the whole trigger.
- **A real compatibility gate** — parses the `## Compatibility` block from the
  `spicetify/cli` release notes, the only machine-readable source upstream publishes.
  Three tiers: supported, untested (proceeds, says so), too old (refuses).
- **Cannot corrupt your backup** — `clear backup apply` over an already-patched install
  saves the *patched* files as pristine and makes `spicetify restore` useless forever.
  Every repair is gated on an explicit applied-state check, which is what the
  run-at-every-login auto-updaters miss.
- **Never kills your music** — all checks run before Spotify is touched, then it closes
  with `WM_CLOSE` rather than a kill, and restarts after.
- **Config snapshots** before every destructive operation; last 30 restorable.

## Tray icon

Green when healthy, amber when Spicetify is missing, grey when paused.

- **Left-click** — dashboard
- **Right-click** — repair, restart Spotify, pause (1/3/7 days or until the next Spotify
  update), block/unblock Spotify updates, strict mode

## Dashboard

- **Status** — versions, applied state, tested Spotify range, last run, pause countdown
- **Actions** — repair, update, restore to stock, backup+apply, install/uninstall
  Spicetify, install/remove Marketplace, block/unblock updates, config snapshots
- **Config** — tick extensions and custom apps on and off, switch theme; changes are
  batched into one `apply`
- **Health** — config entries pointing at deleted files, Marketplace without its
  placeholder theme, duplicate extension filenames across the two extension folders
- **Log** — what it did and why

## Settings

Three, deliberately: *Repair automatically*, *Show notifications*, *Path to
spicetify.exe*.

Everything else is in the dashboard. A Windhawk mod can read its settings but not write
them, so anything the mod changes at runtime (pause, strict mode, the attempt counter)
has to persist elsewhere — it goes to `%LOCALAPPDATA%\SpicetifyGuardian\state.json`,
which is the same file the standalone scripts use, so the two stay in agreement.

## Implementation notes

Runs as a [tool mod](https://github.com/ramensoftware/windhawk/wiki/Mods-as-tools:-Running-mods-in-a-dedicated-process)
in a dedicated `windhawk.exe` process — it hooks nothing and injects nowhere. Session 0
is rejected and a named mutex enforces a single instance, so the several `windhawk.exe`
processes on a normal system produce exactly one tray icon.

Compatibility lookups go through WinHTTP rather than `Wh_GetUrlContent`, because the
GitHub API rejects requests without a `User-Agent` and `Wh_GetUrlContent` cannot set
headers.

## License

MIT
