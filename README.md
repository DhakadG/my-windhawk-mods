# Win-X Hot Corners

macOS-style hot corners **and screen edges** for Windows 10 and 11, with full
multi-monitor support. Built as a [Windhawk](https://windhawk.net) mod.

Move your cursor into a screen corner or against an edge and an action fires —
Task View, Show Desktop, lock, a key combination, or any command you like.
Every zone is configurable per monitor.

Inspired by [WinXCorners](https://github.com/vhanla/winxcorners).

### Related mods

If you only want one specific behaviour, a smaller Windhawk mod may suit you
better:

- [edge-hot-corner-desktop-switch](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/edge-hot-corner-desktop-switch.wh.cpp)
  — hovering the left or right screen edge switches virtual desktop. That is
  one of the actions here, so this mod is a superset, but if it is all you
  need that one is far simpler.
- [hotcorner-hotkeys](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/hotcorner-hotkeys.wh.cpp)
  — sends a key combination from a corner, dispatching on a hotkey rather than
  on hover.

---

## Install

1. Install [Windhawk](https://windhawk.net/).
2. Open Windhawk → **Explore** → search for **Win-X Hot Corners** → Install.

To install from source instead: Windhawk → **Create new mod**, paste the
contents of `win-x-hotcorners.wh.cpp`, then **Compile** and **Save**.

---

## Configure

**Not in Windhawk — this mod has no Settings page.** It adds a tray icon next
to the clock, and that is where everything lives:

| | |
|---|---|
| **Left-click** | Turn the hot corners on and off |
| **Right-click** | Suspend for a while; skip while fullscreen; skip while dragging |
| **Right-click → Zones & settings...** | The dashboard |

The dashboard is a normal window with a clickable preview of your screen. Pick
a display, click the corner or edge you want, choose its action. Twelve zones
per display — four corners, four edges, and the centre of each edge — each with
its own action and, if you want, its own size, delay and modifier. Every field
explains itself on hover.

This is deliberate. Twelve zones on each of up to eight displays, each with a
forty-entry action list and six timing overrides, is not something a settings
form can present without becoming a tree nobody can navigate — and a Windhawk
mod cannot write its own settings from code, so anything changed in the mod's
own UI could never be written back to the page. Two places to configure one
thing, guaranteed to disagree. Now there is one.

> **Upgrading from 4.0.x or earlier?** Configuration you saved from the
> dashboard carries over untouched. Configuration you typed into Windhawk's
> Settings page does not — set it up again from the dashboard.

### Picking a monitor

The dashboard lists your displays by name, so normally there is nothing to
pick out. The names also go to the mod's **Log** tab in Windhawk, which is
where to look when a display is unplugged and you want to know which
configuration belonged to it:

```
+-- Your monitors ---------------------------------------
|  Copy a name below into this mod's "Monitor" setting.
|  Use  *  to apply one configuration to every monitor.
|
|   1. "DELL S2725QC"   [primary]   3840 x 2160  at (0, 0)
|   2. "BOE0C29"                    1920 x 1080  at (-1920, 772)
+--------------------------------------------------------
```

The list refreshes whenever you plug in, unplug or rearrange a display, so it
always reflects what is actually connected. The dashboard's first entry, **All
monitors**, applies one configuration everywhere.

Names come from the display's EDID, so rearranging your desktop or changing
which display is primary never reshuffles your configuration. Two identical
monitors get a ` #2` suffix so they stay separately configurable.

Resolution is **per zone**: a name-matched entry wins for the zones it defines,
and `*` supplies the rest. Put a shared config on `*` and override a single
corner on one display.

### Options

The dashboard's **Options** tab, grouped as it appears there. Every one of
these is a default that an individual zone can override on the **Zones** tab.

**How big the zones are**

| Option | Default | What it does |
|--------|---------|--------------|
| Corner size | 6 px | How large the corner squares are |
| Edge size | 6 px | How thick the edge strips are |
| Centre zone width | 20% | How much of an edge the centre zone takes |

**When a zone fires**

| Option | Default | What it does |
|--------|---------|--------------|
| Activation delay | 0 ms | Dwell time before firing |
| Pass-through guard | 80 ms | Stops a zone firing when you merely cross it |
| Knock window | 0 (off) | Require two quick entries before firing |
| Cooldown | 300 ms | Minimum gap before the same zone fires again |
| Require modifier | None | Zones stay inert unless Ctrl/Alt/Shift/Win is held |

**When to stay out of the way**

| Option | Default | What it does |
|--------|---------|--------------|
| Excluded processes | *(empty)* | Semicolon-separated exe names to stay quiet in |
| Skip while an app is fullscreen | on | Ignore zones on the display a game or video occupies |
| Skip while dragging the mouse | on | Ignore zones while a mouse button is held |
| Keep zones off the taskbar | off | Build zones from the work area, avoiding the taskbar |

**Everything else**

| Option | Default | What it does |
|--------|---------|--------------|
| Blank delay after lock | 1200 ms | Used by Lock and Turn Off Monitors |
| List my monitors in the log | on | Prints your displays so you can copy their names |

---

## Actions

**Switching** — Task View · Switch to Last Window (Alt+Tab) · Task Switcher
(Ctrl+Alt+Tab) · Virtual Desktop Next / Previous / New / Close

**Windows** — Show Desktop · Hide Other Windows · Minimize · Maximize · Snap
Left · Snap Right · Close Window

**System** — Start Menu · Search · Settings · File Explorer · Quick Settings ·
Notification Center · Clipboard History · Screenshot / Snip · Project ·
Task Manager · Mute Volume

**Power** — Lock Computer · Lock and Turn Off Monitors · Turn Off Monitors ·
Sleep · Start Screen Saver · Keep Awake On / Off

**Custom** — Virtual Key Press · Alternate Key Press · Custom Command ·
Alternate Command · Nothing

### Tray icon

Left-click toggles the hot corners on and off. Right-click opens the menu:
**Zones & settings...** for the dashboard, suspend for 15/30/60 minutes, and
the fullscreen and drag guards. **Reset these toggles** puts those three back
to their defaults and cancels any suspend — it does not touch your zones.
Wiping your zones is the dashboard's **Reset everything to defaults** button,
which asks first.

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

Detection runs on its own thread inside a dedicated Windhawk process, asking to
be woken every 16 ms — flat, with no adaptive backoff, so the sampling rate
never drops while a zone is armed. The wait expires on a system timer tick, and
the default tick is 15.625 ms, so in practice a sample lands every 16–31 ms
depending on what else has raised the timer resolution; the mod does not raise
it itself, because doing so costs battery machine-wide. It idles at 100 ms only
when the mod is switched off, suspended, or has no zones armed. There is no global
mouse hook, so the mod
adds nothing to the input path of your games and applications, and nothing the
shell does can delay or starve it. Actions run on a separate worker thread, so
a slow launch never holds up detection.

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

**Diagnosing anything else.** Watch the mod's **Log** tab in Windhawk. It
reports every trigger with the zone, the monitor and the foreground window at
the time, every suppressed trigger and why, and any key-injection failure.

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
- For anything that misbehaves at a specific corner, paste the relevant lines
  from the mod's **Log** tab. It names the zone, the monitor and the
  foreground window at the moment it fired.
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
