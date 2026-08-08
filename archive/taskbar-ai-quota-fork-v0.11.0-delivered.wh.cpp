// ==WindhawkMod==
// @id              taskbar-ai-quota-fork
// @name            Taskbar AI Quota Bars - Fork
// @description     Shows compact 5-hour and weekly AI agent/LLM subscription quota bars for Anthropic and OpenAI on the Windows 11 taskbar
// @version         0.11.0
// @author          Cleroth
// @github          https://github.com/Cleroth
// @include         explorer.exe
// @architecture    x86-64
// @license         MIT
// @compilerOptions -DWIN32_LEAN_AND_MEAN -lole32 -loleaut32 -lruntimeobject -lwindowsapp -lwinhttp -luser32 -lshell32 -lgdi32 -lws2_32 -lcrypt32 -lbcrypt -ladvapi32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Taskbar AI Quota Bars

Shows Anthropic Claude and OpenAI/Codex AI agent and LLM subscription quota usage as
compact bars on the Windows 11 taskbar, next to the system tray.
Can show on the primary taskbar only, all taskbars, or one specific monitor.

![Taskbar AI Quota Bars](https://i.imgur.com/LD0K31E.png)
![Taskbar AI Quota Bars Detail](https://i.imgur.com/H7agnz2.png)

Each account gets one compact column:
- stacked layout: stacked horizontal bars, filling left-to-right
- vertical layout: side-by-side bars, filling bottom-up

## Placement

**Position** offers 25 anchor points. Tray positions (far left/right, either side of the
clock, the Network/Volume button, the language switcher, the tray icons, the hidden-icons
chevron, or Show Desktop) reserve their own column, so they never overlap Explorer's own
icons. Taskbar positions anchor to the Start, Search, Task View, or Widgets buttons and
follow them as the taskbar recentres. The three Overlay positions float above the taskbar
without reserving any space.

## Reading the bars

Each bar can carry two pieces of text: the percentage at the leading end and the time
until that window resets ("4h 52m") at the trailing end. Text over the coloured fill flips
to black or white automatically, whichever reads better against that threshold colour, and
it flips exactly where the fill crosses each glyph rather than all at once.

A thin strip can sit above a bar showing how far through its reset window you are.

The primary window (5h), the secondary window (weekly, or a 30-day plan window), and the
reset strip each have their own length and thickness. Any account can override all three
plus the stacked/vertical layout, so a focused setup - a chunky 5h bar, a thin weekly bar
under it, and a hairline reset strip - is just a matter of settings. Bars too thin to hold
readable text drop it automatically rather than squeezing it.

Hover for exact percentages and reset times. Click a column to refresh that account
or open the provider dashboard, depending on settings. Right-click for Refresh all,
Open dashboard, and a checkbox list to show/hide individual accounts. Hidden accounts
stop updating and are left to go stale; the choice persists across restarts (at least
one account always stays visible).
Bars use configurable green/yellow/orange/red thresholds, with a colorblind palette option.
Stale errors can mark labels/tooltips with `!`.

Optionally fires a Windows notification when an account first crosses the red threshold
(5-hour or weekly), so you don't have to keep glancing at the bars.

## Signing in

A column that needs auth shows "click to sign in"; left-click it (or use **Sign in** in
the right-click menu):
- **Anthropic**: a browser opens to claude.ai; after you authorize, paste the code shown
  on the page into the prompt.
- **OpenAI**: a browser opens to chatgpt.com; the redirect is caught automatically on
  `localhost:1455`.

The mod refreshes the access token itself. Tokens are stored encrypted (Windows DPAPI).
Use **Sign out** to remove a stored token.

## Suggestions & bugs

Have a suggestion or found a bug?
[Open an issue](https://github.com/Cleroth/windhawk-taskbar-ai-quota/issues/new).
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- accounts:
    - - provider: anthropic
        $name: Provider
        $description: 'Choose the API provider. Sign in per account from the right-click menu.'
        $options:
          - anthropic: Anthropic (Claude)
          - openai: OpenAI (ChatGPT/Codex)
      - label: A
        $name: Label
        $description: 'Default: A for Anthropic, O for OpenAI. The label also identifies the stored sign-in; renaming it requires signing in again.'
      - primaryBar: ''
        $name: Primary bar size override
        $description: 'Optional. Two numbers, "length thickness" (for example "140 18"), overriding the global primary bar size for this account only. Leave empty to inherit. Use one number to override length only.'
      - secondaryBar: ''
        $name: Secondary bar size override
        $description: 'Optional. Two numbers, "length thickness", overriding the global secondary bar size for this account only. Leave empty to inherit.'
      - resetBar: ''
        $name: Reset strip size override
        $description: 'Optional. Two numbers, "length thickness", overriding the global reset strip size for this account only. Leave empty to inherit.'
      - barLayout: inherit
        $name: Bar layout override
        $description: 'Default: Inherit. Lets one account stack its bars while another draws them vertically.'
        $options:
          - inherit: Inherit global setting
          - stacked: Stacked Horizontal
          - vertical: Vertical
    - - provider: openai
      - label: O
  $name: Accounts
  $description: 'Default: two accounts, Anthropic A and OpenAI O. Sign in to each from a quota column''s right-click menu.'
- taskbarMonitorMode: primary
  $name: Taskbar monitors
  $description: 'Default: Primary only. Choose where quota bars are shown.'
  $options:
    - primary: Primary monitor only
    - all: All monitors
    - specific: Specific monitor number
- taskbarMonitorNumber: 1
  $name: Specific monitor number
  $description: 'Default: 1. Used when Taskbar monitors is Specific. 1 is the primary taskbar; 2+ are secondary taskbars in monitor order.'
- position: tray_left
  $name: Position
  $description: 'Default: Tray - Far left. Where the quota bars are inserted into the taskbar. Tray positions reserve their own space; the three Overlay positions float above the taskbar without reserving any.'
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
- clickAction: refresh
  $name: Click action
  $description: 'Default: Refresh account. Choose what left-clicking a quota column does.'
  $options:
    - refresh: Refresh account
    - open-dashboard: Open provider dashboard
- pollIntervalMinutes: 10
  $name: Poll interval (minutes)
  $description: 'Default: 10'
- barLayout: stacked
  $name: Bar layout (default)
  $description: 'Default: Stacked. Stacked draws left-to-right bars; Vertical draws side-by-side bars filling bottom-up. Each account can override this.'
  $options:
    - stacked: Stacked Horizontal
    - vertical: Vertical
- barMode: used
  $name: Bar mode
  $description: 'Default: Used. Used fills bars as quota is consumed; Remaining fills bars with the quota left and shows "X% remaining" in tooltips.'
  $options:
    - used: Used
    - remaining: Remaining
- barPrimaryLength: 110
  $name: Primary bar length (px)
  $description: 'Default: 110. The primary bar is the short rolling window (5h on Anthropic). Width for stacked bars, height for vertical bars.'
- barPrimaryThickness: 15
  $name: Primary bar thickness (px)
  $description: 'Default: 15. Height for stacked bars, width for vertical bars. Needs roughly font size + 4 to fit text inside.'
- barSecondaryLength: 110
  $name: Secondary bar length (px)
  $description: 'Default: 110. The secondary bar is the long window (weekly, or a 30-day plan window).'
- barSecondaryThickness: 8
  $name: Secondary bar thickness (px)
  $description: 'Default: 8. Keep this thinner than the primary bar to put visual weight on the short window.'
- barGap: 2
  $name: Gap between quota bars (px)
  $description: 'Default: 2.'
- showResetBar: true
  $name: Show reset progress strip
  $description: 'Default: true. A thin strip above a bar showing how far through its reset window you are.'
- resetBarWindows: primary
  $name: Reset strip on which bars
  $description: 'Default: Primary only.'
  $options:
    - primary: Primary bar only
    - secondary: Secondary bar only
    - both: Both bars
- barResetThickness: 3
  $name: Reset strip thickness (px)
  $description: 'Default: 3. Minimum: 1.'
- barResetLength: 0
  $name: Reset strip length (px)
  $description: 'Default: 0, meaning match the length of the bar it sits above.'
- resetBarGap: 2
  $name: Gap below reset strip (px)
  $description: 'Default: 2. Space between the reset strip and the bar under it.'
- resetBarMode: elapsed
  $name: Reset strip direction
  $description: 'Default: Elapsed. Elapsed fills as the window runs out; Remaining drains toward the reset.'
  $options:
    - elapsed: Fill as time elapses
    - remaining: Drain as time elapses
- showPercentText: true
  $name: Show percent text
  $description: 'Default: true. Draws the percentage inside the bar, inverting only over the filled portion.'
- percentTextWindows: primary
  $name: Percent text on which bars
  $description: 'Default: Primary only. Text is dropped automatically from any bar too thin to hold it legibly.'
  $options:
    - primary: Primary bar only
    - secondary: Secondary bar only
    - both: Both bars
- percentFontSize: 11
  $name: Percent font size (px)
  $description: 'Default: 11. Shrinks automatically to fit thin bars.'
- showResetText: true
  $name: Show reset countdown text
  $description: 'Default: true. Draws the time until reset (for example 4h 52m) at the far end of the bar.'
- resetTextWindows: primary
  $name: Reset countdown on which bars
  $description: 'Default: Primary only.'
  $options:
    - primary: Primary bar only
    - secondary: Secondary bar only
    - both: Both bars
- resetFontSize: 10
  $name: Reset countdown font size (px)
  $description: 'Default: 10. Shrinks automatically to fit thin bars.'
- barTextPadding: 5
  $name: Bar text padding (px)
  $description: 'Default: 5. Inset of the percent and countdown text from the ends of the bar.'
- barCornerRadius: -1
  $name: Bar corner radius (px)
  $description: 'Default: -1, meaning fully rounded (half the bar thickness). Use 0 for square ends.'
- showLabels: true
  $name: Show labels
  $description: 'Default: true'
- labelOnLeft: true
  $name: Put labels on the left
  $description: 'Default: true'
- labelFontSize: 11
  $name: Label font size (px)
  $description: 'Default: 11'
- accountMargin: 3
  $name: Account margin (px)
  $description: 'Default: 3. Horizontal margin around each account column.'
- labelGap: 3
  $name: Label gap (px)
  $description: 'Default: 3. Gap between label and bars.'
- rightMargin: 4
  $name: Right gap (px)
  $description: 'Default: 4. Gap on the trailing side of the quota bars.'
- leftMargin: 4
  $name: Left gap (px)
  $description: 'Default: 4. Gap on the leading side of the quota bars.'
- diagnosticLogUsageResponses: false
  $name: Log raw usage responses
  $description: 'Default: false. Writes the fetched usage JSON (not OAuth tokens) to the Windhawk debug log. Enable temporarily for plan/limit diagnostics, refresh the account, then disable it.'
- showCodexSparkInTooltip: false
  $name: Show Codex Spark in tooltip
  $description: 'Default: false. Shows OpenAI/Codex Spark plan and rate-limit lines in tooltips.'
- yellowThreshold: 50
  $name: Yellow threshold (%)
  $description: 'Default: 50. Usage below this stays green.'
- orangeThreshold: 75
  $name: Orange threshold (%)
  $description: 'Default: 75. Usage below this stays yellow.'
- redThreshold: 90
  $name: Red threshold (%)
  $description: 'Default: 90. Usage at or above this turns red.'
- enableNotifications: true
  $name: Threshold notifications
  $description: 'Default: true. Shows a Windows notification when an account first crosses the red threshold (5h or weekly). Re-arms after usage drops back below.'
- colorblindMode: false
  $name: Colorblind mode
  $description: 'Default: false. Uses a blue-to-orange palette instead of green/red.'
- showStaleWarning: true
  $name: Show stale warning
  $description: 'Default: true. Marks stale error states with ! in labels and tooltips.'
*/
// ==/WindhawkModSettings==

// Windhawk implicitly includes windhawk_api.h (and thus windows.h) before this file,
// so winsock2.h can't be ordered ahead of windows.h here. WIN32_LEAN_AND_MEAN (set in
// @compilerOptions) keeps that windows.h from pulling in the legacy winsock.h, so
// winsock2.h is included cleanly below without redefinition conflicts.
#include <winsock2.h>
#include <ws2tcpip.h>

#include <windhawk_utils.h>

#include <windows.h>
#include <shellapi.h>
#include <winhttp.h>
#include <bcrypt.h>
#include <wincrypt.h>
#include <dpapi.h>
#include <unknwn.h>

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

#include <winrt/base.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Documents.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Media.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

using namespace winrt::Windows::Data::Json;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

namespace wuxcp = winrt::Windows::UI::Xaml::Controls::Primitives;
namespace wuxd = winrt::Windows::UI::Xaml::Documents;
namespace wuxi = winrt::Windows::UI::Xaml::Input;

/**********************************************/
//  Settings and State
/**********************************************/

enum class TaskbarMonitorMode {
    Primary,
    All,
    Specific,
};

enum class ClickAction {
    Refresh,
    OpenDashboard,
};

enum class BarLayout {
    Stacked,
    Vertical,
};

enum class BarMode {
    Used,
    Remaining,
};

// Which of the two quota windows an optional bar decoration applies to.
enum class BarWindows {
    Primary,
    Secondary,
    Both,
};

static bool BarWindowsInclude(BarWindows which, int windowIndex) {
    if (which == BarWindows::Both) return true;
    return windowIndex == 0 ? which == BarWindows::Primary : which == BarWindows::Secondary;
}

// Length runs along the bar, thickness across it. 0 means "inherit the global value".
struct BarSpec {
    int length = 0;
    int thickness = 0;
};

struct AccountConfig {
    std::wstring provider;  // "anthropic" or "openai".
    std::wstring label;
    bool hidden = false;  // Runtime show/hide toggle (right-click menu), persisted in mod storage.
    // Per-account size overrides, parsed from "<length> <thickness>" setting strings.
    // A zero field falls back to the matching global setting.
    BarSpec primary;
    BarSpec secondary;
    BarSpec reset;
    bool hasLayoutOverride = false;
    BarLayout layoutOverride = BarLayout::Stacked;
};

struct Settings {
    std::vector<AccountConfig> accounts;
    std::wstring position = L"tray_left";
    TaskbarMonitorMode taskbarMonitorMode = TaskbarMonitorMode::Primary;
    ClickAction clickAction = ClickAction::Refresh;
    BarLayout barLayout = BarLayout::Stacked;
    BarMode barMode = BarMode::Used;
    BarWindows resetBarWindows = BarWindows::Primary;
    BarWindows percentTextWindows = BarWindows::Primary;
    BarWindows resetTextWindows = BarWindows::Primary;
    int taskbarMonitorNumber = 1;
    int pollMinutes = 10;
    BarSpec primaryBar{110, 15};
    BarSpec secondaryBar{110, 8};
    BarSpec resetBar{0, 3};
    int barGap = 2;
    int resetBarGap = 2;
    int barCornerRadius = -1;
    int percentFontSize = 11;
    int resetFontSize = 10;
    int barTextPadding = 5;
    int labelFontSize = 11;
    int accountMargin = 3;
    int labelGap = 3;
    int rightMargin = 4;
    int leftMargin = 4;
    int yellowThreshold = 50;
    int orangeThreshold = 75;
    int redThreshold = 90;
    bool showLabels = true;
    bool labelOnLeft = true;
    bool showPercentText = true;
    bool showResetText = true;
    bool showResetBar = true;
    bool resetBarElapsed = true;
    bool showCodexSparkInTooltip = false;
    bool colorblindMode = false;
    bool showStaleWarning = true;
    bool enableNotifications = true;
    bool diagnosticLogUsageResponses = false;
};

// Everything BuildQuotaGrid and UpdateQuotaUi need to lay out one bar, resolved once
// per account per window so the two stay in lockstep. Keeping the arithmetic in a single
// place is what lets per-account and per-window sizes be genuinely independent.
struct ResolvedBar {
    int length = 0;
    int thickness = 0;
    int resetLength = 0;
    int resetThickness = 0;
    int percentFontSize = 0;  // 0 = no percent text on this bar.
    int resetFontSize = 0;    // 0 = no countdown text on this bar.
    int textPadding = 0;
    double cornerRadius = 0;
    double resetCornerRadius = 0;
    bool showResetBar = false;
    bool vertical = false;
};

struct WindowUsage {
    double pct = -1;
    ULONGLONG resetUnixMs = 0;
    ULONGLONG durationSeconds = 0;
};

struct AccountData {
    WindowUsage win5h;
    WindowUsage winWeek;
    std::wstring plan;
    std::wstring codexSparkLines;
    std::wstring extraLines;
    std::wstring error;
    ULONGLONG lastSuccessMs = 0;
    ULONGLONG retryDeadlineMs = 0;
    bool stale = true;
    bool needsLogin = false;  // Sign-in required; left-click signs in instead of refreshing.
};

struct AppliedState {
    int fillPx[2] = {-1, -1};
    uint32_t fillColor[2] = {0, 0};
    int resetFillPx[2] = {-1, -1};
    int resetVisible[2] = {-1, -1};
    int windowVisible[2] = {-1, -1};
    uint32_t onFillColor[2] = {0, 0};
    std::wstring tip;
    std::array<std::wstring, 2> percentText;
    std::array<std::wstring, 2> resetText;
    std::wstring labelText;
    double labelOpacity = -1;
    double columnOpacity = -1;
    int visible = -1;  // -1 unset, 0 collapsed, 1 visible.
};

struct PointerHandlers {
    UIElement element{nullptr};
    winrt::event_token tappedToken{};
    winrt::event_token pointerMovedToken{};
    winrt::event_token pointerExitedToken{};
    winrt::event_token pointerCaptureLostToken{};
    winrt::event_token pointerCanceledToken{};
};

struct MenuItemClickHandler {
    MenuFlyoutItem item{nullptr};
    winrt::event_token token{};
};

struct BarUiRefs {
    StackPanel slot{nullptr};    // Reset strip + bar, stacked along the cross axis.
    Border track{nullptr};       // Rounded background of the quota bar.
    Border fill{nullptr};        // Coloured usage fill.
    Border resetTrack{nullptr};  // Thin strip above the bar.
    Border resetFill{nullptr};
    // Two identical text layers. The base layer sits over the unfilled track; the
    // inverted layer is an exact copy clipped to the filled region, so glyphs flip
    // colour exactly where the fill crosses them instead of at a glyph boundary.
    Grid textBase{nullptr};
    Grid textInverted{nullptr};
    TextBlock percentBase{nullptr};
    TextBlock percentInverted{nullptr};
    TextBlock resetBase{nullptr};
    TextBlock resetInverted{nullptr};
    RectangleGeometry invertClip{nullptr};
};

struct AccountUiRefs {
    StackPanel column{nullptr};
    ToolTip toolTip{nullptr};
    winrt::event_token toolTipOpenedToken{};
    DispatcherTimer manualToolTipTimer{nullptr};
    winrt::event_token manualToolTipTimerToken{};
    std::array<BarUiRefs, 2> bars{};
    TextBlock label{nullptr};
    POINT toolTipOpenCursor{};
    bool hasToolTipOpenCursor = false;
    bool reopenToolTipOnMove = false;
    bool manualToolTipOpen = false;
};

struct QuotaUiInstance {
    HWND hWnd = nullptr;
    Grid quotaGrid{nullptr};
    Grid injectionParent{nullptr};
    int quotaColumn = -1;
    bool insertedColumn = false;
    bool overlayPositioned = false;   // Floats above the taskbar; reserves no grid column.
    int appliedLightTheme = -1;       // -1 unknown, else the theme the chrome was painted for.
    // Drives the in-bar countdown between polls, so "4h 52m" stays honest without
    // hammering the provider APIs.
    DispatcherTimer tickTimer{nullptr};
    winrt::event_token tickTimerToken{};
    // Anchored taskbar positions: the button we hang off, its untouched margin, and the
    // LayoutUpdated subscription that keeps both in sync as the taskbar recentres.
    FrameworkElement trackedElement{nullptr};
    Thickness trackedOriginalMargin{};
    bool hasTrackedOriginalMargin = false;
    bool trackAnchorOnLeft = false;
    winrt::event_token layoutUpdatedToken{};
    bool hasLayoutUpdatedToken = false;
    std::vector<PointerHandlers> pointerHandlers;
    std::vector<MenuItemClickHandler> menuItemClickHandlers;
    std::vector<AccountUiRefs> accountRefs;
    std::vector<AppliedState> applied;
    // Per-account show/hide toggle items, paired with their account index, for cross-instance
    // IsChecked sync in UpdateQuotaUi (Click is revoked via menuItemClickHandlers).
    std::vector<std::pair<int, ToggleMenuFlyoutItem>> accountToggleItems;
};

static Settings g_settings;
static std::mutex g_settingsMutex;
static ULONGLONG g_settingsGeneration = 0;
static std::vector<AccountData> g_data;
static std::mutex g_dataMutex;

static std::atomic<bool> g_unloading{false};
static std::atomic<bool> g_refreshing{false};
static std::atomic<int> g_refreshAccountIndex{-1};
static std::atomic<ULONGLONG> g_refreshGeneration{0};
static std::atomic<bool> g_uiInjected{false};
static std::atomic<bool> g_fetchThreadStarted{false};
static HANDLE g_stopEvent = nullptr;
static HANDLE g_refreshEvent = nullptr;
static HANDLE g_wakeInjectEvent = nullptr;  // Wakes the inject watchdog after a settings change.
static HANDLE g_fetchThread = nullptr;
static HANDLE g_retryThread = nullptr;
static std::atomic<ULONGLONG> g_nextInjectFailureLogMs{0};
static std::mutex g_httpHandlesMutex;
static std::vector<HINTERNET> g_httpHandles;

// Fetch-thread-owned: hidden message-only window that owns the mod's tray icon.
static HWND g_notifyWnd = nullptr;

static std::vector<std::unique_ptr<QuotaUiInstance>> g_uiInstances;

static const wchar_t* kRootName = L"AiQuota_Root";
static constexpr ULONGLONG kFileTimeUnixEpochOffsetMs = 11644473600000ULL;
static constexpr ULONGLONG kUnixTimestampMsThreshold = 100000000000ULL;

using WindowThreadProc = bool (*)(void*);
static bool RunFromWindowThread(HWND hWnd, WindowThreadProc proc, void* param, DWORD timeoutMs = 2000);
static std::vector<HWND> FindCurrentProcessTaskbarWnds();
static QuotaUiInstance* FindUiState(HWND hWnd);
static void UpdateQuotaUi(QuotaUiInstance& state);
static void RemoveQuotaGridFromState(QuotaUiInstance& state);
static void PostUiUpdate();

/**********************************************/
//  Helpers
/**********************************************/

static ULONGLONG NowUnixMs() {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULONGLONG t = ((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    return t / 10000 - kFileTimeUnixEpochOffsetMs;
}

static std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    if (n <= 0) return {};
    std::wstring w(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), n);
    return w;
}

static std::string WideToUtf8(const std::wstring& s) {
    if (s.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0, nullptr, nullptr);
    if (n <= 0) return {};
    std::string out(n, 0);
    WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), n, nullptr, nullptr);
    return out;
}

// Stable identity used to preserve g_data across settings reloads, persist the show/hide
// toggle, and key the encrypted token store. FNV-1a over provider+label; relabeling an
// account therefore points it at a fresh (unsigned-in) identity.
static uint64_t AccountIdentityHash(const AccountConfig& a) {
    std::wstring id = a.provider + L"\n" + a.label;
    uint64_t h = 1469598103934665603ull;
    for (const auto* p = reinterpret_cast<const unsigned char*>(id.data());
         p != reinterpret_cast<const unsigned char*>(id.data() + id.size()); ++p) {
        h = (h ^ *p) * 1099511628211ull;
    }
    return h;
}

static ULONGLONG ParseIso8601Ms(const std::wstring& s) {
    int y = 0, mo = 0, d = 0, h = 0, mi = 0;
    double sec = 0;
    if (swscanf(s.c_str(), L"%d-%d-%dT%d:%d:%lf", &y, &mo, &d, &h, &mi, &sec) != 6) {
        return 0;
    }

    SYSTEMTIME st{};
    st.wYear = (WORD)y;
    st.wMonth = (WORD)mo;
    st.wDay = (WORD)d;
    st.wHour = (WORD)h;
    st.wMinute = (WORD)mi;
    st.wSecond = (WORD)sec;
    st.wMilliseconds = (WORD)((sec - st.wSecond) * 1000);

    FILETIME ft;
    if (!SystemTimeToFileTime(&st, &ft)) return 0;
    ULONGLONG t = ((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    ULONGLONG unixMs = t / 10000 - kFileTimeUnixEpochOffsetMs;

    size_t tpos = s.find(L'T');
    size_t sign = tpos == std::wstring::npos ? std::wstring::npos : s.find_first_of(L"+-", tpos);
    if (sign != std::wstring::npos) {
        int oh = 0, om = 0;
        PCWSTR p = s.c_str() + sign + 1;
        size_t end = s.find_first_not_of(L"0123456789:", sign + 1);
        size_t len = (end == std::wstring::npos ? s.size() : end) - sign - 1;
        auto digit = [](wchar_t c) { return c >= L'0' && c <= L'9'; };
        bool parsedOffset = len >= 2 && digit(p[0]) && digit(p[1]);
        if (parsedOffset) {
            oh = (p[0] - L'0') * 10 + (p[1] - L'0');
            if (len == 5 && p[2] == L':' && digit(p[3]) && digit(p[4])) {
                om = (p[3] - L'0') * 10 + (p[4] - L'0');
            } else if (len == 4 && digit(p[2]) && digit(p[3])) {
                om = (p[2] - L'0') * 10 + (p[3] - L'0');
            } else if (len != 2) {
                parsedOffset = false;
            }
        }
        if (parsedOffset && oh <= 23 && om <= 59) {
            LONGLONG off = ((LONGLONG)oh * 60 + om) * 60000;
            unixMs = s[sign] == L'+' ? unixMs - off : unixMs + off;
        }
    }
    return unixMs;
}

static bool UnixMsToLocalSystemTime(ULONGLONG unixMs, SYSTEMTIME* local) {
    if (!unixMs || !local) return false;

    ULONGLONG t = (unixMs + kFileTimeUnixEpochOffsetMs) * 10000;
    FILETIME ft{(DWORD)(t & 0xFFFFFFFF), (DWORD)(t >> 32)};
    SYSTEMTIME utc;
    return FileTimeToSystemTime(&ft, &utc) &&
           SystemTimeToTzSpecificLocalTime(nullptr, &utc, local);
}

static std::wstring FormatLocalTime(SYSTEMTIME const& local) {
    wchar_t buf[64];
    if (GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, TIME_NOSECONDS, &local, nullptr, buf,
                        ARRAYSIZE(buf)) > 0) {
        return buf;
    }

    swprintf(buf, ARRAYSIZE(buf), L"%02u:%02u", local.wHour, local.wMinute);
    return buf;
}

static std::wstring FormatReset(ULONGLONG unixMs) {
    if (!unixMs) return L"?";
    LONGLONG delta = (LONGLONG)(unixMs - NowUnixMs());
    if (delta <= 0) return L"now";

    ULONGLONG totalMin = ((ULONGLONG)delta + 59999) / 60000;
    ULONGLONG days = totalMin / (24 * 60);
    ULONGLONG hours = (totalMin / 60) % 24;
    ULONGLONG mins = totalMin % 60;

    wchar_t rel[64];
    if (days > 0) {
        if (hours > 0) swprintf(rel, ARRAYSIZE(rel), L"in %llud %lluh", days, hours);
        else swprintf(rel, ARRAYSIZE(rel), L"in %llud", days);
    } else if (hours > 0) {
        if (mins > 0) swprintf(rel, ARRAYSIZE(rel), L"in %lluh %llum", hours, mins);
        else swprintf(rel, ARRAYSIZE(rel), L"in %lluh", hours);
    } else {
        swprintf(rel, ARRAYSIZE(rel), L"in %llum", mins);
    }

    SYSTEMTIME local;
    if (!UnixMsToLocalSystemTime(unixMs, &local)) {
        return rel;
    }

    std::wstring localTime = FormatLocalTime(local);
    if (delta < 24LL * 3600 * 1000) {
        return std::wstring(rel) + L" (" + localTime + L")";
    }

    wchar_t day[16] = L"";
    GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &local, L"ddd", day, ARRAYSIZE(day), nullptr);
    return std::wstring(rel) + L" (" + day + L" " + localTime + L")";
}

static std::wstring FormatUpdated(ULONGLONG unixMs, bool stale) {
    if (!unixMs) return L"updated: no data yet";
    SYSTEMTIME local;
    if (!UnixMsToLocalSystemTime(unixMs, &local)) {
        return L"updated: ?";
    }
    return std::wstring(L"updated: ") + FormatLocalTime(local) + (stale ? L" (stale)" : L"");
}

// A provider can return any plan-specific window (for example, 30 days for the
// currently returned OpenAI Free limit), rather than only the historical 5h/week pair.
static std::wstring FormatUsageWindowLabel(ULONGLONG durationSeconds, int fallbackIndex) {
    if (durationSeconds == 5ULL * 60 * 60) return L"5h";
    if (durationSeconds == 7ULL * 24 * 60 * 60) return L"week";
    if (durationSeconds > 0 && durationSeconds % (24ULL * 60 * 60) == 0) {
        return std::to_wstring(durationSeconds / (24ULL * 60 * 60)) + L"d";
    }
    if (durationSeconds > 0 && durationSeconds % (60ULL * 60) == 0) {
        return std::to_wstring(durationSeconds / (60ULL * 60)) + L"h";
    }
    return fallbackIndex == 0 ? L"primary" : L"secondary";
}

static winrt::Windows::UI::Color UsageColor(double pct, bool stale, int yellowThreshold,
                                            int orangeThreshold, int redThreshold,
                                            bool colorblindMode) {
    if (stale || pct < 0) return {255, 0x9E, 0x9E, 0x9E};

    if (colorblindMode) {
        if (pct >= redThreshold) return {255, 0xD5, 0x5E, 0x00};
        if (pct >= orangeThreshold) return {255, 0xE6, 0x9F, 0x00};
        if (pct >= yellowThreshold) return {255, 0x56, 0xB4, 0xE9};
        return {255, 0x00, 0x72, 0xB2};
    }

    if (pct >= redThreshold) return {255, 0xE5, 0x39, 0x35};
    if (pct >= orangeThreshold) return {255, 0xFB, 0x8C, 0x00};
    if (pct >= yellowThreshold) return {255, 0xFD, 0xD8, 0x35};
    return {255, 0x43, 0xA0, 0x47};
}


static bool IsSystemLightTheme() {
    DWORD value = 0;
    DWORD size = sizeof(value);
    if (RegGetValueW(HKEY_CURRENT_USER,
                     L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                     L"SystemUsesLightTheme", RRF_RT_REG_DWORD, nullptr, &value,
                     &size) == ERROR_SUCCESS) {
        return value != 0;
    }
    return false;
}

// Palette that the bar chrome is drawn from. Recomputed whenever the system theme flips
// so bars stay legible on both a light and a dark taskbar without a mod reload.
struct BarPalette {
    bool light = false;
    winrt::Windows::UI::Color track{0x38, 0xFF, 0xFF, 0xFF};
    winrt::Windows::UI::Color onTrack{255, 0xFF, 0xFF, 0xFF};
    winrt::Windows::UI::Color resetTrack{0x2E, 0xFF, 0xFF, 0xFF};
    winrt::Windows::UI::Color resetFill{0x99, 0xFF, 0xFF, 0xFF};
};

static BarPalette MakeBarPalette(bool light) {
    BarPalette p;
    p.light = light;
    if (light) {
        p.track = {0x30, 0x00, 0x00, 0x00};
        p.onTrack = {255, 0x1A, 0x1A, 0x1A};
        p.resetTrack = {0x24, 0x00, 0x00, 0x00};
        p.resetFill = {0x8A, 0x00, 0x00, 0x00};
    } else {
        p.track = {0x38, 0xFF, 0xFF, 0xFF};
        p.onTrack = {255, 0xFF, 0xFF, 0xFF};
        p.resetTrack = {0x2E, 0xFF, 0xFF, 0xFF};
        p.resetFill = {0x99, 0xFF, 0xFF, 0xFF};
    }
    return p;
}

static double SrgbChannelToLinear(double c) {
    return c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
}

static double RelativeLuminance(winrt::Windows::UI::Color const& c) {
    return 0.2126 * SrgbChannelToLinear(c.R / 255.0) +
           0.7152 * SrgbChannelToLinear(c.G / 255.0) +
           0.0722 * SrgbChannelToLinear(c.B / 255.0);
}

// Picks whichever of near-black / white has the higher WCAG contrast against the fill.
// 0.179 is the luminance at which the two are exactly equal, so this is the choice that
// maximises legibility for any threshold palette the user configures, colorblind included.
static winrt::Windows::UI::Color OnColorFor(winrt::Windows::UI::Color const& background) {
    return RelativeLuminance(background) > 0.179
               ? winrt::Windows::UI::Color{255, 0x11, 0x14, 0x17}
               : winrt::Windows::UI::Color{255, 0xFF, 0xFF, 0xFF};
}

static uint32_t PackColor(winrt::Windows::UI::Color const& c) {
    return ((uint32_t)c.A << 24) | ((uint32_t)c.R << 16) | ((uint32_t)c.G << 8) | c.B;
}

// Short in-bar countdown: "3d 4h", "4h 52m", "12m". Deliberately coarser than the
// tooltip's FormatReset, which also spells out the wall-clock reset time.
static std::wstring FormatCountdownCompact(ULONGLONG resetUnixMs) {
    if (!resetUnixMs) return L"";

    LONGLONG delta = (LONGLONG)(resetUnixMs - NowUnixMs());
    if (delta <= 0) return L"now";

    ULONGLONG totalMin = ((ULONGLONG)delta + 59999) / 60000;
    ULONGLONG days = totalMin / (24 * 60);
    ULONGLONG hours = (totalMin / 60) % 24;
    ULONGLONG mins = totalMin % 60;

    wchar_t out[32];
    if (days > 0) {
        if (hours > 0) swprintf(out, ARRAYSIZE(out), L"%llud %lluh", days, hours);
        else swprintf(out, ARRAYSIZE(out), L"%llud", days);
    } else if (hours > 0) {
        swprintf(out, ARRAYSIZE(out), L"%lluh %llum", hours, mins);
    } else {
        swprintf(out, ARRAYSIZE(out), L"%llum", mins);
    }
    return out;
}

// Fraction of the reset window already elapsed, or -1 when the provider gave us no
// window length to measure against.
static double ResetWindowProgress(WindowUsage const& wu) {
    if (!wu.resetUnixMs || wu.durationSeconds == 0) return -1;

    double windowMs = (double)wu.durationSeconds * 1000.0;
    double remaining = (double)(LONGLONG)(wu.resetUnixMs - NowUnixMs());
    remaining = std::clamp(remaining, 0.0, windowMs);
    return 1.0 - remaining / windowMs;
}

// Single source of truth for how big one bar is. BuildQuotaGrid and UpdateQuotaUi both
// call this, so per-account overrides and per-window sizes can never drift apart.
static ResolvedBar ResolveBar(Settings const& s, AccountConfig const& acc, int w) {
    ResolvedBar r;
    BarSpec const& globalSpec = w == 0 ? s.primaryBar : s.secondaryBar;
    BarSpec const& accSpec = w == 0 ? acc.primary : acc.secondary;

    r.length = std::max(accSpec.length > 0 ? accSpec.length : globalSpec.length, 10);
    r.thickness = std::clamp(accSpec.thickness > 0 ? accSpec.thickness : globalSpec.thickness, 2, 64);
    r.vertical = (acc.hasLayoutOverride ? acc.layoutOverride : s.barLayout) == BarLayout::Vertical;

    r.resetThickness =
        std::clamp(acc.reset.thickness > 0 ? acc.reset.thickness : s.resetBar.thickness, 1, 32);
    int resetLength = acc.reset.length > 0 ? acc.reset.length : s.resetBar.length;
    r.resetLength = resetLength > 0 ? std::clamp(resetLength, 4, r.length) : r.length;
    r.showResetBar = s.showResetBar && BarWindowsInclude(s.resetBarWindows, w);

    r.textPadding = std::clamp(s.barTextPadding, 0, 40);
    // Text only runs along the bar, so a vertical bar has no room for it. Otherwise the
    // font shrinks to fit a thin bar and is dropped entirely once it would be unreadable.
    auto fitFont = [&](bool enabled, int requested) {
        if (!enabled || r.vertical) return 0;
        int size = std::min(requested, r.thickness - 3);
        return size >= 7 ? size : 0;
    };
    r.percentFontSize =
        fitFont(s.showPercentText && BarWindowsInclude(s.percentTextWindows, w), s.percentFontSize);
    r.resetFontSize =
        fitFont(s.showResetText && BarWindowsInclude(s.resetTextWindows, w), s.resetFontSize);

    r.cornerRadius = s.barCornerRadius >= 0
                         ? std::min((double)s.barCornerRadius, r.thickness / 2.0)
                         : r.thickness / 2.0;
    r.resetCornerRadius = s.barCornerRadius >= 0
                              ? std::min((double)s.barCornerRadius, r.resetThickness / 2.0)
                              : r.resetThickness / 2.0;
    return r;
}

static void UpdateQuotaToolTip(ToolTip const& toolTip, std::wstring const& tip, bool hasError) {
    constexpr double maxWidth = 460;
    auto muted = SolidColorBrush(winrt::Windows::UI::Color{255, 0xD6, 0xD6, 0xD6});
    auto quotaLabel = SolidColorBrush(winrt::Windows::UI::Color{255, 0xFF, 0xD7, 0x66});
    auto infoLabel = SolidColorBrush(winrt::Windows::UI::Color{255, 0xA8, 0xD8, 0xFF});
    auto creditLabel = SolidColorBrush(winrt::Windows::UI::Color{255, 0xC7, 0x9B, 0xFF});
    auto duration = SolidColorBrush(winrt::Windows::UI::Color{255, 0xB7, 0xE4, 0xA3});
    auto accent = SolidColorBrush(hasError ?
        winrt::Windows::UI::Color{255, 0xFF, 0xB4, 0xA9} :
        winrt::Windows::UI::Color{255, 0x8A, 0xD1, 0xFF});
    auto border = SolidColorBrush(hasError ?
        winrt::Windows::UI::Color{0xB8, 0xD1, 0x34, 0x38} :
        winrt::Windows::UI::Color{0x72, 0x8A, 0xD1, 0xFF});

    size_t firstBreak = tip.find(L'\n');
    std::wstring title = firstBreak == std::wstring::npos ? tip : tip.substr(0, firstBreak);
    std::wstring body = firstBreak == std::wstring::npos ? L"" : tip.substr(firstBreak + 1);

    TextBlock titleBlock;
    titleBlock.Text(winrt::hstring(title));
    titleBlock.FontSize(12.5);
    titleBlock.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
    titleBlock.Foreground(accent);
    titleBlock.TextWrapping(TextWrapping::Wrap);
    titleBlock.MaxWidth(maxWidth);
    titleBlock.IsHitTestVisible(false);

    StackPanel content;
    content.Orientation(Orientation::Vertical);
    content.MaxWidth(maxWidth);
    content.IsHitTestVisible(false);
    content.Children().Append(titleBlock);

    if (!body.empty()) {
        auto appendRun = [](TextBlock const& textBlock, std::wstring const& text,
                            Brush const& brush, bool bold = false) {
            if (text.empty()) return;
            wuxd::Run run;
            run.Text(winrt::hstring(text));
            run.Foreground(brush);
            if (bold) run.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
            textBlock.Inlines().Append(run);
        };

        bool firstLine = true;
        for (size_t pos = 0; pos <= body.size();) {
            size_t next = body.find(L'\n', pos);
            std::wstring line = next == std::wstring::npos ? body.substr(pos) : body.substr(pos, next - pos);

            TextBlock lineBlock;
            lineBlock.FontSize(12);
            lineBlock.LineHeight(16);
            lineBlock.TextWrapping(TextWrapping::Wrap);
            lineBlock.MaxWidth(maxWidth);
            lineBlock.Margin(firstLine ? Thickness{0, 4, 0, 0} : Thickness{0, 1, 0, 0});
            lineBlock.IsHitTestVisible(false);

            Brush labelBrush = muted;
            size_t labelEnd = std::wstring::npos;
            size_t highlightStart = std::wstring::npos;
            size_t highlightEnd = std::wstring::npos;
            bool labelBold = false;
            bool quotaLine = false;
            bool errorLine = false;
            if (line.rfind(L"5h:", 0) == 0) {
                labelBrush = quotaLabel;
                labelEnd = 3;
                labelBold = true;
                quotaLine = true;
            } else if (line.rfind(L"week:", 0) == 0) {
                labelBrush = quotaLabel;
                labelEnd = 5;
                labelBold = true;
                quotaLine = true;
            } else if (line.rfind(L"error:", 0) == 0) {
                labelBrush = accent;
                labelEnd = 6;
                labelBold = true;
                errorLine = true;
            } else if (line.rfind(L"credits:", 0) == 0) {
                labelBrush = creditLabel;
                labelEnd = 8;
                highlightStart = line.find_first_not_of(L" ", labelEnd);
                if (highlightStart != std::wstring::npos) {
                    highlightEnd = highlightStart;
                    while (highlightEnd < line.size()) {
                        wchar_t ch = line[highlightEnd];
                        if ((ch < L'0' || ch > L'9') && ch != L'.') break;
                        highlightEnd++;
                    }
                }
                labelBold = true;
            } else if (line.rfind(L"extra usage:", 0) == 0) {
                labelBrush = creditLabel;
                labelEnd = 12;
                labelBold = true;
                quotaLine = true;
            } else if (line.rfind(L"updated:", 0) == 0) {
                labelBrush = infoLabel;
                labelEnd = 8;
                highlightStart = line.find(L"no data yet", labelEnd);
                if (highlightStart != std::wstring::npos) highlightEnd = highlightStart + 11;
                labelBold = true;
            }

            size_t textStart = 0;
            if (labelEnd != std::wstring::npos) {
                appendRun(lineBlock, line.substr(0, labelEnd), labelBrush, labelBold);
                textStart = labelEnd;
            }

            if (errorLine) {
                appendRun(lineBlock, line.substr(textStart), accent);
                content.Children().Append(lineBlock);
                firstLine = false;
                if (next == std::wstring::npos) break;
                pos = next + 1;
                continue;
            }

            size_t cursor = textStart;
            if (quotaLine) {
                size_t percentEnd = line.find(L"%", cursor);
                if (percentEnd != std::wstring::npos) {
                    size_t percentStart = percentEnd;
                    while (percentStart > cursor) {
                        wchar_t ch = line[percentStart - 1];
                        if ((ch < L'0' || ch > L'9') && ch != L'.') break;
                        percentStart--;
                    }
                    appendRun(lineBlock, line.substr(cursor, percentStart - cursor), muted);
                    appendRun(lineBlock, line.substr(percentStart, percentEnd + 1 - percentStart), duration, true);
                    cursor = percentEnd + 1;
                }
            }

            size_t inPos = line.find(L"in ", cursor);
            if (inPos != std::wstring::npos) {
                size_t durationStart = inPos + 3;
                size_t durationEnd = line.find(L" (", durationStart);
                size_t dashEnd = line.find(L" - ", durationStart);
                if (dashEnd != std::wstring::npos && (durationEnd == std::wstring::npos || dashEnd < durationEnd)) {
                    durationEnd = dashEnd;
                }
                if (durationEnd == std::wstring::npos) durationEnd = line.size();

                appendRun(lineBlock, line.substr(cursor, durationStart - cursor), muted);
                appendRun(lineBlock, line.substr(durationStart, durationEnd - durationStart), duration, true);
                appendRun(lineBlock, line.substr(durationEnd), muted);
            } else if (highlightStart != std::wstring::npos && highlightStart < highlightEnd) {
                appendRun(lineBlock, line.substr(cursor, highlightStart - cursor), muted);
                appendRun(lineBlock, line.substr(highlightStart, highlightEnd - highlightStart), duration, true);
                appendRun(lineBlock, line.substr(highlightEnd), muted);
            } else {
                appendRun(lineBlock, line.substr(cursor), muted);
            }

            content.Children().Append(lineBlock);
            firstLine = false;
            if (next == std::wstring::npos) break;
            pos = next + 1;
        }
    }

    toolTip.BorderBrush(border);
    toolTip.Content(content);
}

static void OpenUrl(PCWSTR url) {
    if (g_unloading || !url || !*url) return;
    ShellExecuteW(nullptr, L"open", url, nullptr, nullptr, SW_SHOWNORMAL);
}

static void RefreshQuota(int accountIndex) {
    if (g_unloading) return;

    if (!g_fetchThreadStarted.load(std::memory_order_acquire)) {
        g_refreshing = false;
        g_refreshAccountIndex = -1;
        PostUiUpdate();
        return;
    }

    g_refreshing = true;
    g_refreshAccountIndex = accountIndex;
    g_refreshGeneration++;
    PostUiUpdate();
    if (g_refreshEvent) SetEvent(g_refreshEvent);
}

static void OpenDashboardForAccount(int accountIndex) {
    std::wstring provider;
    {
        std::lock_guard<std::mutex> lk(g_settingsMutex);
        if (accountIndex < 0 || accountIndex >= (int)g_settings.accounts.size()) return;
        provider = g_settings.accounts[accountIndex].provider;
    }

    OpenUrl(provider == L"anthropic" ? L"https://claude.ai/settings/usage"
                                     : L"https://chatgpt.com/codex/cloud/settings/analytics#usage");
}

// Right-click menu: flip an account's show/hide state, keep at least one visible, persist the
// hidden-set to mod storage, then wake the fetch thread and refresh all taskbars. Runs on a
// taskbar UI thread (menu click); `sender` is the clicked ToggleMenuFlyoutItem (already flipped).
static void ToggleAccountVisibility(int accountIndex,
                                    winrt::Windows::Foundation::IInspectable const& sender) {
    if (g_unloading) return;

    auto toggle = sender.try_as<ToggleMenuFlyoutItem>();
    std::wstring hashes;
    bool refreshNow = false;
    {
        std::lock_guard<std::mutex> lk(g_settingsMutex);
        if (accountIndex < 0 || accountIndex >= (int)g_settings.accounts.size()) return;
        bool wantVisible = toggle ? toggle.IsChecked() : g_settings.accounts[accountIndex].hidden;

        // Refuse to hide the last visible account: there'd be no bar left to right-click.
        if (!wantVisible && !g_settings.accounts[accountIndex].hidden) {
            int visibleCount = 0;
            for (const auto& a : g_settings.accounts) {
                if (!a.hidden) visibleCount++;
            }
            if (visibleCount <= 1) {
                if (toggle) toggle.IsChecked(true);
                return;
            }
        }
        bool newHidden = !wantVisible;
        if (newHidden != g_settings.accounts[accountIndex].hidden) {
            g_settings.accounts[accountIndex].hidden = newHidden;
            if (newHidden) {
                // Hiding: bump the generation so a fetch already in flight from the pre-toggle
                // snapshot is discarded at publish (guard checks settingsGeneration), preventing a
                // refresh or red-threshold notification for the now-hidden account.
                g_settingsGeneration++;
            } else {
                // Showing: keep the existing (possibly stale) data and only re-query if it has
                // already gone stale, matching the UI's grey-out threshold. This stops repeated
                // hide/show from triggering fetches and hitting provider rate limits.
                int intervalMin = g_settings.pollMinutes;
                ULONGLONG now = NowUnixMs();
                std::lock_guard<std::mutex> lk2(g_dataMutex);
                if (accountIndex >= (int)g_data.size()) {
                    refreshNow = true;
                } else {
                    const AccountData& d = g_data[accountIndex];
                    refreshNow = d.stale || d.lastSuccessMs == 0 ||
                                 now - d.lastSuccessMs > (ULONGLONG)intervalMin * 2 * 60000;
                }
            }
        }

        wchar_t buf[24];
        for (const auto& a : g_settings.accounts) {
            if (!a.hidden) continue;
            if (!hashes.empty()) hashes += L";";
            swprintf(buf, ARRAYSIZE(buf), L"%016llx", (unsigned long long)AccountIdentityHash(a));
            hashes += buf;
        }
    }

    Wh_SetStringValue(L"hiddenAccounts", hashes.c_str());
    // RefreshQuota re-queries only this account (and posts the UI); otherwise just repaint so the
    // column collapses/reappears with its existing data without any network request.
    if (refreshNow) RefreshQuota(accountIndex);
    else PostUiUpdate();
}

/**********************************************/
//  JSON Helpers
/**********************************************/

static JsonObject GetObj(JsonObject const& o, PCWSTR name) {
    if (!o || !o.HasKey(name)) return nullptr;
    auto v = o.GetNamedValue(name);
    return v.ValueType() == JsonValueType::Object ? v.GetObject() : nullptr;
}

static double GetNum(JsonObject const& o, PCWSTR name, double def = -1) {
    if (!o || !o.HasKey(name)) return def;
    auto v = o.GetNamedValue(name);
    return v.ValueType() == JsonValueType::Number ? v.GetNumber() : def;
}

static std::wstring GetStr(JsonObject const& o, PCWSTR name) {
    if (!o || !o.HasKey(name)) return {};
    auto v = o.GetNamedValue(name);
    if (v.ValueType() != JsonValueType::String) return {};
    auto s = v.GetString();
    return std::wstring(s.c_str(), s.size());
}

static bool GetBool(JsonObject const& o, PCWSTR name) {
    if (!o || !o.HasKey(name)) return false;
    auto v = o.GetNamedValue(name);
    return v.ValueType() == JsonValueType::Boolean && v.GetBoolean();
}

static std::wstring DescribeJsonBody(const std::string& body) {
    if (body.empty()) return L"empty body";

    try {
        auto root = JsonObject::Parse(Utf8ToWide(body));
        std::wstring keys;
        int count = 0;
        for (auto const& kv : root) {
            if (count == 8) {
                keys += L", ...";
                break;
            }
            if (!keys.empty()) keys += L", ";
            auto key = kv.Key();
            keys += std::wstring(key.c_str(), key.size());
            count++;
        }
        return keys.empty() ? L"JSON object with no keys" : L"keys: " + keys;
    } catch (...) {
        return L"non-object or invalid JSON body";
    }
}

/**********************************************/
//  Crypto and Encoding
/**********************************************/

static std::string Base64Encode(const BYTE* data, size_t len) {
    if (!len) return {};
    DWORD outLen = 0;
    if (!CryptBinaryToStringA(data, (DWORD)len, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
                              nullptr, &outLen) || !outLen) {
        return {};
    }
    std::string out(outLen, '\0');
    if (!CryptBinaryToStringA(data, (DWORD)len, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
                              out.data(), &outLen)) {
        return {};
    }
    out.resize(outLen);
    return out;
}

static std::vector<BYTE> Base64Decode(const std::string& s) {
    if (s.empty()) return {};
    DWORD binLen = 0;
    if (!CryptStringToBinaryA(s.data(), (DWORD)s.size(), CRYPT_STRING_BASE64, nullptr, &binLen,
                              nullptr, nullptr) || !binLen) {
        return {};
    }
    std::vector<BYTE> out(binLen);
    if (!CryptStringToBinaryA(s.data(), (DWORD)s.size(), CRYPT_STRING_BASE64, out.data(), &binLen,
                              nullptr, nullptr)) {
        return {};
    }
    out.resize(binLen);
    return out;
}

// RFC 7636 base64url (no padding). Used for PKCE and to decode JWT segments.
static std::string ToBase64Url(const BYTE* data, size_t len) {
    std::string b64 = Base64Encode(data, len);
    std::string out;
    out.reserve(b64.size());
    for (char c : b64) {
        if (c == '+') out += '-';
        else if (c == '/') out += '_';
        else if (c == '=') continue;
        else out += c;
    }
    return out;
}

static std::vector<BYTE> FromBase64Url(std::string s) {
    for (char& c : s) {
        if (c == '-') c = '+';
        else if (c == '_') c = '/';
    }
    s.append((4 - s.size() % 4) % 4, '=');
    return Base64Decode(s);
}

static std::string RandomBase64Url(size_t bytes) {
    std::vector<BYTE> buf(bytes);
    if (BCryptGenRandom(nullptr, buf.data(), (ULONG)buf.size(),
                        BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0) {
        return {};
    }
    return ToBase64Url(buf.data(), buf.size());
}

// PKCE code challenge: base64url(SHA-256(verifier)).
static std::string Sha256Base64Url(const std::string& input) {
    BCRYPT_ALG_HANDLE alg = nullptr;
    if (BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr, 0) != 0) return {};
    std::string out;
    BCRYPT_HASH_HANDLE hash = nullptr;
    std::array<BYTE, 32> digest{};
    if (BCryptCreateHash(alg, &hash, nullptr, 0, nullptr, 0, 0) == 0) {
        if (BCryptHashData(hash, (PUCHAR)input.data(), (ULONG)input.size(), 0) == 0 &&
            BCryptFinishHash(hash, digest.data(), (ULONG)digest.size(), 0) == 0) {
            out = ToBase64Url(digest.data(), digest.size());
        }
        BCryptDestroyHash(hash);
    }
    BCryptCloseAlgorithmProvider(alg, 0);
    return out;
}

// Decodes a JWT's payload segment into a JSON object (claims). Empty on any failure.
static JsonObject ParseJwtPayload(const std::wstring& jwt) {
    std::string narrow = WideToUtf8(jwt);
    size_t first = narrow.find('.');
    if (first == std::string::npos) return nullptr;
    size_t second = narrow.find('.', first + 1);
    std::string payload = narrow.substr(first + 1,
        second == std::string::npos ? std::string::npos : second - first - 1);
    std::vector<BYTE> bytes = FromBase64Url(payload);
    if (bytes.empty()) return nullptr;
    try {
        return JsonObject::Parse(Utf8ToWide(std::string((char*)bytes.data(), bytes.size())));
    } catch (...) {
        return nullptr;
    }
}

// DPAPI (current user) so stored tokens are not plaintext at rest. Returns base64 of the
// protected blob, or empty on failure.
static std::string DpapiProtect(const std::string& plain) {
    DATA_BLOB in{(DWORD)plain.size(), (BYTE*)plain.data()};
    DATA_BLOB out{};
    if (!CryptProtectData(&in, L"taskbar-ai-quota", nullptr, nullptr, nullptr, 0, &out)) {
        return {};
    }
    std::string b64 = Base64Encode(out.pbData, out.cbData);
    LocalFree(out.pbData);
    return b64;
}

static std::string DpapiUnprotect(const std::string& b64) {
    std::vector<BYTE> blob = Base64Decode(b64);
    if (blob.empty()) return {};
    DATA_BLOB in{(DWORD)blob.size(), blob.data()};
    DATA_BLOB out{};
    if (!CryptUnprotectData(&in, nullptr, nullptr, nullptr, nullptr, 0, &out)) {
        return {};
    }
    std::string plain((char*)out.pbData, out.cbData);
    LocalFree(out.pbData);
    return plain;
}

/**********************************************/
//  Token Store
/**********************************************/

// Per-account OAuth credentials, owned entirely by the mod (the CLI credential files are
// never touched). Persisted DPAPI-encrypted in Windhawk mod storage, keyed by identity hash.
struct StoredToken {
    std::wstring accessToken;
    std::wstring refreshToken;
    std::wstring accountId;  // OpenAI ChatGPT-Account-Id; empty for Anthropic.
    ULONGLONG expiresMs = 0;  // 0 = unknown; refresh is then driven reactively by 401s.
};

static std::mutex g_authMutex;
static std::mutex g_authEpochMutex;
static std::vector<std::pair<uint64_t, ULONGLONG>> g_authEpochs;

enum class TokenSaveResult {
    Saved,
    Stale,
    Failed,
};

static ULONGLONG AuthEpochLocked(uint64_t idHash) {
    for (const auto& [hash, epoch] : g_authEpochs) {
        if (hash == idHash) return epoch;
    }
    return 0;
}

static ULONGLONG CurrentAuthEpoch(uint64_t idHash) {
    std::lock_guard<std::mutex> lk(g_authEpochMutex);
    return AuthEpochLocked(idHash);
}

static std::wstring TokenStorageKey(uint64_t idHash) {
    wchar_t buf[32];
    swprintf(buf, ARRAYSIZE(buf), L"auth_%016llx", (unsigned long long)idHash);
    return buf;
}

static bool LoadStoredToken(uint64_t idHash, StoredToken* out) {
    std::lock_guard<std::mutex> lk(g_authMutex);
    std::vector<wchar_t> buf(16384);
    Wh_GetStringValue(TokenStorageKey(idHash).c_str(), buf.data(), buf.size());
    std::wstring stored = buf.data();
    if (stored.empty()) return false;

    std::string plain = DpapiUnprotect(WideToUtf8(stored));
    if (plain.empty()) return false;
    try {
        auto root = JsonObject::Parse(Utf8ToWide(plain));
        out->accessToken = GetStr(root, L"access");
        out->refreshToken = GetStr(root, L"refresh");
        out->accountId = GetStr(root, L"accountId");
        out->expiresMs = (ULONGLONG)GetNum(root, L"expiresMs", 0);
        return !out->accessToken.empty() || !out->refreshToken.empty();
    } catch (...) {
        return false;
    }
}

static bool SaveStoredToken(uint64_t idHash, const StoredToken& t) {
    std::lock_guard<std::mutex> lk(g_authMutex);
    std::wstring json;
    try {
        JsonObject root;
        root.SetNamedValue(L"access", JsonValue::CreateStringValue(winrt::hstring(t.accessToken)));
        root.SetNamedValue(L"refresh", JsonValue::CreateStringValue(winrt::hstring(t.refreshToken)));
        root.SetNamedValue(L"accountId", JsonValue::CreateStringValue(winrt::hstring(t.accountId)));
        root.SetNamedValue(L"expiresMs", JsonValue::CreateNumberValue((double)t.expiresMs));
        json = root.Stringify().c_str();
    } catch (...) {
        return false;
    }
    std::string b64 = DpapiProtect(WideToUtf8(json));
    if (b64.empty()) return false;
    return Wh_SetStringValue(TokenStorageKey(idHash).c_str(), Utf8ToWide(b64).c_str());
}

static void ClearStoredToken(uint64_t idHash) {
    std::lock_guard<std::mutex> lk(g_authMutex);
    Wh_SetStringValue(TokenStorageKey(idHash).c_str(), L"");
}

static TokenSaveResult SaveStoredTokenIfCurrent(uint64_t idHash, ULONGLONG authEpoch,
                                                const StoredToken& t) {
    std::lock_guard<std::mutex> lk(g_authEpochMutex);
    if (AuthEpochLocked(idHash) != authEpoch) return TokenSaveResult::Stale;
    return SaveStoredToken(idHash, t) ? TokenSaveResult::Saved : TokenSaveResult::Failed;
}

static void ClearStoredTokenAndBumpAuthEpoch(uint64_t idHash) {
    std::lock_guard<std::mutex> lk(g_authEpochMutex);
    for (auto& [hash, epoch] : g_authEpochs) {
        if (hash == idHash) {
            epoch++;
            ClearStoredToken(idHash);
            return;
        }
    }
    g_authEpochs.push_back({idHash, 1});
    ClearStoredToken(idHash);
}

/**********************************************/
//  HTTP
/**********************************************/

struct HttpResult {
    bool ok = false;
    int status = 0;
    int retryAfterSec = 0;
    std::string body;
};

static bool TrackHttpHandle(HINTERNET h) {
    if (!h) return false;
    std::lock_guard<std::mutex> lk(g_httpHandlesMutex);
    if (g_unloading) {
        WinHttpCloseHandle(h);
        return false;
    }
    g_httpHandles.push_back(h);
    return true;
}

static bool UntrackHttpHandle(HINTERNET h) {
    std::lock_guard<std::mutex> lk(g_httpHandlesMutex);
    auto it = std::find(g_httpHandles.begin(), g_httpHandles.end(), h);
    if (it == g_httpHandles.end()) return false;
    g_httpHandles.erase(it);
    return true;
}

static void CloseActiveHttpHandles() {
    std::vector<HINTERNET> handles;
    {
        std::lock_guard<std::mutex> lk(g_httpHandlesMutex);
        handles.swap(g_httpHandles);
    }
    for (auto it = handles.rbegin(); it != handles.rend(); ++it) WinHttpCloseHandle(*it);
}

static HttpResult HttpRequest(PCWSTR method, PCWSTR host, PCWSTR path, PCWSTR userAgent,
                              const std::wstring& headers, const std::string& body = {}) {
    HttpResult res;
    HINTERNET ses = WinHttpOpen(userAgent, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                                WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS,
                                0);
    if (ses && !TrackHttpHandle(ses)) ses = nullptr;
    HINTERNET con = nullptr;
    HINTERNET req = nullptr;

    if (ses && !g_unloading) {
        WinHttpSetTimeouts(ses, 5000, 5000, 5000, 10000);
        con = WinHttpConnect(ses, host, INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (con && !TrackHttpHandle(con)) con = nullptr;
        req = con ? WinHttpOpenRequest(con, method, path, nullptr,
                                        WINHTTP_NO_REFERER,
                                        WINHTTP_DEFAULT_ACCEPT_TYPES,
                                        WINHTTP_FLAG_SECURE)
                  : nullptr;
        if (req && !TrackHttpHandle(req)) req = nullptr;
    }

    if (!g_unloading && req &&
        WinHttpSendRequest(req, headers.c_str(), (DWORD)headers.size(),
                           body.empty() ? WINHTTP_NO_REQUEST_DATA : (LPVOID)body.data(),
                           (DWORD)body.size(), (DWORD)body.size(), 0) &&
        WinHttpReceiveResponse(req, nullptr)) {
        DWORD status = 0, sz = sizeof(status);
        WinHttpQueryHeaders(req, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                            WINHTTP_HEADER_NAME_BY_INDEX, &status, &sz,
                            WINHTTP_NO_HEADER_INDEX);
        res.status = (int)status;

        wchar_t ra[128]{};
        DWORD raSz = sizeof(ra);
        if (WinHttpQueryHeaders(req, WINHTTP_QUERY_RETRY_AFTER,
                                WINHTTP_HEADER_NAME_BY_INDEX, ra, &raSz,
                                WINHTTP_NO_HEADER_INDEX)) {
            res.retryAfterSec = _wtoi(ra);
            if (res.retryAfterSec <= 0) {
                SYSTEMTIME st{};
                FILETIME ft{};
                if (WinHttpTimeToSystemTime(ra, &st) && SystemTimeToFileTime(&st, &ft)) {
                    ULONGLONG t = ((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
                    ULONGLONG fileMs = t / 10000;
                    if (fileMs > kFileTimeUnixEpochOffsetMs) {
                        ULONGLONG retryUnixMs = fileMs - kFileTimeUnixEpochOffsetMs;
                        ULONGLONG now = NowUnixMs();
                        if (retryUnixMs > now) {
                            ULONGLONG deltaSec = (retryUnixMs - now + 999) / 1000;
                            res.retryAfterSec = (int)std::min(deltaSec, 24ULL * 60 * 60);
                        }
                    }
                }
            }
        }

        bool bodyOk = true;
        for (;;) {
            DWORD available = 0;
            if (!WinHttpQueryDataAvailable(req, &available)) {
                bodyOk = false;
                break;
            }
            if (!available) break;

            size_t prev = res.body.size();
            res.body.resize(prev + available);
            DWORD read = 0;
            if (!WinHttpReadData(req, res.body.data() + prev, available, &read)) {
                res.body.resize(prev);
                bodyOk = false;
                break;
            }
            res.body.resize(prev + read);
            if (!read || res.body.size() > 4 * 1024 * 1024) break;
        }
        res.ok = bodyOk;
    }

    if (req && UntrackHttpHandle(req)) WinHttpCloseHandle(req);
    if (con && UntrackHttpHandle(con)) WinHttpCloseHandle(con);
    if (ses && UntrackHttpHandle(ses)) WinHttpCloseHandle(ses);
    return res;
}

/**********************************************/
//  OAuth
/**********************************************/

// Public OAuth clients used by the official CLIs; the mod runs the same flows so a sign-in
// here is independent of (and never touches) OpenCode/Claude Code/Codex credential files.
static constexpr PCWSTR kAnthropicClientId = L"9d1c250a-e61b-44d9-88ed-5944d1962f5e";
static constexpr PCWSTR kAnthropicTokenHost = L"console.anthropic.com";
static constexpr PCWSTR kAnthropicTokenPath = L"/v1/oauth/token";
static constexpr PCWSTR kAnthropicRedirect = L"https://console.anthropic.com/oauth/code/callback";
static constexpr PCWSTR kAnthropicScope = L"org:create_api_key user:profile user:inference";
static constexpr PCWSTR kOpenAiClientId = L"app_EMoamEEZ73f0CkXaXp7hrann";
static constexpr PCWSTR kOpenAiTokenHost = L"auth.openai.com";
static constexpr PCWSTR kOpenAiTokenPath = L"/oauth/token";
static constexpr PCWSTR kOpenAiScope =
    L"openid profile email offline_access api.connectors.read api.connectors.invoke";
static constexpr PCWSTR kOAuthUserAgent = L"taskbar-ai-quota/0.1";

static std::string UrlEncode(const std::string& s) {
    static const char* kHex = "0123456789ABCDEF";
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            out += (char)c;
        } else {
            out += '%';
            out += kHex[c >> 4];
            out += kHex[c & 0xF];
        }
    }
    return out;
}

static std::string UrlDecode(const std::string& s) {
    auto hex = [](char h) -> int {
        if (h >= '0' && h <= '9') return h - '0';
        if (h >= 'a' && h <= 'f') return h - 'a' + 10;
        if (h >= 'A' && h <= 'F') return h - 'A' + 10;
        return -1;
    };
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if (c == '%' && i + 2 < s.size()) {
            int hi = hex(s[i + 1]), lo = hex(s[i + 2]);
            if (hi >= 0 && lo >= 0) {
                out += (char)((hi << 4) | lo);
                i += 2;
                continue;
            }
        }
        out += (c == '+') ? ' ' : c;
    }
    return out;
}

static std::wstring BuildAnthropicAuthorizeUrl(const std::string& challenge, const std::string& state) {
    std::string url = "https://claude.ai/oauth/authorize?code=true";
    url += "&client_id=" + UrlEncode(WideToUtf8(kAnthropicClientId));
    url += "&response_type=code";
    url += "&redirect_uri=" + UrlEncode(WideToUtf8(kAnthropicRedirect));
    url += "&scope=" + UrlEncode(WideToUtf8(kAnthropicScope));
    url += "&code_challenge=" + challenge;
    url += "&code_challenge_method=S256";
    url += "&state=" + state;
    return Utf8ToWide(url);
}

static std::wstring BuildOpenAiAuthorizeUrl(int port, const std::string& challenge,
                                            const std::string& state) {
    std::string redirect = "http://localhost:" + std::to_string(port) + "/auth/callback";
    std::string url = "https://auth.openai.com/oauth/authorize?response_type=code";
    url += "&client_id=" + UrlEncode(WideToUtf8(kOpenAiClientId));
    url += "&redirect_uri=" + UrlEncode(redirect);
    url += "&scope=" + UrlEncode(WideToUtf8(kOpenAiScope));
    url += "&code_challenge=" + challenge;
    url += "&code_challenge_method=S256";
    url += "&id_token_add_organizations=true";
    url += "&codex_cli_simplified_flow=true";
    url += "&originator=codex_cli_rs";
    url += "&state=" + state;
    return Utf8ToWide(url);
}

static std::wstring ParseOAuthError(const std::string& body) {
    try {
        auto root = JsonObject::Parse(Utf8ToWide(body));
        std::wstring desc = GetStr(root, L"error_description");
        if (!desc.empty()) return desc;
        return GetStr(root, L"error");
    } catch (...) {
        return {};
    }
}

// Fills *tok from an OAuth token response (access/refresh/id_token), extracting the OpenAI
// ChatGPT-Account-Id and access-token expiry from JWT claims when present. Used for both the
// authorization-code exchange and refresh.
static bool ParseTokenResponse(const std::string& body, bool anthropic, StoredToken* tok,
                               std::wstring* err) {
    try {
        auto root = JsonObject::Parse(Utf8ToWide(body));
        std::wstring access = GetStr(root, L"access_token");
        std::wstring refresh = GetStr(root, L"refresh_token");
        if (access.empty()) {
            *err = L"no access token in response";
            return false;
        }
        tok->accessToken = access;
        if (!refresh.empty()) tok->refreshToken = refresh;

        tok->expiresMs = 0;
        double expiresIn = GetNum(root, L"expires_in", 0);
        if (expiresIn > 0) tok->expiresMs = NowUnixMs() + (ULONGLONG)(expiresIn * 1000);

        if (!anthropic) {
            std::wstring idToken = GetStr(root, L"id_token");
            if (auto claims = ParseJwtPayload(idToken.empty() ? access : idToken)) {
                if (auto authObj = GetObj(claims, L"https://api.openai.com/auth")) {
                    std::wstring acc = GetStr(authObj, L"chatgpt_account_id");
                    if (!acc.empty()) tok->accountId = acc;
                }
            }
            // OpenAI responses omit expires_in; derive expiry from the access-token JWT exp.
            if (tok->expiresMs == 0) {
                if (auto claims = ParseJwtPayload(access)) {
                    double exp = GetNum(claims, L"exp", 0);
                    if (exp > 0) tok->expiresMs = (ULONGLONG)(exp * 1000);
                }
            }
        }
        return true;
    } catch (...) {
        *err = L"invalid token response";
        return false;
    }
}

static bool PostTokenEndpoint(bool anthropic, const std::wstring& contentType,
                              const std::string& body, StoredToken* tok, std::wstring* err) {
    std::wstring headers = L"Content-Type: " + contentType + L"\r\nAccept: application/json\r\n";
    PCWSTR host = anthropic ? kAnthropicTokenHost : kOpenAiTokenHost;
    PCWSTR path = anthropic ? kAnthropicTokenPath : kOpenAiTokenPath;
    HttpResult r = HttpRequest(L"POST", host, path, kOAuthUserAgent, headers, body);
    if (!r.ok) {
        *err = L"network error";
        return false;
    }
    if (r.status < 200 || r.status >= 300) {
        std::wstring detail = ParseOAuthError(r.body);
        *err = detail.empty() ? (L"HTTP " + std::to_wstring(r.status)) : detail;
        return false;
    }
    return ParseTokenResponse(r.body, anthropic, tok, err);
}

// grant_type=refresh_token. Anthropic sends JSON; OpenAI sends JSON too (its auth-code
// exchange is the form-encoded one). Rotated refresh tokens come back in the response.
static bool RefreshToken(const std::wstring& provider, StoredToken* tok, std::wstring* err) {
    if (tok->refreshToken.empty()) {
        *err = L"no refresh token";
        return false;
    }
    bool anthropic = provider == L"anthropic";
    std::string body;
    try {
        JsonObject obj;
        obj.SetNamedValue(L"grant_type", JsonValue::CreateStringValue(L"refresh_token"));
        obj.SetNamedValue(L"refresh_token",
                          JsonValue::CreateStringValue(winrt::hstring(tok->refreshToken)));
        obj.SetNamedValue(L"client_id", JsonValue::CreateStringValue(
                                            winrt::hstring(anthropic ? kAnthropicClientId : kOpenAiClientId)));
        body = WideToUtf8(std::wstring(obj.Stringify().c_str()));
    } catch (...) {
        *err = L"internal error";
        return false;
    }
    return PostTokenEndpoint(anthropic, L"application/json", body, tok, err);
}

/**********************************************/
//  Login
/**********************************************/

struct LoginRequest {
    std::wstring provider;
    std::wstring label;
    uint64_t idHash = 0;
    ULONGLONG authEpoch = 0;
    int index = -1;
};

static std::atomic<bool> g_loginInProgress{false};
static HANDLE g_loginThread = nullptr;
static std::mutex g_loginThreadMutex;  // Guards g_loginThread handoff vs. the unload join.
static std::atomic<HWND> g_loginWnd{nullptr};        // Anthropic paste dialog window.
static std::atomic<SOCKET> g_loginSocket{INVALID_SOCKET};  // OpenAI loopback listener.

// Small modal-style input window so the Anthropic flow can collect the pasted code#state.
// Runs its own message loop on the login thread; closed on cancel, submit, or unload.
struct LoginDialogState {
    std::wstring result;
    HWND edit = nullptr;
    bool ok = false;
    bool done = false;
};

static LRESULT CALLBACK LoginDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            auto* st = reinterpret_cast<LoginDialogState*>(cs->lpCreateParams);
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(st));
            HINSTANCE hInst = GetModuleHandleW(nullptr);
            HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            HWND label = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT,
                                         12, 10, 424, 52, hWnd, (HMENU)100, hInst, nullptr);
            HWND edit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                                        12, 68, 424, 24, hWnd, (HMENU)101, hInst, nullptr);
            HWND ok = CreateWindowExW(0, L"BUTTON", L"Sign in",
                                      WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                                      262, 104, 84, 30, hWnd, (HMENU)IDOK, hInst, nullptr);
            HWND cancel = CreateWindowExW(0, L"BUTTON", L"Cancel",
                                          WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                          352, 104, 84, 30, hWnd, (HMENU)IDCANCEL, hInst, nullptr);
            for (HWND c : {label, edit, ok, cancel}) {
                SendMessageW(c, WM_SETFONT, (WPARAM)font, TRUE);
            }
            if (st) st->edit = edit;
            SetFocus(edit);
            return 0;
        }
        case WM_COMMAND: {
            auto* st = reinterpret_cast<LoginDialogState*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
            WORD id = LOWORD(wParam);
            if (id == IDOK && st && st->edit) {
                int len = GetWindowTextLengthW(st->edit);
                std::wstring buf(len + 1, L'\0');
                GetWindowTextW(st->edit, buf.data(), len + 1);
                buf.resize(len);
                st->result = std::move(buf);
                st->ok = true;
                DestroyWindow(hWnd);
                return 0;
            }
            if (id == IDCANCEL) {
                DestroyWindow(hWnd);
                return 0;
            }
            break;
        }
        case WM_CLOSE:
            DestroyWindow(hWnd);
            return 0;
        case WM_DESTROY: {
            auto* st = reinterpret_cast<LoginDialogState*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
            if (st) st->done = true;
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

static std::wstring ShowLoginInputDialog(const std::wstring& title, const std::wstring& instructions) {
    static PCWSTR kClass = L"AiQuotaLoginDlg_" WH_MOD_ID;
    HINSTANCE hInst = GetModuleHandleW(nullptr);
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = LoginDlgProc;
    wc.hInstance = hInst;
    wc.lpszClassName = kClass;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClassExW(&wc);  // ERROR_CLASS_ALREADY_EXISTS is fine.

    LoginDialogState st;
    int w = 460, h = 184;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
    HWND wnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_DLGMODALFRAME, kClass, title.c_str(),
                               WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, x, y, w, h,
                               nullptr, nullptr, hInst, &st);
    if (!wnd) return {};
    SetWindowTextW(GetDlgItem(wnd, 100), instructions.c_str());
    g_loginWnd.store(wnd);

    MSG msg;
    while (!g_unloading && GetMessageW(&msg, nullptr, 0, 0)) {
        if (!IsDialogMessageW(wnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (st.done) break;
    }
    g_loginWnd.store(nullptr);
    if (IsWindow(wnd)) DestroyWindow(wnd);
    // Unregister so a later mod reload can't reuse a class pointing at this now-unloaded WndProc.
    UnregisterClassW(kClass, hInst);
    return st.ok ? st.result : std::wstring();
}

static bool StartLoopback(SOCKET* outSock, int* outPort) {
    for (int port : {1455, 1457}) {
        SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s == INVALID_SOCKET) continue;
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons((u_short)port);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (bind(s, (sockaddr*)&addr, sizeof(addr)) == 0 && listen(s, 1) == 0) {
            *outSock = s;
            *outPort = port;
            return true;
        }
        closesocket(s);
    }
    return false;
}

// Accepts one localhost callback and returns the (URL-decoded) authorization code. Polls with
// a 1s select timeout so it stays responsive to g_unloading and an overall deadline.
static std::string WaitForLoopbackCode(SOCKET listener, const std::string& expectedState,
                                       DWORD totalTimeoutMs, std::wstring* terminalError) {
    if (terminalError) terminalError->clear();
    auto getParam = [](const std::string& query, const std::string& key) -> std::string {
        std::string pat = key + "=";
        for (size_t p = 0; (p = query.find(pat, p)) != std::string::npos; p += pat.size()) {
            if (p == 0 || query[p - 1] == '&' || query[p - 1] == '?') {
                size_t v = p + pat.size();
                size_t end = query.find('&', v);
                return UrlDecode(query.substr(v, end == std::string::npos ? std::string::npos : end - v));
            }
        }
        return {};
    };

    ULONGLONG deadline = GetTickCount64() + totalTimeoutMs;
    while (!g_unloading) {
        fd_set rd;
        FD_ZERO(&rd);
        FD_SET(listener, &rd);
        timeval tv{1, 0};
        int sel = select(0, &rd, nullptr, nullptr, &tv);
        if (g_unloading) break;
        if (sel <= 0) {
            if (GetTickCount64() > deadline) break;
            continue;
        }

        SOCKET c = accept(listener, nullptr, nullptr);
        if (c == INVALID_SOCKET) {
            if (g_unloading) break;
            continue;
        }

        DWORD rcvTimeout = 3000;
        setsockopt(c, SOL_SOCKET, SO_RCVTIMEO, (char*)&rcvTimeout, sizeof(rcvTimeout));
        std::string request;
        char buf[2048];
        for (int i = 0; i < 16; i++) {
            int n = recv(c, buf, sizeof(buf), 0);
            if (n <= 0) break;
            request.append(buf, n);
            if (request.find("\r\n\r\n") != std::string::npos || request.size() > 16384) break;
        }

        std::string path;
        size_t sp1 = request.find(' ');
        size_t sp2 = sp1 == std::string::npos ? std::string::npos : request.find(' ', sp1 + 1);
        if (sp1 != std::string::npos && sp2 != std::string::npos) {
            path = request.substr(sp1 + 1, sp2 - sp1 - 1);
        }
        size_t q = path.find('?');
        std::string route = q == std::string::npos ? path : path.substr(0, q);
        std::string query = q == std::string::npos ? "" : path.substr(q + 1);
        std::string code = getParam(query, "code");
        std::string state = getParam(query, "state");
        std::string oauthError = getParam(query, "error");
        std::string oauthErrorDesc = getParam(query, "error_description");
        bool callback = route == "/auth/callback";
        bool stateOk = callback && (expectedState.empty() || state == expectedState);
        bool success = !code.empty() && stateOk;
        bool terminalFailure = stateOk && (code.empty() || !oauthError.empty());

        std::string html = success
            ? "<!doctype html><meta charset='utf-8'><body style='font-family:sans-serif;padding:2em'>"
              "<h3>Signed in.</h3><p>You can close this tab and return to the taskbar.</p></body>"
            : "<!doctype html><meta charset='utf-8'><body style='font-family:sans-serif;padding:2em'>"
              "<h3>Sign-in failed.</h3><p>You can close this tab and try again from the taskbar.</p></body>";
        std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n"
                           "Connection: close\r\nContent-Length: " + std::to_string(html.size()) +
                           "\r\n\r\n" + html;
        send(c, resp.data(), (int)resp.size(), 0);
        closesocket(c);

        if (success) return code;
        if (terminalFailure) {
            if (terminalError) {
                if (!oauthErrorDesc.empty()) *terminalError = Utf8ToWide(oauthErrorDesc);
                else if (!oauthError.empty()) *terminalError = Utf8ToWide(oauthError);
                else *terminalError = L"missing authorization code";
            }
            return {};
        }
        // Ignore stray hits (favicon, etc.) and keep waiting until the deadline.
        if (GetTickCount64() > deadline) break;
    }
    return {};
}

static void DoAnthropicLogin(const LoginRequest& req) {
    std::string verifier = RandomBase64Url(32);
    std::string challenge = Sha256Base64Url(verifier);
    std::string state = RandomBase64Url(32);
    if (verifier.empty() || challenge.empty() || state.empty()) {
        Wh_Log(L"Sign-in: crypto init failed");
        return;
    }

    OpenUrl(BuildAnthropicAuthorizeUrl(challenge, state).c_str());
    std::wstring title = L"Sign in: " + req.label + L" (Anthropic)";
    std::wstring instructions =
        L"A browser window opened to claude.ai. Approve access, then copy the code shown "
        L"on the page and paste it below.";
    std::wstring pasted = ShowLoginInputDialog(title, instructions);
    if (g_unloading || pasted.empty()) return;

    size_t b = pasted.find_first_not_of(L" \t\r\n");
    size_t e = pasted.find_last_not_of(L" \t\r\n");
    std::wstring code = b == std::wstring::npos ? L"" : pasted.substr(b, e - b + 1);
    size_t hashPos = code.find(L'#');  // pasted value is code#state.
    if (hashPos != std::wstring::npos) code = code.substr(0, hashPos);
    if (code.empty()) {
        Wh_Log(L"Sign-in [%s]: no code entered", req.label.c_str());
        return;
    }

    std::string body;
    try {
        JsonObject obj;
        obj.SetNamedValue(L"grant_type", JsonValue::CreateStringValue(L"authorization_code"));
        obj.SetNamedValue(L"client_id", JsonValue::CreateStringValue(winrt::hstring(kAnthropicClientId)));
        obj.SetNamedValue(L"code", JsonValue::CreateStringValue(winrt::hstring(code)));
        obj.SetNamedValue(L"redirect_uri", JsonValue::CreateStringValue(winrt::hstring(kAnthropicRedirect)));
        obj.SetNamedValue(L"code_verifier", JsonValue::CreateStringValue(winrt::hstring(Utf8ToWide(verifier))));
        obj.SetNamedValue(L"state", JsonValue::CreateStringValue(winrt::hstring(Utf8ToWide(state))));
        body = WideToUtf8(std::wstring(obj.Stringify().c_str()));
    } catch (...) {
        Wh_Log(L"Sign-in [%s]: internal error", req.label.c_str());
        return;
    }

    StoredToken tok;
    std::wstring err;
    if (PostTokenEndpoint(/*anthropic*/ true, L"application/json", body, &tok, &err)) {
        TokenSaveResult saved = SaveStoredTokenIfCurrent(req.idHash, req.authEpoch, tok);
        if (saved == TokenSaveResult::Stale) {
            Wh_Log(L"Sign-in [%s]: cancelled before saving token", req.label.c_str());
            return;
        }
        if (saved != TokenSaveResult::Saved) {
            Wh_Log(L"Sign-in [%s] failed: could not save token", req.label.c_str());
            return;
        }
        RefreshQuota(req.index);
        Wh_Log(L"Sign-in [%s]: success", req.label.c_str());
    } else {
        Wh_Log(L"Sign-in [%s] failed: %s", req.label.c_str(), err.c_str());
    }
}

static void DoOpenAiLogin(const LoginRequest& req) {
    std::string verifier = RandomBase64Url(32);
    std::string challenge = Sha256Base64Url(verifier);
    std::string state = RandomBase64Url(32);
    if (verifier.empty() || challenge.empty() || state.empty()) {
        Wh_Log(L"Sign-in: crypto init failed");
        return;
    }

    SOCKET listener = INVALID_SOCKET;
    int port = 0;
    if (!StartLoopback(&listener, &port)) {
        Wh_Log(L"Sign-in [%s]: could not bind localhost:1455/1457 (Codex running?)",
               req.label.c_str());
        return;
    }
    g_loginSocket.store(listener);
    OpenUrl(BuildOpenAiAuthorizeUrl(port, challenge, state).c_str());
    std::wstring callbackErr;
    std::string code = WaitForLoopbackCode(listener, state, 180000, &callbackErr);
    SOCKET s = g_loginSocket.exchange(INVALID_SOCKET);
    if (s != INVALID_SOCKET) closesocket(s);
    if (g_unloading || code.empty()) {
        if (code.empty() && !g_unloading) {
            if (!callbackErr.empty()) {
                Wh_Log(L"Sign-in [%s] failed: %s", req.label.c_str(), callbackErr.c_str());
            } else {
                Wh_Log(L"Sign-in [%s]: cancelled or timed out", req.label.c_str());
            }
        }
        return;
    }

    std::string redirect = "http://localhost:" + std::to_string(port) + "/auth/callback";
    std::string formBody = "grant_type=authorization_code&code=" + UrlEncode(code) +
                           "&redirect_uri=" + UrlEncode(redirect) +
                           "&client_id=" + UrlEncode(WideToUtf8(kOpenAiClientId)) +
                           "&code_verifier=" + UrlEncode(verifier);

    StoredToken tok;
    std::wstring err;
    if (PostTokenEndpoint(/*anthropic*/ false, L"application/x-www-form-urlencoded", formBody,
                          &tok, &err)) {
        TokenSaveResult saved = SaveStoredTokenIfCurrent(req.idHash, req.authEpoch, tok);
        if (saved == TokenSaveResult::Stale) {
            Wh_Log(L"Sign-in [%s]: cancelled before saving token", req.label.c_str());
            return;
        }
        if (saved != TokenSaveResult::Saved) {
            Wh_Log(L"Sign-in [%s] failed: could not save token", req.label.c_str());
            return;
        }
        RefreshQuota(req.index);
        Wh_Log(L"Sign-in [%s]: success", req.label.c_str());
    } else {
        Wh_Log(L"Sign-in [%s] failed: %s", req.label.c_str(), err.c_str());
    }
}

static DWORD WINAPI LoginThreadProc(LPVOID param) {
    std::unique_ptr<LoginRequest> req(reinterpret_cast<LoginRequest*>(param));
    bool apartmentInitialized = false;
    try {
        winrt::init_apartment(winrt::apartment_type::multi_threaded);
        apartmentInitialized = true;
    } catch (...) {}
    try {
        if (!g_unloading) {
            if (req->provider == L"anthropic") DoAnthropicLogin(*req);
            else DoOpenAiLogin(*req);
        }
    } catch (...) {
        Wh_Log(L"Sign-in: exception");
    }
    if (apartmentInitialized) winrt::uninit_apartment();
    g_loginInProgress.store(false);
    return 0;
}

// Kicks off a sign-in on a dedicated thread (browser + paste dialog or loopback are blocking).
// One at a time; runs on a taskbar UI thread (menu click).
static void StartLogin(int accountIndex) {
    if (g_unloading) return;
    bool expected = false;
    if (!g_loginInProgress.compare_exchange_strong(expected, true)) return;

    auto* req = new LoginRequest();
    {
        std::lock_guard<std::mutex> lk(g_settingsMutex);
        if (accountIndex < 0 || accountIndex >= (int)g_settings.accounts.size()) {
            delete req;
            g_loginInProgress.store(false);
            return;
        }
        const AccountConfig& a = g_settings.accounts[accountIndex];
        req->provider = a.provider;
        req->label = a.label;
        req->idHash = AccountIdentityHash(a);
        req->index = accountIndex;
    }
    req->authEpoch = CurrentAuthEpoch(req->idHash);

    // Hand off g_loginThread under the lock and re-check g_unloading: Wh_ModUninit sets
    // g_unloading before joining under the same lock, so we never spawn a thread into an
    // unloading DLL or leave a handle the join would miss.
    std::lock_guard<std::mutex> lk(g_loginThreadMutex);
    if (g_unloading) {
        delete req;
        g_loginInProgress.store(false);
        return;
    }
    // A prior login thread has already cleared g_loginInProgress; release its handle before reuse.
    if (g_loginThread) {
        CloseHandle(g_loginThread);
        g_loginThread = nullptr;
    }
    g_loginThread = CreateThread(nullptr, 0, LoginThreadProc, req, 0, nullptr);
    if (!g_loginThread) {
        delete req;
        g_loginInProgress.store(false);
    }
}

static void SignOutAccount(int accountIndex) {
    if (g_unloading) return;
    uint64_t idHash;
    {
        std::lock_guard<std::mutex> lk(g_settingsMutex);
        if (accountIndex < 0 || accountIndex >= (int)g_settings.accounts.size()) return;
        idHash = AccountIdentityHash(g_settings.accounts[accountIndex]);
    }
    ClearStoredTokenAndBumpAuthEpoch(idHash);
    RefreshQuota(accountIndex);  // re-fetch so the column flips to "not signed in".
}

/**********************************************/
//  Usage Parsers
/**********************************************/

static bool ParseAnthropicUsage(const std::string& body, AccountData* d, std::wstring* error) {
    try {
        auto root = JsonObject::Parse(Utf8ToWide(body));
        JsonObject usage = root;
        if (!GetObj(usage, L"five_hour") && !GetObj(usage, L"seven_day")) {
            if (auto wrapped = GetObj(root, L"usage")) usage = wrapped;
            else if (auto wrapped = GetObj(root, L"data")) usage = wrapped;
        }

        if (auto fh = GetObj(usage, L"five_hour")) {
            d->win5h.pct = GetNum(fh, L"utilization");
            d->win5h.resetUnixMs = ParseIso8601Ms(GetStr(fh, L"resets_at"));
            d->win5h.durationSeconds = 5ULL * 60 * 60;
        }
        if (auto sd = GetObj(usage, L"seven_day")) {
            d->winWeek.pct = GetNum(sd, L"utilization");
            d->winWeek.resetUnixMs = ParseIso8601Ms(GetStr(sd, L"resets_at"));
            d->winWeek.durationSeconds = 7ULL * 24 * 60 * 60;
        }

        d->plan.clear();
        d->extraLines.clear();
        if (auto op = GetObj(usage, L"seven_day_opus"); op && GetNum(op, L"utilization") >= 0) {
            wchar_t line[64];
            swprintf(line, ARRAYSIZE(line), L"opus week: %.0f%%", GetNum(op, L"utilization"));
            d->extraLines += line;
        }
        if (auto eu = GetObj(usage, L"extra_usage"); eu && GetBool(eu, L"is_enabled")) {
            wchar_t line[96];
            swprintf(line, ARRAYSIZE(line), L"extra usage: %.1f%% monthly",
                     GetNum(eu, L"utilization", 0));
            if (!d->extraLines.empty()) d->extraLines += L"\n";
            d->extraLines += line;
        }
        bool parsed = d->win5h.pct >= 0 || d->winWeek.pct >= 0;
        if (!parsed && error) *error = L"unexpected response format (" + DescribeJsonBody(body) + L")";
        return parsed;
    } catch (...) {
        if (error) *error = L"unexpected response format (" + DescribeJsonBody(body) + L")";
        return false;
    }
}

static bool ParseOpenAiUsage(const std::string& body, AccountData* d, std::wstring* error) {
    try {
        auto root = JsonObject::Parse(Utf8ToWide(body));
        auto hasWindows = [](JsonObject const& o) -> bool {
            return GetObj(o, L"primary_window") || GetObj(o, L"secondary_window") ||
                   GetObj(o, L"primary") || GetObj(o, L"secondary") ||
                   GetObj(o, L"five_hour") || GetObj(o, L"weekly") ||
                   GetObj(o, L"five_hour_limit") || GetObj(o, L"weekly_limit");
        };

        JsonObject usage = root;
        if (!GetObj(root, L"rate_limit") && !hasWindows(root)) {
            if (auto wrapped = GetObj(root, L"usage");
                wrapped && (GetObj(wrapped, L"rate_limit") || hasWindows(wrapped))) {
                usage = wrapped;
            } else if (auto wrapped = GetObj(root, L"data");
                       wrapped && (GetObj(wrapped, L"rate_limit") || hasWindows(wrapped))) {
                usage = wrapped;
            }
        }

        auto rl = GetObj(usage, L"rate_limit");
        if (!rl && hasWindows(usage)) rl = usage;

        auto resetUnixMs = [](JsonObject const& window) -> ULONGLONG {
            double resetAt = GetNum(window, L"reset_time_ms", 0);
            if (resetAt > 0) return (ULONGLONG)resetAt;

            resetAt = GetNum(window, L"reset_at", 0);
            if (resetAt > (double)kUnixTimestampMsThreshold) return (ULONGLONG)resetAt;
            if (resetAt > 0) return (ULONGLONG)(resetAt * 1000);

            double resetAfter = GetNum(window, L"reset_after_seconds", 0);
            return resetAfter > 0 ? NowUnixMs() + (ULONGLONG)(resetAfter * 1000) : 0ULL;
        };

        auto applyWindow = [&](JsonObject const& window, WindowUsage* fallback) {
            if (!window) return;
            WindowUsage* target = fallback;
            double windowSeconds = GetNum(window, L"limit_window_seconds", 0);
            if (windowSeconds == 5 * 60 * 60) target = &d->win5h;
            else if (windowSeconds == 7 * 24 * 60 * 60) target = &d->winWeek;

            double pct = GetNum(window, L"used_percent");
            if (pct < 0) {
                pct = GetNum(window, L"percent_left");
                if (pct >= 0) pct = 100 - pct;
            }
            if (pct < 0) {
                pct = GetNum(window, L"remaining_percent");
                if (pct >= 0) pct = 100 - pct;
            }
            if (pct < 0) return;
            target->pct = pct;
            target->resetUnixMs = resetUnixMs(window);
            target->durationSeconds = windowSeconds > 0 ? (ULONGLONG)windowSeconds : 0;
        };

        applyWindow(GetObj(rl, L"primary_window"), &d->win5h);
        applyWindow(GetObj(rl, L"secondary_window"), &d->winWeek);
        applyWindow(GetObj(rl, L"primary"), &d->win5h);
        applyWindow(GetObj(rl, L"secondary"), &d->winWeek);
        applyWindow(GetObj(rl, L"five_hour"), &d->win5h);
        applyWindow(GetObj(rl, L"weekly"), &d->winWeek);
        applyWindow(GetObj(rl, L"five_hour_limit"), &d->win5h);
        applyWindow(GetObj(rl, L"weekly_limit"), &d->winWeek);

        d->plan = GetStr(usage, L"plan_type");
        d->codexSparkLines.clear();
        d->extraLines.clear();
        int extraLimitLineCount = 0;
        auto addLimitLine = [&](JsonObject const& item) {
            auto itemRl = GetObj(item, L"rate_limit");
            auto pw = GetObj(itemRl, L"primary_window");
            auto sw = GetObj(itemRl, L"secondary_window");
            std::wstring name = GetStr(item, L"limit_name");
            if (name.empty() || (!pw && !sw)) return;
            bool isCodexSpark = name.find(L"Codex-Spark") != std::wstring::npos ||
                                 name.find(L"Codex Spark") != std::wstring::npos ||
                                 name.find(L"codex-spark") != std::wstring::npos ||
                                 name.find(L"codex spark") != std::wstring::npos;
            if (!isCodexSpark && extraLimitLineCount >= 3) return;

            wchar_t line[128];
            swprintf(line, ARRAYSIZE(line), L"%s: 5h %.0f%% | wk %.0f%%",
                     name.c_str(), GetNum(pw, L"used_percent", 0),
                     GetNum(sw, L"used_percent", 0));
            std::wstring& target = isCodexSpark ? d->codexSparkLines : d->extraLines;
            if (!target.empty()) target += L"\n";
            target += line;
            if (!isCodexSpark) extraLimitLineCount++;
        };
        if (usage.HasKey(L"additional_rate_limits")) {
            auto limits = usage.GetNamedValue(L"additional_rate_limits");
            if (limits.ValueType() == JsonValueType::Array) {
                auto arr = limits.GetArray();
                for (uint32_t i = 0; i < arr.Size(); i++) {
                    if (arr.GetAt(i).ValueType() == JsonValueType::Object) addLimitLine(arr.GetAt(i).GetObject());
                }
            } else if (limits.ValueType() == JsonValueType::Object) {
                addLimitLine(limits.GetObject());
            }
        }

        if (auto cr = GetObj(usage, L"credits"); cr && GetBool(cr, L"has_credits")) {
            double balance = GetNum(cr, L"balance", -1);
            if (balance < 0) {
                std::wstring s = GetStr(cr, L"balance");
                if (!s.empty()) balance = wcstod(s.c_str(), nullptr);
            }
            if (balance >= 0) {
                wchar_t line[64];
                swprintf(line, ARRAYSIZE(line), L"credits: %.2f", balance);
                if (!d->extraLines.empty()) d->extraLines += L"\n";
                d->extraLines += line;
            }
        }
        bool parsed = d->win5h.pct >= 0 || d->winWeek.pct >= 0;
        if (!parsed && error) *error = L"unexpected response format (" + DescribeJsonBody(body) + L")";
        return parsed;
    } catch (...) {
        if (error) *error = L"unexpected response format (" + DescribeJsonBody(body) + L")";
        return false;
    }
}

/**********************************************/
//  Fetch Thread
/**********************************************/

static void FetchAccount(const AccountConfig& acc, AccountData* d, int* retryAfterSec) {
    d->error.clear();
    d->retryDeadlineMs = 0;
    d->needsLogin = false;

    uint64_t idHash = AccountIdentityHash(acc);
    ULONGLONG authEpoch = CurrentAuthEpoch(idHash);
    StoredToken tok;
    if (!LoadStoredToken(idHash, &tok) || tok.accessToken.empty()) {
        d->stale = true;
        d->needsLogin = true;
        d->error = L"not signed in - click to sign in";
        return;
    }

    // Refresh just before expiry so a request rarely races the token going stale.
    if (tok.expiresMs && tok.expiresMs < NowUnixMs() + 60000) {
        std::wstring refreshErr;
        if (RefreshToken(acc.provider, &tok, &refreshErr)) {
            TokenSaveResult saved = SaveStoredTokenIfCurrent(idHash, authEpoch, tok);
            if (saved == TokenSaveResult::Stale) {
                d->stale = true;
                d->needsLogin = true;
                d->error = L"not signed in - click to sign in";
                return;
            }
            if (saved != TokenSaveResult::Saved) {
                d->stale = true;
                d->error = L"could not save refreshed token";
                return;
            }
        } else {
            d->stale = true;
            d->needsLogin = true;
            d->error = L"session expired - click to sign in";
            return;
        }
    }

    auto requestUsage = [&](const StoredToken& t) -> HttpResult {
        if (acc.provider == L"anthropic") {
            std::wstring headers = L"Authorization: Bearer " + t.accessToken +
                                   L"\r\nanthropic-beta: oauth-2025-04-20"
                                   L"\r\nAccept: application/json\r\n";
            return HttpRequest(L"GET", L"api.anthropic.com", L"/api/oauth/usage",
                               L"claude-code/2.1.0", headers);
        }
        std::wstring headers = L"Authorization: Bearer " + t.accessToken +
                               L"\r\nOrigin: https://chatgpt.com"
                               L"\r\nReferer: https://chatgpt.com/"
                               L"\r\nAccept: application/json\r\n";
        if (!t.accountId.empty()) headers += L"ChatGPT-Account-Id: " + t.accountId + L"\r\n";
        return HttpRequest(L"GET", L"chatgpt.com", L"/backend-api/wham/usage",
                           L"taskbar-ai-quota/0.1", headers);
    };

    HttpResult r = requestUsage(tok);
    // Reactive refresh: the access token may have been revoked or expired early.
    if (r.ok && r.status == 401 && !tok.refreshToken.empty()) {
        std::wstring refreshErr;
        if (RefreshToken(acc.provider, &tok, &refreshErr)) {
            TokenSaveResult saved = SaveStoredTokenIfCurrent(idHash, authEpoch, tok);
            if (saved == TokenSaveResult::Saved) {
                r = requestUsage(tok);
            } else if (saved == TokenSaveResult::Stale) {
                d->stale = true;
                d->needsLogin = true;
                d->error = L"not signed in - click to sign in";
                return;
            } else {
                d->stale = true;
                d->error = L"could not save refreshed token";
                return;
            }
        }
    }

    bool logUsageResponse = false;
    {
        std::lock_guard<std::mutex> lk(g_settingsMutex);
        logUsageResponse = g_settings.diagnosticLogUsageResponses;
    }
    if (logUsageResponse) {
        // Usage endpoints do not return OAuth credentials. Keep this opt-in and
        // bounded anyway, because the response can include plan/account metadata.
        std::wstring body = Utf8ToWide(r.body);
        constexpr size_t kMaxLoggedUsageChars = 16384;
        if (body.size() > kMaxLoggedUsageChars) {
            body.resize(kMaxLoggedUsageChars);
            body += L" ... [truncated]";
        }
        Wh_Log(L"Usage response [%s/%s] HTTP %d:\n%s", acc.label.c_str(),
               acc.provider.c_str(), r.status, body.c_str());
    }

    if (!r.ok) {
        d->stale = true;
        d->error = L"network error";
        return;
    }
    if (r.status == 401) {
        d->stale = true;
        d->needsLogin = true;
        d->error = L"unauthorized - click to sign in";
        return;
    }
    if (r.status == 429) {
        d->stale = true;
        d->error = L"rate limited by API";
        *retryAfterSec = r.retryAfterSec > 0 ? r.retryAfterSec : 120;
        return;
    }
    if (r.status != 200) {
        d->stale = true;
        d->error = L"HTTP " + std::to_wstring(r.status);
        return;
    }

    AccountData fresh;
    std::wstring parseError;
    bool parsed = acc.provider == L"anthropic" ? ParseAnthropicUsage(r.body, &fresh, &parseError)
                                               : ParseOpenAiUsage(r.body, &fresh, &parseError);
    if (!parsed) {
        d->stale = true;
        d->error = parseError.empty() ? L"unexpected response format" : parseError;
        return;
    }

    fresh.stale = false;
    fresh.lastSuccessMs = NowUnixMs();
    *d = std::move(fresh);
}

static void PostUiUpdate() {
    // Never resolve XAML refs on the fetch thread; marshal first.
    if (g_unloading || !g_uiInjected.load(std::memory_order_acquire)) return;

    for (HWND hWnd : FindCurrentProcessTaskbarWnds()) {
        RunFromWindowThread(hWnd, [](void* param) -> bool {
            HWND hWnd = static_cast<HWND>(param);
            auto* state = FindUiState(hWnd);
            if (!g_unloading && state && state->quotaGrid) UpdateQuotaUi(*state);
            return true;
        }, hWnd);
    }
}

/**********************************************/
//  Threshold Notifications
/**********************************************/

static const UINT kNotifyIconId = 1;
static PCWSTR kNotifyClassName = L"AiQuotaNotify_" WH_MOD_ID;

// Fetch-thread only. Drops the tray icon, window, and class created on demand.
static void RemoveNotifyIcon() {
    if (!g_notifyWnd) return;
    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = g_notifyWnd;
    nid.uID = kNotifyIconId;
    Shell_NotifyIconW(NIM_DELETE, &nid);
    DestroyWindow(g_notifyWnd);
    g_notifyWnd = nullptr;
    UnregisterClassW(kNotifyClassName, GetModuleHandleW(nullptr));
}

// Fetch-thread only. Lazily creates a hidden message-only window owning one tray
// icon, then shows a balloon (rendered as a toast on Win11, kept in notify center).
static void FireThresholdNotification(const std::wstring& title, const std::wstring& body) {
    HINSTANCE hInst = GetModuleHandleW(nullptr);
    if (!g_notifyWnd) {
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = DefWindowProcW;
        wc.hInstance = hInst;
        wc.lpszClassName = kNotifyClassName;
        RegisterClassExW(&wc);  // ERROR_CLASS_ALREADY_EXISTS is fine; CreateWindow reuses it.

        g_notifyWnd = CreateWindowExW(0, kNotifyClassName, L"", 0, 0, 0, 0, 0,
                                      HWND_MESSAGE, nullptr, hInst, nullptr);
        if (!g_notifyWnd) {
            Wh_Log(L"Notify window creation failed: %lu", GetLastError());
            return;
        }

        NOTIFYICONDATAW nid{};
        nid.cbSize = sizeof(nid);
        nid.hWnd = g_notifyWnd;
        nid.uID = kNotifyIconId;
        nid.uFlags = NIF_ICON | NIF_STATE;
        nid.dwState = NIS_HIDDEN;
        nid.dwStateMask = NIS_HIDDEN;
        nid.hIcon = LoadIconW(nullptr, IDI_WARNING);
        if (!Shell_NotifyIconW(NIM_ADD, &nid)) {
            Wh_Log(L"Shell_NotifyIcon NIM_ADD failed");
            DestroyWindow(g_notifyWnd);
            g_notifyWnd = nullptr;
            return;
        }
    }

    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = g_notifyWnd;
    nid.uID = kNotifyIconId;
    nid.uFlags = NIF_INFO;
    nid.dwInfoFlags = NIIF_WARNING | NIIF_RESPECT_QUIET_TIME;
    wcsncpy_s(nid.szInfoTitle, title.c_str(), _TRUNCATE);
    wcsncpy_s(nid.szInfo, body.c_str(), _TRUNCATE);
    Shell_NotifyIconW(NIM_MODIFY, &nid);
}

static DWORD WINAPI FetchThreadProc(LPVOID) {
    bool apartmentInitialized = false;
    try {
        winrt::init_apartment(winrt::apartment_type::multi_threaded);
        apartmentInitialized = true;
    } catch (...) {}

    std::vector<std::wstring> lastLoggedErrorStates;
    std::vector<ULONGLONG> retryDeadlineMs;
    // Per-account red-crossing arm state, indexed [account][0=5h,1=weekly]:
    // -1 unknown (primes without firing), 0 below/armed, 1 above/already notified.
    std::vector<std::array<int, 2>> redState;
    ULONGLONG lastLoggedSettingsGeneration = 0;
    while (!g_unloading) {
        ULONGLONG refreshGeneration = g_refreshGeneration.load();
        int refreshAccountIndex = g_refreshAccountIndex.load();
        std::vector<AccountConfig> accounts;
        int intervalMin, redThreshold;
        bool enableNotifications;
        ULONGLONG settingsGeneration;
        {
            std::lock_guard<std::mutex> lk(g_settingsMutex);
            accounts = g_settings.accounts;
            intervalMin = g_settings.pollMinutes;
            redThreshold = g_settings.redThreshold;
            enableNotifications = g_settings.enableNotifications;
            settingsGeneration = g_settingsGeneration;
        }
        bool settingsChanged = lastLoggedSettingsGeneration != settingsGeneration ||
                               lastLoggedErrorStates.size() != accounts.size();
        if (settingsChanged) {
            lastLoggedErrorStates.assign(accounts.size(), {});
            retryDeadlineMs.assign(accounts.size(), 0);
            lastLoggedSettingsGeneration = settingsGeneration;
        }

        std::vector<AccountData> results(accounts.size());
        {
            std::lock_guard<std::mutex> lk(g_dataMutex);
            if (g_data.size() == results.size()) results = g_data;
        }
        if (settingsChanged || redState.size() != accounts.size()) {
            redState.assign(accounts.size(), std::array<int, 2>{-1, -1});
            for (size_t i = 0; i < accounts.size(); i++) {
                for (int w = 0; w < 2; w++) {
                    const WindowUsage& wu = w == 0 ? results[i].win5h : results[i].winWeek;
                    if (wu.pct >= 0) redState[i][w] = wu.pct >= redThreshold ? 1 : 0;
                }
            }
        }

        bool refreshSingleAccount = g_refreshing.load() && refreshAccountIndex >= 0 &&
                                    refreshAccountIndex < (int)accounts.size();
        std::vector<bool> fetchedOk(accounts.size(), false);
        ULONGLONG nextRetryMs = 0;
        bool anyError = false;
        for (size_t i = 0; i < accounts.size() && !g_unloading; i++) {
            ULONGLONG nowMs = NowUnixMs();
            // Hidden accounts are never fetched (poll or Refresh all); results[i] keeps the
            // prior g_data[i] value and goes stale via the lastSuccessMs/interval check.
            if (accounts[i].hidden) continue;
            if (refreshSingleAccount && (int)i != refreshAccountIndex) {
                if (retryDeadlineMs[i] > nowMs &&
                    (nextRetryMs == 0 || retryDeadlineMs[i] < nextRetryMs)) {
                    nextRetryMs = retryDeadlineMs[i];
                }
                continue;
            }

            if (retryDeadlineMs[i] > nowMs) {
                if (nextRetryMs == 0 || retryDeadlineMs[i] < nextRetryMs) {
                    nextRetryMs = retryDeadlineMs[i];
                }
                continue;
            }

            int retryAfter = 0;
            FetchAccount(accounts[i], &results[i], &retryAfter);
            if (retryAfter > 0) {
                retryDeadlineMs[i] = NowUnixMs() + (ULONGLONG)retryAfter * 1000;
                results[i].retryDeadlineMs = retryDeadlineMs[i];
                if (nextRetryMs == 0 || retryDeadlineMs[i] < nextRetryMs) {
                    nextRetryMs = retryDeadlineMs[i];
                }
            } else {
                retryDeadlineMs[i] = 0;
                results[i].retryDeadlineMs = 0;
            }

            if (!results[i].error.empty()) {
                if (retryAfter <= 0 && !results[i].needsLogin) anyError = true;
                std::wstring errorState = accounts[i].provider + L"\n" + accounts[i].label +
                                          L"\n" + results[i].error;
                if (errorState != lastLoggedErrorStates[i]) {
                    Wh_Log(L"Fetch [%d] %s (%s): %s", (int)i, accounts[i].label.c_str(),
                           accounts[i].provider.c_str(), results[i].error.c_str());
                    lastLoggedErrorStates[i] = std::move(errorState);
                }
            } else {
                lastLoggedErrorStates[i].clear();
                fetchedOk[i] = true;
            }
        }
        if (!enableNotifications && g_notifyWnd) RemoveNotifyIcon();

        bool published = false;
        {
            std::lock_guard<std::mutex> lk(g_settingsMutex);
            if (settingsGeneration == g_settingsGeneration) {
                std::lock_guard<std::mutex> lk2(g_dataMutex);
                if (g_data.size() == results.size()) {
                    g_data = results;
                    published = true;
                }
            }
        }
        if (refreshGeneration == g_refreshGeneration.load()) {
            g_refreshing = false;
            g_refreshAccountIndex = -1;
        }
        if (published) {
            // Fire one toast per upward crossing of the red threshold; re-arm when
            // usage drops back below. Notification side effects happen only after the
            // matching settings generation publishes, avoiding stale in-flight toasts.
            for (size_t i = 0; i < accounts.size(); i++) {
                if (!fetchedOk[i]) continue;
                for (int w = 0; w < 2; w++) {
                    const WindowUsage& wu = w == 0 ? results[i].win5h : results[i].winWeek;
                    int& st = redState[i][w];
                    if (wu.pct < 0) continue;
                    if (wu.pct >= redThreshold) {
                        if (st == 0 && enableNotifications) {
                            std::wstring providerName =
                                accounts[i].provider == L"anthropic" ? L"Anthropic" : L"OpenAI";
                            wchar_t title[96];
                            swprintf(title, ARRAYSIZE(title), L"%s usage at %.0f%%",
                                     w == 0 ? L"5h" : L"weekly", wu.pct);
                            std::wstring body = providerName + L" - resets " + FormatReset(wu.resetUnixMs);
                            FireThresholdNotification(title, body);
                        }
                        st = 1;
                    } else {
                        st = 0;
                    }
                }
            }
            PostUiUpdate();
        }

        DWORD waitMs = (DWORD)intervalMin * 60000;
        if (anyError) waitMs = std::min(waitMs, (DWORD)120000);
        ULONGLONG nowMs = NowUnixMs();
        if (nextRetryMs > nowMs) {
            waitMs = (DWORD)std::min<ULONGLONG>(waitMs, nextRetryMs - nowMs);
            waitMs = std::min<DWORD>(waitMs, 1000);
        }

        HANDLE handles[2] = {g_stopEvent, g_refreshEvent};
        if (WaitForMultipleObjects(2, handles, FALSE, waitMs) == WAIT_OBJECT_0) break;
    }
    RemoveNotifyIcon();
    if (apartmentInitialized) winrt::uninit_apartment();
    return 0;
}

/**********************************************/
//  Taskbar XAML Access
/**********************************************/

using CTaskBand_GetTaskbarHost_t = void*(WINAPI*)(void*, void*);
using CSecondaryTaskBand_GetTaskbarHost_t = void*(WINAPI*)(void*, void*);
using TaskbarHost_FrameHeight_t = int(WINAPI*)(void*);
using Std_Ref_Decref_t = void(WINAPI*)(void*);

static CTaskBand_GetTaskbarHost_t CTaskBand_GetTaskbarHost_Original = nullptr;
static CSecondaryTaskBand_GetTaskbarHost_t CSecondaryTaskBand_GetTaskbarHost_Original = nullptr;
static TaskbarHost_FrameHeight_t TaskbarHost_FrameHeight_Original = nullptr;
static Std_Ref_Decref_t Std_Ref_Decref_Original = nullptr;
static void* CTaskBand_ITaskListWndSite_vftable = nullptr;
static void* CSecondaryTaskBand_ITaskListWndSite_vftable = nullptr;

static bool RunFromWindowThread(HWND hWnd, WindowThreadProc proc, void* param, DWORD timeoutMs) {
    static const UINT kMsg = RegisterWindowMessage(L"Windhawk_RunFromWindowThread_" WH_MOD_ID);
    struct Payload {
        WindowThreadProc proc;
        void* param;
        std::atomic<bool> ran{false};
        std::atomic<bool> result{false};

        Payload(WindowThreadProc proc, void* param) : proc(proc), param(param) {}
    };
    using PayloadRef = std::shared_ptr<Payload>;
    DWORD tid = GetWindowThreadProcessId(hWnd, nullptr);
    if (!tid) return false;
    if (tid == GetCurrentThreadId()) {
        return proc(param);
    }

    HHOOK hook = SetWindowsHookExW(
        WH_CALLWNDPROC,
        [](int code, WPARAM w, LPARAM l) CALLBACK -> LRESULT {
            if (code == HC_ACTION) {
                auto* cwp = reinterpret_cast<const CWPSTRUCT*>(l);
                static const UINT kM = RegisterWindowMessage(L"Windhawk_RunFromWindowThread_" WH_MOD_ID);
                if (cwp->message == kM) {
                    std::unique_ptr<PayloadRef> holder(reinterpret_cast<PayloadRef*>(cwp->lParam));
                    PayloadRef p = *holder;
                    p->result.store(p->proc(p->param), std::memory_order_release);
                    p->ran.store(true, std::memory_order_release);
                }
            }
            return CallNextHookEx(nullptr, code, w, l);
        }, nullptr, tid);
    if (!hook) return false;

    PayloadRef pay = std::make_shared<Payload>(proc, param);
    auto* holder = new PayloadRef(pay);
    bool sent = true;
    if (timeoutMs == INFINITE) {
        SendMessageW(hWnd, kMsg, 0, reinterpret_cast<LPARAM>(holder));
    } else {
        DWORD_PTR ignored = 0;
        // Avoid worker/UI deadlocks during unload if the target thread stops pumping messages.
        sent = SendMessageTimeoutW(hWnd, kMsg, 0, reinterpret_cast<LPARAM>(holder),
                                   SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG,
                                   timeoutMs, &ignored) != 0;
    }

    bool ran = pay->ran.load(std::memory_order_acquire);
    bool result = sent && ran && pay->result.load(std::memory_order_acquire);
    if (!sent) {
        UnhookWindowsHookEx(hook);
        // The target thread may still consume holder after a timeout; the hook owns it now.
        return false;
    }
    if (!ran) delete holder;
    UnhookWindowsHookEx(hook);
    return result;
}

static std::vector<HWND> FindCurrentProcessTaskbarWnds() {
    TaskbarMonitorMode mode;
    int targetMonitorNumber;
    {
        std::lock_guard<std::mutex> lk(g_settingsMutex);
        mode = g_settings.taskbarMonitorMode;
        targetMonitorNumber = g_settings.taskbarMonitorNumber;
    }

    std::vector<HMONITOR> monitors;
    EnumDisplayMonitors(nullptr, nullptr,
        [](HMONITOR hMonitor, HDC, LPRECT, LPARAM lp) CALLBACK -> BOOL {
            reinterpret_cast<std::vector<HMONITOR>*>(lp)->push_back(hMonitor);
            return TRUE;
        }, reinterpret_cast<LPARAM>(&monitors));

    struct TaskbarWindowInfo {
        HWND hWnd;
        bool primary;
        int monitorNumber;
    };
    struct EnumContext {
        DWORD pid;
        TaskbarMonitorMode mode;
        std::vector<HMONITOR>* monitors;
        std::vector<TaskbarWindowInfo>* windows;
    } ctx{GetCurrentProcessId(), mode, &monitors, nullptr};

    std::vector<TaskbarWindowInfo> windows;
    ctx.windows = &windows;
    EnumWindows([](HWND hWnd, LPARAM lp) CALLBACK -> BOOL {
        auto* ctx = reinterpret_cast<EnumContext*>(lp);
        DWORD pid = 0;
        wchar_t cls[64] = {};
        if (!GetWindowThreadProcessId(hWnd, &pid) || pid != ctx->pid ||
            !GetClassNameW(hWnd, cls, ARRAYSIZE(cls))) {
            return TRUE;
        }

        bool primary = _wcsicmp(cls, L"Shell_TrayWnd") == 0;
        bool secondary = _wcsicmp(cls, L"Shell_SecondaryTrayWnd") == 0;
        if (!primary && !secondary) return TRUE;
        if (ctx->mode == TaskbarMonitorMode::Primary && !primary) return TRUE;

        int monitorNumber = 0;
        if (HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST)) {
            for (size_t i = 0; i < ctx->monitors->size(); ++i) {
                if ((*ctx->monitors)[i] == hMonitor) {
                    monitorNumber = (int)i + 1;
                    break;
                }
            }
        }
        ctx->windows->push_back({hWnd, primary, monitorNumber});
        return TRUE;
    }, reinterpret_cast<LPARAM>(&ctx));

    std::sort(windows.begin(), windows.end(), [](const auto& a, const auto& b) {
        if (a.primary != b.primary) return a.primary;
        if (a.monitorNumber != b.monitorNumber) {
            if (a.monitorNumber == 0) return false;
            if (b.monitorNumber == 0) return true;
            return a.monitorNumber < b.monitorNumber;
        }
        return reinterpret_cast<UINT_PTR>(a.hWnd) < reinterpret_cast<UINT_PTR>(b.hWnd);
    });

    std::vector<HWND> result;
    result.reserve(windows.size());
    if (mode == TaskbarMonitorMode::Specific) {
        if (targetMonitorNumber >= 1 && targetMonitorNumber <= (int)windows.size()) {
            result.push_back(windows[targetMonitorNumber - 1].hWnd);
        }
        return result;
    }

    for (const auto& window : windows) result.push_back(window.hWnd);
    return result;
}

static HRESULT TryGetTaskbarElementAbi(HWND hTaskbarWnd, void** result) {
    *result = nullptr;
    void* taskbarHostSharedPtr[2]{};

    auto cleanup = [&]() {
        if (taskbarHostSharedPtr[1] && Std_Ref_Decref_Original) Std_Ref_Decref_Original(taskbarHostSharedPtr[1]);
    };

    wchar_t cls[64] = {};
    bool isSecondary = GetClassNameW(hTaskbarWnd, cls, ARRAYSIZE(cls)) &&
                       _wcsicmp(cls, L"Shell_SecondaryTrayWnd") == 0;
    HWND hTaskSwWnd = isSecondary ? FindWindowExW(hTaskbarWnd, nullptr, L"WorkerW", nullptr)
                                  : (HWND)GetProp(hTaskbarWnd, L"TaskbandHWND");
    if (!hTaskSwWnd) return E_HANDLE;

    void* taskBand = (void*)GetWindowLongPtrW(hTaskSwWnd, 0);
    if (!taskBand) return E_POINTER;

    void* taskBandVftable = isSecondary ? CSecondaryTaskBand_ITaskListWndSite_vftable
                                        : CTaskBand_ITaskListWndSite_vftable;
    auto getTaskbarHost = isSecondary ? CSecondaryTaskBand_GetTaskbarHost_Original
                                      : CTaskBand_GetTaskbarHost_Original;
    if (!taskBandVftable || !getTaskbarHost) return E_NOINTERFACE;

    void* taskBandForTaskListWndSite = taskBand;
    for (int i = 0; *(void**)taskBandForTaskListWndSite != taskBandVftable; i++) {
        if (i == 20) return E_NOINTERFACE;
        taskBandForTaskListWndSite = (void**)taskBandForTaskListWndSite + 1;
        if (!taskBandForTaskListWndSite) return E_POINTER;
    }

    getTaskbarHost(taskBandForTaskListWndSite, taskbarHostSharedPtr);
    if (!taskbarHostSharedPtr[0]) {
        cleanup();
        return E_POINTER;
    }

    size_t taskbarElementIUnknownOffset = 0x10;

#if defined(_M_X64)
    {
        // 48:83EC 28 | sub rsp,28
        // 48:83C1 48 | add rcx,48
        const BYTE* b = (const BYTE*)TaskbarHost_FrameHeight_Original;
        if (b[0] == 0x48 && b[1] == 0x83 && b[2] == 0xEC && b[4] == 0x48 &&
            b[5] == 0x83 && b[6] == 0xC1 && b[7] <= 0x7F) {
            taskbarElementIUnknownOffset = b[7];
        } else {
            Wh_Log(L"Unsupported TaskbarHost::FrameHeight");
        }
    }
#elif defined(_M_ARM64)
    {
        // 7f2303d5 pacibsp
        // fd7bbfa9 stp     fp, lr, [sp, #-0x10]!
        // fd030091 mov     fp, sp
        // 080c41f8 ldr     x8, [x0, #0x10]!
        const DWORD* p = (const DWORD*)TaskbarHost_FrameHeight_Original;
        if (p[0] == 0xD503237F && (p[1] & 0xFFC07FFF) == 0xA9807BFD &&
            p[2] == 0x910003FD && (p[3] & 0xFFF00FE0) == 0xF8400C00) {
            taskbarElementIUnknownOffset = (p[3] >> 12) & 0xFF;
        } else {
            Wh_Log(L"Unsupported TaskbarHost::FrameHeight");
        }
    }
#else
#error "Unsupported architecture"
#endif

    auto* taskbarElementIUnknown =
        *(IUnknown**)((BYTE*)taskbarHostSharedPtr[0] + taskbarElementIUnknownOffset);
    if (!taskbarElementIUnknown) {
        cleanup();
        return E_POINTER;
    }

    HRESULT hr = taskbarElementIUnknown->QueryInterface(winrt::guid_of<FrameworkElement>(), result);
    cleanup();
    if (FAILED(hr) && *result) {
        static_cast<IUnknown*>(*result)->Release();
        *result = nullptr;
    }

    return hr;
}

static XamlRoot GetTaskbarXamlRoot(HWND hTaskbarWnd) {
    if (!CTaskBand_ITaskListWndSite_vftable || !CTaskBand_GetTaskbarHost_Original ||
        !TaskbarHost_FrameHeight_Original) {
        return nullptr;
    }

    void* taskbarElementAbi = nullptr;
    if (FAILED(TryGetTaskbarElementAbi(hTaskbarWnd, &taskbarElementAbi)) || !taskbarElementAbi) return nullptr;

    FrameworkElement taskbarElement{nullptr};
    winrt::attach_abi(taskbarElement, taskbarElementAbi);
    auto result = taskbarElement ? taskbarElement.XamlRoot() : nullptr;
    return result;
}

static FrameworkElement FindChildByName(FrameworkElement const& root, std::wstring_view name, int depth = 32) {
    if (!root || depth == 0) return nullptr;
    int n = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < n; ++i) {
        auto child = VisualTreeHelper::GetChild(root, i).try_as<FrameworkElement>();
        if (!child) continue;
        if (child.Name() == name) return child;
        if (auto found = FindChildByName(child, name, depth - 1)) return found;
    }
    return nullptr;
}

/**********************************************/
//  UI
/**********************************************/


static const wchar_t* const kStartButtonNames[] = {
    L"StartButton",
    L"StartMenuButton",
    L"StartMenuLaunchButton",
    L"LaunchListButton",
};

static FrameworkElement FindChildByClassName(FrameworkElement const& parent, const wchar_t* className,
                                             int depth = 32) {
    if (!parent || depth <= 0) return nullptr;

    int count = VisualTreeHelper::GetChildrenCount(parent);
    for (int i = 0; i < count; i++) {
        auto child = VisualTreeHelper::GetChild(parent, i).try_as<FrameworkElement>();
        if (!child) continue;
        if (winrt::get_class_name(child) == className) return child;
        if (auto found = FindChildByClassName(child, className, depth - 1)) return found;
    }
    return nullptr;
}

static FrameworkElement FindNthElementByClassName(FrameworkElement const& parent,
                                                  const wchar_t* className, int index) {
    if (!parent) return nullptr;

    int found = 0;
    int count = VisualTreeHelper::GetChildrenCount(parent);
    for (int i = 0; i < count; i++) {
        auto child = VisualTreeHelper::GetChild(parent, i).try_as<FrameworkElement>();
        if (!child) continue;
        if (winrt::get_class_name(child) == className) {
            if (found == index) return child;
            found++;
        }
    }
    return nullptr;
}

// Taskbar buttons live inside an ItemsRepeater, one wrapper level deeper than a plain
// child search expects, so check direct children first and then one level down.
static FrameworkElement FindElementInRepeater(FrameworkElement const& repeater,
                                              const wchar_t* const* names, int nameCount) {
    if (!repeater) return nullptr;

    int count = VisualTreeHelper::GetChildrenCount(repeater);
    for (int i = 0; i < count; i++) {
        auto child = VisualTreeHelper::GetChild(repeater, i).try_as<FrameworkElement>();
        if (!child) continue;
        for (int j = 0; j < nameCount; j++) {
            if (child.Name() == names[j]) return child;
        }
    }
    for (int i = 0; i < count; i++) {
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

static Grid FindTaskbarRootGrid(FrameworkElement const& root) {
    FrameworkElement taskbarFrame{nullptr};
    int count = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < count; i++) {
        auto child = VisualTreeHelper::GetChild(root, i).try_as<FrameworkElement>();
        if (child && winrt::get_class_name(child) == L"Taskbar.TaskbarFrame") {
            taskbarFrame = child;
            break;
        }
    }
    if (!taskbarFrame) return nullptr;

    auto rootGrid = FindChildByName(taskbarFrame, L"RootGrid");
    return rootGrid ? rootGrid.try_as<Grid>() : nullptr;
}

static FrameworkElement FindTrayElement(FrameworkElement const& trayGrid, FrameworkElement const& root,
                                        const wchar_t* name) {
    auto element = FindChildByName(trayGrid, name);
    if (!element) element = FindChildByName(root, name);
    return element;
}

struct InjectionTarget {
    Grid grid{nullptr};
    int column = -1;  // -1 means "not a reserved grid column" (overlay or anchored).
};

static bool IsOverlayPosition(std::wstring const& position) {
    return position == L"taskbar_left_edge" || position == L"taskbar_center_edge" ||
           position == L"taskbar_right_edge";
}

static bool IsAnchoredPosition(std::wstring const& position) {
    return position == L"taskbar_left_start" || position == L"taskbar_right_start" ||
           position == L"taskbar_after_search_left" || position == L"taskbar_after_search_right" ||
           position == L"taskbar_after_taskview_left" || position == L"taskbar_after_taskview_right" ||
           position == L"taskbar_after_widgets_left" || position == L"taskbar_after_widgets_right";
}

// Maps a position name onto a concrete grid plus the column to reserve. Tray positions
// resolve against SystemTrayFrameGrid; taskbar positions resolve against the taskbar's
// RootGrid and are placed by margin rather than by column.
static InjectionTarget ResolveInjectionTarget(FrameworkElement const& root,
                                              std::wstring const& position) {
    if (!IsOverlayPosition(position) && !IsAnchoredPosition(position)) {
        auto trayFrame = FindChildByName(root, L"SystemTrayFrameGrid");
        auto trayGrid = trayFrame ? trayFrame.try_as<Grid>() : nullptr;
        if (!trayGrid) return {};

        int columnCount = (int)trayGrid.ColumnDefinitions().Size();
        int col = -1;
        auto columnOf = [&](FrameworkElement const& element, int offset) {
            return element ? Grid::GetColumn(element) + offset : -1;
        };

        if (position == L"tray_right") {
            col = columnCount;
        } else if (position == L"tray_left") {
            col = 0;
        } else if (position == L"tray_before_clock") {
            col = columnOf(FindTrayElement(trayGrid, root, L"NotificationCenterButton"), 0);
        } else if (position == L"tray_after_clock") {
            col = columnOf(FindTrayElement(trayGrid, root, L"ShowDesktopStack"), 0);
        } else if (position == L"tray_before_omni_left") {
            col = columnOf(FindTrayElement(trayGrid, root, L"ControlCenterButton"), 0);
        } else if (position == L"tray_before_omni_right") {
            col = columnOf(FindTrayElement(trayGrid, root, L"ControlCenterButton"), 1);
        } else if (position == L"tray_language_left") {
            col = columnOf(FindTrayElement(trayGrid, root, L"NonActivatableStack"), 0);
        } else if (position == L"tray_language_right") {
            col = columnOf(FindTrayElement(trayGrid, root, L"NonActivatableStack"), 1);
        } else if (position == L"tray_icons_left") {
            col = columnOf(FindTrayElement(trayGrid, root, L"NotificationAreaIcons"), 0);
        } else if (position == L"tray_icons_right") {
            col = columnOf(FindTrayElement(trayGrid, root, L"NotificationAreaIcons"), 1);
        } else if (position == L"tray_hidden_icons_left") {
            col = columnOf(FindTrayElement(trayGrid, root, L"NotifyIconStack"), 0);
        } else if (position == L"tray_hidden_icons_right") {
            col = columnOf(FindTrayElement(trayGrid, root, L"NotifyIconStack"), 1);
        } else if (position == L"tray_after_showdesktop_left") {
            col = columnOf(FindTrayElement(trayGrid, root, L"ShowDesktopStack"), 0);
        } else if (position == L"tray_after_showdesktop_right") {
            auto showDesktop = FindTrayElement(trayGrid, root, L"ShowDesktopStack");
            col = showDesktop ? Grid::GetColumn(showDesktop) + 1 : columnCount;
        } else {
            col = 0;  // Unknown value: behave like the documented default.
        }

        if (col >= 0) return {trayGrid, std::clamp(col, 0, columnCount)};
        return {};
    }

    auto rootGrid = FindTaskbarRootGrid(root);
    if (rootGrid) return {rootGrid, -1};

    // Taskbar frame not realised yet - fall back to the tray so the bars still appear.
    auto trayFrame = FindChildByName(root, L"SystemTrayFrameGrid");
    if (auto trayGrid = trayFrame ? trayFrame.try_as<Grid>() : nullptr) {
        return {trayGrid, (int)trayGrid.ColumnDefinitions().Size()};
    }
    return {};
}

// Resolves the taskbar button an anchored position hangs off, plus which side of it.
static FrameworkElement ResolveAnchorElement(FrameworkElement const& root,
                                             std::wstring const& position, bool* anchorOnLeft) {
    auto rootGrid = FindTaskbarRootGrid(root);
    if (!rootGrid) return nullptr;

    auto repeater = FindChildByName(rootGrid, L"TaskbarFrameRepeater");
    if (!repeater) return nullptr;

    *anchorOnLeft = position.size() > 5 && position.compare(position.size() - 5, 5, L"_left") == 0;
    if (position == L"taskbar_left_start") *anchorOnLeft = true;
    if (position == L"taskbar_right_start") *anchorOnLeft = false;

    if (position == L"taskbar_left_start" || position == L"taskbar_right_start") {
        return FindElementInRepeater(repeater, kStartButtonNames, ARRAYSIZE(kStartButtonNames));
    }
    if (position == L"taskbar_after_search_left" || position == L"taskbar_after_search_right") {
        return FindChildByClassName(repeater, L"Taskbar.TaskbarExtensionElement", 3);
    }
    if (position == L"taskbar_after_taskview_left" || position == L"taskbar_after_taskview_right") {
        return FindNthElementByClassName(repeater, L"Taskbar.ExperienceToggleButton", 1);
    }
    if (position == L"taskbar_after_widgets_left" || position == L"taskbar_after_widgets_right") {
        auto widgets = FindChildByName(repeater, L"AugmentedEntryPointButton");
        if (!widgets) widgets = FindChildByClassName(repeater, L"Taskbar.AugmentedEntryPointButton", 3);
        return widgets;
    }
    return nullptr;
}

static QuotaUiInstance* FindUiState(HWND hWnd) {
    for (auto& state : g_uiInstances) {
        if (state->hWnd == hWnd) return state.get();
    }
    return nullptr;
}

static void EraseUiState(HWND hWnd) {
    g_uiInstances.erase(std::remove_if(g_uiInstances.begin(), g_uiInstances.end(),
        [hWnd](const auto& state) { return state->hWnd == hWnd; }), g_uiInstances.end());
    g_uiInjected.store(!g_uiInstances.empty(), std::memory_order_release);
}

static void ClearQuotaEventState(QuotaUiInstance& state) {
    // Routed event delegates point into this DLL, so revoke before XAML tears down the subtree.
    try {
        if (state.tickTimer) {
            state.tickTimer.Stop();
            state.tickTimer.Tick(state.tickTimerToken);
        }
    } catch (...) {}
    state.tickTimer = nullptr;
    state.tickTimerToken = {};
    for (auto& refs : state.accountRefs) {
        refs.hasToolTipOpenCursor = false;
        refs.reopenToolTipOnMove = false;
        refs.manualToolTipOpen = false;
        try {
            if (refs.manualToolTipTimer) refs.manualToolTipTimer.Stop();
        } catch (...) {}
        try {
            if (refs.manualToolTipTimer) {
                refs.manualToolTipTimer.Tick(refs.manualToolTipTimerToken);
            }
        } catch (...) {}
        try {
            if (refs.toolTip) refs.toolTip.Opened(refs.toolTipOpenedToken);
        } catch (...) {}
        try {
            if (refs.toolTip && refs.toolTip.IsOpen()) refs.toolTip.IsOpen(false);
        } catch (...) {}
    }

    for (auto& handler : state.pointerHandlers) {
        if (!handler.element) continue;

        try { handler.element.Tapped(handler.tappedToken); } catch (...) {}
        try { handler.element.PointerMoved(handler.pointerMovedToken); } catch (...) {}
        try { handler.element.PointerExited(handler.pointerExitedToken); } catch (...) {}
        try { handler.element.PointerCaptureLost(handler.pointerCaptureLostToken); } catch (...) {}
        try { handler.element.PointerCanceled(handler.pointerCanceledToken); } catch (...) {}
        try { handler.element.ContextFlyout(nullptr); } catch (...) {}
        try {
            ToolTipService::SetToolTip(handler.element, winrt::Windows::Foundation::IInspectable{nullptr});
        } catch (...) {}
    }
    state.pointerHandlers.clear();

    for (auto& handler : state.menuItemClickHandlers) {
        if (!handler.item) continue;
        try { handler.item.Click(handler.token); } catch (...) {}
    }
    state.menuItemClickHandlers.clear();
    state.accountToggleItems.clear();
    state.accountRefs.clear();
}

static Grid BuildQuotaGrid(QuotaUiInstance& state) {
    try {
        std::vector<AccountConfig> accounts;
        Settings s;
        int labelFontSize, accountMargin, labelGap, barGap, resetBarGap;
        bool showLabels, labelOnLeft;
        ClickAction clickAction;
        {
            std::lock_guard<std::mutex> lk(g_settingsMutex);
            s = g_settings;
        }
        accounts = s.accounts;
        clickAction = s.clickAction;
        labelFontSize = s.labelFontSize;
        accountMargin = s.accountMargin;
        labelGap = s.labelGap;
        barGap = s.barGap;
        resetBarGap = s.resetBarGap;
        showLabels = s.showLabels;
        labelOnLeft = s.labelOnLeft;
        state.accountRefs.clear();
        if (accounts.empty()) return nullptr;
        state.accountRefs.reserve(accounts.size());
        BarPalette palette = MakeBarPalette(IsSystemLightTheme());
        state.appliedLightTheme = palette.light ? 1 : 0;
        UINT toolTipDurationSeconds = 5;
        if (!SystemParametersInfoW(SPI_GETMESSAGEDURATION, 0, &toolTipDurationSeconds, 0)) {
            toolTipDurationSeconds = 5;
        }
        auto manualToolTipShowDuration =
            std::chrono::seconds(std::clamp(toolTipDurationSeconds, 1u, 60u));
        UINT toolTipHoverDelayMs = 400;
        if (!SystemParametersInfoW(SPI_GETMOUSEHOVERTIME, 0, &toolTipHoverDelayMs, 0)) {
            toolTipHoverDelayMs = 400;
        }
        auto manualToolTipHoverDelay =
            std::chrono::milliseconds(std::clamp(toolTipHoverDelayMs, 100u, 2000u));
        UINT toolTipMoveThresholdX = 4;
        UINT toolTipMoveThresholdY = 4;
        SystemParametersInfoW(SPI_GETMOUSEHOVERWIDTH, 0, &toolTipMoveThresholdX, 0);
        SystemParametersInfoW(SPI_GETMOUSEHOVERHEIGHT, 0, &toolTipMoveThresholdY, 0);
        toolTipMoveThresholdX = std::clamp(toolTipMoveThresholdX, 2u, 32u);
        toolTipMoveThresholdY = std::clamp(toolTipMoveThresholdY, 2u, 32u);

        Grid root;
        root.Name(kRootName);
        root.VerticalAlignment(VerticalAlignment::Center);
        root.UseLayoutRounding(true);

        StackPanel panel;
        panel.Orientation(Orientation::Horizontal);
        panel.Margin({0, 0, 0, 0});

        wchar_t name[64];
        for (size_t i = 0; i < accounts.size(); i++) {
            StackPanel col;
            col.Orientation(labelOnLeft ? Orientation::Horizontal : Orientation::Vertical);
            col.VerticalAlignment(VerticalAlignment::Center);
            col.Margin({(double)accountMargin, 0, (double)accountMargin, 0});
            col.Background(SolidColorBrush(winrt::Windows::UI::Color{0, 0, 0, 0}));
            swprintf(name, ARRAYSIZE(name), L"AiQuota_Acc_%d", (int)i);
            col.Name(name);
            col.Visibility(accounts[i].hidden ? Visibility::Collapsed : Visibility::Visible);
            AccountUiRefs refs;
            refs.column = col;

            if (showLabels) {
                TextBlock label;
                label.Text(accounts[i].label);
                label.FontSize(labelFontSize);
                label.VerticalAlignment(VerticalAlignment::Center);
                label.HorizontalAlignment(labelOnLeft ? HorizontalAlignment::Left : HorizontalAlignment::Center);
                label.Margin(labelOnLeft ? Thickness{0, -2, (double)labelGap, 0} :
                                           Thickness{0, 0, 0, (double)labelGap});
                label.Opacity(0.8);
                swprintf(name, ARRAYSIZE(name), L"AiQuota_Label_%d", (int)i);
                label.Name(name);
                refs.label = label;
                col.Children().Append(label);
            }

            StackPanel bars;
            bars.Orientation(ResolveBar(s, accounts[i], 0).vertical ? Orientation::Horizontal
                                                                    : Orientation::Vertical);
            bars.VerticalAlignment(VerticalAlignment::Center);
            bars.UseLayoutRounding(true);

            for (int w = 0; w < 2; w++) {
                ResolvedBar rb = ResolveBar(s, accounts[i], w);
                BarUiRefs bar;

                // The slot stacks the reset strip against the bar along the cross axis, so
                // the pair moves and hides as one unit.
                StackPanel slot;
                slot.Orientation(rb.vertical ? Orientation::Horizontal : Orientation::Vertical);
                slot.UseLayoutRounding(true);
                slot.HorizontalAlignment(rb.vertical ? HorizontalAlignment::Center
                                                     : HorizontalAlignment::Left);
                slot.VerticalAlignment(rb.vertical ? VerticalAlignment::Bottom
                                                   : VerticalAlignment::Center);
                double halfGap = barGap / 2.0;
                slot.Margin(rb.vertical
                                ? (w == 0 ? Thickness{0, 0, halfGap, 0} : Thickness{halfGap, 0, 0, 0})
                                : (w == 0 ? Thickness{0, 0, 0, halfGap} : Thickness{0, halfGap, 0, 0}));
                bar.slot = slot;

                if (rb.showResetBar) {
                    Border resetTrack;
                    resetTrack.Width(rb.vertical ? rb.resetThickness : rb.resetLength);
                    resetTrack.Height(rb.vertical ? rb.resetLength : rb.resetThickness);
                    resetTrack.CornerRadius({rb.resetCornerRadius, rb.resetCornerRadius,
                                             rb.resetCornerRadius, rb.resetCornerRadius});
                    resetTrack.Background(SolidColorBrush(palette.resetTrack));
                    resetTrack.UseLayoutRounding(true);
                    resetTrack.HorizontalAlignment(HorizontalAlignment::Center);
                    resetTrack.VerticalAlignment(rb.vertical ? VerticalAlignment::Bottom
                                                             : VerticalAlignment::Center);
                    resetTrack.Margin(rb.vertical ? Thickness{0, 0, (double)resetBarGap, 0}
                                                  : Thickness{0, 0, 0, (double)resetBarGap});

                    Border resetFill;
                    resetFill.Width(rb.vertical ? rb.resetThickness : 0);
                    resetFill.Height(rb.vertical ? 0 : rb.resetThickness);
                    resetFill.CornerRadius({rb.resetCornerRadius, rb.resetCornerRadius,
                                            rb.resetCornerRadius, rb.resetCornerRadius});
                    resetFill.Background(SolidColorBrush(palette.resetFill));
                    resetFill.HorizontalAlignment(rb.vertical ? HorizontalAlignment::Center
                                                              : HorizontalAlignment::Left);
                    resetFill.VerticalAlignment(rb.vertical ? VerticalAlignment::Bottom
                                                            : VerticalAlignment::Center);
                    resetTrack.Child(resetFill);

                    bar.resetTrack = resetTrack;
                    bar.resetFill = resetFill;
                    slot.Children().Append(resetTrack);
                }

                Border track;
                track.Width(rb.vertical ? rb.thickness : rb.length);
                track.Height(rb.vertical ? rb.length : rb.thickness);
                track.CornerRadius({rb.cornerRadius, rb.cornerRadius, rb.cornerRadius, rb.cornerRadius});
                track.Background(SolidColorBrush(palette.track));
                track.UseLayoutRounding(true);
                track.HorizontalAlignment(HorizontalAlignment::Center);
                track.VerticalAlignment(rb.vertical ? VerticalAlignment::Bottom
                                                    : VerticalAlignment::Center);
                bar.track = track;

                Border fill;
                fill.Width(rb.vertical ? rb.thickness : 0);
                fill.Height(rb.vertical ? 0 : rb.thickness);
                fill.CornerRadius({rb.cornerRadius, rb.cornerRadius, rb.cornerRadius, rb.cornerRadius});
                fill.HorizontalAlignment(rb.vertical ? HorizontalAlignment::Center
                                                     : HorizontalAlignment::Left);
                fill.VerticalAlignment(rb.vertical ? VerticalAlignment::Bottom
                                                   : VerticalAlignment::Center);
                fill.Background(SolidColorBrush(winrt::Windows::UI::Color{255, 0x9E, 0x9E, 0x9E}));
                fill.UseLayoutRounding(true);
                swprintf(name, ARRAYSIZE(name), L"AiQuota_Fill_%d_%d", (int)i, w);
                fill.Name(name);
                bar.fill = fill;

                Grid barContent;
                barContent.HorizontalAlignment(HorizontalAlignment::Stretch);
                barContent.VerticalAlignment(VerticalAlignment::Stretch);
                barContent.UseLayoutRounding(true);
                barContent.Children().Append(fill);

                if (rb.percentFontSize > 0 || rb.resetFontSize > 0) {
                    // Two identical, full-bleed text layers. The second is clipped to exactly
                    // the filled pixels, so a glyph the fill runs through is half one colour
                    // and half the other rather than flipping all at once. Clipping the
                    // wrapper (not the TextBlock) is what makes the clip rectangle line up
                    // with the bar: a centred TextBlock's local origin is not the bar's.
                    auto makeTextLayer = [&](winrt::Windows::UI::Color const& color,
                                             TextBlock& percentOut, TextBlock& resetOut) {
                        Grid layer;
                        layer.HorizontalAlignment(HorizontalAlignment::Stretch);
                        layer.VerticalAlignment(VerticalAlignment::Stretch);
                        layer.IsHitTestVisible(false);
                        layer.UseLayoutRounding(true);

                        auto makeText = [&](int fontSize, bool leading,
                                            winrt::Windows::UI::Text::FontWeight weight) {
                            TextBlock tb;
                            tb.FontSize(fontSize);
                            tb.FontWeight(weight);
                            tb.Foreground(SolidColorBrush(color));
                            tb.IsHitTestVisible(false);
                            tb.UseLayoutRounding(true);
                            tb.IsTextSelectionEnabled(false);
                            // Tight line bounds measure the actual glyph ink, so centring
                            // lands on a whole pixel instead of the font's padded line box.
                            // That padding is what was smearing the old text.
                            tb.TextLineBounds(TextLineBounds::Tight);
                            tb.OpticalMarginAlignment(OpticalMarginAlignment::TrimSideBearings);
                            tb.TextWrapping(TextWrapping::NoWrap);
                            tb.VerticalAlignment(VerticalAlignment::Center);
                            tb.HorizontalAlignment(leading ? HorizontalAlignment::Left
                                                           : HorizontalAlignment::Right);
                            tb.TextAlignment(leading ? TextAlignment::Left : TextAlignment::Right);
                            tb.Margin(leading ? Thickness{(double)rb.textPadding, 0, 0, 0}
                                              : Thickness{0, 0, (double)rb.textPadding, 0});
                            layer.Children().Append(tb);
                            return tb;
                        };

                        if (rb.percentFontSize > 0) {
                            percentOut = makeText(rb.percentFontSize, true,
                                                  winrt::Windows::UI::Text::FontWeights::SemiBold());
                        }
                        if (rb.resetFontSize > 0) {
                            // Lighter than the percentage: the countdown is context, not the
                            // headline number.
                            resetOut = makeText(rb.resetFontSize, false,
                                                winrt::Windows::UI::Text::FontWeights::Medium());
                        }
                        return layer;
                    };

                    Grid textBase = makeTextLayer(palette.onTrack, bar.percentBase, bar.resetBase);
                    Canvas::SetZIndex(textBase, 1);
                    bar.textBase = textBase;
                    barContent.Children().Append(textBase);

                    Grid textInverted =
                        makeTextLayer(winrt::Windows::UI::Color{255, 0x11, 0x14, 0x17},
                                      bar.percentInverted, bar.resetInverted);
                    RectangleGeometry invertClip;
                    invertClip.Rect(winrt::Windows::Foundation::Rect{0.0f, 0.0f, 0.0f, 0.0f});
                    textInverted.Clip(invertClip);
                    Canvas::SetZIndex(textInverted, 2);
                    bar.textInverted = textInverted;
                    bar.invertClip = invertClip;
                    barContent.Children().Append(textInverted);
                }

                track.Child(barContent);
                slot.Children().Append(track);
                bars.Children().Append(slot);
                refs.bars[w] = bar;
            }

            col.Children().Append(bars);

            ToolTip toolTip;
            toolTip.Placement(wuxcp::PlacementMode::Top);
            toolTip.VerticalOffset(20);
            toolTip.Padding(Thickness{10, 8, 10, 8});
            toolTip.Background(SolidColorBrush(winrt::Windows::UI::Color{0xF7, 0x1F, 0x1F, 0x1F}));
            toolTip.Foreground(SolidColorBrush(winrt::Windows::UI::Color{255, 0xF3, 0xF3, 0xF3}));
            toolTip.BorderThickness(Thickness{1, 1, 1, 1});
            toolTip.IsHitTestVisible(false);
            UpdateQuotaToolTip(toolTip, L"loading...", false);
            ToolTipService::SetToolTip(col, toolTip);
            refs.toolTip = toolTip;
            ToolTipService::SetPlacement(col, wuxcp::PlacementMode::Top);
            UIElement tappedElement = col.as<UIElement>();
            QuotaUiInstance* statePtr = &state;
            int accountIndex = (int)i;
            refs.toolTipOpenedToken = toolTip.Opened(
                [hWnd = state.hWnd, accountIndex](winrt::Windows::Foundation::IInspectable const&,
                                                  RoutedEventArgs const&) {
                    if (g_unloading) return;

                    try {
                        auto* uiState = FindUiState(hWnd);
                        if (!uiState || accountIndex >= (int)uiState->accountRefs.size()) return;

                        auto& refs = uiState->accountRefs[accountIndex];
                        refs.hasToolTipOpenCursor = GetCursorPos(&refs.toolTipOpenCursor) != FALSE;
                    } catch (...) {}
                });
            DispatcherTimer manualToolTipTimer;
            manualToolTipTimer.Interval(manualToolTipHoverDelay);
            auto manualToolTipTimerToken = manualToolTipTimer.Tick(
                [hWnd = state.hWnd, accountIndex, manualToolTipShowDuration](
                    winrt::Windows::Foundation::IInspectable const& sender,
                    winrt::Windows::Foundation::IInspectable const&) {
                    try {
                        if (auto timer = sender.try_as<DispatcherTimer>()) timer.Stop();
                        if (g_unloading) return;

                        auto* uiState = FindUiState(hWnd);
                        if (!uiState || !uiState->quotaGrid ||
                            accountIndex >= (int)uiState->accountRefs.size()) return;

                        auto& refs = uiState->accountRefs[accountIndex];
                        if (!refs.toolTip) return;
                        if (refs.manualToolTipOpen) {
                            refs.manualToolTipOpen = false;
                            refs.reopenToolTipOnMove = true;
                            refs.toolTip.IsOpen(false);
                            return;
                        }
                        if (!refs.reopenToolTipOnMove) return;

                        refs.hasToolTipOpenCursor =
                            GetCursorPos(&refs.toolTipOpenCursor) != FALSE;
                        refs.toolTip.IsOpen(true);
                        refs.reopenToolTipOnMove = false;
                        refs.manualToolTipOpen = true;
                        if (refs.manualToolTipTimer) {
                            refs.manualToolTipTimer.Interval(manualToolTipShowDuration);
                            refs.manualToolTipTimer.Start();
                        }
                    } catch (...) {}
                });
            refs.manualToolTipTimer = manualToolTipTimer;
            refs.manualToolTipTimerToken = manualToolTipTimerToken;
            auto tappedToken = tappedElement.Tapped(
                [statePtr, accountIndex, clickAction, manualToolTipHoverDelay](
                    winrt::Windows::Foundation::IInspectable const&,
                    wuxi::TappedRoutedEventArgs const& e) {
                    if (g_unloading || !statePtr->quotaGrid) {
                        e.Handled(true);
                        return;
                    }

                    bool needsLogin = false;
                    {
                        std::lock_guard<std::mutex> lk(g_dataMutex);
                        if (accountIndex < (int)g_data.size()) needsLogin = g_data[accountIndex].needsLogin;
                    }
                    if (needsLogin) StartLogin(accountIndex);
                    else if (clickAction == ClickAction::OpenDashboard) OpenDashboardForAccount(accountIndex);
                    else {
                        RefreshQuota(accountIndex);
                        try {
                            if (e.PointerDeviceType() ==
                                    winrt::Windows::Devices::Input::PointerDeviceType::Mouse &&
                                accountIndex < (int)statePtr->accountRefs.size()) {
                                auto& refs = statePtr->accountRefs[accountIndex];
                                if (refs.manualToolTipTimer) refs.manualToolTipTimer.Stop();
                                if (refs.manualToolTipOpen) {
                                    refs.manualToolTipOpen = false;
                                    if (refs.toolTip) refs.toolTip.IsOpen(false);
                                }
                                refs.hasToolTipOpenCursor =
                                    GetCursorPos(&refs.toolTipOpenCursor) != FALSE;
                                refs.reopenToolTipOnMove = true;
                                if (refs.manualToolTipTimer) {
                                    refs.manualToolTipTimer.Interval(manualToolTipHoverDelay);
                                    refs.manualToolTipTimer.Start();
                                }
                            }
                        } catch (...) {}
                    }
                    e.Handled(true);
                });

            // System XAML suppresses the automatic tooltip after a click until pointer re-entry.
            // A short stationary hover reopens it; further movement dismisses it.
            auto pointerMovedToken = tappedElement.PointerMoved(
                [statePtr, accountIndex, manualToolTipHoverDelay,
                 toolTipMoveThresholdX, toolTipMoveThresholdY](
                    winrt::Windows::Foundation::IInspectable const&,
                    wuxi::PointerRoutedEventArgs const& e) {
                    if (g_unloading || !statePtr->quotaGrid ||
                        accountIndex >= (int)statePtr->accountRefs.size()) return;

                    try {
                        if (e.Pointer().PointerDeviceType() !=
                            winrt::Windows::Devices::Input::PointerDeviceType::Mouse) return;

                        auto& refs = statePtr->accountRefs[accountIndex];
                        if (!refs.toolTip) return;

                        bool wasOpen = refs.toolTip.IsOpen();
                        POINT cursor{};
                        bool haveCursor = GetCursorPos(&cursor) != FALSE;
                        bool trackingHover = wasOpen || refs.reopenToolTipOnMove ||
                                             refs.manualToolTipOpen;
                        if (trackingHover && refs.hasToolTipOpenCursor && haveCursor &&
                            std::abs(cursor.x - refs.toolTipOpenCursor.x) < toolTipMoveThresholdX &&
                            std::abs(cursor.y - refs.toolTipOpenCursor.y) < toolTipMoveThresholdY) {
                            return;
                        }
                        if (refs.manualToolTipTimer) refs.manualToolTipTimer.Stop();
                        bool rearmAfterMove = wasOpen || refs.reopenToolTipOnMove ||
                                              refs.manualToolTipOpen;
                        refs.manualToolTipOpen = false;
                        refs.hasToolTipOpenCursor = false;
                        if (wasOpen) {
                            // Automatic and manual tooltips both dismiss on local pointer movement.
                            refs.toolTip.IsOpen(false);
                        }
                        if (!rearmAfterMove) return;

                        refs.reopenToolTipOnMove = true;
                        refs.hasToolTipOpenCursor = haveCursor;
                        if (haveCursor) refs.toolTipOpenCursor = cursor;
                        if (refs.manualToolTipTimer) {
                            refs.manualToolTipTimer.Interval(manualToolTipHoverDelay);
                            refs.manualToolTipTimer.Start();
                        }
                    } catch (...) {}
                });

            auto closeToolTip =
                [statePtr, accountIndex](winrt::Windows::Foundation::IInspectable const&,
                                         wuxi::PointerRoutedEventArgs const&) {
                    if (g_unloading || accountIndex >= (int)statePtr->accountRefs.size()) return;

                    auto& refs = statePtr->accountRefs[accountIndex];
                    refs.hasToolTipOpenCursor = false;
                    refs.reopenToolTipOnMove = false;
                    try {
                        if (refs.manualToolTipTimer) refs.manualToolTipTimer.Stop();
                    } catch (...) {}

                    refs.manualToolTipOpen = false;
                    try {
                        if (refs.toolTip && refs.toolTip.IsOpen()) refs.toolTip.IsOpen(false);
                    } catch (...) {}
                };
            auto pointerExitedToken = tappedElement.PointerExited(closeToolTip);
            auto pointerCaptureLostToken = tappedElement.PointerCaptureLost(closeToolTip);
            auto pointerCanceledToken = tappedElement.PointerCanceled(closeToolTip);
            state.pointerHandlers.push_back({tappedElement, tappedToken, pointerMovedToken,
                                             pointerExitedToken, pointerCaptureLostToken,
                                             pointerCanceledToken});

            MenuFlyout menu;
            MenuFlyoutItem refreshAllItem;
            refreshAllItem.Text(L"Refresh all");
            auto refreshAllToken = refreshAllItem.Click(
                [](winrt::Windows::Foundation::IInspectable const&, RoutedEventArgs const&) {
                    RefreshQuota(-1);
                });
            state.menuItemClickHandlers.push_back({refreshAllItem, refreshAllToken});
            menu.Items().Append(refreshAllItem);

            MenuFlyoutItem dashboardItem;
            dashboardItem.Text(L"Open dashboard");
            auto dashboardToken = dashboardItem.Click(
                [accountIndex](winrt::Windows::Foundation::IInspectable const&, RoutedEventArgs const&) {
                    OpenDashboardForAccount(accountIndex);
                });
            state.menuItemClickHandlers.push_back({dashboardItem, dashboardToken});
            menu.Items().Append(dashboardItem);

            // Per-account show/hide checkboxes (checked = visible). Every column carries the same
            // list; toggling flips global state, persists, and re-syncs all instances via UpdateQuotaUi.
            menu.Items().Append(MenuFlyoutSeparator{});
            for (size_t k = 0; k < accounts.size(); k++) {
                ToggleMenuFlyoutItem toggle;
                toggle.Text(accounts[k].label + L" - " +
                            (accounts[k].provider == L"anthropic" ? L"Anthropic" : L"OpenAI"));
                toggle.IsChecked(!accounts[k].hidden);
                int toggleIndex = (int)k;
                auto toggleToken = toggle.Click(
                    [toggleIndex](winrt::Windows::Foundation::IInspectable const& sender,
                                  RoutedEventArgs const&) {
                        ToggleAccountVisibility(toggleIndex, sender);
                    });
                state.menuItemClickHandlers.push_back({toggle, toggleToken});
                state.accountToggleItems.push_back({toggleIndex, toggle});
                menu.Items().Append(toggle);
            }

            // Sign in / Sign out submenus drive the mod's own OAuth per account.
            menu.Items().Append(MenuFlyoutSeparator{});
            MenuFlyoutSubItem signInSub;
            signInSub.Text(L"Sign in");
            MenuFlyoutSubItem signOutSub;
            signOutSub.Text(L"Sign out");
            for (size_t k = 0; k < accounts.size(); k++) {
                std::wstring name = accounts[k].label + L" - " +
                                    (accounts[k].provider == L"anthropic" ? L"Anthropic" : L"OpenAI");
                int authIndex = (int)k;

                MenuFlyoutItem signInItem;
                signInItem.Text(name);
                auto signInToken = signInItem.Click(
                    [authIndex](winrt::Windows::Foundation::IInspectable const&,
                                RoutedEventArgs const&) { StartLogin(authIndex); });
                state.menuItemClickHandlers.push_back({signInItem, signInToken});
                signInSub.Items().Append(signInItem);

                MenuFlyoutItem signOutItem;
                signOutItem.Text(name);
                auto signOutToken = signOutItem.Click(
                    [authIndex](winrt::Windows::Foundation::IInspectable const&,
                                RoutedEventArgs const&) { SignOutAccount(authIndex); });
                state.menuItemClickHandlers.push_back({signOutItem, signOutToken});
                signOutSub.Items().Append(signOutItem);
            }
            menu.Items().Append(signInSub);
            menu.Items().Append(signOutSub);

            tappedElement.ContextFlyout(menu);

            panel.Children().Append(col);
            state.accountRefs.push_back(std::move(refs));
        }

        root.Children().Append(panel);
        return root;
    } catch (...) {
        ClearQuotaEventState(state);
        Wh_Log(L"BuildQuotaGrid: exception");
        return nullptr;
    }
}

static void UpdateQuotaUi(QuotaUiInstance& state) {
    if (!state.quotaGrid) return;

    std::vector<AccountConfig> accounts;
    Settings s;
    {
        std::lock_guard<std::mutex> lk(g_settingsMutex);
        s = g_settings;
    }
    accounts = s.accounts;
    int intervalMin = s.pollMinutes;
    ClickAction clickAction = s.clickAction;
    BarMode barMode = s.barMode;
    int yellowThreshold = s.yellowThreshold;
    int orangeThreshold = s.orangeThreshold;
    int redThreshold = s.redThreshold;
    bool showCodexSparkInTooltip = s.showCodexSparkInTooltip;
    bool colorblindMode = s.colorblindMode;
    bool showStaleWarning = s.showStaleWarning;

    std::vector<AccountData> data;
    {
        std::lock_guard<std::mutex> lk(g_dataMutex);
        data = g_data;
    }
    if (data.size() != accounts.size()) return;
    if (state.accountRefs.size() != data.size()) return;
    if (state.applied.size() != data.size()) state.applied.assign(data.size(), {});

    ULONGLONG now = NowUnixMs();
    bool refreshing = g_refreshing.load();
    int refreshAccountIndex = g_refreshAccountIndex.load();

    // Static chrome is only repainted when the system theme actually flips, so a light
    // taskbar gets dark-on-light bars without needing the mod reloaded.
    BarPalette palette = MakeBarPalette(IsSystemLightTheme());
    bool themeChanged = state.appliedLightTheme != (palette.light ? 1 : 0);
    if (themeChanged) state.appliedLightTheme = palette.light ? 1 : 0;

    // Remaining mode shows the quota left (100 - used); n/a (pct < 0) stays unchanged.
    auto displayPct = [&](double pct) {
        return barMode == BarMode::Remaining && pct >= 0 ? std::clamp(100.0 - pct, 0.0, 100.0) : pct;
    };
    const wchar_t* remainingSuffix = barMode == BarMode::Remaining ? L" remaining" : L"";
    try {
        for (size_t i = 0; i < data.size(); i++) {
            const AccountData& d = data[i];
            AppliedState& ap = state.applied[i];
            const AccountUiRefs& ui = state.accountRefs[i];

            // Hidden accounts collapse their column (no space, not right-clickable) and skip
            // visual work; data stays in g_data and repaints on un-hide (which posts an update).
            int visible = accounts[i].hidden ? 0 : 1;
            if (visible != ap.visible) {
                if (ui.column) {
                    ui.column.Visibility(visible ? Visibility::Visible : Visibility::Collapsed);
                }
                ap.visible = visible;
            }
            if (!visible) continue;

            bool stale = d.stale || d.lastSuccessMs == 0 ||
                         now - d.lastSuccessMs > (ULONGLONG)intervalMin * 2 * 60000;
            bool warn = showStaleWarning && stale && !d.error.empty();
            bool accountRefreshing = refreshing && (refreshAccountIndex < 0 || refreshAccountIndex == (int)i);

            for (int w = 0; w < 2; w++) {
                const WindowUsage& wu = w == 0 ? d.win5h : d.winWeek;
                const BarUiRefs& bar = ui.bars[w];
                ResolvedBar rb = ResolveBar(s, accounts[i], w);

                if (themeChanged) {
                    if (bar.track) bar.track.Background(SolidColorBrush(palette.track));
                    if (bar.resetTrack) bar.resetTrack.Background(SolidColorBrush(palette.resetTrack));
                    if (bar.resetFill) bar.resetFill.Background(SolidColorBrush(palette.resetFill));
                    if (bar.percentBase) bar.percentBase.Foreground(SolidColorBrush(palette.onTrack));
                    if (bar.resetBase) bar.resetBase.Foreground(SolidColorBrush(palette.onTrack));
                }

                int windowVisible = wu.pct >= 0 ? 1 : 0;
                if (windowVisible != ap.windowVisible[w]) {
                    if (bar.slot) {
                        bar.slot.Visibility(windowVisible ? Visibility::Visible
                                                          : Visibility::Collapsed);
                    }
                    ap.windowVisible[w] = windowVisible;
                }
                if (!windowVisible) continue;

                double dispPct = displayPct(wu.pct);
                int px = dispPct > 0
                             ? std::clamp((int)std::lround(rb.length * dispPct / 100.0), 2, rb.length)
                             : 0;
                // Colour stays keyed to actual usage so depleting quota still reds out.
                auto c = UsageColor(wu.pct, stale, yellowThreshold, orangeThreshold, redThreshold,
                                    colorblindMode);
                uint32_t cv = PackColor(c);
                if (px != ap.fillPx[w] || cv != ap.fillColor[w]) {
                    if (bar.fill) {
                        if (rb.vertical) bar.fill.Height(px);
                        else bar.fill.Width(px);
                        bar.fill.Background(SolidColorBrush(c));
                    }
                    if (bar.invertClip) {
                        // The clip is expressed in the text layer's own coordinates, and that
                        // layer is stretched to exactly the bar rectangle, so this masks
                        // precisely the filled pixels. Vertical bars fill bottom-up.
                        bar.invertClip.Rect(
                            rb.vertical
                                ? winrt::Windows::Foundation::Rect{0.0f, (float)(rb.length - px),
                                                                   (float)rb.thickness, (float)px}
                                : winrt::Windows::Foundation::Rect{0.0f, 0.0f, (float)px,
                                                                   (float)rb.thickness});
                    }
                    ap.fillPx[w] = px;
                    ap.fillColor[w] = cv;
                }

                // Text over the fill flips to whichever of near-black / white reads better
                // against the current threshold colour.
                auto onFillColor = OnColorFor(c);
                uint32_t onFillPacked = PackColor(onFillColor);
                if (onFillPacked != ap.onFillColor[w]) {
                    if (bar.percentInverted) {
                        bar.percentInverted.Foreground(SolidColorBrush(onFillColor));
                    }
                    if (bar.resetInverted) {
                        bar.resetInverted.Foreground(SolidColorBrush(onFillColor));
                    }
                    ap.onFillColor[w] = onFillPacked;
                }

                if (rb.percentFontSize > 0) {
                    wchar_t text[16];
                    swprintf(text, ARRAYSIZE(text), L"%.0f%%", dispPct);
                    std::wstring percentText = text;
                    if (percentText != ap.percentText[w]) {
                        if (bar.percentBase) bar.percentBase.Text(percentText);
                        if (bar.percentInverted) bar.percentInverted.Text(percentText);
                        ap.percentText[w] = std::move(percentText);
                    }
                }

                if (rb.resetFontSize > 0) {
                    std::wstring resetText = FormatCountdownCompact(wu.resetUnixMs);
                    if (resetText != ap.resetText[w]) {
                        if (bar.resetBase) bar.resetBase.Text(resetText);
                        if (bar.resetInverted) bar.resetInverted.Text(resetText);
                        ap.resetText[w] = std::move(resetText);
                    }
                }

                if (bar.resetTrack) {
                    double progress = ResetWindowProgress(wu);
                    int resetVisible = progress >= 0 ? 1 : 0;
                    if (resetVisible != ap.resetVisible[w]) {
                        bar.resetTrack.Visibility(resetVisible ? Visibility::Visible
                                                               : Visibility::Collapsed);
                        ap.resetVisible[w] = resetVisible;
                    }
                    if (resetVisible) {
                        double shown = s.resetBarElapsed ? progress : 1.0 - progress;
                        int resetPx =
                            std::clamp((int)std::lround(rb.resetLength * shown), 0, rb.resetLength);
                        if (resetPx != ap.resetFillPx[w]) {
                            if (bar.resetFill) {
                                if (rb.vertical) bar.resetFill.Height(resetPx);
                                else bar.resetFill.Width(resetPx);
                            }
                            ap.resetFillPx[w] = resetPx;
                        }
                    }
                }
            }

            std::wstring tip = (warn ? L"! " : L"") + accounts[i].label + L" - " +
                               (accounts[i].provider == L"anthropic" ? L"Anthropic" : L"OpenAI");
            bool planIsSpark = d.plan.find(L"Spark") != std::wstring::npos ||
                               d.plan.find(L"spark") != std::wstring::npos;
            if (!d.plan.empty() && (showCodexSparkInTooltip || !planIsSpark)) {
                tip += L" (" + d.plan + L")";
            }
            wchar_t line[160];
            bool hasUsageWindow = false;
            for (int w = 0; w < 2; w++) {
                const WindowUsage& wu = w == 0 ? d.win5h : d.winWeek;
                if (wu.pct < 0) continue;
                std::wstring windowLabel = FormatUsageWindowLabel(wu.durationSeconds, w);
                swprintf(line, ARRAYSIZE(line), L"\n%s: %.0f%%%s | resets %s", windowLabel.c_str(),
                         displayPct(wu.pct), remainingSuffix, FormatReset(wu.resetUnixMs).c_str());
                tip += line;
                hasUsageWindow = true;
            }
            if (!hasUsageWindow) tip += L"\nno applicable usage window";
            if (showCodexSparkInTooltip && accounts[i].provider == L"openai" && !d.codexSparkLines.empty()) {
                tip += L"\n" + d.codexSparkLines;
            }
            if (!d.extraLines.empty()) tip += L"\n" + d.extraLines;
            if (!d.error.empty()) {
                tip += L"\nerror: " + d.error;
                if (d.retryDeadlineMs > now) {
                    ULONGLONG retrySec = (d.retryDeadlineMs - now + 999) / 1000;
                    ULONGLONG days = retrySec / (24 * 60 * 60);
                    ULONGLONG hours = (retrySec / (60 * 60)) % 24;
                    ULONGLONG mins = (retrySec / 60) % 60;
                    ULONGLONG secs = retrySec % 60;
                    wchar_t retry[64];
                    if (days > 0) {
                        if (hours > 0) swprintf(retry, ARRAYSIZE(retry), L"%llud %lluh", days, hours);
                        else swprintf(retry, ARRAYSIZE(retry), L"%llud", days);
                    } else if (hours > 0) {
                        if (mins > 0) swprintf(retry, ARRAYSIZE(retry), L"%lluh %llum", hours, mins);
                        else swprintf(retry, ARRAYSIZE(retry), L"%lluh", hours);
                    } else if (mins > 0) {
                        if (secs > 0) swprintf(retry, ARRAYSIZE(retry), L"%llum %llus", mins, secs);
                        else swprintf(retry, ARRAYSIZE(retry), L"%llum", mins);
                    } else {
                        swprintf(retry, ARRAYSIZE(retry), L"%llus", secs);
                    }
                    tip += L" - retry in ";
                    tip += retry;
                }
            }
            tip += L"\n" + FormatUpdated(d.lastSuccessMs, stale);
            tip += accountRefreshing ? L" - refreshing..." :
                   d.needsLogin ? L" - click to sign in" :
                   clickAction == ClickAction::OpenDashboard ? L" - click to open dashboard" :
                   L" - click to refresh";

            if (tip != ap.tip) {
                // Keep the attached ToolTip object alive so an in-place refresh doesn't reset
                // ToolTipService's pointer-over state and require a leave/re-enter cycle.
                if (ui.toolTip) UpdateQuotaToolTip(ui.toolTip, tip, !d.error.empty());
                ap.tip = tip;
            }

            double columnOpacity = accountRefreshing ? 0.65 : 1.0;
            if (columnOpacity != ap.columnOpacity) {
                if (ui.column) ui.column.Opacity(columnOpacity);
                ap.columnOpacity = columnOpacity;
            }

            double labelOpacity = stale ? 0.45 : 0.8;
            std::wstring labelText = warn ? accounts[i].label + L"!" : accounts[i].label;
            if (labelOpacity != ap.labelOpacity || labelText != ap.labelText) {
                if (ui.label) {
                    ui.label.Opacity(labelOpacity);
                    ui.label.Text(labelText);
                }
                ap.labelOpacity = labelOpacity;
                ap.labelText = std::move(labelText);
            }
        }

        // Keep every instance's menu checkboxes in sync with the shared hidden state
        // (programmatic IsChecked does not raise Click).
        for (auto& [idx, item] : state.accountToggleItems) {
            if (!item || idx < 0 || idx >= (int)accounts.size()) continue;
            bool checked = !accounts[idx].hidden;
            if (item.IsChecked() != checked) item.IsChecked(checked);
        }
    } catch (...) {
        Wh_Log(L"UpdateQuotaUi: exception");
    }
}

static int RemoveQuotaChildren(Grid const& targetGrid, QuotaUiInstance& state) {
    if (!targetGrid) return -1;
    ClearQuotaEventState(state);

    int firstCol = -1;
    for (int i = (int)targetGrid.Children().Size() - 1; i >= 0; --i) {
        auto fe = targetGrid.Children().GetAt(i).try_as<FrameworkElement>();
        if (fe && fe.Name() == kRootName) {
            if (firstCol < 0) firstCol = Grid::GetColumn(fe);
            try { targetGrid.Children().RemoveAt(i); } catch (...) {}
        }
    }
    return firstCol;
}

static void RemoveQuotaGridFromState(QuotaUiInstance& state) {
    // Anchor tracking must be undone first: the LayoutUpdated delegate points into this
    // DLL, and the anchored taskbar button keeps the margin we pushed onto it otherwise.
    if (state.hasLayoutUpdatedToken && state.injectionParent) {
        try { state.injectionParent.LayoutUpdated(state.layoutUpdatedToken); } catch (...) {}
    }
    state.layoutUpdatedToken = {};
    state.hasLayoutUpdatedToken = false;
    if (state.trackedElement && state.hasTrackedOriginalMargin) {
        try { state.trackedElement.Margin(state.trackedOriginalMargin); } catch (...) {}
    }
    state.trackedElement = nullptr;
    state.hasTrackedOriginalMargin = false;

    if (!state.injectionParent) {
        ClearQuotaEventState(state);
        state.quotaGrid = nullptr;
        state.quotaColumn = -1;
        state.insertedColumn = false;
        state.overlayPositioned = false;
        state.applied.clear();
        return;
    }

    try {
        auto targetGrid = state.injectionParent;
        int quotaCol = RemoveQuotaChildren(targetGrid, state);
        // Only touch column definitions when our own child was actually found. If Explorer
        // already tore the subtree down, a stale index would point at a column it owns, and
        // removing that is exactly how a tray ends up with overlapping icons.
        if (state.insertedColumn && quotaCol >= 0 &&
            quotaCol < (int)targetGrid.ColumnDefinitions().Size()) {
            for (uint32_t i = 0; i < targetGrid.Children().Size(); ++i) {
                auto child = targetGrid.Children().GetAt(i).try_as<FrameworkElement>();
                if (child) {
                    int childCol = Grid::GetColumn(child);
                    if (childCol > quotaCol) Grid::SetColumn(child, childCol - 1);
                }
            }
            targetGrid.ColumnDefinitions().RemoveAt(quotaCol);
        }
    } catch (...) {
        Wh_Log(L"RemoveQuotaGrid: exception");
    }

    state.quotaGrid = nullptr;
    state.injectionParent = nullptr;
    state.quotaColumn = -1;
    state.insertedColumn = false;
    state.overlayPositioned = false;
    state.applied.clear();
}

// True only when our element is still attached to the grid that the current position
// setting resolves to. Checking the object identity of the parent as well as mere
// non-null parentage is what catches Explorer swapping the whole tray frame out.
static bool IsQuotaUiLive(QuotaUiInstance& state, FrameworkElement const& root,
                          std::wstring const& position) {
    if (!state.quotaGrid || !state.injectionParent) return false;

    try {
        // Cheap test first: a detached element has no visual parent at all.
        if (!VisualTreeHelper::GetParent(state.quotaGrid)) return false;

        // Then confirm we are still parented to the grid the position setting resolves to
        // now, rather than to an orphaned tray frame Explorer has already replaced.
        InjectionTarget current = ResolveInjectionTarget(root, position);
        if (!current.grid) return false;
        return current.grid == state.injectionParent;
    } catch (...) {
        return false;
    }
}

// Anchored taskbar positions have no column to live in, so the bars are positioned by
// margin and the anchor button is pushed aside to make room. Both have to be recomputed
// on every layout pass, because the taskbar recentres itself as buttons come and go.
static void AttachAnchorTracking(QuotaUiInstance& state, Grid const& rootGrid,
                                 FrameworkElement const& anchor, bool anchorOnLeft,
                                 int leftMargin, int rightMargin) {
    state.trackedElement = anchor;
    state.trackedOriginalMargin = anchor.Margin();
    state.hasTrackedOriginalMargin = true;
    state.trackAnchorOnLeft = anchorOnLeft;

    HWND hWnd = state.hWnd;
    state.layoutUpdatedToken = rootGrid.LayoutUpdated(
        [hWnd, leftMargin, rightMargin](winrt::Windows::Foundation::IInspectable const&,
                                        winrt::Windows::Foundation::IInspectable const&) {
            if (g_unloading) return;

            auto* uiState = FindUiState(hWnd);
            if (!uiState || !uiState->quotaGrid || !uiState->trackedElement) return;

            try {
                bool visible = uiState->quotaGrid.Visibility() == Visibility::Visible;
                double width = visible ? uiState->quotaGrid.ActualWidth() : 0.0;
                double gap = visible ? width + leftMargin + rightMargin : 0.0;

                auto margin = uiState->hasTrackedOriginalMargin ? uiState->trackedOriginalMargin
                                                                : uiState->trackedElement.Margin();
                auto current = uiState->trackedElement.Margin();
                if (uiState->trackAnchorOnLeft) {
                    if (std::abs(current.Left - gap) > 1.0) {
                        margin.Left = gap;
                        uiState->trackedElement.Margin(margin);
                    }
                } else {
                    if (std::abs(current.Right - gap) > 1.0) {
                        margin.Right = gap;
                        uiState->trackedElement.Margin(margin);
                    }
                }

                if (!visible) return;

                auto point = uiState->trackedElement.TransformToVisual(uiState->injectionParent)
                                 .TransformPoint({0, 0});
                double left = uiState->trackAnchorOnLeft
                                  ? point.X - gap + leftMargin
                                  : point.X + uiState->trackedElement.ActualWidth() + leftMargin;
                auto quotaMargin = uiState->quotaGrid.Margin();
                if (std::abs(quotaMargin.Left - left) > 1.0) {
                    uiState->quotaGrid.Margin({left, 0, 0, 0});
                }
            } catch (...) {
                // The anchor was recycled out of the repeater; stop chasing it and let the
                // watchdog rebuild on the next pass.
                uiState->trackedElement = nullptr;
                uiState->hasTrackedOriginalMargin = false;
            }
        });
    state.hasLayoutUpdatedToken = true;
}

static bool InjectQuotaGrid(HWND hWnd) {
    if (!hWnd) return false;

    auto fail = [&](PCWSTR reason) {
        ULONGLONG now = NowUnixMs();
        ULONGLONG nextLogMs = g_nextInjectFailureLogMs.load(std::memory_order_acquire);
        if (now >= nextLogMs &&
            g_nextInjectFailureLogMs.compare_exchange_strong(nextLogMs, now + 5000,
                                                            std::memory_order_acq_rel)) {
            wchar_t cls[64] = {};
            GetClassNameW(hWnd, cls, ARRAYSIZE(cls));
            Wh_Log(L"InjectQuotaGrid failed: hwnd=%p class=%s reason=%s", hWnd, cls, reason);
        }
        return false;
    };

    QuotaUiInstance* state = nullptr;
    try {
        auto xamlRoot = GetTaskbarXamlRoot(hWnd);
        if (!xamlRoot) return fail(L"no XamlRoot");
        auto root = xamlRoot.Content().try_as<FrameworkElement>();
        if (!root) return fail(L"no XamlRoot content");

        std::wstring position;
        int leftMargin, rightMargin;
        {
            std::lock_guard<std::mutex> lk(g_settingsMutex);
            position = g_settings.position;
            leftMargin = g_settings.leftMargin;
            rightMargin = g_settings.rightMargin;
        }

        // On a cold start the XamlRoot exists before the tray and taskbar contents are
        // realised, so the target may legitimately be missing for the first few attempts.
        // Bail and let the watchdog poll; never inject somewhere else, which would draw
        // the bars on top of the clock or the task buttons.
        InjectionTarget target = ResolveInjectionTarget(root, position);
        if (!target.grid) return fail(L"position target not in the visual tree yet");

        state = FindUiState(hWnd);
        if (!state) {
            auto newState = std::make_unique<QuotaUiInstance>();
            newState->hWnd = hWnd;
            state = newState.get();
            g_uiInstances.push_back(std::move(newState));
        }
        state->injectionParent = target.grid;

        int oldCol = RemoveQuotaChildren(target.grid, *state);
        if (oldCol < 0 && state->insertedColumn && state->quotaColumn >= 0 &&
            state->quotaColumn < (int)target.grid.ColumnDefinitions().Size()) {
            // Explorer dropped our child but left behind the column we inserted. Reclaim it,
            // but only if nothing else has moved into it, so repeated re-injection after a
            // tray rebuild cannot pile up stray columns.
            bool occupied = false;
            for (uint32_t i = 0; i < target.grid.Children().Size(); ++i) {
                auto child = target.grid.Children().GetAt(i).try_as<FrameworkElement>();
                if (child && Grid::GetColumn(child) == state->quotaColumn) {
                    occupied = true;
                    break;
                }
            }
            if (!occupied) oldCol = state->quotaColumn;
        }
        if (state->insertedColumn && oldCol >= 0 &&
            oldCol < (int)target.grid.ColumnDefinitions().Size()) {
            for (uint32_t i = 0; i < target.grid.Children().Size(); ++i) {
                auto child = target.grid.Children().GetAt(i).try_as<FrameworkElement>();
                if (child) {
                    int childCol = Grid::GetColumn(child);
                    if (childCol > oldCol) Grid::SetColumn(child, childCol - 1);
                }
            }
            target.grid.ColumnDefinitions().RemoveAt(oldCol);
        }
        state->insertedColumn = false;
        state->quotaColumn = -1;

        Grid quota = BuildQuotaGrid(*state);
        if (!quota) {
            ClearQuotaEventState(*state);
            state->injectionParent = nullptr;
            state->applied.clear();
            EraseUiState(hWnd);
            return fail(L"BuildQuotaGrid failed");
        }

        bool overlay = IsOverlayPosition(position);
        bool anchored = IsAnchoredPosition(position);
        state->overlayPositioned = overlay || anchored;

        if (!state->overlayPositioned) {
            ColumnDefinition newCol;
            newCol.Width({1.0, GridUnitType::Auto});
            int columnCount = (int)target.grid.ColumnDefinitions().Size();
            int insertCol = std::clamp(target.column, 0, columnCount);
            if (insertCol >= columnCount) {
                target.grid.ColumnDefinitions().Append(newCol);
            } else {
                target.grid.ColumnDefinitions().InsertAt(insertCol, newCol);
                for (uint32_t i = 0; i < target.grid.Children().Size(); ++i) {
                    auto child = target.grid.Children().GetAt(i).try_as<FrameworkElement>();
                    if (child) {
                        int childCol = Grid::GetColumn(child);
                        if (childCol >= insertCol) Grid::SetColumn(child, childCol + 1);
                    }
                }
            }
            quota.Margin({(double)leftMargin, 0, (double)rightMargin, 0});
            Grid::SetColumn(quota, insertCol);
            target.grid.Children().Append(quota);
            state->quotaColumn = insertCol;
            state->insertedColumn = true;
        } else {
            quota.HorizontalAlignment(HorizontalAlignment::Left);
            if (position == L"taskbar_center_edge") {
                quota.HorizontalAlignment(HorizontalAlignment::Center);
                quota.Margin({(double)leftMargin, 0, (double)rightMargin, 0});
            } else if (position == L"taskbar_right_edge") {
                quota.HorizontalAlignment(HorizontalAlignment::Right);
                double right = rightMargin;
                if (auto trayFrame = FindChildByName(root, L"SystemTrayFrameGrid")) {
                    right += trayFrame.ActualWidth() + 4;
                }
                quota.Margin({(double)leftMargin, 0, right, 0});
            } else {
                quota.Margin({(double)leftMargin, 0, (double)rightMargin, 0});
            }
            Grid::SetColumn(quota, 0);
            Canvas::SetZIndex(quota, 1000);
            target.grid.Children().Append(quota);

            if (anchored) {
                bool anchorOnLeft = false;
                if (auto anchor = ResolveAnchorElement(root, position, &anchorOnLeft)) {
                    AttachAnchorTracking(*state, target.grid, anchor, anchorOnLeft, leftMargin,
                                         rightMargin);
                }
            }
        }

        state->quotaGrid = quota;
        g_uiInjected.store(true, std::memory_order_release);
        state->applied.clear();

        // Keeps the in-bar countdown honest between polls, without extra API calls.
        if (!state->tickTimer) {
            DispatcherTimer tick;
            tick.Interval(std::chrono::seconds(30));
            state->tickTimerToken =
                tick.Tick([hWnd](winrt::Windows::Foundation::IInspectable const&,
                                 winrt::Windows::Foundation::IInspectable const&) {
                    if (g_unloading) return;
                    auto* uiState = FindUiState(hWnd);
                    if (uiState && uiState->quotaGrid) UpdateQuotaUi(*uiState);
                });
            state->tickTimer = tick;
            tick.Start();
        }

        UpdateQuotaUi(*state);
        Wh_Log(L"Injected quota bars at position %s", position.c_str());
        return true;
    } catch (...) {
        if (state) {
            RemoveQuotaGridFromState(*state);
            EraseUiState(hWnd);
        }
        g_uiInjected.store(!g_uiInstances.empty(), std::memory_order_release);
        Wh_Log(L"InjectQuotaGrid: exception");
        return false;
    }
}

// Re-injects when the element has been detached. This is the actual fix for the bars
// randomly disappearing: the old code short-circuited on a non-null quotaGrid, but a
// WinRT reference stays perfectly valid after Explorer drops the element from the tree,
// so re-injection silently did nothing and the bars never came back.
static bool EnsureQuotaGrid(HWND hWnd) {
    if (!hWnd) return false;

    auto* state = FindUiState(hWnd);
    if (state && state->quotaGrid) {
        std::wstring position;
        {
            std::lock_guard<std::mutex> lk(g_settingsMutex);
            position = g_settings.position;
        }
        try {
            auto xamlRoot = GetTaskbarXamlRoot(hWnd);
            auto root = xamlRoot ? xamlRoot.Content().try_as<FrameworkElement>() : nullptr;
            if (root && IsQuotaUiLive(*state, root, position)) return true;
        } catch (...) {}

        Wh_Log(L"Quota bars were detached from the taskbar; re-injecting");
        RemoveQuotaGridFromState(*state);
    }
    return InjectQuotaGrid(hWnd);
}

static void RemoveQuotaGrid(HWND hWnd) {
    auto* state = FindUiState(hWnd);
    if (!state) return;

    RemoveQuotaGridFromState(*state);
    EraseUiState(hWnd);
}

static void RemoveAllQuotaGrids(bool waitForCompletion = false) {
    std::vector<HWND> hWnds;
    hWnds.reserve(g_uiInstances.size());
    for (auto& state : g_uiInstances) hWnds.push_back(state->hWnd);

    for (HWND hWnd : hWnds) {
        if (!hWnd || !IsWindow(hWnd)) continue;
        if (!RunFromWindowThread(hWnd, [](void* param) -> bool {
                RemoveQuotaGrid(static_cast<HWND>(param));
                return true;
            }, hWnd, waitForCompletion ? INFINITE : 2000)) {
            Wh_Log(L"RemoveQuotaGrid marshal failed");
        }
    }

    for (auto& state : g_uiInstances) {
        if (state->hWnd && IsWindow(state->hWnd)) continue;

        try {
            if (state->tickTimer) {
                state->tickTimer.Stop();
                state->tickTimer.Tick(state->tickTimerToken);
            }
        } catch (...) {}
        state->tickTimer = nullptr;
        state->tickTimerToken = {};
        state->trackedElement = nullptr;
        state->hasTrackedOriginalMargin = false;
        state->hasLayoutUpdatedToken = false;
        state->layoutUpdatedToken = {};
        state->pointerHandlers.clear();
        state->quotaGrid = nullptr;
        state->injectionParent = nullptr;
        state->quotaColumn = -1;
        state->insertedColumn = false;
        state->overlayPositioned = false;
        state->applied.clear();
    }
    g_uiInstances.erase(std::remove_if(g_uiInstances.begin(), g_uiInstances.end(),
        [](const auto& state) { return !state->hWnd || !IsWindow(state->hWnd); }),
        g_uiInstances.end());
    g_uiInjected.store(!g_uiInstances.empty(), std::memory_order_release);
}

static DWORD WINAPI RetryInjectThreadProc(LPVOID) {
    // A permanent watchdog rather than a bounded retry loop. Explorer rebuilds the tray's
    // visual tree for plenty of reasons that never reach TrayUI::StartTaskbar - tray icon
    // churn, DPI and monitor changes, a tray host restart, an Explorer refresh - and it
    // drops our element when it does. Polling for that is what stops the bars from
    // vanishing until the mod is manually reloaded.
    while (!g_unloading) {
        auto hWnds = FindCurrentProcessTaskbarWnds();
        TaskbarMonitorMode mode;
        int targetMonitorNumber;
        {
            std::lock_guard<std::mutex> lk(g_settingsMutex);
            mode = g_settings.taskbarMonitorMode;
            targetMonitorNumber = g_settings.taskbarMonitorNumber;
        }
        if (hWnds.empty() || (mode == TaskbarMonitorMode::All && hWnds.size() < 2)) {
            ULONGLONG now = NowUnixMs();
            ULONGLONG nextLogMs = g_nextInjectFailureLogMs.load(std::memory_order_acquire);
            if (now >= nextLogMs &&
                g_nextInjectFailureLogMs.compare_exchange_strong(nextLogMs, now + 5000,
                                                                std::memory_order_acq_rel)) {
                Wh_Log(L"Taskbar discovery: mode=%d target=%d eligible=%zu",
                       (int)mode, targetMonitorNumber, hWnds.size());
            }
        }

        bool allInjected = !hWnds.empty();
        for (HWND hWnd : hWnds) {
            bool injected = RunFromWindowThread(hWnd, [](void* param) -> bool {
                return !g_unloading && EnsureQuotaGrid(static_cast<HWND>(param));
            }, hWnd);
            allInjected = allInjected && injected;
        }

        // Poll hard while something is missing, then settle into a cheap heartbeat.
        DWORD waitMs = allInjected ? 3000 : 100;
        if (g_stopEvent && g_wakeInjectEvent) {
            HANDLE waits[2] = {g_stopEvent, g_wakeInjectEvent};
            if (WaitForMultipleObjects(2, waits, FALSE, waitMs) == WAIT_OBJECT_0) break;
        } else if (g_stopEvent) {
            if (WaitForSingleObject(g_stopEvent, waitMs) == WAIT_OBJECT_0) break;
        } else {
            Sleep(waitMs);
        }
    }
    return 0;
}

static void StartRetryInject() {
    if (g_unloading) return;

    if (g_retryThread) {
        DWORD threadState = WaitForSingleObject(g_retryThread, 0);
        if (threadState == WAIT_TIMEOUT) {
            // Watchdog already running - nudge it so a settings change applies at once
            // instead of waiting out the heartbeat.
            if (g_wakeInjectEvent) SetEvent(g_wakeInjectEvent);
            return;
        }
        CloseHandle(g_retryThread);
        g_retryThread = nullptr;
    }

    g_retryThread = CreateThread(nullptr, 0, RetryInjectThreadProc, nullptr, 0, nullptr);
    if (g_retryThread) return;

    DWORD err = GetLastError();
    Wh_Log(L"CreateThread RetryInjectThreadProc failed: %lu", err);

    bool anyInjected = false;
    for (HWND hWnd : FindCurrentProcessTaskbarWnds()) {
        if (RunFromWindowThread(hWnd, [](void* param) -> bool {
                return !g_unloading && EnsureQuotaGrid(static_cast<HWND>(param));
            }, hWnd)) {
            anyInjected = true;
        }
    }
    if (!anyInjected) {
        Wh_Log(L"InjectQuotaGrid fallback failed");
    }
}

/**********************************************/
//  Hooks
/**********************************************/

using TrayUI_StartTaskbar_t = void(WINAPI*)(void*);
static TrayUI_StartTaskbar_t TrayUI_StartTaskbar_Original = nullptr;

static void WINAPI TrayUI_StartTaskbar_Hook(void* pThis) {
    TrayUI_StartTaskbar_Original(pThis);
    if (g_unloading) return;

    RemoveAllQuotaGrids();
    StartRetryInject();
}

static bool HookTaskbarDllSymbols() {
    HMODULE h = LoadLibraryExW(L"taskbar.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!h) return false;

    WindhawkUtils::SYMBOL_HOOK taskbarDllHooks[] = {
        {{LR"(const CTaskBand::`vftable'{for `ITaskListWndSite'})"}, &CTaskBand_ITaskListWndSite_vftable},
        {{LR"(const CSecondaryTaskBand::`vftable'{for `ITaskListWndSite'})"}, &CSecondaryTaskBand_ITaskListWndSite_vftable},
        {{LR"(public: virtual class std::shared_ptr<class TaskbarHost> __cdecl CTaskBand::GetTaskbarHost(void)const )"}, &CTaskBand_GetTaskbarHost_Original},
        {{LR"(public: virtual class std::shared_ptr<class TaskbarHost> __cdecl CSecondaryTaskBand::GetTaskbarHost(void)const )"}, &CSecondaryTaskBand_GetTaskbarHost_Original},
        {{LR"(public: int __cdecl TaskbarHost::FrameHeight(void)const )"}, &TaskbarHost_FrameHeight_Original},
        {{LR"(public: void __cdecl std::_Ref_count_base::_Decref(void))"}, &Std_Ref_Decref_Original},
        {{LR"(public: virtual void __cdecl TrayUI::StartTaskbar(void))"}, &TrayUI_StartTaskbar_Original, TrayUI_StartTaskbar_Hook},
    };
    return WindhawkUtils::HookSymbols(h, taskbarDllHooks, ARRAYSIZE(taskbarDllHooks));
}

/**********************************************/
//  Settings
/**********************************************/

// Parses a per-account "<length> <thickness>" override. Any missing field stays 0,
// which ResolveBar treats as "inherit the global setting".
static BarSpec ParseBarSpec(std::wstring const& text) {
    BarSpec spec;
    int values[2] = {0, 0};
    int count = 0;
    size_t pos = 0;
    while (count < 2 && pos < text.size()) {
        while (pos < text.size() && (text[pos] < L'0' || text[pos] > L'9')) pos++;
        if (pos >= text.size()) break;

        wchar_t* end = nullptr;
        long value = wcstol(text.c_str() + pos, &end, 10);
        if (!end || end == text.c_str() + pos) break;

        values[count++] = (int)value;
        pos = (size_t)(end - text.c_str());
    }
    if (count >= 1) spec.length = std::max(values[0], 0);
    if (count >= 2) spec.thickness = std::max(values[1], 0);
    return spec;
}

static void LoadSettings() {
    Settings s;

    auto getAccountText = [](PCWSTR name, int index) {
        PCWSTR text = Wh_GetStringSetting(name, index);
        std::wstring value = text;
        Wh_FreeStringSetting(text);
        return value;
    };

    for (int i = 0;; i++) {
        PCWSTR provider = Wh_GetStringSetting(L"accounts[%d].provider", i);
        if (!*provider) {
            Wh_FreeStringSetting(provider);
            break;
        }

        AccountConfig a;
        std::wstring providerSetting = provider;
        Wh_FreeStringSetting(provider);
        // Bare "anthropic"/"openai"; older configs used "<provider>-<source>" - keep the provider.
        a.provider = providerSetting.find(L"openai") != std::wstring::npos ? L"openai" : L"anthropic";

        a.label = getAccountText(L"accounts[%d].label", i);
        if (a.label.empty()) a.label = a.provider == L"anthropic" ? L"A" : L"O";

        a.primary = ParseBarSpec(getAccountText(L"accounts[%d].primaryBar", i));
        a.secondary = ParseBarSpec(getAccountText(L"accounts[%d].secondaryBar", i));
        a.reset = ParseBarSpec(getAccountText(L"accounts[%d].resetBar", i));

        std::wstring layoutOverride = getAccountText(L"accounts[%d].barLayout", i);
        if (layoutOverride == L"stacked" || layoutOverride == L"vertical") {
            a.hasLayoutOverride = true;
            a.layoutOverride =
                layoutOverride == L"vertical" ? BarLayout::Vertical : BarLayout::Stacked;
        }

        s.accounts.push_back(std::move(a));
    }

    if (s.accounts.empty()) {
        s.accounts.push_back({L"anthropic", L"A"});
        s.accounts.push_back({L"openai", L"O"});
    }

    // Apply persisted show/hide toggles from mod storage (semicolon-separated identity hashes).
    {
        wchar_t buf[4096] = {};
        Wh_GetStringValue(L"hiddenAccounts", buf, ARRAYSIZE(buf));
        std::vector<uint64_t> hiddenHashes;
        for (std::wstring rest = buf; !rest.empty();) {
            size_t end = rest.find(L';');
            std::wstring tok = rest.substr(0, end);
            if (!tok.empty()) hiddenHashes.push_back(wcstoull(tok.c_str(), nullptr, 16));
            if (end == std::wstring::npos) break;
            rest.erase(0, end + 1);
        }
        int visibleCount = 0;
        for (auto& a : s.accounts) {
            a.hidden = std::find(hiddenHashes.begin(), hiddenHashes.end(),
                                 AccountIdentityHash(a)) != hiddenHashes.end();
            if (!a.hidden) visibleCount++;
        }
        // Never persist into an all-hidden state with no bar left to right-click.
        if (visibleCount == 0 && !s.accounts.empty()) s.accounts[0].hidden = false;
    }

    auto getSettingText = [](PCWSTR name) {
        PCWSTR text = Wh_GetStringSetting(name);
        std::wstring value = text;
        Wh_FreeStringSetting(text);
        return value;
    };
    auto getIntSetting = [&](PCWSTR name, int defaultValue) {
        std::wstring text = getSettingText(name);
        return text.empty() ? defaultValue : _wtoi(text.c_str());
    };
    auto getBoolSetting = [&](PCWSTR name, bool defaultValue) {
        std::wstring text = getSettingText(name);
        if (text.empty()) return defaultValue;
        if (_wcsicmp(text.c_str(), L"true") == 0) return true;
        if (_wcsicmp(text.c_str(), L"false") == 0) return false;
        return _wtoi(text.c_str()) != 0;
    };

    int pollMinutes = getIntSetting(L"pollIntervalMinutes", 10);
    int labelFontSize = getIntSetting(L"labelFontSize", 11);
    int accountMargin = getIntSetting(L"accountMargin", 3);
    int labelGap = getIntSetting(L"labelGap", 3);
    int barGap = getIntSetting(L"barGap", 2);
    int resetBarGap = getIntSetting(L"resetBarGap", 2);
    int rightMargin = getIntSetting(L"rightMargin", 4);
    int leftMargin = getIntSetting(L"leftMargin", 4);
    int yellowThreshold = getIntSetting(L"yellowThreshold", 50);
    int orangeThreshold = getIntSetting(L"orangeThreshold", 75);
    int redThreshold = getIntSetting(L"redThreshold", 90);

    // Each bar carries its own dimensions. Nothing here is shared between the primary
    // window, the secondary window, and the reset strip.
    s.primaryBar.length = std::max(getIntSetting(L"barPrimaryLength", 110), 10);
    s.primaryBar.thickness = std::clamp(getIntSetting(L"barPrimaryThickness", 15), 2, 64);
    s.secondaryBar.length = std::max(getIntSetting(L"barSecondaryLength", 110), 10);
    s.secondaryBar.thickness = std::clamp(getIntSetting(L"barSecondaryThickness", 8), 2, 64);
    s.resetBar.length = std::max(getIntSetting(L"barResetLength", 0), 0);
    s.resetBar.thickness = std::clamp(getIntSetting(L"barResetThickness", 3), 1, 32);
    s.percentFontSize = std::clamp(getIntSetting(L"percentFontSize", 11), 6, 32);
    s.resetFontSize = std::clamp(getIntSetting(L"resetFontSize", 10), 6, 32);
    s.barTextPadding = std::clamp(getIntSetting(L"barTextPadding", 5), 0, 40);
    s.barCornerRadius = std::clamp(getIntSetting(L"barCornerRadius", -1), -1, 64);

    std::wstring position = getSettingText(L"position");
    s.position = position.empty() ? std::wstring(L"tray_left") : position;

    PCWSTR taskbarMonitorModeText = Wh_GetStringSetting(L"taskbarMonitorMode");
    std::wstring taskbarMonitorMode = taskbarMonitorModeText;
    Wh_FreeStringSetting(taskbarMonitorModeText);
    if (taskbarMonitorMode == L"all") {
        s.taskbarMonitorMode = TaskbarMonitorMode::All;
    } else if (taskbarMonitorMode == L"specific") {
        s.taskbarMonitorMode = TaskbarMonitorMode::Specific;
    } else {
        s.taskbarMonitorMode = TaskbarMonitorMode::Primary;
    }
    std::wstring clickAction = getSettingText(L"clickAction");
    s.clickAction = clickAction == L"open-dashboard" ? ClickAction::OpenDashboard : ClickAction::Refresh;
    std::wstring barLayout = getSettingText(L"barLayout");
    s.barLayout = barLayout == L"vertical" ? BarLayout::Vertical : BarLayout::Stacked;
    std::wstring barMode = getSettingText(L"barMode");
    s.barMode = barMode == L"remaining" ? BarMode::Remaining : BarMode::Used;
    std::wstring resetBarMode = getSettingText(L"resetBarMode");
    s.resetBarElapsed = resetBarMode != L"remaining";

    auto getBarWindows = [&](PCWSTR name, BarWindows defaultValue) {
        std::wstring text = getSettingText(name);
        if (text == L"primary") return BarWindows::Primary;
        if (text == L"secondary") return BarWindows::Secondary;
        if (text == L"both") return BarWindows::Both;
        return defaultValue;
    };
    s.resetBarWindows = getBarWindows(L"resetBarWindows", BarWindows::Primary);
    s.percentTextWindows = getBarWindows(L"percentTextWindows", BarWindows::Primary);
    s.resetTextWindows = getBarWindows(L"resetTextWindows", BarWindows::Primary);

    int taskbarMonitorNumber = getIntSetting(L"taskbarMonitorNumber", 1);

    s.pollMinutes = std::clamp(pollMinutes > 0 ? pollMinutes : 10, 2, 24 * 60);
    s.taskbarMonitorNumber = std::clamp(taskbarMonitorNumber > 0 ? taskbarMonitorNumber : 1, 1, 64);
    s.labelFontSize = std::clamp(labelFontSize > 0 ? labelFontSize : 11, 6, 24);
    s.accountMargin = std::max(accountMargin, 0);
    s.labelGap = std::max(labelGap, 0);
    s.barGap = std::max(barGap, 0);
    s.resetBarGap = std::max(resetBarGap, 0);
    s.rightMargin = std::max(rightMargin, 0);
    s.leftMargin = std::max(leftMargin, 0);
    s.yellowThreshold = std::clamp(yellowThreshold, 0, 100);
    s.orangeThreshold = std::clamp(orangeThreshold, s.yellowThreshold, 100);
    s.redThreshold = std::clamp(redThreshold, s.orangeThreshold, 100);
    s.showLabels = getBoolSetting(L"showLabels", true);
    s.labelOnLeft = getBoolSetting(L"labelOnLeft", true);
    s.showPercentText = getBoolSetting(L"showPercentText", true);
    s.showResetText = getBoolSetting(L"showResetText", true);
    s.showResetBar = getBoolSetting(L"showResetBar", true);
    s.showCodexSparkInTooltip = getBoolSetting(L"showCodexSparkInTooltip", false);
    s.colorblindMode = getBoolSetting(L"colorblindMode", false);
    s.showStaleWarning = getBoolSetting(L"showStaleWarning", true);
    s.enableNotifications = getBoolSetting(L"enableNotifications", true);
    s.diagnosticLogUsageResponses = getBoolSetting(L"diagnosticLogUsageResponses", false);

    {
        std::lock_guard<std::mutex> lk(g_settingsMutex);
        std::lock_guard<std::mutex> lk2(g_dataMutex);
        std::vector<AccountData> newData(s.accounts.size());
        std::vector<bool> oldDataUsed(g_data.size(), false);
        for (size_t i = 0; i < s.accounts.size(); i++) {
            for (size_t j = 0; j < g_settings.accounts.size() && j < g_data.size(); j++) {
                if (oldDataUsed[j]) continue;

                if (AccountIdentityHash(g_settings.accounts[j]) ==
                    AccountIdentityHash(s.accounts[i])) {
                    newData[i] = g_data[j];
                    oldDataUsed[j] = true;
                    break;
                }
            }
        }

        g_settings = std::move(s);
        g_settingsGeneration++;
        g_data = std::move(newData);
    }
}

/**********************************************/
//  Lifecycle
/**********************************************/

BOOL Wh_ModInit() {
    Wh_Log(L"Init");
    g_unloading = false;
    g_refreshing = false;
    g_refreshAccountIndex = -1;
    g_refreshGeneration = 0;
    g_uiInjected.store(false, std::memory_order_release);
    g_fetchThreadStarted.store(false, std::memory_order_release);
    g_loginInProgress.store(false);
    LoadSettings();

    WSADATA wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        Wh_Log(L"WSAStartup failed; OpenAI sign-in unavailable");
    }

    if (!HookTaskbarDllSymbols()) {
        Wh_Log(L"HookTaskbarDllSymbols failed");
        return FALSE;
    }

    g_stopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    g_refreshEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    g_wakeInjectEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    return g_stopEvent && g_refreshEvent && g_wakeInjectEvent;
}

void Wh_ModAfterInit() {
    g_fetchThread = CreateThread(nullptr, 0, FetchThreadProc, nullptr, 0, nullptr);
    if (g_fetchThread) {
        g_fetchThreadStarted.store(true, std::memory_order_release);
    } else {
        DWORD err = GetLastError();
        Wh_Log(L"CreateThread FetchThreadProc failed: %lu", err);
        g_refreshing = false;
        g_fetchThreadStarted.store(false, std::memory_order_release);
        {
            std::lock_guard<std::mutex> lk(g_dataMutex);
            for (auto& data : g_data) {
                data.stale = true;
                data.error = L"fetch thread failed";
            }
        }
    }

    StartRetryInject();
    if (!g_fetchThread) PostUiUpdate();
}

void Wh_ModUninit() {
    Wh_Log(L"Uninit");
    g_unloading = true;
    g_uiInjected.store(false, std::memory_order_release);
    if (g_stopEvent) SetEvent(g_stopEvent);
    CloseActiveHttpHandles();

    // Unblock an in-flight sign-in: close the paste dialog and/or the loopback listener so the
    // login thread falls out of its message/accept loop, then join it. The lock pairs with
    // StartLogin (g_unloading is already set) so a concurrent click can't spawn a thread we miss.
    {
        std::lock_guard<std::mutex> lk(g_loginThreadMutex);
        if (HWND loginWnd = g_loginWnd.load()) PostMessageW(loginWnd, WM_CLOSE, 0, 0);
        if (SOCKET s = g_loginSocket.exchange(INVALID_SOCKET); s != INVALID_SOCKET) closesocket(s);
        if (g_loginThread) {
            WaitForSingleObject(g_loginThread, INFINITE);
            CloseHandle(g_loginThread);
            g_loginThread = nullptr;
        }
    }
    g_loginInProgress.store(false);

    if (g_retryThread) {
        WaitForSingleObject(g_retryThread, INFINITE);
        CloseHandle(g_retryThread);
        g_retryThread = nullptr;
    }

    if (g_fetchThread) {
        WaitForSingleObject(g_fetchThread, INFINITE);
        CloseHandle(g_fetchThread);
        g_fetchThread = nullptr;
    }
    g_fetchThreadStarted.store(false, std::memory_order_release);

    RemoveAllQuotaGrids(true);

    if (g_stopEvent) CloseHandle(g_stopEvent);
    if (g_refreshEvent) CloseHandle(g_refreshEvent);
    if (g_wakeInjectEvent) CloseHandle(g_wakeInjectEvent);
    g_stopEvent = nullptr;
    g_refreshEvent = nullptr;
    g_wakeInjectEvent = nullptr;
    WSACleanup();
}

void Wh_ModSettingsChanged() {
    Wh_Log(L"SettingsChanged");
    LoadSettings();
    g_refreshAccountIndex = -1;
    g_refreshGeneration++;

    RemoveAllQuotaGrids();
    StartRetryInject();
    if (g_refreshEvent) SetEvent(g_refreshEvent);
}
