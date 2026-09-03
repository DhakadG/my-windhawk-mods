// ==WindhawkMod==
// @id              taskbar-system-info-fork
// @name            Taskbar System Info - Fork
// @description     Fork of Taskbar System Info with network throughput, an internet-status dot, HWiNFO- or LibreHardwareMonitor-backed clock/power readings, and real StackPanel/Grid taskbar insertion instead of an overlay.
// @version         1.6.0
// @author          lost_husky
// @github          https://github.com/DhakadG
// @include         explorer.exe
// @architecture    x86-64
// @license         GPL-3.0
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -lpdh -ldxgi -liphlpapi -lws2_32 -lpowrprof -lwininet -DWIN32_LEAN_AND_MEAN
// ==/WindhawkMod==

// This fork carries forward the CPU/GPU/RAM/VRAM engine (PDH counters, D3DKMT adapter
// enumeration, HWiNFO shared-memory and Gadget Registry temperature readers) from
// Taskbar System Info by Yevhenii Starychenko (GPL-3.0):
// https://github.com/starychenko/windhawk-taskbar-system-info
// Taskbar and GPU discovery techniques in that engine are themselves credited there to
// m417z's "Multirow taskbar for Windows 11" and Taskbar Clock Customization.
//
// Replaced: the overlay + TaskbarFrameRepeater-margin insertion is replaced with the real
// StackPanel/Grid tray insertion (and the 26300+ StackPanel tray branch) already proven in
// this author's own taskbar-ai-quota-fork and taskbar-fluent-media-player-fork.
// Added: network throughput, an internet-status indicator, and HWiNFO-backed CPU/GPU clock
// and power readings, with a native NtPowerInformation fallback for CPU clock so that one
// doesn't depend on HWiNFO running. (GPU power has no native fallback: D3DKMT's Power
// field is undocumented and its unit unverified, so it stays HWiNFO-only rather than
// risk a wrong number.)

// ==WindhawkModReadme==
/*
# Taskbar System Info - Fork

> **This is a fork.** The metrics engine — PDH counters, D3DKMT GPU adapter
> enumeration, and the HWiNFO shared-memory/Gadget-Registry temperature readers — is
> from [Taskbar System Info](https://windhawk.net/mods/taskbar-system-info) by
> **[Yevhenii Starychenko](https://github.com/starychenko)**, GPL-3.0. This fork is
> maintained by [lost_husky](https://github.com/DhakadG) and changes two things:
>
> 1. **Placement.** The original overlays the widget on the taskbar's `RootGrid` and
>    fakes reserved space by shrinking the pinned-icons repeater's margin. This fork
>    inserts it as a real child of the tray `StackPanel`/`Grid` or tracks a taskbar
>    button's margin instead — the same insertion approach already proven in this
>    author's `taskbar-ai-quota-fork` and `taskbar-fluent-media-player-fork`, including
>    the Windows 11 26H2 (build 26300) StackPanel tray branch.
> 2. **More readings.** Network upload/download throughput, a subtle internet-status
>    dot, and CPU/GPU clock and power — from LibreHardwareMonitor or HWiNFO, with a
>    native `NtPowerInformation` fallback for CPU clock so that one reading needs
>    neither tool running.

A compact taskbar system monitor: CPU and GPU usage/temperature with 60-second history
graphs, RAM/VRAM capacity bars, network throughput, and an internet-status dot —
inserted into the taskbar like a native tray item, not overlaid on top of it. Two rows,
in three columns:

```text
● ↑ 033.96 KB/s │ CPU 030.0% 065°C 4.52GHz 032W ▁▂▃ │ RAM  051.0% 16.38/31.78G
  ↓ 353.56 KB/s │ GPU 002.9% 047°C 0585MHz 012W ▁▁▂ │ VRAM 038.4% 02.22/05.78G
```

Every value occupies a constant number of digits and every cell a constant width, so
nothing shifts as the numbers change.

## Placement

**Position** offers the same anchor set as this author's other taskbar forks: tray
positions (far left/right, either side of the clock, the Network/Volume button, the
language switcher, tray icons, the hidden-icons chevron, Show Desktop) reserve their own
slot so they never overlap Explorer's own icons; taskbar positions track the Start,
Search, Task View, or Widgets buttons; three edge positions overlay the taskbar for
anyone who prefers that instead.

## Appearance

A card-style background box appears **on hover** by default (or always, or never —
**Background box**), matching how this author's other taskbar forks behave. Thin
frosted dividers separate the three column groups, and each history graph sits on a
faint panel, with the region under the trace tinted so the line reads against the area
it covers instead of as a bare squiggle (*Graph area fill opacity*).

### Why the columns don't shift

Three things together, all configurable:

1. **A monospaced font** (default Consolas; Cascadia Mono/Code, Lucida Console and
   Courier New are offered, plus any family you like via *Custom font family*). This is
   the part that actually matters — in a proportional font like Segoe UI, `1` is
   narrower than `0`, so no amount of padding can hold a column still.
2. **Tabular figures** (*Force tabular figures*) — asks the font for fixed-advance
   digits via the OpenType `tnum` feature. Redundant for a monospaced font; it is what
   makes the proportional options usable if you prefer one.
3. **Padding to a constant digit count** (*Number padding*) — `030.0%` rather than
   `30.0%`, so the digit count never changes either. Leading zeros by default; blank
   figure-spaces and no padding are the alternatives.

### Hiding readings

Every reading has its own switch under **Visible Fields** — usage, temperature,
frequency, power and the history graph for CPU and GPU separately; percentage,
used/total and the capacity bar for RAM and VRAM separately. Each row also has a master
switch.

Hiding is not cosmetic. Because columns size to their content, a hidden field closes its
gap rather than leaving a hole, and a row with nothing left in it gives up its height
and its gap entirely — so does a whole section, along with the divider beside it. Turn
off the GPU row and VRAM together and the mod stops enumerating the adapter and drops
the `GPU Engine` and `GPU Adapter Memory` performance counters, which is what actually
makes it stop monitoring the GPU rather than just hiding the numbers. The same applies
to temperature and clock/power sensors: nothing visible needs them, nothing reads them.

### Spacing

Columns size themselves to their content rather than reserving room for the widest
string a cell could ever hold, so there is no dead gap between a label and its value.
Both rows of a section share one set of columns, so a column is as wide as the wider of
the two values and the rows stay aligned — a column only changes width when a value
genuinely needs more room, and with padding on (above) it never needs to. The gutter
between values is one number, *Gap between values*, applied uniformly.

Font size, row/column gap, gap between values, graph width, bar thickness, box padding,
corner radius, opacities, colours and per-graph visibility are all settings.

## Readings

- CPU utilization from Windows system-time counters; GPU utilization and VRAM from PDH
  GPU counters; RAM from Windows memory status. Capacities keep a full two decimal
  places (`16.68/32.00G`), not rounded to whole gigabytes.
- Network throughput from PDH network-interface counters, with an optional adapter-name
  filter, dynamic KB/s-MB/s scaling, and a configurable decimal count.
- An internet-status dot from an ICMP ping against two configurable hosts, subtle by
  design — a small colored dot (green/red/gray), not a banner.
- CPU/GPU temperature: HWiNFO or LibreHardwareMonitor, falling back to Windows D3DKMT
  (GPU) and ACPI thermal zones (CPU) — exactly as in the original.
- CPU/GPU clock and power (optional): HWiNFO or LibreHardwareMonitor first; CPU clock
  falls back to `NtPowerInformation` when neither is available. GPU clock and both
  power readings have no trustworthy native equivalent — D3DKMT exposes a `Power`
  field, but its actual unit isn't documented anywhere, so it's left unused rather than
  risk showing a plausible-looking wrong number — and show `--` without a monitor tool
  running.

## Getting temperature, clock and power data

Every one of these needs a third-party monitor running — the free tier (usage,
capacity, network) never does. Two are supported, and either is enough on its own.
**LibreHardwareMonitor is the recommended default** — it's free with no time limit,
unlike HWiNFO's free-tier restriction below.

**LibreHardwareMonitor** (recommended; the default **Temperature source** and
**Clock/power source** are Automatic, which tries this first):
1. Download [LibreHardwareMonitor](https://github.com/LibreHardwareMonitor/LibreHardwareMonitor)
   (portable, no install needed) and run it.
2. **Options → Remote Web Server → Run** — this is what actually exposes the
   `data.json` this mod reads; LHM running without it is invisible to the mod, the
   same way HWiNFO running without Shared Memory Support is (below).
3. Turn on **Enable LibreHardwareMonitor integration** under this mod's
   **LibreHardwareMonitor** settings group; the port (default `8085`) must match
   LHM's own web server port.

**HWiNFO** (an alternative or a supplement, not required if LHM is enabled):
1. Install and run [HWiNFO](https://www.hwinfo.com/) in Sensors-only or full mode.
2. Open **Settings** (the gear icon) → enable **Shared Memory Support**. This is
   **off by default** — HWiNFO running without it looks identical to HWiNFO not
   running at all from this mod's side, and is the single most common reason nothing
   shows up. Enable **Verbose logging** in this mod's Debug settings and check the
   Windhawk log (or a tool like DebugView) if readings still don't appear after this —
   it reports exactly which step failed.
3. The free edition disables shared memory after 12 hours of continuous use, needing a
   restart to resume — HWiNFO64 Pro has no such limit, and neither does
   LibreHardwareMonitor, which is why LHM is the better default for most people.
   Gadget Registry (Settings → Sensor Settings → HWiNFO Gadget → **Report to
   Gadget**) is a separate, always-on HWiNFO interface if you'd rather not restart
   HWiNFO daily but still don't want to run LHM.

Both can run at once. In **Automatic** mode (the default for both Temperature source
and Clock/power source) this mod tries LibreHardwareMonitor first, then HWiNFO, then
falls back to what Windows exposes natively — picking an explicit source instead of
Automatic makes that one exclusive (LHM only fills a genuine gap afterward, never
overriding an explicit non-Automatic choice). Any reading still unavailable after all
of that shows `--` — every other reading keeps working.

## Compatibility

Windows 11 64-bit, primary taskbar. Coexists with Taskbar Styler.

## Credits and license

Metrics engine and GPU/taskbar discovery techniques from
[Taskbar System Info](https://github.com/starychenko/windhawk-taskbar-system-info) by
Yevhenii Starychenko. Released under GPL-3.0, as required by that license.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- Placement:
  - position: tray_left
    $name: Position
    $description: >-
      Default: Tray - Far left. Tray positions reserve their own space; the three edge
      positions overlay the taskbar without reserving any.
    $options:
      - tray_left: 'Tray - Far left'
      - tray_right: 'Tray - Far right'
      - tray_before_clock: 'Tray - Left of Clock'
      - tray_after_clock: 'Tray - Right of Clock'
      - tray_before_omni_left: 'Tray - Left of Network/Volume button'
      - tray_before_omni_right: 'Tray - Right of Network/Volume button'
      - tray_language_left: 'Tray - Left of Language button'
      - tray_language_right: 'Tray - Right of Language button'
      - tray_icons_left: 'Tray - Left of Tray Icons'
      - tray_icons_right: 'Tray - Right of Tray Icons'
      - tray_hidden_icons_left: 'Tray - Left of Hidden icons button'
      - tray_hidden_icons_right: 'Tray - Right of Hidden icons button'
      - tray_after_showdesktop_left: 'Tray - Left of Show Desktop'
      - tray_after_showdesktop_right: 'Tray - Right of Show Desktop'
      - taskbar_left_start: 'Taskbar - Left of Start button'
      - taskbar_right_start: 'Taskbar - Right of Start button'
      - taskbar_after_search_left: 'Taskbar - Left of Search button'
      - taskbar_after_search_right: 'Taskbar - Right of Search button'
      - taskbar_after_taskview_left: 'Taskbar - Left of Task View button'
      - taskbar_after_taskview_right: 'Taskbar - Right of Task View button'
      - taskbar_after_widgets_left: 'Taskbar - Left of Widgets button'
      - taskbar_after_widgets_right: 'Taskbar - Right of Widgets button'
      - taskbar_left_edge: 'Taskbar - Left edge (Overlay)'
      - taskbar_center_edge: 'Taskbar - Center (Overlay)'
      - taskbar_right_edge: 'Taskbar - Right edge (Overlay)'
  - leftMargin: 8
    $name: Left gap (px)
  - rightMargin: 8
    $name: Right gap (px)
  $name: Placement

- Sizing:
  - widthMin: 0
    $name: Minimum width (px)
    $description: 'Default: 0, meaning no minimum. Content decides the width.'
  - widthMax: 0
    $name: Maximum width (px)
    $description: 'Default: 0, meaning no maximum.'
  - heightMin: 0
    $name: Minimum height (px)
  - heightMax: 0
    $name: Maximum height (px)
  - rowGap: 2
    $name: Row gap (px)
  - columnGap: 14
    $name: Column gap between sections (px)
  - graphWidth: 56
    $name: History graph width (px)
  - cellGap: 8
    $name: Gap between values (px)
    $description: >-
      Default: 8. Columns size themselves to their content, so this single number is
      the whole of the horizontal spacing between values.
  - barHeight: 3
    $name: RAM/VRAM bar thickness (px)
  $name: Sizing & Spacing

- Metrics:
  - updateInterval: 1
    $name: Update interval (seconds)
    $description: 'Default: 1. From 1 to 10 seconds.'
  - historySeconds: 60
    $name: Graph history (seconds)
    $description: 'Default: 60. From 15 to 180 seconds.'
  - gpuAdapter: ""
    $name: GPU adapter filter
    $description: 'Optional partial adapter name. Empty selects the adapter with the most dedicated VRAM.'
  - usageDecimals: 0
    $name: Usage percent decimals
    $description: 'Default: 0. From 0 to 1 (e.g. 13.2% instead of 13%).'
  - capacityDecimals: 2
    $name: RAM/VRAM capacity decimals
    $description: 'Default: 2 (e.g. 16.68/32.00G). From 0 to 2.'
  - numberPadding: zero
    $name: Number padding
    $description: >-
      Default: Leading zeros. Pads every value to a constant digit count so nothing
      shifts as numbers grow or shrink - "07.8%" rather than "7.8%". Combined with
      tabular figures (see Font below) this makes the columns perfectly stable.
    $options:
      - zero: 'Leading zeros (07.8%)'
      - space: 'Blank space ( 7.8%)'
      - none: 'None (7.8%)'
  $name: Metrics

- Display:
  - showCpuRow: true
    $name: CPU row
    $description: >-
      Master switch for the CPU line. Off hides the whole row and stops reading
      anything only it needed.
  - showCpuUsage: true
    $name: "CPU: usage"
  - showCpuTemp: true
    $name: "CPU: temperature"
  - showCpuClock: true
    $name: "CPU: frequency"
  - showCpuPower: true
    $name: "CPU: power"
  - showCpuGraph: true
    $name: "CPU: history graph"
  - showGpuRow: true
    $name: GPU row
    $description: >-
      Master switch for the GPU line. Off with VRAM also off stops the GPU performance
      counters entirely - the way to turn GPU monitoring off altogether.
  - showGpuUsage: true
    $name: "GPU: usage"
  - showGpuTemp: true
    $name: "GPU: temperature"
  - showGpuClock: true
    $name: "GPU: frequency"
  - showGpuPower: true
    $name: "GPU: power"
  - showGpuGraph: true
    $name: "GPU: history graph"
  - showRamRow: true
    $name: RAM row
  - showRamPercent: true
    $name: "RAM: percentage"
  - showRamCapacity: true
    $name: "RAM: used/total"
  - showRamBar: true
    $name: "RAM: capacity bar"
  - showVramRow: true
    $name: VRAM row
  - showVramPercent: true
    $name: "VRAM: percentage"
  - showVramCapacity: true
    $name: "VRAM: used/total"
  - showVramBar: true
    $name: "VRAM: capacity bar"
  $name: Visible Fields
  $description: >-
    Every reading can be hidden on its own. Columns size to what is left, so hiding a
    field closes its gap rather than leaving a hole, and a row whose fields are all
    hidden takes no vertical space at all.

- Network:
  - showNetwork: true
    $name: Show network throughput
  - networkAdapter: ""
    $name: Network adapter filter
    $description: 'Optional partial adapter name. Empty sums every adapter.'
  - networkFormat: mbsDynamic
    $name: Network speed format
    $options:
      - mbs: MB/s (fixed)
      - mbsDynamic: KB/s or MB/s (dynamic)
      - mbits: Mbit/s (fixed)
      - mbitsDynamic: Kbit/s or Mbit/s (dynamic)
  - networkDecimals: -1
    $name: Network speed decimals
    $description: 'Default: -1, meaning automatic (0 decimals above 100, else 1). 0 to 2 forces a fixed count.'
  $name: Network

- InternetStatus:
  - showInternetStatus: true
    $name: Show internet-status indicator
  - netDotShape: circle
    $name: Indicator shape
    $options:
      - circle: Circle
      - rounded: Rounded square
      - square: Square
      - bar: Vertical bar
  - netDotMarginLeft: 0
    $name: Indicator gap on the left (px)
  - netDotMarginRight: 4
    $name: Indicator gap on the right (px)
  - primaryHost: 8.8.8.8
    $name: Primary check host
  - secondaryHost: 1.1.1.1
    $name: Secondary check host
    $description: 'Tried only when the primary host does not answer.'
  - checkIntervalSeconds: 5
    $name: Check interval (seconds)
  - timeoutMs: 2000
    $name: Ping timeout (ms)
  - connectedText: Online
    $name: Connected text
  - disconnectedText: Offline
    $name: Disconnected text
  $name: Internet Status

- Temperature:
  - temperatureSource: auto
    $name: Temperature source
    $description: >-
      Default: Automatic. Tries HWiNFO first, then LibreHardwareMonitor (if enabled
      below), then Windows D3DKMT for GPU and ACPI thermal zones for CPU.
    $options:
      - auto: Automatic
      - hwinfoAuto: HWiNFO automatic
      - sharedMemory: HWiNFO Shared Memory
      - gadgetRegistry: HWiNFO Gadget Registry
      - lhm: LibreHardwareMonitor only
      - windowsNative: Windows native (ACPI CPU + D3DKMT GPU)
      - disabled: Disabled
  - cpuTempSensor: ""
    $name: CPU temperature sensor filter
  - gpuTempSensor: ""
    $name: GPU temperature sensor filter
  - windowsThermalZoneFilter: ""
    $name: Windows thermal zone filter
  - windowsThermalZoneAggregation: average
    $name: Windows thermal zone aggregation
    $options:
      - average: Average
      - hottest: Hottest
  - cpuWarningTemp: 75
    $name: CPU temperature warning (°C)
  - cpuCriticalTemp: 85
    $name: CPU temperature critical (°C)
  - gpuWarningTemp: 80
    $name: GPU temperature warning (°C)
  - gpuCriticalTemp: 90
    $name: GPU temperature critical (°C)
  $name: Temperature

- ExtraSensors:
  - extraSensorsSource: auto
    $name: Clock/power source
    $description: >-
      Default: Automatic. HWiNFO first, then LibreHardwareMonitor (if enabled below),
      then the native fallback for CPU clock only.
    $options:
      - auto: Automatic (HWiNFO + LHM + native fallback)
      - hwinfo: HWiNFO only
      - lhm: LibreHardwareMonitor only
      - disabled: Disabled
  - cpuClockSensor: ""
    $name: CPU clock sensor filter (HWiNFO)
  - gpuClockSensor: ""
    $name: GPU clock sensor filter (HWiNFO)
  - cpuPowerSensor: ""
    $name: CPU power sensor filter (HWiNFO)
  - gpuPowerSensor: ""
    $name: GPU power sensor filter (HWiNFO)
  $name: Clock & Power (HWiNFO)

- LibreHardwareMonitor:
  - lhmEnabled: false
    $name: Enable LibreHardwareMonitor integration
    $description: >-
      Default: false. An alternative to HWiNFO for temperature, clock and power -
      does not need HWiNFO installed or running. Requires LibreHardwareMonitor running
      with its web server enabled (Options > Remote Web Server > Run). Used as a
      fallback behind HWiNFO in Automatic mode, unless "LibreHardwareMonitor only" is
      selected above.
  - lhmPort: 8085
    $name: LibreHardwareMonitor web server port
    $description: 'Default: 8085, matching LibreHardwareMonitor''s own default.'
  - lhmUpdateInterval: 2
    $name: LibreHardwareMonitor fetch interval (seconds)
    $description: 'Default: 2. How often to fetch from the LHM web server, independent of the general update interval above.'
  $name: LibreHardwareMonitor (alternative to HWiNFO)

- MemoryAlerts:
  - memoryWarningPercent: 80
    $name: Memory usage warning (%)
  - memoryCriticalPercent: 90
    $name: Memory usage critical (%)
  $name: Memory Alerts

- Appearance:
  - fontFamily: "Consolas"
    $name: Font
    $description: >-
      Default: Consolas. A monospaced font is strongly recommended - every digit is
      then the same width, which is what actually keeps the columns from shifting as
      values change. Proportional fonts (the Segoe entries) are only stable if the font
      supports tabular figures, which the setting below requests where available.
    $options:
      - "Consolas": 'Consolas (monospace, on every Windows)'
      - "Cascadia Mono": 'Cascadia Mono (monospace, Windows 11)'
      - "Cascadia Code": 'Cascadia Code (monospace, Windows 11)'
      - "Lucida Console": 'Lucida Console (monospace)'
      - "Courier New": 'Courier New (monospace)'
      - "Segoe UI Variable Text": 'Segoe UI Variable Text (system default, proportional)'
      - "Segoe UI": 'Segoe UI (proportional)'
      - "": 'Custom - use the field below'
  - fontFamilyCustom: ""
    $name: Custom font family
    $description: 'Only used when Font is set to "Custom". Any installed family name.'
  - tabularFigures: true
    $name: Force tabular figures
    $description: >-
      Default: true. Asks the font for fixed-width digits (the OpenType "tnum" feature).
      Monospaced fonts are unaffected; for proportional fonts that support it, such as
      Segoe UI, this is what stops digits of different widths shifting the layout.
  - fontSize: 11
    $name: Font size (px)
    $description: 'From 9 to 13.'
  - textColor: ""
    $name: Text color
    $description: '#RRGGBB or #AARRGGBB. Empty uses the system color.'
  - labelColor: ""
    $name: Label color
    $description: 'Colour of the CPU/GPU/RAM/VRAM labels. Empty follows the text color at reduced opacity.'
  - graphColor: "#78A8FF"
    $name: Graph, bar and dot color
  - warningColor: "#FFFFB900"
    $name: Warning color
  - criticalColor: "#FFFF6B6B"
    $name: Critical color
  - textOpacity: 96
    $name: Text opacity (%)
  - labelOpacity: 62
    $name: Label opacity (%)
    $description: 'Default: 62. Relative to the text opacity, so labels read as secondary.'
  - graphBackgroundOpacity: 10
    $name: Graph background opacity (%)
    $description: >-
      Default: 10. A faint panel behind each history graph so its extent and baseline
      are visible even when the trace is flat. 0 disables it.
  - graphOpacity: 85
    $name: Graph line opacity (%)
  - graphAreaOpacity: 16
    $name: Graph area fill opacity (%)
    $description: >-
      Default: 16. Tints the region under the trace so the line reads against the area
      it covers rather than as a bare squiggle. 0 disables it.
  - showDividers: true
    $name: Show dividers between sections
  - boxMode: hover
    $name: Background box
    $description: >-
      Default: On hover. A card-style background behind the whole widget, matching this
      author's other taskbar forks. "On hover" keeps the taskbar clean until you point
      at the widget; "Always" keeps it drawn permanently.
    $options:
      - hover: On hover
      - always: Always
      - none: Never
  - boxColor: ""
    $name: Box background color
    $description: '#RRGGBB or #AARRGGBB. Empty uses a near-opaque dark gray default.'
  - boxCornerRadius: 6
    $name: Box corner radius (px)
  - boxPadding: 8
    $name: Box inner padding (px)
  $name: Appearance

- Debug:
  - verboseLogging: false
    $name: Verbose logging
  $name: Debug
*/
// ==/WindhawkModSettings==

#include <winsock2.h>
#include <ws2tcpip.h>

#include <windhawk_utils.h>

#include <dxgi.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <wininet.h>
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <deque>
#include <limits>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#undef GetCurrentTime

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Documents.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Shapes.h>
#include <winrt/Windows.UI.Xaml.h>

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;
using XamlPolyline = winrt::Windows::UI::Xaml::Shapes::Polyline;
using XamlRectangle = winrt::Windows::UI::Xaml::Shapes::Rectangle;
using XamlPolygon = winrt::Windows::UI::Xaml::Shapes::Polygon;

namespace {

// Every row is one band of text plus a reserved strip beneath it for the memory bars.
// Text is Top-aligned at a fixed line height and bars are Bottom-aligned, so text in
// every column - metric rows, memory rows, network rows - shares one baseline while the
// bars still get clear space instead of overlapping the glyphs.
//
// The total must stay under roughly 40px: that is what the tray gives a child inside a
// 48px taskbar, and it clips the overflow from the bottom (which is what hid the VRAM
// bar entirely, since it sits on the last row's bottom edge).
//   17 + gap + 17 + 2*2 padding = 38 + gap
constexpr wchar_t kWidgetName[] = L"WindhawkTaskbarSystemInfoFork";
constexpr double kRowHeight = 17.0;
constexpr double kTextLineHeight = 13.0;
constexpr double kGraphHeight = 11.0;
constexpr double kGraphLeftGap = 6.0;
constexpr double kDotSize = 7.0;
constexpr uint32_t kHwInfoSignature = 0x53695748;  // "HWiS"
// HWiNFO shared-memory reading types (SDK-documented, stable across versions).
constexpr uint32_t kHwInfoReadingTemperature = 1;
constexpr uint32_t kHwInfoReadingPower = 4;
constexpr uint32_t kHwInfoReadingClock = 5;

enum class TemperatureSource {
    Auto,
    HwInfoAuto,
    SharedMemory,
    GadgetRegistry,
    Lhm,
    WindowsNative,
    Disabled,
};

enum class ExtraSensorsSource {
    Auto,
    HwInfoOnly,
    LhmOnly,
    Disabled,
};

enum class ThermalZoneAggregation {
    Average,
    Hottest,
};

enum class MetricProvider {
    None,
    HwInfoSharedMemory,
    HwInfoGadgetRegistry,
    LibreHardwareMonitor,
    WindowsD3dkmt,
    WindowsThermalZones,
    WindowsPowerInformation,
};

enum class BoxMode {
    Hover,
    Always,
    None,
};

enum class DotShape {
    Circle,
    Rounded,
    Square,
    Bar,
};

enum class NumberPadding {
    Zero,
    Space,
    None,
};

enum class NetworkFormat {
    Mbs,
    MbsDynamic,
    Mbits,
    MbitsDynamic,
};

struct ModSettings {
    std::wstring position = L"tray_left";
    std::wstring fontFamily;
    std::wstring textColor;
    std::wstring labelColor;
    std::wstring graphColor;
    std::wstring boxColor;
    std::wstring warningColor;
    std::wstring criticalColor;
    std::wstring gpuAdapter;
    std::wstring networkAdapter;
    std::wstring cpuTempSensor;
    std::wstring gpuTempSensor;
    std::wstring cpuClockSensor;
    std::wstring gpuClockSensor;
    std::wstring cpuPowerSensor;
    std::wstring gpuPowerSensor;
    std::wstring windowsThermalZoneFilter;
    std::wstring primaryHost = L"8.8.8.8";
    std::wstring secondaryHost = L"1.1.1.1";
    std::wstring connectedText = L"Online";
    std::wstring disconnectedText = L"Offline";
    TemperatureSource temperatureSource = TemperatureSource::Auto;
    ExtraSensorsSource extraSensorsSource = ExtraSensorsSource::Auto;
    ThermalZoneAggregation windowsThermalZoneAggregation =
        ThermalZoneAggregation::Average;
    NetworkFormat networkFormat = NetworkFormat::MbsDynamic;
    int leftMargin = 8;
    int rightMargin = 8;
    int widthMin = 0;
    int widthMax = 0;
    int heightMin = 0;
    int heightMax = 0;
    int rowGap = 2;
    int columnGap = 14;
    int graphWidth = 56;
    int cellGap = 8;
    int barHeight = 3;
    int updateInterval = 1;
    int historySeconds = 60;
    bool showCpuRow = true;
    bool showCpuUsage = true;
    bool showCpuTemp = true;
    bool showCpuClock = true;
    bool showCpuPower = true;
    bool showCpuGraph = true;
    bool showGpuRow = true;
    bool showGpuUsage = true;
    bool showGpuTemp = true;
    bool showGpuClock = true;
    bool showGpuPower = true;
    bool showGpuGraph = true;
    bool showRamRow = true;
    bool showRamPercent = true;
    bool showRamCapacity = true;
    bool showRamBar = true;
    bool showVramRow = true;
    bool showVramPercent = true;
    bool showVramCapacity = true;
    bool showVramBar = true;
    int usageDecimals = 0;
    int capacityDecimals = 2;
    NumberPadding numberPadding = NumberPadding::Zero;
    bool showNetwork = true;
    int networkDecimals = -1;
    bool showInternetStatus = true;
    DotShape netDotShape = DotShape::Circle;
    int netDotMarginLeft = 0;
    int netDotMarginRight = 4;
    int checkIntervalSeconds = 5;
    int timeoutMs = 2000;
    bool lhmEnabled = false;
    int lhmPort = 8085;
    int lhmUpdateInterval = 2;
    int fontSize = 11;
    int textOpacity = 96;
    int labelOpacity = 62;
    bool tabularFigures = true;
    int graphBackgroundOpacity = 10;
    int graphOpacity = 85;
    int graphAreaOpacity = 16;
    bool showDividers = true;
    BoxMode boxMode = BoxMode::Hover;
    int boxCornerRadius = 6;
    int boxPadding = 8;
    int cpuWarningTemp = 75;
    int cpuCriticalTemp = 85;
    int gpuWarningTemp = 80;
    int gpuCriticalTemp = 90;
    int memoryWarningPercent = 80;
    int memoryCriticalPercent = 90;
    bool verboseLogging = false;
};

// One place deciding what is on screen, so the readers and the layout can never
// disagree about whether something is needed. A row is present only if at least one of
// its own fields is - otherwise it would reserve a row's height to show nothing.
bool CpuRowVisible(const ModSettings& s) {
    return s.showCpuRow && (s.showCpuUsage || s.showCpuTemp || s.showCpuClock ||
                            s.showCpuPower || s.showCpuGraph);
}
bool GpuRowVisible(const ModSettings& s) {
    return s.showGpuRow && (s.showGpuUsage || s.showGpuTemp || s.showGpuClock ||
                            s.showGpuPower || s.showGpuGraph);
}
bool RamRowVisible(const ModSettings& s) {
    return s.showRamRow && (s.showRamPercent || s.showRamCapacity || s.showRamBar);
}
bool VramRowVisible(const ModSettings& s) {
    return s.showVramRow && (s.showVramPercent || s.showVramCapacity || s.showVramBar);
}
// GPU usage feeds both the number and the graph, so either one keeps the counter alive.
bool WantGpuUsage(const ModSettings& s) {
    return s.showGpuRow && (s.showGpuUsage || s.showGpuGraph);
}
bool WantCpuClockPower(const ModSettings& s) {
    return s.showCpuRow && (s.showCpuClock || s.showCpuPower);
}
bool WantGpuClockPower(const ModSettings& s) {
    return s.showGpuRow && (s.showGpuClock || s.showGpuPower);
}
bool WantAnyTemperature(const ModSettings& s) {
    return (s.showCpuRow && s.showCpuTemp) || (s.showGpuRow && s.showGpuTemp);
}

ModSettings g_settings;
std::mutex g_settingsMutex;
std::atomic<bool> g_unloading;
std::atomic<bool> g_uiTornDown;
std::atomic<bool> g_taskbarViewDllLoaded;
std::atomic<HWND> g_taskbarWindow{nullptr};
std::atomic<DWORD> g_taskbarThreadId{0};

// Injection target/state. `panel` is the tray or taskbar RootGrid we ended up parented
// to; `column` >= 0 means a pre-26300 Grid tray (reserve a ColumnDefinition);
// `childIndex` >= 0 means a 26300+ StackPanel tray (insert at that index); both stay -1
// for overlay/anchored positions, which are placed by margin instead. Same shape as
// taskbar-ai-quota-fork's InjectionTarget, reused here for the same reason.
struct InjectionTarget {
    Panel panel{nullptr};
    int column = -1;
    int childIndex = -1;
};

[[clang::no_destroy]] Grid g_widget{nullptr};
// The outer Border that's actually positioned/injected/sized - g_widget (the content
// Grid) is its Child. A Border, not the Grid itself, because Grid has no Padding
// property in this XAML version, and a card-style background needs one.
[[clang::no_destroy]] Border g_widgetBorder{nullptr};
bool g_widgetHovered = false;
event_token g_pointerEnteredToken{};
event_token g_pointerExitedToken{};
[[clang::no_destroy]] Panel g_injectionParent{nullptr};
// Tray sub-positions (tray_left, tray_before_clock, ...) all resolve to the same
// SystemTrayFrameGrid object, so a plain "same panel" check can't tell a real
// position change apart from an untouched one - track the string too.
std::wstring g_lastInjectedPosition;
int g_widgetColumn = -1;
bool g_widgetInsertedColumn = false;
[[clang::no_destroy]] FrameworkElement g_trackedElement{nullptr};
Thickness g_trackedOriginalMargin{};
bool g_hasTrackedOriginalMargin = false;
bool g_trackAnchorOnLeft = false;
event_token g_layoutUpdatedToken{};
bool g_hasLayoutUpdatedToken = false;

[[clang::no_destroy]] DispatcherTimer g_timer{nullptr};
event_token g_timerToken{};
[[clang::no_destroy]] std::optional<std::list<FrameworkElement::Loaded_revoker>>
    g_loadedRevokers{std::in_place};

[[clang::no_destroy]] TextBlock g_cpuLabel{nullptr};
[[clang::no_destroy]] TextBlock g_cpuUsageText{nullptr};
[[clang::no_destroy]] TextBlock g_cpuTempText{nullptr};
[[clang::no_destroy]] TextBlock g_cpuClockText{nullptr};
[[clang::no_destroy]] TextBlock g_cpuPowerText{nullptr};
[[clang::no_destroy]] TextBlock g_gpuLabel{nullptr};
[[clang::no_destroy]] TextBlock g_gpuUsageText{nullptr};
[[clang::no_destroy]] TextBlock g_gpuTempText{nullptr};
[[clang::no_destroy]] TextBlock g_gpuClockText{nullptr};
[[clang::no_destroy]] TextBlock g_gpuPowerText{nullptr};
[[clang::no_destroy]] TextBlock g_ramLabel{nullptr};
[[clang::no_destroy]] TextBlock g_ramPercentText{nullptr};
[[clang::no_destroy]] TextBlock g_ramCapacityText{nullptr};
[[clang::no_destroy]] TextBlock g_vramLabel{nullptr};
[[clang::no_destroy]] TextBlock g_vramPercentText{nullptr};
[[clang::no_destroy]] TextBlock g_vramCapacityText{nullptr};
[[clang::no_destroy]] TextBlock g_netDownText{nullptr};
[[clang::no_destroy]] TextBlock g_netUpText{nullptr};
[[clang::no_destroy]] TextBlock g_netDownArrow{nullptr};
[[clang::no_destroy]] TextBlock g_netUpArrow{nullptr};
[[clang::no_destroy]] XamlRectangle g_netDot{nullptr};
[[clang::no_destroy]] XamlPolyline g_cpuGraph{nullptr};
[[clang::no_destroy]] XamlPolyline g_gpuGraph{nullptr};
[[clang::no_destroy]] XamlRectangle g_cpuGraphBackground{nullptr};
[[clang::no_destroy]] XamlRectangle g_gpuGraphBackground{nullptr};
[[clang::no_destroy]] XamlRectangle g_dividerNet{nullptr};
[[clang::no_destroy]] XamlRectangle g_dividerPanels{nullptr};

[[clang::no_destroy]] XamlRectangle g_ramTrack{nullptr};
[[clang::no_destroy]] XamlRectangle g_ramFill{nullptr};
[[clang::no_destroy]] XamlRectangle g_vramTrack{nullptr};
[[clang::no_destroy]] XamlRectangle g_vramFill{nullptr};
[[clang::no_destroy]] ColumnDefinition g_leftColumn{nullptr};
[[clang::no_destroy]] ColumnDefinition g_gapColumn{nullptr};
[[clang::no_destroy]] ColumnDefinition g_rightColumn{nullptr};
[[clang::no_destroy]] XamlPolygon g_cpuGraphArea{nullptr};
[[clang::no_destroy]] XamlPolygon g_gpuGraphArea{nullptr};
[[clang::no_destroy]] ColumnDefinition g_netGapColumn{nullptr};
[[clang::no_destroy]] ColumnDefinition g_netColumnDef{nullptr};
[[clang::no_destroy]] RowDefinition g_leftGapRow{nullptr};
[[clang::no_destroy]] RowDefinition g_cpuRowDef{nullptr};
[[clang::no_destroy]] RowDefinition g_gpuRowDef{nullptr};
[[clang::no_destroy]] RowDefinition g_ramRowDef{nullptr};
[[clang::no_destroy]] RowDefinition g_vramRowDef{nullptr};
[[clang::no_destroy]] Grid g_leftPanel{nullptr};
[[clang::no_destroy]] Grid g_rightPanel{nullptr};
[[clang::no_destroy]] Grid g_netColumn{nullptr};
[[clang::no_destroy]] RowDefinition g_rightGapRow{nullptr};
[[clang::no_destroy]] RowDefinition g_netGapRow{nullptr};
double g_graphWidth = 96.0;

std::deque<double> g_cpuHistory;
std::deque<double> g_gpuHistory;
int g_historyInterval = 0;
int g_historyWindow = 0;

PDH_HQUERY g_pdhQuery = nullptr;
PDH_HCOUNTER g_gpuCounter = nullptr;
PDH_HCOUNTER g_vramCounter = nullptr;
PDH_HCOUNTER g_sharedVramCounter = nullptr;
PDH_HCOUNTER g_thermalZoneCounter = nullptr;
PDH_HCOUNTER g_netRecvCounter = nullptr;
PDH_HCOUNTER g_netSentCounter = nullptr;
std::chrono::steady_clock::time_point g_nextPdhCounterRetry{};
std::chrono::steady_clock::time_point g_nextPdhRecovery{};
std::chrono::steady_clock::time_point g_nextGpuIdentityCheck{};
uint32_t g_consecutivePdhReadFailures = 0;

struct MetricsSnapshot {
    double cpu = 0.0;
    double ram = 0.0;
    double ramUsedGb = 0.0;
    double ramTotalGb = 0.0;
    double gpu = 0.0;
    bool gpuAvailable = false;
    double vram = 0.0;
    double vramUsedGb = 0.0;
    double vramTotalGb = 0.0;
    bool vramAvailable = false;
    double networkRecvBytesPerSec = 0.0;
    double networkSentBytesPerSec = 0.0;
    bool networkRecvAvailable = false;
    bool networkSentAvailable = false;
    std::optional<double> cpuTemp;
    std::optional<double> gpuTemp;
    std::optional<double> cpuClockMhz;
    std::optional<double> gpuClockMhz;
    std::optional<double> cpuPowerW;
    std::optional<double> gpuPowerW;
    MetricProvider cpuTempProvider = MetricProvider::None;
    MetricProvider gpuTempProvider = MetricProvider::None;
    MetricProvider cpuClockProvider = MetricProvider::None;
    MetricProvider gpuClockProvider = MetricProvider::None;
    MetricProvider cpuPowerProvider = MetricProvider::None;
    MetricProvider gpuPowerProvider = MetricProvider::None;
};

std::mutex g_metricsMutex;
MetricsSnapshot g_latestMetrics;
uint64_t g_latestMetricsSequence = 0;
bool g_latestMetricsAvailable = false;
uint64_t g_lastRenderedMetricsSequence = 0;
bool GetLatestMetrics(MetricsSnapshot& snapshot, uint64_t& sequence);

std::mutex g_metricsWorkerMutex;
std::atomic<bool> g_stopMetricsWorker{false};
HANDLE g_metricsWorkerWakeEvent = nullptr;
[[clang::no_destroy]] std::optional<std::thread> g_metricsWorker;

// Internet status runs on its own cadence (ping timeout, not the metrics interval), and
// is read directly by the UI tick rather than folded into MetricsSnapshot.
std::atomic<bool> g_stopInternetWorker{false};
HANDLE g_internetWorkerWakeEvent = nullptr;
[[clang::no_destroy]] std::optional<std::thread> g_internetWorker;
enum class InternetState { Unknown, Connected, Disconnected };
std::atomic<InternetState> g_internetState{InternetState::Unknown};

std::atomic<bool> g_stopWatchdog{false};
HANDLE g_watchdogWakeEvent = nullptr;
HANDLE g_watchdogThreadHandle = nullptr;
std::atomic<ULONGLONG> g_nextInjectFailureLogMs{0};

std::wstring GetStringSetting(PCWSTR name) {
    return WindhawkUtils::StringSetting::make(name).get();
}

TemperatureSource ParseTemperatureSource(const std::wstring& value) {
    if (value == L"hwinfoAuto") return TemperatureSource::HwInfoAuto;
    if (value == L"sharedMemory") return TemperatureSource::SharedMemory;
    if (value == L"gadgetRegistry") return TemperatureSource::GadgetRegistry;
    if (value == L"lhm") return TemperatureSource::Lhm;
    if (value == L"windowsNative") return TemperatureSource::WindowsNative;
    if (value == L"disabled") return TemperatureSource::Disabled;
    return TemperatureSource::Auto;
}

ExtraSensorsSource ParseExtraSensorsSource(const std::wstring& value) {
    if (value == L"hwinfo") return ExtraSensorsSource::HwInfoOnly;
    if (value == L"lhm") return ExtraSensorsSource::LhmOnly;
    if (value == L"disabled") return ExtraSensorsSource::Disabled;
    return ExtraSensorsSource::Auto;
}

ThermalZoneAggregation ParseThermalZoneAggregation(const std::wstring& value) {
    return value == L"hottest" ? ThermalZoneAggregation::Hottest
                                : ThermalZoneAggregation::Average;
}

DotShape ParseDotShape(const std::wstring& value) {
    if (value == L"rounded") return DotShape::Rounded;
    if (value == L"square") return DotShape::Square;
    if (value == L"bar") return DotShape::Bar;
    return DotShape::Circle;
}

NumberPadding ParseNumberPadding(const std::wstring& value) {
    if (value == L"space") return NumberPadding::Space;
    if (value == L"none") return NumberPadding::None;
    return NumberPadding::Zero;
}

BoxMode ParseBoxMode(const std::wstring& value) {
    if (value == L"always") return BoxMode::Always;
    if (value == L"none") return BoxMode::None;
    return BoxMode::Hover;
}

NetworkFormat ParseNetworkFormat(const std::wstring& value) {
    if (value == L"mbs") return NetworkFormat::Mbs;
    if (value == L"mbits") return NetworkFormat::Mbits;
    if (value == L"mbitsDynamic") return NetworkFormat::MbitsDynamic;
    return NetworkFormat::MbsDynamic;
}

void LoadSettings() {
    ModSettings settings;
    settings.position = GetStringSetting(L"Placement.position");
    settings.fontFamily = GetStringSetting(L"Appearance.fontFamily");
    if (settings.fontFamily.empty()) {
        settings.fontFamily = GetStringSetting(L"Appearance.fontFamilyCustom");
    }
    settings.textColor = GetStringSetting(L"Appearance.textColor");
    settings.labelColor = GetStringSetting(L"Appearance.labelColor");
    settings.graphColor = GetStringSetting(L"Appearance.graphColor");
    settings.boxColor = GetStringSetting(L"Appearance.boxColor");
    settings.warningColor = GetStringSetting(L"Appearance.warningColor");
    settings.criticalColor = GetStringSetting(L"Appearance.criticalColor");
    settings.gpuAdapter = GetStringSetting(L"Metrics.gpuAdapter");
    settings.networkAdapter = GetStringSetting(L"Network.networkAdapter");
    settings.cpuTempSensor = GetStringSetting(L"Temperature.cpuTempSensor");
    settings.gpuTempSensor = GetStringSetting(L"Temperature.gpuTempSensor");
    settings.cpuClockSensor = GetStringSetting(L"ExtraSensors.cpuClockSensor");
    settings.gpuClockSensor = GetStringSetting(L"ExtraSensors.gpuClockSensor");
    settings.cpuPowerSensor = GetStringSetting(L"ExtraSensors.cpuPowerSensor");
    settings.gpuPowerSensor = GetStringSetting(L"ExtraSensors.gpuPowerSensor");
    settings.windowsThermalZoneFilter =
        GetStringSetting(L"Temperature.windowsThermalZoneFilter");
    settings.primaryHost = GetStringSetting(L"InternetStatus.primaryHost");
    settings.secondaryHost = GetStringSetting(L"InternetStatus.secondaryHost");
    settings.connectedText = GetStringSetting(L"InternetStatus.connectedText");
    settings.disconnectedText = GetStringSetting(L"InternetStatus.disconnectedText");
    settings.temperatureSource =
        ParseTemperatureSource(GetStringSetting(L"Temperature.temperatureSource"));
    settings.extraSensorsSource =
        ParseExtraSensorsSource(GetStringSetting(L"ExtraSensors.extraSensorsSource"));
    settings.windowsThermalZoneAggregation = ParseThermalZoneAggregation(
        GetStringSetting(L"Temperature.windowsThermalZoneAggregation"));
    settings.networkFormat =
        ParseNetworkFormat(GetStringSetting(L"Network.networkFormat"));

    settings.leftMargin = std::clamp(Wh_GetIntSetting(L"Placement.leftMargin"), 0, 200);
    settings.rightMargin = std::clamp(Wh_GetIntSetting(L"Placement.rightMargin"), 0, 200);
    settings.widthMin = std::clamp(Wh_GetIntSetting(L"Sizing.widthMin"), 0, 2000);
    settings.widthMax = std::clamp(Wh_GetIntSetting(L"Sizing.widthMax"), 0, 2000);
    settings.heightMin = std::clamp(Wh_GetIntSetting(L"Sizing.heightMin"), 0, 500);
    settings.heightMax = std::clamp(Wh_GetIntSetting(L"Sizing.heightMax"), 0, 500);
    settings.rowGap = std::clamp(Wh_GetIntSetting(L"Sizing.rowGap"), 0, 40);
    settings.columnGap = std::clamp(Wh_GetIntSetting(L"Sizing.columnGap"), 0, 100);
    settings.graphWidth = std::clamp(Wh_GetIntSetting(L"Sizing.graphWidth"), 20, 400);
    settings.cellGap = std::clamp(Wh_GetIntSetting(L"Sizing.cellGap"), 0, 40);
    settings.barHeight = std::clamp(Wh_GetIntSetting(L"Sizing.barHeight"), 1, 6);
    settings.updateInterval =
        std::clamp(Wh_GetIntSetting(L"Metrics.updateInterval"), 1, 10);
    settings.historySeconds =
        std::clamp(Wh_GetIntSetting(L"Metrics.historySeconds"), 15, 180);
    settings.showCpuRow = Wh_GetIntSetting(L"Display.showCpuRow") != 0;
    settings.showCpuUsage = Wh_GetIntSetting(L"Display.showCpuUsage") != 0;
    settings.showCpuTemp = Wh_GetIntSetting(L"Display.showCpuTemp") != 0;
    settings.showCpuClock = Wh_GetIntSetting(L"Display.showCpuClock") != 0;
    settings.showCpuPower = Wh_GetIntSetting(L"Display.showCpuPower") != 0;
    settings.showCpuGraph = Wh_GetIntSetting(L"Display.showCpuGraph") != 0;
    settings.showGpuRow = Wh_GetIntSetting(L"Display.showGpuRow") != 0;
    settings.showGpuUsage = Wh_GetIntSetting(L"Display.showGpuUsage") != 0;
    settings.showGpuTemp = Wh_GetIntSetting(L"Display.showGpuTemp") != 0;
    settings.showGpuClock = Wh_GetIntSetting(L"Display.showGpuClock") != 0;
    settings.showGpuPower = Wh_GetIntSetting(L"Display.showGpuPower") != 0;
    settings.showGpuGraph = Wh_GetIntSetting(L"Display.showGpuGraph") != 0;
    settings.showRamRow = Wh_GetIntSetting(L"Display.showRamRow") != 0;
    settings.showRamPercent = Wh_GetIntSetting(L"Display.showRamPercent") != 0;
    settings.showRamCapacity = Wh_GetIntSetting(L"Display.showRamCapacity") != 0;
    settings.showRamBar = Wh_GetIntSetting(L"Display.showRamBar") != 0;
    settings.showVramRow = Wh_GetIntSetting(L"Display.showVramRow") != 0;
    settings.showVramPercent = Wh_GetIntSetting(L"Display.showVramPercent") != 0;
    settings.showVramCapacity = Wh_GetIntSetting(L"Display.showVramCapacity") != 0;
    settings.showVramBar = Wh_GetIntSetting(L"Display.showVramBar") != 0;
    settings.usageDecimals = std::clamp(Wh_GetIntSetting(L"Metrics.usageDecimals"), 0, 1);
    settings.capacityDecimals =
        std::clamp(Wh_GetIntSetting(L"Metrics.capacityDecimals"), 0, 2);
    settings.numberPadding =
        ParseNumberPadding(GetStringSetting(L"Metrics.numberPadding"));
    settings.showNetwork = Wh_GetIntSetting(L"Network.showNetwork") != 0;
    settings.networkDecimals =
        std::clamp(Wh_GetIntSetting(L"Network.networkDecimals"), -1, 2);
    settings.showInternetStatus =
        Wh_GetIntSetting(L"InternetStatus.showInternetStatus") != 0;
    settings.netDotShape = ParseDotShape(GetStringSetting(L"InternetStatus.netDotShape"));
    settings.netDotMarginLeft =
        std::clamp(Wh_GetIntSetting(L"InternetStatus.netDotMarginLeft"), 0, 40);
    settings.netDotMarginRight =
        std::clamp(Wh_GetIntSetting(L"InternetStatus.netDotMarginRight"), 0, 40);
    settings.checkIntervalSeconds =
        std::clamp(Wh_GetIntSetting(L"InternetStatus.checkIntervalSeconds"), 2, 120);
    settings.timeoutMs =
        std::clamp(Wh_GetIntSetting(L"InternetStatus.timeoutMs"), 200, 10000);
    settings.lhmEnabled = Wh_GetIntSetting(L"LibreHardwareMonitor.lhmEnabled") != 0;
    settings.lhmPort = std::clamp(Wh_GetIntSetting(L"LibreHardwareMonitor.lhmPort"), 1, 65535);
    settings.lhmUpdateInterval =
        std::clamp(Wh_GetIntSetting(L"LibreHardwareMonitor.lhmUpdateInterval"), 1, 60);
    settings.fontSize = std::clamp(Wh_GetIntSetting(L"Appearance.fontSize"), 9, 13);
    settings.textOpacity =
        std::clamp(Wh_GetIntSetting(L"Appearance.textOpacity"), 0, 100);
    settings.labelOpacity =
        std::clamp(Wh_GetIntSetting(L"Appearance.labelOpacity"), 0, 100);
    settings.tabularFigures = Wh_GetIntSetting(L"Appearance.tabularFigures") != 0;
    settings.graphBackgroundOpacity =
        std::clamp(Wh_GetIntSetting(L"Appearance.graphBackgroundOpacity"), 0, 100);
    settings.graphOpacity =
        std::clamp(Wh_GetIntSetting(L"Appearance.graphOpacity"), 0, 100);
    settings.graphAreaOpacity =
        std::clamp(Wh_GetIntSetting(L"Appearance.graphAreaOpacity"), 0, 100);
    settings.showDividers = Wh_GetIntSetting(L"Appearance.showDividers") != 0;
    settings.boxMode = ParseBoxMode(GetStringSetting(L"Appearance.boxMode"));
    settings.boxCornerRadius =
        std::clamp(Wh_GetIntSetting(L"Appearance.boxCornerRadius"), 0, 20);
    settings.boxPadding = std::clamp(Wh_GetIntSetting(L"Appearance.boxPadding"), 0, 24);
    settings.cpuWarningTemp =
        std::clamp(Wh_GetIntSetting(L"Temperature.cpuWarningTemp"), 40, 95);
    settings.cpuCriticalTemp =
        std::clamp(Wh_GetIntSetting(L"Temperature.cpuCriticalTemp"),
                   settings.cpuWarningTemp + 1, 105);
    settings.gpuWarningTemp =
        std::clamp(Wh_GetIntSetting(L"Temperature.gpuWarningTemp"), 40, 105);
    settings.gpuCriticalTemp =
        std::clamp(Wh_GetIntSetting(L"Temperature.gpuCriticalTemp"),
                   settings.gpuWarningTemp + 1, 115);
    settings.memoryWarningPercent =
        std::clamp(Wh_GetIntSetting(L"MemoryAlerts.memoryWarningPercent"), 50, 98);
    settings.memoryCriticalPercent =
        std::clamp(Wh_GetIntSetting(L"MemoryAlerts.memoryCriticalPercent"),
                   settings.memoryWarningPercent + 1, 100);
    settings.verboseLogging = Wh_GetIntSetting(L"Debug.verboseLogging") != 0;

    if (settings.position.empty()) settings.position = L"tray_left";
    if (settings.fontFamily.empty()) settings.fontFamily = L"Consolas";
    if (settings.graphColor.empty()) settings.graphColor = L"#78A8FF";
    // Near-opaque dark gray, matching taskbar-fluent-media-player-fork's own default
    // background (solidColor "35 35 35" at 100% opacity when a background is enabled)
    // - a translucent fill was tried first and was nearly invisible against an
    // already-near-black taskbar.
    if (settings.boxColor.empty()) settings.boxColor = L"#E6242424";
    if (settings.warningColor.empty()) settings.warningColor = L"#FFFFB900";
    if (settings.criticalColor.empty()) settings.criticalColor = L"#FFFF6B6B";
    if (settings.primaryHost.empty()) settings.primaryHost = L"8.8.8.8";
    if (settings.secondaryHost.empty()) settings.secondaryHost = L"1.1.1.1";
    if (settings.connectedText.empty()) settings.connectedText = L"Online";
    if (settings.disconnectedText.empty()) settings.disconnectedText = L"Offline";

    std::lock_guard lock(g_settingsMutex);
    g_settings = std::move(settings);
}

ModSettings CurrentSettings() {
    std::lock_guard lock(g_settingsMutex);
    return g_settings;
}

std::wstring ToLower(std::wstring value) {
    std::transform(value.begin(), value.end(), value.begin(), [](wchar_t ch) {
        return static_cast<wchar_t>(std::towlower(ch));
    });
    return value;
}

bool Contains(const std::wstring& text, const std::wstring& needle) {
    return needle.empty() || text.find(needle) != std::wstring::npos;
}

std::wstring FixedAnsiToWide(const char* value, size_t capacity) {
    size_t length = 0;
    while (length < capacity && value[length]) length++;
    if (!length) return {};

    int wideLength =
        MultiByteToWideChar(CP_ACP, 0, value, static_cast<int>(length), nullptr, 0);
    if (wideLength <= 0) return {};

    std::wstring result(wideLength, L'\0');
    MultiByteToWideChar(CP_ACP, 0, value, static_cast<int>(length), result.data(),
                        wideLength);
    return result;
}

std::wstring FixedWideToString(const wchar_t* value, size_t capacity) {
    size_t length = 0;
    while (length < capacity && value[length]) length++;
    return std::wstring(value, length);
}

bool IsCpuSensorName(const std::wstring& sensorLower) {
    bool gpuSensor = Contains(sensorLower, L"gpu") || Contains(sensorLower, L"nvidia") ||
                     Contains(sensorLower, L"radeon");
    return !gpuSensor &&
           (Contains(sensorLower, L"cpu") || Contains(sensorLower, L"processor") ||
            Contains(sensorLower, L"ryzen") || Contains(sensorLower, L"threadripper") ||
            Contains(sensorLower, L"epyc") || Contains(sensorLower, L"xeon") ||
            Contains(sensorLower, L"intel"));
}

bool IsGpuSensorName(const std::wstring& sensorLower) {
    return Contains(sensorLower, L"gpu") || Contains(sensorLower, L"nvidia") ||
           Contains(sensorLower, L"radeon");
}

// Same shape as the base mod's temperature scoring: a preferred-name filter wins
// outright, otherwise a ranked list of known-good labels with a low-priority
// localized-label fallback at the end. Reused with different keyword sets below for
// clock and power so a HWiNFO sensor dump full of per-core/per-rail readings still
// converges on the one composite reading that matches what the widget's single number
// means (package total, average core clock).
int CpuTemperatureScore(const std::wstring& sensorName,
                        const std::wstring& label,
                        const std::wstring& preferred) {
    std::wstring sensor = ToLower(sensorName);
    std::wstring reading = ToLower(label);
    std::wstring combined = sensor + L" " + reading;
    std::wstring preferredLower = ToLower(preferred);

    if (!preferredLower.empty()) {
        return Contains(combined, preferredLower) ? 10000 : -1;
    }
    if (!IsCpuSensorName(sensor)) return -1;
    if (Contains(reading, L"vrm") || Contains(reading, L"ccd") ||
        Contains(reading, L"iod") || Contains(reading, L"soc") ||
        Contains(reading, L"l3 cache")) {
        return -1;
    }
    if (Contains(reading, L"tctl/tdie")) return 1000;
    if (Contains(reading, L"cpu die (average)")) return 950;
    if (Contains(reading, L"cpu package")) return 900;
    if (Contains(reading, L"package temperature")) return 850;
    if (Contains(reading, L"cpu temperature")) return 800;
    if (Contains(reading, L"core temperatures")) return 700;
    if (Contains(reading, L"temperature")) return 400;
    return 100;
}

int GpuTemperatureScore(const std::wstring& sensorName,
                        const std::wstring& label,
                        const std::wstring& preferred) {
    std::wstring sensor = ToLower(sensorName);
    std::wstring reading = ToLower(label);
    std::wstring combined = sensor + L" " + reading;
    std::wstring preferredLower = ToLower(preferred);

    if (!preferredLower.empty()) {
        return Contains(combined, preferredLower) ? 10000 : -1;
    }
    if (!IsGpuSensorName(sensor)) return -1;
    if (Contains(reading, L"hot spot") || Contains(reading, L"hotspot") ||
        Contains(reading, L"memory") || Contains(reading, L"vram")) {
        return -1;
    }
    if (reading == L"gpu temperature") return 1000;
    if (Contains(reading, L"gpu temperature")) return 950;
    if (Contains(reading, L"gpu core")) return 900;
    if (Contains(reading, L"temperature")) return 500;
    if (Contains(reading, L"gpu")) {
        return 400 - static_cast<int>(std::min<size_t>(reading.size(), 200));
    }
    return 100;
}

int CpuClockScore(const std::wstring& sensorName,
                  const std::wstring& label,
                  const std::wstring& preferred) {
    std::wstring sensor = ToLower(sensorName);
    std::wstring reading = ToLower(label);
    std::wstring combined = sensor + L" " + reading;
    std::wstring preferredLower = ToLower(preferred);

    if (!preferredLower.empty()) {
        return Contains(combined, preferredLower) ? 10000 : -1;
    }
    if (!IsCpuSensorName(sensor)) return -1;
    if (Contains(reading, L"bus") || Contains(reading, L"uncore") ||
        Contains(reading, L"ring") || Contains(reading, L"cache") ||
        Contains(reading, L"memory") || Contains(reading, L"multiplier")) {
        return -1;
    }
    if (Contains(reading, L"core clocks (avg)")) return 1000;
    if (Contains(reading, L"cpu clock")) return 900;
    if (Contains(reading, L"average effective clock")) return 850;
    if (Contains(reading, L"core") && Contains(reading, L"clock")) return 500;
    if (Contains(reading, L"clock")) return 400;
    return 100;
}

int GpuClockScore(const std::wstring& sensorName,
                  const std::wstring& label,
                  const std::wstring& preferred) {
    std::wstring sensor = ToLower(sensorName);
    std::wstring reading = ToLower(label);
    std::wstring combined = sensor + L" " + reading;
    std::wstring preferredLower = ToLower(preferred);

    if (!preferredLower.empty()) {
        return Contains(combined, preferredLower) ? 10000 : -1;
    }
    if (!IsGpuSensorName(sensor)) return -1;
    if (Contains(reading, L"memory") || Contains(reading, L"mem clock")) return -1;
    if (reading == L"gpu clock") return 1000;
    if (Contains(reading, L"gpu clock")) return 900;
    if (Contains(reading, L"core clock")) return 800;
    if (Contains(reading, L"shader clock")) return 400;
    if (Contains(reading, L"clock")) return 300;
    return 100;
}

int CpuPowerScore(const std::wstring& sensorName,
                  const std::wstring& label,
                  const std::wstring& preferred) {
    std::wstring sensor = ToLower(sensorName);
    std::wstring reading = ToLower(label);
    std::wstring combined = sensor + L" " + reading;
    std::wstring preferredLower = ToLower(preferred);

    if (!preferredLower.empty()) {
        return Contains(combined, preferredLower) ? 10000 : -1;
    }
    if (!IsCpuSensorName(sensor)) return -1;
    if (Contains(reading, L"limit")) return -1;
    if (Contains(reading, L"package power")) return 1000;
    if (Contains(reading, L"ppt")) return 900;
    if (Contains(reading, L"cpu power")) return 850;
    if (Contains(reading, L"power")) return 400;
    return 100;
}

int GpuPowerScore(const std::wstring& sensorName,
                  const std::wstring& label,
                  const std::wstring& preferred) {
    std::wstring sensor = ToLower(sensorName);
    std::wstring reading = ToLower(label);
    std::wstring combined = sensor + L" " + reading;
    std::wstring preferredLower = ToLower(preferred);

    if (!preferredLower.empty()) {
        return Contains(combined, preferredLower) ? 10000 : -1;
    }
    if (!IsGpuSensorName(sensor)) return -1;
    if (Contains(reading, L"limit")) return -1;
    if (reading == L"gpu power") return 1000;
    if (Contains(reading, L"gpu power")) return 900;
    if (Contains(reading, L"power")) return 400;
    return 100;
}

// HWiNFO's published shared-memory layout explicitly uses one-byte packing.
#pragma pack(push, 1)
struct HwInfoHeader {
    uint32_t signature;
    uint32_t version;
    uint32_t revision;
    int64_t pollTime;
    uint32_t sensorOffset;
    uint32_t sensorStride;
    uint32_t sensorCount;
    uint32_t readingOffset;
    uint32_t readingStride;
    uint32_t readingCount;
    uint32_t pollingPeriod;
};

struct HwInfoSensorPrefix {
    uint32_t sensorId;
    uint32_t sensorInstance;
    char originalName[128];
    char userName[128];
};

struct HwInfoReadingPrefix {
    uint32_t readingType;
    uint32_t sensorIndex;
    uint32_t readingId;
    char originalLabel[128];
    char userLabel[128];
    char unit[16];
    double value;
};
#pragma pack(pop)

static_assert(sizeof(HwInfoHeader) == 48);
static_assert(offsetof(HwInfoHeader, pollTime) == 12);
static_assert(offsetof(HwInfoHeader, sensorOffset) == 20);
static_assert(sizeof(HwInfoSensorPrefix) == 264);
static_assert(offsetof(HwInfoReadingPrefix, value) == 284);
static_assert(sizeof(HwInfoReadingPrefix) == 292);

bool IsRangeValid(size_t totalSize,
                  uint32_t offset,
                  uint32_t stride,
                  uint32_t count,
                  size_t minimumStride) {
    if (stride < minimumStride || offset > totalSize) return false;
    size_t remaining = totalSize - offset;
    return count <= remaining / stride;
}

std::optional<double> NormalizeTemperature(double value,
                                           std::wstring unitOrFormattedValue) {
    if (!std::isfinite(value)) return std::nullopt;

    std::wstring unit = ToLower(unitOrFormattedValue);
    unit.erase(std::remove_if(unit.begin(), unit.end(),
                              [](wchar_t c) { return std::iswspace(c) != 0; }),
               unit.end());

    bool celsius = Contains(unit, L"°c") || Contains(unit, L"℃") ||
                   unit == L"c" || unit == L"celsius";
    bool fahrenheit = Contains(unit, L"°f") || Contains(unit, L"℉") ||
                      unit == L"f" || unit == L"fahrenheit";
    if (celsius == fahrenheit) return std::nullopt;

    double celsiusValue = fahrenheit ? (value - 32.0) * 5.0 / 9.0 : value;
    if (!std::isfinite(celsiusValue) || celsiusValue < -50.0 || celsiusValue > 200.0) {
        return std::nullopt;
    }
    return celsiusValue;
}

constexpr char HwInfoTemperatureUnit(const char* unit, size_t capacity) {
    char result = 0;
    for (size_t i = 0; i < capacity && unit[i]; i++) {
        char candidate = 0;
        if (unit[i] == 'C' || unit[i] == 'c') {
            candidate = 'C';
        } else if (unit[i] == 'F' || unit[i] == 'f') {
            candidate = 'F';
        }
        if (candidate) {
            if (result && result != candidate) return 0;
            result = candidate;
        }
    }
    return result;
}

constexpr char kHwInfoRawCelsiusUnit[] = {static_cast<char>(0xB0), 'C', 0};
constexpr char kHwInfoRawFahrenheitUnit[] = {static_cast<char>(0xB0), 'F', 0};
static_assert(HwInfoTemperatureUnit(kHwInfoRawCelsiusUnit,
                                    std::size(kHwInfoRawCelsiusUnit)) == 'C');
static_assert(HwInfoTemperatureUnit(kHwInfoRawFahrenheitUnit,
                                    std::size(kHwInfoRawFahrenheitUnit)) == 'F');

std::optional<double> NormalizeHwInfoTemperature(double value,
                                                 const char* unit,
                                                 size_t capacity) {
    // HWiNFO stores a raw single-byte degree sign followed by an ASCII unit letter.
    // Decoding through CP_ACP corrupts that sequence on DBCS locales, so classify the
    // ASCII letter directly from the bytes.
    char unitLetter = HwInfoTemperatureUnit(unit, capacity);
    if (!unitLetter) return std::nullopt;
    return NormalizeTemperature(value, unitLetter == 'F' ? L"F" : L"C");
}

// Power and clock readings need no unit decoding: HWiNFO already reports them in
// watts and MHz, and the reading type (not the unit bytes) is what selects them.
bool IsPlausiblePowerWatts(double value) {
    return std::isfinite(value) && value >= 0.0 && value <= 1500.0;
}

bool IsPlausibleClockMhz(double value) {
    return std::isfinite(value) && value >= 1.0 && value <= 10000.0;
}

struct HwInfoExtras {
    std::optional<double> cpuTemp;
    std::optional<double> gpuTemp;
    std::optional<double> cpuClockMhz;
    std::optional<double> gpuClockMhz;
    std::optional<double> cpuPowerW;
    std::optional<double> gpuPowerW;
    // Set by whichever of ReadHwInfoSharedMemory/ReadHwInfoGadgetRegistry actually
    // supplied each value, so callers report the real source instead of assuming.
    MetricProvider cpuTempProvider = MetricProvider::None;
    MetricProvider gpuTempProvider = MetricProvider::None;
    MetricProvider cpuClockProvider = MetricProvider::None;
    MetricProvider gpuClockProvider = MetricProvider::None;
    MetricProvider cpuPowerProvider = MetricProvider::None;
    MetricProvider gpuPowerProvider = MetricProvider::None;
};

// One pass over every HWiNFO reading, scoring each against whichever of the six wanted
// values is enabled - hence temperature and clock/power sharing one walk rather than
// each keeping its own copy.
// Diagnoses exactly where a shared-memory read comes up empty, logged once per distinct
// reason (not every cycle) so a report from the user actually says something - "HWiNFO
// unavailable" alone doesn't distinguish "not running", "running without Shared Memory
// Support enabled" (off by default in HWiNFO, the most common cause), and "running, SM
// on, but no sensor name matched this hardware".
enum class HwInfoSmDiagnosis {
    Unknown,
    MappingNotFound,
    MutexBusy,
    ViewFailed,
    HeaderInvalid,
    NoMatchingReadings,
    Ok,
};

void LogHwInfoSmDiagnosis(HwInfoSmDiagnosis diagnosis, bool verboseLogging, uint32_t readingCount = 0) {
    static HwInfoSmDiagnosis lastDiagnosis = HwInfoSmDiagnosis::Unknown;
    if (!verboseLogging || diagnosis == lastDiagnosis) return;
    lastDiagnosis = diagnosis;
    switch (diagnosis) {
        case HwInfoSmDiagnosis::MappingNotFound:
            Wh_Log(L"HWiNFO shared memory: mapping not found. HWiNFO must be running with "
                   L"Settings > General > \"Shared Memory Support\" enabled - it is off by "
                   L"default and this is the most common reason nothing shows up.");
            break;
        case HwInfoSmDiagnosis::MutexBusy:
            Wh_Log(L"HWiNFO shared memory: mapping found but the sync mutex would not "
                   L"signal within 50ms; will retry next cycle.");
            break;
        case HwInfoSmDiagnosis::ViewFailed:
            Wh_Log(L"HWiNFO shared memory: mapping found but MapViewOfFile failed: %u",
                   GetLastError());
            break;
        case HwInfoSmDiagnosis::HeaderInvalid:
            Wh_Log(L"HWiNFO shared memory: mapped, but the header signature or sensor/"
                   L"reading table bounds didn't validate - unexpected HWiNFO version?");
            break;
        case HwInfoSmDiagnosis::NoMatchingReadings:
            Wh_Log(L"HWiNFO shared memory: read %u readings successfully, but none scored "
                   L"as a CPU/GPU temperature, clock or power sensor for this hardware. "
                   L"Try the sensor-name filter settings if your hardware uses unusual "
                   L"sensor labels.",
                   readingCount);
            break;
        case HwInfoSmDiagnosis::Ok:
            Wh_Log(L"HWiNFO shared memory: reading successfully.");
            break;
        case HwInfoSmDiagnosis::Unknown:
            break;
    }
}

void ReadHwInfoSharedMemory(HwInfoExtras& out,
                            const ModSettings& settings,
                            bool wantTemperature,
                            bool wantClockPower) {
    HANDLE mapping =
        OpenFileMappingW(FILE_MAP_READ, FALSE, L"Global\\HWiNFO_SENS_SM2");
    if (!mapping) {
        LogHwInfoSmDiagnosis(HwInfoSmDiagnosis::MappingNotFound, settings.verboseLogging);
        return;
    }

    HANDLE mutex =
        OpenMutexW(SYNCHRONIZE | MUTEX_MODIFY_STATE, FALSE, L"Global\\HWiNFO_SM2_MUTEX");
    bool mutexOwned = false;
    if (mutex) {
        DWORD waitResult = WaitForSingleObject(mutex, 50);
        if (waitResult == WAIT_OBJECT_0 || waitResult == WAIT_ABANDONED) {
            mutexOwned = true;
        } else {
            CloseHandle(mutex);
            CloseHandle(mapping);
            LogHwInfoSmDiagnosis(HwInfoSmDiagnosis::MutexBusy, settings.verboseLogging);
            return;
        }
    }

    void* view = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
    if (!view) {
        if (mutexOwned) ReleaseMutex(mutex);
        if (mutex) CloseHandle(mutex);
        CloseHandle(mapping);
        LogHwInfoSmDiagnosis(HwInfoSmDiagnosis::ViewFailed, settings.verboseLogging);
        return;
    }

    HwInfoSmDiagnosis diagnosis = HwInfoSmDiagnosis::HeaderInvalid;
    uint32_t scannedReadings = 0;
    MEMORY_BASIC_INFORMATION memoryInfo{};
    if (VirtualQuery(view, &memoryInfo, sizeof(memoryInfo))) {
        size_t viewOffset = static_cast<const uint8_t*>(view) -
                            static_cast<const uint8_t*>(memoryInfo.BaseAddress);
        size_t mappedSize = viewOffset <= memoryInfo.RegionSize
                                ? memoryInfo.RegionSize - viewOffset
                                : 0;
        if (mappedSize >= sizeof(HwInfoHeader)) {
            HwInfoHeader header{};
            std::memcpy(&header, view, sizeof(header));

            if (header.signature == kHwInfoSignature &&
                IsRangeValid(mappedSize, header.sensorOffset, header.sensorStride,
                            header.sensorCount, sizeof(HwInfoSensorPrefix)) &&
                IsRangeValid(mappedSize, header.readingOffset, header.readingStride,
                            header.readingCount, sizeof(HwInfoReadingPrefix))) {
                scannedReadings = header.readingCount;
                diagnosis = HwInfoSmDiagnosis::NoMatchingReadings;
                int bestCpuTemp = -1, bestGpuTemp = -1;
                int bestCpuClock = -1, bestGpuClock = -1;
                int bestCpuPower = -1, bestGpuPower = -1;
                const auto* bytes = static_cast<const uint8_t*>(view);

                for (uint32_t i = 0; i < header.readingCount; i++) {
                    HwInfoReadingPrefix reading{};
                    const uint8_t* readingAddress =
                        bytes + header.readingOffset +
                        static_cast<size_t>(i) * header.readingStride;
                    std::memcpy(&reading, readingAddress, sizeof(reading));

                    bool isTemp = reading.readingType == kHwInfoReadingTemperature;
                    bool isClock = reading.readingType == kHwInfoReadingClock;
                    bool isPower = reading.readingType == kHwInfoReadingPower;
                    if ((!wantTemperature || !isTemp) &&
                        (!wantClockPower || !(isClock || isPower))) {
                        continue;
                    }
                    if (reading.sensorIndex >= header.sensorCount) continue;

                    HwInfoSensorPrefix sensor{};
                    const uint8_t* sensorAddress =
                        bytes + header.sensorOffset +
                        static_cast<size_t>(reading.sensorIndex) * header.sensorStride;
                    std::memcpy(&sensor, sensorAddress, sizeof(sensor));

                    std::wstring sensorName = FixedAnsiToWide(
                        sensor.originalName, std::size(sensor.originalName));
                    std::wstring label = FixedAnsiToWide(
                        reading.originalLabel, std::size(reading.originalLabel));

                    if (isTemp && wantTemperature) {
                        auto value = NormalizeHwInfoTemperature(
                            reading.value, reading.unit, std::size(reading.unit));
                        if (value) {
                            int cpuScore = CpuTemperatureScore(sensorName, label,
                                                               settings.cpuTempSensor);
                            if (cpuScore > bestCpuTemp) {
                                bestCpuTemp = cpuScore;
                                out.cpuTemp = *value;
                                out.cpuTempProvider = MetricProvider::HwInfoSharedMemory;
                            }
                            int gpuScore = GpuTemperatureScore(sensorName, label,
                                                               settings.gpuTempSensor);
                            if (gpuScore > bestGpuTemp) {
                                bestGpuTemp = gpuScore;
                                out.gpuTemp = *value;
                                out.gpuTempProvider = MetricProvider::HwInfoSharedMemory;
                            }
                        }
                    } else if (isClock && wantClockPower &&
                              IsPlausibleClockMhz(reading.value)) {
                        int cpuScore =
                            CpuClockScore(sensorName, label, settings.cpuClockSensor);
                        if (cpuScore > bestCpuClock) {
                            bestCpuClock = cpuScore;
                            out.cpuClockMhz = reading.value;
                            out.cpuClockProvider = MetricProvider::HwInfoSharedMemory;
                        }
                        int gpuScore =
                            GpuClockScore(sensorName, label, settings.gpuClockSensor);
                        if (gpuScore > bestGpuClock) {
                            bestGpuClock = gpuScore;
                            out.gpuClockMhz = reading.value;
                            out.gpuClockProvider = MetricProvider::HwInfoSharedMemory;
                        }
                    } else if (isPower && wantClockPower &&
                              IsPlausiblePowerWatts(reading.value)) {
                        int cpuScore =
                            CpuPowerScore(sensorName, label, settings.cpuPowerSensor);
                        if (cpuScore > bestCpuPower) {
                            bestCpuPower = cpuScore;
                            out.cpuPowerW = reading.value;
                            out.cpuPowerProvider = MetricProvider::HwInfoSharedMemory;
                        }
                        int gpuScore =
                            GpuPowerScore(sensorName, label, settings.gpuPowerSensor);
                        if (gpuScore > bestGpuPower) {
                            bestGpuPower = gpuScore;
                            out.gpuPowerW = reading.value;
                            out.gpuPowerProvider = MetricProvider::HwInfoSharedMemory;
                        }
                    }
                }
                if (bestCpuTemp >= 0 || bestGpuTemp >= 0 || bestCpuClock >= 0 ||
                    bestGpuClock >= 0 || bestCpuPower >= 0 || bestGpuPower >= 0) {
                    diagnosis = HwInfoSmDiagnosis::Ok;
                }
            }
        }
    }

    UnmapViewOfFile(view);
    if (mutexOwned) ReleaseMutex(mutex);
    if (mutex) CloseHandle(mutex);
    CloseHandle(mapping);
    LogHwInfoSmDiagnosis(diagnosis, settings.verboseLogging, scannedReadings);
}

std::optional<std::wstring> ReadRegistryString(HKEY key, const std::wstring& name) {
    DWORD type = 0;
    DWORD bytes = 0;
    LONG status =
        RegQueryValueExW(key, name.c_str(), nullptr, &type, nullptr, &bytes);
    if (status != ERROR_SUCCESS || (type != REG_SZ && type != REG_EXPAND_SZ) ||
        bytes < sizeof(wchar_t)) {
        return std::nullopt;
    }

    std::vector<wchar_t> buffer(bytes / sizeof(wchar_t) + 1, L'\0');
    status = RegQueryValueExW(key, name.c_str(), nullptr, &type,
                              reinterpret_cast<BYTE*>(buffer.data()), &bytes);
    if (status != ERROR_SUCCESS) return std::nullopt;
    return std::wstring(buffer.data());
}

std::optional<double> ParseLocalizedDouble(std::wstring value) {
    std::replace(value.begin(), value.end(), L',', L'.');
    wchar_t* end = nullptr;
    double result = std::wcstod(value.c_str(), &end);
    if (end == value.c_str() || !std::isfinite(result)) return std::nullopt;
    return result;
}

std::optional<double> NormalizeRegistryTemperature(const std::wstring& rawValue,
                                                    const std::wstring& formattedValue) {
    auto value = ParseLocalizedDouble(rawValue);
    if (!value) return std::nullopt;
    return NormalizeTemperature(*value, formattedValue);
}

// The Gadget Registry has no reading-type byte, only the formatted string HWiNFO's UI
// would have shown - so a plausible unit substring is what tells a clock or power
// reading apart from everything else parked under the same numbered key.
std::optional<double> NormalizeRegistryClock(const std::wstring& rawValue,
                                             const std::wstring& formattedValue) {
    if (!Contains(ToLower(formattedValue), L"mhz")) return std::nullopt;
    auto value = ParseLocalizedDouble(rawValue);
    if (!value || !IsPlausibleClockMhz(*value)) return std::nullopt;
    return value;
}

std::optional<double> NormalizeRegistryPower(const std::wstring& rawValue,
                                             const std::wstring& formattedValue) {
    if (!Contains(ToLower(formattedValue), L"w")) return std::nullopt;
    auto value = ParseLocalizedDouble(rawValue);
    if (!value || !IsPlausiblePowerWatts(*value)) return std::nullopt;
    return value;
}

void ReadHwInfoGadgetRegistry(HwInfoExtras& out,
                              const ModSettings& settings,
                              bool wantTemperature,
                              bool wantClockPower) {
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\HWiNFO64\\VSB", 0,
                      KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) {
        return;
    }

    int bestCpuTemp = out.cpuTemp ? 10000 : -1;
    int bestGpuTemp = out.gpuTemp ? 10000 : -1;
    int bestCpuClock = out.cpuClockMhz ? 10000 : -1;
    int bestGpuClock = out.gpuClockMhz ? 10000 : -1;
    int bestCpuPower = out.cpuPowerW ? 10000 : -1;
    int bestGpuPower = out.gpuPowerW ? 10000 : -1;
    int consecutiveMissing = 0;
    for (int i = 0; i < 1024; i++) {
        std::wstring suffix = std::to_wstring(i);
        auto sensor = ReadRegistryString(key, L"Sensor" + suffix);
        if (!sensor) {
            if (++consecutiveMissing >= 16) break;
            continue;
        }
        consecutiveMissing = 0;
        auto label = ReadRegistryString(key, L"Label" + suffix);
        auto rawValue = ReadRegistryString(key, L"ValueRaw" + suffix);
        auto formattedValue = ReadRegistryString(key, L"Value" + suffix);
        if (!label || !rawValue || !formattedValue) continue;

        if (wantTemperature) {
            if (auto value = NormalizeRegistryTemperature(*rawValue, *formattedValue)) {
                int cpuScore = CpuTemperatureScore(*sensor, *label, settings.cpuTempSensor);
                if (cpuScore > bestCpuTemp) {
                    bestCpuTemp = cpuScore;
                    out.cpuTemp = *value;
                    out.cpuTempProvider = MetricProvider::HwInfoGadgetRegistry;
                }
                int gpuScore = GpuTemperatureScore(*sensor, *label, settings.gpuTempSensor);
                if (gpuScore > bestGpuTemp) {
                    bestGpuTemp = gpuScore;
                    out.gpuTemp = *value;
                    out.gpuTempProvider = MetricProvider::HwInfoGadgetRegistry;
                }
            }
        }
        if (wantClockPower) {
            if (auto clock = NormalizeRegistryClock(*rawValue, *formattedValue)) {
                int cpuScore = CpuClockScore(*sensor, *label, settings.cpuClockSensor);
                if (cpuScore > bestCpuClock) {
                    bestCpuClock = cpuScore;
                    out.cpuClockMhz = *clock;
                    out.cpuClockProvider = MetricProvider::HwInfoGadgetRegistry;
                }
                int gpuScore = GpuClockScore(*sensor, *label, settings.gpuClockSensor);
                if (gpuScore > bestGpuClock) {
                    bestGpuClock = gpuScore;
                    out.gpuClockMhz = *clock;
                    out.gpuClockProvider = MetricProvider::HwInfoGadgetRegistry;
                }
            }
            if (auto power = NormalizeRegistryPower(*rawValue, *formattedValue)) {
                int cpuScore = CpuPowerScore(*sensor, *label, settings.cpuPowerSensor);
                if (cpuScore > bestCpuPower) {
                    bestCpuPower = cpuScore;
                    out.cpuPowerW = *power;
                    out.cpuPowerProvider = MetricProvider::HwInfoGadgetRegistry;
                }
                int gpuScore = GpuPowerScore(*sensor, *label, settings.gpuPowerSensor);
                if (gpuScore > bestGpuPower) {
                    bestGpuPower = gpuScore;
                    out.gpuPowerW = *power;
                    out.gpuPowerProvider = MetricProvider::HwInfoGadgetRegistry;
                }
            }
        }
    }

    RegCloseKey(key);
}

// LibreHardwareMonitor: an alternative to HWiNFO for CPU/GPU temperature, clock and
// power. Its web server (Options > Remote Web Server > Run) serves /data.json as a
// recursive tree of {Text, Value, Children} nodes. Only temperature/clock/power are
// read; usage and RAM/VRAM already come from native PDH counters.
//
// Adapted from taskbar-clock-customization-v3, with one fix: that version uses 0.0 as
// a "not found" sentinel, indistinguishable from a genuine zero reading. This one
// returns std::optional so a real zero can't fall through to the next candidate name.

struct LhmJsonNode {
    std::wstring text;
    std::wstring value;
    std::vector<LhmJsonNode> children;
};

void SkipLhmWs(const std::wstring& text, size_t& pos) {
    while (pos < text.size() && std::iswspace(text[pos])) pos++;
}

std::optional<std::wstring> ParseLhmJsonString(const std::wstring& text, size_t& pos) {
    if (pos >= text.size() || text[pos] != L'"') return std::nullopt;
    pos++;
    std::wstring result;
    while (pos < text.size() && text[pos] != L'"') {
        wchar_t c = text[pos];
        if (c == L'\\' && pos + 1 < text.size()) {
            wchar_t next = text[pos + 1];
            switch (next) {
                case L'"': result += L'"'; break;
                case L'\\': result += L'\\'; break;
                case L'/': result += L'/'; break;
                case L'n': result += L'\n'; break;
                case L't': result += L'\t'; break;
                case L'r': result += L'\r'; break;
                default: result += next; break;
            }
            pos += 2;
        } else {
            result += c;
            pos++;
        }
    }
    if (pos >= text.size()) return std::nullopt;
    pos++;  // closing quote
    return result;
}

// Skips a JSON value of any type (object, array, string, number, literal) without
// interpreting it - used for every key this mod doesn't care about (Min, Max, Type,
// SensorId, ImageURL, ...), of which LHM's data.json has several per node.
void SkipLhmJsonValue(const std::wstring& text, size_t& pos) {
    SkipLhmWs(text, pos);
    if (pos >= text.size()) return;
    wchar_t c = text[pos];
    if (c == L'"') {
        ParseLhmJsonString(text, pos);
    } else if (c == L'{' || c == L'[') {
        wchar_t open = c, close = c == L'{' ? L'}' : L']';
        int depth = 0;
        bool inString = false;
        while (pos < text.size()) {
            wchar_t ch = text[pos];
            if (inString) {
                if (ch == L'\\') {
                    pos += 2;
                    continue;
                }
                if (ch == L'"') inString = false;
            } else {
                if (ch == L'"') inString = true;
                else if (ch == open) depth++;
                else if (ch == close && --depth == 0) {
                    pos++;
                    return;
                }
            }
            pos++;
        }
    } else {
        while (pos < text.size() && text[pos] != L',' && text[pos] != L'}' &&
              text[pos] != L']') {
            pos++;
        }
    }
}

std::optional<LhmJsonNode> ParseLhmJsonNode(const std::wstring& text, size_t& pos) {
    SkipLhmWs(text, pos);
    if (pos >= text.size() || text[pos] != L'{') return std::nullopt;
    pos++;

    LhmJsonNode node;
    while (true) {
        SkipLhmWs(text, pos);
        if (pos >= text.size()) return std::nullopt;
        if (text[pos] == L'}') {
            pos++;
            return node;
        }
        if (text[pos] == L',') {
            pos++;
            continue;
        }
        auto key = ParseLhmJsonString(text, pos);
        if (!key) return std::nullopt;
        SkipLhmWs(text, pos);
        if (pos >= text.size() || text[pos] != L':') return std::nullopt;
        pos++;
        SkipLhmWs(text, pos);

        if (*key == L"Text") {
            auto value = ParseLhmJsonString(text, pos);
            if (value) node.text = *value;
        } else if (*key == L"Value") {
            auto value = ParseLhmJsonString(text, pos);
            if (value) node.value = *value;
        } else if (*key == L"Children" && pos < text.size() && text[pos] == L'[') {
            pos++;
            while (true) {
                SkipLhmWs(text, pos);
                if (pos >= text.size()) return std::nullopt;
                if (text[pos] == L']') {
                    pos++;
                    break;
                }
                if (text[pos] == L',') {
                    pos++;
                    continue;
                }
                auto child = ParseLhmJsonNode(text, pos);
                if (!child) return std::nullopt;
                node.children.push_back(std::move(*child));
            }
        } else {
            SkipLhmJsonValue(text, pos);
        }
    }
}

// The leading numeric portion of an LHM value string ("45.2 °C", "1234.0 MHz") - the
// unit suffix is discarded, matching LHM's own convention of reporting each sensor
// type in one fixed unit (temperatures in °C, clocks in MHz, power in W).
std::optional<double> ParseLhmValue(const std::wstring& value) {
    std::wstring normalized = value;
    std::replace(normalized.begin(), normalized.end(), L',', L'.');
    wchar_t* end = nullptr;
    double result = std::wcstod(normalized.c_str(), &end);
    if (end == normalized.c_str() || !std::isfinite(result)) return std::nullopt;
    return result;
}

// Depth-first search for the first sensor whose group node's Text case-insensitively
// equals hwType (e.g. "Temperatures") and whose own Text contains sensorName as a
// case-sensitive substring - not scoped to any particular hardware device, so it works
// the same regardless of how LHM names the CPU/GPU node for a given build/version.
std::optional<double> FindLhmSensorValue(const LhmJsonNode& node,
                                         const std::wstring& hwType,
                                         const std::wstring& sensorName) {
    if (_wcsicmp(node.text.c_str(), hwType.c_str()) == 0) {
        for (const auto& child : node.children) {
            if (child.text.find(sensorName) != std::wstring::npos) {
                if (auto value = ParseLhmValue(child.value)) return value;
            }
        }
    }
    for (const auto& child : node.children) {
        if (auto value = FindLhmSensorValue(child, hwType, sensorName)) return value;
    }
    return std::nullopt;
}

std::optional<double> FindLhmSensorValueAny(
    const LhmJsonNode& root,
    const std::wstring& hwType,
    std::initializer_list<const wchar_t*> candidates) {
    for (const wchar_t* candidate : candidates) {
        if (auto value = FindLhmSensorValue(root, hwType, candidate)) return value;
    }
    return std::nullopt;
}

// WinINet, matching taskbar-clock-customization-v3's GetUrlContent: a short-lived
// INTERNET_OPEN_TYPE_PRECONFIG session, no cache/cookies/UI, read in fixed chunks into
// a growing buffer, decoded from UTF-8 (LHM's data.json is served as UTF-8 JSON).
std::optional<std::wstring> FetchLhmUrlContent(const std::wstring& url) {
    HINTERNET session =
        InternetOpenW(L"WindhawkTaskbarSystemInfoFork", INTERNET_OPEN_TYPE_PRECONFIG,
                      nullptr, nullptr, 0);
    if (!session) return std::nullopt;

    HINTERNET request = InternetOpenUrlW(
        session, url.c_str(), nullptr, 0,
        INTERNET_FLAG_NO_AUTH | INTERNET_FLAG_NO_CACHE_WRITE |
            INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_NO_UI |
            INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD,
        0);
    if (!request) {
        InternetCloseHandle(session);
        return std::nullopt;
    }

    std::string body;
    char chunk[1024];
    DWORD bytesRead = 0;
    while (InternetReadFile(request, chunk, sizeof(chunk), &bytesRead) && bytesRead > 0) {
        body.append(chunk, bytesRead);
        if (body.size() > 8 * 1024 * 1024) break;  // sanity cap, a real data.json is small
    }
    InternetCloseHandle(request);
    InternetCloseHandle(session);
    if (body.empty()) return std::nullopt;

    int wideLength =
        MultiByteToWideChar(CP_UTF8, 0, body.data(), static_cast<int>(body.size()),
                            nullptr, 0);
    if (wideLength <= 0) return std::nullopt;
    std::wstring wide(wideLength, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, body.data(), static_cast<int>(body.size()),
                        wide.data(), wideLength);
    return wide;
}

struct LhmSnapshot {
    std::optional<double> cpuTemp;
    std::optional<double> gpuTemp;
    std::optional<double> cpuClockMhz;
    std::optional<double> gpuClockMhz;
    std::optional<double> cpuPowerW;
    std::optional<double> gpuPowerW;
};

LhmSnapshot FetchLhmSnapshot(int port) {
    LhmSnapshot snapshot;
    wchar_t url[64];
    swprintf(url, std::size(url), L"http://localhost:%d/data.json", port);

    auto content = FetchLhmUrlContent(url);
    if (!content) return snapshot;

    size_t pos = 0;
    auto root = ParseLhmJsonNode(*content, pos);
    if (!root) return snapshot;

    if (auto value = FindLhmSensorValueAny(
            *root, L"Temperatures",
            {L"CPU Package", L"Core (Tctl/Tdie)", L"Core Average", L"CPU Core"})) {
        if (auto normalized = NormalizeTemperature(*value, L"C")) snapshot.cpuTemp = normalized;
    }
    if (auto value =
            FindLhmSensorValueAny(*root, L"Temperatures", {L"GPU Core", L"GPU Hot Spot"})) {
        if (auto normalized = NormalizeTemperature(*value, L"C")) snapshot.gpuTemp = normalized;
    }
    if (auto value =
            FindLhmSensorValueAny(*root, L"Clocks", {L"CPU Core #1", L"Core #1"})) {
        if (IsPlausibleClockMhz(*value)) snapshot.cpuClockMhz = value;
    }
    if (auto value = FindLhmSensorValueAny(*root, L"Clocks", {L"GPU Core"})) {
        if (IsPlausibleClockMhz(*value)) snapshot.gpuClockMhz = value;
    }
    if (auto value = FindLhmSensorValueAny(*root, L"Powers",
                                           {L"CPU Package", L"Package", L"CPU Cores"})) {
        if (IsPlausiblePowerWatts(*value)) snapshot.cpuPowerW = value;
    }
    if (auto value = FindLhmSensorValueAny(*root, L"Powers", {L"GPU", L"GPU Power"})) {
        if (IsPlausiblePowerWatts(*value)) snapshot.gpuPowerW = value;
    }
    return snapshot;
}

std::mutex g_lhmMutex;
LhmSnapshot g_lhmSnapshot;
bool g_lhmDataLoaded = false;

std::atomic<bool> g_stopLhmWorker{false};
HANDLE g_lhmWorkerWakeEvent = nullptr;
[[clang::no_destroy]] std::optional<std::thread> g_lhmWorker;

void LhmWorkerProc() {
    while (!g_stopLhmWorker) {
        ModSettings settings = CurrentSettings();
        if (settings.lhmEnabled) {
            LhmSnapshot snapshot = FetchLhmSnapshot(settings.lhmPort);
            std::lock_guard lock(g_lhmMutex);
            g_lhmSnapshot = snapshot;
            g_lhmDataLoaded = true;
        }
        DWORD waitMs =
            static_cast<DWORD>(std::max(1, settings.lhmUpdateInterval)) * 1000;
        if (WaitForSingleObject(g_lhmWorkerWakeEvent, waitMs) == WAIT_FAILED) break;
    }
}

bool StartLhmWorker() {
    if (g_lhmWorker) return true;
    if (g_unloading) return false;
    g_lhmWorkerWakeEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!g_lhmWorkerWakeEvent) return false;
    g_stopLhmWorker = false;
    try {
        g_lhmWorker.emplace(LhmWorkerProc);
    } catch (...) {
        CloseHandle(g_lhmWorkerWakeEvent);
        g_lhmWorkerWakeEvent = nullptr;
        return false;
    }
    return true;
}

void StopLhmWorker() {
    g_stopLhmWorker = true;
    if (g_lhmWorkerWakeEvent) SetEvent(g_lhmWorkerWakeEvent);
    if (g_lhmWorker) {
        if (g_lhmWorker->joinable()) g_lhmWorker->join();
        g_lhmWorker.reset();
    }
    if (g_lhmWorkerWakeEvent) {
        CloseHandle(g_lhmWorkerWakeEvent);
        g_lhmWorkerWakeEvent = nullptr;
    }
    std::lock_guard lock(g_lhmMutex);
    g_lhmSnapshot = {};
    g_lhmDataLoaded = false;
}

// LHM only ever fills fields that are still empty, never overwrites. Split in two
// because temperature and clock/power have separate source-mode settings: each half is
// called before its Read function when that mode is Auto (making LHM primary - it has
// no HWiNFO free-tier 12-hour cutoff), and again after for every other mode, where the
// explicitly chosen source must stay authoritative.
bool GetLhmSnapshotIfEnabled(const ModSettings& settings, LhmSnapshot& out) {
    if (!settings.lhmEnabled) return false;
    std::lock_guard lock(g_lhmMutex);
    if (!g_lhmDataLoaded) return false;
    out = g_lhmSnapshot;
    return true;
}

void FillTempFromLhm(MetricsSnapshot& snapshot, const ModSettings& settings) {
    LhmSnapshot lhm;
    if (!GetLhmSnapshotIfEnabled(settings, lhm)) return;
    if (!snapshot.cpuTemp && lhm.cpuTemp) {
        snapshot.cpuTemp = lhm.cpuTemp;
        snapshot.cpuTempProvider = MetricProvider::LibreHardwareMonitor;
    }
    if (!snapshot.gpuTemp && lhm.gpuTemp) {
        snapshot.gpuTemp = lhm.gpuTemp;
        snapshot.gpuTempProvider = MetricProvider::LibreHardwareMonitor;
    }
}

void FillExtrasFromLhm(MetricsSnapshot& snapshot, const ModSettings& settings) {
    LhmSnapshot lhm;
    if (!GetLhmSnapshotIfEnabled(settings, lhm)) return;
    if (!snapshot.cpuClockMhz && lhm.cpuClockMhz) {
        snapshot.cpuClockMhz = lhm.cpuClockMhz;
        snapshot.cpuClockProvider = MetricProvider::LibreHardwareMonitor;
    }
    if (!snapshot.gpuClockMhz && lhm.gpuClockMhz) {
        snapshot.gpuClockMhz = lhm.gpuClockMhz;
        snapshot.gpuClockProvider = MetricProvider::LibreHardwareMonitor;
    }
    if (!snapshot.cpuPowerW && lhm.cpuPowerW) {
        snapshot.cpuPowerW = lhm.cpuPowerW;
        snapshot.cpuPowerProvider = MetricProvider::LibreHardwareMonitor;
    }
    if (!snapshot.gpuPowerW && lhm.gpuPowerW) {
        snapshot.gpuPowerW = lhm.gpuPowerW;
        snapshot.gpuPowerProvider = MetricProvider::LibreHardwareMonitor;
    }
}

constexpr double kGiB = 1024.0 * 1024.0 * 1024.0;

// D3DKMT exposes display-adapter performance data, including temperature and power,
// without requiring a third-party monitoring application. Declarations are kept local
// so the mod can build in Windhawk environments without d3dkmthk.h.
using D3DKMT_HANDLE = UINT32;

struct D3DKMT_OPENADAPTERFROMLUID {
    LUID AdapterLuid;
    D3DKMT_HANDLE hAdapter;
};

struct D3DKMT_CLOSEADAPTER {
    D3DKMT_HANDLE hAdapter;
};

struct D3DKMT_QUERYADAPTERINFO {
    D3DKMT_HANDLE hAdapter;
    UINT Type;
    void* pPrivateDriverData;
    UINT PrivateDriverDataSize;
};

struct D3DKMT_ADAPTER_PERFDATA {
    UINT PhysicalAdapterIndex;
    ULONGLONG MemoryFrequency;
    ULONGLONG MaxMemoryFrequency;
    ULONGLONG MaxMemoryFrequencyOC;
    ULONGLONG MemoryBandwidth;
    ULONGLONG PCIEBandwidth;
    ULONG FanRPM;
    ULONG Power;
    ULONG Temperature;
    UCHAR PowerStateOverride;
};

struct D3DKMT_ADAPTERINFO {
    D3DKMT_HANDLE hAdapter;
    LUID AdapterLuid;
    ULONG NumOfSources;
    BOOL bPresentMoveRegionsPreferred;
};

struct D3DKMT_ENUMADAPTERS2 {
    ULONG NumAdapters;
    D3DKMT_ADAPTERINFO* pAdapters;
};

struct D3DKMT_ADAPTERREGISTRYINFO {
    WCHAR AdapterString[MAX_PATH];
    WCHAR BiosString[MAX_PATH];
    WCHAR DacType[MAX_PATH];
    WCHAR ChipType[MAX_PATH];
};

struct D3DKMT_SEGMENTSIZEINFO {
    ULONGLONG DedicatedVideoMemorySize;
    ULONGLONG DedicatedSystemMemorySize;
    ULONGLONG SharedSystemMemorySize;
};

constexpr UINT kAdapterRegistryInfoQueryType = 8;
constexpr UINT kAdapterSegmentSizeQueryType = 3;
constexpr UINT kAdapterPerfDataQueryType = 62;  // KMTQAITYPE_ADAPTERPERFDATA
constexpr ULONG kMaxD3dkmtAdapters = 16;

using D3DKMTEnumAdapters2_t = LONG(WINAPI*)(D3DKMT_ENUMADAPTERS2*);
using D3DKMTOpenAdapterFromLuid_t = LONG(WINAPI*)(D3DKMT_OPENADAPTERFROMLUID*);
using D3DKMTQueryAdapterInfo_t = LONG(WINAPI*)(D3DKMT_QUERYADAPTERINFO*);
using D3DKMTCloseAdapter_t = LONG(WINAPI*)(const D3DKMT_CLOSEADAPTER*);

D3DKMTEnumAdapters2_t g_d3dkmtEnumAdapters2 = nullptr;
D3DKMTOpenAdapterFromLuid_t g_d3dkmtOpenAdapterFromLuid = nullptr;
D3DKMTQueryAdapterInfo_t g_d3dkmtQueryAdapterInfo = nullptr;
D3DKMTCloseAdapter_t g_d3dkmtCloseAdapter = nullptr;

struct GpuAdapterInfo {
    std::wstring description;
    std::wstring luid;
    LUID luidValue{};
    uint64_t dedicatedVideoMemory = 0;
    uint64_t sharedSystemMemory = 0;
};

std::optional<std::wstring> g_cachedGpuAdapterFilter;
std::optional<GpuAdapterInfo> g_cachedGpuAdapterInfo;
bool g_cachedGpuAdapterResolved = false;

void InvalidateGpuAdapterCache() {
    g_cachedGpuAdapterFilter.reset();
    g_cachedGpuAdapterInfo.reset();
    g_cachedGpuAdapterResolved = false;
    g_nextGpuIdentityCheck = {};
}

std::wstring FormatAdapterLuid(const LUID& luid) {
    wchar_t buffer[32];
    swprintf(buffer, std::size(buffer), L"0x%08X_0x%08X",
             static_cast<DWORD>(luid.HighPart), luid.LowPart);
    return ToLower(buffer);
}

std::optional<GpuAdapterInfo> GetLiveD3dkmtAdapterInfo(const std::wstring& filterLower) {
    if (!g_d3dkmtEnumAdapters2 || !g_d3dkmtQueryAdapterInfo || !g_d3dkmtCloseAdapter) {
        return std::nullopt;
    }

    D3DKMT_ADAPTERINFO adapters[kMaxD3dkmtAdapters]{};
    D3DKMT_ENUMADAPTERS2 enumeration{};
    enumeration.NumAdapters = std::size(adapters);
    enumeration.pAdapters = adapters;
    if (g_d3dkmtEnumAdapters2(&enumeration) != 0) return std::nullopt;

    std::optional<GpuAdapterInfo> selected;
    ULONG adapterCount = std::min<ULONG>(enumeration.NumAdapters, std::size(adapters));
    for (ULONG index = 0; index < adapterCount; index++) {
        const auto& adapter = adapters[index];

        D3DKMT_ADAPTERREGISTRYINFO registryInfo{};
        D3DKMT_QUERYADAPTERINFO registryQuery{};
        registryQuery.hAdapter = adapter.hAdapter;
        registryQuery.Type = kAdapterRegistryInfoQueryType;
        registryQuery.pPrivateDriverData = &registryInfo;
        registryQuery.PrivateDriverDataSize = sizeof(registryInfo);
        bool registryAvailable = g_d3dkmtQueryAdapterInfo(&registryQuery) == 0;

        D3DKMT_SEGMENTSIZEINFO segmentInfo{};
        D3DKMT_QUERYADAPTERINFO segmentQuery{};
        segmentQuery.hAdapter = adapter.hAdapter;
        segmentQuery.Type = kAdapterSegmentSizeQueryType;
        segmentQuery.pPrivateDriverData = &segmentInfo;
        segmentQuery.PrivateDriverDataSize = sizeof(segmentInfo);
        bool segmentsAvailable = g_d3dkmtQueryAdapterInfo(&segmentQuery) == 0;

        std::wstring description =
            registryAvailable
                ? FixedWideToString(registryInfo.AdapterString,
                                    std::size(registryInfo.AdapterString))
                : L"";
        GpuAdapterInfo candidate{
            description, FormatAdapterLuid(adapter.AdapterLuid), adapter.AdapterLuid,
            segmentsAvailable ? segmentInfo.DedicatedVideoMemorySize : 0,
            segmentsAvailable ? segmentInfo.SharedSystemMemorySize : 0,
        };

        bool matchesFilter = filterLower.empty() ||
                             Contains(ToLower(candidate.description), filterLower);
        bool betterCandidate =
            !selected ||
            candidate.dedicatedVideoMemory > selected->dedicatedVideoMemory ||
            (candidate.dedicatedVideoMemory == selected->dedicatedVideoMemory &&
             !candidate.description.empty() && selected->description.empty()) ||
            (candidate.dedicatedVideoMemory == selected->dedicatedVideoMemory &&
             candidate.description.empty() == selected->description.empty() &&
             candidate.sharedSystemMemory > selected->sharedSystemMemory);
        if (matchesFilter && betterCandidate) selected = std::move(candidate);
    }

    for (ULONG index = 0; index < adapterCount; index++) {
        if (adapters[index].hAdapter) {
            D3DKMT_CLOSEADAPTER closeAdapter{adapters[index].hAdapter};
            g_d3dkmtCloseAdapter(&closeAdapter);
        }
    }

    // Stale D3DKMT duplicates can retain the full memory sizes while losing their
    // registry identity. Prefer the DXGI compatibility path over an ambiguous adapter.
    return selected && !selected->description.empty() ? selected : std::nullopt;
}

std::optional<GpuAdapterInfo> GetDxgiAdapterInfo(const std::wstring& filterLower) {
    com_ptr<IDXGIFactory> factory;
    if (FAILED(CreateDXGIFactory(IID_PPV_ARGS(factory.put())))) return std::nullopt;

    DXGI_ADAPTER_DESC selected{};
    bool found = false;
    for (UINT index = 0;; index++) {
        com_ptr<IDXGIAdapter> adapter;
        HRESULT result = factory->EnumAdapters(index, adapter.put());
        if (result == DXGI_ERROR_NOT_FOUND) break;
        if (FAILED(result)) continue;

        DXGI_ADAPTER_DESC description{};
        if (FAILED(adapter->GetDesc(&description))) continue;

        if (!filterLower.empty()) {
            if (Contains(ToLower(description.Description), filterLower)) {
                selected = description;
                found = true;
                break;
            }
        } else if (!found ||
                  description.DedicatedVideoMemory > selected.DedicatedVideoMemory) {
            selected = description;
            found = true;
        }
    }

    if (!found) return std::nullopt;
    return GpuAdapterInfo{selected.Description, FormatAdapterLuid(selected.AdapterLuid),
                          selected.AdapterLuid, selected.DedicatedVideoMemory,
                          selected.SharedSystemMemory};
}

std::optional<GpuAdapterInfo> ResolveCurrentGpuAdapterInfo(const std::wstring& filterLower,
                                                           PCWSTR* provider = nullptr) {
    auto adapter = GetLiveD3dkmtAdapterInfo(filterLower);
    PCWSTR resolvedProvider = L"D3DKMT";
    if (!adapter) {
        adapter = GetDxgiAdapterInfo(filterLower);
        resolvedProvider = L"DXGI fallback";
    }
    if (provider) *provider = resolvedProvider;
    return adapter;
}

std::optional<GpuAdapterInfo> GetGpuAdapterInfo(const std::wstring& adapterFilter) {
    std::wstring filterLower = ToLower(adapterFilter);
    if (g_cachedGpuAdapterResolved && g_cachedGpuAdapterFilter == filterLower) {
        return g_cachedGpuAdapterInfo;
    }

    g_cachedGpuAdapterFilter = filterLower;
    PCWSTR provider = nullptr;
    g_cachedGpuAdapterInfo = ResolveCurrentGpuAdapterInfo(filterLower, &provider);
    g_cachedGpuAdapterResolved = true;

    if (!g_cachedGpuAdapterInfo) {
        if (filterLower.empty()) {
            Wh_Log(L"No GPU adapter found");
        } else {
            Wh_Log(L"No GPU adapter matched: %s", filterLower.c_str());
        }
        return std::nullopt;
    }

    Wh_Log(L"Selected GPU (%s): %s, LUID %s, dedicated %.1f GiB, shared %.1f GiB",
           provider, g_cachedGpuAdapterInfo->description.c_str(),
           g_cachedGpuAdapterInfo->luid.c_str(),
           static_cast<double>(g_cachedGpuAdapterInfo->dedicatedVideoMemory) / kGiB,
           static_cast<double>(g_cachedGpuAdapterInfo->sharedSystemMemory) / kGiB);
    return g_cachedGpuAdapterInfo;
}

bool HasGpuAdapterIdentityChanged(const GpuAdapterInfo& cachedAdapter,
                                  const std::wstring& adapterFilter) {
    auto now = std::chrono::steady_clock::now();
    if (now < g_nextGpuIdentityCheck) return false;
    g_nextGpuIdentityCheck = now + std::chrono::seconds(5);

    auto currentAdapter = ResolveCurrentGpuAdapterInfo(ToLower(adapterFilter));
    return currentAdapter && currentAdapter->luid != cachedAdapter.luid;
}

// Temperature only. D3DKMT_ADAPTER_PERFDATA is a reverse-engineered layout: the
// tenths-of-a-degree Temperature field is corroborated by multiple public tools, but
// the Power field's unit has no authoritative source, and a number labelled "W" that
// might not be watts is worse than no number.
struct WindowsGpuPerfData {
    std::optional<double> temperature;
};

WindowsGpuPerfData ReadWindowsGpuPerfData(const ModSettings& settings) {
    WindowsGpuPerfData result;
    if (!g_d3dkmtOpenAdapterFromLuid || !g_d3dkmtQueryAdapterInfo ||
        !g_d3dkmtCloseAdapter) {
        return result;
    }

    auto adapter = GetGpuAdapterInfo(settings.gpuAdapter);
    if (!adapter) return result;

    D3DKMT_OPENADAPTERFROMLUID openAdapter{};
    openAdapter.AdapterLuid = adapter->luidValue;
    if (g_d3dkmtOpenAdapterFromLuid(&openAdapter) != 0) return result;

    D3DKMT_ADAPTER_PERFDATA perfData{};
    D3DKMT_QUERYADAPTERINFO queryInfo{};
    queryInfo.hAdapter = openAdapter.hAdapter;
    queryInfo.Type = kAdapterPerfDataQueryType;
    queryInfo.pPrivateDriverData = &perfData;
    queryInfo.PrivateDriverDataSize = sizeof(perfData);
    LONG status = g_d3dkmtQueryAdapterInfo(&queryInfo);

    D3DKMT_CLOSEADAPTER closeAdapter{openAdapter.hAdapter};
    g_d3dkmtCloseAdapter(&closeAdapter);
    if (status != 0) return result;

    // The driver reports tenths of a degree Celsius; zero means unavailable.
    if (perfData.Temperature != 0 && perfData.Temperature <= 2000) {
        result.temperature = perfData.Temperature / 10.0;
    }
    return result;
}

// PROCESSOR_POWER_INFORMATION and CallNtPowerInformation's ProcessorInformation level
// are declared locally, the same way the D3DKMT structs above are: a stable,
// documented layout that Windhawk's mingw <powrprof.h> does not reliably expose.
struct LocalProcessorPowerInformation {
    ULONG Number;
    ULONG MaxMhz;
    ULONG CurrentMhz;
    ULONG MhzLimit;
    ULONG MaxIdleState;
    ULONG CurrentIdleState;
};
constexpr int kProcessorInformationLevel = 11;  // POWER_INFORMATION_LEVEL::ProcessorInformation

extern "C" LONG WINAPI CallNtPowerInformation(int InformationLevel,
                                              PVOID InputBuffer,
                                              ULONG InputBufferLength,
                                              PVOID OutputBuffer,
                                              ULONG OutputBufferLength);

// NtPowerInformation's per-core CurrentMhz needs no third-party monitor and no admin
// rights; it is what Task Manager's own CPU clock reading is built on.
std::optional<double> ReadNativeCpuClockMhz() {
    SYSTEM_INFO sysInfo{};
    GetSystemInfo(&sysInfo);
    DWORD processorCount = std::max<DWORD>(1, sysInfo.dwNumberOfProcessors);
    processorCount = std::min<DWORD>(processorCount, 256);

    std::vector<LocalProcessorPowerInformation> info(processorCount);
    LONG status = CallNtPowerInformation(
        kProcessorInformationLevel, nullptr, 0, info.data(),
        static_cast<ULONG>(info.size() * sizeof(LocalProcessorPowerInformation)));
    if (status != 0) return std::nullopt;

    double sum = 0.0;
    size_t count = 0;
    for (const auto& core : info) {
        if (core.CurrentMhz > 0 && core.CurrentMhz < 20000) {
            sum += core.CurrentMhz;
            count++;
        }
    }
    if (!count) return std::nullopt;
    return sum / static_cast<double>(count);
}

bool MatchesGpuAdapter(const std::wstring& instance,
                       const std::optional<GpuAdapterInfo>& adapter) {
    return !adapter || Contains(ToLower(instance), adapter->luid);
}

void CloseMetricSources();

constexpr auto kPdhCounterRetryInterval = std::chrono::seconds(30);
constexpr auto kPdhRecoveryRetryDelay = std::chrono::seconds(1);
constexpr auto kPdhRecoveryCooldown = std::chrono::seconds(30);
constexpr uint32_t kPdhReadFailureThreshold = 3;

void ClosePdhQuery() {
    if (g_pdhQuery) {
        PdhCloseQuery(g_pdhQuery);
        g_pdhQuery = nullptr;
    }
    g_gpuCounter = nullptr;
    g_vramCounter = nullptr;
    g_sharedVramCounter = nullptr;
    g_thermalZoneCounter = nullptr;
    g_netRecvCounter = nullptr;
    g_netSentCounter = nullptr;
}

void RecreatePdhSources(PCWSTR reason,
                        PDH_STATUS status,
                        std::chrono::steady_clock::time_point now) {
    if (status == ERROR_SUCCESS) {
        Wh_Log(L"Recreating performance counters after %s", reason);
    } else {
        Wh_Log(L"Recreating performance counters after %s: %08X", reason, status);
    }

    ClosePdhQuery();
    InvalidateGpuAdapterCache();
    g_consecutivePdhReadFailures = 0;
    g_nextPdhCounterRetry = now + kPdhRecoveryRetryDelay;
    g_nextPdhRecovery = now + kPdhRecoveryCooldown;
}

void RecordPdhReadFailure(PCWSTR reason, PDH_STATUS status = ERROR_SUCCESS) {
    g_consecutivePdhReadFailures =
        std::min(g_consecutivePdhReadFailures + 1, kPdhReadFailureThreshold);

    auto now = std::chrono::steady_clock::now();
    if (g_consecutivePdhReadFailures < kPdhReadFailureThreshold || now < g_nextPdhRecovery) {
        return;
    }
    RecreatePdhSources(reason, status, now);
}

void RecoverFromGpuAdapterIdentityChange() {
    auto now = std::chrono::steady_clock::now();
    if (now < g_nextPdhRecovery) return;
    RecreatePdhSources(L"confirmed adapter LUID change", ERROR_SUCCESS, now);
}

void RecordPdhReadSuccess() {
    g_consecutivePdhReadFailures = 0;
}

bool AddPdhCounter(PDH_HCOUNTER& counter, PCWSTR path, PCWSTR description) {
    if (counter) return false;

    PDH_HCOUNTER newCounter = nullptr;
    PDH_STATUS status = PdhAddEnglishCounterW(g_pdhQuery, path, 0, &newCounter);
    if (status != ERROR_SUCCESS) {
        Wh_Log(L"Adding the %s counter failed: %08X", description, status);
        return false;
    }
    counter = newCounter;
    return true;
}

bool NeedsWindowsThermalZones(const ModSettings& settings) {
    return settings.temperatureSource == TemperatureSource::Auto ||
           settings.temperatureSource == TemperatureSource::WindowsNative;
}

void EnsurePdhQuery(const ModSettings& settings) {
    auto now = std::chrono::steady_clock::now();
    bool thermalZonesRequired = NeedsWindowsThermalZones(settings);
    if (!thermalZonesRequired && g_thermalZoneCounter) {
        PdhRemoveCounter(g_thermalZoneCounter);
        g_thermalZoneCounter = nullptr;
    }
    // Same idiom as the thermal-zone and network counters below: drop what nothing on
    // screen needs. The GPU Engine counter is a wildcard over every engine instance on
    // the machine, so leaving it registered is most of what "GPU row off" should stop.
    bool gpuRequired = WantGpuUsage(settings);
    bool vramRequired = VramRowVisible(settings);
    if (!gpuRequired && g_gpuCounter) {
        PdhRemoveCounter(g_gpuCounter);
        g_gpuCounter = nullptr;
    }
    if (!vramRequired && (g_vramCounter || g_sharedVramCounter)) {
        if (g_vramCounter) PdhRemoveCounter(g_vramCounter);
        if (g_sharedVramCounter) PdhRemoveCounter(g_sharedVramCounter);
        g_vramCounter = nullptr;
        g_sharedVramCounter = nullptr;
    }
    bool networkRequired = settings.showNetwork;
    if (!networkRequired && (g_netRecvCounter || g_netSentCounter)) {
        if (g_netRecvCounter) PdhRemoveCounter(g_netRecvCounter);
        if (g_netSentCounter) PdhRemoveCounter(g_netSentCounter);
        g_netRecvCounter = nullptr;
        g_netSentCounter = nullptr;
    }

    bool queryCreated = false;
    if (!g_pdhQuery) {
        if (now < g_nextPdhCounterRetry) return;
        if (PdhOpenQueryW(nullptr, 0, &g_pdhQuery) != ERROR_SUCCESS) {
            g_pdhQuery = nullptr;
            g_nextPdhCounterRetry = now + kPdhCounterRetryInterval;
            return;
        }
        queryCreated = true;
    } else if ((!gpuRequired || g_gpuCounter) &&
              (!vramRequired || (g_vramCounter && g_sharedVramCounter)) &&
              (!thermalZonesRequired || g_thermalZoneCounter) &&
              (!networkRequired || (g_netRecvCounter && g_netSentCounter))) {
        return;
    }

    if (!queryCreated && now < g_nextPdhCounterRetry) return;

    bool counterAdded = false;
    if (gpuRequired) {
        counterAdded |= AddPdhCounter(g_gpuCounter,
                                      L"\\GPU Engine(*)\\Utilization Percentage",
                                      L"GPU usage");
    }
    if (vramRequired) {
        counterAdded |= AddPdhCounter(
            g_vramCounter, L"\\GPU Adapter Memory(*)\\Dedicated Usage", L"VRAM usage");
        counterAdded |= AddPdhCounter(g_sharedVramCounter,
                                      L"\\GPU Adapter Memory(*)\\Shared Usage",
                                      L"shared GPU-memory usage");
    }
    if (thermalZonesRequired) {
        counterAdded |= AddPdhCounter(g_thermalZoneCounter,
                                      L"\\Thermal Zone Information(*)\\Temperature",
                                      L"Windows thermal-zone");
    }
    if (networkRequired) {
        counterAdded |= AddPdhCounter(g_netRecvCounter,
                                      L"\\Network Interface(*)\\Bytes Received/sec",
                                      L"network receive");
        counterAdded |= AddPdhCounter(g_netSentCounter,
                                      L"\\Network Interface(*)\\Bytes Sent/sec",
                                      L"network send");
    }

    bool haveAnyCounter = g_gpuCounter || g_vramCounter || g_sharedVramCounter ||
                         (thermalZonesRequired && g_thermalZoneCounter) ||
                         (networkRequired && (g_netRecvCounter || g_netSentCounter));
    if (!haveAnyCounter) {
        PdhCloseQuery(g_pdhQuery);
        g_pdhQuery = nullptr;
        g_nextPdhCounterRetry = now + kPdhCounterRetryInterval;
        return;
    }

    bool missingSome = !g_gpuCounter || !g_vramCounter || !g_sharedVramCounter ||
                       (thermalZonesRequired && !g_thermalZoneCounter) ||
                       (networkRequired && (!g_netRecvCounter || !g_netSentCounter));
    if (missingSome) g_nextPdhCounterRetry = now + kPdhCounterRetryInterval;

    if (queryCreated || counterAdded) {
        PDH_STATUS collectStatus = PdhCollectQueryData(g_pdhQuery);
        if (collectStatus != ERROR_SUCCESS) {
            Wh_Log(L"Initial metric counter collection failed: %08X", collectStatus);
        }
    }
}

PDH_STATUS ReadPdhArray(PDH_HCOUNTER counter,
                        std::vector<uint8_t>& buffer,
                        DWORD& itemCount) {
    if (!counter) return PDH_CSTATUS_NO_COUNTER;
    DWORD bufferSize = 0;
    PDH_STATUS status = PdhGetFormattedCounterArrayW(counter, PDH_FMT_DOUBLE,
                                                     &bufferSize, &itemCount, nullptr);
    if (status == ERROR_SUCCESS && !bufferSize) {
        buffer.clear();
        itemCount = 0;
        return ERROR_SUCCESS;
    }
    if (status != static_cast<PDH_STATUS>(PDH_MORE_DATA) || !bufferSize) {
        // PdhGetFormattedCounterArrayW can write a stray itemCount even on a status
        // that isn't PDH_MORE_DATA; every caller already checks the returned status
        // before touching itemCount, but leaving it non-zero here would make that a
        // requirement future callers have to remember rather than something this
        // function guarantees on its own.
        itemCount = 0;
        return status;
    }

    buffer.resize(bufferSize);
    auto* items = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_W*>(buffer.data());
    return PdhGetFormattedCounterArrayW(counter, PDH_FMT_DOUBLE, &bufferSize, &itemCount,
                                        items);
}

bool IsHardPdhArrayFailure(PDH_STATUS status) {
    return status != ERROR_SUCCESS && status != static_cast<PDH_STATUS>(PDH_NO_DATA) &&
           status != static_cast<PDH_STATUS>(PDH_CSTATUS_NO_INSTANCE);
}

void ReadWindowsThermalZones(MetricsSnapshot& snapshot, const ModSettings& settings) {
    if (snapshot.cpuTemp || !g_thermalZoneCounter) return;

    std::vector<uint8_t> buffer;
    DWORD itemCount = 0;
    if (ReadPdhArray(g_thermalZoneCounter, buffer, itemCount) != ERROR_SUCCESS) return;

    std::wstring filter = ToLower(settings.windowsThermalZoneFilter);
    auto* items = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_W*>(buffer.data());
    double aggregate = 0.0;
    size_t validCount = 0;

    for (DWORD i = 0; i < itemCount; i++) {
        const auto& value = items[i].FmtValue;
        if (value.CStatus != PDH_CSTATUS_VALID_DATA &&
            value.CStatus != PDH_CSTATUS_NEW_DATA) {
            continue;
        }
        std::wstring instance = items[i].szName ? ToLower(items[i].szName) : L"";
        if (!filter.empty() && !Contains(instance, filter)) continue;

        // Reported in Kelvin; reject dead zones below 200 K and values above the
        // 200 °C ceiling as corrupt data.
        double kelvin = value.doubleValue;
        if (!std::isfinite(kelvin) || kelvin < 200.0 || kelvin > 473.15) continue;

        double celsius = kelvin - 273.15;
        if (settings.windowsThermalZoneAggregation == ThermalZoneAggregation::Hottest) {
            aggregate = validCount ? std::max(aggregate, celsius) : celsius;
        } else {
            aggregate += celsius;
        }
        validCount++;
    }

    if (!validCount) return;
    if (settings.windowsThermalZoneAggregation == ThermalZoneAggregation::Average) {
        aggregate /= validCount;
    }
    snapshot.cpuTemp = aggregate;
    snapshot.cpuTempProvider = MetricProvider::WindowsThermalZones;
}

std::optional<double> ReadGpuUsage(const std::optional<GpuAdapterInfo>& adapter,
                                   PDH_STATUS& readStatus) {
    std::vector<uint8_t> buffer;
    DWORD itemCount = 0;
    readStatus = ReadPdhArray(g_gpuCounter, buffer, itemCount);
    if (readStatus == static_cast<PDH_STATUS>(PDH_NO_DATA) ||
        readStatus == static_cast<PDH_STATUS>(PDH_CSTATUS_NO_INSTANCE)) {
        return 0.0;
    }
    if (readStatus != ERROR_SUCCESS) return std::nullopt;

    auto* items = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_W*>(buffer.data());
    std::unordered_map<std::wstring, double> engineTotals;
    bool found = false;
    for (DWORD i = 0; i < itemCount; i++) {
        const auto& value = items[i].FmtValue;
        if ((value.CStatus != PDH_CSTATUS_VALID_DATA &&
            value.CStatus != PDH_CSTATUS_NEW_DATA) ||
            !std::isfinite(value.doubleValue) || value.doubleValue < 0.0) {
            continue;
        }
        std::wstring instance = items[i].szName ? items[i].szName : L"";
        if (!MatchesGpuAdapter(instance, adapter)) continue;
        size_t luidPosition = instance.find(L"luid_");
        std::wstring engineKey =
            luidPosition == std::wstring::npos ? instance : instance.substr(luidPosition);
        engineTotals[engineKey] += value.doubleValue;
        found = true;
    }

    double busiestEngine = 0.0;
    for (const auto& [engine, usage] : engineTotals) {
        busiestEngine = std::max(busiestEngine, usage);
    }
    return found ? std::clamp(busiestEngine, 0.0, 100.0) : 0.0;
}

std::optional<double> ReadVramUsedBytes(PDH_HCOUNTER counter,
                                        const std::optional<GpuAdapterInfo>& adapter,
                                        bool& anyValidSample,
                                        PDH_STATUS& readStatus) {
    std::vector<uint8_t> buffer;
    DWORD itemCount = 0;
    readStatus = ReadPdhArray(counter, buffer, itemCount);
    if (readStatus != ERROR_SUCCESS) return std::nullopt;

    auto* items = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_W*>(buffer.data());
    double total = 0.0;
    bool found = false;
    for (DWORD i = 0; i < itemCount; i++) {
        const auto& value = items[i].FmtValue;
        if ((value.CStatus != PDH_CSTATUS_VALID_DATA &&
            value.CStatus != PDH_CSTATUS_NEW_DATA) ||
            !std::isfinite(value.doubleValue) || value.doubleValue < 0.0) {
            continue;
        }
        anyValidSample = true;
        std::wstring instance = items[i].szName ? items[i].szName : L"";
        if (!MatchesGpuAdapter(instance, adapter)) continue;
        total += value.doubleValue;
        found = true;
    }
    return found ? std::optional<double>(total) : std::nullopt;
}

// Network interface instances aren't tied to a GPU adapter, so every valid instance is
// summed - the adapter-name filter (if any) narrows that sum instead of picking one.
// nullopt distinguishes "the read itself failed" from "read fine, zero traffic" - both
// looked like a plain 0.0 before, which showed "0 B/s" instead of "--" on a PDH failure.
std::optional<double> ReadNetworkCounterTotal(PDH_HCOUNTER counter,
                                              const std::wstring& filterLower) {
    std::vector<uint8_t> buffer;
    DWORD itemCount = 0;
    if (ReadPdhArray(counter, buffer, itemCount) != ERROR_SUCCESS) return std::nullopt;

    auto* items = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_W*>(buffer.data());
    double total = 0.0;
    for (DWORD i = 0; i < itemCount; i++) {
        const auto& value = items[i].FmtValue;
        if ((value.CStatus != PDH_CSTATUS_VALID_DATA &&
            value.CStatus != PDH_CSTATUS_NEW_DATA) ||
            !std::isfinite(value.doubleValue) || value.doubleValue < 0.0) {
            continue;
        }
        std::wstring instance = items[i].szName ? ToLower(items[i].szName) : L"";
        // The PDH "_Total" pseudo-instance would double-count every real adapter.
        if (instance == L"_total") continue;
        if (!filterLower.empty() && !Contains(instance, filterLower)) continue;
        total += value.doubleValue;
    }
    return total;
}

void ReadNetworkThroughput(MetricsSnapshot& snapshot, const ModSettings& settings) {
    if (!settings.showNetwork || (!g_netRecvCounter && !g_netSentCounter)) return;
    std::wstring filter = ToLower(settings.networkAdapter);
    auto recv = ReadNetworkCounterTotal(g_netRecvCounter, filter);
    auto sent = ReadNetworkCounterTotal(g_netSentCounter, filter);
    if (recv) snapshot.networkRecvBytesPerSec = *recv;
    if (sent) snapshot.networkSentBytesPerSec = *sent;
    snapshot.networkRecvAvailable = recv.has_value();
    snapshot.networkSentAvailable = sent.has_value();
}

void ReadPdhMetrics(MetricsSnapshot& snapshot, const ModSettings& settings) {
    EnsurePdhQuery(settings);
    if (!g_pdhQuery) return;

    PDH_STATUS collectStatus = PdhCollectQueryData(g_pdhQuery);
    if (collectStatus != ERROR_SUCCESS) {
        RecordPdhReadFailure(L"collection", collectStatus);
        return;
    }

    // Nothing on screen needs the GPU? Then don't enumerate the adapter or touch its
    // counters at all - this is what "turn GPU monitoring off" has to actually mean.
    bool wantGpu = WantGpuUsage(settings);
    bool wantVram = VramRowVisible(settings);
    std::optional<GpuAdapterInfo> adapter;
    if (wantGpu || wantVram) adapter = GetGpuAdapterInfo(settings.gpuAdapter);
    PDH_STATUS gpuReadStatus = ERROR_SUCCESS;
    if (wantGpu) {
        auto gpuUsage = ReadGpuUsage(adapter, gpuReadStatus);
        if (gpuUsage) {
            snapshot.gpu = *gpuUsage;
            snapshot.gpuAvailable = true;
        }
    }
    uint64_t vramTotalBytes = 0;
    PDH_HCOUNTER vramCounter = nullptr;
    if (wantVram && adapter) {
        if (adapter->dedicatedVideoMemory > 0) {
            vramTotalBytes = adapter->dedicatedVideoMemory;
            vramCounter = g_vramCounter;
        } else if (adapter->sharedSystemMemory > 0) {
            vramTotalBytes = adapter->sharedSystemMemory;
            vramCounter = g_sharedVramCounter;
        }
    }
    bool vramCounterHasData = false;
    PDH_STATUS vramReadStatus = ERROR_SUCCESS;
    auto vramUsedBytes =
        ReadVramUsedBytes(vramCounter, adapter, vramCounterHasData, vramReadStatus);
    bool vramAvailable = vramTotalBytes > 0 && vramUsedBytes.has_value();
    if (vramAvailable) {
        snapshot.vramUsedGb = *vramUsedBytes / kGiB;
        snapshot.vramTotalGb = static_cast<double>(vramTotalBytes) / kGiB;
        snapshot.vram =
            std::clamp(snapshot.vramUsedGb / snapshot.vramTotalGb * 100.0, 0.0, 100.0);
        snapshot.vramAvailable = true;
    }

    ReadNetworkThroughput(snapshot, settings);

    bool hardReadFailure =
        (wantGpu && g_gpuCounter && IsHardPdhArrayFailure(gpuReadStatus)) ||
        (vramCounter && IsHardPdhArrayFailure(vramReadStatus));
    bool adapterMismatch = adapter && vramReadStatus == ERROR_SUCCESS &&
                          vramCounterHasData && !vramAvailable;
    bool adapterIdentityChanged =
        adapterMismatch && HasGpuAdapterIdentityChanged(*adapter, settings.gpuAdapter);
    if (hardReadFailure) {
        RecordPdhReadFailure(L"counter read");
    } else if (adapterIdentityChanged) {
        RecoverFromGpuAdapterIdentityChange();
    } else {
        RecordPdhReadSuccess();
    }
}

uint64_t FileTimeValue(const FILETIME& value) {
    ULARGE_INTEGER result{};
    result.LowPart = value.dwLowDateTime;
    result.HighPart = value.dwHighDateTime;
    return result.QuadPart;
}

double ReadCpuUsage() {
    static bool initialized = false;
    static uint64_t previousIdle = 0, previousKernel = 0, previousUser = 0;

    FILETIME idleTime{}, kernelTime{}, userTime{};
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) return 0.0;

    uint64_t idle = FileTimeValue(idleTime);
    uint64_t kernel = FileTimeValue(kernelTime);
    uint64_t user = FileTimeValue(userTime);
    if (!initialized) {
        initialized = true;
        previousIdle = idle;
        previousKernel = kernel;
        previousUser = user;
        return 0.0;
    }

    uint64_t idleDelta = idle - previousIdle;
    uint64_t kernelDelta = kernel - previousKernel;
    uint64_t userDelta = user - previousUser;
    previousIdle = idle;
    previousKernel = kernel;
    previousUser = user;

    uint64_t total = kernelDelta + userDelta;
    if (!total || idleDelta > total) return 0.0;
    return std::clamp(
        100.0 * static_cast<double>(total - idleDelta) / static_cast<double>(total), 0.0,
        100.0);
}

void ReadMemory(MetricsSnapshot& snapshot) {
    MEMORYSTATUSEX memory{};
    memory.dwLength = sizeof(memory);
    if (!GlobalMemoryStatusEx(&memory)) return;

    snapshot.ram = static_cast<double>(memory.dwMemoryLoad);
    snapshot.ramTotalGb = static_cast<double>(memory.ullTotalPhys) / kGiB;
    snapshot.ramUsedGb =
        static_cast<double>(memory.ullTotalPhys - memory.ullAvailPhys) / kGiB;
}

void ReadTemperatures(MetricsSnapshot& snapshot, const ModSettings& settings) {
    if (!WantAnyTemperature(settings)) return;

    bool wantHwInfo = false, wantWindowsNative = false;
    switch (settings.temperatureSource) {
        case TemperatureSource::SharedMemory:
        case TemperatureSource::GadgetRegistry:
        case TemperatureSource::HwInfoAuto:
            wantHwInfo = true;
            break;
        case TemperatureSource::WindowsNative:
            wantWindowsNative = true;
            break;
        case TemperatureSource::Disabled:
        case TemperatureSource::Lhm:
            // Lhm: leave everything unset here - CollectMetrics's FillFromLhm call
            // supplies it exclusively, since nothing else ran.
            return;
        case TemperatureSource::Auto:
        default:
            wantHwInfo = true;
            wantWindowsNative = true;
            break;
    }

    if (wantHwInfo) {
        HwInfoExtras extras;
        if (settings.temperatureSource == TemperatureSource::SharedMemory) {
            ReadHwInfoSharedMemory(extras, settings, true, false);
        } else if (settings.temperatureSource == TemperatureSource::GadgetRegistry) {
            ReadHwInfoGadgetRegistry(extras, settings, true, false);
        } else {
            ReadHwInfoSharedMemory(extras, settings, true, false);
            if (!extras.cpuTemp || !extras.gpuTemp) {
                ReadHwInfoGadgetRegistry(extras, settings, true, false);
            }
        }
        // Gap-aware (not an unconditional overwrite): in Auto mode, CollectMetrics
        // now runs FillFromLhm before this, so LibreHardwareMonitor is the primary
        // source when enabled - HWiNFO only supplies what LHM didn't already.
        if (extras.cpuTemp && !snapshot.cpuTemp) {
            snapshot.cpuTemp = extras.cpuTemp;
            snapshot.cpuTempProvider = extras.cpuTempProvider;
        }
        if (extras.gpuTemp && !snapshot.gpuTemp) {
            snapshot.gpuTemp = extras.gpuTemp;
            snapshot.gpuTempProvider = extras.gpuTempProvider;
        }
    }

    if ((wantWindowsNative || settings.temperatureSource == TemperatureSource::Auto) &&
        !snapshot.gpuTemp) {
        if (auto perf = ReadWindowsGpuPerfData(settings); perf.temperature) {
            snapshot.gpuTemp = perf.temperature;
            snapshot.gpuTempProvider = MetricProvider::WindowsD3dkmt;
        }
    }
    if ((wantWindowsNative || settings.temperatureSource == TemperatureSource::Auto) &&
        !snapshot.cpuTemp) {
        ReadWindowsThermalZones(snapshot, settings);
    }
}

void ReadExtraSensors(MetricsSnapshot& snapshot, const ModSettings& settings) {
    if ((!WantCpuClockPower(settings) && !WantGpuClockPower(settings)) ||
        settings.extraSensorsSource == ExtraSensorsSource::Disabled ||
        settings.extraSensorsSource == ExtraSensorsSource::LhmOnly) {
        // LhmOnly: leave everything unset - CollectMetrics's FillFromLhm call supplies
        // it exclusively, since nothing else ran.
        return;
    }

    HwInfoExtras extras;
    ReadHwInfoSharedMemory(extras, settings, false, true);
    if (!extras.cpuClockMhz || !extras.gpuClockMhz || !extras.cpuPowerW ||
        !extras.gpuPowerW) {
        ReadHwInfoGadgetRegistry(extras, settings, false, true);
    }
    // Gap-aware: in Auto mode, CollectMetrics runs FillExtrasFromLhm before this, so
    // LibreHardwareMonitor is the primary source when enabled.
    if (extras.cpuClockMhz && !snapshot.cpuClockMhz) {
        snapshot.cpuClockMhz = extras.cpuClockMhz;
        snapshot.cpuClockProvider = extras.cpuClockProvider;
    }
    if (extras.gpuClockMhz && !snapshot.gpuClockMhz) {
        snapshot.gpuClockMhz = extras.gpuClockMhz;
        snapshot.gpuClockProvider = extras.gpuClockProvider;
    }
    if (extras.cpuPowerW && !snapshot.cpuPowerW) {
        snapshot.cpuPowerW = extras.cpuPowerW;
        snapshot.cpuPowerProvider = extras.cpuPowerProvider;
    }
    if (extras.gpuPowerW && !snapshot.gpuPowerW) {
        snapshot.gpuPowerW = extras.gpuPowerW;
        snapshot.gpuPowerProvider = extras.gpuPowerProvider;
    }

    if (settings.extraSensorsSource != ExtraSensorsSource::Auto) return;

    // A native fallback only exists for CPU clock; GPU clock and both power readings
    // stay HWiNFO-only (see the comment on WindowsGpuPerfData for why GPU power has no
    // D3DKMT fallback despite the struct having a Power field).
    if (!snapshot.cpuClockMhz) {
        if (auto clock = ReadNativeCpuClockMhz()) {
            snapshot.cpuClockMhz = clock;
            snapshot.cpuClockProvider = MetricProvider::WindowsPowerInformation;
        }
    }
}

MetricsSnapshot CollectMetrics(const ModSettings& settings) {
    MetricsSnapshot snapshot;
    snapshot.cpu = ReadCpuUsage();
    ReadMemory(snapshot);
    ReadPdhMetrics(snapshot, settings);

    // LibreHardwareMonitor first when its source mode is Auto (no HWiNFO-free-tier
    // 12-hour cutoff, so it's the preferred primary), then whatever the setting
    // selects fills any remaining gap; the same two calls run again afterward
    // unconditionally so every *other* explicit source (HWiNFO-only, native-only, ...)
    // still gets LHM as a plain fallback for a genuine gap, without LHM ever
    // preempting an explicit non-Auto choice.
    if (settings.temperatureSource == TemperatureSource::Auto) FillTempFromLhm(snapshot, settings);
    if (settings.extraSensorsSource == ExtraSensorsSource::Auto) FillExtrasFromLhm(snapshot, settings);
    ReadTemperatures(snapshot, settings);
    ReadExtraSensors(snapshot, settings);
    FillTempFromLhm(snapshot, settings);
    FillExtrasFromLhm(snapshot, settings);
    return snapshot;
}

std::wstring FormatFixed(double value, int decimals) {
    wchar_t buffer[64];
    const wchar_t* fmt = decimals <= 0 ? L"%.0f" : (decimals == 1 ? L"%.1f" : L"%.2f");
    swprintf(buffer, std::size(buffer), fmt, value);
    return buffer;
}

// Pads the integer part out to a constant digit count. Character padding alone is only
// half the job: it fixes the glyph *count*, not the rendered width, and in a
// proportional font a "1" is narrower than a "0". The other half is the font - either a
// monospaced family or tabular figures, both handled in ApplyTextStyle.
std::wstring PadNumber(std::wstring text, int minIntegerDigits, NumberPadding mode) {
    if (mode == NumberPadding::None) return text;
    size_t start = (!text.empty() && text[0] == L'-') ? 1 : 0;
    size_t i = start;
    while (i < text.size() && std::iswdigit(text[i])) i++;
    int pad = minIntegerDigits - static_cast<int>(i - start);
    if (pad > 0) {
        // U+2007 FIGURE SPACE is digit-width by definition, so it occupies exactly the
        // room a digit would; a plain space does not.
        wchar_t filler = mode == NumberPadding::Zero ? L'0' : L'\u2007';
        text.insert(start, std::wstring(pad, filler));
    }
    return text;
}

std::wstring FormatPercent(double value, int decimals, NumberPadding pad) {
    return PadNumber(FormatFixed(std::clamp(value, 0.0, 100.0), decimals), 3, pad) + L"%";
}

std::wstring FormatTemperature(const std::optional<double>& value, NumberPadding pad) {
    if (!value) return L"--\u00B0C";
    return PadNumber(FormatFixed(*value, 0), 2, pad) + L"\u00B0C";
}

// The used figure pads to the total's own integer-digit count, so the pair reads as one
// column ("03.08/24.00") instead of the left half drifting a digit narrower.
std::wstring FormatCapacity(double usedGb,
                            double totalGb,
                            bool available,
                            int decimals,
                            NumberPadding pad) {
    if (!available || !std::isfinite(usedGb) || !std::isfinite(totalGb) ||
        totalGb <= 0.0) {
        return L"--/--G";
    }
    std::wstring totalText = FormatFixed(totalGb, decimals);
    size_t totalIntDigits = totalText.find(L'.');
    if (totalIntDigits == std::wstring::npos) totalIntDigits = totalText.size();
    std::wstring usedText =
        PadNumber(FormatFixed(usedGb, decimals), static_cast<int>(totalIntDigits), pad);
    return usedText + L"/" + totalText + L"G";
}

// Both branches land on 7 glyphs ("4.52GHz" / "0690MHz"), so the clock half of the
// extras cell keeps a constant width across the MHz/GHz switch.
std::wstring FormatClock(const std::optional<double>& mhz, NumberPadding pad) {
    if (!mhz) return L"-------";
    return *mhz >= 1000.0 ? PadNumber(FormatFixed(*mhz / 1000.0, 2), 1, pad) + L"GHz"
                          : PadNumber(FormatFixed(*mhz, 0), 4, pad) + L"MHz";
}

// Whole watts always: a sub-10W reading rendered as "8.7W" is two glyphs wider than
// "53W", which pushed the extras cell past its column and clipped the trailing "W".
std::wstring FormatPower(const std::optional<double>& watts, NumberPadding pad) {
    if (!watts) return L"---";
    return PadNumber(FormatFixed(*watts, 0), 2, pad) + L"W";
}

// Mirrors taskbar-clock-customization-v3's dynamic KB/s-MB/s formatter: -1 decimals
// means automatic (fewer decimals once the number is already large enough to read),
// 0-2 forces a fixed count so the display never jitters in width.
std::wstring FormatTransferSpeed(double bytesPerSecond,
                                 NetworkFormat format,
                                 int fixedDecimals) {
    constexpr double kKB = 1024.0;
    constexpr double kMB = 1024.0 * 1024.0;
    constexpr double kKbit = 1000.0 / 8.0;
    constexpr double kMbit = 1000.0 * 1000.0 / 8.0;

    double scaled = 0.0;
    PCWSTR unit = L"";
    switch (format) {
        case NetworkFormat::Mbs:
            scaled = bytesPerSecond / kMB;
            unit = L"MB/s";
            break;
        case NetworkFormat::MbsDynamic:
            if (bytesPerSecond / kKB < 1000.0) {
                scaled = bytesPerSecond / kKB;
                unit = L"KB/s";
            } else {
                scaled = bytesPerSecond / kMB;
                unit = L"MB/s";
            }
            break;
        case NetworkFormat::Mbits:
            scaled = bytesPerSecond / kMbit;
            unit = L"Mbit/s";
            break;
        case NetworkFormat::MbitsDynamic:
            if (bytesPerSecond / kKbit < 1000.0) {
                scaled = bytesPerSecond / kKbit;
                unit = L"Kbit/s";
            } else {
                scaled = bytesPerSecond / kMbit;
                unit = L"Mbit/s";
            }
            break;
    }

    int decimals = fixedDecimals;
    if (decimals < 0) decimals = scaled >= 100.0 ? 0 : 1;
    return FormatFixed(scaled, decimals) + L" " + unit;
}

enum class AlertLevel { Normal, Warning, Critical };

AlertLevel g_cpuTemperatureAlert = AlertLevel::Normal;
AlertLevel g_gpuTemperatureAlert = AlertLevel::Normal;
AlertLevel g_ramAlert = AlertLevel::Normal;
AlertLevel g_vramAlert = AlertLevel::Normal;

AlertLevel EvaluateAlert(double value,
                         double warning,
                         double critical,
                         AlertLevel previous,
                         double releaseMargin) {
    if (!std::isfinite(value)) return AlertLevel::Normal;
    if (value >= critical ||
        (previous == AlertLevel::Critical && value >= critical - releaseMargin)) {
        return AlertLevel::Critical;
    }
    if (value >= warning ||
        (previous != AlertLevel::Normal && value >= warning - releaseMargin)) {
        return AlertLevel::Warning;
    }
    return AlertLevel::Normal;
}

std::optional<Color> ParseColor(const std::wstring& value) {
    std::wstring hex = value;
    if (!hex.empty() && hex.front() == L'#') hex.erase(hex.begin());
    if (hex.size() != 6 && hex.size() != 8) return std::nullopt;
    if (!std::all_of(hex.begin(), hex.end(),
                     [](wchar_t c) { return std::iswxdigit(c) != 0; })) {
        return std::nullopt;
    }

    wchar_t* end = nullptr;
    unsigned long parsed = std::wcstoul(hex.c_str(), &end, 16);
    if (!end || *end) return std::nullopt;

    Color color{};
    color.A = hex.size() == 8 ? static_cast<uint8_t>((parsed >> 24) & 0xFF) : 0xFF;
    color.R = static_cast<uint8_t>((parsed >> 16) & 0xFF);
    color.G = static_cast<uint8_t>((parsed >> 8) & 0xFF);
    color.B = static_cast<uint8_t>(parsed & 0xFF);
    return color;
}

Color MakeColor(uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue) {
    Color color{};
    color.A = alpha;
    color.R = red;
    color.G = green;
    color.B = blue;
    return color;
}

SolidColorBrush BrushFromSetting(const std::wstring& value, Color fallback) {
    return SolidColorBrush(ParseColor(value).value_or(fallback));
}

void SetTextForeground(TextBlock text, AlertLevel alert, const ModSettings& settings) {
    if (!text) return;
    std::optional<Color> color;
    if (alert == AlertLevel::Critical) {
        color = ParseColor(settings.criticalColor);
    } else if (alert == AlertLevel::Warning) {
        color = ParseColor(settings.warningColor);
    } else {
        color = ParseColor(settings.textColor);
    }
    if (color) {
        text.Foreground(SolidColorBrush(*color));
    } else {
        text.ClearValue(TextBlock::ForegroundProperty());
    }
}

SolidColorBrush AlertBrush(AlertLevel alert, const ModSettings& settings) {
    if (alert == AlertLevel::Critical) {
        return BrushFromSetting(settings.criticalColor, MakeColor(0xFF, 0xFF, 0x6B, 0x6B));
    }
    if (alert == AlertLevel::Warning) {
        return BrushFromSetting(settings.warningColor, MakeColor(0xFF, 0xFF, 0xB9, 0x00));
    }
    return BrushFromSetting(settings.graphColor, MakeColor(0xFF, 0x78, 0xA8, 0xFF));
}

size_t HistoryCapacity(const ModSettings& settings) {
    int intervals =
        (settings.historySeconds + settings.updateInterval - 1) / settings.updateInterval;
    return std::max<size_t>(2, static_cast<size_t>(intervals) + 1);
}

void AppendHistory(std::deque<double>& history, double value, size_t capacity) {
    history.push_back(std::clamp(value, 0.0, 100.0));
    while (history.size() > capacity) history.pop_front();
}

// Builds the trace and, from the same points, the closed region beneath it. The fill
// is what separates "the line" from "the area under the line" - without it a sparkline
// on a panel reads as a stray squiggle rather than a level.
void UpdateSparkline(XamlPolyline graph,
                     XamlPolygon area,
                     const std::deque<double>& history,
                     size_t capacity) {
    if (!graph) return;

    auto hide = [&] {
        graph.Visibility(Visibility::Collapsed);
        if (area) area.Visibility(Visibility::Collapsed);
    };
    graph.Points().Clear();
    if (area) area.Points().Clear();
    if (history.size() < 2 || capacity < 2 || g_graphWidth <= 1.0) {
        hide();
        return;
    }

    constexpr double verticalPadding = 1.0;
    double usableHeight = kGraphHeight - verticalPadding * 2.0;
    double step = g_graphWidth / static_cast<double>(capacity - 1);
    double firstX = g_graphWidth - step * static_cast<double>(history.size() - 1);
    double lastX = firstX;
    for (size_t i = 0; i < history.size(); i++) {
        double x = firstX + step * static_cast<double>(i);
        double y = verticalPadding + (100.0 - std::clamp(history[i], 0.0, 100.0)) /
                                         100.0 * usableHeight;
        Point p{static_cast<float>(x), static_cast<float>(y)};
        graph.Points().Append(p);
        if (area) area.Points().Append(p);
        lastX = x;
    }
    if (area) {
        // Drop to the baseline at both ends to close the shape under the trace.
        area.Points().Append(
            Point{static_cast<float>(lastX), static_cast<float>(kGraphHeight)});
        area.Points().Append(
            Point{static_cast<float>(firstX), static_cast<float>(kGraphHeight)});
        area.Visibility(Visibility::Visible);
    }
    graph.Visibility(Visibility::Visible);
}

void UpdateMemoryBar(XamlRectangle fill,
                     XamlRectangle track,
                     double percent,
                     bool available,
                     AlertLevel alert,
                     const ModSettings& settings) {
    if (!fill) return;
    // The track stretches across the whole row, so whatever width the Auto columns
    // settled on this pass is already measured on it - no need to predict the panel.
    double full = track ? track.ActualWidth() : 0.0;
    fill.Width(available ? full * std::clamp(percent, 0.0, 100.0) / 100.0 : 0.0);
    fill.Fill(AlertBrush(alert, settings));
}

template <typename F>
FrameworkElement FindChildRecursive(FrameworkElement element, F callback, int depth = 16) {
    if (!element || depth <= 0) return nullptr;
    int count = VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < count; i++) {
        auto child = VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();
        if (!child) continue;
        if (callback(child)) return child;
        if (auto nested = FindChildRecursive(child, callback, depth - 1)) return nested;
    }
    return nullptr;
}

FrameworkElement FindDirectChildByName(FrameworkElement parent, PCWSTR name) {
    if (!parent) return nullptr;
    int count = VisualTreeHelper::GetChildrenCount(parent);
    for (int i = 0; i < count; i++) {
        auto child = VisualTreeHelper::GetChild(parent, i).try_as<FrameworkElement>();
        if (child && child.Name() == name) return child;
    }
    return nullptr;
}

// SystemTrayFrameGrid sits several levels below xamlRoot.Content() (content -> ... ->
// TaskbarFrame -> RootGrid -> ... -> SystemTrayFrame -> SystemTrayFrameGrid), not as a
// direct child of it - FindDirectChildByName against the content root never finds it.
FrameworkElement FindDescendantByName(FrameworkElement const& root, PCWSTR name) {
    std::wstring target = name;
    return FindChildRecursive(
        root, [&target](FrameworkElement child) { return child.Name() == target; });
}

FrameworkElement FindTrayElement(FrameworkElement const& trayGrid,
                                 FrameworkElement const& root,
                                 PCWSTR name) {
    auto elem = FindDirectChildByName(trayGrid, name);
    if (!elem) elem = FindDescendantByName(root, name);
    return elem;
}

int TrayChildIndex(Panel const& panel, PCWSTR name) {
    if (!panel) return -1;
    auto children = panel.Children();
    for (uint32_t i = 0; i < children.Size(); ++i) {
        auto fe = children.GetAt(i).try_as<FrameworkElement>();
        if (fe && fe.Name() == name) return static_cast<int>(i);
    }
    return -1;
}

void ApplyTextStyle(TextBlock text, bool label, const ModSettings& settings) {
    if (!text) return;
    text.FontFamily(Media::FontFamily(settings.fontFamily));
    text.FontSize(settings.fontSize);
    text.FontWeight(label ? Text::FontWeights::SemiBold() : Text::FontWeights::Normal());
    double opacity = static_cast<double>(settings.textOpacity) / 100.0;
    text.Opacity(label ? opacity * settings.labelOpacity / 100.0 : opacity);
    text.TextWrapping(TextWrapping::NoWrap);
    text.TextTrimming(TextTrimming::None);

    // The half of column stability that padding cannot provide. Padding fixes the
    // number of glyphs; this fixes their width, by asking the font for tabular
    // (fixed-advance) figures - the OpenType "tnum" feature. Without it, in any
    // proportional font, "11" is visibly narrower than "00" and every cell still
    // shifts no matter how carefully the string was padded.
    Documents::Typography::SetNumeralAlignment(
        text, settings.tabularFigures ? FontNumeralAlignment::Tabular
                                      : FontNumeralAlignment::Normal);

    // Pin every cell to one line box of exactly the text band height, top-aligned.
    // Left to its own font metrics a TextBlock's height varies with which glyphs it
    // holds, so an arrow, a degree sign and plain digits would each sit on a slightly
    // different line; this puts every column on one baseline and leaves the strip
    // below the band clear for the memory bars.
    text.LineHeight(kTextLineHeight);
    text.LineStackingStrategy(LineStackingStrategy::BlockLineHeight);
    text.VerticalAlignment(VerticalAlignment::Top);

    if (label && !settings.labelColor.empty()) {
        if (auto color = ParseColor(settings.labelColor)) {
            text.Foreground(SolidColorBrush(*color));
            return;
        }
    }
    SetTextForeground(text, AlertLevel::Normal, settings);
}

// Background only - what a hover enter/exit needs, without redoing every column width.
void ApplyBoxBackground(const ModSettings& settings) {
    if (!g_widgetBorder) return;
    bool visible = settings.boxMode == BoxMode::Always ||
                   (settings.boxMode == BoxMode::Hover && g_widgetHovered);
    if (visible) {
        g_widgetBorder.Background(
            BrushFromSetting(settings.boxColor, MakeColor(0xE6, 0x24, 0x24, 0x24)));
    } else {
        // A transparent brush rather than ClearValue: an unset Background makes the
        // Border invisible to hit testing, which would break hover-to-reveal.
        g_widgetBorder.Background(SolidColorBrush(MakeColor(0x00, 0x00, 0x00, 0x00)));
    }
}

void ApplyWidgetGeometry(const ModSettings& settings) {
    if (!g_widget || !g_widgetBorder) return;

    // No predicted widths any more: every cell column is Auto, so each one measures
    // its own content and the panel is exactly the sum of what is actually on screen.
    // Font size needs no scaling factor either - Auto follows the glyphs.
    bool cpuRow = CpuRowVisible(settings);
    bool gpuRow = GpuRowVisible(settings);
    bool ramRow = RamRowVisible(settings);
    bool vramRow = VramRowVisible(settings);
    bool cpuGraph = cpuRow && settings.showCpuGraph;
    bool gpuGraph = gpuRow && settings.showGpuGraph;
    bool anyGraph = cpuGraph || gpuGraph;
    g_graphWidth = anyGraph ? static_cast<double>(settings.graphWidth) : 0.0;

    g_widgetBorder.MinWidth(settings.widthMin);
    g_widgetBorder.MaxWidth(settings.widthMax > 0 ? settings.widthMax
                                                  : std::numeric_limits<double>::infinity());
    g_widgetBorder.MinHeight(settings.heightMin);
    g_widgetBorder.MaxHeight(settings.heightMax > 0
                                 ? settings.heightMax
                                 : std::numeric_limits<double>::infinity());
    // Padding is applied whatever the box mode, so the layout doesn't jump sideways the
    // moment a hover-revealed background appears or disappears. The vertical half is
    // capped hard: the tray clips a too-tall child from the bottom, and the memory bars
    // live on that edge.
    double padX = static_cast<double>(settings.boxPadding);
    double padY = std::min(2.0, padX / 2);
    g_widgetBorder.Padding(Thickness{padX, padY, padX, padY});
    g_widgetBorder.CornerRadius(
        CornerRadius{static_cast<double>(settings.boxCornerRadius),
                     static_cast<double>(settings.boxCornerRadius),
                     static_cast<double>(settings.boxCornerRadius),
                     static_cast<double>(settings.boxCornerRadius)});

    ApplyBoxBackground(settings);

    if (g_gapColumn) {
        g_gapColumn.Width(GridLength{
            (RamRowVisible(settings) || VramRowVisible(settings)) &&
                    (CpuRowVisible(settings) || GpuRowVisible(settings))
                ? static_cast<double>(settings.columnGap)
                : 0.0,
            GridUnitType::Pixel});
    }
    // Auto columns sit flush against each other, so the gutter is an explicit margin on
    // the cells that follow another one. This is the whole of the horizontal spacing:
    // one number, applied uniformly, instead of slack hidden in per-cell widths.
    double cellGap = static_cast<double>(settings.cellGap);
    for (TextBlock cell : {g_cpuUsageText, g_cpuTempText, g_cpuClockText,
                           g_cpuPowerText, g_gpuUsageText, g_gpuTempText,
                           g_gpuClockText, g_gpuPowerText, g_ramPercentText,
                           g_ramCapacityText, g_vramPercentText, g_vramCapacityText,
                           g_netUpText, g_netDownText}) {
        if (cell) cell.Margin(Thickness{cellGap, 0, 0, 0});
    }

    auto vis = [](bool on) { return on ? Visibility::Visible : Visibility::Collapsed; };

    // Per-field visibility. Auto columns mean a hidden cell's column collapses to
    // nothing on its own, so hiding a field closes its gap instead of leaving a hole.
    if (g_cpuUsageText) g_cpuUsageText.Visibility(vis(cpuRow && settings.showCpuUsage));
    if (g_cpuTempText) g_cpuTempText.Visibility(vis(cpuRow && settings.showCpuTemp));
    if (g_cpuClockText) g_cpuClockText.Visibility(vis(cpuRow && settings.showCpuClock));
    if (g_cpuPowerText) g_cpuPowerText.Visibility(vis(cpuRow && settings.showCpuPower));
    if (g_cpuLabel) g_cpuLabel.Visibility(vis(cpuRow));
    if (g_gpuUsageText) g_gpuUsageText.Visibility(vis(gpuRow && settings.showGpuUsage));
    if (g_gpuTempText) g_gpuTempText.Visibility(vis(gpuRow && settings.showGpuTemp));
    if (g_gpuClockText) g_gpuClockText.Visibility(vis(gpuRow && settings.showGpuClock));
    if (g_gpuPowerText) g_gpuPowerText.Visibility(vis(gpuRow && settings.showGpuPower));
    if (g_gpuLabel) g_gpuLabel.Visibility(vis(gpuRow));
    if (g_ramLabel) g_ramLabel.Visibility(vis(ramRow));
    if (g_ramPercentText) {
        g_ramPercentText.Visibility(vis(ramRow && settings.showRamPercent));
    }
    if (g_ramCapacityText) {
        g_ramCapacityText.Visibility(vis(ramRow && settings.showRamCapacity));
    }
    if (g_vramLabel) g_vramLabel.Visibility(vis(vramRow));
    if (g_vramPercentText) {
        g_vramPercentText.Visibility(vis(vramRow && settings.showVramPercent));
    }
    if (g_vramCapacityText) {
        g_vramCapacityText.Visibility(vis(vramRow && settings.showVramCapacity));
    }
    for (XamlRectangle bar : {g_ramTrack, g_ramFill}) {
        if (bar) bar.Visibility(vis(ramRow && settings.showRamBar));
    }
    for (XamlRectangle bar : {g_vramTrack, g_vramFill}) {
        if (bar) bar.Visibility(vis(vramRow && settings.showVramBar));
    }

    // An empty row must give up its height too, or it leaves a blank band behind. The
    // gap row only earns its height when there are two rows to separate.
    if (g_cpuRowDef) {
        g_cpuRowDef.Height(GridLength{cpuRow ? kRowHeight : 0.0, GridUnitType::Pixel});
    }
    if (g_gpuRowDef) {
        g_gpuRowDef.Height(GridLength{gpuRow ? kRowHeight : 0.0, GridUnitType::Pixel});
    }
    if (g_ramRowDef) {
        g_ramRowDef.Height(GridLength{ramRow ? kRowHeight : 0.0, GridUnitType::Pixel});
    }
    if (g_vramRowDef) {
        g_vramRowDef.Height(GridLength{vramRow ? kRowHeight : 0.0, GridUnitType::Pixel});
    }
    double rowGap = static_cast<double>(settings.rowGap);
    if (g_leftGapRow) {
        g_leftGapRow.Height(
            GridLength{cpuRow && gpuRow ? rowGap : 0.0, GridUnitType::Pixel});
    }
    if (g_rightGapRow) {
        g_rightGapRow.Height(
            GridLength{ramRow && vramRow ? rowGap : 0.0, GridUnitType::Pixel});
    }
    if (g_netGapRow) {
        g_netGapRow.Height(GridLength{rowGap, GridUnitType::Pixel});
    }

    bool showNetColumn = settings.showNetwork || settings.showInternetStatus;
    bool leftVisible = cpuRow || gpuRow;
    bool rightVisible = ramRow || vramRow;
    if (g_leftPanel) g_leftPanel.Visibility(vis(leftVisible));
    if (g_rightPanel) g_rightPanel.Visibility(vis(rightVisible));
    if (g_netColumn) g_netColumn.Visibility(vis(showNetColumn));

    // A seam only makes sense with something on both sides of it, so a section that is
    // switched off takes its divider and its gap with it rather than leaving a stray
    // line at the edge.
    bool netSeam = showNetColumn && (leftVisible || rightVisible);
    bool panelSeam = leftVisible && rightVisible;
    double columnGap = static_cast<double>(settings.columnGap);
    if (g_netGapColumn) {
        g_netGapColumn.Width(GridLength{netSeam ? columnGap : 0.0, GridUnitType::Pixel});
    }
    if (g_dividerNet) {
        g_dividerNet.Visibility(vis(settings.showDividers && netSeam));
    }
    if (g_dividerPanels) {
        g_dividerPanels.Visibility(vis(settings.showDividers && panelSeam));
    }
    if (g_netDownText) {
        g_netDownText.Visibility(settings.showNetwork ? Visibility::Visible
                                                       : Visibility::Collapsed);
    }
    if (g_netUpText) {
        g_netUpText.Visibility(settings.showNetwork ? Visibility::Visible
                                                     : Visibility::Collapsed);
    }
    for (TextBlock arrow : {g_netUpArrow, g_netDownArrow}) {
        if (arrow) {
            arrow.Visibility(settings.showNetwork ? Visibility::Visible
                                                  : Visibility::Collapsed);
        }
    }
    if (g_netDot) {
        g_netDot.Visibility(vis(settings.showInternetStatus));
        // One Rectangle covers every shape: a full corner radius reads as a circle, a
        // small one as a rounded square, none as a square, and a narrow tall box as a
        // bar. Cheaper than swapping the element out per shape.
        double w = kDotSize, h = kDotSize, radius = kDotSize / 2;
        switch (settings.netDotShape) {
            case DotShape::Rounded:
                radius = 2.0;
                break;
            case DotShape::Square:
                radius = 0.0;
                break;
            case DotShape::Bar:
                w = 3.0;
                h = kDotSize * 1.7;
                radius = 1.5;
                break;
            case DotShape::Circle:
            default:
                break;
        }
        g_netDot.Width(w);
        g_netDot.Height(h);
        g_netDot.RadiusX(radius);
        g_netDot.RadiusY(radius);
        g_netDot.Margin(Thickness{static_cast<double>(settings.netDotMarginLeft), 0,
                                  static_cast<double>(settings.netDotMarginRight), 0});
    }

    if (g_cpuGraph) {
        g_cpuGraph.Width(g_graphWidth);
        g_cpuGraph.Height(kGraphHeight);
        g_cpuGraph.Visibility(vis(cpuGraph));
    }
    if (g_gpuGraph) {
        g_gpuGraph.Width(g_graphWidth);
        g_gpuGraph.Height(kGraphHeight);
        g_gpuGraph.Visibility(vis(gpuGraph));
    }
    double barH = static_cast<double>(settings.barHeight);
    for (XamlRectangle bar : {g_ramTrack, g_vramTrack, g_ramFill, g_vramFill}) {
        if (!bar) continue;
        bar.Height(barH);
        bar.RadiusX(barH / 2);
        bar.RadiusY(barH / 2);
    }
    bool showBg = settings.graphBackgroundOpacity > 0;
    if (g_cpuGraphBackground) g_cpuGraphBackground.Visibility(vis(cpuGraph && showBg));
    if (g_gpuGraphBackground) g_gpuGraphBackground.Visibility(vis(gpuGraph && showBg));
    for (XamlRectangle background : {g_cpuGraphBackground, g_gpuGraphBackground}) {
        if (!background) continue;
        background.Width(g_graphWidth);
        background.Height(kGraphHeight);
    }
    if (g_cpuGraphArea) g_cpuGraphArea.Visibility(vis(cpuGraph));
    if (g_gpuGraphArea) g_gpuGraphArea.Visibility(vis(gpuGraph));
    for (XamlPolygon area : {g_cpuGraphArea, g_gpuGraphArea}) {
        if (!area) continue;
        area.Width(g_graphWidth);
        area.Height(kGraphHeight);
    }
}

void ApplyWidgetSettings() {
    if (!g_widget || !g_widgetBorder) return;
    ModSettings settings = CurrentSettings();
    if (g_historyInterval != settings.updateInterval ||
        g_historyWindow != settings.historySeconds) {
        g_cpuHistory.clear();
        g_gpuHistory.clear();
        g_historyInterval = settings.updateInterval;
        g_historyWindow = settings.historySeconds;
    }
    ApplyWidgetGeometry(settings);

    for (TextBlock label : {g_cpuLabel, g_gpuLabel, g_ramLabel, g_vramLabel}) {
        ApplyTextStyle(label, true, settings);
    }
    for (TextBlock value :
        {g_cpuUsageText, g_cpuTempText, g_cpuClockText, g_cpuPowerText,
         g_gpuUsageText, g_gpuTempText, g_gpuClockText, g_gpuPowerText,
         g_ramPercentText, g_ramCapacityText, g_vramPercentText,
         g_vramCapacityText, g_netDownText, g_netUpText, g_netDownArrow,
         g_netUpArrow}) {
        ApplyTextStyle(value, false, settings);
    }

    SolidColorBrush graphBrush = AlertBrush(AlertLevel::Normal, settings);
    for (XamlPolyline graph : {g_cpuGraph, g_gpuGraph}) {
        if (graph) {
            graph.Stroke(graphBrush);
            graph.StrokeThickness(1.25);
            graph.StrokeStartLineCap(PenLineCap::Round);
            graph.StrokeEndLineCap(PenLineCap::Round);
            graph.StrokeLineJoin(PenLineJoin::Round);
            graph.Opacity(settings.graphOpacity / 100.0);
        }
    }
    for (XamlRectangle background : {g_cpuGraphBackground, g_gpuGraphBackground}) {
        if (background) {
            background.Fill(graphBrush);
            background.Opacity(settings.graphBackgroundOpacity / 100.0);
        }
    }
    for (XamlPolygon area : {g_cpuGraphArea, g_gpuGraphArea}) {
        if (area) {
            area.Fill(graphBrush);
            area.Opacity(settings.graphAreaOpacity / 100.0);
        }
    }
    for (XamlRectangle track : {g_ramTrack, g_vramTrack}) {
        if (track) {
            track.Fill(graphBrush);
            track.Opacity(0.18);
        }
    }
    for (XamlRectangle fill : {g_ramFill, g_vramFill}) {
        if (fill) {
            fill.Fill(graphBrush);
            fill.Opacity(0.76);
        }
    }
    if (g_netDot) g_netDot.Fill(SolidColorBrush(MakeColor(0xFF, 0x80, 0x80, 0x80)));

    size_t capacity = HistoryCapacity(settings);
    while (g_cpuHistory.size() > capacity) g_cpuHistory.pop_front();
    while (g_gpuHistory.size() > capacity) g_gpuHistory.pop_front();
    UpdateSparkline(g_cpuGraph, g_cpuGraphArea, g_cpuHistory, capacity);
    UpdateSparkline(g_gpuGraph, g_gpuGraphArea, g_gpuHistory, capacity);

    if (g_timer) g_timer.Interval(std::chrono::seconds(settings.updateInterval));
}

void UpdateInternetStatusUi(const ModSettings& settings) {
    if (!settings.showInternetStatus) return;
    InternetState state = g_internetState.load();
    if (g_netDot) {
        Color dotColor;
        switch (state) {
            case InternetState::Connected:
                dotColor = MakeColor(0xFF, 0x4C, 0xD9, 0x64);
                break;
            case InternetState::Disconnected:
                dotColor = ParseColor(settings.criticalColor)
                              .value_or(MakeColor(0xFF, 0xFF, 0x6B, 0x6B));
                break;
            default:
                dotColor = MakeColor(0xFF, 0x80, 0x80, 0x80);
                break;
        }
        g_netDot.Fill(SolidColorBrush(dotColor));
    }
}

void UpdateWidgetText() {
    if (!g_widget || g_unloading) return;
    ModSettings settings = CurrentSettings();
    MetricsSnapshot snapshot;
    uint64_t metricsSequence = 0;
    if (!GetLatestMetrics(snapshot, metricsSequence)) return;

    g_cpuTemperatureAlert =
        snapshot.cpuTemp ? EvaluateAlert(*snapshot.cpuTemp, settings.cpuWarningTemp,
                                         settings.cpuCriticalTemp,
                                         g_cpuTemperatureAlert, 3.0)
                         : AlertLevel::Normal;
    g_gpuTemperatureAlert =
        snapshot.gpuTemp ? EvaluateAlert(*snapshot.gpuTemp, settings.gpuWarningTemp,
                                         settings.gpuCriticalTemp,
                                         g_gpuTemperatureAlert, 3.0)
                         : AlertLevel::Normal;
    g_ramAlert = EvaluateAlert(snapshot.ram, settings.memoryWarningPercent,
                               settings.memoryCriticalPercent, g_ramAlert, 3.0);
    g_vramAlert = snapshot.vramAvailable
                     ? EvaluateAlert(snapshot.vram, settings.memoryWarningPercent,
                                     settings.memoryCriticalPercent, g_vramAlert, 3.0)
                     : AlertLevel::Normal;

    NumberPadding pad = settings.numberPadding;
    if (g_cpuUsageText) {
        g_cpuUsageText.Text(FormatPercent(snapshot.cpu, settings.usageDecimals, pad));
    }
    if (g_cpuTempText) {
        g_cpuTempText.Text(FormatTemperature(snapshot.cpuTemp, pad));
        SetTextForeground(g_cpuTempText, g_cpuTemperatureAlert, settings);
    }
    if (g_cpuClockText) g_cpuClockText.Text(FormatClock(snapshot.cpuClockMhz, pad));
    if (g_cpuPowerText) g_cpuPowerText.Text(FormatPower(snapshot.cpuPowerW, pad));
    if (g_gpuUsageText) {
        g_gpuUsageText.Text(snapshot.gpuAvailable
                               ? FormatPercent(snapshot.gpu, settings.usageDecimals, pad)
                               : L"--%");
    }
    if (g_gpuTempText) {
        g_gpuTempText.Text(FormatTemperature(snapshot.gpuTemp, pad));
        SetTextForeground(g_gpuTempText, g_gpuTemperatureAlert, settings);
    }
    if (g_gpuClockText) g_gpuClockText.Text(FormatClock(snapshot.gpuClockMhz, pad));
    if (g_gpuPowerText) g_gpuPowerText.Text(FormatPower(snapshot.gpuPowerW, pad));
    if (g_ramPercentText) {
        g_ramPercentText.Text(FormatPercent(snapshot.ram, settings.usageDecimals, pad));
        SetTextForeground(g_ramPercentText, g_ramAlert, settings);
    }
    if (g_ramCapacityText) {
        g_ramCapacityText.Text(FormatCapacity(snapshot.ramUsedGb, snapshot.ramTotalGb,
                                              true, settings.capacityDecimals, pad));
    }
    if (g_vramPercentText) {
        g_vramPercentText.Text(snapshot.vramAvailable
                                   ? FormatPercent(snapshot.vram, settings.usageDecimals, pad)
                                   : L"--%");
        SetTextForeground(g_vramPercentText, g_vramAlert, settings);
    }
    if (g_vramCapacityText) {
        g_vramCapacityText.Text(FormatCapacity(snapshot.vramUsedGb, snapshot.vramTotalGb,
                                               snapshot.vramAvailable,
                                               settings.capacityDecimals, pad));
    }
    if (settings.showNetwork) {
        if (g_netDownText) {
            g_netDownText.Text(
                snapshot.networkRecvAvailable
                    ? FormatTransferSpeed(snapshot.networkRecvBytesPerSec,
                                          settings.networkFormat,
                                          settings.networkDecimals)
                    : L"--");
        }
        if (g_netUpText) {
            g_netUpText.Text(
                snapshot.networkSentAvailable
                    ? FormatTransferSpeed(snapshot.networkSentBytesPerSec,
                                          settings.networkFormat,
                                          settings.networkDecimals)
                    : L"--");
        }
    }
    UpdateInternetStatusUi(settings);

    if (metricsSequence != g_lastRenderedMetricsSequence) {
        size_t historyCapacity = HistoryCapacity(settings);
        AppendHistory(g_cpuHistory, snapshot.cpu, historyCapacity);
        if (snapshot.gpuAvailable) AppendHistory(g_gpuHistory, snapshot.gpu, historyCapacity);
        if (CpuRowVisible(settings) && settings.showCpuGraph) {
            UpdateSparkline(g_cpuGraph, g_cpuGraphArea, g_cpuHistory, historyCapacity);
        }
        if (GpuRowVisible(settings) && settings.showGpuGraph) {
            UpdateSparkline(g_gpuGraph, g_gpuGraphArea, g_gpuHistory, historyCapacity);
        }
        g_lastRenderedMetricsSequence = metricsSequence;
    }
    UpdateMemoryBar(g_ramFill, g_ramTrack, snapshot.ram, true, g_ramAlert, settings);
    UpdateMemoryBar(g_vramFill, g_vramTrack, snapshot.vram, snapshot.vramAvailable, g_vramAlert, settings);
}

void EnsureTimer() {
    if (g_timer) return;
    ModSettings settings = CurrentSettings();
    g_timer = DispatcherTimer();
    g_timer.Interval(std::chrono::seconds(settings.updateInterval));
    g_timerToken = g_timer.Tick([](IInspectable const&, IInspectable const&) {
        try {
            UpdateWidgetText();
        } catch (...) {
            HRESULT error = winrt::to_hresult();
            Wh_Log(L"Metrics update failed: %08X", static_cast<unsigned>(error));
        }
    });
    g_timer.Start();
}

RowDefinition PixelRow(double height) {
    RowDefinition row;
    row.Height(GridLength{height, GridUnitType::Pixel});
    return row;
}

ColumnDefinition AutoColumn() {
    ColumnDefinition column;
    column.Width(GridLength{0, GridUnitType::Auto});
    return column;
}

TextBlock CreateCellText(PCWSTR name, TextAlignment alignment) {
    TextBlock text;
    text.Name(name);
    text.HorizontalAlignment(HorizontalAlignment::Stretch);
    text.TextAlignment(alignment);
    text.TextWrapping(TextWrapping::NoWrap);
    text.TextTrimming(TextTrimming::None);
    text.IsHitTestVisible(false);
    return text;
}

// Both compute rows are placed into one shared Grid rather than each getting its own.
// That is what lets the columns be Auto: they size to the widest value across CPU and
// GPU together, so a cell is only ever as wide as its content actually needs, and the
// two rows stay aligned because they are literally the same columns. Fixed worst-case
// widths were what left the dead gap between "CPU" and its percentage.
void AddComputeRow(Grid panel,
                   int row,
                   PCWSTR label,
                   PCWSTR prefix,
                   TextBlock& labelText,
                   TextBlock& usageText,
                   TextBlock& temperatureText,
                   TextBlock& clockText,
                   TextBlock& powerText,
                   XamlPolyline& graph,
                   XamlPolygon& graphArea,
                   XamlRectangle& graphBackground) {
    labelText = CreateCellText((std::wstring(prefix) + L"Label").c_str(),
                               TextAlignment::Left);
    labelText.Text(label);
    usageText = CreateCellText((std::wstring(prefix) + L"Usage").c_str(),
                               TextAlignment::Right);
    usageText.Text(L"--%");
    temperatureText = CreateCellText((std::wstring(prefix) + L"Temperature").c_str(),
                                     TextAlignment::Right);
    temperatureText.Text(L"--\u00B0C");
    clockText = CreateCellText((std::wstring(prefix) + L"Clock").c_str(),
                               TextAlignment::Right);
    clockText.Text(L"--");
    powerText = CreateCellText((std::wstring(prefix) + L"Power").c_str(),
                               TextAlignment::Right);
    powerText.Text(L"--");

    // Faint panel marking the graph's extent, the filled region under the trace, then
    // the trace itself - painted in that order so each sits on top of the last.
    graphBackground = XamlRectangle();
    graphBackground.Name((std::wstring(prefix) + L"GraphBg").c_str());
    graphBackground.RadiusX(2);
    graphBackground.RadiusY(2);
    graphArea = XamlPolygon();
    graphArea.Name((std::wstring(prefix) + L"GraphArea").c_str());
    graph = XamlPolyline();
    graph.Name((std::wstring(prefix) + L"History").c_str());
    for (Shapes::Shape shape : {graphBackground.as<Shapes::Shape>(),
                                graphArea.as<Shapes::Shape>(),
                                graph.as<Shapes::Shape>()}) {
        shape.HorizontalAlignment(HorizontalAlignment::Left);
        shape.VerticalAlignment(VerticalAlignment::Top);
        shape.Margin(Thickness{kGraphLeftGap, 1, 0, 0});
        shape.Stretch(Stretch::None);
        shape.IsHitTestVisible(false);
        Grid::SetRow(shape, row);
        Grid::SetColumn(shape, 5);
        panel.Children().Append(shape);
    }

    TextBlock cells[] = {labelText, usageText, temperatureText, clockText, powerText};
    for (int i = 0; i < 5; i++) {
        Grid::SetRow(cells[i], row);
        Grid::SetColumn(cells[i], i);
        panel.Children().Append(cells[i]);
    }
}

// RAM/VRAM row into the shared memory panel, same Auto-column reasoning as above. The
// capacity bar stretches the full row width; its fill is sized from the track's
// measured width at update time, since Auto columns decide that only after layout.
void AddMemoryRow(Grid panel,
                  int row,
                  PCWSTR label,
                  PCWSTR prefix,
                  TextBlock& labelText,
                  TextBlock& percentText,
                  TextBlock& capacityText,
                  XamlRectangle& track,
                  XamlRectangle& fill) {
    track = XamlRectangle();
    track.Name((std::wstring(prefix) + L"Track").c_str());
    track.HorizontalAlignment(HorizontalAlignment::Stretch);
    fill = XamlRectangle();
    fill.Name((std::wstring(prefix) + L"Fill").c_str());
    fill.HorizontalAlignment(HorizontalAlignment::Left);
    for (XamlRectangle bar : {track, fill}) {
        bar.VerticalAlignment(VerticalAlignment::Bottom);
        bar.IsHitTestVisible(false);
        Grid::SetRow(bar, row);
        Grid::SetColumn(bar, 0);
        Grid::SetColumnSpan(bar, 3);
        panel.Children().Append(bar);
    }

    labelText =
        CreateCellText((std::wstring(prefix) + L"Label").c_str(), TextAlignment::Left);
    labelText.Text(label);
    percentText =
        CreateCellText((std::wstring(prefix) + L"Percent").c_str(), TextAlignment::Right);
    percentText.Text(L"--%");
    capacityText = CreateCellText((std::wstring(prefix) + L"Capacity").c_str(),
                                  TextAlignment::Right);
    capacityText.Text(L"--/--G");

    TextBlock cells[] = {labelText, percentText, capacityText};
    for (int i = 0; i < 3; i++) {
        Grid::SetRow(cells[i], row);
        Grid::SetColumn(cells[i], i);
        panel.Children().Append(cells[i]);
    }
}

// A column, not a third row: up/down stacked over the same two-row height as the
// CPU/GPU and RAM/VRAM panels, with the internet-status dot as a colour-only badge.
Grid CreateNetworkColumn() {
    Grid column;
    column.Name(L"NetworkColumn");
    column.IsHitTestVisible(false);
    column.VerticalAlignment(VerticalAlignment::Center);

    g_netGapRow = PixelRow(2.0);
    column.RowDefinitions().Append(PixelRow(kRowHeight));
    column.RowDefinitions().Append(g_netGapRow);
    column.RowDefinitions().Append(PixelRow(kRowHeight));
    // Arrow in its own fixed column rather than glued onto the value string: keeps the
    // arrows from drifting as the number beside them changes width, and provides the
    // arrow-to-value gap without padding either string.
    column.ColumnDefinitions().Append(AutoColumn());
    column.ColumnDefinitions().Append(AutoColumn());
    column.ColumnDefinitions().Append(AutoColumn());

    g_netDot = XamlRectangle();
    g_netDot.Name(L"InternetDot");
    g_netDot.HorizontalAlignment(HorizontalAlignment::Center);
    g_netDot.VerticalAlignment(VerticalAlignment::Center);
    g_netDot.IsHitTestVisible(false);

    // Upload on top, download on bottom, matching the user's own
    // taskbar-clock-customization-v3 config (TopLine carries %upload_speed%).
    g_netUpArrow = CreateCellText(L"NetUpArrow", TextAlignment::Left);
    g_netUpArrow.Text(L"↑");
    g_netDownArrow = CreateCellText(L"NetDownArrow", TextAlignment::Left);
    g_netDownArrow.Text(L"↓");
    g_netUpText = CreateCellText(L"NetUp", TextAlignment::Right);
    g_netUpText.Text(L"--");
    g_netDownText = CreateCellText(L"NetDown", TextAlignment::Right);
    g_netDownText.Text(L"--");

    Grid::SetColumn(g_netDot, 0);
    Grid::SetRow(g_netDot, 0);
    Grid::SetRowSpan(g_netDot, 3);
    Grid::SetColumn(g_netUpArrow, 1);
    Grid::SetRow(g_netUpArrow, 0);
    Grid::SetColumn(g_netUpText, 2);
    Grid::SetRow(g_netUpText, 0);
    Grid::SetColumn(g_netDownArrow, 1);
    Grid::SetRow(g_netDownArrow, 2);
    Grid::SetColumn(g_netDownText, 2);
    Grid::SetRow(g_netDownText, 2);

    column.Children().Append(g_netDot);
    column.Children().Append(g_netUpArrow);
    column.Children().Append(g_netUpText);
    column.Children().Append(g_netDownArrow);
    column.Children().Append(g_netDownText);
    return column;
}

// Undoes AttachAnchorTracking: revokes the LayoutUpdated subscription and restores the
// anchor button's original margin. Must run before g_injectionParent/g_trackedElement
// are reassigned to a new target - otherwise a stale handler from the old anchored
// position keeps running against the new position's state (both read the same
// globals), and the old anchor's margin stays permanently widened.
void DetachAnchorTracking() {
    if (g_hasLayoutUpdatedToken && g_injectionParent) {
        try {
            g_injectionParent.LayoutUpdated(g_layoutUpdatedToken);
        } catch (...) {
        }
    }
    g_layoutUpdatedToken = {};
    g_hasLayoutUpdatedToken = false;
    if (g_trackedElement && g_hasTrackedOriginalMargin) {
        try {
            g_trackedElement.Margin(g_trackedOriginalMargin);
        } catch (...) {
        }
    }
    g_trackedElement = nullptr;
    g_hasTrackedOriginalMargin = false;
}

int RemoveWidgetFromPanel(Panel const& targetPanel);

// Revokes the hover handlers off whatever Border currently owns them. Must run before
// a new Border replaces g_widgetBorder as well as at teardown: re-injection builds a
// fresh Border and would otherwise overwrite the tokens, stranding live delegates that
// point into this DLL on the discarded one.
void DetachHoverTracking() {
    if (g_widgetBorder) {
        try {
            if (g_pointerEnteredToken.value) {
                g_widgetBorder.PointerEntered(g_pointerEnteredToken);
            }
            if (g_pointerExitedToken.value) {
                g_widgetBorder.PointerExited(g_pointerExitedToken);
            }
        } catch (...) {
        }
    }
    g_pointerEnteredToken = {};
    g_pointerExitedToken = {};
    g_widgetHovered = false;
}

void RemoveWidget() {
    if (g_timer) {
        g_timer.Stop();
        g_timer.Tick(g_timerToken);
        g_timer = nullptr;
        g_timerToken = {};
    }
    DetachAnchorTracking();

    // Setting g_widget/g_injectionParent to nullptr only drops *our* reference - the
    // widget's parent panel in the live taskbar tree holds its own, so without this it
    // stays visible (and its event handlers keep pointing into this DLL) after unload.
    if (g_injectionParent) {
        try {
            int removedCol = RemoveWidgetFromPanel(g_injectionParent);
            if (auto oldGrid = g_injectionParent.try_as<Grid>()) {
                if (g_widgetInsertedColumn && removedCol >= 0 &&
                    removedCol < static_cast<int>(oldGrid.ColumnDefinitions().Size())) {
                    for (uint32_t i = 0; i < oldGrid.Children().Size(); ++i) {
                        auto child = oldGrid.Children().GetAt(i).try_as<FrameworkElement>();
                        if (child) {
                            int childCol = Grid::GetColumn(child);
                            if (childCol > removedCol) Grid::SetColumn(child, childCol - 1);
                        }
                    }
                    oldGrid.ColumnDefinitions().RemoveAt(removedCol);
                }
            }
        } catch (...) {
        }
    }

    DetachHoverTracking();

    g_widget = nullptr;
    g_widgetBorder = nullptr;
    g_injectionParent = nullptr;
    g_lastInjectedPosition.clear();
    g_widgetColumn = -1;
    g_widgetInsertedColumn = false;
    g_cpuLabel = nullptr;
    g_cpuUsageText = nullptr;
    g_cpuTempText = nullptr;
    g_cpuClockText = nullptr;
    g_cpuPowerText = nullptr;
    g_gpuLabel = nullptr;
    g_gpuUsageText = nullptr;
    g_gpuTempText = nullptr;
    g_gpuClockText = nullptr;
    g_gpuPowerText = nullptr;
    g_ramLabel = nullptr;
    g_ramPercentText = nullptr;
    g_ramCapacityText = nullptr;
    g_vramLabel = nullptr;
    g_vramPercentText = nullptr;
    g_vramCapacityText = nullptr;
    g_netDownText = nullptr;
    g_netUpText = nullptr;
    g_netDownArrow = nullptr;
    g_netUpArrow = nullptr;
    g_netDot = nullptr;
    g_cpuGraph = nullptr;
    g_gpuGraph = nullptr;
    g_cpuGraphBackground = nullptr;
    g_gpuGraphBackground = nullptr;
    g_dividerNet = nullptr;
    g_dividerPanels = nullptr;
    g_ramTrack = nullptr;
    g_ramFill = nullptr;
    g_vramTrack = nullptr;
    g_vramFill = nullptr;
    g_leftColumn = nullptr;
    g_gapColumn = nullptr;
    g_rightColumn = nullptr;
    g_cpuGraphArea = nullptr;
    g_gpuGraphArea = nullptr;
    g_netGapColumn = nullptr;
    g_netColumnDef = nullptr;
    g_leftGapRow = nullptr;
    g_rightGapRow = nullptr;
    g_netGapRow = nullptr;
    g_cpuRowDef = nullptr;
    g_gpuRowDef = nullptr;
    g_ramRowDef = nullptr;
    g_vramRowDef = nullptr;
    g_leftPanel = nullptr;
    g_rightPanel = nullptr;
    g_netColumn = nullptr;
    g_cpuHistory.clear();
    g_gpuHistory.clear();
    g_lastRenderedMetricsSequence = 0;
    g_cpuTemperatureAlert = AlertLevel::Normal;
    g_gpuTemperatureAlert = AlertLevel::Normal;
    g_ramAlert = AlertLevel::Normal;
    g_vramAlert = AlertLevel::Normal;
}

// A thin vertical seam between column groups. The gradient fades to transparent at
// both ends rather than using a flat fill, so it reads as a soft "frosted" edge instead
// of a hard line touching the row boundaries top and bottom.
XamlRectangle CreateColumnDivider(PCWSTR name) {
    XamlRectangle divider;
    divider.Name(name);
    divider.Width(1.0);
    divider.HorizontalAlignment(HorizontalAlignment::Center);
    divider.VerticalAlignment(VerticalAlignment::Stretch);
    divider.IsHitTestVisible(false);

    LinearGradientBrush brush;
    brush.StartPoint(Point{0, 0});
    brush.EndPoint(Point{0, 1});
    auto stops = brush.GradientStops();
    GradientStop stopStart;
    stopStart.Offset(0.0);
    stopStart.Color(MakeColor(0x00, 0xFF, 0xFF, 0xFF));
    GradientStop stopMid;
    stopMid.Offset(0.5);
    stopMid.Color(MakeColor(0x38, 0xFF, 0xFF, 0xFF));
    GradientStop stopEnd;
    stopEnd.Offset(1.0);
    stopEnd.Color(MakeColor(0x00, 0xFF, 0xFF, 0xFF));
    stops.Append(stopStart);
    stops.Append(stopMid);
    stops.Append(stopEnd);
    divider.Fill(brush);
    return divider;
}

Border BuildWidgetGrid() {
    Grid widget;
    widget.IsHitTestVisible(false);
    widget.VerticalAlignment(VerticalAlignment::Center);

    // Column order left to right: network/status, CPU+GPU, RAM+VRAM - the network
    // column comes first per the user's own taskbar-clock-customization-v3 layout.
    g_netColumnDef = AutoColumn();
    g_netGapColumn = ColumnDefinition();
    g_leftColumn = AutoColumn();
    g_gapColumn = ColumnDefinition();
    g_rightColumn = AutoColumn();
    widget.ColumnDefinitions().Append(g_netColumnDef);
    widget.ColumnDefinitions().Append(g_netGapColumn);
    widget.ColumnDefinitions().Append(g_leftColumn);
    widget.ColumnDefinitions().Append(g_gapColumn);
    widget.ColumnDefinitions().Append(g_rightColumn);

    Grid leftPanel;
    leftPanel.IsHitTestVisible(false);
    g_leftGapRow = PixelRow(2.0);
    g_cpuRowDef = PixelRow(kRowHeight);
    g_gpuRowDef = PixelRow(kRowHeight);
    leftPanel.RowDefinitions().Append(g_cpuRowDef);
    leftPanel.RowDefinitions().Append(g_leftGapRow);
    leftPanel.RowDefinitions().Append(g_gpuRowDef);
    for (int i = 0; i < 6; i++) leftPanel.ColumnDefinitions().Append(AutoColumn());
    AddComputeRow(leftPanel, 0, L"CPU", L"Cpu", g_cpuLabel, g_cpuUsageText,
                  g_cpuTempText, g_cpuClockText, g_cpuPowerText, g_cpuGraph,
                  g_cpuGraphArea, g_cpuGraphBackground);
    AddComputeRow(leftPanel, 2, L"GPU", L"Gpu", g_gpuLabel, g_gpuUsageText,
                  g_gpuTempText, g_gpuClockText, g_gpuPowerText, g_gpuGraph,
                  g_gpuGraphArea, g_gpuGraphBackground);

    Grid rightPanel;
    rightPanel.IsHitTestVisible(false);
    g_rightGapRow = PixelRow(2.0);
    g_ramRowDef = PixelRow(kRowHeight);
    g_vramRowDef = PixelRow(kRowHeight);
    rightPanel.RowDefinitions().Append(g_ramRowDef);
    rightPanel.RowDefinitions().Append(g_rightGapRow);
    rightPanel.RowDefinitions().Append(g_vramRowDef);
    for (int i = 0; i < 3; i++) rightPanel.ColumnDefinitions().Append(AutoColumn());
    AddMemoryRow(rightPanel, 0, L"RAM", L"Ram", g_ramLabel, g_ramPercentText,
                 g_ramCapacityText, g_ramTrack, g_ramFill);
    AddMemoryRow(rightPanel, 2, L"VRAM", L"Vram", g_vramLabel, g_vramPercentText,
                 g_vramCapacityText, g_vramTrack, g_vramFill);

    Grid netColumn = CreateNetworkColumn();
    g_leftPanel = leftPanel;
    g_rightPanel = rightPanel;
    g_netColumn = netColumn;

    // Thin translucent dividers in the gap columns - a visual seam between sections
    // rather than bare empty space, "frosted" via a soft vertical opacity taper at
    // each end so it doesn't read as a hard line touching the row edges.
    XamlRectangle dividerNet = CreateColumnDivider(L"DividerNet");
    XamlRectangle dividerPanels = CreateColumnDivider(L"DividerPanels");
    g_dividerNet = dividerNet;
    g_dividerPanels = dividerPanels;

    Grid::SetColumn(netColumn, 0);
    Grid::SetColumn(dividerNet, 1);
    Grid::SetColumn(leftPanel, 2);
    Grid::SetColumn(dividerPanels, 3);
    Grid::SetColumn(rightPanel, 4);
    widget.Children().Append(netColumn);
    widget.Children().Append(dividerNet);
    widget.Children().Append(leftPanel);
    widget.Children().Append(dividerPanels);
    widget.Children().Append(rightPanel);

    Border border;
    border.Name(kWidgetName);
    // The one hit-testable element in the widget, so hover-to-reveal works. Everything
    // inside stays click-through; the Border occupies its own reserved tray slot, so
    // there is nothing underneath it to swallow clicks from.
    border.IsHitTestVisible(true);
    border.VerticalAlignment(VerticalAlignment::Center);
    border.Child(widget);
    g_widget = widget;
    g_widgetBorder = border;
    g_widgetHovered = false;

    g_pointerEnteredToken = border.PointerEntered(
        [](IInspectable const&, winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const&) {
            g_widgetHovered = true;
            if (!g_unloading) ApplyBoxBackground(CurrentSettings());
        });
    g_pointerExitedToken = border.PointerExited(
        [](IInspectable const&, winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const&) {
            g_widgetHovered = false;
            if (!g_unloading) ApplyBoxBackground(CurrentSettings());
        });
    return border;
}

// ---------------------------------------------------------------------------
// Taskbar insertion. Ported from this author's taskbar-ai-quota-fork: a real child of
// the tray StackPanel/Grid or a tracked taskbar-button margin, never an overlay with a
// borrowed z-index - see the InjectionTarget comment below for why both a `column` and
// a `childIndex` exist.
// ---------------------------------------------------------------------------

bool IsOverlayPosition(const std::wstring& position) {
    return position == L"taskbar_left_edge" || position == L"taskbar_center_edge" ||
           position == L"taskbar_right_edge";
}

bool IsAnchoredPosition(const std::wstring& position) {
    return position == L"taskbar_left_start" || position == L"taskbar_right_start" ||
           position == L"taskbar_after_search_left" ||
           position == L"taskbar_after_search_right" ||
           position == L"taskbar_after_taskview_left" ||
           position == L"taskbar_after_taskview_right" ||
           position == L"taskbar_after_widgets_left" ||
           position == L"taskbar_after_widgets_right";
}

const wchar_t* const kStartButtonNames[] = {
    L"StartButton",
    L"StartMenuButton",
    L"StartMenuLaunchButton",
    L"LaunchListButton",
};

Grid FindTaskbarRootGrid(FrameworkElement const& root) {
    auto taskbarFrame = FindChildRecursive(root, [](FrameworkElement child) {
        return winrt::get_class_name(child) == L"Taskbar.TaskbarFrame";
    });
    if (!taskbarFrame) return nullptr;
    return FindDirectChildByName(taskbarFrame, L"RootGrid").try_as<Grid>();
}

FrameworkElement FindElementInRepeater(FrameworkElement const& repeater,
                                       const wchar_t* const* names,
                                       int nameCount) {
    if (!repeater) return nullptr;
    int childCount = VisualTreeHelper::GetChildrenCount(repeater);
    for (int i = 0; i < childCount; i++) {
        auto child = VisualTreeHelper::GetChild(repeater, i).try_as<FrameworkElement>();
        if (!child) continue;
        for (int j = 0; j < nameCount; j++) {
            if (child.Name() == names[j]) return child;
        }
    }
    for (int i = 0; i < childCount; i++) {
        auto child = VisualTreeHelper::GetChild(repeater, i).try_as<FrameworkElement>();
        if (!child) continue;
        int subCount = VisualTreeHelper::GetChildrenCount(child);
        for (int k = 0; k < subCount; k++) {
            auto subChild = VisualTreeHelper::GetChild(child, k).try_as<FrameworkElement>();
            if (!subChild) continue;
            for (int j = 0; j < nameCount; j++) {
                if (subChild.Name() == names[j]) return subChild;
            }
        }
    }
    return nullptr;
}

FrameworkElement FindChildByClassName(FrameworkElement const& parent,
                                      const wchar_t* className,
                                      int depth = 3) {
    if (!parent || depth < 0) return nullptr;
    int childCount = VisualTreeHelper::GetChildrenCount(parent);
    for (int i = 0; i < childCount; i++) {
        auto child = VisualTreeHelper::GetChild(parent, i).try_as<FrameworkElement>();
        if (!child) continue;
        if (winrt::get_class_name(child) == className) return child;
        if (auto found = FindChildByClassName(child, className, depth - 1)) return found;
    }
    return nullptr;
}

FrameworkElement FindNthElementByClassName(FrameworkElement const& parent,
                                           const wchar_t* className,
                                           int index) {
    if (!parent) return nullptr;
    int foundCount = 0;
    int childCount = VisualTreeHelper::GetChildrenCount(parent);
    for (int i = 0; i < childCount; i++) {
        auto child = VisualTreeHelper::GetChild(parent, i).try_as<FrameworkElement>();
        if (!child) continue;
        if (winrt::get_class_name(child) == className) {
            if (foundCount == index) return child;
            foundCount++;
        }
    }
    return nullptr;
}

// Maps a position name onto a concrete panel plus where in it to insert. Tray positions
// resolve against SystemTrayFrameGrid (branching on Grid vs the 26300+ StackPanel);
// taskbar positions resolve against the taskbar's RootGrid and are placed by margin.
InjectionTarget ResolveInjectionTarget(FrameworkElement const& root,
                                       const std::wstring& position) {
    if (!IsOverlayPosition(position) && !IsAnchoredPosition(position)) {
        auto trayFrame = FindDescendantByName(root, L"SystemTrayFrameGrid");
        auto trayPanel = trayFrame ? trayFrame.try_as<Panel>() : nullptr;
        if (!trayPanel) return {};

        const wchar_t* anchor = nullptr;
        bool after = false;
        bool farEnd = false;
        bool farEndIsRight = false;

        if (position == L"tray_right") { farEnd = true; farEndIsRight = true; }
        else if (position == L"tray_left") { farEnd = true; farEndIsRight = false; }
        else if (position == L"tray_before_clock") { anchor = L"NotificationCenterButton"; }
        else if (position == L"tray_after_clock") { anchor = L"ShowDesktopStack"; }
        else if (position == L"tray_before_omni_left") { anchor = L"ControlCenterButton"; }
        else if (position == L"tray_before_omni_right") {
            anchor = L"ControlCenterButton"; after = true;
        } else if (position == L"tray_language_left") { anchor = L"NonActivatableStack"; }
        else if (position == L"tray_language_right") {
            anchor = L"NonActivatableStack"; after = true;
        } else if (position == L"tray_icons_left") { anchor = L"NotificationAreaIcons"; }
        else if (position == L"tray_icons_right") {
            anchor = L"NotificationAreaIcons"; after = true;
        } else if (position == L"tray_hidden_icons_left") { anchor = L"NotifyIconStack"; }
        else if (position == L"tray_hidden_icons_right") {
            anchor = L"NotifyIconStack"; after = true;
        } else if (position == L"tray_after_showdesktop_left") { anchor = L"ShowDesktopStack"; }
        else if (position == L"tray_after_showdesktop_right") {
            anchor = L"ShowDesktopStack"; after = true;
        } else { farEnd = true; farEndIsRight = false; }

        // Windows 11 26H2 (build 26300+): a StackPanel, ordered by child index.
        if (!trayPanel.try_as<Grid>()) {
            int childCount = static_cast<int>(trayPanel.Children().Size());
            int index;
            if (farEnd) {
                index = farEndIsRight ? childCount : 0;
            } else {
                index = TrayChildIndex(trayPanel, anchor);
                if (index < 0) return {};
                if (after) index++;
            }
            InjectionTarget target;
            target.panel = trayPanel;
            target.childIndex = std::clamp(index, 0, childCount);
            return target;
        }

        // Pre-26300: a Grid, where each tray element owns a reserved column.
        auto trayGrid = trayPanel.as<Grid>();
        int columnCount = static_cast<int>(trayGrid.ColumnDefinitions().Size());
        int col;
        if (farEnd) {
            col = farEndIsRight ? columnCount : 0;
        } else {
            auto element = FindTrayElement(trayGrid, root, anchor);
            if (!element) {
                if (position != L"tray_after_showdesktop_right") return {};
                col = columnCount;
            } else {
                col = Grid::GetColumn(element) + (after ? 1 : 0);
            }
        }
        InjectionTarget target;
        target.panel = trayGrid;
        target.column = std::clamp(col, 0, columnCount);
        return target;
    }

    if (auto rootGrid = FindTaskbarRootGrid(root)) {
        InjectionTarget target;
        target.panel = rootGrid;
        return target;  // column/childIndex stay -1: placed by margin, not by slot.
    }

    // Taskbar frame not realised yet - fall back to the tray so the widget still appears.
    auto trayFrame = FindDescendantByName(root, L"SystemTrayFrameGrid");
    if (auto trayPanel = trayFrame ? trayFrame.try_as<Panel>() : nullptr) {
        InjectionTarget target;
        target.panel = trayPanel;
        if (auto trayGrid = trayPanel.try_as<Grid>()) {
            target.column = static_cast<int>(trayGrid.ColumnDefinitions().Size());
        } else {
            target.childIndex = static_cast<int>(trayPanel.Children().Size());
        }
        return target;
    }
    return {};
}

FrameworkElement ResolveAnchorElement(FrameworkElement const& root,
                                      const std::wstring& position,
                                      bool* anchorOnLeft) {
    auto rootGrid = FindTaskbarRootGrid(root);
    if (!rootGrid) return nullptr;
    auto repeater = FindDirectChildByName(rootGrid, L"TaskbarFrameRepeater");
    if (!repeater) return nullptr;

    *anchorOnLeft = position.size() > 5 &&
                   position.compare(position.size() - 5, 5, L"_left") == 0;
    if (position == L"taskbar_left_start") *anchorOnLeft = true;
    if (position == L"taskbar_right_start") *anchorOnLeft = false;

    if (position == L"taskbar_left_start" || position == L"taskbar_right_start") {
        return FindElementInRepeater(repeater, kStartButtonNames,
                                     ARRAYSIZE(kStartButtonNames));
    }
    if (position == L"taskbar_after_search_left" ||
        position == L"taskbar_after_search_right") {
        return FindChildByClassName(repeater, L"Taskbar.TaskbarExtensionElement", 3);
    }
    if (position == L"taskbar_after_taskview_left" ||
        position == L"taskbar_after_taskview_right") {
        return FindNthElementByClassName(repeater, L"Taskbar.ExperienceToggleButton", 1);
    }
    if (position == L"taskbar_after_widgets_left" ||
        position == L"taskbar_after_widgets_right") {
        auto widgets = FindDirectChildByName(repeater, L"AugmentedEntryPointButton");
        if (!widgets) {
            widgets = FindChildByClassName(repeater, L"Taskbar.AugmentedEntryPointButton", 3);
        }
        return widgets;
    }
    return nullptr;
}

// Anchored taskbar positions have no column to live in: the widget is positioned by
// margin, and the anchor button is pushed aside by the widget's own width. Both are
// recomputed on every layout pass since the taskbar recentres as buttons come and go.
void AttachAnchorTracking(Panel const& parent,
                          FrameworkElement const& anchor,
                          bool anchorOnLeft,
                          int leftMargin,
                          int rightMargin) {
    g_trackedElement = anchor;
    g_trackedOriginalMargin = anchor.Margin();
    g_hasTrackedOriginalMargin = true;
    g_trackAnchorOnLeft = anchorOnLeft;

    g_layoutUpdatedToken = parent.LayoutUpdated(
        [leftMargin, rightMargin](IInspectable const&, IInspectable const&) {
            if (g_unloading || !g_widgetBorder || !g_trackedElement) return;
            try {
                bool visible = g_widgetBorder.Visibility() == Visibility::Visible;
                double width = visible ? g_widgetBorder.ActualWidth() : 0.0;
                double gap = visible ? width + leftMargin + rightMargin : 0.0;

                auto margin = g_hasTrackedOriginalMargin ? g_trackedOriginalMargin
                                                         : g_trackedElement.Margin();
                auto current = g_trackedElement.Margin();
                if (g_trackAnchorOnLeft) {
                    if (std::abs(current.Left - gap) > 1.0) {
                        margin.Left = gap;
                        g_trackedElement.Margin(margin);
                    }
                } else if (std::abs(current.Right - gap) > 1.0) {
                    margin.Right = gap;
                    g_trackedElement.Margin(margin);
                }

                if (!visible || !g_injectionParent) return;
                auto point = g_trackedElement.TransformToVisual(g_injectionParent)
                                .TransformPoint({0, 0});
                double left = g_trackAnchorOnLeft
                                 ? point.X - gap + leftMargin
                                 : point.X + g_trackedElement.ActualWidth() + leftMargin;
                auto widgetMargin = g_widgetBorder.Margin();
                if (std::abs(widgetMargin.Left - left) > 1.0) {
                    g_widgetBorder.Margin(Thickness{left, 0, 0, 0});
                }
            } catch (...) {
                g_trackedElement = nullptr;
                g_hasTrackedOriginalMargin = false;
            }
        });
    g_hasLayoutUpdatedToken = true;
}

int RemoveWidgetFromPanel(Panel const& targetPanel) {
    if (!targetPanel) return -1;
    bool isGrid = targetPanel.try_as<Grid>() != nullptr;
    int firstCol = -1;
    for (int i = static_cast<int>(targetPanel.Children().Size()) - 1; i >= 0; --i) {
        auto fe = targetPanel.Children().GetAt(i).try_as<FrameworkElement>();
        if (fe && fe.Name() == kWidgetName) {
            if (isGrid && firstCol < 0) firstCol = Grid::GetColumn(fe);
            try {
                targetPanel.Children().RemoveAt(i);
            } catch (...) {
            }
        }
    }
    return firstCol;
}

using RunFromWindowThreadProc = bool (*)(void*);

// Dispatches `proc` onto `window`'s UI thread and returns what `proc` returned, not
// merely whether the dispatch happened - the watchdog's retry cadence depends on
// telling "no taskbar window yet" apart from "found it, EnsureWidget itself failed".
// SendMessageTimeout (rather than a plain SendMessage) avoids deadlocking this thread
// against a taskbar UI thread that has stopped pumping messages during unload.
bool RunFromWindowThread(HWND window,
                         RunFromWindowThreadProc proc,
                         void* param,
                         DWORD timeoutMs = 2000) {
    static const UINT message =
        RegisterWindowMessageW(L"Windhawk_RunFromWindowThread_" WH_MOD_ID);
    struct Payload {
        RunFromWindowThreadProc proc;
        void* param;
        std::atomic<bool> ran{false};
        std::atomic<bool> result{false};
        Payload(RunFromWindowThreadProc proc, void* param) : proc(proc), param(param) {}
    };
    using PayloadRef = std::shared_ptr<Payload>;

    DWORD threadId = GetWindowThreadProcessId(window, nullptr);
    if (!threadId) return false;
    if (threadId == GetCurrentThreadId()) return proc(param);

    HHOOK hook = SetWindowsHookExW(
        WH_CALLWNDPROC,
        [](int code, WPARAM wParam, LPARAM lParam) -> LRESULT {
            if (code == HC_ACTION) {
                const auto* messageData = reinterpret_cast<const CWPSTRUCT*>(lParam);
                static const UINT innerMessage =
                    RegisterWindowMessageW(L"Windhawk_RunFromWindowThread_" WH_MOD_ID);
                if (messageData->message == innerMessage) {
                    std::unique_ptr<PayloadRef> holder(
                        reinterpret_cast<PayloadRef*>(messageData->lParam));
                    PayloadRef payload = *holder;
                    payload->result.store(payload->proc(payload->param),
                                          std::memory_order_release);
                    payload->ran.store(true, std::memory_order_release);
                }
            }
            return CallNextHookEx(nullptr, code, wParam, lParam);
        },
        nullptr, threadId);
    if (!hook) {
        Wh_Log(L"SetWindowsHookEx failed for taskbar thread %u: %u", threadId,
               GetLastError());
        return false;
    }

    PayloadRef payload = std::make_shared<Payload>(proc, param);
    auto* holder = new PayloadRef(payload);
    bool sent;
    if (timeoutMs == INFINITE) {
        SendMessageW(window, message, 0, reinterpret_cast<LPARAM>(holder));
        sent = true;
    } else {
        DWORD_PTR ignored = 0;
        sent = SendMessageTimeoutW(
                  window, message, 0, reinterpret_cast<LPARAM>(holder),
                  SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, timeoutMs,
                  &ignored) != 0;
    }

    bool ran = payload->ran.load(std::memory_order_acquire);
    bool result = sent && ran && payload->result.load(std::memory_order_acquire);
    if (!sent) {
        // SendMessageTimeoutW gave up because the target thread looked hung. We unhook
        // immediately rather than wait around: if the thread later recovers and the
        // message is delivered after all, the hook is already gone and nothing frees
        // `holder` - a small, bounded one-shot leak (a shared_ptr control block plus a
        // Payload) that only happens on this hang path, traded for never blocking here.
        UnhookWindowsHookEx(hook);
        return false;
    }
    if (!ran) delete holder;
    UnhookWindowsHookEx(hook);
    if (!ran) Wh_Log(L"Taskbar thread dispatch failed for thread %u", threadId);
    return result;
}

HWND FindCurrentProcessTaskbarWindow() {
    HWND result = nullptr;
    EnumWindows(
        [](HWND window, LPARAM context) -> BOOL {
            DWORD processId = 0;
            WCHAR className[64];
            if (GetWindowThreadProcessId(window, &processId) &&
                processId == GetCurrentProcessId() &&
                GetClassNameW(window, className, std::size(className)) &&
                _wcsicmp(className, L"Shell_TrayWnd") == 0) {
                *reinterpret_cast<HWND*>(context) = window;
                return FALSE;
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&result));
    return result;
}

bool IsCurrentProcessTaskbarWindow(HWND window) {
    DWORD processId = 0;
    WCHAR className[64];
    return window && IsWindow(window) &&
           GetWindowThreadProcessId(window, &processId) != 0 &&
           processId == GetCurrentProcessId() &&
           GetClassNameW(window, className, std::size(className)) != 0 &&
           _wcsicmp(className, L"Shell_TrayWnd") == 0;
}

void RememberTaskbarWindow(HWND window) {
    if (!IsCurrentProcessTaskbarWindow(window)) return;
    DWORD threadId = GetWindowThreadProcessId(window, nullptr);
    if (threadId) {
        g_taskbarWindow = window;
        g_taskbarThreadId = threadId;
    }
}

HWND FindRememberedTaskbarWindow() {
    HWND rememberedWindow = g_taskbarWindow.load();
    if (IsCurrentProcessTaskbarWindow(rememberedWindow)) return rememberedWindow;

    DWORD rememberedThreadId = g_taskbarThreadId.load();
    if (rememberedThreadId) {
        HWND threadWindow = nullptr;
        EnumThreadWindows(
            rememberedThreadId,
            [](HWND window, LPARAM context) -> BOOL {
                WCHAR className[64];
                if (GetClassNameW(window, className, std::size(className)) &&
                    _wcsicmp(className, L"Shell_TrayWnd") == 0) {
                    *reinterpret_cast<HWND*>(context) = window;
                    return FALSE;
                }
                return TRUE;
            },
            reinterpret_cast<LPARAM>(&threadWindow));
        if (IsCurrentProcessTaskbarWindow(threadWindow)) {
            RememberTaskbarWindow(threadWindow);
            return threadWindow;
        }
    }

    HWND currentWindow = FindCurrentProcessTaskbarWindow();
    if (currentWindow) RememberTaskbarWindow(currentWindow);
    return currentWindow;
}

HWND FindAnyWindowOnTaskbarThread(HWND excludedWindow) {
    DWORD threadId = g_taskbarThreadId.load();
    if (!threadId) return nullptr;

    struct SearchContext {
        HWND excludedWindow;
        HWND result;
    } context{excludedWindow, nullptr};
    EnumThreadWindows(
        threadId,
        [](HWND window, LPARAM contextValue) -> BOOL {
            auto* ctx = reinterpret_cast<SearchContext*>(contextValue);
            DWORD processId = 0;
            if (window != ctx->excludedWindow &&
                GetWindowThreadProcessId(window, &processId) != 0 &&
                processId == GetCurrentProcessId()) {
                ctx->result = window;
                return FALSE;
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&context));
    return context.result;
}

using CTaskBand_GetTaskbarHost_t = void*(WINAPI*)(void* pThis, void* taskbarHostSharedPtr);
CTaskBand_GetTaskbarHost_t CTaskBand_GetTaskbarHost_Original = nullptr;

using TaskbarHost_FrameHeight_t = int(WINAPI*)(void* pThis);
TaskbarHost_FrameHeight_t TaskbarHost_FrameHeight_Original = nullptr;

using RefCountBase_Decref_t = void(WINAPI*)(void* pThis);
RefCountBase_Decref_t RefCountBase_Decref_Original = nullptr;

void* CTaskBand_ITaskListWndSite_vftable = nullptr;

XamlRoot GetTaskbarXamlRoot(HWND taskbarWindow) {
    if (!CTaskBand_GetTaskbarHost_Original || !TaskbarHost_FrameHeight_Original ||
        !RefCountBase_Decref_Original || !CTaskBand_ITaskListWndSite_vftable) {
        return nullptr;
    }

    HWND taskBandWindow = reinterpret_cast<HWND>(GetPropW(taskbarWindow, L"TaskbandHWND"));
    if (!taskBandWindow) return nullptr;

    void* taskBand = reinterpret_cast<void*>(GetWindowLongPtrW(taskBandWindow, 0));
    if (!taskBand) return nullptr;

    void* taskBandForSite = taskBand;
    for (int i = 0;
        *reinterpret_cast<void**>(taskBandForSite) != CTaskBand_ITaskListWndSite_vftable;
        i++) {
        if (i == 20) return nullptr;
        taskBandForSite = reinterpret_cast<void**>(taskBandForSite) + 1;
    }

    void* taskbarHostSharedPtr[2]{};
    CTaskBand_GetTaskbarHost_Original(taskBandForSite, taskbarHostSharedPtr);
    if (!taskbarHostSharedPtr[0] || !taskbarHostSharedPtr[1]) {
        if (taskbarHostSharedPtr[1]) RefCountBase_Decref_Original(taskbarHostSharedPtr[1]);
        return nullptr;
    }

    size_t elementOffset = 0;
#if defined(_M_X64)
    const BYTE* code = reinterpret_cast<const BYTE*>(TaskbarHost_FrameHeight_Original);
    if (code[0] == 0x48 && code[1] == 0x83 && code[2] == 0xEC && code[4] == 0x48 &&
        code[5] == 0x83 && code[6] == 0xC1 && code[7] <= 0x7F) {
        elementOffset = code[7];
    } else {
        Wh_Log(L"Unsupported TaskbarHost::FrameHeight pattern");
        RefCountBase_Decref_Original(taskbarHostSharedPtr[1]);
        return nullptr;
    }
#elif defined(_M_ARM64)
    const DWORD* code = reinterpret_cast<const DWORD*>(TaskbarHost_FrameHeight_Original);
    if (code[0] == 0xD503237F && (code[1] & 0xFFC07FFF) == 0xA9807BFD &&
        code[2] == 0x910003FD && (code[3] & 0xFFF00FE0) == 0xF8400C00) {
        elementOffset = (code[3] >> 12) & 0xFF;
    } else {
        Wh_Log(L"Unsupported TaskbarHost::FrameHeight pattern");
        RefCountBase_Decref_Original(taskbarHostSharedPtr[1]);
        return nullptr;
    }
#else
#error "Unsupported architecture"
#endif

    auto* elementUnknown = *reinterpret_cast<::IUnknown**>(
        static_cast<BYTE*>(taskbarHostSharedPtr[0]) + elementOffset);
    if (!elementUnknown) {
        RefCountBase_Decref_Original(taskbarHostSharedPtr[1]);
        return nullptr;
    }

    FrameworkElement taskbarElement = nullptr;
    elementUnknown->QueryInterface(winrt::guid_of<FrameworkElement>(),
                                   winrt::put_abi(taskbarElement));
    XamlRoot result = taskbarElement ? taskbarElement.XamlRoot() : nullptr;
    RefCountBase_Decref_Original(taskbarHostSharedPtr[1]);
    return result;
}

bool StartMetricsWorker();

bool InjectWidget(HWND taskbarWindow) {
    if (!taskbarWindow || g_unloading) return false;

    auto fail = [&](PCWSTR reason) {
        ULONGLONG now = GetTickCount64();
        ULONGLONG nextLogMs = g_nextInjectFailureLogMs.load(std::memory_order_acquire);
        if (now >= nextLogMs &&
            g_nextInjectFailureLogMs.compare_exchange_strong(
                nextLogMs, now + 5000, std::memory_order_acq_rel)) {
            Wh_Log(L"InjectWidget failed: %s", reason);
        }
        return false;
    };

    try {
        XamlRoot xamlRoot = GetTaskbarXamlRoot(taskbarWindow);
        if (!xamlRoot) return fail(L"no XamlRoot");
        auto root = xamlRoot.Content().try_as<FrameworkElement>();
        if (!root) return fail(L"no XamlRoot content");

        std::wstring position;
        int leftMargin, rightMargin;
        {
            std::lock_guard lock(g_settingsMutex);
            position = g_settings.position;
            leftMargin = g_settings.leftMargin;
            rightMargin = g_settings.rightMargin;
        }

        InjectionTarget target = ResolveInjectionTarget(root, position);
        if (!target.panel) return fail(L"position target not in the visual tree yet");
        auto targetGrid = target.panel.try_as<Grid>();

        // Clean up wherever the widget currently lives first - the position setting may
        // have moved it to a different panel entirely (tray -> taskbar RootGrid), and
        // removing only from `target.panel` would leave the old copy stranded there.
        if (g_injectionParent) {
            int removedCol = RemoveWidgetFromPanel(g_injectionParent);
            if (auto oldGrid = g_injectionParent.try_as<Grid>()) {
                if (g_widgetInsertedColumn && removedCol >= 0 &&
                    removedCol < static_cast<int>(oldGrid.ColumnDefinitions().Size())) {
                    for (uint32_t i = 0; i < oldGrid.Children().Size(); ++i) {
                        auto child = oldGrid.Children().GetAt(i).try_as<FrameworkElement>();
                        if (child) {
                            int childCol = Grid::GetColumn(child);
                            if (childCol > removedCol) Grid::SetColumn(child, childCol - 1);
                        }
                    }
                    oldGrid.ColumnDefinitions().RemoveAt(removedCol);
                }
            }
        }
        g_widgetInsertedColumn = false;
        g_widgetColumn = -1;
        // Defensive sweep of the new target too, in case a stale instance is already
        // sitting there from an injection that never got torn down.
        RemoveWidgetFromPanel(target.panel);

        // Revoke the previous injection's handlers before BuildWidgetGrid overwrites
        // g_widgetBorder / the anchor-tracking globals - a stale delegate left live on
        // the discarded element would keep firing against the new state below.
        DetachHoverTracking();
        DetachAnchorTracking();

        Border widget = BuildWidgetGrid();
        g_injectionParent = target.panel;

        bool overlay = IsOverlayPosition(position);
        bool anchored = IsAnchoredPosition(position);

        if (!overlay && !anchored && !targetGrid) {
            // 26300+ StackPanel tray. Re-resolve first: target.childIndex was computed
            // before RemoveWidgetFromPanel ran, so if our previous element sat left of
            // the anchor, every index after it has since shifted down by one.
            InjectionTarget fresh = ResolveInjectionTarget(root, position);
            int insertAt = (fresh.panel == target.panel && fresh.childIndex >= 0)
                              ? fresh.childIndex
                              : target.childIndex;
            insertAt =
                std::clamp(insertAt, 0, static_cast<int>(target.panel.Children().Size()));

            widget.Margin(Thickness{static_cast<double>(leftMargin), 0,
                                    static_cast<double>(rightMargin), 0});
            target.panel.Children().InsertAt(static_cast<uint32_t>(insertAt), widget);
        } else if (!overlay && !anchored) {
            ColumnDefinition newCol;
            newCol.Width(GridLength{1.0, GridUnitType::Auto});
            int columnCount = static_cast<int>(targetGrid.ColumnDefinitions().Size());
            int insertCol = std::clamp(target.column, 0, columnCount);
            if (insertCol >= columnCount) {
                targetGrid.ColumnDefinitions().Append(newCol);
            } else {
                targetGrid.ColumnDefinitions().InsertAt(insertCol, newCol);
                for (uint32_t i = 0; i < targetGrid.Children().Size(); ++i) {
                    auto child = targetGrid.Children().GetAt(i).try_as<FrameworkElement>();
                    if (child) {
                        int childCol = Grid::GetColumn(child);
                        if (childCol >= insertCol) Grid::SetColumn(child, childCol + 1);
                    }
                }
            }
            widget.Margin(Thickness{static_cast<double>(leftMargin), 0,
                                    static_cast<double>(rightMargin), 0});
            Grid::SetColumn(widget, insertCol);
            targetGrid.Children().Append(widget);
            g_widgetColumn = insertCol;
            g_widgetInsertedColumn = true;
        } else {
            widget.HorizontalAlignment(HorizontalAlignment::Left);
            if (position == L"taskbar_center_edge") {
                widget.HorizontalAlignment(HorizontalAlignment::Center);
                widget.Margin(Thickness{static_cast<double>(leftMargin), 0,
                                        static_cast<double>(rightMargin), 0});
            } else if (position == L"taskbar_right_edge") {
                widget.HorizontalAlignment(HorizontalAlignment::Right);
                double right = rightMargin;
                if (auto trayFrame = FindDescendantByName(root, L"SystemTrayFrameGrid")) {
                    right += trayFrame.ActualWidth() + 4;
                }
                widget.Margin(Thickness{static_cast<double>(leftMargin), 0, right, 0});
            } else {
                widget.Margin(Thickness{static_cast<double>(leftMargin), 0,
                                        static_cast<double>(rightMargin), 0});
            }
            Grid::SetColumn(widget, 0);
            Canvas::SetZIndex(widget, 1000);
            target.panel.Children().Append(widget);

            if (anchored) {
                bool anchorOnLeft = false;
                if (auto anchor = ResolveAnchorElement(root, position, &anchorOnLeft)) {
                    AttachAnchorTracking(target.panel, anchor, anchorOnLeft, leftMargin,
                                        rightMargin);
                }
            }
        }

        g_lastInjectedPosition = position;
        ApplyWidgetSettings();
        if (!StartMetricsWorker()) Wh_Log(L"Metrics worker unavailable");
        EnsureTimer();
        UpdateWidgetText();
        if (CurrentSettings().verboseLogging) {
            Wh_Log(L"Injected Taskbar System Info - Fork at position %s", position.c_str());
        }
        return true;
    } catch (...) {
        HRESULT error = winrt::to_hresult();
        Wh_Log(L"InjectWidget: exception %08X", static_cast<unsigned>(error));
        return false;
    }
}

bool IsWidgetLive(FrameworkElement const& root, const std::wstring& position) {
    if (!g_widgetBorder || !g_injectionParent) return false;
    if (position != g_lastInjectedPosition) return false;
    try {
        // g_widget's parent is always g_widgetBorder now, so that check would be
        // vacuous - g_widgetBorder's parent is the actual target panel.
        if (!VisualTreeHelper::GetParent(g_widgetBorder)) return false;
        InjectionTarget current = ResolveInjectionTarget(root, position);
        return current.panel && current.panel == g_injectionParent;
    } catch (...) {
        return false;
    }
}

bool EnsureWidget(HWND taskbarWindow) {
    if (!taskbarWindow || g_unloading) return false;
    try {
        XamlRoot xamlRoot = GetTaskbarXamlRoot(taskbarWindow);
        if (!xamlRoot) return false;
        auto root = xamlRoot.Content().try_as<FrameworkElement>();
        if (!root) return false;

        std::wstring position = CurrentSettings().position;
        if (IsWidgetLive(root, position)) {
            ApplyWidgetSettings();
            if (!StartMetricsWorker()) Wh_Log(L"Metrics worker unavailable");
            EnsureTimer();
            UpdateWidgetText();
            return true;
        }
    } catch (...) {
    }
    return InjectWidget(taskbarWindow);
}

bool RemoveWidgetFromCurrentTaskbar(void*) {
    try {
        RemoveWidget();
    } catch (...) {
        HRESULT error = winrt::to_hresult();
        Wh_Log(L"Removing widget failed: %08X", static_cast<unsigned>(error));
    }
    try {
        g_loadedRevokers.reset();
    } catch (...) {
    }
    g_taskbarWindow = nullptr;
    g_taskbarThreadId = 0;
    return true;
}

bool TearDownTaskbarUi() {
    HWND taskbarWindow = FindRememberedTaskbarWindow();
    if (taskbarWindow &&
        RunFromWindowThread(taskbarWindow, RemoveWidgetFromCurrentTaskbar, nullptr)) {
        return true;
    }
    // Shell_TrayWnd can disappear during Explorer teardown; any surviving window on its
    // UI thread is enough - the WH_CALLWNDPROC hook runs the callback, SendMessage only
    // wakes the thread.
    HWND fallbackWindow = FindAnyWindowOnTaskbarThread(taskbarWindow);
    return fallbackWindow &&
           RunFromWindowThread(fallbackWindow, RemoveWidgetFromCurrentTaskbar, nullptr);
}

PCWSTR MetricProviderName(MetricProvider provider) {
    switch (provider) {
        case MetricProvider::HwInfoSharedMemory: return L"HWiNFO Shared Memory";
        case MetricProvider::HwInfoGadgetRegistry: return L"HWiNFO Gadget Registry";
        case MetricProvider::LibreHardwareMonitor: return L"LibreHardwareMonitor";
        case MetricProvider::WindowsD3dkmt: return L"Windows D3DKMT";
        case MetricProvider::WindowsThermalZones: return L"Windows thermal zones";
        case MetricProvider::WindowsPowerInformation: return L"NtPowerInformation";
        case MetricProvider::None:
        default: return L"unavailable";
    }
}

void PublishMetrics(MetricsSnapshot snapshot) {
    std::lock_guard lock(g_metricsMutex);
    g_latestMetrics = std::move(snapshot);
    g_latestMetricsSequence++;
    g_latestMetricsAvailable = true;
}

bool GetLatestMetrics(MetricsSnapshot& snapshot, uint64_t& sequence) {
    std::lock_guard lock(g_metricsMutex);
    if (!g_latestMetricsAvailable) return false;
    snapshot = g_latestMetrics;
    sequence = g_latestMetricsSequence;
    return true;
}

void MetricsWorkerProc() {
    ReadCpuUsage();
    EnsurePdhQuery(CurrentSettings());

    bool firstSample = true;
    bool providersLogged = false;
    MetricProvider lastCpuTempProvider = MetricProvider::None;
    MetricProvider lastGpuTempProvider = MetricProvider::None;
    while (!g_stopMetricsWorker) {
        ModSettings settings = CurrentSettings();
        DWORD waitMilliseconds =
            firstSample ? 250 : static_cast<DWORD>(settings.updateInterval) * 1000;
        DWORD waitResult =
            WaitForSingleObject(g_metricsWorkerWakeEvent, waitMilliseconds);
        firstSample = false;

        if (g_stopMetricsWorker) break;
        if (waitResult == WAIT_FAILED) {
            Wh_Log(L"Metrics worker wait failed: %u", GetLastError());
            break;
        }

        settings = CurrentSettings();
        MetricsSnapshot snapshot = CollectMetrics(settings);
        if (settings.verboseLogging &&
            (!providersLogged || snapshot.cpuTempProvider != lastCpuTempProvider ||
             snapshot.gpuTempProvider != lastGpuTempProvider)) {
            Wh_Log(L"Temperature providers: CPU=%s, GPU=%s",
                   MetricProviderName(snapshot.cpuTempProvider),
                   MetricProviderName(snapshot.gpuTempProvider));
            lastCpuTempProvider = snapshot.cpuTempProvider;
            lastGpuTempProvider = snapshot.gpuTempProvider;
            providersLogged = true;
        }
        PublishMetrics(std::move(snapshot));
    }

    CloseMetricSources();
}

bool StartMetricsWorker() {
    std::lock_guard lock(g_metricsWorkerMutex);
    if (g_metricsWorker) return true;
    if (g_unloading) return false;

    g_metricsWorkerWakeEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!g_metricsWorkerWakeEvent) {
        Wh_Log(L"Creating metrics worker event failed: %u", GetLastError());
        return false;
    }
    {
        std::lock_guard metricsLock(g_metricsMutex);
        g_latestMetrics = {};
        g_latestMetricsSequence = 0;
        g_latestMetricsAvailable = false;
    }
    g_stopMetricsWorker = false;
    try {
        g_metricsWorker.emplace(MetricsWorkerProc);
    } catch (...) {
        CloseHandle(g_metricsWorkerWakeEvent);
        g_metricsWorkerWakeEvent = nullptr;
        Wh_Log(L"Starting metrics worker failed");
        return false;
    }
    return true;
}

void WakeMetricsWorker() {
    std::lock_guard lock(g_metricsWorkerMutex);
    if (g_metricsWorkerWakeEvent) SetEvent(g_metricsWorkerWakeEvent);
}

void StopMetricsWorker() {
    std::lock_guard lock(g_metricsWorkerMutex);
    g_stopMetricsWorker = true;
    if (g_metricsWorkerWakeEvent) SetEvent(g_metricsWorkerWakeEvent);
    if (g_metricsWorker) {
        if (g_metricsWorker->joinable()) g_metricsWorker->join();
        g_metricsWorker.reset();
    }
    if (g_metricsWorkerWakeEvent) {
        CloseHandle(g_metricsWorkerWakeEvent);
        g_metricsWorkerWakeEvent = nullptr;
    }
    std::lock_guard metricsLock(g_metricsMutex);
    g_latestMetrics = {};
    g_latestMetricsSequence = 0;
    g_latestMetricsAvailable = false;
}

// ---------------------------------------------------------------------------
// Internet status. Ported from taskbar-clock-customization-v3's NetStatus thread:
// ICMP ping against a primary host, falling back to a secondary host only when the
// primary does not answer.
// ---------------------------------------------------------------------------

bool PingHost(PCWSTR hostW, int timeoutMs) {
    HANDLE hIcmp = IcmpCreateFile();
    if (hIcmp == INVALID_HANDLE_VALUE) return false;

    ADDRINFOW hints{};
    PADDRINFOW result = nullptr;
    hints.ai_family = AF_INET;
    if (GetAddrInfoW(hostW, nullptr, &hints, &result) != 0) {
        IcmpCloseHandle(hIcmp);
        return false;
    }

    auto* addr = reinterpret_cast<sockaddr_in*>(result->ai_addr);
    IPAddr destIp = addr->sin_addr.s_addr;
    FreeAddrInfoW(result);

    char sendData[] = "TaskbarSystemInfoFork";
    // +8 bytes per the ICMP_ECHO_REPLY32 alignment note in the Windows ICMP API docs.
    DWORD replySize = sizeof(ICMP_ECHO_REPLY) + sizeof(sendData) + 8;
    std::vector<uint8_t> replyBuffer(replySize);

    DWORD replies =
        IcmpSendEcho(hIcmp, destIp, sendData, sizeof(sendData), nullptr,
                    replyBuffer.data(), replySize, static_cast<DWORD>(timeoutMs));
    bool success = false;
    if (replies > 0) {
        auto* reply = reinterpret_cast<PICMP_ECHO_REPLY>(replyBuffer.data());
        success = reply->Status == IP_SUCCESS;
    }
    IcmpCloseHandle(hIcmp);
    return success;
}

void InternetWorkerProc() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        Wh_Log(L"NetStatus: WSAStartup failed");
        return;
    }

    while (!g_stopInternetWorker) {
        ModSettings settings = CurrentSettings();
        if (!settings.showInternetStatus) {
            DWORD wait = WaitForSingleObject(g_internetWorkerWakeEvent, 2000);
            if (wait == WAIT_FAILED) break;
            continue;
        }

        bool primaryOk = PingHost(settings.primaryHost.c_str(), settings.timeoutMs);
        bool connected = primaryOk;
        if (!primaryOk && !g_stopInternetWorker) {
            connected = PingHost(settings.secondaryHost.c_str(), settings.timeoutMs);
        }

        InternetState previous = g_internetState.exchange(
            connected ? InternetState::Connected : InternetState::Disconnected);
        if (settings.verboseLogging &&
            previous != (connected ? InternetState::Connected
                                   : InternetState::Disconnected)) {
            Wh_Log(L"NetStatus: %s", connected ? L"connected" : L"disconnected");
        }

        DWORD waitMs = static_cast<DWORD>(std::max(2, settings.checkIntervalSeconds)) * 1000;
        DWORD waitResult = WaitForSingleObject(g_internetWorkerWakeEvent, waitMs);
        if (waitResult == WAIT_FAILED) break;
    }

    WSACleanup();
}

bool StartInternetWorker() {
    if (g_internetWorker) return true;
    if (g_unloading) return false;

    g_internetWorkerWakeEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!g_internetWorkerWakeEvent) return false;
    g_stopInternetWorker = false;
    g_internetState = InternetState::Unknown;
    try {
        g_internetWorker.emplace(InternetWorkerProc);
    } catch (...) {
        CloseHandle(g_internetWorkerWakeEvent);
        g_internetWorkerWakeEvent = nullptr;
        return false;
    }
    return true;
}

void StopInternetWorker() {
    g_stopInternetWorker = true;
    if (g_internetWorkerWakeEvent) SetEvent(g_internetWorkerWakeEvent);
    if (g_internetWorker) {
        if (g_internetWorker->joinable()) g_internetWorker->join();
        g_internetWorker.reset();
    }
    if (g_internetWorkerWakeEvent) {
        CloseHandle(g_internetWorkerWakeEvent);
        g_internetWorkerWakeEvent = nullptr;
    }
    g_internetState = InternetState::Unknown;
}

void CloseMetricSources() {
    ClosePdhQuery();
    InvalidateGpuAdapterCache();
    g_nextPdhCounterRetry = {};
    g_nextPdhRecovery = {};
    g_consecutivePdhReadFailures = 0;
}

// A permanent watchdog rather than a bounded retry loop: Explorer rebuilds the tray's
// visual tree for reasons that never reach TaskbarFrame::Loaded - tray icon churn, DPI
// and monitor changes, a tray host restart - and drops the widget when it does. Same
// pattern as taskbar-ai-quota-fork's RetryInjectThreadProc.
DWORD WINAPI WatchdogThreadProc(LPVOID) {
    while (!g_unloading) {
        HWND taskbarWindow = FindCurrentProcessTaskbarWindow();
        bool injected = false;
        if (taskbarWindow) {
            injected = RunFromWindowThread(
                taskbarWindow,
                [](void* param) -> bool {
                    return !g_unloading && EnsureWidget(static_cast<HWND>(param));
                },
                taskbarWindow);
            RememberTaskbarWindow(taskbarWindow);
        }

        DWORD waitMs = injected ? 3000 : 250;
        if (g_watchdogWakeEvent) {
            if (WaitForSingleObject(g_watchdogWakeEvent, waitMs) == WAIT_FAILED) break;
        } else {
            Sleep(waitMs);
        }
        if (g_stopWatchdog) break;
    }
    return 0;
}

void StartWatchdog() {
    if (g_unloading) return;
    if (g_watchdogThreadHandle) {
        if (WaitForSingleObject(g_watchdogThreadHandle, 0) == WAIT_TIMEOUT) {
            // Still running - nudge it so a settings change applies immediately
            // instead of waiting out the heartbeat.
            if (g_watchdogWakeEvent) SetEvent(g_watchdogWakeEvent);
            return;
        }
        CloseHandle(g_watchdogThreadHandle);
        g_watchdogThreadHandle = nullptr;
    }
    if (!g_watchdogWakeEvent) {
        g_watchdogWakeEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    }
    g_stopWatchdog = false;
    g_watchdogThreadHandle =
        CreateThread(nullptr, 0, WatchdogThreadProc, nullptr, 0, nullptr);
    if (!g_watchdogThreadHandle) {
        Wh_Log(L"CreateThread WatchdogThreadProc failed: %lu", GetLastError());
    }
}

void StopWatchdogThread() {
    g_stopWatchdog = true;
    if (g_watchdogWakeEvent) SetEvent(g_watchdogWakeEvent);
    if (g_watchdogThreadHandle) {
        // Must actually wait for exit, not just give it a chance: the DLL is unmapped
        // shortly after Wh_ModUninit returns, and a bounded wait that gives up while
        // the thread is still mid-iteration (blocked inside RunFromWindowThread's own
        // up-to-2000ms SendMessageTimeoutW call) would let it resume executing code
        // that no longer exists. That inner call is itself bounded, so this converges
        // quickly in practice - it does not risk a real deadlock.
        WaitForSingleObject(g_watchdogThreadHandle, INFINITE);
        CloseHandle(g_watchdogThreadHandle);
        g_watchdogThreadHandle = nullptr;
    }
    if (g_watchdogWakeEvent) {
        CloseHandle(g_watchdogWakeEvent);
        g_watchdogWakeEvent = nullptr;
    }
}

using TrayUI_StartTaskbar_t = void(WINAPI*)(void*);
TrayUI_StartTaskbar_t TrayUI_StartTaskbar_Original = nullptr;

void WINAPI TrayUI_StartTaskbar_Hook(void* pThis) {
    TrayUI_StartTaskbar_Original(pThis);
    if (g_unloading) return;
    StartWatchdog();
}

bool HookTaskbarDllSymbols() {
    HMODULE module =
        LoadLibraryExW(L"taskbar.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!module) return false;

    WindhawkUtils::SYMBOL_HOOK taskbarDllHooks[] = {
        {{LR"(const CTaskBand::`vftable'{for `ITaskListWndSite'})"},
         &CTaskBand_ITaskListWndSite_vftable},
        {{LR"(public: virtual class std::shared_ptr<class TaskbarHost> __cdecl CTaskBand::GetTaskbarHost(void)const )"},
         &CTaskBand_GetTaskbarHost_Original},
        {{LR"(public: int __cdecl TaskbarHost::FrameHeight(void)const )"},
         &TaskbarHost_FrameHeight_Original},
        {{LR"(public: void __cdecl std::_Ref_count_base::_Decref(void))"},
         &RefCountBase_Decref_Original},
        {{LR"(public: virtual void __cdecl TrayUI::StartTaskbar(void))"},
         &TrayUI_StartTaskbar_Original, TrayUI_StartTaskbar_Hook},
    };
    return WindhawkUtils::HookSymbols(module, taskbarDllHooks,
                                      std::size(taskbarDllHooks));
}

using TaskbarFrame_Constructor_t = void*(WINAPI*)(void* pThis);
TaskbarFrame_Constructor_t TaskbarFrame_Constructor_Original = nullptr;

void* WINAPI TaskbarFrame_Constructor_Hook(void* pThis) {
    void* result = TaskbarFrame_Constructor_Original(pThis);
    if (g_unloading || !g_loadedRevokers) return result;

    FrameworkElement taskbarFrame = nullptr;
    reinterpret_cast<::IUnknown**>(pThis)[1]->QueryInterface(
        winrt::guid_of<FrameworkElement>(), winrt::put_abi(taskbarFrame));
    if (!taskbarFrame) return result;

    g_loadedRevokers->emplace_back();
    auto revoker = std::prev(g_loadedRevokers->end());
    *revoker = taskbarFrame.Loaded(
        winrt::auto_revoke_t{}, [revoker](IInspectable const&, RoutedEventArgs const&) {
            if (!g_loadedRevokers) return;
            g_loadedRevokers->erase(revoker);
            if (g_unloading) return;
            StartWatchdog();
        });
    return result;
}

bool HookTaskbarViewSymbols(HMODULE module) {
    // Taskbar.View.dll, ExplorerExtensions.dll
    WindhawkUtils::SYMBOL_HOOK hooks[] = {{
        {LR"(public: __cdecl winrt::Taskbar::implementation::TaskbarFrame::TaskbarFrame(void))"},
        &TaskbarFrame_Constructor_Original,
        TaskbarFrame_Constructor_Hook,
    }};
    return WindhawkUtils::HookSymbols(module, hooks, std::size(hooks));
}

HMODULE GetTaskbarViewModule() {
    HMODULE module = GetModuleHandleW(L"Taskbar.View.dll");
    if (!module) module = GetModuleHandleW(L"ExplorerExtensions.dll");
    return module;
}

using LoadLibraryExW_t = decltype(&LoadLibraryExW);
LoadLibraryExW_t LoadLibraryExW_Original = nullptr;

HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR fileName, HANDLE file, DWORD flags) {
    HMODULE module = LoadLibraryExW_Original(fileName, file, flags);
    if (module && !g_taskbarViewDllLoaded && GetTaskbarViewModule() == module &&
        !g_taskbarViewDllLoaded.exchange(true)) {
        if (HookTaskbarViewSymbols(module)) Wh_ApplyHookOperations();
    }
    return module;
}

}  // namespace

BOOL Wh_ModInit() {
    g_uiTornDown = false;
    if (HMODULE gdi32 = GetModuleHandleW(L"gdi32.dll")) {
        g_d3dkmtEnumAdapters2 = reinterpret_cast<D3DKMTEnumAdapters2_t>(
            GetProcAddress(gdi32, "D3DKMTEnumAdapters2"));
        g_d3dkmtOpenAdapterFromLuid = reinterpret_cast<D3DKMTOpenAdapterFromLuid_t>(
            GetProcAddress(gdi32, "D3DKMTOpenAdapterFromLuid"));
        g_d3dkmtQueryAdapterInfo = reinterpret_cast<D3DKMTQueryAdapterInfo_t>(
            GetProcAddress(gdi32, "D3DKMTQueryAdapterInfo"));
        g_d3dkmtCloseAdapter = reinterpret_cast<D3DKMTCloseAdapter_t>(
            GetProcAddress(gdi32, "D3DKMTCloseAdapter"));
    }

    LoadSettings();

    if (!HookTaskbarDllSymbols()) {
        Wh_Log(L"taskbar.dll symbols unavailable");
        return FALSE;
    }

    if (HMODULE module = GetTaskbarViewModule()) {
        g_taskbarViewDllLoaded = true;
        if (!HookTaskbarViewSymbols(module)) {
            Wh_Log(L"Taskbar.View symbols unavailable");
            return FALSE;
        }
    } else {
        HMODULE kernelBase = GetModuleHandleW(L"kernelbase.dll");
        if (!kernelBase) kernelBase = GetModuleHandleW(L"kernel32.dll");
        auto loadLibraryEx =
            kernelBase ? reinterpret_cast<LoadLibraryExW_t>(
                            GetProcAddress(kernelBase, "LoadLibraryExW"))
                      : nullptr;
        if (!loadLibraryEx ||
            !WindhawkUtils::SetFunctionHook(loadLibraryEx, LoadLibraryExW_Hook,
                                            &LoadLibraryExW_Original)) {
            return FALSE;
        }
    }

    StartInternetWorker();
    StartLhmWorker();
    return TRUE;
}

void Wh_ModAfterInit() {
    if (!g_taskbarViewDllLoaded) {
        if (HMODULE module = GetTaskbarViewModule()) {
            if (!g_taskbarViewDllLoaded.exchange(true)) {
                if (HookTaskbarViewSymbols(module)) Wh_ApplyHookOperations();
            }
        }
    }
    StartWatchdog();
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    WakeMetricsWorker();
    if (g_watchdogWakeEvent) SetEvent(g_watchdogWakeEvent);
}

void Wh_ModBeforeUninit() {
    g_unloading = true;
    StopWatchdogThread();
    StopMetricsWorker();
    StopInternetWorker();
    StopLhmWorker();

    g_uiTornDown = TearDownTaskbarUi();
    if (!g_uiTornDown) Wh_Log(L"Initial taskbar UI teardown failed; will retry");
}

void Wh_ModUninit() {
    if (!g_uiTornDown) {
        g_uiTornDown = TearDownTaskbarUi();
        if (!g_uiTornDown) Wh_Log(L"Taskbar UI teardown retry failed");
    }
    StopMetricsWorker();
    StopInternetWorker();
    StopLhmWorker();
    CloseMetricSources();
}
