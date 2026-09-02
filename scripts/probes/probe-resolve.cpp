// Run tray-hover-expand-plus's real identification pipeline against the live
// taskbar: CollectCandidates -> ClampCandidates -> PickChevron / PickItem.
//
// This answers "would the shipped example items actually match on this machine,
// and would they match exactly one element" without installing the mod.
//
// Build:
//   & 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe' -std=c++23 -O1 -static `
//       -target x86_64-w64-mingw32 -DUNICODE -D_UNICODE -municode `
//       scripts\probes\probe-resolve.cpp -lole32 -loleaut32 -o probe-resolve.exe

#include <windows.h>

#include <cstdio>

#define WH_MOD_ID L"tray-hover-expand-plus"

static void LogLine(const wchar_t* fmt, ...);
#define Wh_Log(...) LogLine(__VA_ARGS__)

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

static void LogLine(const wchar_t* fmt, ...) {
    wchar_t buf[4096];
    va_list ap;
    va_start(ap, fmt);
    _vsnwprintf_s(buf, _TRUNCATE, fmt, ap);
    va_end(ap);
    char utf8[8192];
    int n = WideCharToMultiByte(CP_UTF8, 0, buf, -1, utf8, sizeof(utf8), nullptr, nullptr);
    if (n > 0) {
        fwrite(utf8, 1, n - 1, stdout);
        fputc('\n', stdout);
    }
}

// probe-resolve.exe open   opens the hidden-icons flyout first, walks with it
//                          open, then closes it again. That is the state that
//                          broke ClampCandidates.
int wmain(int argc, wchar_t** argv) {
    bool openFlyout = argc >= 2 && wcscmp(argv[1], L"open") == 0;
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    IUIAutomation* a = nullptr;
    if (FAILED(CoCreateInstance(__uuidof(CUIAutomation), nullptr, CLSCTX_INPROC_SERVER,
                                __uuidof(IUIAutomation), (void**)&a)) ||
        !a) {
        printf("no UIA\n");
        return 1;
    }

    Settings s;  // defaults, i.e. what the settings block ships
    for (const auto& k : s.keywords) s.keywordsLower.push_back(ToLower(k));

    // The two example items exactly as the settings block defines them, but
    // enabled, and with matchName lowercased the way LoadSettings does it.
    ItemConfig et;
    et.enabled = true;
    et.label = L"EarTrumpet";
    et.matchName = ToLower(std::wstring(L"EarTrumpet"));
    ItemConfig qs;
    qs.enabled = true;
    qs.label = L"Quick Settings";
    qs.matchAutomationId = L"SystemTrayIcon";
    qs.matchClass = L"SystemTray.OmniButtonCenter";
    s.items.push_back(et);
    s.items.push_back(qs);

    std::vector<Target> targets;
    Target chevron;
    chevron.isChevron = true;
    targets.push_back(std::move(chevron));
    for (const auto& c : s.items) {
        Target t;
        t.cfg = c;
        targets.push_back(std::move(t));
    }

    HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
    DWORD pid = 0;
    GetWindowThreadProcessId(tray, &pid);

    // Open the flyout through the mod's own code path, so the walk below sees
    // the real "flyout is open" tray, grid of icons and all.
    std::vector<Target> opener;
    if (openFlyout) {
        Target c;
        c.isChevron = true;
        opener.push_back(std::move(c));
        ResolveTargets(a, s, opener, tray, nullptr, false, nullptr, nullptr);
        if (opener[0].el) {
            DoInvoke(opener[0].el, false);
            Sleep(1500);
        } else {
            printf("could not resolve the chevron to open the flyout\n");
        }
    }

    HWND flyout = GetVisibleFlyout(s, pid);
    printf("flyout open: %s\n", flyout ? "yes" : "no");

    bool logged = false, weak = false;
    printf("--- resolve pass 1 ---\n");
    ResolveTargets(a, s, targets, tray, flyout, /*logCandidates=*/true, &logged, &weak);

    printf("--- result ---\n");
    int resolved = 0;
    for (auto& t : targets) {
        if (t.el) {
            resolved++;
            printf("  RESOLVED %-16ls rect=(%d,%d %dx%d)\n", t.Label(), (int)t.rect.left,
                   (int)t.rect.top, (int)(t.rect.right - t.rect.left),
                   (int)(t.rect.bottom - t.rect.top));
        } else {
            printf("  unresolved %ls\n", t.Label());
        }
    }

    // A second pass must not hand an already-resolved element to another target.
    printf("--- resolve pass 2 (idempotence) ---\n");
    ResolveTargets(a, s, targets, tray, flyout, false, nullptr, nullptr);
    int resolved2 = 0;
    for (auto& t : targets)
        if (t.el) resolved2++;
    printf("  resolved: pass1=%d pass2=%d %s\n", resolved, resolved2,
           resolved == resolved2 ? "ok" : "CHANGED");

    if (openFlyout && !opener.empty() && opener[0].el) {
        DoInvoke(opener[0].el, true);  // put it back
        Sleep(300);
    }
    for (auto& t : opener) t.DropElement();
    for (auto& t : targets) t.DropElement();
    a->Release();
    CoUninitialize();
    return 0;
}
