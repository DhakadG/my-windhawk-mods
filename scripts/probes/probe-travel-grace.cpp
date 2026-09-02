// Exercise tray-hover-expand-plus's real decision logic against the geometry
// measured on this machine, the way probe-sendkeys.cpp drives the real SendKeys.
//
// The mod's .wh.cpp is #included with the Windhawk API stubbed out, so these
// assertions run the shipping ShouldClose(), ClampCandidates() and hit tests --
// not a copy of them, which would only prove the copy right.
//
// Measured on Windows 11 build 26340 (probe-tray-uia / probe-tray-tree):
//   EarTrumpet tray icon   UIA rect (3016,2100)-(3840,2160)  <- 824px wide, wrong
//                          real hit region ends at x=3061 (next tray button)
//   EarTrumpet popup       (3375,923)-(3825,2080)
// So the cursor has to cross a ~315px horizontal gap and travel upwards.
//
// Build:
//   & 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe' -std=c++23 -O1 -static `
//       -target x86_64-w64-mingw32 -DUNICODE -D_UNICODE -municode `
//       scripts\probes\probe-travel-grace.cpp -lole32 -loleaut32 -o probe-travel-grace.exe

#include <windows.h>

#include <cstdio>

// ---- Windhawk API stubs, enough to compile the mod as a plain program -------
#define WH_MOD_ID L"tray-hover-expand-plus"
#define Wh_Log(...) ((void)0)
static int Wh_GetIntSetting(const wchar_t*, ...) {
    return 0;
}
static const wchar_t* Wh_GetStringSetting(const wchar_t*, ...) {
    return L"";
}
static void Wh_FreeStringSetting(const wchar_t*) {}
static BOOL Wh_SetFunctionHook(void*, void*, void**) {
    return TRUE;
}

#include "../../mods/tray-hover-expand-plus/tray-hover-expand-plus.wh.cpp"

// ---- test harness ----------------------------------------------------------

static int g_failed = 0;

static void Check(bool ok, const char* what) {
    printf("  %-58s %s\n", what, ok ? "ok" : "FAILED");
    if (!ok) g_failed++;
}

static const RECT ICON{3016, 2100, 3061, 2160};
static const RECT POPUP{3375, 923, 3825, 2080};

static Settings BaseSettings() {
    Settings s;
    s.itemCloseDelay = 400;
    s.itemTravelTimeout = 1500;
    s.itemTravelPad = 48;
    s.itemPad = 6;
    s.itemPopupPad = 24;
    return s;
}

static Target OpenTarget() {
    Target t;
    t.isChevron = false;
    t.rect = ICON;
    t.haveRect = true;
    t.state = St::Open;
    t.popup = (HWND)1;
    t.popupPid = 100;
    t.popupRect = POPUP;
    return t;
}

static TickContext Ctx(const Settings& s, ULONGLONG now, POINT pt) {
    TickContext c{};
    c.now = now;
    c.pt = pt;
    c.s = &s;
    c.selfPid = 0;
    c.anyButtonDown = false;
    c.pressedSinceTick = false;
    c.menu = nullptr;
    return c;
}

// Run the popup's lifetime with the cursor following `path`, one sample every
// 50ms (the default polling interval). Returns the time at which the mod would
// have closed the popup, or 0 if it survived the whole path.
template <typename PathFn>
static ULONGLONG RunPath(const Settings& s, PathFn path, ULONGLONG durationMs,
                         bool overIconFirstTick = true) {
    Target t = OpenTarget();
    ULONGLONG t0 = 100000;
    t.lastEngagedAt = t0;
    for (ULONGLONG ms = 0; ms <= durationMs; ms += 50) {
        POINT p = path(ms);
        TickContext c = Ctx(s, t0 + ms, p);
        bool overIcon = PtInRectPad(t.rect, p, s.itemPad);
        if (ms == 0 && overIconFirstTick) overIcon = true;
        if (ShouldClose(t, c, overIcon)) return ms;
    }
    return 0;
}

int wmain() {
    Settings s = BaseSettings();

    printf("Rect clamping (the 824px-wide notification icon)\n");
    {
        // Two candidates as the tray really reports them: the notification icon
        // with a rectangle spanning the rest of the tray, and the network button
        // that actually sits at x=3061.
        std::vector<Candidate> cands;
        Candidate a{};
        a.rect = {3016, 2100, 3840, 2160};
        Candidate b{};
        b.rect = {3061, 2100, 3091, 2156};
        Candidate clock{};
        clock.rect = {3332, 2100, 3460, 2156};
        cands.push_back(a);
        cands.push_back(b);
        cands.push_back(clock);
        ClampCandidates(cands);
        Check(cands[0].rect.right == 3061, "icon clipped at the next button, not 3840");
        Check(cands[1].rect.right == 3091, "the neighbour is left alone");
        Check(cands[2].rect.right == 3460, "a wide but legitimate button is left alone");
        POINT onClock{3400, 2130};
        Check(!PtInRectPad(cands[0].rect, onClock, 6),
              "the clock is no longer inside the icon's hit area");
    }

    printf("Open hidden-icons flyout must not clip the taskbar underneath\n");
    {
        // Live regression. With the flyout open the walk returns its icons too,
        // as a grid ABOVE the taskbar. Row-blind clipping cut the chevron from
        // 40px to 8px and inverted the flyout icons' rectangles, which broke the
        // chevron hit test and put the mod into a re-find storm and a
        // close/reopen loop.
        std::vector<Candidate> cands;
        Candidate chevron{};
        chevron.rect = {2219, 2100, 2259, 2156};  // on the taskbar
        Candidate flyIconA{};
        flyIconA.rect = {2214, 1878, 2264, 1928};  // in the flyout, above it
        Candidate flyIconB{};
        flyIconB.rect = {2264, 1878, 2314, 1928};
        cands.push_back(chevron);
        cands.push_back(flyIconA);
        cands.push_back(flyIconB);
        ClampCandidates(cands);
        Check(cands[0].rect.right == 2259, "chevron keeps its full width while the flyout is open");
        Check(cands[0].rect.bottom - cands[0].rect.top == 56, "chevron height untouched");
        Check(cands[1].rect.bottom > cands[1].rect.top, "flyout icon rect stays valid");
        Check(cands[1].rect.right == 2264, "flyout icons still clip against their own row");
        POINT onChevron{2240, 2130};
        Check(PtInRectPad(cands[0].rect, onChevron, 4),
              "cursor on the chevron is still on the chevron");
    }

    printf("Popup lifetime\n");
    {
        // A: cursor sits inside the popup -> never closes.
        ULONGLONG closed = RunPath(s, [](ULONGLONG) { return POINT{3600, 1500}; }, 5000);
        Check(closed == 0, "A cursor resting in the popup keeps it open");
    }
    {
        // B: the real trip. Icon centre to popup centre over 900ms, which is a
        // slow, deliberate movement across the gap.
        POINT from{3038, 2130}, to{3600, 1500};
        ULONGLONG closed = RunPath(
            s,
            [&](ULONGLONG ms) {
                double f = ms / 900.0;
                if (f > 1.0) f = 1.0;
                return POINT{(LONG)(from.x + (to.x - from.x) * f),
                             (LONG)(from.y + (to.y - from.y) * f)};
            },
            3000);
        Check(closed == 0, "B travelling icon -> popup never closes on the way");
    }
    {
        // C: leaves the icon heading the other way, along the taskbar and away
        // from the popup. Must close, and close on the plain close delay rather
        // than lingering for the travel timeout.
        ULONGLONG closed = RunPath(
            s, [](ULONGLONG ms) { return POINT{(LONG)(3010 - (LONG)ms), 2130}; }, 5000);
        Check(closed != 0, "C cursor moving away closes the popup");
        Check(closed <= 700, "C ... on the close delay, not the travel timeout");
    }
    {
        // D: stops dead in the gap. Held generously, but bounded.
        ULONGLONG closed = RunPath(s, [](ULONGLONG) { return POINT{3200, 2050}; }, 6000);
        Check(closed != 0, "D cursor parked in the gap eventually closes");
        Check(closed >= 1500 && closed <= 2400,
              "D ... after travel timeout + close delay, not sooner or forever");
    }
    {
        // E: reaches the popup, then overshoots just past its left edge. The
        // doubled pad after having been inside must absorb it.
        ULONGLONG closed = RunPath(
            s,
            [](ULONGLONG ms) {
                if (ms < 500) return POINT{3600, 1500};   // inside
                return POINT{3340, 1500};                 // 35px outside the edge
            },
            4000);
        Check(closed == 0, "E small overshoot after entering the popup is forgiven");
    }
    {
        // F: a context menu is up somewhere. The user is busy; never close.
        Target t = OpenTarget();
        ULONGLONG t0 = 100000;
        t.lastEngagedAt = t0;
        bool closedAny = false;
        for (ULONGLONG ms = 0; ms <= 6000; ms += 50) {
            TickContext c = Ctx(s, t0 + ms, POINT{100, 100});
            c.menu = (HWND)7;
            if (ShouldClose(t, c, false)) closedAny = true;
        }
        Check(!closedAny, "F an open context menu suspends closing entirely");
    }
    {
        // G: a mouse button is held (dragging a volume slider) far from both.
        Target t = OpenTarget();
        ULONGLONG t0 = 100000;
        t.lastEngagedAt = t0;
        bool closedAny = false;
        for (ULONGLONG ms = 0; ms <= 6000; ms += 50) {
            TickContext c = Ctx(s, t0 + ms, POINT{3600, 1500});
            c.anyButtonDown = true;
            if (ShouldClose(t, c, false)) closedAny = true;
        }
        Check(!closedAny, "G a held mouse button suspends closing");
    }
    {
        // H: travel grace turned off -> plain close delay, no corridor at all.
        Settings off = BaseSettings();
        off.itemTravelTimeout = 0;
        POINT from{3038, 2130}, to{3600, 1500};
        ULONGLONG closed = RunPath(
            off,
            [&](ULONGLONG ms) {
                double f = ms / 900.0;
                if (f > 1.0) f = 1.0;
                return POINT{(LONG)(from.x + (to.x - from.x) * f),
                             (LONG)(from.y + (to.y - from.y) * f)};
            },
            3000);
        Check(closed != 0 && closed <= 700, "H travel timeout 0 falls back to the close delay");
    }
    {
        // I: a fast flick, 3000px/s, sampled at 50ms -> only a handful of
        // samples between the icon and the popup.
        POINT from{3038, 2130}, to{3600, 1500};
        ULONGLONG closed = RunPath(
            s,
            [&](ULONGLONG ms) {
                double f = ms / 200.0;
                if (f > 1.0) f = 1.0;
                return POINT{(LONG)(from.x + (to.x - from.x) * f),
                             (LONG)(from.y + (to.y - from.y) * f)};
            },
            3000);
        Check(closed == 0, "I a fast flick to the popup is not cut off");
    }

    {
        // J: another popup is on top of this one, covering the same area. Seen
        // live: Quick Settings (3360,0 480x2095) sits over EarTrumpet's flyout
        // (3375,924 450x1157), and the rectangle test alone held EarTrumpet open
        // for 7.5s while the user worked in Quick Settings.
        Target t = OpenTarget();
        ULONGLONG t0 = 100000;
        t.lastEngagedAt = t0;
        t.popup = (HWND)1;
        ULONGLONG closed = 0;
        for (ULONGLONG ms = 0; ms <= 6000 && !closed; ms += 50) {
            TickContext c = Ctx(s, t0 + ms, POINT{3600, 1500});  // inside our rect
            c.underRoot = (HWND)2;                               // but another window is on top
            c.underRootPid = 999;                                // ... from another process
            if (ShouldClose(t, c, false)) closed = ms;
        }
        Check(closed != 0, "J a popup covered by another app's window stops being held open");
        Check(closed <= 700, "J ... and closes on the close delay");

        // Same point, nothing on top -> still held, i.e. the fix did not just
        // break the ordinary case.
        Target t2 = OpenTarget();
        t2.lastEngagedAt = t0;
        ULONGLONG closed2 = 0;
        for (ULONGLONG ms = 0; ms <= 3000 && !closed2; ms += 50) {
            TickContext c = Ctx(s, t0 + ms, POINT{3600, 1500});
            c.underRoot = t2.popup;
            c.underRootPid = t2.popupPid;
            if (ShouldClose(t2, c, false)) closed2 = ms;
        }
        Check(closed2 == 0, "J ... while the popup itself under the cursor still holds");

        // K: a sibling window of the SAME process on top. That is how a XAML
        // flyout hosts its own content, so it must not read as occlusion.
        Target t3 = OpenTarget();
        t3.lastEngagedAt = t0;
        ULONGLONG closed3 = 0;
        for (ULONGLONG ms = 0; ms <= 3000 && !closed3; ms += 50) {
            TickContext c = Ctx(s, t0 + ms, POINT{3600, 1500});
            c.underRoot = (HWND)77;          // a different window
            c.underRootPid = t3.popupPid;    // but the popup's own process
            if (ShouldClose(t3, c, false)) closed3 = ms;
        }
        Check(closed3 == 0, "K a same-process child popup on top is not occlusion");
    }

    printf("\n%s\n", g_failed ? "FAILURES" : "all checks passed");
    return g_failed ? 1 : 0;
}
