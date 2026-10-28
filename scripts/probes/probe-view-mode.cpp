// Settles two questions for explorer-justified-view against the real DefView,
// hosted in a private ExplorerBrowser (no Explorer window is touched):
//
//  1. Does DefView keep FVM_THUMBNAIL as a view mode of its own: report it
//     back, persist it per folder, and restore it on the next visit?
//  2. What does the background context menu's View submenu look like (ids,
//     radio group, verbs), and which item is checked under FVM_THUMBNAIL?
//
//   & 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe' -std=c++20 -static `
//       '-Wl,--subsystem,console' scripts/probes/probe-view-mode.cpp `
//       -o probe-view-mode.exe -lole32 -loleaut32 -lshell32 -luuid -lshlwapi -luser32
//   .\probe-view-mode.exe <empty-or-private-folder>
//
// Persistence uses a private property bag name, so the user's own folder
// views are not changed.

#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <cstdio>
#include <initializer_list>

constexpr wchar_t kBag[] = L"WindhawkJustifiedViewProbe";
#ifndef SHGVSPB_FOLDERNODEFAULTS
#define SHGVSPB_FOLDERNODEFAULTS 0x80000005 // PERUSER | PERFOLDER | NOAUTODEFAULTS
#endif

void Pump(DWORD ms) {
    const DWORD end = GetTickCount() + ms;
    MSG msg;
    while (GetTickCount() < end) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        Sleep(10);
    }
}

IExplorerBrowser* Open(HWND host, PCIDLIST_ABSOLUTE folder) {
    IExplorerBrowser* browser = nullptr;
    if (FAILED(CoCreateInstance(CLSID_ExplorerBrowser, nullptr, CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&browser)))) return nullptr;
    RECT rc{0, 0, 800, 600};
    FOLDERSETTINGS fs{FVM_DETAILS, 0};
    browser->SetOptions(EBO_NOBORDER);
    if (FAILED(browser->Initialize(host, &rc, &fs)) || FAILED(browser->SetPropertyBag(kBag)) ||
        FAILED(browser->BrowseToIDList(folder, SBSP_ABSOLUTE))) {
        browser->Release();
        return nullptr;
    }
    Pump(1500);
    return browser;
}

void Report(IExplorerBrowser* browser, const char* when) {
    IFolderView2* view = nullptr;
    if (FAILED(browser->GetCurrentView(IID_PPV_ARGS(&view)))) { printf("%s: no view\n", when); return; }
    FOLDERVIEWMODE mode = FVM_AUTO; int size = 0; UINT current = 0;
    const HRESULT a = view->GetViewModeAndIconSize(&mode, &size);
    const HRESULT b = view->GetCurrentViewMode(&current);
    printf("%s: GetViewModeAndIconSize hr=0x%08lX mode=%d size=%d | GetCurrentViewMode hr=0x%08lX mode=%u\n",
           when, a, mode, size, b, current);
    view->Release();
}

void DumpMenu(HMENU menu, IContextMenu* cm, IContextMenu3* cm3, int depth) {
    const int count = GetMenuItemCount(menu);
    for (int i = 0; i < count; ++i) {
        wchar_t text[256]{};
        MENUITEMINFOW info{sizeof(info)};
        info.fMask = MIIM_ID | MIIM_FTYPE | MIIM_STATE | MIIM_SUBMENU | MIIM_STRING;
        info.dwTypeData = text; info.cch = ARRAYSIZE(text);
        if (!GetMenuItemInfoW(menu, i, TRUE, &info)) continue;
        if (info.fType & MFT_SEPARATOR) { printf("%*s----\n", depth * 2, ""); continue; }
        wchar_t verb[128]{};
        if (!info.hSubMenu && info.wID >= 1)
            cm->GetCommandString(info.wID - 1, GCS_VERBW, nullptr, reinterpret_cast<LPSTR>(verb), ARRAYSIZE(verb));
        printf("%*s[%u] \"%ls\" type=0x%X state=0x%X%s%s verb=\"%ls\"\n", depth * 2, "", info.wID, text,
               info.fType, info.fState, (info.fType & MFT_RADIOCHECK) ? " RADIO" : "",
               (info.fState & MFS_CHECKED) ? " CHECKED" : "", verb);
        if (info.hSubMenu) {
            if (cm3) cm3->HandleMenuMsg2(WM_INITMENUPOPUP, reinterpret_cast<WPARAM>(info.hSubMenu), i, nullptr);
            DumpMenu(info.hSubMenu, cm, cm3, depth + 1);
        }
    }
}

void DumpBackgroundMenu(IExplorerBrowser* browser, const char* when) {
    IShellView* view = nullptr;
    IContextMenu* cm = nullptr;
    IContextMenu3* cm3 = nullptr;
    if (FAILED(browser->GetCurrentView(IID_PPV_ARGS(&view))) ||
        FAILED(view->GetItemObject(SVGIO_BACKGROUND, IID_PPV_ARGS(&cm)))) {
        printf("%s: no background menu\n", when);
        if (view) view->Release();
        return;
    }
    cm->QueryInterface(IID_PPV_ARGS(&cm3));
    HMENU menu = CreatePopupMenu();
    const HRESULT hr = cm->QueryContextMenu(menu, 0, 1, 0x7FFF, CMF_NORMAL);
    printf("== background menu %s (QueryContextMenu hr=0x%08lX)\n", when, hr);
    DumpMenu(menu, cm, cm3, 1);
    DestroyMenu(menu);
    if (cm3) cm3->Release();
    cm->Release();
    view->Release();
}

// 3. Can a mod keep its own per-folder value in Explorer's view-state bags?
//    probe-view-mode bag <folder> [set|clear]   (run twice: set, then read)
int Bag(PCIDLIST_ABSOLUTE folder, const wchar_t* action) {
    // Documented under shlobj_core.h, but exported by shlwapi.dll (not
    // shell32) and missing from the mingw headers/import libs.
    using Fn = HRESULT(WINAPI*)(PCIDLIST_ABSOLUTE, PCWSTR, DWORD, REFIID, void**);
    const auto get = reinterpret_cast<Fn>(reinterpret_cast<void*>(
        GetProcAddress(LoadLibraryW(L"shlwapi.dll"), "SHGetViewStatePropertyBag")));
    if (!get) { printf("SHGetViewStatePropertyBag not exported\n"); return 1; }
    IPropertyBag* bag = nullptr;
    HRESULT hr = get(folder, L"WindhawkExplorerJustifiedView",
        SHGVSPB_FOLDERNODEFAULTS, IID_PPV_ARGS(&bag));
    printf("SHGetViewStatePropertyBag hr=0x%08lX\n", hr);
    if (FAILED(hr)) return 1;
    if (action && wcscmp(action, L"set") == 0) {
        VARIANT v{}; v.vt = VT_I4; v.lVal = 1;
        printf("Write hr=0x%08lX\n", bag->Write(L"Layout", &v));
    } else if (action && wcscmp(action, L"clear") == 0) {
        VARIANT v{}; v.vt = VT_EMPTY;
        printf("Write(empty) hr=0x%08lX\n", bag->Write(L"Layout", &v));
    }
    VARIANT v{}; v.vt = VT_I4;
    hr = bag->Read(L"Layout", &v, nullptr);
    printf("Read hr=0x%08lX vt=%d value=%ld\n", hr, v.vt, v.vt == VT_I4 ? v.lVal : -1);
    bag->Release();
    return 0;
}

int wmain(int argc, wchar_t** argv) {
    if (argc < 2) { printf("usage: probe-view-mode <folder> | bag <folder> [set|clear]\n"); return 2; }
    if (FAILED(OleInitialize(nullptr))) return 1;
    PIDLIST_ABSOLUTE folder = nullptr;
    const bool bagMode = wcscmp(argv[1], L"bag") == 0 && argc >= 3;
    if (FAILED(SHParseDisplayName(argv[bagMode ? 2 : 1], nullptr, &folder, 0, nullptr))) { printf("bad folder\n"); return 1; }
    if (bagMode) {
        const int result = Bag(folder, argc >= 4 ? argv[3] : nullptr);
        CoTaskMemFree(folder);
        OleUninitialize();
        return result;
    }
    WNDCLASSW wc{}; wc.lpfnWndProc = DefWindowProcW; wc.lpszClassName = L"ProbeViewModeHost";
    wc.hInstance = GetModuleHandleW(nullptr);
    RegisterClassW(&wc);
    HWND host = CreateWindowExW(0, wc.lpszClassName, L"probe", WS_OVERLAPPEDWINDOW,
        -2000, -2000, 800, 600, nullptr, nullptr, wc.hInstance, nullptr);

    IExplorerBrowser* browser = Open(host, folder);
    if (!browser) { printf("ExplorerBrowser failed\n"); return 1; }
    Report(browser, "initial");
    DumpBackgroundMenu(browser, "initial");

    for (int size : {96, 160, 256}) {
        IFolderView2* view = nullptr;
        browser->GetCurrentView(IID_PPV_ARGS(&view));
        const HRESULT hr = view->SetViewModeAndIconSize(FVM_THUMBNAIL, size);
        view->Release();
        Pump(300);
        char label[64]; sprintf_s(label, "after Set(FVM_THUMBNAIL,%d) hr=0x%08lX", size, hr);
        Report(browser, label);
    }
    DumpBackgroundMenu(browser, "under FVM_THUMBNAIL");

    // Leave and come back in a fresh browser: does the bag restore it?
    browser->Destroy();
    browser->Release();
    Pump(300);
    browser = Open(host, folder);
    if (!browser) { printf("reopen failed\n"); return 1; }
    Report(browser, "reopened (persisted?)");

    // And with FVM_ICON at the same size, for comparison.
    IFolderView2* view = nullptr;
    browser->GetCurrentView(IID_PPV_ARGS(&view));
    view->SetViewModeAndIconSize(FVM_ICON, 256);
    view->Release();
    Pump(300);
    Report(browser, "after Set(FVM_ICON,256)");
    browser->Destroy();
    browser->Release();
    Pump(300);
    browser = Open(host, folder);
    Report(browser, "reopened after FVM_ICON");

    // Odd icon sizes as a marker: kept exactly, and persisted?
    browser->GetCurrentView(IID_PPV_ARGS(&view));
    view->SetViewModeAndIconSize(FVM_ICON, 161);
    view->Release();
    Pump(300);
    Report(browser, "after Set(FVM_ICON,161)");
    browser->Destroy();
    browser->Release();
    Pump(300);
    browser = Open(host, folder);
    Report(browser, "reopened after 161");

    // Every size native Ctrl+wheel zoom produces, from Small icons upward.
    browser->GetCurrentView(IID_PPV_ARGS(&view));
    view->SetViewModeAndIconSize(FVM_ICON, 16);
    IShellView* shellView = nullptr;
    view->QueryInterface(IID_PPV_ARGS(&shellView));
    HWND defView = nullptr;
    shellView->GetWindow(&defView);
    HWND items = FindWindowExW(defView, nullptr, L"DirectUIHWND", nullptr);
    printf("wheel target defview=%p items=%p\n", defView, items);
    printf("Ctrl+wheel sizes:");
    for (int step = 0; step < 40; ++step) {
        SendMessageW(items ? items : defView, WM_MOUSEWHEEL, MAKEWPARAM(MK_CONTROL, WHEEL_DELTA), MAKELPARAM(-1000, -1000));
        Pump(30);
        FOLDERVIEWMODE mode = FVM_AUTO; int size = 0;
        view->GetViewModeAndIconSize(&mode, &size);
        printf(" %d:%d", mode, size);
    }
    printf("\n");
    shellView->Release();
    view->Release();
    browser->Destroy();
    browser->Release();

    CoTaskMemFree(folder);
    DestroyWindow(host);
    OleUninitialize();
    return 0;
}
