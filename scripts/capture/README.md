# Windhawk demo capture scripts — improved

The scripts now share a session logger and are designed to fail loudly without
leaving the desktop or modifier keys in a bad state.

## Main recording

From PowerShell 7:

```powershell
cd "C:\path\to\this\folder"
.\runall.ps1
```

Each run creates:

```text
logs\run-YYYYMMDD-HHMMSS-XXXXXXXX.log
logs\debug-YYYYMMDD-HHMMSS-XXXXXXXX.log
```

The run log records staging, pointer moves, waits, key presses, ffmpeg startup/
exit, verification, failures, and the final summary. The debug log captures the
mod's `OutputDebugString` messages with timestamps and PIDs.

To prevent the automatic debug watcher:

```powershell
.\runall.ps1 -NoDebugWatch
```

To stop the whole run at the first failed clip:

```powershell
.\runall.ps1 -StopOnError
```

## Dry run

Exercise all zones without recording:

```powershell
.\dryrun.ps1
```

The modifier test now holds Ctrl **before** entering the trigger region.

## GIF conversion

```powershell
.\mkgif.ps1
```

The converter verifies ffmpeg exit status and probes each GIF after creation.

## Notes

`ctl.ps1` is safe to dot-source repeatedly, so `runall.ps1` and `rec.ps1` no
longer collide when both load the native C# helper types.

`rec.ps1` captures ffmpeg stdout/stderr into per-clip log files and verifies
that every recording is actually 3840x2160 before reporting success.

`runall.ps1` continues to the next clip by default. Use `-StopOnError` when a
single failure should abort the session.
