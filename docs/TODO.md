# Deferred work

Ordered by when it should happen, not by size. Nothing here blocks the current
priority, which is getting the taskbar mods rendering again on 26H2 build 26300.

## 1. Bring the settings back into the mods' own settings pages

`taskbar-ai-quota-fork` retired its settings page in v4.1.0 in favour of a dashboard,
because a flat list of ~40 settings in the old Windhawk UI was unusable. Windhawk v2
removes that constraint: settings now render as **collapsible groups**, with a
per-group "Reset to default" and a settings count on the collapsed header, plus a
Visual/Textual toggle. `taskbar-fluent-media-player` already uses this shape —
Main Settings > Media player / Album Art / Text area / Media Buttons / Visualizer.

So the dashboard is no longer carrying its weight. Plan:

- Group the quota mod's settings the way the media player does: Accounts, Bars,
  Placement, Colours, Reset window, Notifications, Debug.
- Move everything the dashboard currently owns back into YAML, keeping the setting
  keys unchanged so nobody's configuration resets.
- Keep the dashboard only if something genuinely cannot be expressed as a setting
  (sign-in flow, live token state). Otherwise delete it — that is a large amount of
  custom XAML to stop maintaining.
- Do the same review for `taskbar-clock-customization-v3`, whose settings block is
  already ~700 lines and flat.

Do this **after** the mods render again, so a rendering regression is never confused
with a settings-migration regression.

## 2. Reconcile installed versions with this repo

`win-x-hotcorners` is 4.1.4 here and 4.1.2 in `ModsSource` — the repo is ahead, so the
installed copy is stale. Either compile 4.1.4 in Windhawk or work out why it was never
installed. `scripts/sync-from-modssource.ps1` prints versions as it copies, so this
should not silently drift again.

## 3. Prune `archive/`

Four superseded working copies are tracked there so nothing was lost during the
reorganisation. `archive/win-x-hotcorners-v4.1.4-duplicate.cpp` is byte-identical to
the tracked mod source and can go immediately. The other three are older working
copies of mods whose current source is now under `mods/`; delete once confirmed.

## 4. Decide what happens to `DhakadG/win-x-hotcorners`

Its history now lives here, and `origin` points at this repo. The old repo is still on
GitHub as remote `old-win-x-hotcorners`. Archive it or leave it as the public
standalone home for that one mod — but do not delete `DhakadG/windhawk-mods`, which is
the gallery fork backing open PR
[#5001](https://github.com/ramensoftware/windhawk-mods/pull/5001).
