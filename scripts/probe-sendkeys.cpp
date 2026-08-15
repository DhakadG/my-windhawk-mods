// Runs the mod's ACTUAL SendKeys against a real window, with and without a
// physically-held modifier. Proves the 1.1.1 fix: a Ctrl-guarded zone bound to
// Snap left used to send Ctrl+Win+Left and do nothing on a single desktop.
//
// sk.inc is sliced out of the mod rather than copied, so this cannot quietly
// test a stale copy of the function. Regenerate and build it with:
//
//   $m = 'mods/win-x-hotcorners/win-x-hotcorners.wh.cpp'
//   $s = Get-Content $m
//   $a = ($s | Select-String '^static bool IsExtendedKey').LineNumber
//   $b = ($s | Select-String '^// Convenience overload').LineNumber
//   $s[($a-1)..($b-2)] -replace 'Wh_Log\(','LogStub(' | Set-Content scripts/sk.inc
//   & 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe' -std=c++20 -static `
//       '-Wl,--subsystem,console' -Iscripts scripts/probe-sendkeys.cpp `
//       -o scripts/probe-sendkeys.exe -luser32
//
// It drives real input, so it moves a window of its own for ~10 seconds.

#include <windows.h>
#include <cstdio>
#include <vector>
#include <algorithm>

static void LogStub(const wchar_t *, ...) {}

#include "sk.inc"

static void Key(WORD vk, bool down)
{
    INPUT i = {};
    i.type = INPUT_KEYBOARD;
    i.ki.wVk = vk;
    i.ki.dwFlags = (down ? 0 : KEYEVENTF_KEYUP) |
                   (IsExtendedKey(vk) ? KEYEVENTF_EXTENDEDKEY : 0);
    SendInput(1, &i, sizeof(INPUT));
}

static void Pump(DWORD ms)
{
    DWORD end = GetTickCount() + ms;
    MSG m;
    for (;;) {
        while (PeekMessageW(&m, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&m);
            DispatchMessageW(&m);
        }
        if (GetTickCount() >= end) return;
        Sleep(10);
    }
}

static HWND g_w;

static bool Snapped()
{
    RECT r;
    GetWindowRect(g_w, &r);
    return r.left != 500 || r.top != 400;
}

static void Reset()
{
    SetWindowPos(g_w, nullptr, 500, 400, 900, 600, SWP_NOZORDER);
    SetForegroundWindow(g_w);
    Pump(900);
}

static void Trial(const char *what, WORD hold, std::vector<WORD> combo)
{
    Reset();
    if (hold) { Key(hold, true); Pump(120); }
    SendKeys(combo);
    Pump(1300);
    if (hold) { Key(hold, false); Pump(200); }
    printf("  %-46s %s\n", what, Snapped() ? "acted" : "NOTHING HAPPENED");
    fflush(stdout);
}

int main()
{
    if (auto p = (BOOL(WINAPI *)(HANDLE))GetProcAddress(
            GetModuleHandleW(L"user32.dll"), "SetProcessDpiAwarenessContext"))
        p((HANDLE)-4);

    WNDCLASSW wc = {};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"ProbeSendKeysWnd";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassW(&wc);
    g_w = CreateWindowExW(0, L"ProbeSendKeysWnd", L"SendKeys probe",
                          WS_OVERLAPPEDWINDOW | WS_VISIBLE, 500, 400, 900, 600,
                          nullptr, nullptr, wc.hInstance, nullptr);
    SetForegroundWindow(g_w);
    Pump(700);

    printf("\nSnap left  (Win+Left)\n");
    Trial("nothing held", 0, {VK_LWIN, VK_LEFT});
    Trial("Ctrl held   - the modifier-zone case", VK_LCONTROL, {VK_LWIN, VK_LEFT});
    Trial("Shift held  - user's own key, unrelated", VK_LSHIFT, {VK_LWIN, VK_LEFT});
    Trial("Alt held", VK_LMENU, {VK_LWIN, VK_LEFT});

    printf("\nA combo that contains the held key itself must still work\n");
    Reset();
    Key(VK_LWIN, true);
    Pump(120);
    SendKeys({VK_LWIN, VK_LEFT});
    Pump(1300);
    Key(VK_LWIN, false);
    Pump(200);
    printf("  %-46s %s\n", "Win held, combo is Win+Left",
           Snapped() ? "acted" : "NOTHING HAPPENED");

    printf("\nStuck-key check after all of the above\n");
    const struct { WORD vk; const char *n; } m[] = {
        {VK_CONTROL, "Ctrl"}, {VK_MENU, "Alt"},
        {VK_SHIFT, "Shift"}, {VK_LWIN, "Win"}};
    bool any = false;
    for (auto &e : m)
        if (GetAsyncKeyState(e.vk) & 0x8000) { printf("  %s STUCK\n", e.n); any = true; }
    if (!any) printf("  all modifiers up\n");

    DestroyWindow(g_w);
    return 0;
}
