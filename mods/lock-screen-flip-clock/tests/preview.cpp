// Lock Screen Flip Clock preview and self-test. Compiles the mod itself
// (LSFC_NO_ENTRY_POINTS drops the Windhawk entry points, WH_EDITING turns every
// Wh_* call into a no-op) and hosts its real clock in a XAML island, so the
// cards, the flips and the sounds can be checked without locking the PC.
//
//   preview check  <out>            self-checks (motion, time, sound) + WAVs of every sound
//   preview art    <out>            every card, both colours, as PNG
//   preview frames <out> [k=v ...]  a scripted run filmed frame by frame (exact: the
//                                   storyboards are paused and seeked), plus its soundtrack
//   preview live   [k=v ...]        the clock in a window, real time, real sound
//
// k=v: style=digits|pairs theme=dark|light|auto anim=classic|gravity|... intro=roll|flip|none
//      sound=... thump=off|light|firm seconds=off|small|large ampm=corner|card
//      cardDate=0|1 cardWeekday=0|1 h24=0|1 cards=0|1 date=0|1
//      background=black|dim|wallpaper size=30..100 start=HH:MM:SS secs=N fps=N
//      w=px h=px reshowAt=ms
//
// Build and run: tests\run-preview.ps1 <mode> ...

#define LSFC_NO_ENTRY_POINTS
#include "../lock-screen-flip-clock.wh.cpp"

#include <wincodec.h>
#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>

#include <cstdio>
#include <map>

namespace wuxh = winrt::Windows::UI::Xaml::Hosting;

static std::map<std::string, std::string> g_args;
static std::string Arg(const char* k, const char* def) {
    auto it = g_args.find(k);
    return it == g_args.end() ? def : it->second;
}
static int ArgInt(const char* k, int def) {
    auto it = g_args.find(k);
    return it == g_args.end() ? def : atoi(it->second.c_str());
}

static void ApplyArgs() {
    ModSettings s;  // the YAML defaults
    auto pick = [](const std::string& v, std::initializer_list<const char*> opts) {
        int i = 0;
        for (auto o : opts) {
            if (v == o) return i;
            i++;
        }
        return 0;
    };
    s.theme = pick(Arg("theme", "dark"), {"dark", "light", "auto"});
    s.anim = (Anim)pick(Arg("anim", "classic"), {"classic", "gravity", "snappy", "bounce", "fold", "cascade", "roll", "fade"});
    s.intro = pick(Arg("intro", "roll"), {"roll", "flip", "none"});
    s.sound = (Sound)pick(Arg("sound", "mechanical"), {"off", "solari", "mechanical", "click", "whoosh", "soft"});
    s.thump = pick(Arg("thump", "light"), {"off", "light", "firm"});
    s.style = pick(Arg("style", "digits"), {"digits", "pairs"});
    s.secondsCard = pick(Arg("seconds", "off"), {"off", "small", "large"});
    s.ampmCard = Arg("ampm", "corner") == "card";
    s.cardDate = ArgInt("cardDate", 0);
    s.cardWeekday = ArgInt("cardWeekday", 0);
    s.h24 = ArgInt("h24", 0);
    s.cards = ArgInt("cards", 1);
    s.date = ArgInt("date", 0);
    s.background = pick(Arg("background", "black"), {"black", "dim", "wallpaper"});
    s.layout = pick(Arg("layout", "auto"), {"auto", "side", "stacked"});
    s.scale = ArgInt("size", 96) / 100.0;
    s.volume = ArgInt("volume", 40) / 100.0;
    s.soundEveryFlip = Arg("soundEvery", "minute") == "all";
    s.speed = ArgInt("speed", 100) / 100.0;
    s.latency = ArgInt("latency", 0);
    g_settings = s;
}

// ---- fake time: a wall clock that starts at start= and a steady ms counter
static double g_fakeMs = 0;
static ULONGLONG g_fakeBase = 0;  // FILETIME ticks of start=
static SYSTEMTIME FakeNow() {
    ULARGE_INTEGER u;
    u.QuadPart = g_fakeBase + (ULONGLONG)(g_fakeMs * 10000);
    FILETIME ft{u.LowPart, u.HighPart};
    SYSTEMTIME st;
    FileTimeToSystemTime(&ft, &st);
    return st;
}
static double FakeMs() {
    return g_fakeMs;
}
static void SetStart(const std::string& hms) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    int h = 10, m = 59, s = 57;
    sscanf(hms.c_str(), "%d:%d:%d", &h, &m, &s);
    st.wHour = (WORD)h, st.wMinute = (WORD)m, st.wSecond = (WORD)s, st.wMilliseconds = 0;
    FILETIME ft;
    SystemTimeToFileTime(&st, &ft);
    g_fakeBase = ((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
}

// ---- output helpers
static void WriteWav(const std::wstring& path, const std::vector<float>& s) {
    FILE* f = _wfopen(path.c_str(), L"wb");
    if (!f) return;
    std::vector<int16_t> pcm(s.size());
    for (size_t i = 0; i < s.size(); i++) pcm[i] = (int16_t)std::lround(Clamp(s[i], -1, 1) * 32767);
    uint32_t data = (uint32_t)(pcm.size() * 2), riff = 36 + data, fmtLen = 16, rate = kRate, byteRate = kRate * 2;
    uint16_t pcmTag = 1, ch = 1, align = 2, bits = 16;
    fwrite("RIFF", 1, 4, f), fwrite(&riff, 4, 1, f), fwrite("WAVEfmt ", 1, 8, f), fwrite(&fmtLen, 4, 1, f);
    fwrite(&pcmTag, 2, 1, f), fwrite(&ch, 2, 1, f), fwrite(&rate, 4, 1, f), fwrite(&byteRate, 4, 1, f);
    fwrite(&align, 2, 1, f), fwrite(&bits, 2, 1, f), fwrite("data", 1, 4, f), fwrite(&data, 4, 1, f);
    fwrite(pcm.data(), 2, pcm.size(), f);
    fclose(f);
}

static void WritePng(const std::wstring& path, UINT w, UINT h, const uint8_t* bgra, bool premultiplied) {
    winrt::com_ptr<IWICImagingFactory> wic;
    winrt::check_hresult(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                          __uuidof(IWICImagingFactory), wic.put_void()));
    winrt::com_ptr<IWICBitmap> bmp;
    winrt::check_hresult(wic->CreateBitmapFromMemory(w, h, premultiplied ? GUID_WICPixelFormat32bppPBGRA : GUID_WICPixelFormat32bppBGRA,
                                                     w * 4, w * h * 4, (BYTE*)bgra, bmp.put()));
    winrt::com_ptr<IWICBitmapSource> src = bmp.as<IWICBitmapSource>();
    if (premultiplied) {  // PNG wants straight alpha
        winrt::com_ptr<IWICBitmapSource> conv;
        winrt::check_hresult(WICConvertBitmapSource(GUID_WICPixelFormat32bppBGRA, src.get(), conv.put()));
        src = conv;
    }
    winrt::com_ptr<IWICStream> stream;
    winrt::check_hresult(wic->CreateStream(stream.put()));
    winrt::check_hresult(stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE));
    winrt::com_ptr<IWICBitmapEncoder> enc;
    winrt::check_hresult(wic->CreateEncoder(GUID_ContainerFormatPng, nullptr, enc.put()));
    winrt::check_hresult(enc->Initialize(stream.get(), WICBitmapEncoderNoCache));
    winrt::com_ptr<IWICBitmapFrameEncode> frame;
    winrt::check_hresult(enc->CreateNewFrame(frame.put(), nullptr));
    winrt::check_hresult(frame->Initialize(nullptr));
    winrt::check_hresult(frame->SetSize(w, h));
    WICPixelFormatGUID fmt = GUID_WICPixelFormat32bppBGRA;
    winrt::check_hresult(frame->SetPixelFormat(&fmt));
    winrt::check_hresult(frame->WriteSource(src.get(), nullptr));
    winrt::check_hresult(frame->Commit());
    winrt::check_hresult(enc->Commit());
}

static std::wstring W(const std::string& s) {
    return std::wstring(s.begin(), s.end());
}

// ---- self-checks
static int g_pass = 0, g_fail = 0;
static void Check(bool ok, const char* what, double a = NAN, double b = NAN) {
    (ok ? g_pass : g_fail)++;
    printf("%s  %s", ok ? "PASS" : "FAIL", what);
    if (!std::isnan(a)) printf("  [%g", a);
    if (!std::isnan(b)) printf(", %g", b);
    if (!std::isnan(a)) printf("]");
    printf("\n");
}

static double Peak(const std::vector<float>& s) {
    double p = 0;
    for (float v : s) p = std::max(p, (double)std::abs(v));
    return p;
}
static bool Finite(const std::vector<float>& s) {
    for (float v : s) {
        if (!std::isfinite(v)) return false;
    }
    return true;
}
// Mean frequency from zero crossings over [a, b) seconds.
static double ZcHz(const std::vector<float>& s, double a, double b) {
    size_t i0 = (size_t)(a * kRate), i1 = std::min(s.size(), (size_t)(b * kRate));
    int z = 0;
    for (size_t i = i0 + 1; i < i1; i++) z += (s[i - 1] < 0) != (s[i] < 0);
    return z / 2.0 / ((i1 - i0) / (double)kRate);
}
static double Energy(const std::vector<float>& s, double a, double b) {
    size_t i0 = (size_t)(a * kRate), i1 = std::min(s.size(), (size_t)(b * kRate));
    double e = 0;
    for (size_t i = i0; i < i1; i++) e += s[i] * s[i];
    return e;
}

static int RunCheck(const std::wstring& out) {
    g_settings = ModSettings{};
    // motion
    Check(std::abs(EaseAt(kEaseIn, 0)) < 1e-6 && std::abs(EaseAt(kEaseIn, 1) - 1) < 1e-6, "cubic-bezier hits its endpoints");
    Check(EaseAt(kEaseIn, 0.5) < 0.5 && EaseAt(kEaseOut, 0.5) > 0.5, "ease-in lags, ease-out leads", EaseAt(kEaseIn, 0.5), EaseAt(kEaseOut, 0.5));
    Check(std::abs(EaseAt(kCssEaseInOut, 0.5) - 0.5) < 1e-3, "ease-in-out is symmetric");
    {
        std::vector<Key> k = {{0, 0, 1}, {0.5, -45, 1}, {1, -90, 0}};
        Check(std::abs(KeyAt(k, 0.25).rot + 22.5) < 1e-9 && std::abs(KeyAt(k, 0.75).op - 0.5) < 1e-9, "keyframes interpolate linearly");
    }
    for (Anim a : {Anim::Classic, Anim::Snappy, Anim::Bounce, Anim::Fold, Anim::Cascade, Anim::Gravity}) {
        Motion m = MotionFor(a, 1);
        bool ok = m.fall > 0 && m.dur > 0 && m.hit > 0 && m.hit <= m.dur + 1e-9 && m.top.back().rot == -90 &&
                  m.top.back().op == 0 && m.bot.front().rot == 90 && m.bot.front().op == 0 && m.bot.back().op == 1 &&
                  std::abs(m.bot.back().rot) < 1e-6;
        for (size_t i = 1; i < m.bot.size(); i++) ok = ok && m.bot[i].off >= m.bot[i - 1].off;
        Check(ok, "a flap falls to -90, lands at 0, keys in order", m.fall, m.dur);
    }
    {
        Motion g = MotionFor(Anim::Gravity, 1);
        double T90 = GravityT90();
        Check(T90 > 0.3 && T90 < 0.95, "gravity passes horizontal part-way through", T90);
        // fastest at the bottom: the last tenth of the top flap's fall covers more angle than the first tenth
        Key a = KeyAt(g.top, 0.1), b = KeyAt(g.top, 0.9), c = KeyAt(g.top, 0.97);
        Check(std::abs(a.rot) < std::abs(c.rot - b.rot) * 3, "gravity accelerates", a.rot, c.rot - b.rot);
    }
    // time formatting
    g_now = FakeNow;
    g_ms = FakeMs;
    g_ui = new ClockUi();
    auto at = [](const char* hms) {
        SetStart(hms);
        g_fakeMs = 0;
        return PartsNow();
    };
    g_settings.h24 = false;
    Group hours{nullptr, 'h', false, {}}, minutes{nullptr, 'm', false, {}};
    hours.count = minutes.count = 2;
    Parts p = at("00:05:09");
    Check(p.h == "12" && p.m == "05" && p.s == "09" && p.ampm == "AM", "12-hour: 00:05 is 12:05 AM");
    p = at("09:07:00");
    Check(p.h == " 9" && TensBlank(p) && LabelFor(&p, hours, 1).tl == L"AM" && LabelFor(&p, hours, 0).tl.empty(),
          "12-hour: 9:07 collapses the tens card, AM top left on the units card");
    p = at("13:00:00");
    Check(p.h == " 1" && p.ampm == "PM" && LabelFor(&p, hours, 1).bl == L"PM", "12-hour: 13:00 is 1 PM, bottom left");
    g_settings.cardDate = g_settings.cardWeekday = true;
    p = at("13:00:00");
    Check(!p.date.empty() && !p.weekday.empty() && LabelFor(&p, hours, 1).tl == p.date &&
              LabelFor(&p, hours, 1).bl == L"PM" && LabelFor(&p, minutes, 1).tc == p.weekday &&
              LabelFor(&p, minutes, 0).tc.empty(),
          "date top left with PM below it, weekday on the last minutes card");
    g_settings.ampmCard = true;
    Check(LabelFor(&p, hours, 1).bl.empty(), "AM / PM as its own card leaves the corner empty");
    g_settings.ampmCard = false;
    g_settings.style = 1;  // Pairs
    hours.count = minutes.count = 1;
    p = at("09:07:46");
    Check(ValueOf(p, 'h', 0) == "09" && ValueOf(p, 'm', 0) == "07" && ValueOf(p, 's', 0) == "46" &&
              ValueOf(p, 'a', 0) == "AM" && LabelFor(&p, hours, 0).bl == L"AM" && LabelFor(&p, minutes, 0).tc == p.weekday,
          "Pairs: 9:07 is 09 07 with AM and the weekday on its cards");
    Check(Advance("00", "46") == "11" && Advance("33", "46") == "44" && Advance("45", "46") == "46" &&
              Advance("0", "7") == "1" && Advance("  ", "10") == "00",
          "the airport board moves every figure not yet there, together");
    {
        std::string s = "00";
        int flaps = 0;
        while (s != "46" && flaps < 20) s = Advance(s, "46"), flaps++;
        Check(flaps == 6, "00 reaches 46 in six flaps", flaps);
    }
    Check(Filled("09", '0') == "00" && Filled(" 9", '0') == " 0" && Filled("09", ' ') == "  " && Filled("09", 0) == "09",
          "filling the board: 0s, blanks, or the time");
    g_settings.style = 0;
    g_settings.cardDate = g_settings.cardWeekday = false;
    g_settings.h24 = true;
    p = at("09:07:00");
    Check(p.h == "09" && p.ampm.empty() && !TensBlank(p), "24-hour: 09:07");
    g_ui->secs = false;
    Check(KeyOf(at("09:07:01")) == KeyOf(at("09:07:59")), "without seconds the key changes once a minute");
    g_ui->secs = true;
    Check(KeyOf(at("09:07:01")) != KeyOf(at("09:07:02")), "with seconds it changes every second");
    g_settings.theme = 2;
    g_settings.darkFrom = 22, g_settings.darkTo = 7;
    at("23:00:00");
    bool night = ThemeNow() == 0;
    at("12:00:00");
    Check(night && ThemeNow() == 1, "automatic: dark at 23:00, light at noon");
    delete g_ui;
    g_ui = nullptr;
    g_settings = ModSettings{};

    // sound
    std::vector<std::pair<const char*, Sound>> kinds = {{"solari", Sound::Solari}, {"mechanical", Sound::Mechanical},
                                                         {"click", Sound::Click},   {"whoosh", Sound::Whoosh},
                                                         {"soft", Sound::Soft}};
    Motion m = MotionFor(Anim::Classic, 1);
    const double fall = 0.05, land = fall + (m.fall + m.hit) / 1000;
    for (auto [name, kind] : kinds) {
        g_settings.sound = kind;
        g_settings.thump = 0;
        SoundJob j;
        j.Card(fall, land, 1);
        Limit(j.mix.s);
        Check(!j.mix.s.empty() && Finite(j.mix.s) && Peak(j.mix.s) > 0.02 && Peak(j.mix.s) < 0.999, name, Peak(j.mix.s));
        if (kind != Sound::Whoosh) {
            Check(Energy(j.mix.s, land - 0.005, land + 0.06) > Energy(j.mix.s, 0, land - 0.03), "   ...the loudest part is the landing");
        }
        WriteWav(out + L"\\sound-" + W(name) + L".wav", j.mix.s);
    }
    for (int t : {1, 2}) {
        g_settings.sound = Sound::Off;
        g_settings.thump = t;
        SoundJob j;
        j.Thump(0.02);
        Limit(j.mix.s);
        double hz = ZcHz(j.mix.s, 0.02, 0.02 + (t == 2 ? 0.05 : 0.03));
        double head = Energy(j.mix.s, 0, 0.02 + (t == 2 ? 0.05 : 0.03) + 0.02), all = Energy(j.mix.s, 0, 1);
        Check(Finite(j.mix.s) && Peak(j.mix.s) > 0.2 && Peak(j.mix.s) < 0.999, t == 2 ? "firm thump: loud enough, never clips" : "light thump: loud enough, never clips", Peak(j.mix.s));
        Check(hz > 40 && hz < 200, "   ...it is a low thump, not a click (Hz)", hz);
        Check(head / all > 0.9, "   ...and short: 90% of it within the motor run + 20 ms", head / all);
        WriteWav(out + (t == 2 ? L"\\thump-firm.wav" : L"\\thump-light.wav"), j.mix.s);
    }
    {
        g_settings = ModSettings{};
        SoundJob j;
        j.Clack(0.1, 1);
        size_t before = j.landed.size();
        j.Clack(0.104, 1);  // 4 ms later: dropped
        Check(j.landed.size() == before, "two clacks 4 ms apart: the second is dropped");
        j.Clack(0.13, 1);
        Check(j.landed.size() == before + 1, "30 ms apart: both play");
        SoundJob t;
        t.Thump(0.2);
        t.Thump(0.3);
        t.Thump(0.5);
        Check(t.thumps.size() == 2, "cards landing within 140 ms share one thump");
    }
    {
        // A whole board landing at once: four cards, staggered like a real minute flip, then the thump.
        g_settings = ModSettings{};
        g_settings.sound = Sound::Solari;
        SoundJob j;
        for (int i = 0; i < 4; i++) j.Card(i * 0.035, i * 0.035 + (m.fall + m.hit) / 1000, 0.8);
        j.Thump(3 * 0.035 + (m.fall + m.hit) / 1000);
        std::vector<float> raw = j.mix.s;
        Limit(j.mix.s);
        Check(Peak(j.mix.s) < 0.999 && Peak(j.mix.s) > 0.1, "a whole board through the limiter stays below full scale", Peak(raw), Peak(j.mix.s));
        WriteWav(out + L"\\board-solari.wav", j.mix.s);
    }
    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail;
}

static int RunArt(const std::wstring& out) {
    Painter p;
    if (!p.Init()) {
        printf("FAIL  Direct2D init\n");
        return 1;
    }
    auto one = [&](const std::string& v, bool light, bool withCard, float wu, const wchar_t* name) {
        UINT h, w = (UINT)wu;
        double t0 = SteadyMs();
        auto px = p.Card(v, light, withCard, wu, w, h);
        WritePng(out + L"\\" + name + L".png", w, h, px.data(), true);
        printf("%ls  %.0f ms\n", name, SteadyMs() - t0);
    };
    for (int light = 0; light < 2; light++) {
        const std::wstring pre = light ? L"light-" : L"";
        for (int withCard = 1; withCard >= 0; withCard--) {
            for (int d = -1; d <= 9; d++) {
                if (d < 0 && !withCard) continue;
                std::wstring name = pre + (withCard ? L"card-" : L"numeral-") + (d < 0 ? L"blank" : std::to_wstring(d));
                one(d < 0 ? "" : std::string(1, (char)('0' + d)), light, withCard, kDigitW, name.c_str());
            }
        }
        for (const char* v : {"04", "11", "29", "59", "AM", "PM"}) {
            std::wstring name = pre + L"pair-" + std::wstring(v, v + 2);
            one(v, light, true, kPairW, name.c_str());
        }
    }
    return 0;
}

// ---- the XAML island
static HWND g_hwnd, g_island;
static wuxc::Grid g_host{nullptr};

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_SIZE:
            if (g_island) SetWindowPos(g_island, nullptr, 0, 0, LOWORD(lp), HIWORD(lp), SWP_SHOWWINDOW);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            if (wp == VK_ESCAPE) DestroyWindow(hwnd);
            if (wp == 'R' && g_ui) g_ui->wantIntro = true;
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

static void Pump(int ms) {
    double until = SteadyMs() + ms;
    MSG msg;
    do {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (ms) MsgWaitForMultipleObjects(0, nullptr, FALSE, 2, QS_ALLINPUT);
    } while (SteadyMs() < until);
}

template <typename Op>
static void Wait(const Op& op) {
    while (op.Status() == wf::AsyncStatus::Started) Pump(1);
}

static bool CreateIsland(int w, int h) {
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"LsfcPreview";
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    RegisterClassW(&wc);
    RECT r{0, 0, w, h};
    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
    g_hwnd = CreateWindowW(L"LsfcPreview", L"Lock Screen Flip Clock - preview", WS_OVERLAPPEDWINDOW, 40, 40,
                           r.right - r.left, r.bottom - r.top, nullptr, nullptr, wc.hInstance, nullptr);
    static wuxh::WindowsXamlManager manager = wuxh::WindowsXamlManager::InitializeForCurrentThread();
    static wuxh::DesktopWindowXamlSource source;
    auto native = source.as<IDesktopWindowXamlSourceNative>();
    winrt::check_hresult(native->AttachToWindow(g_hwnd));
    native->get_WindowHandle(&g_island);
    g_host = wuxc::Grid();
    // A stand-in wallpaper, for the dimmed and wallpaper backgrounds.
    wuxm::LinearGradientBrush wall;
    wall.StartPoint({0, 0});
    wall.EndPoint({1, 1});
    for (auto [off, c] : {std::pair{0.0, 0x1d3557u}, {0.55, 0x457b9du}, {1.0, 0xe9c46au}}) {
        wuxm::GradientStop s;
        s.Offset(off);
        s.Color(winrt::Windows::UI::ColorHelper::FromArgb(255, (c >> 16) & 255, (c >> 8) & 255, c & 255));
        wall.GradientStops().Append(s);
    }
    g_host.Background(wall);
    source.Content(g_host);
    ShowWindow(g_hwnd, SW_SHOW);
    RECT cr;
    GetClientRect(g_hwnd, &cr);
    SetWindowPos(g_island, nullptr, 0, 0, cr.right, cr.bottom, SWP_SHOWWINDOW);
    g_host.Children().Append(ClockCreate());
    for (int i = 0; i < 100 && g_ui->root.ActualWidth() < 1; i++) Pump(10);
    return g_ui->root.ActualWidth() >= 1;
}

// tee=<path>: everything handed to XAudio2 also goes to <path>.wav, placed on a
// timeline that starts at the FILETIME written to <path>.txt, so
// find-sound.py can find it in a loopback recording.
static std::vector<float> g_tee;
static ULONGLONG g_teeStart = 0;
static void TeeSink(std::vector<float>&& pcm) {
    FILETIME ft;
    GetSystemTimePreciseAsFileTime(&ft);
    ULONGLONG now = ((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    if (!g_teeStart) g_teeStart = now;
    size_t at = (size_t)((now - g_teeStart) / 1e7 * kRate);
    if (g_tee.size() < at + pcm.size()) g_tee.resize(at + pcm.size(), 0.0f);
    for (size_t i = 0; i < pcm.size(); i++) g_tee[at + i] += pcm[i];
    g_audioSink = nullptr;  // and play it for real
    AudioPlay(std::move(pcm));
    g_audioSink = TeeSink;
}

// start=HH:MM:SS in live mode: a wall clock that starts there and runs in real time.
static double g_liveT0 = 0;
static SYSTEMTIME LiveNow() {
    g_fakeMs = SteadyMs() - g_liveT0;
    return FakeNow();
}

static int RunLive() {
    const std::string tee = Arg("tee", "");
    if (!tee.empty()) g_audioSink = TeeSink;
    if (g_args.count("start")) {
        SetStart(Arg("start", ""));
        g_liveT0 = SteadyMs();
        g_now = LiveNow;
    }
    if (!CreateIsland(ArgInt("w", 1600), ArgInt("h", 900))) return 1;
    const double until = SteadyMs() + ArgInt("secs", 0) * 1000.0;
    MSG msg;
    for (;;) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) return 0;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        ClockTick(false);
        if (ArgInt("secs", 0) && SteadyMs() > until) {
            if (!tee.empty()) {
                WriteWav(W(tee) + L".wav", g_tee);
                FILE* f = fopen((tee + ".txt").c_str(), "w");
                fprintf(f, "%llu\n", g_teeStart);
                fclose(f);
                printf("tee: %zu samples\n", g_tee.size());
            }
            return 0;
        }
        MsgWaitForMultipleObjects(0, nullptr, FALSE, 10, QS_ALLINPUT);
    }
}

// Films a scripted run: the fake clock advances one frame at a time, every new
// storyboard is paused at once, and each frame seeks them all to their exact
// position before rendering. reshowAt=ms replays the lock screen coming up.
// Needs an unlocked desktop: while the session is locked XAML jumps island
// animations straight to their end, and every frame shows the settled clock.
static int RunFrames(const std::wstring& out) {
    g_now = FakeNow;
    g_ms = FakeMs;
    SetStart(Arg("start", "10:59:57"));
    std::vector<float> track;
    static std::vector<float>* soundtrack = &track;
    g_audioSink = [](std::vector<float>&& pcm) {
        size_t at = (size_t)(g_fakeMs / 1000 * kRate);
        if (soundtrack->size() < at + pcm.size()) soundtrack->resize(at + pcm.size(), 0.0f);
        for (size_t i = 0; i < pcm.size(); i++) (*soundtrack)[at + i] += pcm[i];
    };
    if (!CreateIsland(ArgInt("w", 1280), ArgInt("h", 720))) return 1;
    const int fps = ArgInt("fps", 50), frames = (int)(ArgInt("secs", 5) * fps / 1.0);
    const int reshowAt = ArgInt("reshowAt", -1);
    std::vector<std::pair<wuxma::Storyboard, double>> boards;
    for (int f = 0; f < frames; f++) {
        g_fakeMs = f * 1000.0 / fps;
        bool reshow = reshowAt >= 0 && std::abs(g_fakeMs - reshowAt) < 500.0 / fps;
        ClockTick(reshow);
        for (Cell* c : VisibleCells()) {
            if (!c->sb) continue;
            bool known = false;
            for (auto& [sb, t] : boards) known = known || sb == c->sb;
            if (!known) {
                c->sb.Pause();
                boards.push_back({c->sb, g_fakeMs});
            }
        }
        for (auto& [sb, t] : boards) {
            sb.SeekAlignedToLastTick(std::chrono::duration_cast<wf::TimeSpan>(
                std::chrono::duration<double, std::milli>(g_fakeMs - t)));
        }
        wuxmi::RenderTargetBitmap rtb;
        auto op = rtb.RenderAsync(g_host);
        Wait(op);
        auto px = rtb.GetPixelsAsync();
        Wait(px);
        auto buf = px.GetResults();
        wchar_t name[32];
        swprintf(name, 32, L"\\f%04d.png", f);
        WritePng(out + name, rtb.PixelWidth(), rtb.PixelHeight(), buf.data(), true);
    }
    track.resize((size_t)(frames * (double)kRate / fps), 0.0f);
    WriteWav(out + L"\\soundtrack.wav", track);
    printf("%d frames at %d fps, %zu boards\n", frames, fps, boards.size());
    return 0;
}

int main(int argc, char** argv) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    std::string mode = argc > 1 ? argv[1] : "live";
    std::wstring out = argc > 2 && !strchr(argv[2], '=') ? W(argv[2]) : L".";
    for (int i = 2; i < argc; i++) {
        if (const char* eq = strchr(argv[i], '=')) g_args[std::string(argv[i], eq - argv[i])] = eq + 1;
    }
    ApplyArgs();
    CreateDirectoryW(out.c_str(), nullptr);
    try {
        if (mode == "check") return RunCheck(out);
        if (mode == "art") return RunArt(out);
        if (mode == "frames") return RunFrames(out);
        return RunLive();
    } catch (winrt::hresult_error const& e) {
        printf("FAIL  %ls (0x%08X)\n", e.message().c_str(), (unsigned)e.code());
        return 2;
    }
}
