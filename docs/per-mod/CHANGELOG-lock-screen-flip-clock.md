# Changelog - Lock Screen Flip Clock

## 1.0.1 - 2026-10-28

- The readme and its media moved into this repository (`docs/media/lock-screen-flip-clock/`);
  `@homepage` points at the mod's folder here.

## 1.0.0 - 2026-10-28

First release.

- A flip clock on the Windows 11 lock screen, ported from LyricDock's: four-layer
  cards with real 3D flaps, eight flip styles, the airport-board roll-in, a
  colour change that flips every card over.
- The Figma "Puff 3D" card art drawn with Direct2D at the screen's real size, dark
  and light.
- Synthesised flap sounds (Airport board, Mechanical, Click, Whoosh, Soft tap) and
  the vibration thump, rendered per flip and played through XAudio2; a sound
  latency setting for Bluetooth speakers.
- Hosted inside the lock screen's curtain, so it leaves with it; hides the
  Windows clock and the widgets, keeps the media controls and status icons.
- Silent and idle while the display is off.
- Tests: self-checks, a XAML-island preview that films the clock frame by frame,
  a speaker loopback check, and a test build that runs on the real lock screen.
