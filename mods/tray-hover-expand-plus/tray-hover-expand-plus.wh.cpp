// ==WindhawkMod==
// @id              tray-hover-expand-plus
// @name            Tray hover expand plus
// @description     Open the hidden tray icons flyout and any tray icon's own flyout on hover, with a forgiving cursor path from the icon to the popup
// @version         2.3.0
// @author          lost_husky
// @github          https://github.com/DhakadG
// @include         windhawk.exe
// @compilerOptions -lole32 -loleaut32 -lshell32
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Tray hover expand plus

Open tray flyouts by hovering instead of clicking.

A fork of [Tray hover expand](https://windhawk.net/mods/tray-hover-expand) by
[wygodad](https://github.com/wygodad), which does this for the hidden icons
chevron. That still works exactly as before. This fork adds the same behaviour
for **any tray icon you configure** — hover EarTrumpet and its volume flyout
opens, hover the volume button and Quick Settings (the Win+A panel) opens — and
makes the cursor's trip from the icon to the popup forgiving, because the two
are usually nowhere near each other.

## The problem it solves

A tray icon and the popup it opens are often far apart. Measured on the build
this was developed against: the EarTrumpet icon sits at x 3023-3068 along the
bottom edge, and its flyout opens at (3375,924) — a 300px gap to the right and
1100px up.

```
                                 +-------------+
                                 |             |
                                 |    popup    |
                                 |             |
                                 +-------------+
                                        ^  cursor must cross a gap
                                        |  to get here
 [ ... ] [icon] [ ... ] [clock] --------+
```

Closing the popup the moment the cursor leaves the icon would make the feature
useless. So while a popup is open the mod watches whether the cursor looks like
it is *travelling towards it*: still inside the corridor between the two, and
not getting further away. While that holds, the popup is kept alive, up to a
bounded travel timeout. Move the other way and it closes on the normal close
delay. Reach the popup and the pending close is cancelled outright.

The grace period is therefore dynamic, not one big timeout:

| Cursor is doing | Popup lives for |
|---|---|
| sitting in the popup | as long as you like |
| heading towards the popup | up to travel timeout + close delay |
| parked in the gap | up to travel timeout + close delay, then closes |
| heading anywhere else | close delay |

## Quick start

Two examples ship ready to use, both **disabled**:

- **EarTrumpet** — matched by name.
- **Quick Settings** — matched by class name. Hovering the volume/network/
  battery button opens the Win+A panel.

Tick *Enabled* on one and save. If it works, you are done. If it does not, your
build names things differently — follow the next section, that is what the
diagnostic log is for.

These two are only examples. Edit them, delete them, add your own.

## Configuring a tray icon

1. Turn on **Log tray candidates**. Save.
2. Open the mod's log in Windhawk. Within a few seconds a table appears listing
   every button on the taskbar.
3. Find your icon in it. A line looks like this:

```
[16] tray  class=SystemTray.NormalButton  aid=NotifyItemIcon
           rect=(3023,2100 45x60)  name=EarTrumpet: 30% - Voicemeeter Input (ROOT)
```

4. Add an item under **Tray items** and copy values into the match fields.
5. Tick its *Enabled*. Save. Test.
6. Turn **Log tray candidates** back off.

The table is logged again only when it actually changes, so leaving diagnostics
on does not flood the log — but it does keep scanning the taskbar every few
seconds, so turn it off when you are done.

### The match fields

| Field | Meaning |
|---|---|
| **Match name** | Case-insensitive *substring* of the name. `eartrumpet` matches `EarTrumpet: 30% - ...` |
| **Match the name exactly** | Require the whole name to be equal instead of contained |
| **Match AutomationId** | Exact match, e.g. `NotifyItemIcon` |
| **Match class name** | Exact match, e.g. `SystemTray.NormalButton` |

**Every field you fill in must match, and exactly one element must match
overall.** Leave a field empty to ignore it. If zero elements match, or if two
or more do, the item is skipped and the log says which — the mod will not pick
one at random, because invoking the wrong tray button opens the wrong thing.

### How many fields to fill in

More identifiers means a safer match. It also means more things that can break
when Windows or the app updates. Fill in the fewest that identify the icon
uniquely.

**Use the name** for third-party apps. It is usually the icon's tooltip, so it
is descriptive and stable enough:

```
Match name = EarTrumpet
```

That is the whole EarTrumpet example. Note the full name on a live system is
`EarTrumpet: 30% - Voicemeeter Input (ROOT)` — it carries the current volume and
device, so it changes constantly. Substring matching is what makes that work;
do not turn on *Match the name exactly* for an icon like this.

Reach for the name when the app has no stable AutomationId, or its class name is
shared with every other notification icon (on Windows 11 they are all
`SystemTray.NormalButton` / `NotifyItemIcon`, so class and AutomationId alone
cannot tell two notification icons apart — only the name can).

**Use class + AutomationId** for the Windows system buttons. Names there are
localised; class names are not:

```
Match class name  = SystemTray.OmniButtonCenter
Match AutomationId = SystemTrayIcon
```

That is the whole Quick Settings example.

## Configuring Windows system buttons

On the build this was developed against, the tray exposes these. **Check your
own diagnostic output rather than copying this table** — Microsoft moves these
around, and a build that splits or merges the buttons will not match.

| Button | Class | AutomationId | Opens |
|---|---|---|---|
| Hidden icons chevron | `SystemTray.NormalButton` | `SystemTrayIcon` | the overflow flyout |
| A notification icon | `SystemTray.NormalButton` | `NotifyItemIcon` | that app's own popup |
| Volume | `SystemTray.OmniButtonCenter` | `SystemTrayIcon` | Quick Settings |
| Network | `SystemTray.AccentButton` | `SystemTrayIcon` | Quick Settings |
| Battery | `SystemTray.AccentButton` | `SystemTrayIcon` | Quick Settings |
| Clock | `SystemTray.OmniButton` | `SystemTrayIcon` | the calendar panel |

Network and Battery share a class *and* an AutomationId on that build, so those
two rules match two elements each and the item is refused as ambiguous. Add a
name fragment to separate them, accepting that the name is localised.

The chevron is not configured as an item — it has its own settings group,
because it is the one tray button Windows guarantees exists.

## How popup detection works

The popup cannot be predicted, so it is discovered. EarTrumpet's window class is
`HwndWrapper[EarTrumpet.exe;;<guid>]` with **a fresh GUID every time the app
starts**, which is exactly why nothing is hard-coded here.

After invoking an icon, for the length of the discovery window, the mod looks
for, in order:

1. a popup menu (`#32768`) that was not already on screen — what icons using the
   classic tray API show,
2. the foreground window changing to a window that was not there before — the
   strongest signal, and what catches EarTrumpet, Quick Settings and the
   calendar,
3. otherwise any newly visible top-level window, nearest to the icon.

Anything the shell owns, anything covering a whole monitor, anything smaller
than 48x32, and the tooltip window class are all rejected, so a tooltip or a
maximised app cannot be mistaken for a popup.

If that picks the wrong window for some item, pin it down with **Popup
matching** set to `process` (and fill in e.g. `EarTrumpet.exe`) or `class` (a
fragment of the window class). Leave it on `auto` unless you have a problem.

## How travel grace works

Once the popup is found, every poll the mod asks: is the cursor engaged?

Engaged means on the icon, on the popup, or busy — a mouse button held down
(dragging a volume slider) or a context menu open. Engaged holds the close timer
at zero.

Not engaged starts the travel test: the cursor must be inside the **corridor**
(the box spanning the icon and the popup, grown by the travel padding) and must
not be getting further from the popup. While both hold, the popup is kept alive,
until the travel timeout runs out. Then the close delay runs and it closes.

Two details that matter in practice:

- Once the cursor has actually been inside the popup, the popup padding is
  doubled, which absorbs overshooting slightly past its edge.
- A small amount of jitter is tolerated, so "not getting further away" does not
  mean a perfectly straight line. Slow movement, fast flicks and diagonal paths
  all work; the popup sits above, below or to either side without any change.

## Recommended settings

The defaults are these. They were picked against a real icon-to-popup gap and
are a good starting point.

| Setting | Default | Raise it if | Lower it if |
|---|---|---|---|
| Hover delay | 250 ms | popups fire as you cross the tray | opening feels sluggish |
| Close delay | 400 ms | popups close a touch too eagerly | popups linger |
| Travel timeout | 1500 ms | you move slowly and lose the popup | a popup you abandoned hangs around |
| Travel corridor padding | 48 px | your path to the popup wanders | unrelated movement keeps popups alive |
| Popup hit area padding | 24 px | you overshoot the popup edge | — |
| Popup discovery window | 1200 ms | a slow app's popup is never found | — |

Keep the chevron's own hover delay at 0 unless the flyout opens when you only
brush past it on the way to the clock; 150 is a good value then.

### Per-item overrides

Every item can override the global defaults, or inherit them:

```
Hover delay = -1     use the global default
Hover delay = 500    this item waits 500 ms
```

`-1` means inherit for every numeric field. The dropdowns work the same way, with
an explicit *Inherit* option. Prefer inheriting — only set a value on an item
that genuinely needs to differ.

## Troubleshooting

| Problem | What to do |
|---|---|
| Icon does not open anything | Turn on diagnostics, check the match fields against the table |
| The log says the item matched nothing | A rule is wrong, or the icon is inside the hidden icons flyout and the flyout is closed |
| The log says the item matched several elements | Add another rule, or a name fragment, to narrow it |
| The wrong thing opens | Your rule matched a different button. Add class + AutomationId |
| Popup closes too quickly | Raise Close delay |
| Popup closes while you move towards it | Raise Travel timeout, then Travel corridor padding |
| Popup stays open too long | Lower Close delay and Travel timeout |
| Popup is never detected | Raise Popup discovery window |
| The wrong window is treated as the popup | Set Popup matching to process or class |
| It broke after a Windows update | Turn diagnostics on, find the icon again, compare its class / AutomationId / name to what you configured, update the item |
| Nothing works at all after an update | Check the chevron settings group too; its class name may have changed |

## Windows compatibility

Windows 11 only. Identification relies on the Windows 11 shell's UI Automation
types; on Windows 10 nothing matches and the mod does nothing.

Developed and verified against build 26340.9233. It is not tied to that build —
the chevron has a class match, a name match in 22 languages and an optional
positional guess, and items are matched by whatever rules you give them — but
Microsoft renames these things, and when that happens the diagnostic log is how
you find the new names. That is the main reason it exists.

Other notes:

- Only the primary taskbar (`Shell_TrayWnd`) is searched, which is where
  Windows 11 puts tray icons. Popups on any monitor are handled.
- Icons *inside* the opened hidden icons flyout can be configured too — the
  flyout is searched as a second location while it is open. Their popups stop
  being tracked if the flyout closes underneath them.
- Per-monitor DPI v2 is used throughout, so UI Automation rectangles, window
  rectangles and the cursor stay in one coordinate space on mixed-DPI setups.
- Explorer restarts are recovered from automatically: elements that go stale are
  dropped and re-found.

## Performance

With diagnostics off and nothing open, a poll costs three Win32 calls. The
expensive UI Automation walk of the taskbar happens only when some configured
item has no element yet, only while the cursor is at the taskbar, and it backs
off after repeated failures. One walk serves every item. Element rectangles are
refreshed a few times a second at most, and only near the taskbar. Desktop-wide
window enumeration happens only during the discovery window right after an icon
is invoked.

With diagnostics on, the taskbar is walked every five seconds regardless. That
is the cost of the feature; turn it off when you are done configuring.

## Safety

The mod never invokes a tray element it could not confidently identify. Zero
matches or several matches means the item is skipped and the reason is logged.

Only tray elements are eligible, so a taskbar app button — whose name is its
window title and could contain anything — can never be matched by accident.
Targeting one anyway requires ticking *Allow non-tray taskbar elements* on that
item.

It does not fight manual clicks. Clicking inside a popup suspends auto-close
until that popup goes away, and once the mod has learned what an item's popup
looks like it will not invoke the icon while that popup is already on screen.

It does not hide tray tooltips. Only the chevron has an optional tooltip
suppression setting, off by default, because the chevron's own tooltip covers
the flyout it opens.

It works through UI Automation and runs as a tool mod in its own process. It
does not hook shell internals and does not inject into Explorer.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- enabled: true
  $name: Enabled
  $description: Master switch. Turn off to leave the taskbar completely alone without uninstalling the mod.
- pollInterval: 50
  $name: Polling interval (ms)
  $description: How often the cursor position is checked. Lower is smoother and costs more CPU.
- diagnostics: false
  $name: Log tray candidates
  $description: Periodically log every tray element found, with class name, AutomationId, position and name. Turn this on to find the values to match an item on, then turn it off.
- chevronEnabled: true
  $name: 'Chevron: open the hidden icons flyout on hover'
  $description: The original behaviour of this mod. The chevron is identified separately from the items below because it is the one tray button Windows guarantees is there.
- autoClose: true
  $name: 'Chevron: collapse when the cursor leaves'
  $description: After the cursor leaves the opened icons (and the chevron), the flyout closes itself.
- hoverDelay: 0
  $name: 'Chevron: hover delay (ms)'
  $description: How long the cursor must stay on the chevron before the flyout opens. 0 opens immediately; a small value stops the flyout from opening when you only brush past the chevron on the way to the clock.
- grace: 200
  $name: 'Chevron: collapse delay (ms)'
  $description: How long the cursor must stay outside the area before the flyout closes (prevents flicker).
- pad: 4
  $name: 'Chevron: hit area padding (pixels)'
  $description: Enlarges the hover area around the chevron button.
- suppressInFullscreen: true
  $name: Do not activate over fullscreen apps
  $description: When a fullscreen app is in the foreground (e.g. a fullscreen video or a game), hovering will not open anything, so nothing can pop up over the content. Applies to the chevron and to every item that inherits it.
- hideTooltip: false
  $name: 'Chevron: hide its tooltip'
  $description: While the cursor is on the chevron, hide the tooltip Windows shows for it ("Show hidden icons", which can appear before the flyout does, and "Hide", which covers the bottom row of icons). This only ever touches the chevron's own tooltip; tray icon tooltips are never hidden.
- itemHoverDelay: 250
  $name: 'Items: hover delay (ms)'
  $description: Default for tray items. How long the cursor must rest on an icon before its popup is opened. Keep this comfortably above zero so that crossing the tray does not fire everything on the way.
- itemCloseDelay: 400
  $name: 'Items: close delay (ms)'
  $description: Default for tray items. How long the cursor must be away from the icon, the popup and the travel corridor before the popup is closed.
- itemTravelTimeout: 1500
  $name: 'Items: travel timeout (ms)'
  $description: Default for tray items. The longest the popup is held open while the cursor merely looks like it is heading towards it. This is the upper bound on the forgiving behaviour; 0 disables travel grace entirely.
- itemTravelPad: 48
  $name: 'Items: travel corridor padding (pixels)'
  $description: The corridor is the box spanning the icon and the popup, grown by this much. Outside it, the cursor is not considered to be travelling towards the popup.
- itemPad: 6
  $name: 'Items: icon hit area padding (pixels)'
  $description: Default for tray items. Enlarges the hover area around the icon.
- itemPopupPad: 24
  $name: 'Items: popup hit area padding (pixels)'
  $description: Default for tray items. Enlarges the popup area, which absorbs overshooting past its edge. It is doubled once the cursor has been inside the popup at least once.
- itemDiscoverMs: 1200
  $name: 'Items: popup discovery window (ms)'
  $description: Default for tray items. How long to keep watching for a popup after invoking an icon. Apps that are slow to show their window need more.
- itemAutoClose: true
  $name: 'Items: close the popup when the cursor leaves'
  $description: Default for tray items. Turn off to open popups on hover but never close them automatically.
- targets:
  - - enabled: false
      $name: Enabled
    - name: EarTrumpet
      $name: Label
      $description: Only used in the log, so that you can tell items apart.
    - matchName: EarTrumpet
      $name: Match name
      $description: Case-insensitive substring of the element's name, which for a notification icon is its tooltip text. Leave empty to ignore.
    - matchNameExact: false
      $name: Match the name exactly
      $description: Require the whole name to be equal instead of contained.
    - matchAutomationId: ""
      $name: Match AutomationId
      $description: Exact match, e.g. NotifyItemIcon for a notification icon. Leave empty to ignore.
    - matchClass: ""
      $name: Match class name
      $description: Exact match, e.g. SystemTray.NormalButton. Language-independent, so prefer it. Leave empty to ignore.
    - allowNonTray: false
      $name: Allow non-tray taskbar elements
      $description: By default only tray elements can be matched, so that a taskbar app button (whose name is its window title) can never be invoked by accident. Turn this on only to target something else on the taskbar, and only with a class name or AutomationId rule.
    - hoverDelay: -1
      $name: Hover delay (ms)
      $description: -1 inherits the global item default.
    - closeDelay: -1
      $name: Close delay (ms)
      $description: -1 inherits the global item default.
    - travelTimeout: -1
      $name: Travel timeout (ms)
      $description: -1 inherits the global item default.
    - pad: -1
      $name: Icon hit area padding (pixels)
      $description: -1 inherits the global item default.
    - popupPad: -1
      $name: Popup hit area padding (pixels)
      $description: -1 inherits the global item default.
    - discoverMs: -1
      $name: Popup discovery window (ms)
      $description: -1 inherits the global item default.
    - autoClose: inherit
      $name: Close the popup when the cursor leaves
      $options:
      - inherit: Inherit the global item default
      - "yes": Close it
      - "no": Leave it open
    - closeAction: toggle
      $name: How to close the popup
      $options:
      - toggle: Invoke the icon again, the way a second click would
      - none: Do not close it, let the app dismiss it
    - popupMatch: auto
      $name: Popup matching
      $options:
      - auto: Detect it automatically
      - process: Only a window of the process below
      - class: Only a window whose class contains the text below
    - popupProcess: ""
      $name: Popup process
      $description: Executable name, e.g. EarTrumpet.exe. Used when popup matching is set to process.
    - popupClass: ""
      $name: Popup window class
      $description: Case-insensitive substring of the window class. Used when popup matching is set to class.
    - suppressInFullscreen: inherit
      $name: Do not activate over fullscreen apps
      $options:
      - inherit: Inherit the global setting
      - "yes": Suppress
      - "no": Always activate
  - - enabled: false
      $name: Enabled
    - name: Quick Settings
      $name: Label
      $description: Only used in the log, so that you can tell items apart.
    - matchName: ""
      $name: Match name
      $description: Case-insensitive substring of the element's name, which for a notification icon is its tooltip text. Leave empty to ignore.
    - matchNameExact: false
      $name: Match the name exactly
      $description: Require the whole name to be equal instead of contained.
    - matchAutomationId: SystemTrayIcon
      $name: Match AutomationId
      $description: Exact match, e.g. NotifyItemIcon for a notification icon. Leave empty to ignore.
    - matchClass: SystemTray.OmniButtonCenter
      $name: Match class name
      $description: Exact match, e.g. SystemTray.NormalButton. Language-independent, so prefer it. Leave empty to ignore.
    - allowNonTray: false
      $name: Allow non-tray taskbar elements
      $description: By default only tray elements can be matched, so that a taskbar app button (whose name is its window title) can never be invoked by accident. Turn this on only to target something else on the taskbar, and only with a class name or AutomationId rule.
    - hoverDelay: -1
      $name: Hover delay (ms)
      $description: -1 inherits the global item default.
    - closeDelay: -1
      $name: Close delay (ms)
      $description: -1 inherits the global item default.
    - travelTimeout: -1
      $name: Travel timeout (ms)
      $description: -1 inherits the global item default.
    - pad: -1
      $name: Icon hit area padding (pixels)
      $description: -1 inherits the global item default.
    - popupPad: -1
      $name: Popup hit area padding (pixels)
      $description: -1 inherits the global item default.
    - discoverMs: -1
      $name: Popup discovery window (ms)
      $description: -1 inherits the global item default.
    - autoClose: inherit
      $name: Close the popup when the cursor leaves
      $options:
      - inherit: Inherit the global item default
      - "yes": Close it
      - "no": Leave it open
    - closeAction: toggle
      $name: How to close the popup
      $options:
      - toggle: Invoke the icon again, the way a second click would
      - none: Do not close it, let the app dismiss it
    - popupMatch: auto
      $name: Popup matching
      $options:
      - auto: Detect it automatically
      - process: Only a window of the process below
      - class: Only a window whose class contains the text below
    - popupProcess: ""
      $name: Popup process
      $description: Executable name, e.g. EarTrumpet.exe. Used when popup matching is set to process.
    - popupClass: ""
      $name: Popup window class
      $description: Case-insensitive substring of the window class. Used when popup matching is set to class.
    - suppressInFullscreen: inherit
      $name: Do not activate over fullscreen apps
      $options:
      - inherit: Inherit the global setting
      - "yes": Suppress
      - "no": Always activate
  $name: Tray items
  $description: Each item is matched and timed independently. Turn on "Log tray candidates" to see what to match on.
- keywords: ["hidden icons", "ukryte ikony", "verborgen pictogrammen", "ausgeblendete symbole", "icônes masquées", "iconos ocultos", "icone nascoste", "ícones ocultos", "skryté ikony", "rejtett ikonok", "pictograme ascunse", "dolda ikoner", "skjulte ikoner", "piilotetut kuvakkeet", "gizli simgeleri", "скрытые значки", "приховані піктограми", "κρυφών εικονιδίων", "隐藏的图标", "隱藏的圖示", "隠れている", "숨겨진 아이콘"]
  $name: 'Chevron: name keywords'
  $description: Only used when the chevron cannot be identified by its class name. Case-insensitive substrings, matched against tray buttons only. The most common Windows display languages are covered by default.
- chevronClass: SystemTray.NormalButton
  $name: 'Chevron: class name'
  $description: Primary, language-independent identification. The chevron is the only tray element that has this class name together with the AutomationId below. Change it if a future Windows build renames it.
- trayIconAutomationId: SystemTrayIcon
  $name: 'Chevron: AutomationId'
  $description: The AutomationId paired with the class name above to identify the chevron. It also tells tray elements apart from taskbar buttons. Change it if a future Windows build renames it.
- positionalFallback: false
  $name: 'Chevron: guess it by position as a last resort'
  $description: When the chevron matches neither the class name nor any keyword, assume it is the leftmost tray button. This is a guess and can select a different button, such as Quick Settings, so it is off by default and the mod simply does nothing instead.
- flyoutClass: TopLevelWindowForOverflowXamlIsland
  $name: 'Chevron: flyout window class'
  $description: Window class name of the opened hidden icons flyout, used to tell whether the cursor is over the icons. Change it if auto-collapse does not work.
- tooltipClass: Xaml_WindowedPopupClass
  $name: Tooltip window class
  $description: Window class of the chevron tooltip. Hidden only when "Chevron - hide its tooltip" is on, and also excluded when detecting an item's popup so that a tooltip is never mistaken for one.
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <shellapi.h>
#include <uiautomation.h>

#include <algorithm>
#include <atomic>
#include <climits>
#include <string>
#include <string_view>
#include <vector>

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

// Tri-state for a per-item override of a global boolean. Read as a string from
// the settings so that Windhawk can render it as a dropdown; a checkbox cannot
// express "inherit".
enum class Tri { Inherit, Off, On };

enum class CloseAction {
    Toggle,  // invoke the element a second time, the way another click would
    None,    // never close it; the app dismisses its own popup
};

enum class PopupMatch { Auto, Process, Class };

struct ItemConfig {
    bool enabled = false;
    std::wstring label;

    // Identification. Every non-empty rule must match, and exactly one element
    // must match overall, or the item is skipped.
    std::wstring matchName;  // stored lowercased
    bool matchNameExact = false;
    std::wstring matchAutomationId;
    std::wstring matchClass;
    bool allowNonTray = false;

    // -1 inherits the corresponding global default.
    int hoverDelay = -1;
    int closeDelay = -1;
    int travelTimeout = -1;
    int pad = -1;
    int popupPad = -1;
    int discoverMs = -1;
    Tri autoClose = Tri::Inherit;
    Tri suppressInFullscreen = Tri::Inherit;

    CloseAction closeAction = CloseAction::Toggle;
    PopupMatch popupMatch = PopupMatch::Auto;
    std::wstring popupProcess;  // stored lowercased
    std::wstring popupClass;    // stored lowercased
};

struct Settings {
    bool enabled = true;
    int pollInterval = 50;
    bool diagnostics = false;

    // Chevron. These keep the names, meanings and defaults of the mod this is
    // forked from, so its behaviour is bit-for-bit the same out of the box.
    bool chevronEnabled = true;
    bool autoClose = true;
    int hoverDelay = 0;
    int grace = 200;
    int pad = 4;
    bool hideTooltip = false;
    bool positionalFallback = false;
    std::wstring flyoutClass = L"TopLevelWindowForOverflowXamlIsland";
    std::wstring tooltipClass = L"Xaml_WindowedPopupClass";
    std::wstring trayIconAutomationId = L"SystemTrayIcon";
    std::wstring chevronClass = L"SystemTray.NormalButton";

    // Shared by the chevron and by every item that inherits it.
    bool suppressInFullscreen = true;

    // Defaults for tray items.
    int itemHoverDelay = 250;
    int itemCloseDelay = 400;
    int itemTravelTimeout = 1500;
    int itemTravelPad = 48;
    int itemPad = 6;
    int itemPopupPad = 24;
    int itemDiscoverMs = 1200;
    bool itemAutoClose = true;

    std::vector<ItemConfig> items;

    // Largest hit-area padding any target can use, computed in LoadSettings.
    // The "cursor is at the taskbar" band must be at least this wide, or there
    // would be a position the mod treats as "on an element" but not as "at the
    // taskbar", where it then refuses to refresh anything.
    int maxPad = 64;

    // Fragments of the chevron's name ("Show hidden icons") across the most
    // common Windows display languages. Each entry is a distinctive part of the
    // name rather than the whole string, so wording differences between builds
    // still match.
    std::vector<std::wstring> keywords = {
        L"hidden icons",            // English
        L"ukryte ikony",            // Polish
        L"verborgen pictogrammen",  // Dutch
        L"ausgeblendete symbole",   // German
        L"icônes masquées",         // French
        L"iconos ocultos",          // Spanish
        L"icone nascoste",          // Italian
        L"ícones ocultos",          // Portuguese
        L"skryté ikony",            // Czech, Slovak
        L"rejtett ikonok",          // Hungarian
        L"pictograme ascunse",      // Romanian
        L"dolda ikoner",            // Swedish
        L"skjulte ikoner",          // Danish, Norwegian
        L"piilotetut kuvakkeet",    // Finnish
        L"gizli simgeleri",         // Turkish
        L"скрытые значки",          // Russian
        L"приховані піктограми",    // Ukrainian
        L"κρυφών εικονιδίων",       // Greek
        L"隐藏的图标",              // Chinese (Simplified)
        L"隱藏的圖示",              // Chinese (Traditional)
        L"隠れている",              // Japanese
        L"숨겨진 아이콘"            // Korean
    };
    // Lowercased copy of `keywords`, built once in LoadSettings so that name
    // matching does not lowercase every keyword for every candidate element.
    std::vector<std::wstring> keywordsLower;
};

// UIA class names of tray elements all share this prefix, which is what makes
// it possible to exclude the rest of the taskbar from the search. Every tray
// element's class name starts with this; taskbar app buttons do not.
static constexpr std::wstring_view TRAY_CLASS_PREFIX = L"SystemTray.";

// Some builds are reported to expose the chevron with this AutomationId instead
// of the configured one. Accepting both costs nothing, because the class name
// plus AutomationId pair is only used when exactly one element matches it.
static constexpr std::wstring_view CHEVRON_AUTOMATION_ID_ALT = L"ChevronButton";

// g_settings is guarded by g_settingsLock; the worker thread keeps a private
// snapshot and refreshes it when g_settingsGeneration changes, so it never
// reads the strings while the settings thread reassigns them.
static Settings g_settings;
static SRWLOCK g_settingsLock = SRWLOCK_INIT;
static std::atomic<int> g_settingsGeneration{0};

static std::atomic<bool> g_running{false};
static HANDLE g_thread = nullptr;
static HANDLE g_stopEvent = nullptr;

static Settings GetSettingsSnapshot() {
    AcquireSRWLockShared(&g_settingsLock);
    Settings s = g_settings;
    ReleaseSRWLockShared(&g_settingsLock);
    return s;
}

// ---------------------------------------------------------------------------
// Small helpers
// ---------------------------------------------------------------------------

// CharLowerBuffW is used instead of towlower because the latter follows the C
// locale and would leave non-ASCII letters (Cyrillic, Greek, ...) untouched,
// breaking case-insensitive matching for those locales.
static std::wstring ToLower(std::wstring r) {
    if (!r.empty()) {
        CharLowerBuffW(&r[0], (DWORD)r.size());
    }
    return r;
}

static bool NameMatchesKeyword(const std::wstring& nameLower, const Settings& s) {
    for (const auto& k : s.keywordsLower) {
        if (!k.empty() && nameLower.find(k) != std::wstring::npos) return true;
    }
    return false;
}

static bool PtInRectPad(const RECT& r, POINT pt, int pad) {
    return pt.x >= r.left - pad && pt.x <= r.right + pad && pt.y >= r.top - pad &&
           pt.y <= r.bottom + pad;
}

static bool RectValid(const RECT& r) {
    return r.right > r.left && r.bottom > r.top;
}

static RECT UnionRect(const RECT& a, const RECT& b) {
    RECT r;
    r.left = std::min(a.left, b.left);
    r.top = std::min(a.top, b.top);
    r.right = std::max(a.right, b.right);
    r.bottom = std::max(a.bottom, b.bottom);
    return r;
}

// Manhattan distance from a point to a rectangle, 0 inside. Cheap and good
// enough: it is only ever compared against itself to answer "is the cursor
// getting closer", never used as a real length.
static int DistToRect(const RECT& r, POINT p) {
    int dx = 0, dy = 0;
    if (p.x < r.left) dx = r.left - p.x;
    else if (p.x > r.right) dx = p.x - r.right;
    if (p.y < r.top) dy = r.top - p.y;
    else if (p.y > r.bottom) dy = p.y - r.bottom;
    return dx + dy;
}

static int Inherit(int perItem, int global) {
    return perItem < 0 ? global : perItem;
}

static bool Inherit(Tri perItem, bool global) {
    return perItem == Tri::Inherit ? global : (perItem == Tri::On);
}

static std::wstring ClassOf(HWND h) {
    WCHAR buf[160] = L"";
    GetClassNameW(h, buf, ARRAYSIZE(buf));
    return buf;
}

static std::wstring ProcessNameOf(DWORD pid) {
    HANDLE p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!p) return L"";
    WCHAR buf[MAX_PATH] = L"";
    DWORD sz = ARRAYSIZE(buf);
    if (!QueryFullProcessImageNameW(p, 0, buf, &sz)) buf[0] = 0;
    CloseHandle(p);
    std::wstring s = buf;
    size_t k = s.find_last_of(L'\\');
    return ToLower(k == std::wstring::npos ? s : s.substr(k + 1));
}

// ---------------------------------------------------------------------------
// UIA pattern helpers
// ---------------------------------------------------------------------------

// Invoking a tray element is the same operation whether it opens, or (for a
// toggle such as the chevron or a notification icon) closes. ExpandCollapse is
// preferred where the element supports it; on Windows 11 tray buttons it does
// not, and Invoke is what actually works.
static void DoInvoke(IUIAutomationElement* e, bool collapse) {
    IUIAutomationExpandCollapsePattern* p = nullptr;
    if (SUCCEEDED(e->GetCurrentPatternAs(UIA_ExpandCollapsePatternId,
                                         __uuidof(IUIAutomationExpandCollapsePattern),
                                         (void**)&p)) &&
        p) {
        if (collapse) {
            p->Collapse();
        } else {
            p->Expand();
        }
        p->Release();
        return;
    }
    // No ExpandCollapse support: a second Invoke toggles the popup closed.
    IUIAutomationInvokePattern* inv = nullptr;
    if (SUCCEEDED(e->GetCurrentPatternAs(UIA_InvokePatternId,
                                         __uuidof(IUIAutomationInvokePattern), (void**)&inv)) &&
        inv) {
        inv->Invoke();
        inv->Release();
    }
}

// ---------------------------------------------------------------------------
// Window helpers
// ---------------------------------------------------------------------------

// Cheap (pure Win32) resolution of the hidden icons flyout — no UIA calls. The
// chevron does not support the ExpandCollapse pattern, so the flyout window's
// visibility is the only reliable state signal. Only windows owned by the
// taskbar count: the class is user-settable, and pointing it at a more generic
// class must not make the mod track another application's window.
static HWND GetVisibleFlyout(const Settings& s, DWORD taskbarPid) {
    HWND h = nullptr;
    while ((h = FindWindowExW(nullptr, h, s.flyoutClass.c_str(), nullptr))) {
        if (!IsWindowVisible(h)) continue;
        if (taskbarPid) {
            DWORD pid = 0;
            GetWindowThreadProcessId(h, &pid);
            if (pid != taskbarPid) continue;
        }
        return h;
    }
    return nullptr;
}

// A tray icon's context menu is a separate window that usually extends past the
// popup, so the cursor sitting on it counts as having left. Menus opened with
// TrackPopupMenu, which is what icons using the classic tray API use, all share
// this window class, so their presence is a reliable "the user is still busy"
// signal even when the click that opened the menu was never observed.
static HWND GetVisiblePopupMenu() {
    // Enumerate rather than sampling the first hit: menu windows are created per
    // thread and kept alive hidden afterwards, so the first one in Z-order is
    // often a leftover from a process that showed a menu earlier.
    HWND h = nullptr;
    while ((h = FindWindowExW(nullptr, h, L"#32768", nullptr))) {
        if (IsWindowVisible(h)) return h;
    }
    return nullptr;
}

static bool PtOverWindow(HWND hwnd, POINT pt, int pad) {
    if (!hwnd) return false;
    RECT r;
    if (!GetWindowRect(hwnd, &r)) return false;
    return PtInRectPad(r, pt, pad);
}

// True when the foreground window covers the whole monitor that `anchor` is on,
// i.e. a fullscreen app (a fullscreen video, a game) whose content a popup would
// cover. Maximized windows stop at the work area and therefore do not match, and
// a fullscreen app on another monitor does not suppress anything here. The
// desktop and shell windows are excluded so an empty desktop is not mistaken for
// a fullscreen app.
static bool IsFullscreenOver(const RECT& anchor) {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd || hwnd == GetShellWindow()) return false;

    std::wstring cls = ClassOf(hwnd);
    if (cls == L"Shell_TrayWnd" || cls == L"Shell_SecondaryTrayWnd" || cls == L"Progman" ||
        cls == L"WorkerW") {
        return false;
    }

    HMONITOR mon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    if (mon != MonitorFromRect(&anchor, MONITOR_DEFAULTTONEAREST)) return false;

    RECT wr;
    if (!GetWindowRect(hwnd, &wr)) return false;
    MONITORINFO mi = {sizeof(mi)};
    if (!GetMonitorInfoW(mon, &mi)) return false;
    return wr.left <= mi.rcMonitor.left && wr.top <= mi.rcMonitor.top &&
           wr.right >= mi.rcMonitor.right && wr.bottom >= mi.rcMonitor.bottom;
}

// Hide the chevron's tooltip, which Windows pops up over it and which can cover
// the bottom row of the flyout's icons. The tooltip class is the generic WinUI
// popup host used by many apps, so only windows owned by the taskbar's own
// process and positioned over the chevron are eligible — never popups elsewhere
// on screen, and never a tray icon's own tooltip.
static void HideChevronTooltip(const Settings& s, const RECT& chevron, HWND flyout,
                               DWORD taskbarPid) {
    if (!taskbarPid) return;
    RECT fly{};
    bool hasFlyout = flyout && GetWindowRect(flyout, &fly);

    HWND h = nullptr;
    while ((h = FindWindowExW(nullptr, h, s.tooltipClass.c_str(), nullptr))) {
        if (!IsWindowVisible(h) || h == flyout) continue;
        DWORD pid = 0;
        GetWindowThreadProcessId(h, &pid);
        if (pid != taskbarPid) continue;
        RECT r;
        if (!GetWindowRect(h, &r)) continue;
        // A tooltip is a line or two of text. The class is the generic WinUI
        // popup host, which also hosts real flyout content, so anything bigger
        // than a tooltip is left alone -- hiding the flyout's own content window
        // would break the thing this mod just opened.
        if (r.right - r.left > 600 || r.bottom - r.top > 120) continue;
        // Horizontally aligned with the chevron, and either overlapping the
        // flyout (the "Hide" tooltip shown while it is open) or sitting right
        // next to the chevron (the "Show hidden icons" tooltip, which can beat
        // the flyout to the screen). Both tests are direction-agnostic, so they
        // hold with the taskbar at the bottom and at the top.
        bool overlapsX = r.left <= chevron.right && r.right >= chevron.left;
        bool overlapsFlyout = hasFlyout && r.left < fly.right && r.right > fly.left &&
                              r.top < fly.bottom && r.bottom > fly.top;
        LONG band = chevron.bottom - chevron.top;
        LONG gapAbove = chevron.top - r.bottom;
        LONG gapBelow = r.top - chevron.bottom;
        bool besideChevron =
            (gapAbove >= 0 && gapAbove <= band) || (gapBelow >= 0 && gapBelow <= band);
        if (overlapsX && (overlapsFlyout || besideChevron)) {
            // Async: the window belongs to explorer, and the synchronous form
            // marshals into its UI thread and blocks until that thread handles
            // it. Hiding a tooltip a frame later is not noticeable; stalling the
            // poll loop behind a busy shell is.
            ShowWindowAsync(h, SW_HIDE);
        }
    }
}

// ---------------------------------------------------------------------------
// Popup discovery
// ---------------------------------------------------------------------------

struct WindowInfo {
    HWND hwnd;
    RECT rect;
    DWORD pid;
    std::wstring cls;
};

static std::vector<WindowInfo> g_enumScratch;

static BOOL CALLBACK CollectWindowProc(HWND h, LPARAM) {
    if (!IsWindowVisible(h)) return TRUE;
    RECT r;
    if (!GetWindowRect(h, &r) || !RectValid(r)) return TRUE;
    WindowInfo w;
    w.hwnd = h;
    w.rect = r;
    w.pid = 0;
    GetWindowThreadProcessId(h, &w.pid);
    w.cls = ClassOf(h);
    g_enumScratch.push_back(std::move(w));
    return TRUE;
}

static std::vector<WindowInfo> VisibleTopLevelWindows() {
    g_enumScratch.clear();
    EnumWindows(CollectWindowProc, 0);
    return std::move(g_enumScratch);
}

// Windows that can never be a tray icon's popup. Rejecting these is what keeps
// automatic detection from grabbing something unrelated.
static bool IsNeverAPopup(const WindowInfo& w, const Settings& s, DWORD selfPid) {
    if (w.pid == selfPid) return true;
    if (w.cls == L"Shell_TrayWnd" || w.cls == L"Shell_SecondaryTrayWnd" || w.cls == L"Progman" ||
        w.cls == L"WorkerW" || w.cls == L"#32769") {
        return true;
    }
    // A tooltip is not a popup, and on Windows 11 it shares its window class
    // with real XAML flyouts, so size alone would not separate them.
    if (w.cls == s.tooltipClass || w.cls == L"tooltips_class32") return true;

    LONG cx = w.rect.right - w.rect.left;
    LONG cy = w.rect.bottom - w.rect.top;
    if (cx < 48 || cy < 32) return true;  // tooltips, helper windows, 1x1 stubs

    // A window covering its whole monitor is an application, not a flyout.
    MONITORINFO mi = {sizeof(mi)};
    HMONITOR mon = MonitorFromRect(&w.rect, MONITOR_DEFAULTTONEAREST);
    if (mon && GetMonitorInfoW(mon, &mi)) {
        if (w.rect.left <= mi.rcMonitor.left && w.rect.top <= mi.rcMonitor.top &&
            w.rect.right >= mi.rcMonitor.right && w.rect.bottom >= mi.rcMonitor.bottom) {
            return true;
        }
    }
    return false;
}

static bool PopupRulesAllow(const ItemConfig& c, const WindowInfo& w) {
    switch (c.popupMatch) {
        case PopupMatch::Process:
            if (c.popupProcess.empty()) return false;
            return ProcessNameOf(w.pid) == c.popupProcess;
        case PopupMatch::Class:
            if (c.popupClass.empty()) return false;
            return ToLower(w.cls).find(c.popupClass) != std::wstring::npos;
        case PopupMatch::Auto:
        default:
            return true;
    }
}

// ---------------------------------------------------------------------------
// Targets
// ---------------------------------------------------------------------------

// One state machine per target, so that two items can be in different states at
// the same time and neither can corrupt the other.
//
//            cursor rests on the icon for hoverDelay
//   Idle ----------------------------------------------> Discover
//     ^                                                     |
//     |  no popup within discoverMs                         | popup found
//     +-----------------------------------------------------+
//     |                                                     v
//     |   ShouldClose() says the user is done             Open
//     +-----------------------------------------------------+
//         (or the popup went away on its own)
//
// The chevron takes a shortcut: its popup is a known window class owned by the
// taskbar, so it never enters Discover -- seeing that window is the state.
enum class St {
    Idle,      // nothing open; waiting for the cursor to rest on the element
    Discover,  // invoked, watching for the popup to appear
    Open,      // popup known and being tracked
};

struct Target {
    bool isChevron = false;
    ItemConfig cfg;  // unused fields for the chevron; its settings are global

    // Resolved element.
    IUIAutomationElement* el = nullptr;
    RECT rect{};
    bool haveRect = false;
    LONG maxWidth = 0;  // clamp learned at find time, see ClampCandidates
    ULONGLONG nextRectRefresh = 0;

    // State machine.
    St state = St::Idle;
    bool armed = true;  // a stay may fire; cleared until the cursor leaves
    ULONGLONG dwellSince = 0;
    ULONGLONG openedAt = 0;
    ULONGLONG cooldownUntil = 0;

    // Popup tracking.
    HWND popup = nullptr;
    RECT popupRect{};
    DWORD popupPid = 0;
    bool userEngaged = false;   // clicked inside; auto-close suspended
    bool everInsidePopup = false;
    // Set the moment the mod invokes to close. Closing is a toggle, so a toggle
    // that lands after the popup has already gone re-opens it; without this the
    // mod would then see it open, close it again, and cycle forever. Cleared
    // only by actually observing the popup gone, or by the cursor returning to
    // the icon, which is a fresh interaction.
    bool suppressUntilHidden = false;
    ULONGLONG lastEngagedAt = 0;
    ULONGLONG leftIconAt = 0;
    int bestDist = INT_MAX;

    // Learned from the first successful open, so that a popup the user opened by
    // clicking is never toggled shut by a later hover.
    std::wstring learnedClass;
    DWORD learnedPid = 0;

    // Snapshot taken immediately before this target's invoke, so that discovery
    // can tell a genuinely new window from one that was already on screen. Per
    // target, because two items can be in discovery at the same time.
    std::vector<WindowInfo> windowsBefore;
    HWND fgBefore = nullptr;

    // Diagnostics, logged once per condition rather than on every walk.
    bool loggedNoMatch = false;
    bool loggedAmbiguous = false;

    void ResetInteraction() {
        state = St::Idle;
        armed = true;
        dwellSince = 0;
        popup = nullptr;
        popupRect = {};
        popupPid = 0;
        userEngaged = false;
        everInsidePopup = false;
        lastEngagedAt = 0;
        leftIconAt = 0;
        bestDist = INT_MAX;
        windowsBefore.clear();
        windowsBefore.shrink_to_fit();
        fgBefore = nullptr;
    }

    void DropElement() {
        if (el) {
            el->Release();
            el = nullptr;
        }
        haveRect = false;
        maxWidth = 0;
    }

    const wchar_t* Label() const {
        if (isChevron) return L"chevron";
        return cfg.label.empty() ? L"item" : cfg.label.c_str();
    }
};

// ---------------------------------------------------------------------------
// Finding elements
// ---------------------------------------------------------------------------

struct Candidate {
    IUIAutomationElement* el;
    std::wstring className;
    std::wstring automationId;
    std::wstring name;
    std::wstring nameLower;
    RECT rect;
    bool isTray;
    bool taken;
};

// Windows 11 build 26340 reports a bogus bounding rectangle for a notification
// icon: the element's left edge is right, but its width spans the whole rest of
// the tray (measured: 824px for a 40px slot), overlapping the clock, the volume
// button and everything else. Hit-testing that rectangle would fire the wrong
// item across half the taskbar.
//
// The tray is a flat, horizontally ordered strip, so the fix does not need to
// know which builds are affected: clip every candidate at the left edge of the
// next candidate that starts after it. Clipping can only ever shrink a hit area,
// so a build with correct rectangles is unaffected.
// Clipping only ever shrinks a hit area, so a build with correct rectangles is
// unaffected.
//
// Row-aware, and that is not optional. The taskbar is one horizontal strip, but
// the opened hidden-icons flyout is a *grid* of icons floating above it. Without
// the vertical-overlap test, a flyout icon at x=2223 clips the chevron
// underneath it at x=2219 down to 8 pixels wide the moment the flyout opens —
// which then breaks the chevron's own hit test, invalidates its cached
// rectangle, and puts the mod into a re-find storm.
static void ClampCandidates(std::vector<Candidate>& cands) {
    for (auto& c : cands) {
        LONG right = c.rect.right;
        for (const auto& o : cands) {
            if (&o == &c) continue;
            // Different row: not a neighbour, cannot clip.
            if (o.rect.bottom <= c.rect.top || o.rect.top >= c.rect.bottom) continue;
            if (o.rect.left > c.rect.left + 4 && o.rect.left < right) right = o.rect.left;
        }
        if (right - c.rect.left >= 8) c.rect.right = right;
    }
}

static void CollectCandidates(IUIAutomation* pAuto, HWND root, const Settings& s,
                              std::vector<Candidate>& out) {
    IUIAutomationElement* pRoot = nullptr;
    if (!root || FAILED(pAuto->ElementFromHandle(root, &pRoot)) || !pRoot) return;

    IUIAutomationCondition* pCond = nullptr;
    VARIANT v;
    VariantInit(&v);
    v.vt = VT_I4;
    v.lVal = UIA_ButtonControlTypeId;
    pAuto->CreatePropertyCondition(UIA_ControlTypePropertyId, v, &pCond);

    // Reading five properties per button one at a time is one cross-process call
    // each, i.e. a few hundred round trips per walk. A cache request collapses
    // them into the single FindAllBuildCache call. The default element mode is
    // Full, so the returned elements still support GetCurrentPatternAs, which is
    // what DoInvoke needs.
    IUIAutomationCacheRequest* pCache = nullptr;
    if (SUCCEEDED(pAuto->CreateCacheRequest(&pCache)) && pCache) {
        pCache->AddProperty(UIA_ClassNamePropertyId);
        pCache->AddProperty(UIA_AutomationIdPropertyId);
        pCache->AddProperty(UIA_NamePropertyId);
        pCache->AddProperty(UIA_IsOffscreenPropertyId);
        pCache->AddProperty(UIA_BoundingRectanglePropertyId);
    }
    const bool cached = (pCache != nullptr);

    IUIAutomationElementArray* pArr = nullptr;
    HRESULT hr = pCond ? (cached ? pRoot->FindAllBuildCache(TreeScope_Subtree, pCond, pCache, &pArr)
                                 : pRoot->FindAll(TreeScope_Subtree, pCond, &pArr))
                       : E_FAIL;
    if (SUCCEEDED(hr) && pArr) {
        int n = 0;
        pArr->get_Length(&n);
        for (int i = 0; i < n; i++) {
            IUIAutomationElement* e = nullptr;
            if (FAILED(pArr->GetElement(i, &e)) || !e) continue;

            BSTR b = nullptr;
            if (cached) e->get_CachedClassName(&b); else e->get_CurrentClassName(&b);
            std::wstring className = b ? b : L"";
            if (b) SysFreeString(b);

            b = nullptr;
            if (cached) e->get_CachedAutomationId(&b); else e->get_CurrentAutomationId(&b);
            std::wstring automationId = b ? b : L"";
            if (b) SysFreeString(b);

            // Elements that are not rendered report an empty {0,0,0,0}
            // rectangle, which would otherwise pass every geometric test.
            BOOL offscreen = FALSE;
            RECT r{};
            HRESULT hrOff = cached ? e->get_CachedIsOffscreen(&offscreen)
                                   : e->get_CurrentIsOffscreen(&offscreen);
            HRESULT hrRect = cached ? e->get_CachedBoundingRectangle(&r)
                                    : e->get_CurrentBoundingRectangle(&r);
            if (FAILED(hrOff) || offscreen || FAILED(hrRect) || !RectValid(r)) {
                e->Release();
                continue;
            }

            b = nullptr;
            if (cached) e->get_CachedName(&b); else e->get_CurrentName(&b);
            std::wstring name = b ? b : L"";
            if (b) SysFreeString(b);

            Candidate c;
            c.el = e;
            c.className = className;
            c.automationId = automationId;
            c.name = name;
            c.nameLower = ToLower(name);
            c.rect = r;
            // Only tray elements are eligible by default; everything else on the
            // taskbar is discarded before any name test is applied. Without this,
            // a task list button (named after its window title) could match a
            // keyword or a user's substring rule and then be invoked.
            c.isTray = className.starts_with(TRAY_CLASS_PREFIX) ||
                       automationId == s.trayIconAutomationId ||
                       automationId == CHEVRON_AUTOMATION_ID_ALT;
            c.taken = false;
            out.push_back(std::move(c));
        }
        pArr->Release();
    }
    if (pCache) pCache->Release();
    if (pCond) pCond->Release();
    pRoot->Release();
}

// Three-step chevron detection, most reliable first, unchanged from the mod this
// is forked from:
//  1) class name + AutomationId, which is language-independent: the chevron is
//     the only tray element that is a `chevronClass` button carrying
//     `trayIconAutomationId`. Notification icons share the class but use a
//     different AutomationId (NotifyItemIcon on 26340), and the clock, volume,
//     network, battery and "show desktop" buttons share the AutomationId but use
//     other classes.
//  2) match by name, for builds where those class names change.
//  3) nothing. Invoking an unidentified tray button would open whatever it
//     happens to be, which is how the chevron was mistaken for Quick Settings in
//     the field, so guessing is opt-in only.
static int PickChevron(const std::vector<Candidate>& cands, const Settings& s, bool* logWeakMatch) {
    int classMatches = 0, firstClassMatch = -1;
    for (size_t i = 0; i < cands.size(); i++) {
        if (cands[i].taken || !cands[i].isTray) continue;
        if (cands[i].className == s.chevronClass &&
            (cands[i].automationId == s.trayIconAutomationId ||
             cands[i].automationId == CHEVRON_AUTOMATION_ID_ALT)) {
            classMatches++;
            if (firstClassMatch < 0) firstClassMatch = (int)i;
        }
    }
    // Accepted only when unambiguous. If a future build gives several tray
    // elements this signature, picking one of them would be a guess.
    if (classMatches == 1) return firstClassMatch;
    if (classMatches > 1 && logWeakMatch) {
        *logWeakMatch = true;
        Wh_Log(L"%d tray elements share the chevron signature, falling back to name",
               classMatches);
    }

    for (size_t i = 0; i < cands.size(); i++) {
        if (cands[i].taken || !cands[i].isTray) continue;
        if (NameMatchesKeyword(cands[i].nameLower, s)) {
            if (logWeakMatch) {
                *logWeakMatch = true;
                Wh_Log(L"Chevron matched by name, not by class name");
            }
            return (int)i;
        }
    }

    if (s.positionalFallback) {
        int chosen = -1;
        for (size_t i = 0; i < cands.size(); i++) {
            if (cands[i].taken || !cands[i].isTray) continue;
            if (cands[i].automationId != s.trayIconAutomationId &&
                cands[i].automationId != CHEVRON_AUTOMATION_ID_ALT) {
                continue;
            }
            if (chosen < 0 || cands[i].rect.left < cands[chosen].rect.left) chosen = (int)i;
        }
        if (chosen >= 0 && logWeakMatch) {
            *logWeakMatch = true;
            Wh_Log(L"Chevron guessed by position: name=%s class=%s x=%d",
                   cands[chosen].name.c_str(), cands[chosen].className.c_str(),
                   (int)cands[chosen].rect.left);
        }
        return chosen;
    }
    return -1;
}

// An item matches a candidate when every rule it specifies matches. `*count`
// receives how many candidates matched, because anything other than exactly one
// means the item is skipped: zero is not identified, and more than one would be
// a coin flip over which tray button gets invoked.
static int PickItem(const std::vector<Candidate>& cands, const ItemConfig& c, int* count) {
    *count = 0;
    int first = -1;
    if (c.matchName.empty() && c.matchAutomationId.empty() && c.matchClass.empty()) {
        return -1;  // no rules at all is never a match
    }
    for (size_t i = 0; i < cands.size(); i++) {
        const Candidate& k = cands[i];
        if (k.taken) continue;
        if (!k.isTray && !c.allowNonTray) continue;
        if (!c.matchClass.empty() && k.className != c.matchClass) continue;
        if (!c.matchAutomationId.empty() && k.automationId != c.matchAutomationId) continue;
        if (!c.matchName.empty()) {
            if (c.matchNameExact) {
                if (k.nameLower != c.matchName) continue;
            } else if (k.nameLower.find(c.matchName) == std::wstring::npos) {
                continue;
            }
        }
        (*count)++;
        if (first < 0) first = (int)i;
    }
    return *count == 1 ? first : -1;
}

// One taskbar walk serves every target that currently has no element. Returns
// true when at least one target was resolved.
// diagSignature, when given, holds a hash of the candidate table from the last
// dump. The table is only logged when it differs, so leaving diagnostics on does
// not push a screenful of identical lines into the log every few seconds.
static bool ResolveTargets(IUIAutomation* pAuto, const Settings& s, std::vector<Target>& targets,
                           HWND taskbar, HWND flyout, bool logCandidates, bool* logged,
                           bool* logWeakMatch, size_t* diagSignature = nullptr) {
    std::vector<Candidate> cands;
    CollectCandidates(pAuto, taskbar, s, cands);
    // Icons living inside the opened hidden-icons flyout are in a different
    // window, so they are only reachable while it is open. Elements found there
    // go stale when it closes, which the rectangle guard already handles.
    if (flyout) CollectCandidates(pAuto, flyout, s, cands);

    ClampCandidates(cands);

    if (logCandidates && !cands.empty() && diagSignature) {
        // Class and AutomationId only. Names carry live text -- the clock's name
        // is the current time, EarTrumpet's is the current volume, the battery's
        // is the current charge -- so hashing them would mark the table as
        // changed on every single walk and defeat the whole point. Rectangles
        // jitter by a pixel for the same reason.
        std::wstring sig;
        for (const auto& c : cands) {
            sig += c.className;
            sig += L'|';
            sig += c.automationId;
            sig += L'\n';
        }
        size_t h = std::hash<std::wstring>{}(sig);
        if (h == *diagSignature) {
            logCandidates = false;
        } else {
            *diagSignature = h;
        }
    }

    if (logCandidates && !cands.empty()) {
        if (logged) *logged = true;
        Wh_Log(L"--- %d tray candidates ---", (int)cands.size());
        for (size_t i = 0; i < cands.size(); i++) {
            Wh_Log(L"  [%d] %s class=%s aid=%s rect=(%d,%d %dx%d) name=%s", (int)i,
                   cands[i].isTray ? L"tray    " : L"non-tray", cands[i].className.c_str(),
                   cands[i].automationId.c_str(), (int)cands[i].rect.left, (int)cands[i].rect.top,
                   (int)(cands[i].rect.right - cands[i].rect.left),
                   (int)(cands[i].rect.bottom - cands[i].rect.top), cands[i].name.c_str());
        }
    }

    // Take the candidates that resolved targets are already driving out of the
    // pool. Without this, a second item with looser rules could match the same
    // tray element and two targets would invoke it independently. Identity is by
    // rectangle: comparing elements would mean a COM call per pair, and two
    // distinct tray buttons never occupy the same rectangle.
    for (const auto& t : targets) {
        if (!t.el || !t.haveRect) continue;
        for (auto& c : cands) {
            if (!c.taken && EqualRect(&c.rect, &t.rect)) {
                c.taken = true;
                break;
            }
        }
    }

    bool resolvedAny = false;
    for (auto& t : targets) {
        if (t.el) continue;
        int idx = -1;
        if (t.isChevron) {
            if (!s.chevronEnabled) continue;
            idx = PickChevron(cands, s, logWeakMatch);
        } else {
            if (!t.cfg.enabled) continue;
            int count = 0;
            idx = PickItem(cands, t.cfg, &count);
            if (idx < 0) {
                if (count > 1 && !t.loggedAmbiguous) {
                    t.loggedAmbiguous = true;
                    Wh_Log(L"'%s' matches %d tray elements, skipped. Narrow the rules.",
                           t.Label(), count);
                } else if (count == 0 && !t.loggedNoMatch) {
                    t.loggedNoMatch = true;
                    Wh_Log(L"'%s' matched no tray element, skipped.", t.Label());
                }
            }
        }
        if (idx < 0) continue;

        cands[idx].taken = true;
        t.el = cands[idx].el;
        cands[idx].el = nullptr;
        t.rect = cands[idx].rect;
        t.maxWidth = t.rect.right - t.rect.left;
        t.haveRect = true;
        t.nextRectRefresh = 0;
        t.loggedNoMatch = false;
        t.loggedAmbiguous = false;
        resolvedAny = true;
        Wh_Log(L"'%s' resolved: class=%s aid=%s rect=(%d,%d %dx%d)", t.Label(),
               cands[idx].className.c_str(), cands[idx].automationId.c_str(), (int)t.rect.left,
               (int)t.rect.top, (int)(t.rect.right - t.rect.left),
               (int)(t.rect.bottom - t.rect.top));
    }

    for (auto& c : cands) {
        if (c.el) c.el->Release();
    }
    return resolvedAny;
}

// ---------------------------------------------------------------------------
// Worker
// ---------------------------------------------------------------------------

static const ULONGLONG ACTION_COOLDOWN_MS = 300;
// Retry the (expensive) element lookup quickly while the cursor is at the
// taskbar, and not at all while it is elsewhere, since nothing can be hovered
// from there anyway.
static const ULONGLONG REFIND_NEAR_MS = 250;
static const ULONGLONG REFIND_MAX_MS = 4000;
// How far from the taskbar the cursor still counts as "at the taskbar". Covers
// the sliver an auto-hidden taskbar leaves at the screen edge while it slides in.
static const int TASKBAR_REVEAL_PAD = 32;
static const ULONGLONG RECT_REFRESH_MS = 750;
static const ULONGLONG IDLE_STATE_CHECK_MS = 500;
static const ULONGLONG DIAGNOSTIC_INTERVAL_MS = 5000;
// The chevron has no discovery state -- seeing its flyout window is its state --
// so between invoking it and its flyout actually appearing there is a window in
// which it still looks closed. The cursor leaving the chevron on its way to the
// flyout re-arms the hover, and without this guard the next tick invokes again
// and toggles the flyout straight back shut. Observed live as four consecutive
// OPEN lines with no CLOSE between them.
static const ULONGLONG CHEVRON_REOPEN_GUARD_MS = 1000;
// Jitter allowance when deciding whether the cursor is still approaching the
// popup. A hand does not move in a straight line, and the poll interval samples
// it coarsely.
static const int TRAVEL_SLACK = 16;

struct TickContext {
    ULONGLONG now;
    POINT pt;
    const Settings* s;
    DWORD selfPid;
    bool anyButtonDown;
    bool pressedSinceTick;
    HWND menu;           // a visible TrackPopupMenu window, or null
    HWND underRoot;      // top-level window under the cursor, or null
    DWORD underRootPid;  // its process, for the occlusion test
};

// Per-tick engagement and close decision for a target whose popup is open. This
// is the whole "help the cursor travel from the icon to the popup" model:
//
//   engaged      -> the close timer is held at now
//   travelling   -> also holds it, but only inside the corridor, only while the
//                   cursor is not getting further from the popup, and only up to
//                   the travel timeout
//   otherwise    -> the close timer runs, and the popup closes after closeDelay
//
// So the effective grace is closeDelay when walking away and up to
// travelTimeout + closeDelay when actually heading for the popup, with a hard
// upper bound either way.
static bool ShouldClose(Target& t, const TickContext& ctx, bool overIcon) {
    const Settings& s = *ctx.s;
    int popupPad = t.isChevron ? 0 : Inherit(t.cfg.popupPad, s.itemPopupPad);
    int travelTimeout = t.isChevron ? 0 : Inherit(t.cfg.travelTimeout, s.itemTravelTimeout);
    int closeDelay = t.isChevron ? s.grace : Inherit(t.cfg.closeDelay, s.itemCloseDelay);

    // Overshooting past the popup's edge is forgiven more generously once the
    // cursor has actually been inside it, which is the "I went one pixel too
    // far" case rather than "I am leaving".
    int effPopupPad = t.everInsidePopup ? popupPad * 2 : popupPad;

    // Two popups can occupy the same screen area: measured on this machine,
    // Quick Settings covers (3360,0 480x2095) and EarTrumpet's flyout
    // (3375,924 450x1157), i.e. almost the same rectangle. A rectangle test
    // alone then reports the cursor as "inside" a popup it is not actually on,
    // and that popup is held open for as long as the user works in the other
    // one. Inside the rectangle, the window under the cursor decides; the pad
    // band outside it stays a pure geometry test, so overshoot is still
    // forgiven even when the cursor lands on the desktop.
    //
    // Only a window from a *different process* counts as occluding. A XAML
    // flyout is not one window: it hosts its content in sibling top-level popup
    // windows of the same process, and treating those as "something else on top"
    // would close the flyout the moment the cursor moved onto its own content.
    bool insideExact = PtInRectPad(t.popupRect, ctx.pt, 0);
    bool occluded = insideExact && ctx.underRoot && ctx.underRoot != t.popup &&
                    t.popupPid && ctx.underRootPid && ctx.underRootPid != t.popupPid;
    bool overPopup = !occluded && PtInRectPad(t.popupRect, ctx.pt, effPopupPad);
    if (insideExact && !occluded) t.everInsidePopup = true;

    // A context menu opened from the popup extends past it, and a held mouse
    // button means a slider is being dragged. Both mean the user is still busy.
    bool busy = t.userEngaged || ctx.anyButtonDown || ctx.menu != nullptr;

    if (overIcon || overPopup || busy) {
        t.lastEngagedAt = ctx.now;
        t.leftIconAt = 0;
        t.bestDist = INT_MAX;
        return false;
    }

    // Travel grace is for crossing the gap. A cursor standing on a window that
    // covers this popup is at distance zero from it, which would read as
    // "arrived" and hold it open for the whole travel timeout; it has not
    // arrived, it is somewhere else.
    if (travelTimeout > 0 && !occluded) {
        if (t.leftIconAt == 0) {
            t.leftIconAt = ctx.now;
            t.bestDist = DistToRect(t.popupRect, ctx.pt);
        }
        int d = DistToRect(t.popupRect, ctx.pt);
        RECT corridor = UnionRect(t.rect, t.popupRect);
        bool inCorridor = PtInRectPad(corridor, ctx.pt, s.itemTravelPad);
        bool approaching = d <= t.bestDist + TRAVEL_SLACK;
        if (d < t.bestDist) t.bestDist = d;
        if (inCorridor && approaching &&
            ctx.now - t.leftIconAt < (ULONGLONG)travelTimeout) {
            t.lastEngagedAt = ctx.now;
            return false;
        }
    }

    if (t.lastEngagedAt == 0) t.lastEngagedAt = ctx.now;
    return ctx.now - t.lastEngagedAt >= (ULONGLONG)closeDelay;
}

// Look for the popup an invoke just produced. Returns nullptr until one is
// found or the discovery window expires (handled by the caller).
static HWND DiscoverPopup(Target& t, const TickContext& ctx) {
    const Settings& s = *ctx.s;
    const std::vector<WindowInfo>& before = t.windowsBefore;

    auto wasThereBefore = [&before](HWND h) {
        for (const auto& b : before) {
            if (b.hwnd == h) return true;
        }
        return false;
    };

    // 1) A classic tray icon shows a TrackPopupMenu menu, which never takes the
    //    foreground and would fail every other test. It has to be a menu that
    //    was not already on screen, or a context menu the user opened elsewhere
    //    just before hovering would be adopted as this item's popup.
    if (ctx.menu && t.cfg.popupMatch == PopupMatch::Auto && !wasThereBefore(ctx.menu)) {
        return ctx.menu;
    }

    // 2) The foreground window changed. This is the strongest signal for a real
    //    flyout: EarTrumpet's window, Quick Settings and the calendar all take
    //    focus, and none of them has a predictable class name (EarTrumpet's
    //    contains a GUID regenerated every run).
    HWND fg = GetForegroundWindow();
    if (fg && fg != t.fgBefore) {
        WindowInfo w;
        w.hwnd = fg;
        w.pid = 0;
        GetWindowThreadProcessId(fg, &w.pid);
        w.cls = ClassOf(fg);
        if (GetWindowRect(fg, &w.rect) && RectValid(w.rect) &&
            !IsNeverAPopup(w, s, ctx.selfPid) && PopupRulesAllow(t.cfg, w)) {
            return fg;
        }
    }

    // 3) Any newly visible top-level window. Some popups never take focus; among
    //    several new windows the one nearest the icon wins.
    std::vector<WindowInfo> after = VisibleTopLevelWindows();
    HWND best = nullptr;
    int bestDist = INT_MAX;
    for (const auto& w : after) {
        if (wasThereBefore(w.hwnd)) continue;
        if (IsNeverAPopup(w, s, ctx.selfPid)) continue;
        if (!PopupRulesAllow(t.cfg, w)) continue;
        POINT c{(t.rect.left + t.rect.right) / 2, (t.rect.top + t.rect.bottom) / 2};
        int d = DistToRect(w.rect, c);
        if (d < bestDist) {
            bestDist = d;
            best = w.hwnd;
        }
    }
    return best;
}

static DWORD WINAPI WorkerThread(LPVOID) {
    // UIA bounding rectangles are physical screen coordinates. Without per
    // monitor v2 awareness, GetCursorPos and GetWindowRect would be virtualized
    // on mixed-DPI setups and the hover test would compare different spaces.
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    IUIAutomation* pAuto = nullptr;
    if (FAILED(CoCreateInstance(__uuidof(CUIAutomation), nullptr, CLSCTX_INPROC_SERVER,
                                __uuidof(IUIAutomation), (void**)&pAuto)) ||
        !pAuto) {
        Wh_Log(L"Failed to create IUIAutomation");
        CoUninitialize();
        return 0;
    }

    Settings s = GetSettingsSnapshot();
    int settingsGen = g_settingsGeneration;
    const DWORD selfPid = GetCurrentProcessId();

    std::vector<Target> targets;
    auto rebuildTargets = [&targets](const Settings& cfg) {
        for (auto& t : targets) t.DropElement();
        targets.clear();
        Target chevron;
        chevron.isChevron = true;
        targets.push_back(std::move(chevron));
        for (const auto& item : cfg.items) {
            Target t;
            t.cfg = item;
            targets.push_back(std::move(t));
        }
    };
    rebuildTargets(s);

    DWORD taskbarPid = 0;
    ULONGLONG nextRefind = 0;
    ULONGLONG lastWalkAt = 0;
    ULONGLONG nextIdleStateCheck = 0;
    ULONGLONG nextDiagnostic = 0;
    size_t diagSignature = 0;
    int refindFailures = 0;
    bool nearTaskbarPrev = false;
    bool anyBtnDownPrev = false;
    bool loggedCandidates = false;
    // Survives a successful find, unlike loggedCandidates, so that a build which
    // always matches by name or by position logs that once rather than on every
    // acquisition.
    bool loggedWeakMatch = false;

    while (g_running) {
        ULONGLONG now = GetTickCount64();

        // Read the generation before taking the snapshot. LoadSettings publishes
        // the settings and only then bumps the counter, so reading it afterwards
        // could record an update as already applied and drop it.
        int gen = g_settingsGeneration;
        if (gen != settingsGen) {
            s = GetSettingsSnapshot();
            settingsGen = gen;
            // Settings can change how anything is detected, so drop every cached
            // element and re-detect. Every target starts disarmed-but-idle, so a
            // reload cannot make popups fire on its own.
            rebuildTargets(s);
            nextRefind = 0;
            nextDiagnostic = 0;
            // Cleared so that toggling diagnostics off and on always produces a
            // fresh table rather than being suppressed as "unchanged".
            diagSignature = 0;
            loggedCandidates = false;
            loggedWeakMatch = false;
            refindFailures = 0;
        }

        if (!s.enabled) {
            for (auto& t : targets) {
                t.DropElement();
                t.ResetInteraction();
            }
            WaitForSingleObject(g_stopEvent, s.pollInterval);
            continue;
        }

        POINT pt;
        GetCursorPos(&pt);

        HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", nullptr);
        // Only read further down when nearTaskbar is true, which already implies
        // GetWindowRect succeeded. Initialised anyway so that stays a fact about
        // the code rather than a fact someone has to re-derive.
        RECT tb{};
        // At least as wide as the hit area, so there is no position the mod
        // treats as "on an element" but not as "at the taskbar".
        int revealPad = std::max(s.maxPad, TASKBAR_REVEAL_PAD);
        bool nearTaskbar =
            hTaskbar && GetWindowRect(hTaskbar, &tb) && PtInRectPad(tb, pt, revealPad);
        if (nearTaskbar && !nearTaskbarPrev) {
            refindFailures = 0;
            nextRefind = 0;
        }
        nearTaskbarPrev = nearTaskbar;

        bool anyOpen = false, anyDiscovering = false;
        for (const auto& t : targets) {
            if (t.state == St::Open) anyOpen = true;
            if (t.state == St::Discover) anyDiscovering = true;
        }

        // The "pressed since the previous call" bit is desktop-wide state that
        // the first caller consumes, so polling it continuously would take it
        // away from every other application for the whole session. Sample it only
        // while something is actually open and being watched.
        TickContext ctx;
        ctx.now = now;
        ctx.pt = pt;
        ctx.s = &s;
        ctx.selfPid = selfPid;
        ctx.anyButtonDown = false;
        ctx.pressedSinceTick = false;
        ctx.menu = nullptr;
        ctx.underRoot = nullptr;
        ctx.underRootPid = 0;
        if (anyOpen || anyDiscovering) {
            SHORT kl = GetAsyncKeyState(VK_LBUTTON);
            SHORT kr = GetAsyncKeyState(VK_RBUTTON);
            SHORT km = GetAsyncKeyState(VK_MBUTTON);
            ctx.anyButtonDown = ((kl | kr | km) & 0x8000) != 0;
            ctx.pressedSinceTick = ((kl | kr | km) & 0x0001) != 0;
            ctx.menu = GetVisiblePopupMenu();
            if (HWND under = WindowFromPoint(pt)) {
                ctx.underRoot = GetAncestor(under, GA_ROOT);
                if (ctx.underRoot) {
                    GetWindowThreadProcessId(ctx.underRoot, &ctx.underRootPid);
                }
            }
        }
        // Recorded here rather than at the end of the loop, which a `continue`
        // can skip: that would keep a two-ticks-old value and let the next tick
        // see a press edge that never happened.
        bool anyBtnDownEdge = ctx.anyButtonDown && !anyBtnDownPrev;
        anyBtnDownPrev = ctx.anyButtonDown;

        // Resolving the hidden-icons flyout is a window enumeration, so it is not
        // done on every tick: only while the chevron is actually in play, or
        // occasionally while idle so that a flyout opened by clicking is still
        // noticed and auto-collapsed. The flyout also doubles as a second search
        // root, so that icons inside it can be configured as items too.
        const bool chevronBusy = !targets.empty() && targets[0].state != St::Idle;
        bool wantFlyout = s.chevronEnabled && (chevronBusy || nearTaskbar ||
                                               (s.autoClose && now >= nextIdleStateCheck));
        if (wantFlyout && !chevronBusy && !nearTaskbar) {
            nextIdleStateCheck = now + IDLE_STATE_CHECK_MS;
        }
        HWND flyout = wantFlyout ? GetVisibleFlyout(s, taskbarPid) : nullptr;

        // Lazily (re-)find elements only when at least one target is missing one.
        // A destroyed or stale element makes the rectangle query below fail,
        // which drops it and triggers a re-find, so no periodic cross-process
        // subtree walk is needed while elements are valid.
        //
        // Consecutive failures back off. Without that, a user with no hidden
        // icons has no chevron to find, so every lookup fails and the fast
        // cadence would walk the taskbar subtree several times a second for the
        // whole session, forcing explorer to build automation peers on its UI
        // thread exactly while the user is working with the taskbar.
        bool anyMissing = false;
        for (const auto& t : targets) {
            if (t.el) continue;
            if (t.isChevron ? s.chevronEnabled : t.cfg.enabled) anyMissing = true;
        }
        bool wantDiagnostic = s.diagnostics && now >= nextDiagnostic;
        // A diagnostic dump is what the user asked for by turning it on, so it
        // is not gated on the cursor being at the taskbar the way the ordinary
        // re-find is; it is only rate limited.
        bool doWalk = wantDiagnostic || (anyMissing && nearTaskbar && now >= nextRefind);
        if (doWalk && now - lastWalkAt >= REFIND_NEAR_MS) {
            lastWalkAt = now;
            taskbarPid = 0;
            GetWindowThreadProcessId(hTaskbar, &taskbarPid);
            bool didLog = false;
            bool didLogWeak = false;
            // Without diagnostics on, candidates are dumped once per failure
            // streak, which is what makes an unsupported build actionable
            // without burying the rest of the log.
            bool logCandidates = wantDiagnostic || (anyMissing && !loggedCandidates);
            bool resolved =
                ResolveTargets(pAuto, s, targets, hTaskbar, flyout, logCandidates, &didLog,
                               loggedWeakMatch ? nullptr : &didLogWeak,
                               wantDiagnostic ? &diagSignature : nullptr);
            loggedWeakMatch = loggedWeakMatch || didLogWeak;
            if (wantDiagnostic) nextDiagnostic = now + DIAGNOSTIC_INTERVAL_MS;
            if (resolved) {
                refindFailures = 0;
                loggedCandidates = false;
            } else if (anyMissing) {
                loggedCandidates = loggedCandidates || didLog;
                ULONGLONG delay = REFIND_NEAR_MS << (refindFailures < 4 ? refindFailures : 4);
                if (delay > REFIND_MAX_MS) delay = REFIND_MAX_MS;
                if (refindFailures < 100) refindFailures++;
                nextRefind = now + delay;
            }
        }

        for (auto& t : targets) {
            bool live = t.isChevron ? s.chevronEnabled : t.cfg.enabled;
            if (!live) {
                if (t.el) t.DropElement();
                if (t.state != St::Idle) t.ResetInteraction();
                continue;
            }

            // Expensive and RARE: query the element rectangle through UIA only
            // periodically, not every tick, and only while the cursor is at the
            // taskbar, where a refreshed rectangle can still change a decision.
            if (t.el && nearTaskbar && now >= t.nextRectRefresh) {
                // An element that is alive but not currently rendered (the
                // taskbar auto-hid, or the last hidden icon was removed) returns
                // S_OK with an empty rectangle rather than an error. Keeping it
                // would turn the top-left corner of the screen into a hover
                // hotspot, and the alternating valid/empty rectangle under a
                // stationary cursor is what makes a flyout cycle open and closed.
                // Treat it exactly like a stale element.
                RECT r{};
                BOOL offscreen = FALSE;
                if (SUCCEEDED(t.el->get_CurrentBoundingRectangle(&r)) && RectValid(r) &&
                    SUCCEEDED(t.el->get_CurrentIsOffscreen(&offscreen)) && !offscreen) {
                    // Re-apply the clamp learned when the neighbours were known;
                    // a single element cannot be clipped against them here.
                    if (t.maxWidth > 0 && r.right - r.left > t.maxWidth) {
                        r.right = r.left + t.maxWidth;
                    }
                    t.rect = r;
                    t.haveRect = true;
                    t.nextRectRefresh = now + RECT_REFRESH_MS;
                } else {
                    // The element is gone: the taskbar auto-hid, Explorer
                    // restarted, or (for an icon inside the hidden-icons flyout)
                    // the flyout that hosted it closed. An open popup is still
                    // tracked so that it is not re-opened and so that its close
                    // decision keeps running; the close itself is then skipped,
                    // because there is no longer an element to invoke.
                    t.DropElement();
                    if (t.state != St::Open) t.ResetInteraction();
                    nextRefind = 0;
                }
            }

            if (!t.haveRect && t.state != St::Open) continue;

            int pad = t.isChevron ? s.pad : Inherit(t.cfg.pad, s.itemPad);
            bool overIcon = t.haveRect && PtInRectPad(t.rect, pt, pad);
            // The dwell timer starts when the cursor arrives, not when the
            // target becomes eligible to fire, so a cooldown that expires under
            // a resting cursor does not restart the wait.
            if (!overIcon) {
                t.armed = true;
                t.dwellSince = 0;
            } else if (t.dwellSince == 0) {
                t.dwellSince = now;
            }

            // The chevron's popup is a known window class owned by the taskbar,
            // so it needs no discovery and, exactly as before, a flyout the user
            // opened by clicking is tracked and auto-collapsed too.
            if (t.isChevron) {
                HWND fly = flyout;

                if (s.hideTooltip && overIcon) {
                    HideChevronTooltip(s, t.rect, fly, taskbarPid);
                }

                bool cooling = now < t.cooldownUntil;
                // Coming back to the chevron is a fresh interaction, so a
                // pending "do not close again" is lifted.
                if (overIcon) t.suppressUntilHidden = false;
                if (fly) {
                    if (t.state != St::Open) {
                        t.state = St::Open;
                        t.popup = fly;
                        t.lastEngagedAt = now;
                        t.leftIconAt = 0;
                        t.bestDist = INT_MAX;
                    }
                    t.popup = fly;
                    t.popupPid = taskbarPid;
                    GetWindowRect(fly, &t.popupRect);
                    // A stay that has already seen an open flyout counts as
                    // served, so re-opening requires leaving and re-entering.
                    if (overIcon) t.armed = false;
                } else {
                    // Flyout observed gone: the close landed, so a further close
                    // is allowed again next time it opens.
                    t.suppressUntilHidden = false;
                    if (t.state == St::Open && !cooling) {
                        t.ResetInteraction();
                        if (overIcon) t.armed = false;
                    }
                }

                if (t.state == St::Open) {
                    if ((ctx.pressedSinceTick || anyBtnDownEdge) &&
                        PtOverWindow(t.popup, pt, 0) && !t.userEngaged) {
                        t.userEngaged = true;
                        Wh_Log(L"'%s' click inside the popup: auto-close suspended", t.Label());
                    }
                    if (s.autoClose && !cooling && !t.userEngaged && t.el &&
                        !t.suppressUntilHidden) {
                        if (ShouldClose(t, ctx, overIcon)) {
                            DoInvoke(t.el, true);
                            t.cooldownUntil = GetTickCount64() + ACTION_COOLDOWN_MS;
                            bool wasOverIcon = overIcon;
                            t.ResetInteraction();
                            t.armed = !wasOverIcon;
                            t.suppressUntilHidden = true;
                            Wh_Log(L"'%s' CLOSE, cursor left", t.Label());
                        }
                    }
                } else if (overIcon && t.armed && !cooling && t.el &&
                           now - t.openedAt >= CHEVRON_REOPEN_GUARD_MS) {
                    if (now - t.dwellSince >= (ULONGLONG)s.hoverDelay) {
                        t.armed = false;
                        if (s.suppressInFullscreen && IsFullscreenOver(t.rect)) {
                            // Nothing: the flyout would cover a game or a video.
                        } else {
                            DoInvoke(t.el, false);
                            ULONGLONG invoked = GetTickCount64();
                            t.openedAt = invoked;
                            t.cooldownUntil = invoked + ACTION_COOLDOWN_MS;
                            Wh_Log(L"'%s' OPEN", t.Label());
                        }
                    }
                }
                continue;
            }

            // --- configurable items ---
            switch (t.state) {
                case St::Idle: {
                    if (!overIcon || !t.armed || !t.el || now < t.cooldownUntil) break;
                    int hoverDelay = Inherit(t.cfg.hoverDelay, s.itemHoverDelay);
                    if (now - t.dwellSince < (ULONGLONG)hoverDelay) break;

                    t.armed = false;
                    bool suppress = Inherit(t.cfg.suppressInFullscreen, s.suppressInFullscreen);
                    if (suppress && IsFullscreenOver(t.rect)) break;

                    // Never toggle shut a popup the user opened by clicking. Once
                    // the mod has opened this item once it knows what its popup
                    // looks like, which is enough to recognise one that is
                    // already on screen and simply stay out of the way.
                    if (!t.learnedClass.empty()) {
                        bool alreadyOpen = false;
                        for (const auto& w : VisibleTopLevelWindows()) {
                            if (w.pid == t.learnedPid && w.cls == t.learnedClass) {
                                alreadyOpen = true;
                                break;
                            }
                        }
                        if (alreadyOpen) {
                            Wh_Log(L"'%s' popup already open, not invoking", t.Label());
                            break;
                        }
                    }

                    t.windowsBefore = VisibleTopLevelWindows();
                    t.fgBefore = GetForegroundWindow();
                    DoInvoke(t.el, false);
                    // Re-read the clock: DoInvoke is a cross-process call into
                    // Explorer's UI thread and has been measured taking over a
                    // second when the shell is busy. Stamping these with the
                    // tick's `now`, captured before the call, spends the whole
                    // discovery window before the popup has had a chance to
                    // appear -- observed live as "no popup appeared within
                    // 1200ms" logged 61ms after the open.
                    ULONGLONG invoked = GetTickCount64();
                    t.openedAt = invoked;
                    t.cooldownUntil = invoked + ACTION_COOLDOWN_MS;
                    t.state = St::Discover;
                    Wh_Log(L"'%s' OPEN", t.Label());
                    break;
                }

                case St::Discover: {
                    HWND p = DiscoverPopup(t, ctx);
                    int discoverMs = Inherit(t.cfg.discoverMs, s.itemDiscoverMs);
                    if (p && GetWindowRect(p, &t.popupRect) && RectValid(t.popupRect)) {
                        t.popup = p;
                        t.state = St::Open;
                        t.windowsBefore.clear();
                        t.windowsBefore.shrink_to_fit();
                        t.lastEngagedAt = now;
                        t.leftIconAt = 0;
                        t.bestDist = INT_MAX;
                        t.everInsidePopup = false;
                        t.userEngaged = false;
                        t.learnedClass = ClassOf(p);
                        t.learnedPid = 0;
                        GetWindowThreadProcessId(p, &t.learnedPid);
                        t.popupPid = t.learnedPid;
                        Wh_Log(L"'%s' popup found: class=%s rect=(%d,%d %dx%d)", t.Label(),
                               t.learnedClass.c_str(), (int)t.popupRect.left,
                               (int)t.popupRect.top,
                               (int)(t.popupRect.right - t.popupRect.left),
                               (int)(t.popupRect.bottom - t.popupRect.top));
                    } else if (now - t.openedAt >= (ULONGLONG)discoverMs) {
                        t.ResetInteraction();
                        t.armed = !overIcon;
                        Wh_Log(L"'%s' no popup appeared within %dms", t.Label(), discoverMs);
                    }
                    break;
                }

                case St::Open: {
                    if (!IsWindow(t.popup) || !IsWindowVisible(t.popup) ||
                        !GetWindowRect(t.popup, &t.popupRect)) {
                        // Closed by the app, by a click elsewhere, or by the user
                        // clicking the icon again. Never re-open on our own; the
                        // cursor has to leave and come back first.
                        t.ResetInteraction();
                        t.armed = !overIcon;
                        t.cooldownUntil = now + ACTION_COOLDOWN_MS;
                        Wh_Log(L"'%s' popup gone", t.Label());
                        break;
                    }

                    if ((ctx.pressedSinceTick || anyBtnDownEdge) && !t.userEngaged &&
                        (PtOverWindow(t.popup, pt, 0) || overIcon)) {
                        t.userEngaged = true;
                        Wh_Log(L"'%s' click inside the popup: auto-close suspended", t.Label());
                    }

                    bool autoClose = Inherit(t.cfg.autoClose, s.itemAutoClose);
                    if (!autoClose || t.cfg.closeAction == CloseAction::None) {
                        // Still track engagement so the state stays meaningful,
                        // but never act on it.
                        ShouldClose(t, ctx, overIcon);
                        break;
                    }
                    if (t.userEngaged || now < t.cooldownUntil) break;

                    if (ShouldClose(t, ctx, overIcon)) {
                        // No element means the tray icon went away underneath an
                        // open popup (Explorer restart, or the hidden-icons
                        // flyout that hosted it closing). Stop tracking rather
                        // than invoking something that no longer exists; the app
                        // dismisses its own popup on the next click elsewhere.
                        if (t.el) DoInvoke(t.el, true);
                        t.cooldownUntil = GetTickCount64() + ACTION_COOLDOWN_MS;
                        t.ResetInteraction();
                        t.armed = !overIcon;
                        Wh_Log(L"'%s' CLOSE, cursor left", t.Label());
                    }
                    break;
                }
            }
        }

        // Interruptible sleep: WhTool_ModUninit signals g_stopEvent so the thread
        // wakes immediately regardless of the polling interval.
        WaitForSingleObject(g_stopEvent, s.pollInterval);
    }

    for (auto& t : targets) t.DropElement();
    if (pAuto) pAuto->Release();
    CoUninitialize();
    return 0;
}

// ---------------------------------------------------------------------------
// Mod lifecycle (tool mod)
// ---------------------------------------------------------------------------

static int ClampInt(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

// -1 means "inherit", so it has to survive clamping; anything else is clamped
// into a sane range so that a typo cannot turn into pathological behaviour.
static int ClampOverride(int v, int lo, int hi) {
    return v < 0 ? -1 : ClampInt(v, lo, hi);
}

static std::wstring GetStr(PCWSTR fmt, int index) {
    PCWSTR v = Wh_GetStringSetting(fmt, index);
    std::wstring s = v ? v : L"";
    Wh_FreeStringSetting(v);
    return s;
}

static Tri ParseTri(const std::wstring& v) {
    if (v == L"yes") return Tri::On;
    if (v == L"no") return Tri::Off;
    return Tri::Inherit;
}

static void LoadSettings() {
    Settings s;

    s.enabled = Wh_GetIntSetting(L"enabled") != 0;
    s.pollInterval = ClampInt((int)Wh_GetIntSetting(L"pollInterval"), 10, 1000);
    s.diagnostics = Wh_GetIntSetting(L"diagnostics") != 0;

    s.chevronEnabled = Wh_GetIntSetting(L"chevronEnabled") != 0;
    s.autoClose = Wh_GetIntSetting(L"autoClose") != 0;
    s.hoverDelay = ClampInt((int)Wh_GetIntSetting(L"hoverDelay"), 0, 10000);
    s.grace = ClampInt((int)Wh_GetIntSetting(L"grace"), 0, 10000);
    // A large pad turns much of the screen into a hover hotspot; a negative one
    // shrinks the hit area below the button itself.
    s.pad = ClampInt((int)Wh_GetIntSetting(L"pad"), 0, 64);
    s.hideTooltip = Wh_GetIntSetting(L"hideTooltip") != 0;
    s.positionalFallback = Wh_GetIntSetting(L"positionalFallback") != 0;
    s.suppressInFullscreen = Wh_GetIntSetting(L"suppressInFullscreen") != 0;

    s.itemHoverDelay = ClampInt((int)Wh_GetIntSetting(L"itemHoverDelay"), 0, 10000);
    s.itemCloseDelay = ClampInt((int)Wh_GetIntSetting(L"itemCloseDelay"), 0, 10000);
    s.itemTravelTimeout = ClampInt((int)Wh_GetIntSetting(L"itemTravelTimeout"), 0, 10000);
    s.itemTravelPad = ClampInt((int)Wh_GetIntSetting(L"itemTravelPad"), 0, 400);
    s.itemPad = ClampInt((int)Wh_GetIntSetting(L"itemPad"), 0, 64);
    s.itemPopupPad = ClampInt((int)Wh_GetIntSetting(L"itemPopupPad"), 0, 200);
    s.itemDiscoverMs = ClampInt((int)Wh_GetIntSetting(L"itemDiscoverMs"), 100, 10000);
    s.itemAutoClose = Wh_GetIntSetting(L"itemAutoClose") != 0;

    // Wh_GetStringSetting never returns NULL (it returns L"" on unset/error), so
    // only override the defaults with non-empty values.
    std::wstring v;
    if (!(v = GetStr(L"flyoutClass", 0)).empty()) s.flyoutClass = v;
    if (!(v = GetStr(L"tooltipClass", 0)).empty()) s.tooltipClass = v;
    if (!(v = GetStr(L"trayIconAutomationId", 0)).empty()) s.trayIconAutomationId = v;
    if (!(v = GetStr(L"chevronClass", 0)).empty()) s.chevronClass = v;

    std::vector<std::wstring> keywords;
    for (int i = 0; i < 256; i++) {
        std::wstring k = GetStr(L"keywords[%d]", i);
        if (k.empty()) break;
        keywords.push_back(std::move(k));
    }
    if (!keywords.empty()) s.keywords = std::move(keywords);
    s.keywordsLower.reserve(s.keywords.size());
    for (const auto& k : s.keywords) s.keywordsLower.push_back(ToLower(k));

    for (int i = 0; i < 64; i++) {
        ItemConfig c;
        c.label = GetStr(L"targets[%d].name", i);
        c.matchName = ToLower(GetStr(L"targets[%d].matchName", i));
        c.matchAutomationId = GetStr(L"targets[%d].matchAutomationId", i);
        c.matchClass = GetStr(L"targets[%d].matchClass", i);
        // An entirely empty record is the end of the list: Windhawk returns
        // empty strings and zeros past the last one.
        if (c.label.empty() && c.matchName.empty() && c.matchAutomationId.empty() &&
            c.matchClass.empty()) {
            break;
        }
        c.enabled = Wh_GetIntSetting(L"targets[%d].enabled", i) != 0;
        c.matchNameExact = Wh_GetIntSetting(L"targets[%d].matchNameExact", i) != 0;
        c.allowNonTray = Wh_GetIntSetting(L"targets[%d].allowNonTray", i) != 0;
        c.hoverDelay = ClampOverride((int)Wh_GetIntSetting(L"targets[%d].hoverDelay", i), 0, 10000);
        c.closeDelay = ClampOverride((int)Wh_GetIntSetting(L"targets[%d].closeDelay", i), 0, 10000);
        c.travelTimeout =
            ClampOverride((int)Wh_GetIntSetting(L"targets[%d].travelTimeout", i), 0, 10000);
        c.pad = ClampOverride((int)Wh_GetIntSetting(L"targets[%d].pad", i), 0, 64);
        c.popupPad = ClampOverride((int)Wh_GetIntSetting(L"targets[%d].popupPad", i), 0, 200);
        c.discoverMs =
            ClampOverride((int)Wh_GetIntSetting(L"targets[%d].discoverMs", i), 100, 10000);
        c.autoClose = ParseTri(GetStr(L"targets[%d].autoClose", i));
        c.suppressInFullscreen = ParseTri(GetStr(L"targets[%d].suppressInFullscreen", i));
        c.closeAction = GetStr(L"targets[%d].closeAction", i) == L"none" ? CloseAction::None
                                                                        : CloseAction::Toggle;
        std::wstring pm = GetStr(L"targets[%d].popupMatch", i);
        c.popupMatch = pm == L"process" ? PopupMatch::Process
                       : pm == L"class" ? PopupMatch::Class
                                        : PopupMatch::Auto;
        c.popupProcess = ToLower(GetStr(L"targets[%d].popupProcess", i));
        c.popupClass = ToLower(GetStr(L"targets[%d].popupClass", i));

        // A rule that cannot identify anything is worse than no item at all,
        // because it would sit there looking configured.
        if (c.enabled && c.matchName.empty() && c.matchAutomationId.empty() &&
            c.matchClass.empty()) {
            Wh_Log(L"Item '%s' has no match rules and is ignored", c.label.c_str());
            c.enabled = false;
        }
        s.items.push_back(std::move(c));
    }

    s.maxPad = std::max(s.pad, s.itemPad);
    for (const auto& c : s.items) {
        s.maxPad = std::max(s.maxPad, c.pad);
    }

    AcquireSRWLockExclusive(&g_settingsLock);
    g_settings = std::move(s);
    ReleaseSRWLockExclusive(&g_settingsLock);
    g_settingsGeneration++;
}

BOOL WhTool_ModInit() {
    LoadSettings();

    g_stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!g_stopEvent) {
        Wh_Log(L"CreateEvent failed");
        return FALSE;
    }

    g_running = true;
    g_thread = CreateThread(nullptr, 0, WorkerThread, nullptr, 0, nullptr);
    if (!g_thread) {
        g_running = false;
        CloseHandle(g_stopEvent);
        g_stopEvent = nullptr;
        return FALSE;
    }
    return TRUE;
}

void WhTool_ModSettingsChanged() {
    LoadSettings();
}

void WhTool_ModUninit() {
    g_running = false;
    if (g_stopEvent) {
        SetEvent(g_stopEvent);
    }
    if (g_thread) {
        // Safe to wait without a timeout: the stop event interrupts the poll
        // sleep, and the cross-process UIA calls the worker can be inside have
        // their own timeouts, so the wait is bounded even if the shell is busy.
        WaitForSingleObject(g_thread, INFINITE);
        CloseHandle(g_thread);
        g_thread = nullptr;
    }
    if (g_stopEvent) {
        CloseHandle(g_stopEvent);
        g_stopEvent = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Windhawk tool mod implementation for mods which don't need to inject to other
// processes or hook other functions. Context:
// https://github.com/ramensoftware/windhawk/wiki/Mods-as-tools:-Running-mods-in-a-dedicated-process
//
// The mod will load and run in a dedicated windhawk.exe process.
//
// Paste the code below as part of the mod code, and use these callbacks:
// * WhTool_ModInit
// * WhTool_ModSettingsChanged
// * WhTool_ModUninit
//
// Currently, other callbacks are not supported.

bool g_isToolModProcessLauncher;
HANDLE g_toolModProcessMutex;

void WINAPI EntryPoint_Hook() {
    Wh_Log(L">");
    ExitThread(0);
}

BOOL Wh_ModInit() {
    DWORD sessionId;
    if (ProcessIdToSessionId(GetCurrentProcessId(), &sessionId) && sessionId == 0) {
        return FALSE;
    }

    bool isExcluded = false;
    bool isToolModProcess = false;
    bool isCurrentToolModProcess = false;
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);
    if (!argv) {
        Wh_Log(L"CommandLineToArgvW failed");
        return FALSE;
    }

    for (int i = 1; i < argc; i++) {
        if (wcscmp(argv[i], L"-service") == 0 || wcscmp(argv[i], L"-service-start") == 0 ||
            wcscmp(argv[i], L"-service-stop") == 0) {
            isExcluded = true;
            break;
        }
    }

    for (int i = 1; i < argc - 1; i++) {
        if (wcscmp(argv[i], L"-tool-mod") == 0) {
            isToolModProcess = true;
            if (wcscmp(argv[i + 1], WH_MOD_ID) == 0) {
                isCurrentToolModProcess = true;
            }
            break;
        }
    }

    LocalFree(argv);

    if (isExcluded) {
        return FALSE;
    }

    if (isCurrentToolModProcess) {
        g_toolModProcessMutex = CreateMutex(nullptr, TRUE, L"windhawk-tool-mod_" WH_MOD_ID);
        if (!g_toolModProcessMutex) {
            Wh_Log(L"CreateMutex failed");
            ExitProcess(1);
        }

        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            Wh_Log(L"Tool mod already running (%s)", WH_MOD_ID);
            ExitProcess(1);
        }

        if (!WhTool_ModInit()) {
            ExitProcess(1);
        }

        IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)GetModuleHandle(nullptr);
        IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)((BYTE*)dosHeader + dosHeader->e_lfanew);

        DWORD entryPointRVA = ntHeaders->OptionalHeader.AddressOfEntryPoint;
        void* entryPoint = (BYTE*)dosHeader + entryPointRVA;

        Wh_SetFunctionHook(entryPoint, (void*)EntryPoint_Hook, nullptr);
        return TRUE;
    }

    if (isToolModProcess) {
        return FALSE;
    }

    g_isToolModProcessLauncher = true;
    return TRUE;
}

void Wh_ModAfterInit() {
    if (!g_isToolModProcessLauncher) {
        return;
    }

    WCHAR currentProcessPath[MAX_PATH];
    switch (GetModuleFileName(nullptr, currentProcessPath, ARRAYSIZE(currentProcessPath))) {
        case 0:
        case ARRAYSIZE(currentProcessPath):
            Wh_Log(L"GetModuleFileName failed");
            return;
    }

    WCHAR commandLine[MAX_PATH + 2 + (sizeof(L" -tool-mod \"" WH_MOD_ID "\"") / sizeof(WCHAR)) - 1];
    swprintf_s(commandLine, L"\"%s\" -tool-mod \"%s\"", currentProcessPath, WH_MOD_ID);

    HMODULE kernelModule = GetModuleHandle(L"kernelbase.dll");
    if (!kernelModule) {
        kernelModule = GetModuleHandle(L"kernel32.dll");
        if (!kernelModule) {
            Wh_Log(L"No kernelbase.dll/kernel32.dll");
            return;
        }
    }

    using CreateProcessInternalW_t =
        BOOL(WINAPI*)(HANDLE hUserToken, LPCWSTR lpApplicationName, LPWSTR lpCommandLine,
                      LPSECURITY_ATTRIBUTES lpProcessAttributes,
                      LPSECURITY_ATTRIBUTES lpThreadAttributes, WINBOOL bInheritHandles,
                      DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory,
                      LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation,
                      PHANDLE hRestrictedUserToken);
    CreateProcessInternalW_t pCreateProcessInternalW =
        (CreateProcessInternalW_t)GetProcAddress(kernelModule, "CreateProcessInternalW");
    if (!pCreateProcessInternalW) {
        Wh_Log(L"No CreateProcessInternalW");
        return;
    }

    STARTUPINFO si{
        .cb = sizeof(STARTUPINFO),
        .dwFlags = STARTF_FORCEOFFFEEDBACK,
    };
    PROCESS_INFORMATION pi;
    if (!pCreateProcessInternalW(nullptr, currentProcessPath, commandLine, nullptr, nullptr, FALSE,
                                 NORMAL_PRIORITY_CLASS, nullptr, nullptr, &si, &pi, nullptr)) {
        Wh_Log(L"CreateProcess failed");
        return;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void Wh_ModSettingsChanged() {
    if (g_isToolModProcessLauncher) {
        return;
    }

    WhTool_ModSettingsChanged();
}

void Wh_ModUninit() {
    if (g_isToolModProcessLauncher) {
        return;
    }

    WhTool_ModUninit();
    ExitProcess(0);
}
