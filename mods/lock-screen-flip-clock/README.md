# Lock Screen Flip Clock

A mechanical flip clock for the Windows 11 lock screen, ported from the flip
clock in [LyricDock](https://github.com/DhakadG/lyricdock): paper-grain cards
whose flaps really flip, an airport-board roll-in, synthesised flap sounds and a
vibration thump.

The showcase - screenshots from the real lock screen, GIFs, video with sound,
sound samples, every setting and how it was tested - lives at
**<https://github.com/DhakadG/lock-screen-flip-clock>**.

| Where | What it is |
| --- | --- |
| [`lock-screen-flip-clock.wh.cpp`](lock-screen-flip-clock.wh.cpp) | Source, and the readme Windhawk renders |
| [`tests/`](tests/) | Self-checks, the XAML-island preview and filming rig, the speaker loopback check, the lock screen test build |
| [DhakadG/lock-screen-flip-clock](https://github.com/DhakadG/lock-screen-flip-clock) | Standalone repo: readme, media, changelog |

## Tests

```powershell
.\tests\run-preview.ps1 check  $env:TEMP\lsfc-check     # 38 self-checks + WAVs of every sound
.\tests\run-preview.ps1 art    $env:TEMP\lsfc-art       # every card, both colours, as PNG
.\tests\run-preview.ps1 frames $env:TEMP\lsfc-frames start=10:59:57 secs=5   # needs an unlocked desktop
.\tests\run-lockapp-test.ps1                            # test build for the real lock screen
```

`lockapp-test.cpp` explains the lock screen rig: no Windhawk install and no UAC,
it is loaded straight into LockApp and talks through LockApp's own sandbox
folder.
