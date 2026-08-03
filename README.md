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

Open the mod's **Settings** tab in Windhawk. Each monitor entry has twelve
zones — four corners, four edges and the centre of each edge — and each takes
its own action.

### Picking a monitor

You don't have to guess the name. Open the mod's **Log** tab in Windhawk — the
mod lists your displays every time it loads, ready to copy:

```
+-- Your monitors ---------------------------------------
|  Copy a name below into this mod's "Monitor" setting.
|  Use  *  to apply one configuration to every monitor.
|
|   1. "DELL S2725QC"   [primary]   3840 x 2160  at (0, 0)
|   2. "BOE0C29"                    1920 x 1080  at (-1920, 772)
+--------------------------------------------------------
```

Paste the text between the quotes into **Monitor**. The list refreshes
whenever you plug in, unplug or rearrange a display, so it always reflects
what is actually connected.

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
| Knock to activate | 0 (off) | Require two quick entries before firing |
| Require a modifier key | None | Zones stay inert unless Ctrl/Alt/Shift/Win is held |
| Centre zone width | 20% | How much of an edge the centre zone takes |
| Cooldown between triggers | 300 ms | Minimum gap before the same zone fires again |
| Disable on fullscreen apps | on | Ignore corners while a game or video is fullscreen |
| Disable during mouse drag | on | Ignore corners while a mouse button is held |
| Excluded processes | *(empty)* | Semicolon-separated exe names to disable in |
| Keep zones off the taskbar | off | Build zones from the work area, avoiding the taskbar |
| Delay before blanking after lock | 1200 ms | Used by Lock and Turn Off Monitors |
| List my monitors in the log | on | Prints your displays so you can copy their names |
| Verbose logging | off | Log every trigger (diagnostics only) |

---

## Actions

**Switching** — Task View · Switch to Last Window (Alt+Tab) · Task Switcher
(Ctrl+Alt+Tab) · Virtual Desktop Next / Previous / New / Close

**Windows** — Show Desktop · Hide Other Windows · Minimize · Maximize · Snap
Left · Snap Right · Close Window

**System** — Start Menu · Search · Settings · File Explorer · Quick Settings ·
Notification Center · Clipboard History · Screenshot / Snip · Project ·
Task Manager · Mute Volume

**Power** — Lock Computer · Lock and Turn Off Monitors · Keep Awake On/Off · Sleep · Turn Off Monitors ·
Start Screen Saver · Keep Awake On / Off

**Custom** — Virtual Key Press · Alternate Key Press · Custom Command ·
Alternate Command · Nothing

### Tray icon

The mod adds a tray icon. Left-click toggles the hot corners on and off;
right-click opens a menu to suspend them for 15/30/60 minutes, flip the
fullscreen and drag guards, or turn verbose logging on while reproducing
something.

A Windhawk mod cannot write the settings shown on its own Settings page, so
these tray changes are kept as *overrides* in the mod's storage and applied on
top of your settings. **Reset to Windhawk settings** clears them and goes back
to exactly what the Settings page says.

### Alternate Key Press / Alternate Command

Two actions separated by `|`. The zone fires the left one, then the right
one, then the left again:

```
Alt+S | Alt+H            show notes, then hide them
notepad.exe | calc.exe
```

Each side accepts everything the single-action version does, so
`Ctrl+C;Ctrl+V | Alt+Tab` is valid. Every zone alternates independently, and
the position resets when settings or the display layout change.

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
