// Full UI Automation tree of the Windows 11 tray, plus ElementFromPoint probes.
//
// probe-tray-uia showed a single NotifyItemIcon button whose rectangle spans the
// whole notification area; this walks the control view depth-first so the real
// per-icon structure (if any) is visible, and asks ElementFromPoint what sits
// under a given screen pixel.
//
//   probe-tray-tree.exe                 dump the tray control tree
//   probe-tray-tree.exe point X Y       also dump the ancestry of the element at X,Y
//   probe-tray-tree.exe scan Y X0 X1 S  ElementFromPoint across a row, step S

#include <windows.h>
#include <uiautomation.h>

#include <cstdio>
#include <string>

static void Out(const wchar_t* fmt, ...) {
    wchar_t buf[8192];
    va_list ap;
    va_start(ap, fmt);
    _vsnwprintf_s(buf, _TRUNCATE, fmt, ap);
    va_end(ap);
    char utf8[16384];
    int n = WideCharToMultiByte(CP_UTF8, 0, buf, -1, utf8, sizeof(utf8), nullptr, nullptr);
    if (n > 0) fwrite(utf8, 1, n - 1, stdout);
}

static std::wstring OneLine(std::wstring s) {
    for (auto& c : s)
        if (c == L'\r' || c == L'\n') c = L'|';
    if (s.size() > 90) s = s.substr(0, 90) + L"...";
    return s;
}

static void Describe(IUIAutomationElement* e, int depth, const wchar_t* tag) {
    BSTR b = nullptr;
    std::wstring cls, aid, name;
    if (SUCCEEDED(e->get_CurrentClassName(&b)) && b) {
        cls = b;
        SysFreeString(b);
    }
    b = nullptr;
    if (SUCCEEDED(e->get_CurrentAutomationId(&b)) && b) {
        aid = b;
        SysFreeString(b);
    }
    b = nullptr;
    if (SUCCEEDED(e->get_CurrentName(&b)) && b) {
        name = b;
        SysFreeString(b);
    }
    CONTROLTYPEID ct = 0;
    e->get_CurrentControlType(&ct);
    RECT r{};
    e->get_CurrentBoundingRectangle(&r);
    BOOL off = FALSE;
    e->get_CurrentIsOffscreen(&off);
    VARIANT pv;
    VariantInit(&pv);
    BOOL inv = FALSE, exp = FALSE;
    if (SUCCEEDED(e->GetCurrentPropertyValue(UIA_IsInvokePatternAvailablePropertyId, &pv))) {
        if (pv.vt == VT_BOOL) inv = pv.boolVal != VARIANT_FALSE;
        VariantClear(&pv);
    }
    VariantInit(&pv);
    if (SUCCEEDED(e->GetCurrentPropertyValue(UIA_IsExpandCollapsePatternAvailablePropertyId, &pv))) {
        if (pv.vt == VT_BOOL) exp = pv.boolVal != VARIANT_FALSE;
        VariantClear(&pv);
    }
    Out(L"%*s%s ct=%d cls=%s aid=%s off=%d inv=%d exp=%d rect=(%d,%d %dx%d) name=%s\n", depth * 2,
        L"", tag, (int)ct, cls.c_str(), aid.c_str(), (int)off, (int)inv, (int)exp, (int)r.left,
        (int)r.top, (int)(r.right - r.left), (int)(r.bottom - r.top), OneLine(name).c_str());
}

static void WalkTree(IUIAutomation* a, IUIAutomationTreeWalker* w, IUIAutomationElement* e,
                     int depth, int maxDepth) {
    Describe(e, depth, L"-");
    if (depth >= maxDepth) return;
    IUIAutomationElement* child = nullptr;
    if (FAILED(w->GetFirstChildElement(e, &child)) || !child) return;
    int guard = 0;
    while (child && guard++ < 200) {
        WalkTree(a, w, child, depth + 1, maxDepth);
        IUIAutomationElement* next = nullptr;
        if (FAILED(w->GetNextSiblingElement(child, &next))) next = nullptr;
        child->Release();
        child = next;
    }
    if (child) child->Release();
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
    IUIAutomationTreeWalker* w = nullptr;
    a->get_ControlViewWalker(&w);

    if (argc >= 4 && wcscmp(argv[1], L"point") == 0) {
        POINT p{_wtoi(argv[2]), _wtoi(argv[3])};
        IUIAutomationElement* e = nullptr;
        if (SUCCEEDED(a->ElementFromPoint(p, &e)) && e) {
            Out(L"[POINT %d,%d] ancestry (leaf first)\n", (int)p.x, (int)p.y);
            IUIAutomationElement* cur = e;
            cur->AddRef();
            for (int d = 0; cur && d < 12; d++) {
                Describe(cur, d, L"^");
                IUIAutomationElement* par = nullptr;
                if (FAILED(w->GetParentElement(cur, &par))) par = nullptr;
                cur->Release();
                cur = par;
            }
            if (cur) cur->Release();
            e->Release();
        } else {
            Out(L"ElementFromPoint failed\n");
        }
    } else if (argc >= 5 && wcscmp(argv[1], L"scan") == 0) {
        int y = _wtoi(argv[2]), x0 = _wtoi(argv[3]), x1 = _wtoi(argv[4]);
        int step = argc >= 6 ? _wtoi(argv[5]) : 10;
        if (step < 1) step = 10;
        Out(L"[SCAN] y=%d x=%d..%d step=%d\n", y, x0, x1, step);
        std::wstring last;
        for (int x = x0; x <= x1; x += step) {
            POINT p{x, y};
            IUIAutomationElement* e = nullptr;
            if (FAILED(a->ElementFromPoint(p, &e)) || !e) continue;
            BSTR b = nullptr;
            std::wstring cls, aid, name;
            if (SUCCEEDED(e->get_CurrentClassName(&b)) && b) {
                cls = b;
                SysFreeString(b);
            }
            b = nullptr;
            if (SUCCEEDED(e->get_CurrentAutomationId(&b)) && b) {
                aid = b;
                SysFreeString(b);
            }
            b = nullptr;
            if (SUCCEEDED(e->get_CurrentName(&b)) && b) {
                name = b;
                SysFreeString(b);
            }
            RECT r{};
            e->get_CurrentBoundingRectangle(&r);
            std::wstring key = cls + L"|" + aid + L"|" + OneLine(name);
            if (key != last) {
                Out(L"  x=%4d cls=%s aid=%s rect=(%d,%d %dx%d) name=%s\n", x, cls.c_str(),
                    aid.c_str(), (int)r.left, (int)r.top, (int)(r.right - r.left),
                    (int)(r.bottom - r.top), OneLine(name).c_str());
                last = key;
            }
            e->Release();
        }
    } else {
        HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
        IUIAutomationElement* root = nullptr;
        if (tray && SUCCEEDED(a->ElementFromHandle(tray, &root)) && root) {
            Out(L"[TREE] Shell_TrayWnd control view\n");
            WalkTree(a, w, root, 0, 14);
            root->Release();
        }
    }

    if (w) w->Release();
    a->Release();
    CoUninitialize();
    return 0;
}
