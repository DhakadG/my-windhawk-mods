// Dump the live UI Automation view of the Windows 11 tray, plus every visible
// top-level window, so that tray-hover-expand-plus can be written against what
// this build actually exposes instead of against remembered class names.
//
// Build (Windhawk's own clang):
//   & 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe' -std=c++23 -O2 `
//       -target x86_64-w64-mingw32 -DUNICODE -D_UNICODE `
//       scripts\probes\probe-tray-uia.cpp -lole32 -loleaut32 -luuid -lpsapi `
//       -o probe-tray-uia.exe
//
//   probe-tray-uia.exe                 dump tray + windows
//   probe-tray-uia.exe invoke <index>  invoke tray candidate <index>, then diff windows
//
// Output is UTF-8 on stdout; redirect to a file and read it as UTF-8.

#include <windows.h>
#include <uiautomation.h>
#include <psapi.h>

#include <cstdio>
#include <string>
#include <vector>

static void Out(const wchar_t* fmt, ...) {
    wchar_t buf[4096];
    va_list ap;
    va_start(ap, fmt);
    _vsnwprintf_s(buf, _TRUNCATE, fmt, ap);
    va_end(ap);
    char utf8[8192];
    int n = WideCharToMultiByte(CP_UTF8, 0, buf, -1, utf8, sizeof(utf8), nullptr, nullptr);
    if (n > 0) fwrite(utf8, 1, n - 1, stdout);
}

struct Cand {
    std::wstring cls, aid, name;
    RECT r{};
    BOOL offscreen = FALSE;
    IUIAutomationElement* el = nullptr;
};

static std::wstring BStr(BSTR b) {
    std::wstring s = b ? b : L"";
    if (b) SysFreeString(b);
    return s;
}

static std::vector<Cand> Walk(IUIAutomation* a, HWND hwnd, const wchar_t* label) {
    std::vector<Cand> out;
    IUIAutomationElement* root = nullptr;
    if (!hwnd || FAILED(a->ElementFromHandle(hwnd, &root)) || !root) {
        Out(L"[%s] no root (hwnd=%p)\n", label, hwnd);
        return out;
    }

    IUIAutomationCondition* cond = nullptr;
    VARIANT v;
    VariantInit(&v);
    v.vt = VT_I4;
    v.lVal = UIA_ButtonControlTypeId;
    a->CreatePropertyCondition(UIA_ControlTypePropertyId, v, &cond);

    IUIAutomationCacheRequest* cache = nullptr;
    if (SUCCEEDED(a->CreateCacheRequest(&cache)) && cache) {
        cache->AddProperty(UIA_ClassNamePropertyId);
        cache->AddProperty(UIA_AutomationIdPropertyId);
        cache->AddProperty(UIA_NamePropertyId);
        cache->AddProperty(UIA_IsOffscreenPropertyId);
        cache->AddProperty(UIA_BoundingRectanglePropertyId);
        cache->AddProperty(UIA_IsExpandCollapsePatternAvailablePropertyId);
        cache->AddProperty(UIA_IsInvokePatternAvailablePropertyId);
    }

    IUIAutomationElementArray* arr = nullptr;
    if (cond && SUCCEEDED(root->FindAllBuildCache(TreeScope_Subtree, cond, cache, &arr)) && arr) {
        int n = 0;
        arr->get_Length(&n);
        Out(L"[%s] %d button elements\n", label, n);
        for (int i = 0; i < n; i++) {
            IUIAutomationElement* e = nullptr;
            if (FAILED(arr->GetElement(i, &e)) || !e) continue;
            Cand c;
            BSTR b = nullptr;
            e->get_CachedClassName(&b);
            c.cls = BStr(b);
            b = nullptr;
            e->get_CachedAutomationId(&b);
            c.aid = BStr(b);
            b = nullptr;
            e->get_CachedName(&b);
            c.name = BStr(b);
            e->get_CachedIsOffscreen(&c.offscreen);
            e->get_CachedBoundingRectangle(&c.r);
            BOOL exp = FALSE, inv = FALSE;
            VARIANT pv;
            VariantInit(&pv);
            if (SUCCEEDED(e->GetCachedPropertyValue(
                    UIA_IsExpandCollapsePatternAvailablePropertyId, &pv))) {
                if (pv.vt == VT_BOOL) exp = (pv.boolVal != VARIANT_FALSE);
                VariantClear(&pv);
            }
            VariantInit(&pv);
            if (SUCCEEDED(e->GetCachedPropertyValue(UIA_IsInvokePatternAvailablePropertyId, &pv))) {
                if (pv.vt == VT_BOOL) inv = (pv.boolVal != VARIANT_FALSE);
                VariantClear(&pv);
            }
            c.el = e;
            Out(L"  [%02d] cls=%-28s aid=%-22s off=%d rect=(%d,%d,%d,%d) exp=%d inv=%d name=%s\n",
                (int)out.size(), c.cls.c_str(), c.aid.c_str(), (int)c.offscreen,
                (int)c.r.left, (int)c.r.top, (int)c.r.right, (int)c.r.bottom,
                (int)exp, (int)inv, c.name.c_str());
            out.push_back(c);
        }
        arr->Release();
    } else {
        Out(L"[%s] FindAll failed\n", label);
    }
    if (cache) cache->Release();
    if (cond) cond->Release();
    root->Release();
    return out;
}

struct Win {
    HWND h;
    std::wstring cls, title, exe;
    RECT r;
    DWORD pid, style, exstyle;
};

static std::wstring ExeOf(DWORD pid) {
    HANDLE p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!p) return L"?";
    wchar_t buf[MAX_PATH] = L"";
    DWORD sz = MAX_PATH;
    if (!QueryFullProcessImageNameW(p, 0, buf, &sz)) buf[0] = 0;
    CloseHandle(p);
    std::wstring s = buf;
    size_t k = s.find_last_of(L'\\');
    return k == std::wstring::npos ? s : s.substr(k + 1);
}

static BOOL CALLBACK EnumProc(HWND h, LPARAM lp) {
    auto* v = (std::vector<Win>*)lp;
    if (!IsWindowVisible(h)) return TRUE;
    RECT r;
    if (!GetWindowRect(h, &r)) return TRUE;
    if (r.right <= r.left || r.bottom <= r.top) return TRUE;
    Win w{};
    w.h = h;
    w.r = r;
    wchar_t cls[128] = L"";
    GetClassNameW(h, cls, 128);
    w.cls = cls;
    wchar_t t[256] = L"";
    GetWindowTextW(h, t, 256);
    w.title = t;
    GetWindowThreadProcessId(h, &w.pid);
    w.exe = ExeOf(w.pid);
    w.style = (DWORD)GetWindowLongPtrW(h, GWL_STYLE);
    w.exstyle = (DWORD)GetWindowLongPtrW(h, GWL_EXSTYLE);
    v->push_back(w);
    return TRUE;
}

static std::vector<Win> Windows() {
    std::vector<Win> v;
    EnumWindows(EnumProc, (LPARAM)&v);
    return v;
}

static void PrintWin(const Win& w, const wchar_t* tag) {
    Out(L"  %s hwnd=%p pid=%lu exe=%-22s cls=%-34s rect=(%d,%d,%d,%d) style=%08x ex=%08x title=%s\n",
        tag, w.h, w.pid, w.exe.c_str(), w.cls.c_str(),
        (int)w.r.left, (int)w.r.top, (int)w.r.right, (int)w.r.bottom,
        w.style, w.exstyle, w.title.c_str());
}

int wmain(int argc, wchar_t** argv) {
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    IUIAutomation* a = nullptr;
    if (FAILED(CoCreateInstance(__uuidof(CUIAutomation), nullptr, CLSCTX_INPROC_SERVER,
                                __uuidof(IUIAutomation), (void**)&a)) ||
        !a) {
        Out(L"no UIA\n");
        return 1;
    }

    HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
    DWORD trayPid = 0;
    GetWindowThreadProcessId(tray, &trayPid);
    Out(L"Shell_TrayWnd=%p pid=%lu\n", tray, trayPid);

    HWND sec = FindWindowW(L"Shell_SecondaryTrayWnd", nullptr);
    Out(L"Shell_SecondaryTrayWnd=%p\n", sec);

    auto tray_cands = Walk(a, tray, L"TRAY");

    HWND fly = nullptr;
    while ((fly = FindWindowExW(nullptr, fly, L"TopLevelWindowForOverflowXamlIsland", nullptr))) {
        if (IsWindowVisible(fly)) break;
    }
    Out(L"overflow flyout visible=%p\n", fly);
    std::vector<Cand> fly_cands;
    if (fly) fly_cands = Walk(a, fly, L"FLYOUT");

    Out(L"\n[WINDOWS] visible top-level\n");
    auto before = Windows();
    for (auto& w : before) PrintWin(w, L" ");

    if (argc >= 3 && wcscmp(argv[1], L"invoke") == 0) {
        int idx = _wtoi(argv[2]);
        std::vector<Cand>* src = &tray_cands;
        if (argc >= 4 && wcscmp(argv[3], L"flyout") == 0) src = &fly_cands;
        if (idx < 0 || idx >= (int)src->size()) {
            Out(L"bad index\n");
            return 1;
        }
        Cand& c = (*src)[idx];
        HWND fgBefore = GetForegroundWindow();
        Out(L"\n[INVOKE] [%d] name=%s cls=%s  fgBefore=%p\n", idx, c.name.c_str(), c.cls.c_str(),
            fgBefore);

        IUIAutomationInvokePattern* inv = nullptr;
        IUIAutomationExpandCollapsePattern* exp = nullptr;
        if (SUCCEEDED(c.el->GetCurrentPatternAs(UIA_ExpandCollapsePatternId,
                                                __uuidof(IUIAutomationExpandCollapsePattern),
                                                (void**)&exp)) &&
            exp) {
            Out(L"  using ExpandCollapse\n");
            exp->Expand();
            exp->Release();
        } else if (SUCCEEDED(c.el->GetCurrentPatternAs(UIA_InvokePatternId,
                                                       __uuidof(IUIAutomationInvokePattern),
                                                       (void**)&inv)) &&
                   inv) {
            Out(L"  using Invoke\n");
            inv->Invoke();
            inv->Release();
        } else {
            Out(L"  no pattern\n");
        }

        for (int step = 1; step <= 8; step++) {
            Sleep(250);
            auto after = Windows();
            Out(L"\n[T+%dms] fg=%p\n", step * 250, GetForegroundWindow());
            for (auto& w : after) {
                bool seen = false;
                for (auto& b : before)
                    if (b.h == w.h) {
                        seen = true;
                        break;
                    }
                if (!seen) PrintWin(w, L"NEW");
            }
        }
    }

    for (auto& c : tray_cands)
        if (c.el) c.el->Release();
    for (auto& c : fly_cands)
        if (c.el) c.el->Release();
    a->Release();
    CoUninitialize();
    return 0;
}
