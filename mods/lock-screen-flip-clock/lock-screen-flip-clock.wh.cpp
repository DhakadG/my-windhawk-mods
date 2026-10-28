// ==WindhawkMod==
// @id              lock-screen-flip-clock
// @name            Lock Screen Flip Clock
// @description     A mechanical flip clock for the lock screen: paper-grain cards that really flip, an airport-board roll-in, synthesised flap sounds and a vibration thump
// @version         1.1.0
// @author          lost_husky
// @github          https://github.com/DhakadG
// @homepage        https://github.com/DhakadG/my-windhawk-mods/tree/main/mods/lock-screen-flip-clock
// @donateUrl       https://ko-fi.com/losthusky_
// @license         MIT
// @include         LockApp.exe
// @architecture    x86-64
// @compilerOptions -ld2d1 -ld3d11 -ldxgi -lole32 -loleaut32 -lpowrprof -lruntimeobject -lxaudio2_9
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Lock Screen Flip Clock

Turns the Windows 11 lock screen into a mechanical flip clock: one card per
digit, raised puff-print numerals on paper-grain cards, and flaps that fall
over and land like the real thing.

![The flip clock on the lock screen](https://raw.githubusercontent.com/DhakadG/my-windhawk-mods/main/docs/media/lock-screen-flip-clock/lock-screen.png)

![A minute flipping over](https://raw.githubusercontent.com/DhakadG/my-windhawk-mods/main/docs/media/lock-screen-flip-clock/flip.gif)

## What it does

- **Real flaps.** Each card is four layers: the top flap tips over in
  perspective and the bottom flap swings down and lands, shadowed as it turns
  away from the light. Eight flip styles: Classic, Gravity (a simulated fall
  that bounces off the stack), Snappy, Bouncy, Slow fold (with a sheen),
  Cascade (digit by digit, like a departure board), Roll and Fade.
- **Airport-board roll-in.** When the lock screen comes up, every card starts
  at 0 and rattles forward flap by flap to the time.
- **Sound, made from scratch.** No audio files: every flap is synthesised as it
  lands (filtered noise bursts and a short knock through one limiter), so
  several cards changing at once rattle like a real board. Airport board,
  Mechanical (the rush of the falling flap, then the clack), Click, Whoosh or
  Soft tap.
- **Vibration thump.** A phone's haptic tick, rebuilt as sound: a low knock
  that sinks like a motor spinning down, with just enough harmonics to carry on
  laptop speakers.
- **Two card styles.** A card per digit (LyricDock's), or one card for the hours
  and one for the minutes - the same numerals, two to a card.
- **On the cards:** the date and the weekday in their top corners, the seconds
  as small flip cards on the minutes card, AM / PM in the hours card's corner or
  as its own small flip card - each a toggle.
- Dark or light cards (or automatic: light by day, dark at night), full-size
  seconds, 12/24-hour, cards on/off, size, gaps, dim, and a black, dimmed or
  wallpaper background.
- Silent while the display is off: a locked PC does not click all night.

The design and the motion are ported from the flip clock in
[LyricDock](https://github.com/DhakadG/lyricdock). The lock screen plumbing
follows [Lock Screen Clock Styler](https://windhawk.net/mods/lock-screen-clock-styler)
by ali0zdas: it runs on the lock screen's own UI thread and uses no XAML
Diagnostics TAP, so it coexists with other XAML mods.

Only `LockApp.exe` is touched - never `LogonUI.exe` or the secure desktop where
you type your password.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- cardStyle: digits
  $name: Card style
  $description: >-
    A card per digit, or one card for the hours and one for the minutes - the
    same raised puff-print numerals, two to a card.
  $options:
  - digits: A card per digit
  - pairs: A card for the hours, one for the minutes
- theme: dark
  $name: Card colour
  $options:
  - dark: Dark cards, light numerals
  - light: Light cards, dark numerals
  - auto: Automatic (light by day, dark at night)
- darkFrom: 22
  $name: Automatic - dark from (hour)
  $description: 0-23. Only used by the Automatic card colour.
- darkTo: 7
  $name: Automatic - light from (hour)
  $description: 0-23. Only used by the Automatic card colour.
- timeFormat: system
  $name: Time format
  $options:
  - system: Follow Windows
  - 12h: 12-hour (AM / PM)
  - 24h: 24-hour
- secondsCard: "off"
  $name: Seconds
  $options:
  - "off": Off
  - small: Small flip cards on the minutes card
  - large: Full-size cards
- ampm: corner
  $name: AM / PM (12-hour time)
  $options:
  - corner: In the corner of the hours card
  - card: A small flip card on the hours card
- cardDate: false
  $name: Date on the hours card
  $description: The short date in its top left corner.
- cardWeekday: false
  $name: Day of the week on the minutes card
  $description: In its top corner.
- animation: classic
  $name: Flip animation
  $description: >-
    Classic: the top half falls, the new bottom half lands. Gravity: falls like
    a real flap, fastest at the bottom, and bounces off the stack. Bouncy: the
    flap overshoots and settles. Slow fold: slower, with a sheen. Cascade:
    every digit flips a beat after the one before. Roll: the digit rolls down
    a drum. Fade: a quiet crossfade.
  $options:
  - classic: Classic (smooth)
  - gravity: Gravity (natural fall)
  - snappy: Snappy
  - bounce: Bouncy
  - fold: Slow fold
  - cascade: Cascade (per digit)
  - roll: Roll
  - fade: Fade
- intro: roll
  $name: When the lock screen appears
  $options:
  - roll: Airport board (every card rolls from 0 to the time)
  - flip: Flip from blank cards
  - none: Nothing
- speed: 100
  $name: Animation speed (%)
  $description: 30-300. Higher is faster.
- sound: mechanical
  $name: Flip sound
  $description: Synthesised as each flap lands, no audio files.
  $options:
  - "off": Off
  - solari: Airport board (clack)
  - mechanical: Mechanical (air + clack)
  - click: Click
  - whoosh: Whoosh (air)
  - soft: Soft tap
- volume: 40
  $name: Sound volume (%)
  $description: 5-100, relative to the system volume.
- latency: 0
  $name: Sound latency (ms)
  $description: >-
    0-500. Bluetooth speakers and audio processors play sound late. If the clack
    comes after the flap lands, raise this until they line up: the flaps then
    wait for the sound.
- soundEvery: minute
  $name: Sound with seconds on
  $options:
  - minute: Only when the minute changes
  - all: Every flip
- thump: light
  $name: Vibration thump
  $description: The phone's haptic tick as sound - a short low knock as the flaps land.
  $options:
  - "off": Off
  - light: Light
  - firm: Firm
- cards: true
  $name: Show the cards
  $description: "Off: only the numerals (the flip still moves them)."
- background: black
  $name: Background
  $options:
  - black: Black (paper with light cards and no cards)
  - dim: Dimmed wallpaper
  - wallpaper: Wallpaper
- layout: auto
  $name: Arrangement
  $options:
  - auto: Follow the screen
  - side: Side by side
  - stacked: Stacked
- size: 96
  $name: Size (%)
  $description: 30-100. How much of the screen the cards fill.
- digitGap: 3
  $name: Gap between digits (% of a card)
  $description: 0-20.
- groupGap: 14
  $name: Gap between hours and minutes (% of a card)
  $description: 0-60.
- dim: 0
  $name: Dim (%)
  $description: 0-85. Darkens the clock.
- date: false
  $name: Show the date
  $description: The long date below the cards.
- hideStockClock: true
  $name: Hide the Windows clock
- hideWidgets: true
  $name: Hide the widgets
  $description: Weather, news and other cards on the lock screen.
*/
// ==/WindhawkModSettings==

// How it works
//
// LockApp is a UWP XAML app; its process is suspended while the PC is unlocked
// and thaws when the lock screen comes up. A worker thread finds its CoreWindow
// and installs a WH_GETMESSAGE hook on that UI thread, so every XAML read and
// write happens there. Settings changes and teardown run synchronously on that
// thread through a WH_CALLWNDPROC hook, so no mod code is left running when the
// DLL unloads.
//
// No XAML event handler is ever registered: a delegate object outliving the
// DLL would call into unloaded code. Every flip is a self-contained Storyboard
// built from precomputed keyframes - image swaps included - so nothing has to
// run when it ends.
//
// The cards are drawn once per size and colour with Direct2D: the Figma "Puff
// 3D" effect stack from LyricDock (drop shadows, inner shadows and noise
// grain), ported from its SVG filters to Direct2D effects. Sounds are rendered
// into one buffer per flip and played through XAudio2.

#include <windhawk_utils.h>

#include <d2d1_1.h>
#include <d2d1effects.h>
#include <d3d11.h>
#include <powersetting.h>
#include <xaudio2.h>

#undef GetCurrentTime  // winbase.h's macro breaks Storyboard::GetCurrentTime

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.Animation.h>
#include <winrt/Windows.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.UI.Xaml.Media.Media3D.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Shapes.h>
#include <winrt/Windows.UI.Xaml.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <set>
#include <string>
#include <vector>

namespace wf = winrt::Windows::Foundation;
namespace wux = winrt::Windows::UI::Xaml;
namespace wuxc = winrt::Windows::UI::Xaml::Controls;
namespace wuxm = winrt::Windows::UI::Xaml::Media;
namespace wuxma = winrt::Windows::UI::Xaml::Media::Animation;
namespace wuxmi = winrt::Windows::UI::Xaml::Media::Imaging;
namespace wuxm3 = winrt::Windows::UI::Xaml::Media::Media3D;
namespace wuxs = winrt::Windows::UI::Xaml::Shapes;

// ---------------------------------------------------------------------------
// Settings
// ---------------------------------------------------------------------------

enum class Anim { Classic, Gravity, Snappy, Bounce, Fold, Cascade, Roll, Fade };
enum class Sound { Off, Solari, Mechanical, Click, Whoosh, Soft };

struct ModSettings {
    int theme = 0;  // 0 dark, 1 light, 2 automatic
    int darkFrom = 22, darkTo = 7;
    bool h24 = false;
    int style = 0;        // 0 a card per digit, 1 pairs
    int secondsCard = 0;  // 0 off, 1 small cards on the minutes card, 2 full size
    bool ampmCard = false;
    bool cardDate = false, cardWeekday = false;
    Anim anim = Anim::Classic;
    int intro = 0;  // 0 roll, 1 flip, 2 none
    double speed = 1;
    Sound sound = Sound::Mechanical;
    double volume = 0.4;
    double latency = 0;  // ms the visuals wait so the sound lines up
    bool soundEveryFlip = false;
    int thump = 1;  // 0 off, 1 light, 2 firm
    bool cards = true;
    int background = 0;  // 0 black, 1 dim, 2 wallpaper
    int layout = 0;      // 0 auto, 1 side, 2 stacked
    double scale = 0.96, digitGap = 0.03, groupGap = 0.14, dim = 0;
    bool date = false;
    bool hideStockClock = true, hideWidgets = true;
};

static ModSettings g_settings;

constexpr double kPi = 3.14159265358979323846;

static double Clamp(double v, double lo, double hi) {
    return v < lo ? lo : v > hi ? hi : v;
}

static bool SystemUses24h() {
    WCHAR fmt[80];
    if (!GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_STIMEFORMAT, fmt, ARRAYSIZE(fmt))) {
        return false;
    }
    return wcschr(fmt, L'H') != nullptr;
}

static void LoadSettings() {
    auto str = [](PCWSTR name) {
        return std::wstring(WindhawkUtils::StringSetting::make(name).get());
    };
    auto pick = [](const std::wstring& v, std::initializer_list<const wchar_t*> opts) {
        int i = 0;
        for (auto o : opts) {
            if (v == o) return i;
            i++;
        }
        return 0;
    };
    ModSettings s;
    s.theme = pick(str(L"theme"), {L"dark", L"light", L"auto"});
    s.darkFrom = (int)Clamp(Wh_GetIntSetting(L"darkFrom"), 0, 23);
    s.darkTo = (int)Clamp(Wh_GetIntSetting(L"darkTo"), 0, 23);
    auto tf = str(L"timeFormat");
    s.h24 = tf == L"24h" || (tf != L"12h" && SystemUses24h());
    s.style = pick(str(L"cardStyle"), {L"digits", L"pairs"});
    s.secondsCard = pick(str(L"secondsCard"), {L"off", L"small", L"large"});
    s.ampmCard = str(L"ampm") == L"card";
    s.cardDate = Wh_GetIntSetting(L"cardDate") != 0;
    s.cardWeekday = Wh_GetIntSetting(L"cardWeekday") != 0;
    s.anim = (Anim)pick(str(L"animation"), {L"classic", L"gravity", L"snappy", L"bounce", L"fold",
                                             L"cascade", L"roll", L"fade"});
    s.intro = pick(str(L"intro"), {L"roll", L"flip", L"none"});
    s.speed = Clamp(Wh_GetIntSetting(L"speed"), 30, 300) / 100.0;
    s.sound = (Sound)pick(str(L"sound"),
                          {L"off", L"solari", L"mechanical", L"click", L"whoosh", L"soft"});
    s.volume = Clamp(Wh_GetIntSetting(L"volume"), 5, 100) / 100.0;
    s.latency = Clamp(Wh_GetIntSetting(L"latency"), 0, 500);
    s.soundEveryFlip = str(L"soundEvery") == L"all";
    s.thump = pick(str(L"thump"), {L"off", L"light", L"firm"});
    s.cards = Wh_GetIntSetting(L"cards") != 0;
    s.background = pick(str(L"background"), {L"black", L"dim", L"wallpaper"});
    s.layout = pick(str(L"layout"), {L"auto", L"side", L"stacked"});
    s.scale = Clamp(Wh_GetIntSetting(L"size"), 30, 100) / 100.0;
    s.digitGap = Clamp(Wh_GetIntSetting(L"digitGap"), 0, 20) / 100.0;
    s.groupGap = Clamp(Wh_GetIntSetting(L"groupGap"), 0, 60) / 100.0;
    s.dim = Clamp(Wh_GetIntSetting(L"dim"), 0, 85) / 100.0;
    s.date = Wh_GetIntSetting(L"date") != 0;
    s.hideStockClock = Wh_GetIntSetting(L"hideStockClock") != 0;
    s.hideWidgets = Wh_GetIntSetting(L"hideWidgets") != 0;
    g_settings = s;
}

// Test seams: the preview harness in tests/ drives the clock outside LockApp
// with a fake clock and captures the audio instead of playing it.
static SYSTEMTIME LocalNow() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    return st;
}
static double SteadyMs() {
    static LARGE_INTEGER freq = [] {
        LARGE_INTEGER f;
        QueryPerformanceFrequency(&f);
        return f;
    }();
    LARGE_INTEGER c;
    QueryPerformanceCounter(&c);
    return c.QuadPart * 1000.0 / freq.QuadPart;
}
static SYSTEMTIME (*g_now)() = LocalNow;
static double (*g_ms)() = SteadyMs;
static void (*g_audioSink)(std::vector<float>&& pcm) = nullptr;
static void (*g_afterUiTick)() = nullptr;  // the lock screen test build's snapshots

static std::mt19937 g_rng{std::random_device{}()};
static double Rand() {
    return std::uniform_real_distribution<double>(0, 1)(g_rng);
}

// ---------------------------------------------------------------------------
// Sound: a port of LyricDock's Web Audio patch, rendered offline
// ---------------------------------------------------------------------------
//
// One flip (or one roll-in) is rendered into one buffer: every card's sounds at
// their own times, through one limiter, the way a whole board landing at once
// went through one DynamicsCompressor in the original.

constexpr int kRate = 48000;

struct Mix {
    std::vector<float> s;
    void Reserve(double endSec) {
        size_t n = (size_t)(endSec * kRate) + 1;
        if (s.size() < n) s.resize(n, 0.0f);
    }
};

static const std::vector<float>& Noise() {
    static std::vector<float> n = [] {
        std::vector<float> v(kRate);  // 1 s: long enough for any burst from any offset
        std::mt19937 r(7);
        std::uniform_real_distribution<float> d(-1, 1);
        for (auto& x : v) x = d(r);
        return v;
    }();
    return n;
}

// Web Audio's exponentialRampToValueAtTime, from (t0, v0) to (t1, v1).
static double ExpRamp(double v0, double v1, double t0, double t1, double t) {
    if (t <= t0) return v0;
    if (t >= t1) return v1;
    return v0 * std::pow(v1 / v0, (t - t0) / (t1 - t0));
}

enum class Filter { Lowpass, Highpass, Bandpass };

// A noise burst through a biquad whose frequency glides f0 -> f1, with an
// exponential attack and decay. Coefficients follow the Web Audio spec, where Q
// is in dB for lowpass / highpass and linear for bandpass.
static void Burst(Mix& m, Filter type, double f0, double f1, double q, double vol, double attack,
                  double len, double at) {
    if (vol <= 0) return;
    const auto& noise = Noise();
    size_t start = (size_t)(std::max(0.0, at) * kRate);
    size_t n = (size_t)((len + 0.02) * kRate);
    m.Reserve(at + len + 0.03);
    size_t off = (size_t)(Rand() * 0.5 * kRate);
    double x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    for (size_t i = 0; i < n; i++) {
        double t = (double)i / kRate;
        double f = ExpRamp(f0, f1, 0, len, t);
        double w0 = 2 * kPi * std::min(f, kRate * 0.49) / kRate;
        double c = std::cos(w0), sn = std::sin(w0);
        double b0, b1, b2, a0, a1, a2;
        if (type == Filter::Bandpass) {
            double alpha = sn / (2 * q);
            b0 = alpha, b1 = 0, b2 = -alpha;
            a0 = 1 + alpha, a1 = -2 * c, a2 = 1 - alpha;
        } else {
            double alpha = sn / (2 * std::pow(10, q / 20));
            double k = type == Filter::Lowpass ? 1 - c : 1 + c;
            b0 = k / 2, b1 = type == Filter::Lowpass ? k : -k, b2 = k / 2;
            a0 = 1 + alpha, a1 = -2 * c, a2 = 1 - alpha;
        }
        double x = noise[(off + i) % noise.size()];
        double y = (b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2) / a0;
        x2 = x1, x1 = x, y2 = y1, y1 = y;
        double g = t < attack ? ExpRamp(1e-4, vol, 0, attack, t) : ExpRamp(vol, 1e-4, attack, len, t);
        m.s[start + i] += (float)(y * g);
    }
}

// A short sine knock that drops in pitch.
static void Knock(Mix& m, double vol, double at, double f = 140) {
    size_t start = (size_t)(std::max(0.0, at) * kRate);
    size_t n = (size_t)(0.07 * kRate);
    m.Reserve(at + 0.08);
    double phase = 0;
    for (size_t i = 0; i < n; i++) {
        double t = (double)i / kRate;
        phase += 2 * kPi * ExpRamp(f, f * 0.45, 0, 0.05, t) / kRate;
        m.s[start + i] += (float)(std::sin(phase) * ExpRamp(vol, 1e-4, 0, 0.06, t));
    }
}

// The phone's vibration tick, as sound. A vibration motor is felt more than
// heard; through speakers the nearest thing is a short low knock: a sine that
// starts near 115 Hz and sinks towards 55 Hz like a motor spinning down, with a
// faint 165 Hz buzz riding on it, a fast attack and a quick tail, then softly
// saturated so its harmonics carry on laptop speakers that cannot play 60 Hz.
// ms: how long the motor runs (LyricDock buzzes 30 ms light, 50 ms firm).
static void VibrationThump(Mix& m, double at, double ms, double vol) {
    const double dur = ms / 1000, tail = 0.045;
    const double fHi = ms > 40 ? 105 : 118, fLo = ms > 40 ? 50 : 58;
    size_t start = (size_t)(std::max(0.0, at) * kRate);
    size_t n = (size_t)((dur + tail) * kRate);
    m.Reserve(at + dur + tail + 0.01);
    const double drive = 1.8, norm = std::tanh(drive);
    double ph = 0, phBuzz = 0;
    for (size_t i = 0; i < n; i++) {
        double t = (double)i / kRate;
        double env = t < 0.0025 ? 0.5 - 0.5 * std::cos(kPi * t / 0.0025)
                     : t < dur  ? 1
                                : std::exp(-(t - dur) / 0.012);
        double f = fLo + (fHi - fLo) * std::exp(-t / (dur * 0.45));
        ph += 2 * kPi * f / kRate;
        phBuzz += 2 * kPi * 165 / kRate;
        double x = std::sin(ph) + 0.22 * std::sin(phBuzz) * (t < dur ? 1 : 0.3);
        m.s[start + i] += (float)(vol * std::tanh(drive * env * x) / norm);
    }
}

// Web Audio's DynamicsCompressor as LyricDock set it up (threshold -14 dB, knee
// 8, ratio 12, attack 1 ms, release 80 ms), with its automatic makeup gain and
// its 6 ms look-ahead (without it a clack's first millisecond clips), then the
// 0.9 master level.
static void Limit(std::vector<float>& s) {
    const double T = -14, W = 8, R = 12;
    auto curve = [&](double x) {  // output level in dB for an input level in dB
        if (2 * (x - T) < -W) return x;
        if (2 * std::abs(x - T) <= W) return x + (1 / R - 1) * std::pow(x - T + W / 2, 2) / (2 * W);
        return T + (x - T) / R;
    };
    const double makeup = std::pow(1 / std::pow(10, curve(0) / 20), 0.6) * 0.9;
    const double att = std::exp(-1 / (0.001 * kRate)), rel = std::exp(-1 / (0.08 * kRate));
    const size_t ahead = (size_t)(0.006 * kRate);
    double env = 0;
    for (size_t i = 0; i < s.size(); i++) {
        // The detector hears the signal 6 ms early, so the gain is already down
        // when a transient arrives; and the level is never taken below the
        // sample itself, so the output cannot clip.
        double a = i + ahead < s.size() ? std::abs(s[i + ahead]) : 0;
        env = a > env ? att * env + (1 - att) * a : rel * env + (1 - rel) * a;
        double db = 20 * std::log10(std::max({env, (double)std::abs(s[i]), 1e-6}));
        double g = std::pow(10, (curve(db) - db) / 20) * makeup;
        s[i] = (float)Clamp(s[i] * g, -1, 1);
    }
}

static double Jit() {
    return 0.85 + Rand() * 0.3;  // no two flaps sound exactly alike
}

// What is scheduled into the buffer being built: real boards rattle, so many
// flaps landing together sound like a quick run of clicks rather than one loud
// blast. Clacks closer than 9 ms to one already scheduled are dropped, and each
// gets quieter the busier its 60 ms window is.
struct SoundJob {
    Mix mix;
    std::vector<double> landed;
    double Budget(double at) {
        for (double x : landed) {
            if (std::abs(x - at) < 0.009) return 0;
        }
        int busy = 0;
        for (double x : landed) busy += std::abs(x - at) < 0.06;
        landed.push_back(at);
        return 1 / std::sqrt(1.0 + busy);
    }
    // A flap hitting the stack: a sharp plastic tick, a short hollow body and a
    // dull knock.
    void Clack(double at, double v) {
        v *= Budget(at);
        if (!v) return;
        Burst(mix, Filter::Highpass, 4200 * Jit(), 4200 * Jit(), 1, 0.9 * v, 0.0008, 0.012, at);
        double body = 1700 * Jit();
        Burst(mix, Filter::Bandpass, body, body, 5, 0.6 * v, 0.001, 0.04, at + 0.002);
        Knock(mix, 0.3 * v, at, 190 * Jit());
    }
    // The flap sweeping down: a soft rush of air that swells until it lands.
    void Air(double from, double to, double v) {
        Burst(mix, Filter::Bandpass, 500, 1400, 0.7, 0.28 * v, std::max(0.03, (to - from) * 0.8),
              std::max(0.05, to - from) + 0.02, from);
    }
    // One card's sound: fall = when its flap lets go, land = when it hits (s).
    void Card(double fall, double land, double scale) {
        double v = g_settings.volume * scale;
        switch (g_settings.sound) {
            case Sound::Off:
                break;
            case Sound::Solari:  // a flap, and the next one rattling
                Clack(land, v);
                Clack(land + 0.016 + Rand() * 0.01, v * 0.45);
                break;
            case Sound::Mechanical:
                Air(fall, land, v * 0.7);
                Clack(land, v);
                break;
            case Sound::Click:
                Clack(land, v * 0.8);
                break;
            case Sound::Whoosh:
                Air(fall, land, v);
                Burst(mix, Filter::Lowpass, 900, 900, 1, 0.25 * v, 0.002, 0.03, land);
                break;
            case Sound::Soft:
                Burst(mix, Filter::Lowpass, 1200, 1200, 1, 0.5 * v, 0.002, 0.04, land);
                Knock(mix, 0.35 * v, land);
                break;
        }
    }
    // Cards landing within 140 ms of each other get one thump.
    std::vector<double> thumps;
    void Thump(double at) {
        if (!g_settings.thump) return;
        for (double x : thumps) {
            if (std::abs(x - at) < 0.14) return;
        }
        thumps.push_back(at);
        VibrationThump(mix, at, g_settings.thump == 2 ? 50 : 30,
                       Clamp(g_settings.volume * 1.6, 0.2, 1) * (g_settings.thump == 2 ? 1 : 0.8));
    }
};

// XAudio2 playback: one source voice per rendered buffer, reaped once it has
// played. The engine is released while the lock screen is away, so a changed
// default device is picked up the next time.
struct Voice {
    IXAudio2SourceVoice* v;
    std::vector<float> pcm;
};
static IXAudio2* g_xa = nullptr;
static IXAudio2MasteringVoice* g_master = nullptr;
static std::vector<Voice*> g_voices;

static void AudioRelease() {
    for (auto* v : g_voices) {
        v->v->DestroyVoice();
        delete v;
    }
    g_voices.clear();
    if (g_master) g_master->DestroyVoice();
    g_master = nullptr;
    if (g_xa) g_xa->Release();
    g_xa = nullptr;
}

static void AudioReap() {
    for (auto it = g_voices.begin(); it != g_voices.end();) {
        XAUDIO2_VOICE_STATE st;
        (*it)->v->GetState(&st, XAUDIO2_VOICE_NOSAMPLESPLAYED);
        if (st.BuffersQueued == 0) {
            (*it)->v->DestroyVoice();
            delete *it;
            it = g_voices.erase(it);
        } else {
            ++it;
        }
    }
}

static void AudioPlay(std::vector<float>&& pcm) {
    if (g_audioSink) {
        g_audioSink(std::move(pcm));
        return;
    }
    for (int attempt = 0; attempt < 2; attempt++) {
        if (!g_xa) {
            if (FAILED(XAudio2Create(&g_xa, 0, XAUDIO2_DEFAULT_PROCESSOR))) {
                g_xa = nullptr;
                return;
            }
            if (FAILED(g_xa->CreateMasteringVoice(&g_master))) {
                Wh_Log(L"No audio output");
                AudioRelease();
                return;
            }
        }
        WAVEFORMATEX fmt{};
        fmt.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
        fmt.nChannels = 1;
        fmt.nSamplesPerSec = kRate;
        fmt.wBitsPerSample = 32;
        fmt.nBlockAlign = 4;
        fmt.nAvgBytesPerSec = kRate * 4;
        IXAudio2SourceVoice* sv = nullptr;
        if (SUCCEEDED(g_xa->CreateSourceVoice(&sv, &fmt))) {
            auto* v = new Voice{sv, std::move(pcm)};
            XAUDIO2_BUFFER b{};
            b.Flags = XAUDIO2_END_OF_STREAM;
            b.AudioBytes = (UINT32)(v->pcm.size() * sizeof(float));
            b.pAudioData = (const BYTE*)v->pcm.data();
            if (SUCCEEDED(sv->SubmitSourceBuffer(&b)) && SUCCEEDED(sv->Start())) {
                g_voices.push_back(v);
                return;
            }
            sv->DestroyVoice();
            pcm = std::move(v->pcm);
            delete v;
        }
        AudioRelease();  // the device went away: start over once
    }
}

static void AudioFlush(SoundJob& job) {
    if (job.mix.s.empty()) return;
    Limit(job.mix.s);
    AudioPlay(std::move(job.mix.s));
}

// ---------------------------------------------------------------------------
// Card art: LyricDock's "Digits - Puff 3D" design, drawn with Direct2D
// ---------------------------------------------------------------------------
//
// Units are the design's 300 x 440 card. Numerals are stroked paths (from
// LyricDock's scripts/flip-assets.html); every Figma effect becomes the same
// chain of filter primitives the SVG port used.

static const char* const kDigitPath[10] = {
    "M150 92.9C196.5 92.9 215.1 127 215.1 173.5V266.5C215.1 313 196.5 347.1 150 347.1C103.5 347.1 "
    "84.9 313 84.9 266.5V173.5C84.9 127 103.5 92.9 150 92.9Z",
    "M109.7 136.3L165.5 92.9V347.1",
    "M88 158C88 114.6 115.9 92.9 150 92.9C187.2 92.9 212 117.7 212 154.9C212 192.1 193.4 213.8 "
    "159.3 247.9L91.1 323.85H221.3",
    "M88 120.8C101.95 100.65 123.65 92.9 150 92.9C187.2 92.9 212 114.6 212 148.7C212 185.9 187.2 "
    "210.7 140.7 210.7C190.3 210.7 215.1 235.5 215.1 275.8C215.1 319.2 187.2 347.1 146.9 "
    "347.1C120.55 347.1 98.85 337.8 83.35 317.65",
    "M187.2 347.1V92.9L84.9 269.6H230.6",
    "M208.9 116.15H109.7L98.85 210.7C114.35 196.75 131.4 190.55 151.55 190.55C193.4 190.55 215.1 "
    "221.55 215.1 266.5C215.1 316.1 187.2 347.1 146.9 347.1C119 347.1 97.3 334.7 83.35 313",
    "M199.6 114.6C187.2 100.65 170.15 92.9 150 92.9C106.6 92.9 84.9 133.2 84.9 204.5V263.4C84.9 "
    "316.1 109.7 347.1 150 347.1C190.3 347.1 215.1 316.1 215.1 269.6C215.1 226.2 190.3 198.3 150 "
    "198.3C115.9 198.3 91.1 220 84.9 247.9",
    "M78.7 116.15H218.2L134.5 347.1",
    "M150 210.7C115.9 210.7 95.75 189 95.75 151.8C95.75 114.6 119 92.9 150 92.9C181 92.9 204.25 "
    "114.6 204.25 151.8C204.25 189 184.1 210.7 150 210.7ZM150 210.7C109.7 210.7 84.9 235.5 84.9 "
    "278.9C84.9 319.2 112.8 347.1 150 347.1C187.2 347.1 215.1 319.2 215.1 278.9C215.1 235.5 190.3 "
    "210.7 150 210.7Z",
    "M100.4 325.4C112.8 340.9 129.85 347.1 150 347.1C193.4 347.1 215.1 306.8 215.1 235.5V176.6C215.1 "
    "123.9 190.3 92.9 150 92.9C109.7 92.9 84.9 123.9 84.9 170.4C84.9 213.8 109.7 241.7 150 "
    "241.7C184.1 241.7 208.9 220 215.1 192.1",
};

// A, M and P for the AM / PM card, drawn in the same strokes and on the same
// grid (x 78-231, y 92.9-347.1) as the numerals.
static const char* const kLetterA = "M80 347.1L150 92.9L220 347.1M106 262H194";
static const char* const kLetterM = "M86 347.1V92.9L150 250L214 92.9V347.1";
static const char* const kLetterP = "M96 347.1V92.9H154C192 92.9 214 117 214 151.8C214 186.6 192 210.7 154 210.7H96";

static const char* GlyphPath(char c) {
    if (c >= '0' && c <= '9') return kDigitPath[c - '0'];
    return c == 'A' ? kLetterA : c == 'M' ? kLetterM : c == 'P' ? kLetterP : nullptr;
}

// Absolute M / L / H / V / C / Z only, which is all the paths above use.
static void BuildPath(ID2D1GeometrySink* sink, const char* p) {
    auto num = [&]() {
        while (*p == ' ' || *p == ',') p++;
        char* end;
        float v = strtof(p, &end);
        p = end;
        return v;
    };
    D2D1_POINT_2F cur{};
    bool open = false;
    char cmd = 0;
    while (*p) {
        while (*p == ' ') p++;
        if (!*p) break;
        if (isalpha((unsigned char)*p)) cmd = *p++;
        switch (cmd) {
            case 'M':
                if (open) sink->EndFigure(D2D1_FIGURE_END_OPEN);
                cur = {num(), 0};
                cur.y = num();
                sink->BeginFigure(cur, D2D1_FIGURE_BEGIN_FILLED);
                open = true;
                cmd = 'L';  // further pairs are line-tos
                break;
            case 'L':
                cur = {num(), 0};
                cur.y = num();
                sink->AddLine(cur);
                break;
            case 'H':
                cur.x = num();
                sink->AddLine(cur);
                break;
            case 'V':
                cur.y = num();
                sink->AddLine(cur);
                break;
            case 'C': {
                D2D1_BEZIER_SEGMENT b;
                b.point1.x = num(), b.point1.y = num();
                b.point2.x = num(), b.point2.y = num();
                b.point3.x = num(), b.point3.y = num();
                sink->AddBezier(b);
                cur = b.point3;
                break;
            }
            case 'Z':
                sink->EndFigure(D2D1_FIGURE_END_CLOSED);
                open = false;
                cmd = 0;
                break;
            default:
                return;
        }
    }
    if (open) sink->EndFigure(D2D1_FIGURE_END_OPEN);
}

enum class FxType { Drop, Inner, Grain };
struct Fx {
    FxType t;
    bool white;  // colour: white or black
    float x, y, r, a;
    float density = 0, size = 0;
};
static Fx Ds(float x, float y, float r, float a) {
    return {FxType::Drop, false, x, y, r, a};
}
static Fx Is(bool white, float x, float y, float r, float a) {
    return {FxType::Inner, white, x, y, r, a};
}
static Fx Grain(bool white, float a, float density, float size) {
    return {FxType::Grain, white, 0, 0, 0, a, density, size};
}

struct CardTheme {
    std::vector<Fx> puffTop, puffBot, cardTop, cardBot;
    uint32_t card[2][2], ink[2][2];
    float cast, fall, hair, hinge;
};

// Dark: off-white ink on black paper. Light: black ink on off-white paper,
// softer shadows (they fall on a light surface) and a faint sheen on the raised
// ink's top edge instead of a bright rim.
static CardTheme MakeTheme(bool light) {
    auto puff = [&](float l) {
        if (light) {
            return std::vector<Fx>{Ds(0, 1.5f, 1.5f, 0.45f),     Ds(0, 4 * l, 7 * l, 0.28f),
                                   Ds(0, 12 * l, 24 * l, 0.2f),  Is(true, 0, 2.5f, 2, 0.32f),
                                   Is(true, 0, 7, 10, 0.1f),     Is(false, 0, -4, 7, 0.55f),
                                   Is(false, 0, 0, 22, 0.3f),    Grain(true, 0.09f, 0.45f, 0.6f),
                                   Grain(false, 0.2f, 0.35f, 0.6f), Grain(true, 0.04f, 0.3f, 2)};
        }
        return std::vector<Fx>{Ds(0, 1.5f, 1.5f, 0.85f),      Ds(0, 5 * l, 8 * l, 0.55f),
                               Ds(0, 14 * l, 26 * l, 0.45f),  Is(true, 0, 3, 2, 0.9f),
                               Is(true, 0, 7, 10, 0.35f),     Is(false, 0, -5, 8, 0.5f),
                               Is(false, 0, 0, 24, 0.34f),    Grain(false, 0.13f, 0.5f, 0.6f),
                               Grain(true, 0.1f, 0.35f, 0.6f), Grain(false, 0.06f, 0.3f, 2)};
    };
    CardTheme t;
    t.puffTop = puff(1);
    t.puffBot = puff(0.8f);
    std::vector<Fx> paper, top, bot;
    if (light) {
        paper = {Grain(false, 0.09f, 0.5f, 0.8f), Grain(false, 0.04f, 0.3f, 2.5f),
                 Grain(true, 0.35f, 0.5f, 0.8f)};
        top = {Is(true, 0, 1.5f, 0, 0.8f), Is(false, 0, -6, 10, 0.08f)};
        bot = {Is(true, 0, 1, 0, 0.5f), Is(false, 0, -3, 5, 0.22f)};
        uint32_t card[2][2] = {{0xF1EEE7, 0xE3DFD7}, {0xD9D5CC, 0xCBC6BC}};
        uint32_t ink[2][2] = {{0x2B2B2A, 0x1D1D1C}, {0x1A1A19, 0x101010}};
        memcpy(t.card, card, sizeof card);
        memcpy(t.ink, ink, sizeof ink);
        t.cast = 0.28f, t.fall = 0.12f, t.hair = 0.7f, t.hinge = 0.35f;
    } else {
        paper = {Grain(true, 0.055f, 0.45f, 0.8f), Grain(true, 0.025f, 0.3f, 2.5f),
                 Grain(false, 0.5f, 0.55f, 0.8f)};
        top = {Is(true, 0, 1.5f, 0, 0.09f), Is(false, 0, -6, 10, 0.35f)};
        bot = {Is(true, 0, 1, 0, 0.05f), Is(false, 0, -3, 5, 0.7f)};
        uint32_t card[2][2] = {{0x1D1D1D, 0x131313}, {0x131313, 0x080808}};
        uint32_t ink[2][2] = {{0xF6F4EE, 0xCCC9C4}, {0xBDBAB5, 0x8F8D8A}};
        memcpy(t.card, card, sizeof card);
        memcpy(t.ink, ink, sizeof ink);
        t.cast = 0.55f, t.fall = 0.3f, t.hair = 0.08f, t.hinge = 0.045f;
    }
    t.cardTop = top;
    t.cardTop.insert(t.cardTop.end(), paper.begin(), paper.end());
    t.cardBot = bot;
    t.cardBot.insert(t.cardBot.end(), paper.begin(), paper.end());
    return t;
}

static D2D1_COLOR_F Rgb(uint32_t c, float a = 1) {
    return {((c >> 16) & 255) / 255.f, ((c >> 8) & 255) / 255.f, (c & 255) / 255.f, a};
}

// Effect CLSIDs, local so the mod does not depend on an import library for them.
static const GUID kFxAffine = {0x6aa97485, 0x6354, 0x4cfc, {0x90, 0x8c, 0xe4, 0xa7, 0x4f, 0x62, 0xc9, 0x6c}};
static const GUID kFxColorMatrix = {0x921f03d6, 0x641c, 0x47df, {0x85, 0x2d, 0xb4, 0xbb, 0x61, 0x53, 0xae, 0x11}};
static const GUID kFxComposite = {0x48fc9f51, 0xf6ac, 0x48f1, {0x8b, 0x58, 0x3b, 0x28, 0xac, 0x46, 0xf7, 0x6d}};
static const GUID kFxLinearTransfer = {0xad47c8fd, 0x63ef, 0x4acc, {0x9b, 0x51, 0x67, 0x97, 0x9c, 0x03, 0x6c, 0x06}};
static const GUID kFxShadow = {0xc67ea361, 0x1863, 0x4e69, {0x89, 0xdb, 0x69, 0x5d, 0x3e, 0x9a, 0x5b, 0x6b}};
static const GUID kFxTurbulence = {0xcf2bb6ae, 0x889a, 0x4ad7, {0xba, 0x29, 0xa2, 0xfd, 0x73, 0x2c, 0x9f, 0xc9}};

struct Painter {
    winrt::com_ptr<ID2D1Factory1> factory;
    winrt::com_ptr<ID2D1DeviceContext> dc;

    bool Init() {
        winrt::com_ptr<ID3D11Device> d3d;
        for (auto type : {D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP}) {
            if (SUCCEEDED(D3D11CreateDevice(nullptr, type, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                                            nullptr, 0, D3D11_SDK_VERSION, d3d.put(), nullptr,
                                            nullptr))) {
                break;
            }
        }
        if (!d3d) return false;
        D2D1_FACTORY_OPTIONS opts{};
        if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1),
                                     &opts, factory.put_void()))) {
            return false;
        }
        winrt::com_ptr<ID2D1Device> dev;
        if (FAILED(factory->CreateDevice(d3d.as<IDXGIDevice>().get(), dev.put()))) return false;
        return SUCCEEDED(dev->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, dc.put()));
    }

    winrt::com_ptr<ID2D1Effect> Effect(const GUID& id) {
        winrt::com_ptr<ID2D1Effect> e;
        winrt::check_hresult(dc->CreateEffect(id, e.put()));
        return e;
    }
    winrt::com_ptr<ID2D1Bitmap1> Target(UINT w, UINT h, bool cpu = false) {
        D2D1_BITMAP_PROPERTIES1 p{};
        p.pixelFormat = {DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED};
        p.dpiX = p.dpiY = 96;
        p.bitmapOptions = cpu ? D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW
                              : D2D1_BITMAP_OPTIONS_TARGET;
        winrt::com_ptr<ID2D1Bitmap1> b;
        winrt::check_hresult(dc->CreateBitmap({w, h}, nullptr, 0, p, b.put()));
        return b;
    }
    static void SetInput(ID2D1Effect* e, UINT i, ID2D1Image* img) { e->SetInput(i, img); }
    winrt::com_ptr<ID2D1Image> Out(ID2D1Effect* e) {
        winrt::com_ptr<ID2D1Image> o;
        e->GetOutput(o.put());
        return o;
    }
    winrt::com_ptr<ID2D1Image> Offset(ID2D1Image* in, float x, float y) {
        auto e = Effect(kFxAffine);
        e->SetInput(0, in);
        auto m = D2D1::Matrix3x2F::Translation(x, y);
        e->SetValue(D2D1_2DAFFINETRANSFORM_PROP_TRANSFORM_MATRIX, m);
        return Out(e.get());
    }
    winrt::com_ptr<ID2D1Image> Shadow(ID2D1Image* in, float sigma, D2D1_COLOR_F c) {
        auto e = Effect(kFxShadow);
        e->SetInput(0, in);
        e->SetValue(D2D1_SHADOW_PROP_BLUR_STANDARD_DEVIATION, std::max(sigma, 0.0f));
        e->SetValue(D2D1_SHADOW_PROP_COLOR, D2D1_VECTOR_4F{c.r, c.g, c.b, c.a});
        return Out(e.get());
    }
    winrt::com_ptr<ID2D1Image> Matrix(ID2D1Image* in, const D2D1_MATRIX_5X4_F& m, bool straight) {
        auto e = Effect(kFxColorMatrix);
        e->SetInput(0, in);
        e->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, m);
        e->SetValue(D2D1_COLORMATRIX_PROP_ALPHA_MODE,
                    straight ? D2D1_COLORMATRIX_ALPHA_MODE_STRAIGHT
                             : D2D1_COLORMATRIX_ALPHA_MODE_PREMULTIPLIED);
        e->SetValue(D2D1_COLORMATRIX_PROP_CLAMP_OUTPUT, TRUE);
        return Out(e.get());
    }
    // source * alpha(mask)
    winrt::com_ptr<ID2D1Image> In(ID2D1Image* source, ID2D1Image* mask) {
        auto e = Effect(kFxComposite);
        e->SetInput(0, mask);
        e->SetInput(1, source);
        e->SetValue(D2D1_COMPOSITE_PROP_MODE, D2D1_COMPOSITE_MODE_SOURCE_IN);
        return Out(e.get());
    }

    // src: the shape drawn on a bitmap with a 40-unit margin all round (the SVG
    // filter region), k pixels per unit. Returns the filtered image, in the
    // same space.
    winrt::com_ptr<ID2D1Image> Filter(ID2D1Bitmap1* src, const std::vector<Fx>& fx, float k, int seed) {
        std::vector<winrt::com_ptr<ID2D1Image>> under, over;
        auto size = src->GetPixelSize();
        int n = 0;
        for (const auto& f : fx) {
            n++;
            D2D1_COLOR_F c = f.white ? D2D1_COLOR_F{1, 1, 1, f.a} : D2D1_COLOR_F{0, 0, 0, f.a};
            if (f.t == FxType::Drop) {
                auto s = Shadow(src, f.r / 2 * k, c);
                under.push_back(Offset(s.get(), f.x * k, f.y * k));
            } else if (f.t == FxType::Inner) {
                D2D1_MATRIX_5X4_F inv{};  // alpha -> 1 - alpha, colour 0
                inv._44 = -1;
                inv._54 = 1;
                auto i = Matrix(src, inv, true);
                auto o = Offset(i.get(), f.x * k, f.y * k);
                auto s = Shadow(o.get(), std::max(f.r / 2, 0.01f) * k, c);
                over.push_back(In(s.get(), src));
            } else {
                // A single-colour speckle covering about `density` of the shape.
                float t = 0.5f + (0.5f - f.density) * 0.28f;
                auto turb = Effect(kFxTurbulence);
                turb->SetValue(D2D1_TURBULENCE_PROP_OFFSET, D2D1_VECTOR_2F{0, 0});
                turb->SetValue(D2D1_TURBULENCE_PROP_SIZE, D2D1_VECTOR_2F{(float)size.width, (float)size.height});
                float bf = 0.95f / f.size / k;
                turb->SetValue(D2D1_TURBULENCE_PROP_BASE_FREQUENCY, D2D1_VECTOR_2F{bf, bf});
                turb->SetValue(D2D1_TURBULENCE_PROP_NUM_OCTAVES, (UINT)2);
                turb->SetValue(D2D1_TURBULENCE_PROP_SEED, (UINT)(seed * 7 + n));
                turb->SetValue(D2D1_TURBULENCE_PROP_NOISE, D2D1_TURBULENCE_NOISE_FRACTAL_SUM);
                turb->SetValue(D2D1_TURBULENCE_PROP_STITCHABLE, TRUE);
                D2D1_MATRIX_5X4_F red{};  // alpha <- red
                red._14 = 1;
                auto a = Matrix(Out(turb.get()).get(), red, false);
                auto lt = Effect(kFxLinearTransfer);
                lt->SetInput(0, a.get());
                lt->SetValue(D2D1_LINEARTRANSFER_PROP_RED_DISABLE, TRUE);
                lt->SetValue(D2D1_LINEARTRANSFER_PROP_GREEN_DISABLE, TRUE);
                lt->SetValue(D2D1_LINEARTRANSFER_PROP_BLUE_DISABLE, TRUE);
                lt->SetValue(D2D1_LINEARTRANSFER_PROP_ALPHA_SLOPE, 14.0f);
                lt->SetValue(D2D1_LINEARTRANSFER_PROP_ALPHA_Y_INTERCEPT, -14 * t);
                lt->SetValue(D2D1_LINEARTRANSFER_PROP_CLAMP_OUTPUT, TRUE);
                auto masked = In(Out(lt.get()).get(), src);
                D2D1_MATRIX_5X4_F tint{};  // colour c, alpha * a
                tint._51 = tint._52 = tint._53 = f.white ? 1.0f : 0.0f;
                tint._44 = f.a;
                over.push_back(Matrix(masked.get(), tint, false));
            }
        }
        auto merge = Effect(kFxComposite);
        std::vector<ID2D1Image*> inputs;
        for (auto it = under.rbegin(); it != under.rend(); ++it) inputs.push_back(it->get());
        inputs.push_back(src);
        for (auto& o : over) inputs.push_back(o.get());
        merge->SetInputCount((UINT32)inputs.size());
        for (UINT i = 0; i < inputs.size(); i++) merge->SetInput(i, inputs[i]);
        merge->SetValue(D2D1_COMPOSITE_PROP_MODE, D2D1_COMPOSITE_MODE_SOURCE_OVER);
        return Out(merge.get());
    }

    winrt::com_ptr<ID2D1LinearGradientBrush> Gradient(float y0, float y1, D2D1_COLOR_F a, D2D1_COLOR_F b) {
        D2D1_GRADIENT_STOP stops[2] = {{0, a}, {1, b}};
        winrt::com_ptr<ID2D1GradientStopCollection> coll;
        winrt::check_hresult(dc->CreateGradientStopCollection(stops, 2, coll.put()));
        winrt::com_ptr<ID2D1LinearGradientBrush> br;
        winrt::check_hresult(dc->CreateLinearGradientBrush({{0, y0}, {0, y1}}, coll.get(), br.put()));
        return br;
    }

    // value: "" for a blank card, otherwise one or two characters ("4", "04",
    // "PM"). wu: the card's width in design units (300 for one character, 480
    // for two). Returns premultiplied BGRA, w x h.
    std::vector<uint8_t> Card(const std::string& value, bool light, bool withCard, float wu, UINT w, UINT& h) {
        const float k = w / wu;
        h = (UINT)std::lround(440 * k);
        const UINT mw = (UINT)std::ceil((wu + 80) * k), mh = (UINT)std::ceil(520 * k);
        const auto th = MakeTheme(light);
        int seed = 11;
        for (char c : value) seed = seed * 31 + c;
        seed &= 0xffff;
        const auto M = [&](float x, float y) { return D2D1::Matrix3x2F::Scale(k, k) * D2D1::Matrix3x2F::Translation(x, y); };
        const float r = 40, c = r * 0.4475f;  // corner radius; its bezier handle

        // Draws one shape on a fresh margin bitmap and returns its filtered image.
        auto layer = [&](auto&& draw, const std::vector<Fx>& fx, int salt) {
            auto bmp = Target(mw, mh);
            dc->SetTarget(bmp.get());
            dc->BeginDraw();
            dc->Clear({0, 0, 0, 0});
            dc->SetTransform(M(40 * k, 40 * k));
            draw();
            winrt::check_hresult(dc->EndDraw());
            return std::make_pair(bmp, Filter(bmp.get(), fx, k, seed * 13 + salt));
        };
        auto halfClip = [&](int half) {
            dc->PushAxisAlignedClip(half ? D2D1_RECT_F{0, 221, wu, 440} : D2D1_RECT_F{0, 0, wu, 219},
                                    D2D1_ANTIALIAS_MODE_ALIASED);
        };

        std::vector<std::pair<winrt::com_ptr<ID2D1Bitmap1>, winrt::com_ptr<ID2D1Image>>> layers;
        if (withCard) {
            char shapes[2][256];
            snprintf(shapes[0], 256, "M0 %.2fC0 %.2f %.2f 0 %.2f 0H%.2fC%.2f 0 %.2f %.2f %.2f %.2fV219H0V%.2fZ", r, c, c, r,
                     wu - r, wu - c, wu, c, wu, r, r);
            snprintf(shapes[1], 256, "M0 221H%.2fV%.2fC%.2f %.2f %.2f 440 %.2f 440H%.2fC%.2f 440 0 %.2f 0 %.2fV221Z", wu,
                     440 - r, wu, 440 - c, wu - c, wu - r, r, c, 440 - c, 440 - r);
            for (int half = 0; half < 2; half++) {
                winrt::com_ptr<ID2D1PathGeometry> g;
                winrt::check_hresult(factory->CreatePathGeometry(g.put()));
                winrt::com_ptr<ID2D1GeometrySink> sink;
                g->Open(sink.put());
                sink->SetFillMode(D2D1_FILL_MODE_WINDING);
                BuildPath(sink.get(), shapes[half]);
                sink->Close();
                auto br = Gradient(half ? 221 : 0, half ? 440 : 219, Rgb(th.card[half][0]), Rgb(th.card[half][1]));
                layers.push_back(layer([&] { dc->FillGeometry(g.get(), br.get()); },
                                       half ? th.cardBot : th.cardTop, half));
            }
        }
        // Each character is one of the stroked numerals from the Figma board (or
        // a letter drawn the same way for AM / PM): one character scaled 1.15x
        // about the card centre, two side by side at 0.92x. The ink gradient lives
        // in the glyph's space, the half clip does not.
        if (!value.empty()) {
            D2D1_STROKE_STYLE_PROPERTIES sp{};
            sp.startCap = sp.endCap = sp.dashCap = D2D1_CAP_STYLE_FLAT;
            sp.lineJoin = D2D1_LINE_JOIN_MITER_OR_BEVEL;
            sp.miterLimit = 2.5f;
            winrt::com_ptr<ID2D1StrokeStyle> stroke;
            winrt::check_hresult(factory->CreateStrokeStyle(sp, nullptr, 0, stroke.put()));
            const size_t n = value.size();
            const float scale = n == 1 ? 1.15f : 0.92f, pitch = 212 * scale;  // two leave room for the corners
            std::vector<std::pair<winrt::com_ptr<ID2D1PathGeometry>, D2D1::Matrix3x2F>> glyphs;
            for (size_t i = 0; i < n; i++) {
                const char* path = GlyphPath(value[i]);
                if (!path) continue;
                winrt::com_ptr<ID2D1PathGeometry> g;
                winrt::check_hresult(factory->CreatePathGeometry(g.put()));
                winrt::com_ptr<ID2D1GeometrySink> sink;
                g->Open(sink.put());
                BuildPath(sink.get(), path);
                sink->Close();
                const float cx = wu / 2 + (i - (n - 1) / 2.0f) * pitch;
                glyphs.push_back({g, D2D1::Matrix3x2F::Translation(-150, -220) * D2D1::Matrix3x2F::Scale(scale, scale) *
                                         D2D1::Matrix3x2F::Translation(cx, 220)});
            }
            for (int half = 0; half < 2; half++) {
                auto br = Gradient(half ? 220 : 85, half ? 355 : 220, Rgb(th.ink[half][0]), Rgb(th.ink[half][1]));
                layers.push_back(layer(
                    [&] {
                        halfClip(half);
                        for (auto& [g, at] : glyphs) {
                            dc->SetTransform(at * M(40 * k, 40 * k));
                            dc->DrawGeometry(g.get(), br.get(), 46.5f, stroke.get());
                        }
                        dc->SetTransform(M(40 * k, 40 * k));
                        dc->PopAxisAlignedClip();
                    },
                    half ? th.puffBot : th.puffTop, 2 + half));
            }
        }

        auto target = Target(w, h);
        dc->SetTarget(target.get());
        dc->BeginDraw();
        dc->Clear({0, 0, 0, 0});
        dc->SetTransform(D2D1::Matrix3x2F::Identity());
        winrt::com_ptr<ID2D1RoundedRectangleGeometry> clip;
        if (withCard) {
            winrt::check_hresult(
                factory->CreateRoundedRectangleGeometry({{0, 0, wu * k, 440 * k}, r * k, r * k}, clip.put()));
            dc->PushLayer(D2D1::LayerParameters1(D2D1::InfiniteRect(), clip.get()), nullptr);
        }
        for (auto& l : layers) {
            dc->DrawImage(l.second.get(), D2D1_POINT_2F{-40 * k, -40 * k});
        }
        dc->SetTransform(M(0, 0));
        winrt::com_ptr<ID2D1SolidColorBrush> solid;
        dc->CreateSolidColorBrush({0, 0, 0, 1}, solid.put());
        if (withCard) {
            // The shadow the top half casts on the bottom one, the darkening
            // above the hinge, a hairline on the top edge and the hinge itself.
            auto cast = Gradient(221, 243, {0, 0, 0, th.cast}, {0, 0, 0, 0});
            dc->FillRectangle({0, 221, wu, 243}, cast.get());
            auto fall = Gradient(205, 219, {0, 0, 0, 0}, {0, 0, 0, th.fall});
            dc->FillRectangle({0, 205, wu, 219}, fall.get());
            solid->SetColor({1, 1, 1, th.hair});
            dc->FillRectangle({r, 0, wu - r, 1}, solid.get());
            solid->SetColor({1, 1, 1, th.hinge});
            dc->FillRectangle({0, 221.5f, wu, 222.5f}, solid.get());
            dc->PopLayer();
        }
        solid->SetColor({0, 0, 0, 1});  // the split between the halves
        dc->FillRectangle({0, 218.5f, wu, 221.5f}, solid.get());
        winrt::check_hresult(dc->EndDraw());

        auto cpu = Target(w, h, true);
        winrt::check_hresult(cpu->CopyFromBitmap(nullptr, target.get(), nullptr));
        D2D1_MAPPED_RECT map;
        winrt::check_hresult(cpu->Map(D2D1_MAP_OPTIONS_READ, &map));
        std::vector<uint8_t> px((size_t)w * h * 4);
        for (UINT y = 0; y < h; y++) memcpy(&px[(size_t)y * w * 4], map.bits + (size_t)y * map.pitch, w * 4);
        cpu->Unmap();
        return px;
    }
};

// ---------------------------------------------------------------------------
// Motion: how a flap moves (ported from LyricDock's flipclock.js)
// ---------------------------------------------------------------------------

struct Ease {
    double x1, y1, x2, y2;
};
static constexpr Ease kLinear{0, 0, 1, 1}, kEaseIn{0.5, 0, 0.85, 0.35}, kEaseOut{0.15, 0.75, 0.35, 1},
    kCssEaseOut{0, 0, 0.58, 1}, kCssEaseInOut{0.42, 0, 0.58, 1};

// CSS cubic-bezier(x1, y1, x2, y2) at progress p.
static double EaseAt(const Ease& e, double p) {
    if (e.x1 == 0 && e.y1 == 0 && e.x2 == 1 && e.y2 == 1) return p;
    auto bez = [](double a, double b, double t) {
        return 3 * a * t * (1 - t) * (1 - t) + 3 * b * t * t * (1 - t) + t * t * t;
    };
    double lo = 0, hi = 1, t = p;
    for (int i = 0; i < 40; i++) {
        (bez(e.x1, e.x2, t) < p ? lo : hi) = t;
        t = (lo + hi) / 2;
    }
    return bez(e.y1, e.y2, t);
}

struct Key {
    double off, rot, op;
};

// Linear between keyframes at progress p (WAAPI: the easing warps p first).
static Key KeyAt(const std::vector<Key>& k, double p) {
    if (p <= k.front().off) return k.front();
    for (size_t i = 1; i < k.size(); i++) {
        if (p <= k[i].off) {
            double u = (p - k[i - 1].off) / std::max(1e-9, k[i].off - k[i - 1].off);
            return {p, k[i - 1].rot + (k[i].rot - k[i - 1].rot) * u, k[i - 1].op + (k[i].op - k[i - 1].op) * u};
        }
    }
    return k.back();
}

struct Motion {
    std::vector<Key> top, bot;
    double fall, dur, hit;  // ms: top flap's fall, bottom flap's swing, when it hits the stack
    Ease topEase, botEase;
};

// Gravity: the path simulated once. Released just past upright, the flap swings
// down about its hinge, pulled harder the further it tips (sin) plus a kick
// from the latch letting go; fastest at the bottom. [time 0..1, angle deg].
static const std::vector<std::pair<double, double>>& GravityPath() {
    static std::vector<std::pair<double, double>> path = [] {
        std::vector<std::pair<double, double>> all;
        double a = 0.18, w = 0, t = 0;
        while (a < kPi) {
            all.push_back({t, a});
            w += (std::sin(a) + 0.9) * 0.001;
            a += w * 0.001;
            t += 0.001;
        }
        all.push_back({t, kPi});
        std::vector<std::pair<double, double>> out;
        for (int i = 0; i <= 32; i++) {  // 32 steps is smooth at 60 fps
            double x = i / 32.0;
            size_t j = std::min(all.size() - 1, (size_t)std::lround(x * (all.size() - 1)));
            out.push_back({x, all[j].second * 180 / kPi});
        }
        return out;
    }();
    return path;
}
static double GravityT90() {
    const auto& p = GravityPath();
    for (size_t i = 1; i < p.size(); i++) {
        if (p[i].second >= 90) {
            auto [t0, a0] = p[i - 1];
            auto [t1, a1] = p[i];
            return t0 + (t1 - t0) * (90 - a0) / (a1 - a0);
        }
    }
    return 0.5;
}

struct Eased {
    double fall, land;
    bool over;
};
static bool EasedFor(Anim a, Eased& e) {
    switch (a) {
        case Anim::Classic: e = {180, 210, false}; return true;
        case Anim::Snappy: e = {110, 150, false}; return true;
        case Anim::Bounce: e = {170, 420, true}; return true;
        case Anim::Fold: e = {300, 340, false}; return true;
        case Anim::Cascade: e = {160, 300, true}; return true;
        default: return false;
    }
}
static double BaseMs(Anim a) {
    Eased e;
    return EasedFor(a, e) ? e.fall + e.land : 230 * 1.5;
}

static Motion MotionFor(Anim anim, double sp) {
    Eased e;
    if (EasedFor(anim, e)) {
        Motion m;
        m.top = {{0, 0, 1}, {0.985, -89, 1}, {1, -90, 0}};
        m.bot = {{0, 90, 0}, {0.1, 70, 0}, {0.14, 64, 1}};
        if (e.over) {
            m.bot.push_back({0.62, -12, 1});
            m.bot.push_back({0.82, 5, 1});
        } else {
            m.bot.push_back({0.82, -3, 1});
        }
        m.bot.push_back({1, 0, 1});
        m.fall = e.fall * sp;
        m.dur = e.land * sp;
        m.hit = m.dur * (e.over ? 0.62 : 0.82);
        m.topEase = kEaseIn;
        m.botEase = e.over ? kCssEaseOut : kEaseOut;
        return m;
    }
    // Gravity, a little uneven like real flaps, bouncing off the stack twice.
    const auto& path = GravityPath();
    const double T90 = GravityT90(), j = 0.93 + Rand() * 0.14;
    const double total = 230 * sp * j, fall = total * T90, drop = total - fall;
    Motion m;
    for (auto [t, a] : path) {
        if (t / T90 < 0.97) m.top.push_back({t / T90, -a, 1});
    }
    m.top.push_back({0.985, -89, 1});
    m.top.push_back({1, -90, 0});  // hidden edge-on
    // Bounces: constant deceleration g (deg/ms^2), sized so a 7 deg bounce takes
    // ~70 ms at speed 1.
    const double hops[2] = {5, 1.5};
    const double g = (2 * 7.0) / (35 * 35) / (sp * sp);
    double hopMs[2], dur = drop;
    for (int i = 0; i < 2; i++) dur += hopMs[i] = 2 * std::sqrt(2 * hops[i] * j / g);
    m.bot.push_back({0, 90, 0});
    for (auto [t, a] : path) {
        if (t <= T90) continue;
        double deg = 180 - a;
        m.bot.push_back({(t - T90) * total / dur, deg, deg > 72 ? 0.0 : 1.0});
    }
    double at = drop;
    for (int i = 0; i < 2; i++) {
        for (int s = 1; s <= 6; s++) {
            double u = s / 6.0;
            m.bot.push_back({(at + hopMs[i] * u) / dur, hops[i] * j * 4 * u * (1 - u), 1});
        }
        at += hopMs[i];
    }
    m.bot.back().off = 1;
    m.fall = fall, m.dur = dur, m.hit = drop;
    m.topEase = m.botEase = kLinear;
    return m;
}

// ---------------------------------------------------------------------------
// Keyframe tracks: a whole flip, or a whole roll-in, as one Storyboard
// ---------------------------------------------------------------------------

struct Kf {
    double t, v;
    bool jump;
};
struct Track {
    std::vector<Kf> k;
    void Cut(double t) {
        while (!k.empty() && k.back().t > t) k.pop_back();
    }
    void Set(double t, double v) {
        Cut(t);
        k.push_back({t, v, true});
    }
    void To(double t, double v) { k.push_back({t, v, false}); }
};
struct ObjTrack {
    std::vector<std::pair<double, wf::IInspectable>> k;
    void Set(double t, wf::IInspectable v) {
        while (!k.empty() && k.back().first > t) k.pop_back();
        k.push_back({t, v});
    }
};

// Samples f(eased progress) every ~8 ms over [t0, t0 + dur] onto linear keys.
template <typename F>
static void Sample(double t0, double dur, Ease e, F&& f) {
    int n = std::max(2, (int)std::ceil(dur / 8));
    for (int i = 1; i <= n; i++) f(t0 + dur * i / n, EaseAt(e, (double)i / n));
}

static wuxma::KeyTime At(double ms) {
    return wuxma::KeyTimeHelper::FromTimeSpan(
        std::chrono::duration_cast<wf::TimeSpan>(std::chrono::duration<double, std::milli>(std::max(0.0, ms))));
}

static void AddTrack(wuxma::Storyboard& sb, const Track& tr, const wux::DependencyObject& target,
                     const wchar_t* prop) {
    if (tr.k.empty()) return;
    wuxma::DoubleAnimationUsingKeyFrames a;
    for (const auto& k : tr.k) {
        if (k.jump) {
            wuxma::DiscreteDoubleKeyFrame f;
            f.KeyTime(At(k.t));
            f.Value(k.v);
            a.KeyFrames().Append(f);
        } else {
            wuxma::LinearDoubleKeyFrame f;
            f.KeyTime(At(k.t));
            f.Value(k.v);
            a.KeyFrames().Append(f);
        }
    }
    wuxma::Storyboard::SetTarget(a, target);
    wuxma::Storyboard::SetTargetProperty(a, prop);
    sb.Children().Append(a);
}

static void AddTrack(wuxma::Storyboard& sb, const ObjTrack& tr, const wux::DependencyObject& target,
                     const wchar_t* prop) {
    if (tr.k.empty()) return;
    wuxma::ObjectAnimationUsingKeyFrames a;
    for (const auto& [t, v] : tr.k) {
        wuxma::DiscreteObjectKeyFrame f;
        f.KeyTime(At(t));
        f.Value(v);
        a.KeyFrames().Append(f);
    }
    wuxma::Storyboard::SetTarget(a, target);
    wuxma::Storyboard::SetTargetProperty(a, prop);
    sb.Children().Append(a);
}

// ---------------------------------------------------------------------------
// The clock
// ---------------------------------------------------------------------------
//
// A card is four layers, each the WHOLE card clipped to one half at the centre
// line, so they line up to the pixel: static top (new digit), static bottom (old
// digit), top flap (old) and bottom flap (new). A flip swaps images and moves
// the two flaps about the centre line: the top flap tips over and is hidden once
// edge-on, and the bottom flap - visible only past ~70 deg, in shadow, so no
// bright sliver pops out at the split - comes down and lands.
//
// Two card styles: Digits, a card per digit (LyricDock's), and Pairs, one card
// for the hours and one for the minutes with the same numerals two to a card. Either way the seconds and AM / PM can
// be small flip cards sitting on the minutes and hours cards' bottom corners,
// and the date and weekday printed in their top corners.

constexpr float kCardH = 440, kDigitW = 300, kPairW = 480;  // design units
constexpr double kMini = 0.22;                               // a small card to a full one
constexpr double kMiniMargin = 22;                           // from the host card's edges, units

// What a card prints in its corners: part of what a flip changes.
struct Corners {
    std::wstring tl, tc, bl;  // top left, top centre, bottom left
    bool operator==(const Corners& o) const { return tl == o.tl && tc == o.tc && bl == o.bl; }
};

struct Layer {
    wuxc::Grid el{nullptr};       // clipped to one half
    wuxc::Grid content{nullptr};  // image + corners; rolls inside the clip (statics)
    wuxc::Image img{nullptr};
    wuxc::TextBlock tl{nullptr}, tc{nullptr}, bl{nullptr};
    wuxm::CompositeTransform roll{nullptr};    // statics: the Roll animation
    wuxm3::CompositeTransform3D turn{nullptr};  // flaps: the flip
    wuxs::Rectangle shade{nullptr};             // flaps: darkening; bottom static: the drop shadow
    wuxm::SolidColorBrush shadeBrush{nullptr};
    wuxm::GradientStop dropA{nullptr}, dropB{nullptr};
    wuxs::Rectangle sheen{nullptr};
    wuxm::TranslateTransform sheenY{nullptr};
};

struct Cell {
    wuxc::Grid host{nullptr};  // the card, plus any small cards sitting on it
    wuxc::Grid el{nullptr};    // the card itself, in perspective
    Layer top, bot, ft, fb;    // static top, static bottom, top flap, bottom flap
    wuxma::Storyboard sb{nullptr};
    std::string d;  // what it shows: "4", "04", "PM", " " (blank)
    Corners label;
    int theme = 0;  // the theme its images show
    bool gone = false;
    float wu = kDigitW;  // width in design units
    double cw = 0, ch = 0;
    UINT px = 0;  // art width in pixels
};

struct Group {
    wuxc::StackPanel el{nullptr};
    char unit;  // h, m, s; a = AM / PM
    bool mini = false;
    std::vector<Cell> cells;
    int count = 0;  // cards planned (cells fills up as they are built)
};

struct Parts {
    std::string h, m, s;  // " 4": in 12-hour Digits the tens card collapses
    std::string ampm;      // "AM", "PM" or ""
    std::wstring date, weekday;
};

struct ClockUi {
    wuxc::Grid root{nullptr};
    wuxs::Rectangle backdrop{nullptr};
    wuxc::StackPanel column{nullptr}, wrap{nullptr};
    wuxc::TextBlock date{nullptr};
    std::vector<Group> groups;
    std::unique_ptr<Painter> painter;
    std::map<std::string, wuxmi::WriteableBitmap> art[2];  // per theme, "px|value"
    double k = 0;                                          // DIPs per design unit
    double lastW = 0, lastH = 0;
    bool secs = false;  // seconds on screen, as cards of either size
    bool built = false;
    bool wantIntro = true;
    double busyUntil = 0;
    std::string lastKey;
    std::wstring lastDate;
    int painted = 0;  // theme on screen
    bool wasOn = true;
};

static ClockUi* g_ui = nullptr;  // UI thread only; deliberately never freed off it

static bool Pairs() {
    return g_settings.style == 1;
}
static bool AmPmCard() {
    return !g_settings.h24 && g_settings.ampmCard;
}

static bool NightNow() {
    int h = g_now().wHour, a = g_settings.darkFrom, b = g_settings.darkTo;
    return a < b ? h >= a && h < b : h >= a || h < b;
}
static int ThemeNow() {
    return g_settings.theme == 2 ? (NightNow() ? 0 : 1) : g_settings.theme;
}

static std::atomic<int> g_displayState{1};  // 0 off, 1 on, 2 dimmed
static bool DisplayOn() {
    return g_displayState.load() != 0;
}

static bool AnimationsOff() {
    BOOL on = TRUE;
    SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0, &on, 0);
    return !on;
}

static Parts PartsNow() {
    SYSTEMTIME st = g_now();
    Parts v;
    int h = st.wHour;
    char buf[8];
    if (!g_settings.h24) {
        v.ampm = h < 12 ? "AM" : "PM";
        h = h % 12 ? h % 12 : 12;
        snprintf(buf, sizeof buf, Pairs() ? "%02d" : "%2d", h);  // Digits 2:08 -> " 2"
    } else {
        snprintf(buf, sizeof buf, "%02d", h);
    }
    v.h = buf;
    snprintf(buf, sizeof buf, "%02d", st.wMinute);
    v.m = buf;
    snprintf(buf, sizeof buf, "%02d", st.wSecond);
    v.s = buf;
    if (g_settings.cardDate || g_settings.cardWeekday) {
        WCHAR d[64];
        if (GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, DATE_SHORTDATE, &st, nullptr, d, ARRAYSIZE(d), nullptr)) v.date = d;
        if (GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &st, L"dddd", d, ARRAYSIZE(d), nullptr)) v.weekday = d;
    }
    return v;
}
static bool TensBlank(const Parts& v) {
    return v.h[0] == ' ';
}
// What cell i of unit u shows at v.
static std::string ValueOf(const Parts& v, char u, int i) {
    if (u == 'a') return v.ampm;
    const std::string& s = u == 'h' ? v.h : u == 'm' ? v.m : v.s;
    return Pairs() ? s : std::string(1, s[i]);
}
// The card the date and AM / PM belong to: the first visible hours card.
static int FirstHours(const Parts& v) {
    return !Pairs() && TensBlank(v) ? 1 : 0;
}
static Corners LabelFor(const Parts* v, const Group& g, int i) {
    Corners c;
    if (!v || g.mini) return c;
    if (g.unit == 'h' && i == FirstHours(*v)) {
        if (g_settings.cardDate) c.tl = v->date;
        if (!v->ampm.empty() && !AmPmCard()) {
            // LyricDock puts AM top left and PM bottom left; with the date on
            // top, and on Pairs cards, it is always bottom left.
            std::wstring a(v->ampm.begin(), v->ampm.end());
            (Pairs() || g_settings.cardDate || v->ampm == "PM" ? c.bl : c.tl) = a;
        }
    }
    if (g.unit == 'm' && i == g.count - 1 && g_settings.cardWeekday) c.tc = v->weekday;
    return c;
}
static std::string KeyOf(const Parts& v) {
    return v.h + v.m + (g_ui->secs ? v.s : "") + v.ampm;
}

static wuxm::SolidColorBrush Brush(uint32_t rgb, double a = 1) {
    return wuxm::SolidColorBrush(winrt::Windows::UI::ColorHelper::FromArgb(
        (uint8_t)std::lround(a * 255), (rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255));
}

static bool Blank(const std::string& d) {
    return d.find_first_not_of(' ') == std::string::npos;
}
static std::string ArtKey(const std::string& d, UINT px) {
    return std::to_string(px) + "|" + (Blank(d) ? "" : d);
}

static bool EnsurePainter() {
    if (g_ui->painter) return true;
    auto p = std::make_unique<Painter>();
    if (!p->Init()) {
        Wh_Log(L"Direct2D is not available");
        return false;
    }
    g_ui->painter = std::move(p);
    return true;
}

// A card's image, drawn the first time it is needed. At 4K a Pairs card is
// ~8 MB, so the cache keeps only what is on screen once it grows; a flip in
// flight holds its own references to the images it uses.
static wf::IInspectable Art(const std::string& d, int theme, const Cell& c) {
    if (Blank(d) && !g_settings.cards) return nullptr;
    auto& cache = g_ui->art[theme];
    const std::string key = ArtKey(d, c.px);
    if (auto it = cache.find(key); it != cache.end()) return it->second;
    if (!EnsurePainter() || c.px < 8) return nullptr;
    if (cache.size() >= 24) {
        std::set<std::string> keep;
        for (auto& g : g_ui->groups) {
            for (auto& o : g.cells) keep.insert(ArtKey(o.d, o.px));
        }
        for (auto it = cache.begin(); it != cache.end();) it = keep.count(it->first) ? std::next(it) : cache.erase(it);
    }
    UINT h;
    auto bgra = g_ui->painter->Card(Blank(d) ? "" : d, theme == 1, g_settings.cards, c.wu, c.px, h);
    wuxmi::WriteableBitmap wb((int)c.px, (int)h);
    memcpy(wb.PixelBuffer().data(), bgra.data(), bgra.size());
    wb.Invalidate();
    cache.insert_or_assign(key, wb);
    return wb;
}

static void Paint(Layer& l, const std::string& d, const Corners& label, int theme, const Cell& c) {
    auto art = Art(d, theme, c);
    l.img.Source(art ? art.as<wuxm::ImageSource>() : nullptr);
    l.tl.Text(label.tl);
    l.tc.Text(label.tc);
    l.bl.Text(label.bl);
}

// A cell at rest showing d: no storyboard, flaps hidden.
static void Rest(Cell& c, const std::string& d, const Corners& label, int theme) {
    if (c.sb) {
        c.sb.Stop();
        c.sb = nullptr;
    }
    c.d = d;
    c.label = label;
    c.theme = theme;
    for (Layer* l : {&c.top, &c.bot, &c.ft, &c.fb}) Paint(*l, d, label, theme, c);
    c.ft.el.Opacity(0);
    c.fb.el.Opacity(0);
    c.ft.turn.RotationX(0);
    c.fb.turn.RotationX(0);
    c.ft.shade.Opacity(0);
    c.fb.shade.Opacity(0);
    c.bot.shade.Opacity(0);
    c.ft.sheen.Opacity(0);
    for (Layer* l : {&c.top, &c.bot}) {
        l->roll.ScaleY(1);
        l->roll.TranslateY(0);
        l->content.Opacity(1);
    }
}

static Layer MakeLayer(const Cell& c, bool top, bool flap) {
    const double cw = c.cw, ch = c.ch, u = ch / kCardH;  // DIPs per design unit
    Layer l;
    l.el = wuxc::Grid();
    l.el.Width(cw);
    l.el.Height(ch);
    l.el.HorizontalAlignment(wux::HorizontalAlignment::Left);
    l.el.VerticalAlignment(wux::VerticalAlignment::Top);
    wuxm::RectangleGeometry clip;
    clip.Rect({0, top ? 0.0f : (float)(ch / 2), (float)cw, (float)(ch / 2)});
    l.el.Clip(clip);

    l.content = wuxc::Grid();
    l.img = wuxc::Image();
    l.img.Width(cw);
    l.img.Height(ch);
    l.img.Stretch(wuxm::Stretch::Fill);
    l.content.Children().Append(l.img);
    const bool light = c.theme == 1;
    // Corners: baselines at y 62 (top) and 412 (bottom), 30 units in.
    auto corner = [&](wux::HorizontalAlignment ha, double baseline) {
        wuxc::TextBlock t;
        // LyricDock's label font, Inter, where it is installed; Segoe UI otherwise.
        t.FontFamily(wuxm::FontFamily(L"Inter, Segoe UI Variable Display"));
        t.FontWeight(winrt::Windows::UI::Text::FontWeights::Bold());
        t.FontSize(32 * u);
        t.CharacterSpacing(20);
        t.TextLineBounds(wux::TextLineBounds::Tight);
        t.Foreground(light ? Brush(0x161616, 0.78) : Brush(0xFFFFFF, 0.74));
        t.HorizontalAlignment(ha);
        t.VerticalAlignment(wux::VerticalAlignment::Top);
        t.TextTrimming(wux::TextTrimming::CharacterEllipsis);
        t.MaxWidth(cw - 60 * u);
        t.Margin({30 * u, (baseline - 25) * u, ha == wux::HorizontalAlignment::Center ? 30 * u : 0, 0});
        l.content.Children().Append(t);
        return t;
    };
    l.tl = corner(wux::HorizontalAlignment::Left, 62);
    l.tc = corner(wux::HorizontalAlignment::Center, 62);
    l.bl = corner(wux::HorizontalAlignment::Left, 412);
    if (!flap) {
        l.roll = wuxm::CompositeTransform();
        l.roll.CenterY(ch / 2);
        l.content.RenderTransform(l.roll);
    }
    l.el.Children().Append(l.content);

    l.shade = wuxs::Rectangle();
    l.shade.Opacity(0);
    if (flap || !top) {
        l.shade.Width(cw);
        l.shade.Height(ch);
        if (flap) {
            l.shadeBrush = Brush(0);
            l.shade.Fill(l.shadeBrush);
        } else {  // the bottom static: a shadow that falls from the hinge
            wuxm::LinearGradientBrush g;
            g.StartPoint({0, 0});
            g.EndPoint({0, 1});
            l.dropA = wuxm::GradientStop();
            l.dropA.Offset(0);
            l.dropB = wuxm::GradientStop();
            l.dropB.Offset(0.7);
            g.GradientStops().Append(l.dropA);
            g.GradientStops().Append(l.dropB);
            l.shade.Fill(g);
        }
        l.el.Children().Append(l.shade);
    }
    if (flap && top) {  // Slow fold's sheen
        l.sheen = wuxs::Rectangle();
        l.sheen.Width(cw);
        l.sheen.Height(ch * 0.6);
        l.sheen.VerticalAlignment(wux::VerticalAlignment::Top);
        wuxm::LinearGradientBrush g;
        g.StartPoint({0, 0});
        g.EndPoint({0, 1});
        for (auto [off, a] : {std::pair{0.0, 0.0}, {0.5, 0.08}, {1.0, 0.0}}) {
            wuxm::GradientStop s;
            s.Offset(off);
            s.Color(winrt::Windows::UI::ColorHelper::FromArgb((uint8_t)(a * 255), 255, 255, 255));
            g.GradientStops().Append(s);
        }
        l.sheen.Fill(g);
        l.sheen.Opacity(0);
        l.sheenY = wuxm::TranslateTransform();
        l.sheen.RenderTransform(l.sheenY);
        l.el.Children().Append(l.sheen);
    }
    if (flap) {
        l.turn = wuxm3::CompositeTransform3D();
        l.turn.CenterX(cw / 2);
        l.turn.CenterY(ch / 2);
        l.el.Transform3D(l.turn);
        l.el.Opacity(0);
    }
    return l;
}

// wu: width in design units; mini: a small card sitting on a full one.
static Cell MakeCell(float wu, bool mini, int theme) {
    const double k = g_ui->k * (mini ? kMini : 1);
    Cell c;
    c.wu = wu;
    c.theme = theme;
    c.cw = std::floor(k * wu);
    c.ch = std::floor(k * kCardH);
    double scale = 1;
    try {
        scale = g_ui->root.XamlRoot().RasterizationScale();
    } catch (...) {
    }
    c.px = (UINT)std::lround(c.cw * scale);
    c.host = wuxc::Grid();
    c.el = wuxc::Grid();
    c.el.Width(c.cw);
    c.el.Height(c.ch);
    wuxm3::PerspectiveTransform3D persp;
    persp.Depth(c.ch * 3.5);
    c.el.Transform3D(persp);
    c.top = MakeLayer(c, true, false);
    c.bot = MakeLayer(c, false, false);
    c.ft = MakeLayer(c, true, true);
    c.fb = MakeLayer(c, false, true);
    for (Layer* l : {&c.top, &c.bot, &c.ft, &c.fb}) c.el.Children().Append(l->el);
    c.host.Children().Append(c.el);
    return c;
}

// The groups on screen: units and how many cards each has.
struct Plan {
    char unit;
    int cards;
    bool mini;
};
static std::vector<Plan> PlanNow() {
    const int per = Pairs() ? 1 : 2;
    std::vector<Plan> p = {{'h', per, false}, {'m', per, false}};
    if (g_settings.secondsCard == 2) p.push_back({'s', per, false});
    if (g_settings.secondsCard == 1) p.push_back({'s', per, true});
    if (AmPmCard()) p.push_back({'a', 1, true});
    return p;
}

// Card size: as big as the space allows - groups side by side when wide,
// stacked when tall - times the size setting. Visible cards only; the small
// cards sit on the big ones and take no room.
static void ComputeSize(double W, double H, bool tensGone) {
    const double wu = Pairs() ? kPairW : kDigitW, inner = g_settings.digitGap * kDigitW,
                 sep = g_settings.groupGap * kDigitW;
    bool stacked = g_settings.layout == 2 || (g_settings.layout == 0 && H > W);
    double rowW = 0, totalW = 0;
    int n = 0;
    for (const auto& p : PlanNow()) {
        if (p.mini) continue;
        int cards = p.cards - (p.unit == 'h' && tensGone ? 1 : 0);
        double w = cards * wu + (cards - 1) * inner;
        rowW = std::max(rowW, w);
        totalW += (n ? sep : 0) + w;
        n++;
    }
    double k = stacked ? std::min(W * 0.88 / rowW, H * 0.84 / (n * kCardH + (n - 1) * sep))
                       : std::min(W * 0.95 / totalW, H * 0.8 / kCardH);
    if (g_settings.date) k *= 0.92;  // room for the date line
    g_ui->k = k * g_settings.scale;
    g_ui->wrap.Orientation(stacked ? wuxc::Orientation::Vertical : wuxc::Orientation::Horizontal);
}

// What a cell shows when the board is filled: 0 = the time; ' ' = blank cards;
// '0' = every figure on 0 (the roll-in starts there; AM / PM starts blank).
static std::string Filled(const std::string& t, char fill) {
    if (!fill) return t;
    std::string s = t;
    for (auto& ch : s) {
        if (ch != ' ') ch = fill == '0' && ch >= '0' && ch <= '9' ? '0' : ' ';
    }
    return s;
}

static void BuildCells(char fill) {
    auto& ui = *g_ui;
    const Parts v = PartsNow();
    const int theme = ThemeNow();
    ui.painted = theme;
    const bool tensGone = !Pairs() && TensBlank(v);
    ComputeSize(ui.lastW, ui.lastH, tensGone);
    if (!EnsurePainter()) return;
    for (auto& g : ui.groups) {
        for (auto& c : g.cells) {
            if (c.sb) c.sb.Stop();
        }
    }
    ui.art[0].clear();
    ui.art[1].clear();
    ui.wrap.Children().Clear();
    ui.wrap.Spacing(ui.k * g_settings.groupGap * kDigitW);
    ui.groups.clear();
    for (const auto& p : PlanNow()) {
        Group grp;
        grp.unit = p.unit;
        grp.mini = p.mini;
        grp.count = p.cards;
        grp.el = wuxc::StackPanel();
        grp.el.Orientation(wuxc::Orientation::Horizontal);
        grp.el.Spacing(ui.k * (p.mini ? kMini : 1) * g_settings.digitGap * kDigitW);
        const float wu = p.unit == 'a' || Pairs() ? kPairW : kDigitW;
        for (int i = 0; i < p.cards; i++) {
            Cell c = MakeCell(wu, p.mini, theme);
            const std::string t = ValueOf(v, p.unit, i);
            Rest(c, p.unit == 'a' && fill ? " " : Filled(t, fill), fill == ' ' ? Corners{} : LabelFor(&v, grp, i),
                 theme);
            c.gone = p.unit == 'h' && i == 0 && tensGone;
            c.host.Visibility(c.gone ? wux::Visibility::Collapsed : wux::Visibility::Visible);
            grp.el.Children().Append(c.host);
            grp.cells.push_back(std::move(c));
        }
        if (!p.mini) {
            grp.el.HorizontalAlignment(wux::HorizontalAlignment::Center);
            ui.wrap.Children().Append(grp.el);
        } else {
            // Seconds on the last minutes card's bottom right, AM / PM on the
            // first hours card's bottom left: inside the bottom half.
            const bool secs = p.unit == 's';
            Group* host = nullptr;
            for (auto& g : ui.groups) {
                if (g.unit == (secs ? 'm' : 'h') && !g.mini) host = &g;
            }
            if (host) {
                Cell& on = secs ? host->cells.back() : host->cells[tensGone ? 1 : 0];
                const double m = kMiniMargin * ui.k;
                grp.el.HorizontalAlignment(secs ? wux::HorizontalAlignment::Right : wux::HorizontalAlignment::Left);
                grp.el.VerticalAlignment(wux::VerticalAlignment::Bottom);
                grp.el.Margin({secs ? 0 : m, 0, secs ? m : 0, m});
                on.host.Children().Append(grp.el);
            }
        }
        ui.groups.push_back(std::move(grp));
    }
    ui.wrap.Opacity(1 - g_settings.dim);
    ui.built = true;
}

struct Hit {
    double fall, land;  // ms from the start of the job
};

// One card from what it shows to d: appends the keyframes to J, starting at t0
// (ms from the start of the job). cut: when this flip takes over the card
// (images swap, a flip still running is cut short). A colour change flips the
// cards over: the side being left keeps the old theme's image.
struct CellJob {
    Track ftOp, ftRot, fbOp, fbRot, ftShade, fbShade, drop, sheenOp, sheenY;
    Track rollScale[2], rollY[2], rollOp[2];
    ObjTrack img[4], tl[4], tc[4], bl[4];
    double end = 0;
};

static Hit Flip(Cell& c, CellJob& J, double cut, double t0, const std::string& d, const Corners& label, double speed,
                Anim anim, int toTheme) {
    const std::string from = c.d;
    const Corners fromLabel = c.label;
    const int fromTheme = c.theme;
    if (AnimationsOff()) anim = Anim::Fade;
    double sp = 1 / (Clamp(g_settings.speed, 0.3, 3) * speed);
    if (g_ui->secs) sp = std::min(sp, 820 / (BaseMs(anim) + (t0 - cut)));  // never outlive the next second
    const double ch = c.ch;

    // static top, static bottom, top flap, bottom flap
    auto images = [&](const std::string& a, int at, const std::string& b, int bt) {
        J.img[0].Set(cut, Art(b, bt, c));
        J.img[1].Set(cut, Art(a, at, c));
        J.img[2].Set(cut, Art(a, at, c));
        J.img[3].Set(cut, Art(b, bt, c));
        const Corners* labels[4] = {&label, &fromLabel, &fromLabel, &label};
        for (int i = 0; i < 4; i++) {
            J.tl[i].Set(cut, winrt::box_value(labels[i]->tl));
            J.tc[i].Set(cut, winrt::box_value(labels[i]->tc));
            J.bl[i].Set(cut, winrt::box_value(labels[i]->bl));
        }
    };
    for (int i = 0; i < 2; i++) {  // the statics' Roll pose at rest
        J.rollScale[i].Set(cut, 1);
        J.rollY[i].Set(cut, 0);
        J.rollOp[i].Set(cut, 1);
    }
    J.drop.Set(cut, 0);
    J.sheenOp.Set(cut, 0);
    J.ftShade.Set(cut, 0);
    c.d = d;
    c.label = label;
    c.theme = toTheme;

    if (anim == Anim::Roll || anim == Anim::Fade) {
        const double land = 420 * sp;
        images(d, toTheme, d, toTheme);
        J.ftOp.Set(cut, 0);
        J.fbOp.Set(cut, 0);
        for (int i = 0; i < 2; i++) {
            const double a0 = anim == Anim::Roll ? 55 : 0, y0 = anim == Anim::Roll ? -0.3 * ch : 0;
            J.rollScale[i].Set(cut, std::cos(a0 * kPi / 180));
            J.rollY[i].Set(cut, y0);
            J.rollOp[i].Set(cut, 0);
            Sample(t0, land, kEaseOut, [&](double t, double p) {
                J.rollScale[i].To(t, std::cos(a0 * (1 - p) * kPi / 180));
                J.rollY[i].To(t, y0 * (1 - p));
                J.rollOp[i].To(t, p);
            });
        }
        J.end = std::max(J.end, t0 + land);
        return {t0, t0 + land};
    }

    images(from, fromTheme, d, toTheme);
    const Motion m = MotionFor(anim, sp);
    // Shading follows the card each flap is made of: black cards sink into
    // near-black as they turn away from the light; paper-white cards only dim a
    // little, in a warm tone.
    struct Shade {
        uint32_t c;
        double fall, land, drop;
    };
    auto SH = [](bool light) { return light ? Shade{0x4b4238, 0.26, 0.34, 0.14} : Shade{0, 0.65, 0.85, 0.35}; };
    const Shade shFrom = SH(fromTheme == 1), shTo = SH(toTheme == 1);
    c.ft.shadeBrush.Color(Brush(shFrom.c).Color());
    c.fb.shadeBrush.Color(Brush(shTo.c).Color());
    c.bot.dropA.Color(Brush(shFrom.c).Color());
    auto clear = Brush(shFrom.c).Color();
    clear.A = 0;
    c.bot.dropB.Color(clear);

    J.ftOp.Set(cut, 1);
    J.ftRot.Set(cut, 0);
    J.fbOp.Set(cut, 0);
    J.fbRot.Set(cut, 90);
    J.fbShade.Set(cut, shTo.land);
    J.ftOp.To(t0, 1);
    J.ftRot.To(t0, 0);
    Sample(t0, m.fall, m.topEase, [&](double t, double p) {
        Key k = KeyAt(m.top, p);
        J.ftRot.To(t, k.rot);
        J.ftOp.To(t, k.op);
    });
    J.ftShade.To(t0, 0);
    Sample(t0, m.fall, kEaseIn, [&](double t, double p) { J.ftShade.To(t, shFrom.fall * p); });
    J.drop.To(t0, 0);
    Sample(t0, m.fall + m.hit * 1.5, kCssEaseInOut,
           [&](double t, double p) { J.drop.To(t, shFrom.drop * (p < 0.5 ? p * 2 : (1 - p) * 2)); });
    if (anim == Anim::Fold) {
        J.sheenY.Set(cut, -0.36 * ch);
        J.sheenOp.To(t0, 0);
        J.sheenY.To(t0, -0.36 * ch);
        Sample(t0, m.fall, kLinear, [&](double t, double p) {
            J.sheenOp.To(t, 0.5 * p);
            J.sheenY.To(t, (-0.36 + 0.6 * p) * ch);
        });
    }
    const double b0 = t0 + m.fall;
    J.fbOp.To(b0, 0);
    J.fbRot.To(b0, 90);
    Sample(b0, m.dur, m.botEase, [&](double t, double p) {
        Key k = KeyAt(m.bot, p);
        J.fbRot.To(t, k.rot);
        J.fbOp.To(t, k.op);
    });
    J.fbShade.To(b0, shTo.land);
    Sample(b0, m.hit * 1.3, kEaseOut, [&](double t, double p) { J.fbShade.To(t, shTo.land * (1 - p)); });
    J.end = std::max(J.end, std::max(b0 + m.dur, t0 + m.fall + m.hit * 1.5));
    return {t0, b0 + m.hit};
}

// Turns the job into the cell's storyboard and starts it.
static void Run(Cell& c, CellJob& J) {
    if (c.sb) c.sb.Stop();
    wuxma::Storyboard sb;
    Layer* L[4] = {&c.top, &c.bot, &c.ft, &c.fb};
    for (int i = 0; i < 4; i++) {
        AddTrack(sb, J.img[i], L[i]->img, L"Source");
        AddTrack(sb, J.tl[i], L[i]->tl, L"Text");
        AddTrack(sb, J.tc[i], L[i]->tc, L"Text");
        AddTrack(sb, J.bl[i], L[i]->bl, L"Text");
    }
    AddTrack(sb, J.ftOp, c.ft.el, L"Opacity");
    AddTrack(sb, J.ftRot, c.ft.turn, L"RotationX");
    AddTrack(sb, J.fbOp, c.fb.el, L"Opacity");
    AddTrack(sb, J.fbRot, c.fb.turn, L"RotationX");
    AddTrack(sb, J.ftShade, c.ft.shade, L"Opacity");
    AddTrack(sb, J.fbShade, c.fb.shade, L"Opacity");
    AddTrack(sb, J.drop, c.bot.shade, L"Opacity");
    AddTrack(sb, J.sheenOp, c.ft.sheen, L"Opacity");
    AddTrack(sb, J.sheenY, c.ft.sheenY, L"Y");
    for (int i = 0; i < 2; i++) {
        AddTrack(sb, J.rollScale[i], L[i]->roll, L"ScaleY");
        AddTrack(sb, J.rollY[i], L[i]->roll, L"TranslateY");
        AddTrack(sb, J.rollOp[i], L[i]->content, L"Opacity");
    }
    sb.BeginTime(std::chrono::duration_cast<wf::TimeSpan>(std::chrono::duration<double, std::milli>(g_settings.latency)));
    c.sb = sb;
    sb.Begin();
}

static std::vector<Cell*> VisibleCells() {
    std::vector<Cell*> out;
    for (auto& g : g_ui->groups) {
        for (auto& c : g.cells) {
            if (!c.gone) out.push_back(&c);
        }
    }
    return out;
}

// Flip every card that differs from v, the small cards first, then right to
// left (the order a real clock turns). Each card makes its own sound as it
// lands, so several changing at once rattle like a real board.
static void Turn(const Parts& v, double stagger, bool withSound, bool animate) {
    auto& ui = *g_ui;
    const int theme = ThemeNow();
    if (!Pairs() && !ui.groups.empty() && ui.groups[0].cells[0].gone != TensBlank(v)) {
        // 9:59 -> 10:00 in 12-hour time: the card count changes, so the cards
        // are rebuilt at their new size showing what they showed, then flip.
        // The tens card that appears flips in from blank.
        std::vector<std::pair<std::string, Corners>> old;
        for (auto& g : ui.groups) {
            for (auto& c : g.cells) old.push_back({c.gone ? " " : c.d, c.gone ? Corners{} : c.label});
        }
        BuildCells(0);
        if (!ui.built) return;
        size_t k = 0;
        for (auto& g : ui.groups) {
            for (auto& c : g.cells) {
                if (k < old.size()) Rest(c, old[k].first, old[k].second, theme);
                k++;
            }
        }
    }
    SoundJob sound;
    std::vector<Hit> hits;
    bool big = false;
    double landAt = -1;
    int n = 0;
    for (auto g = ui.groups.rbegin(); g != ui.groups.rend(); ++g) {
        for (int i = (int)g->cells.size() - 1; i >= 0; i--) {
            Cell& c = g->cells[i];
            const std::string d = ValueOf(v, g->unit, i);
            const Corners label = LabelFor(&v, *g, i);
            if (c.gone || (d == c.d && label == c.label && c.theme == theme)) continue;
            if (!animate) {
                Rest(c, d, label, theme);
                continue;
            }
            if (g->unit != 's') big = true;
            Rest(c, c.d, c.label, c.theme);  // stopping the last flip must not show stale base values
            CellJob J;
            Hit h = Flip(c, J, 0, n * stagger, d, label, 1, g_settings.anim, theme);
            Run(c, J);
            hits.push_back(h);
            landAt = std::max(landAt, h.land);
            n++;
        }
    }
    if (withSound && !hits.empty() && (!ui.secs || g_settings.soundEveryFlip || big)) {
        for (const auto& h : hits) sound.Card(h.fall / 1000, h.land / 1000, hits.size() > 1 ? 0.8 : 1);
        sound.Thump(landAt / 1000);
        AudioFlush(sound);
    }
}

static Anim FlapAnim() {  // the board always flaps
    return g_settings.anim == Anim::Roll || g_settings.anim == Anim::Fade ? Anim::Classic : g_settings.anim;
}

static bool Figures(const std::string& s) {
    return !s.empty() && s.find_first_not_of("0123456789") == std::string::npos;
}
// One flap of the airport board: every figure not yet at its target moves on
// by one, together (so "00" reaches "46" in six flaps, not forty-six).
static std::string Advance(const std::string& cur, const std::string& target) {
    std::string n = cur.size() == target.size() ? cur : std::string(target.size(), '0');
    for (size_t i = 0; i < n.size(); i++) {
        if (n[i] != target[i]) n[i] = n[i] >= '0' && n[i] <= '9' ? (char)('0' + (n[i] - '0' + 1) % 10) : '0';
    }
    return n;
}

// The airport board: every card turns forward one flap at a time until it
// shows its target, each at its own pace, clacking at every flap. The next flap
// lets go as the previous one lands. AM / PM just flips in from blank.
static void Roll(bool withSound) {
    const Parts v = PartsNow();
    const int theme = ThemeNow();
    SoundJob sound;
    double end = 0;
    int i = 0;
    for (auto& g : g_ui->groups) {
        for (int ci = 0; ci < (int)g.cells.size(); ci++) {
            Cell& c = g.cells[ci];
            if (c.gone) continue;
            const std::string target = ValueOf(v, g.unit, ci);
            const Corners label = LabelFor(&v, g, ci);
            const double pace = 3.7 + Rand() * 0.8;  // x normal speed: ~60 ms a flap
            double t = i++ * 40 + Rand() * 45;
            CellJob J;
            for (int guard = 0; c.d != target && guard < 12; guard++) {
                const std::string next = Figures(target) ? Advance(c.d, target) : target;
                Hit h = Flip(c, J, t, t, next, label, pace, FlapAnim(), theme);
                if (withSound) sound.Card(h.fall / 1000, h.land / 1000, 0.55);
                if (next == target) sound.Thump(h.land / 1000);
                t = h.land;
            }
            if (!J.img[0].k.empty()) {
                Run(c, J);
                end = std::max(end, J.end);
            }
        }
    }
    if (withSound) AudioFlush(sound);
    g_ui->busyUntil = g_ms() + g_settings.latency + end;
}

// Enter: roll from 0, flip from blank, or just appear.
static void Show(bool on) {
    auto& ui = *g_ui;
    ui.secs = g_settings.secondsCard != 0;
    const int how = AnimationsOff() || !on ? 2 : g_settings.intro;
    BuildCells(how == 0 ? '0' : how == 1 ? ' ' : 0);
    if (!ui.built) {  // no card art (no Direct2D?): try again shortly
        ui.wantIntro = true;
        ui.busyUntil = g_ms() + 5000;
        return;
    }
    const Parts v = PartsNow();
    ui.lastKey = KeyOf(v);
    if (how == 0) {
        Roll(true);
    } else if (how == 1) {
        Turn(v, 70, false, true);
        ui.busyUntil = g_ms() + g_settings.latency + 900;
    }
    ui.lastKey = "";  // catch up on a second that passes meanwhile
}

// A colour change (Automatic at the dark / light hours): every card flips over,
// left to right, from the old colour to the new one; the figures stay.
static void Retheme() {
    const int theme = ThemeNow();
    const int old = g_ui->painted;
    SoundJob sound;
    double end = 0;
    int i = 0;
    for (Cell* c : VisibleCells()) {
        Rest(*c, c->d, c->label, c->theme);
        CellJob J;
        Hit h = Flip(*c, J, 0, i++ * 70, c->d, c->label, 1, FlapAnim(), theme);
        Run(*c, J);
        sound.Card(h.fall / 1000, h.land / 1000, 0.7);
        end = std::max(end, J.end);
    }
    AudioFlush(sound);
    g_ui->art[old].clear();  // the flips hold the old images they still need
    g_ui->painted = theme;
    g_ui->busyUntil = g_ms() + g_settings.latency + end;
}

static void UpdateDate() {
    auto& ui = *g_ui;
    ui.date.Visibility(g_settings.date ? wux::Visibility::Visible : wux::Visibility::Collapsed);
    if (!g_settings.date) return;
    // "Wednesday, 28 October": the weekday, then the locale's own day and month
    SYSTEMTIME st = g_now();
    WCHAR day[64], pattern[64], md[64];
    if (!GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SMONTHDAY, pattern, ARRAYSIZE(pattern))) {
        wcscpy(pattern, L"d MMMM");
    }
    if (!GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &st, L"dddd", day, ARRAYSIZE(day), nullptr) ||
        !GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &st, pattern, md, ARRAYSIZE(md), nullptr)) {
        return;
    }
    std::wstring text = std::wstring(day) + L", " + md;
    if (ui.lastDate != text) {
        ui.lastDate = text;
        ui.date.Text(text);
    }
    ui.date.FontSize(std::max(18.0, ui.k * kCardH * 0.085));
    const bool lightBg = g_settings.background == 0 && ThemeNow() == 1 && !g_settings.cards;
    ui.date.Foreground(lightBg ? Brush(0, 0.55) : Brush(0xFFFFFF, 0.74));
}

static void StyleBackdrop() {
    auto& ui = *g_ui;
    const bool paper = ThemeNow() == 1 && !g_settings.cards;
    if (g_settings.background == 0) {
        ui.backdrop.Fill(paper ? Brush(0xE7E3DB) : Brush(0));
    } else {
        ui.backdrop.Fill(Brush(0, g_settings.background == 1 ? 0.55 : 0));
    }
}

static wuxc::Grid ClockCreate() {
    g_ui = new ClockUi();
    auto& ui = *g_ui;
    ui.root = wuxc::Grid();
    ui.root.Name(L"WindhawkLockScreenFlipClock");
    ui.root.IsHitTestVisible(false);
    ui.root.HorizontalAlignment(wux::HorizontalAlignment::Stretch);
    ui.root.VerticalAlignment(wux::VerticalAlignment::Stretch);
    wuxc::Grid::SetRowSpan(ui.root, 100);
    wuxc::Grid::SetColumnSpan(ui.root, 100);
    ui.backdrop = wuxs::Rectangle();
    ui.root.Children().Append(ui.backdrop);
    ui.column = wuxc::StackPanel();
    ui.column.HorizontalAlignment(wux::HorizontalAlignment::Center);
    ui.column.VerticalAlignment(wux::VerticalAlignment::Center);
    ui.wrap = wuxc::StackPanel();
    ui.wrap.HorizontalAlignment(wux::HorizontalAlignment::Center);
    ui.column.Children().Append(ui.wrap);
    ui.date = wuxc::TextBlock();
    ui.date.FontFamily(wuxm::FontFamily(L"Segoe UI Variable Display"));
    ui.date.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
    ui.date.HorizontalAlignment(wux::HorizontalAlignment::Center);
    ui.date.Margin({0, 24, 0, 0});
    ui.column.Children().Append(ui.date);
    ui.root.Children().Append(ui.column);
    StyleBackdrop();
    UpdateDate();
    return ui.root;
}

// Every UI tick. reshow: the lock screen has just come up again.
static void ClockTick(bool reshow) {
    AudioReap();
    if (!g_ui) return;
    auto& ui = *g_ui;
    const double W = ui.root.ActualWidth(), H = ui.root.ActualHeight();
    if (W < 1 || H < 1) return;
    const bool on = DisplayOn();
    if (reshow || (on && !ui.wasOn)) ui.wantIntro = true;
    if (!on && ui.wasOn) AudioRelease();
    ui.wasOn = on;
    if (W != ui.lastW || H != ui.lastH) {  // first layout, rotation, resolution change
        ui.lastW = W, ui.lastH = H;
        if (!ui.wantIntro) {
            BuildCells(0);
            ui.lastKey = "";
        }
    }
    if (g_ms() < ui.busyUntil) return;
    if (ui.wantIntro && on) {
        ui.wantIntro = false;
        StyleBackdrop();
        UpdateDate();
        Show(on);
        return;
    }
    if (!ui.built) return;
    if (ThemeNow() != ui.painted) {
        StyleBackdrop();
        if (on) {
            Retheme();
        } else {
            BuildCells(0);
        }
        return;
    }
    const Parts v = PartsNow();
    const std::string key = KeyOf(v);
    if (key == ui.lastKey) return;
    const bool minute = key.substr(0, 4) != ui.lastKey.substr(0, 4);
    ui.lastKey = key;
    if (minute) UpdateDate();
    Turn(v, g_settings.anim == Anim::Cascade ? 110 : 35, true, on);
}

static void ClockDestroy() {
    if (!g_ui) return;
    for (auto& g : g_ui->groups) {
        for (auto& c : g.cells) {
            if (c.sb) c.sb.Stop();
        }
    }
    AudioRelease();
    delete g_ui;
    g_ui = nullptr;
}

// ---------------------------------------------------------------------------
// The lock screen: find the tree, hide the stock clock, host ours
// ---------------------------------------------------------------------------

struct Hidden {
    wux::FrameworkElement el{nullptr};
    wf::IInspectable value{nullptr};  // the local value we replaced, or null
};

struct LockUi {
    wuxc::Panel host{nullptr};
    wux::UIElement clock{nullptr};
    std::vector<Hidden> hidden;
};
static LockUi* g_lock = nullptr;  // UI thread only

static void Unhide() {
    for (auto& h : g_lock->hidden) {
        try {
            if (h.value) {
                h.el.SetValue(wux::UIElement::VisibilityProperty(), h.value);
            } else {
                h.el.ClearValue(wux::UIElement::VisibilityProperty());
            }
        } catch (...) {
        }
    }
    g_lock->hidden.clear();
}

static void Hide(const wux::FrameworkElement& el) {
    for (auto& h : g_lock->hidden) {
        if (h.el == el) {
            el.Visibility(wux::Visibility::Collapsed);  // re-assert: Windows may reset it
            return;
        }
    }
    Hidden h{el, nullptr};
    auto local = el.ReadLocalValue(wux::UIElement::VisibilityProperty());
    if (local != wux::DependencyProperty::UnsetValue()) h.value = local;
    g_lock->hidden.push_back(h);
    el.Visibility(wux::Visibility::Collapsed);
}

// Where the clock goes. LockApp's tree (26300):
//
//   LockRootGrid
//     LockScreenContentPassword       the sign-in screen's background
//     InvokableLockContentControl
//       LockScrollViewer > LockContainer (two screens tall)
//         LockScreenContent           the curtain: scrolls up and away when dismissed
//           wallpaper layers, LockCreativeOverlay (Spotlight)
//           LockScreenTextContent     stock clock, widgets, media controls, network / battery
//
// The clock belongs in LockScreenContent, just under LockScreenTextContent: it
// leaves with the curtain, and the media controls and status icons stay on top
// of the black backdrop. Anywhere higher (LockRootGrid) it would stay behind
// and cover the sign-in screen. Older layouts without it fall back to the root.
struct Found {
    wuxc::Panel curtain{nullptr}, root{nullptr};
    wux::UIElement text{nullptr};  // LockScreenTextContent
};

static void Scan(const wux::DependencyObject& node, Found& found, std::vector<wux::FrameworkElement>& stock,
                 int depth) {
    if (!node || depth > 96) return;
    if (g_lock->clock && node == g_lock->clock) return;
    if (auto el = node.try_as<wux::FrameworkElement>()) {
        auto name = el.Name();
        if (name == L"LockScreenContent") {
            if (auto p = el.try_as<wuxc::Panel>()) found.curtain = p;
        } else if (name == L"LockScreenTextContent") {
            found.text = el;
        } else if (name == L"LockRootGrid" || (!found.root && name == L"MainPageContainer")) {
            if (auto p = el.try_as<wuxc::Panel>()) found.root = p;
        } else if (((name == L"TimeAndDatePanel" || name == L"Time" || name == L"Date") &&
                    g_settings.hideStockClock) ||
                   ((name == L"AdaptiveCardDetailContainer" || name == L"DetailTextPanel" ||
                     name == L"LockWidgetCanvas" || name == L"WidgetCanvasPanel" || name == L"HotspotContainer") &&
                    g_settings.hideWidgets)) {
            stock.push_back(el);
        }
    }
    int n = wuxm::VisualTreeHelper::GetChildrenCount(node);
    for (int i = 0; i < n; i++) Scan(wuxm::VisualTreeHelper::GetChild(node, i), found, stock, depth + 1);
}

static void RemoveClock() {
    if (g_lock->clock && g_lock->host) {
        uint32_t i;
        if (g_lock->host.Children().IndexOf(g_lock->clock, i)) g_lock->host.Children().RemoveAt(i);
    }
    g_lock->clock = nullptr;
    ClockDestroy();
}

static void AttachToTree() {
    auto content = wux::Window::Current().Content();
    if (!content) return;
    Found found;
    std::vector<wux::FrameworkElement> stock;
    Scan(content, found, stock, 0);
    wuxc::Panel root = found.curtain ? found.curtain : found.root;
    if (!root) return;  // still being built: the next scan retries
    if (root != g_lock->host) {
        RemoveClock();
        Unhide();
        g_lock->host = root;
        Wh_Log(L"Found the lock screen tree: %s", found.curtain ? L"LockScreenContent" : L"root only");
    }
    for (auto& el : stock) Hide(el);
    bool attached = g_lock->clock && wuxm::VisualTreeHelper::GetParent(g_lock->clock);
    if (!attached) {
        RemoveClock();
        g_lock->clock = ClockCreate();
        uint32_t at;
        if (found.curtain && found.text && root.Children().IndexOf(found.text, at)) {
            root.Children().InsertAt(at, g_lock->clock);
        } else {
            root.Children().Append(g_lock->clock);
        }
        Wh_Log(L"Clock added");
    }
}

// ---------------------------------------------------------------------------
// UI thread plumbing (after Lock Screen Clock Styler)
// ---------------------------------------------------------------------------

static std::atomic<HWND> g_coreWindow{nullptr};
static std::atomic<HHOOK> g_getMessageHook{nullptr};
static std::atomic<bool> g_settingsDirty{false};
static HANDLE g_stopEvent = nullptr;
static HANDLE g_workerThread = nullptr;
static UINT g_runOnUiThreadMessage = 0;
static HPOWERNOTIFY g_displayNotify = nullptr;

using UiProc = void (*)();
struct UiCall {
    UiProc proc;
    bool ran;
};

static LRESULT CALLBACK RunOnUiThreadHookProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {
        auto* cwp = (const CWPSTRUCT*)lParam;
        if (cwp && cwp->message == g_runOnUiThreadMessage) {
            auto* call = (UiCall*)cwp->lParam;
            try {
                call->proc();
                call->ran = true;
            } catch (winrt::hresult_error const& e) {
                Wh_Log(L"UI call failed: %s", e.message().c_str());
            } catch (...) {
                Wh_Log(L"UI call failed");
            }
        }
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

// Runs proc on the window's thread and returns once it has finished. A
// suspended lock screen cannot answer, hence the timeout.
static bool RunOnUiThread(HWND hwnd, UiProc proc) {
    DWORD tid = GetWindowThreadProcessId(hwnd, nullptr);
    if (!tid) return false;
    if (tid == GetCurrentThreadId()) {
        proc();
        return true;
    }
    static std::mutex lock;
    std::lock_guard<std::mutex> guard(lock);
    HHOOK hook = SetWindowsHookExW(WH_CALLWNDPROC, RunOnUiThreadHookProc, nullptr, tid);
    if (!hook) return false;
    UiCall call{proc, false};
    DWORD_PTR result;
    SendMessageTimeoutW(hwnd, g_runOnUiThreadMessage, 0, (LPARAM)&call, SMTO_NORMAL, 1500, &result);
    UnhookWindowsHookEx(hook);
    return call.ran;
}

static double g_lastTick = 0, g_lastScan = -1e9;

static void Rebuild() {
    g_settingsDirty = false;
    LoadSettings();
    if (g_lock) {
        RemoveClock();
        Unhide();
        g_lock->host = nullptr;
        AttachToTree();
    }
}

static void OnUiTick() {
    const double now = SteadyMs();
    // LockApp is suspended while the PC is unlocked: a long gap between ticks
    // means the lock screen has just come up again.
    const bool reshow = g_lastTick && now - g_lastTick > 2500;
    g_lastTick = now;
    if (!g_lock) g_lock = new LockUi();
    if (g_settingsDirty) {
        Rebuild();
        return;
    }
    if (reshow || now - g_lastScan >= 3000 || !g_lock->clock) {
        g_lastScan = now;
        AttachToTree();
    }
    ClockTick(reshow);
}

static LRESULT CALLBACK GetMessageHookProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {
        try {
            OnUiTick();
            if (g_afterUiTick) g_afterUiTick();
        } catch (winrt::hresult_error const& e) {
            Wh_Log(L"UI tick failed: %s", e.message().c_str());
        } catch (...) {
            Wh_Log(L"UI tick failed");
        }
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

static void TeardownOnUiThread() {
    if (HHOOK hook = g_getMessageHook.exchange(nullptr)) UnhookWindowsHookEx(hook);
    if (g_lock) {
        RemoveClock();
        Unhide();
        delete g_lock;
        g_lock = nullptr;
    }
}

static BOOL CALLBACK FindCoreWindowProc(HWND hwnd, LPARAM lParam) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    WCHAR cls[64];
    if (pid == GetCurrentProcessId() && GetClassNameW(hwnd, cls, ARRAYSIZE(cls)) &&
        !_wcsicmp(cls, L"Windows.UI.Core.CoreWindow")) {
        *(HWND*)lParam = hwnd;
        return FALSE;
    }
    return TRUE;
}

static DWORD WINAPI WorkerThreadProc(LPVOID) {
    // Wait for LockApp's CoreWindow, then hook its UI thread.
    while (WaitForSingleObject(g_stopEvent, 250) == WAIT_TIMEOUT) {
        HWND hwnd = nullptr;
        EnumWindows(FindCoreWindowProc, (LPARAM)&hwnd);
        if (!hwnd) continue;
        HHOOK hook = SetWindowsHookExW(WH_GETMESSAGE, GetMessageHookProc, nullptr,
                                       GetWindowThreadProcessId(hwnd, nullptr));
        if (hook) {
            g_coreWindow = hwnd;
            g_getMessageHook = hook;
            Wh_Log(L"Hooked the lock screen UI thread");
            break;
        }
    }
    // Wake the UI thread just after every second boundary, so flips land on the
    // second, and at least four times a second.
    for (;;) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        DWORD wait = std::min<DWORD>(250, 1000 - st.wMilliseconds + 3);
        if (WaitForSingleObject(g_stopEvent, wait) != WAIT_TIMEOUT) break;
        HWND hwnd = g_coreWindow;
        if (hwnd && !IsWindow(hwnd)) {
            Wh_Log(L"The lock screen window is gone");
            break;
        }
        if (hwnd) PostMessageW(hwnd, WM_NULL, 0, 0);
    }
    return 0;
}

// DEVICE_NOTIFY_SUBSCRIBE_PARAMETERS and GUID_CONSOLE_DISPLAY_STATE, which
// mingw's headers and import libraries lack.
struct DisplayNotifyParams {
    ULONG(CALLBACK* callback)(PVOID context, ULONG type, PVOID setting);
    PVOID context;
};
constexpr DWORD kDeviceNotifyCallback = 2;
static const GUID kConsoleDisplayState = {0x6fe69556, 0x704a, 0x47a0, {0x8f, 0x24, 0xc2, 0x8d, 0x93, 0x6f, 0xda, 0x47}};

static ULONG CALLBACK DisplayStateCallback(PVOID, ULONG type, PVOID setting) {
    auto* s = (POWERBROADCAST_SETTING*)setting;
    if (type == PBT_POWERSETTINGCHANGE && s && s->PowerSetting == kConsoleDisplayState &&
        s->DataLength >= sizeof(DWORD)) {
        g_displayState = *(DWORD*)s->Data;
    }
    return 0;
}

#ifndef LSFC_NO_ENTRY_POINTS

BOOL Wh_ModInit() {
    Wh_Log(L"Init");
    LoadSettings();
    g_runOnUiThreadMessage = RegisterWindowMessageW(L"Windhawk_LockScreenFlipClock_RunOnUiThread");
    g_stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!g_runOnUiThreadMessage || !g_stopEvent) return FALSE;
    static DisplayNotifyParams params{DisplayStateCallback, nullptr};
    if (PowerSettingRegisterNotification(&kConsoleDisplayState, kDeviceNotifyCallback, &params,
                                         &g_displayNotify) != ERROR_SUCCESS) {
        g_displayNotify = nullptr;
        Wh_Log(L"No display-state notifications; assuming the display is on");
    }
    g_workerThread = CreateThread(nullptr, 0, WorkerThreadProc, nullptr, 0, nullptr);
    return g_workerThread != nullptr;
}

void Wh_ModUninit() {
    Wh_Log(L"Uninit");
    SetEvent(g_stopEvent);
    if (g_workerThread) {
        WaitForSingleObject(g_workerThread, INFINITE);
        CloseHandle(g_workerThread);
    }
    if (g_displayNotify) PowerSettingUnregisterNotification(g_displayNotify);
    HWND hwnd = g_coreWindow;
    if (!(hwnd && IsWindow(hwnd) && RunOnUiThread(hwnd, TeardownOnUiThread))) {
        // The UI thread did not answer (suspended). XAML objects belong to it
        // and are left alone; nothing of ours is called back once unhooked.
        if (HHOOK hook = g_getMessageHook.exchange(nullptr)) UnhookWindowsHookEx(hook);
    }
    CloseHandle(g_stopEvent);
}

void Wh_ModSettingsChanged() {
    HWND hwnd = g_coreWindow;
    if (g_getMessageHook && hwnd && IsWindow(hwnd) && RunOnUiThread(hwnd, Rebuild)) return;
    if (!g_getMessageHook) LoadSettings();  // nothing reads them yet
    g_settingsDirty = true;                 // otherwise the next UI tick rebuilds
}

#endif
