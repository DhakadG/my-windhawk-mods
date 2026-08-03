# Win-X Hot Corners

macOS-style hot corners **and screen edges** for Windows 10 and 11, with full
multi-monitor support. Built as a [Windhawk](https://windhawk.net) mod.

Move your cursor into a screen corner or against an edge and an action fires —
Task View, Show Desktop, lock, a key combination, or any command you like.
Every zone is configurable per monitor.

Inspired by [WinXCorners](https://github.com/vhanla/winxcorners).

---

## Install

1. Install [Windhawk](https://windhawk.net/).
2. Open Windhawk → **Explore** → search for **Win-X Hot Corners** → Install.

To install from source instead: Windhawk → **Create new mod**, paste the
contents of `win-x-hotcorners.wh.cpp`, then **Compile** and **Save**.

---

## Configure

Open the mod's **Settings** tab in Windhawk. Each monitor entry has eight
zones — four corners and four edges — and each takes its own action.

### Picking a monitor

Set **Monitor** to a display's friendly name, e.g. `DELL S2725QC`. The exact
names for your displays are written to the mod's log every time it loads:

```
Monitor 1 [PRIMARY] id='DELL S2725QC' device=\\.\DISPLAY1 (0,0)-(3840,2160)
Monitor 2           id='BOE0998'      device=\\.\DISPLAY2 (-1920,772)-(0,1852)
```

| Value | Meaning |
|-------|---------|
| A display name | That display only |
| `*` | Every display |
| *(empty)* | Falls back to the numeric **Monitor Number** field |

Names come from the display's EDID, so rearranging your desktop or changing
which display is primary never reshuffles your configuration. Two identical
monitors get a ` #2` suffix so they stay separately configurable.

Resolution is **per zone**: a name-matched entry wins for the zones it defines,
and `*` supplies the rest. Put a shared config on `*` and override a single
corner on one display.

### Settings

| Setting | Default | What it does |
|---------|---------|--------------|
| Corner activation size | 6 px | How large the corner squares are |
| Edge activation size | 6 px | How thick the edge strips are |
| Activation delay | 0 ms | Dwell time before firing |
| Pass-through guard | 80 ms | Stops a zone firing when you merely cross it |
| Cooldown between triggers | 300 ms | Minimum gap before the same zone fires again |
| Disable on fullscreen apps | on | Ignore corners while a game or video is fullscreen |
| Disable during mouse drag | on | Ignore corners while a mouse button is held |
| Excluded processes | *(empty)* | Semicolon-separated exe names to disable in |
| Verbose logging | off | Log every trigger (diagnostics only) |

---

## Actions

**Switching** — Task View · Switch to Last Window (Alt+Tab) · Task Switcher
(Ctrl+Alt+Tab) · Virtual Desktop Next / Previous / New

**Windows** — Show Desktop · Hide Other Windows · Minimize · Maximize · Snap
Left · Snap Right · Close Window

**System** — Start Menu · Search · Settings · File Explorer · Quick Settings ·
Notification Center · Clipboard History · Screenshot / Snip · Project ·
Task Manager · Mute Volume

**Power** — Lock Computer · Sleep · Turn Off Monitors · Start Screen Saver

**Custom** — Virtual Key Press · Custom Command · Nothing

### Virtual Key Press

One combination is `Modifier+Key`. Separate several with semicolons and they
are sent one after another, not merged into a single chord.

```
Ctrl+Shift+Esc
Alt+F4
Ctrl+C;Alt+Tab     -> sends Ctrl+C, then Alt+Tab
```

Modifiers: `Ctrl` `Alt` `Shift` `Win`
Keys: `A`-`Z`, `0`-`9`, `F1`-`F24`, `Enter`, `Space`, `Tab`, `Escape`, `Home`,
`End`, `Delete`, `Insert`, `PageUp`, `PageDown`, arrow keys, numpad keys and
media keys.

### Custom Command

Any executable, file, folder, or URL. Environment variables are expanded, and
unquoted paths containing spaces are handled.

```
notepad.exe
C:\Program Files\My Tool\tool.exe -arg
%AppData%\my_script.bat
https://example.com
uac;cmd.exe          -> prefix with uac; to request elevation
```

---

## How it works

Detection runs on its own thread inside a dedicated Windhawk process, sampling
the cursor every 16 ms. There is no global mouse hook, so the mod adds nothing
to the input path of your games and applications, and nothing the shell does
can delay or starve it. Actions run on a separate worker thread, so a slow
launch never holds up detection.

Zone rectangles are recomputed whenever the display layout changes — including
the docking, monitor-wake and RDP-reconnect cases where Windows does not send
`WM_DISPLAYCHANGE`. Detection is per-monitor DPI aware, so zones land correctly
on mixed-scaling setups.

### A note on inner corners

Windows lets the pointer cross freely between adjacent monitors, so corners
along a shared boundary have nothing to stop the cursor against. They are hard
to hit deliberately and easy to hit by accident. Prefer the outer perimeter of
your desktop arrangement — the pointer physically stops there, which is what
makes hot corners feel reliable.

---

## Troubleshooting

**Nothing happens at all.** Confirm the mod is actually loaded — check for a
matching DLL and a running process:

```powershell
ls "$env:ProgramData\Windhawk\Engine\Mods\32" -Filter "*hotcorners*"
Get-CimInstance Win32_Process -Filter "Name='windhawk.exe'" | % CommandLine
```

You should see a DLL and a `windhawk.exe -tool-mod "..."` process.

**A zone fires but nothing visible happens.** If a corner and the adjacent edge
are both bound to the same *toggle* action, crossing into the corner can fire
both and cancel out. Raise **Pass-through guard**, or set one of them to
`Nothing`.

**Diagnosing anything else.** Turn on **Verbose logging** and watch the mod's
log in Windhawk. It reports each trigger, the foreground window at the time,
and any key-injection failure. Turn it back off afterwards — the logging path
takes a system-wide lock and is not meant to be left on.

---

## Building

No build step. Windhawk compiles the single `.cpp` file itself. Paste it into
the Windhawk mod editor and press Compile.

---

## Suggestions & bugs

Have a suggestion or found a bug?
**[Open an issue](https://github.com/DhakadG/win-x-hotcorners/issues/new/choose).**

- Clear descriptions, screenshots, or steps to reproduce make fixes much faster.
- Include your Windows version and whether you run more than one display —
  most corner problems turn out to be display-layout related.
- For anything that misbehaves at a specific corner, turn on **Verbose
  logging** and paste the relevant lines. The log names the zone, the monitor
  and the foreground window at the moment it fired.
- Suggestions for new actions or UX improvements are always welcome.

Pull requests are welcome too. The whole mod is a single `.cpp` file, so a
change is usually a small, self-contained diff.

---

## Credits

- **[vhanla](https://github.com/vhanla)** — author of
  [WinXCorners](https://github.com/vhanla/winxcorners), the original Windows
  hot-corners utility that inspired this mod. This is an independent Windhawk
  port rather than a fork: it reuses none of the original code, but the idea,
  the action set and a good deal of the design direction come from that
  project and the discussion in its issue tracker.
- **[Ramen Software](https://github.com/ramensoftware)** — for
  [Windhawk](https://windhawk.net/), which makes a mod like this possible
  without a background application or an installer.

---

## License

[MIT](LICENSE)
