// Lock screen test build. The real mod, compiled into a DLL that can be loaded
// into LockApp.exe without Windhawk (tests/inject.cpp), so it can be tested on
// the real lock screen while nobody is there to approve a UAC prompt.
//
// It talks through LockApp's own temp folder (the only one its sandbox can
// write): %LOCALAPPDATA%\Packages\Microsoft.LockApp_cw5n1h2txyewy\AC\Temp
//
//   lsfc-test.log      Wh_Log output, plus what the clock did each second
//   lsfc-tree.txt      the lock screen's visual tree, once
//   settings.ini       key=value settings (missing keys: the mod's defaults)
//   cmd-snap           drop this file: one screenshot of the lock screen
//                      (RenderTargetBitmap, from inside) -> snap-NNN.png
//   cmd-burst          drop this file holding N: N screenshots back to back
//   cmd-settings       re-read settings.ini (Wh_ModSettingsChanged)
//   cmd-stop           Wh_ModUninit, write played.wav, unload the DLL
//
// dryRun=1 in settings.ini hosts the clock in the first panel of any XAML app
// (Calculator), to check this rig before locking the PC.
//
// Build: tests\run-lockapp-test.ps1

#include <windows.h>

#include <cstdarg>
#include <cstdio>
#include <functional>
#include <map>
#include <mutex>
#include <string>

static std::wstring g_dir;  // LockApp's AC\Temp, with a trailing backslash

static void TestLog(PCWSTR fmt, ...) {
    WCHAR msg[1024];
    va_list ap;
    va_start(ap, fmt);
    _vsnwprintf(msg, ARRAYSIZE(msg) - 1, fmt, ap);
    va_end(ap);
    msg[ARRAYSIZE(msg) - 1] = 0;
    SYSTEMTIME st;
    GetLocalTime(&st);
    char line[1200];
    int n = snprintf(line, sizeof line, "%02d:%02d:%02d.%03d ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    n += WideCharToMultiByte(CP_UTF8, 0, msg, -1, line + n, (int)sizeof line - n - 2, nullptr, nullptr) - 1;
    line[n++] = '\n';
    static std::mutex lock;
    std::lock_guard<std::mutex> g(lock);
    if (FILE* f = _wfopen((g_dir + L"lsfc-test.log").c_str(), L"ab")) {
        fwrite(line, 1, n, f);
        fclose(f);
    }
}

static std::map<std::wstring, std::wstring> g_ini;
static std::mutex g_iniLock;
static const std::map<std::wstring, std::wstring> kDefaults = {
    {L"theme", L"dark"},       {L"darkFrom", L"22"},       {L"darkTo", L"7"},         {L"timeFormat", L"system"},
    {L"secondsCard", L"off"},  {L"animation", L"classic"}, {L"intro", L"roll"},       {L"speed", L"100"},
    {L"sound", L"mechanical"}, {L"volume", L"40"},         {L"latency", L"0"},        {L"soundEvery", L"minute"},
    {L"thump", L"light"},      {L"cards", L"1"},           {L"background", L"black"}, {L"layout", L"auto"},
    {L"size", L"96"},          {L"digitGap", L"3"},        {L"groupGap", L"14"},      {L"dim", L"0"},
    {L"date", L"0"},           {L"hideStockClock", L"1"},  {L"hideWidgets", L"1"},    {L"dryRun", L"0"},
    {L"tee", L"1"},            {L"cardStyle", L"digits"}, {L"ampm", L"corner"},  {L"cardDate", L"0"},
    {L"cardWeekday", L"0"},
};
static void LoadIni() {
    std::map<std::wstring, std::wstring> m;
    if (FILE* f = _wfopen((g_dir + L"settings.ini").c_str(), L"r, ccs=UTF-8")) {
        WCHAR line[256];
        while (fgetws(line, ARRAYSIZE(line), f)) {
            std::wstring s = line;
            while (!s.empty() && (s.back() == L'\n' || s.back() == L'\r' || s.back() == L' ')) s.pop_back();
            auto eq = s.find(L'=');
            if (eq != std::wstring::npos) m[s.substr(0, eq)] = s.substr(eq + 1);
        }
        fclose(f);
    }
    std::lock_guard<std::mutex> g(g_iniLock);
    g_ini = m;
}
static std::wstring IniGet(PCWSTR name) {
    std::lock_guard<std::mutex> g(g_iniLock);
    auto it = g_ini.find(name);
    if (it != g_ini.end()) return it->second;
    auto d = kDefaults.find(name);
    return d != kDefaults.end() ? d->second : L"";
}
static int TestGetInt(PCWSTR name, ...) {
    return _wtoi(IniGet(name).c_str());
}
static PCWSTR TestGetString(PCWSTR name, ...) {
    return _wcsdup(IniGet(name).c_str());
}
static void TestFreeString(PCWSTR s) {
    free((void*)s);
}

#define Wh_Log TestLog
#define Wh_GetIntSetting TestGetInt
#define Wh_GetStringSetting TestGetString
#define Wh_FreeStringSetting TestFreeString
#include "../lock-screen-flip-clock.wh.cpp"

#include <psapi.h>
#include <wincodec.h>
#include <winrt/Windows.UI.Xaml.Automation.Peers.h>
#include <winrt/Windows.UI.Xaml.Automation.Provider.h>

// ---- what the clock plays, on a FILETIME timeline (tests/find-sound.py)
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
    TestLog(L"sound: %.0f ms of audio", pcm.size() * 1000.0 / kRate);
    g_audioSink = nullptr;
    AudioPlay(std::move(pcm));
    g_audioSink = TeeSink;
}
static void WriteTee() {
    if (g_tee.empty()) return;
    FILE* f = _wfopen((g_dir + L"played.wav").c_str(), L"wb");
    if (!f) return;
    std::vector<int16_t> pcm(g_tee.size());
    for (size_t i = 0; i < pcm.size(); i++) pcm[i] = (int16_t)std::lround(Clamp(g_tee[i], -1, 1) * 32767);
    uint32_t data = (uint32_t)(pcm.size() * 2), riff = 36 + data, fl = 16, rate = kRate, br = kRate * 2;
    uint16_t tag = 1, ch = 1, al = 2, bits = 16;
    fwrite("RIFF", 1, 4, f), fwrite(&riff, 4, 1, f), fwrite("WAVEfmt ", 1, 8, f), fwrite(&fl, 4, 1, f);
    fwrite(&tag, 2, 1, f), fwrite(&ch, 2, 1, f), fwrite(&rate, 4, 1, f), fwrite(&br, 4, 1, f), fwrite(&al, 2, 1, f);
    fwrite(&bits, 2, 1, f), fwrite("data", 1, 4, f), fwrite(&data, 4, 1, f), fwrite(pcm.data(), 2, pcm.size(), f);
    fclose(f);
    if (FILE* t = _wfopen((g_dir + L"played.txt").c_str(), L"w")) {
        fprintf(t, "%llu\n", g_teeStart);
        fclose(t);
    }
}

// ---- screenshots from inside, on the UI thread
static void SavePng(const std::wstring& path, UINT w, UINT h, const uint8_t* bgra) {
    winrt::com_ptr<IWICImagingFactory> wic;
    winrt::check_hresult(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                          __uuidof(IWICImagingFactory), wic.put_void()));
    winrt::com_ptr<IWICBitmap> bmp;
    winrt::check_hresult(wic->CreateBitmapFromMemory(w, h, GUID_WICPixelFormat32bppPBGRA, w * 4, w * h * 4,
                                                     (BYTE*)bgra, bmp.put()));
    winrt::com_ptr<IWICBitmapSource> conv;
    winrt::check_hresult(WICConvertBitmapSource(GUID_WICPixelFormat32bppBGRA, bmp.as<IWICBitmapSource>().get(), conv.put()));
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
    winrt::check_hresult(frame->WriteSource(conv.get(), nullptr));
    winrt::check_hresult(frame->Commit());
    winrt::check_hresult(enc->Commit());
}

static std::atomic<int> g_snapsWanted{0};
static std::atomic<int> g_scrollTo{-1};  // cmd-scroll: move the curtain like a swipe would
static std::atomic<bool> g_invoke{false};  // cmd-invoke: dismiss the curtain the way a key press does
static std::atomic<bool> g_stopping{false}, g_released{false};
struct Snap {
    wuxmi::RenderTargetBitmap rtb{nullptr};
    wf::IAsyncAction render{nullptr};
    wf::IAsyncOperation<winrt::Windows::Storage::Streams::IBuffer> pixels{nullptr};
    double started = 0;
};
static Snap* g_snap = nullptr;
static int g_snapIndex = 0;
static bool g_treeDumped = false, g_dryHosted = false;
static std::string g_lastKey;
static int g_lastDisplay = -1;

static void DumpTree(const wux::DependencyObject& node, FILE* f, int depth) {
    if (!node || depth > 40) return;
    if (g_lock && g_lock->clock && node == g_lock->clock) {
        fwprintf(f, L"%*ls[our clock]\n", depth * 2, L"");
        return;
    }
    std::wstring cls(winrt::get_class_name(node));
    std::wstring name;
    double w = 0, h = 0;
    int vis = 0;
    if (auto fe = node.try_as<wux::FrameworkElement>()) {
        name = fe.Name();
        w = fe.ActualWidth(), h = fe.ActualHeight();
        vis = fe.Visibility() == wux::Visibility::Visible;
    }
    fwprintf(f, L"%*ls%ls '%ls' %.0fx%.0f%ls\n", depth * 2, L"", cls.c_str(), name.c_str(), w, h, vis ? L"" : L" (hidden)");
    int n = wuxm::VisualTreeHelper::GetChildrenCount(node);
    for (int i = 0; i < n; i++) DumpTree(wuxm::VisualTreeHelper::GetChild(node, i), f, depth + 1);
}

static void AfterTick() {
    if (g_stopping) {  // XAML objects belong to this thread: let go of them here
        delete g_snap;
        g_snap = nullptr;
        g_released = true;
        return;
    }
    auto content = wux::Window::Current().Content();
    if (!g_treeDumped && content && g_lock && (g_lock->host || IniGet(L"dryRun") == L"1")) {
        g_treeDumped = true;
        if (FILE* f = _wfopen((g_dir + L"lsfc-tree.txt").c_str(), L"w, ccs=UTF-8")) {
            DumpTree(content, f, 0);
            fclose(f);
        }
        if (g_lock->host) {
            auto kids = g_lock->host.Children();
            TestLog(L"host '%s' has %u children:", g_lock->host.as<wux::FrameworkElement>().Name().c_str(), kids.Size());
            for (uint32_t i = 0; i < kids.Size(); i++) {
                auto fe = kids.GetAt(i).try_as<wux::FrameworkElement>();
                TestLog(L"  %u: %s '%s'", i, winrt::get_class_name(kids.GetAt(i)).c_str(), fe ? fe.Name().c_str() : L"");
            }
        }
    }
    // dry run: no LockRootGrid in this app, so host the clock in its first panel
    if (IniGet(L"dryRun") == L"1" && !g_dryHosted && content && g_lock && !g_lock->host) {
        std::function<wuxc::Panel(const wux::DependencyObject&, int)> find = [&](const wux::DependencyObject& n, int d) -> wuxc::Panel {
            if (auto p = n.try_as<wuxc::Panel>()) return p;
            if (d > 20) return nullptr;
            int c = wuxm::VisualTreeHelper::GetChildrenCount(n);
            for (int i = 0; i < c; i++) {
                if (auto p = find(wuxm::VisualTreeHelper::GetChild(n, i), d + 1)) return p;
            }
            return nullptr;
        };
        if (auto panel = find(content, 0)) {
            g_dryHosted = true;
            g_lock->host = panel;
            g_lock->clock = ClockCreate();
            panel.Children().Append(g_lock->clock);
            TestLog(L"dry run: clock hosted in a %s", winrt::get_class_name(panel).c_str());
        }
    }
    if (int y = g_scrollTo.exchange(-1); y >= 0 && content) {
        std::function<wuxc::ScrollViewer(const wux::DependencyObject&, int)> find = [&](const wux::DependencyObject& n, int d) -> wuxc::ScrollViewer {
            if (auto sv = n.try_as<wuxc::ScrollViewer>(); sv && sv.Name() == L"LockScrollViewer") return sv;
            if (d > 30) return nullptr;
            int c = wuxm::VisualTreeHelper::GetChildrenCount(n);
            for (int i = 0; i < c; i++) {
                if (auto sv = find(wuxm::VisualTreeHelper::GetChild(n, i), d + 1)) return sv;
            }
            return nullptr;
        };
        if (auto sv = find(content, 0)) {
            bool ok = sv.ChangeView(nullptr, wf::IReference<double>((double)y), nullptr, true);
            TestLog(L"curtain scrolled to %d: %d (offset now %.0f of %.0f)", y, ok, sv.VerticalOffset(), sv.ScrollableHeight());
        } else {
            TestLog(L"no LockScrollViewer");
        }
    }
    if (g_invoke.exchange(false) && content) {
        std::function<wux::UIElement(const wux::DependencyObject&, int)> find = [&](const wux::DependencyObject& n, int d) -> wux::UIElement {
            if (auto fe = n.try_as<wux::FrameworkElement>(); fe && fe.Name() == L"InvokableLockContentControl") return fe;
            if (d > 30) return nullptr;
            int c = wuxm::VisualTreeHelper::GetChildrenCount(n);
            for (int i = 0; i < c; i++) {
                if (auto e = find(wuxm::VisualTreeHelper::GetChild(n, i), d + 1)) return e;
            }
            return nullptr;
        };
        namespace peers = winrt::Windows::UI::Xaml::Automation::Peers;
        if (auto el = find(content, 0)) {
            auto peer = peers::FrameworkElementAutomationPeer::CreatePeerForElement(el);
            auto inv = peer ? peer.GetPattern(peers::PatternInterface::Invoke)
                                  .try_as<winrt::Windows::UI::Xaml::Automation::Provider::IInvokeProvider>()
                            : nullptr;
            if (inv) {
                inv.Invoke();
                TestLog(L"curtain invoked");
            } else {
                TestLog(L"curtain has no Invoke pattern");
            }
        }
    }
    if (g_ui) {
        int ds = g_displayState.load();
        if (ds != g_lastDisplay) {
            TestLog(L"display state %d", ds);
            g_lastDisplay = ds;
        }
        if (g_ui->lastKey != g_lastKey) {
            g_lastKey = g_ui->lastKey;
            TestLog(L"time key '%S' (built %d, %.3f DIP a unit, %zu images)", g_lastKey.c_str(), g_ui->built, g_ui->k,
                    g_ui->art[g_ui->painted].size());
        }
    }
    // one screenshot at a time: render, then read the pixels, then save
    if (!g_snap && g_snapsWanted > 0 && content) {
        g_snapsWanted--;
        g_snap = new Snap();
        g_snap->rtb = wuxmi::RenderTargetBitmap();
        g_snap->render = g_snap->rtb.RenderAsync(content);
        g_snap->started = SteadyMs();
    }
    if (g_snap && g_snap->render && g_snap->render.Status() != wf::AsyncStatus::Started && !g_snap->pixels) {
        if (g_snap->render.Status() != wf::AsyncStatus::Completed) {
            TestLog(L"snapshot failed to render: 0x%08X", (unsigned)g_snap->render.ErrorCode().value);
            delete g_snap;
            g_snap = nullptr;
            return;
        }
        g_snap->pixels = g_snap->rtb.GetPixelsAsync();
    }
    if (g_snap && g_snap->pixels && g_snap->pixels.Status() != wf::AsyncStatus::Started) {
        if (g_snap->pixels.Status() == wf::AsyncStatus::Completed) {
            auto buf = g_snap->pixels.GetResults();
            wchar_t name[32];
            swprintf(name, 32, L"snap-%03d.png", g_snapIndex++);
            SavePng(g_dir + name, g_snap->rtb.PixelWidth(), g_snap->rtb.PixelHeight(), buf.data());
            TestLog(L"%s: %dx%d in %.0f ms", name, g_snap->rtb.PixelWidth(), g_snap->rtb.PixelHeight(),
                    SteadyMs() - g_snap->started);
        } else {
            TestLog(L"snapshot pixels failed");
        }
        delete g_snap;
        g_snap = nullptr;
    }
}

static HMODULE g_module;

// timeShiftSec=N in settings.ini: the clock runs N seconds ahead, to reach a
// particular time (9:59 -> 10:00) on the real lock screen.
static LONGLONG g_shift = 0;
static SYSTEMTIME ShiftedNow() {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER u{{ft.dwLowDateTime, ft.dwHighDateTime}};
    u.QuadPart += g_shift * 10000000LL;
    FILETIME utc{u.LowPart, u.HighPart}, local;
    FileTimeToLocalFileTime(&utc, &local);
    SYSTEMTIME st;
    FileTimeToSystemTime(&local, &st);
    return st;
}

static bool Take(const wchar_t* cmd, std::string* body = nullptr) {
    std::wstring p = g_dir + cmd;
    FILE* f = _wfopen(p.c_str(), L"r");
    if (!f) return false;
    if (body) {
        char buf[64] = {};
        fread(buf, 1, sizeof buf - 1, f);
        *body = buf;
    }
    fclose(f);
    DeleteFileW(p.c_str());
    return true;
}

// The mod installed through Windhawk loads as <id>_<version>_<n>.dll (local@<id>_... for a
// local build). Once it is here, this test copy steps aside: two clocks would both play.
static bool InstalledModLoaded() {
    HMODULE mods[1024];
    DWORD bytes = 0;
    if (!K32EnumProcessModules(GetCurrentProcess(), mods, sizeof mods, &bytes)) return false;
    for (DWORD i = 0; i < bytes / sizeof(HMODULE); i++) {
        WCHAR name[MAX_PATH];
        if (mods[i] != g_module && K32GetModuleBaseNameW(GetCurrentProcess(), mods[i], name, MAX_PATH) &&
            wcsstr(name, L"lock-screen-flip-clock_")) {
            return true;
        }
    }
    return false;
}

static DWORD WINAPI Control(LPVOID) {
    WCHAR tmp[MAX_PATH];
    GetTempPathW(MAX_PATH, tmp);
    g_dir = tmp;
    TestLog(L"test DLL loaded into pid %u", GetCurrentProcessId());
    LoadIni();
    if (IniGet(L"tee") != L"0") g_audioSink = TeeSink;
    if ((g_shift = _wtoi64(IniGet(L"timeShiftSec").c_str()))) g_now = ShiftedNow;
    g_afterUiTick = AfterTick;
    if (!Wh_ModInit()) TestLog(L"Wh_ModInit failed");
    for (int n = 0;; n++) {
        Sleep(150);
        if (n % 13 == 0 && InstalledModLoaded()) {
            TestLog(L"the installed mod is loaded: stepping aside");
            break;
        }
        std::string body;
        if (Take(L"cmd-snap")) g_snapsWanted++;
        if (Take(L"cmd-burst", &body)) g_snapsWanted += std::max(1, atoi(body.c_str()));
        if (Take(L"cmd-scroll", &body)) g_scrollTo = std::max(0, atoi(body.c_str()));
        if (Take(L"cmd-invoke")) g_invoke = true;
        if (Take(L"cmd-settings")) {
            LoadIni();
            TestLog(L"settings reloaded");
            Wh_ModSettingsChanged();
        }
        if (Take(L"cmd-stop")) break;
    }
    TestLog(L"stopping");
    g_stopping = true;
    for (int i = 0; i < 40 && !g_released; i++) Sleep(50);
    g_afterUiTick = nullptr;
    Wh_ModUninit();
    g_audioSink = nullptr;
    WriteTee();
    TestLog(L"unloaded");
    FreeLibraryAndExitThread(g_module, 0);
}

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_module = module;
        DisableThreadLibraryCalls(module);
        if (HANDLE t = CreateThread(nullptr, 0, Control, nullptr, 0, nullptr)) CloseHandle(t);
    }
    return TRUE;
}
