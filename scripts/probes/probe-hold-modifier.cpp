// Checks the 1.4.0 key parsing and Hold modifier key against the mod's ACTUAL
// code, sliced out of the source so this cannot test a stale copy:
//
//   python - <<'EOF'
//   s = open('mods/win-x-hotcorners/win-x-hotcorners.wh.cpp', encoding='utf-8').read()
//   cut = lambda a, b: s[s.index(a):s.index(b, s.index(a))]
//   open('scripts/hm.inc', 'w', encoding='utf-8').write(
//       cut('static std::wstring TrimStr', '// =====') +
//       cut('static const std::unordered_map<std::wstring, UINT> g_modifierMap',
//           '// =====') +
//       cut('static bool IsExtendedKey', '// Convenience overload') +
//       cut('// Hold modifier key: the keys go down', '// =====')
//       .replace('static void SendKeys', 'static void SendKeysUnused'))
//   EOF
//   & 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe' -std=c++20 -static `
//       '-Wl,--subsystem,console' -DWh_Log=LogStub -Iscripts `
//       scripts/probes/probe-hold-modifier.cpp -o scripts/hm.exe -luser32
//
// The live half presses and releases real keys (Shift, then Win), so run it
// with nothing important focused. Exit code 0 means every check passed.

#include <windows.h>

#include <algorithm>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

static void LogStub(const wchar_t *, ...) {}

#include "hm.inc"

static int g_fail = 0;
#define CHECK(c)                                                               \
    do                                                                         \
    {                                                                          \
        if (!(c))                                                              \
        {                                                                      \
            printf("FAIL line %d: %s\n", __LINE__, #c);                        \
            g_fail++;                                                          \
        }                                                                      \
    } while (0)

using V = std::vector<WORD>;

static bool Down(int vk)
{
    Sleep(80);   // let the raw input thread apply the injected events
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

int main()
{
    // Every key of a combination survives, in order.
    auto c = ParseKeyCombo(L"Ctrl+A+B");
    CHECK(c.size() == 1 && c[0] == (V{VK_LCONTROL, 'A', 'B'}));
    // A typo drops the whole combination instead of sending a bare Win.
    CHECK(ParseKeyCombo(L"Win+PgUpp").empty());
    // ...and only that combination.
    c = ParseKeyCombo(L"Ctrl+C;Bad+X;Alt+Tab");
    CHECK(c.size() == 2 && c[1] == (V{VK_LMENU, VK_TAB}));

    CHECK(ParseHeldModifiers(L"Win") == V{VK_LWIN});
    CHECK(ParseHeldModifiers(L" ctrl + shift ") == (V{VK_LCONTROL, VK_LSHIFT}));
    CHECK(ParseHeldModifiers(L"RCtrl+LWin") == (V{VK_RCONTROL, VK_LWIN}));
    CHECK(ParseHeldModifiers(L"Ctrl+A").empty());   // not only modifiers
    CHECK(ParseHeldModifiers(L"Win;Ctrl").empty()); // two combinations
    CHECK(ParseHeldModifiers(L"").empty());

    // Live: the press holds, the release lets go.
    SendModifierState({VK_LSHIFT}, true);
    CHECK(Down(VK_LSHIFT));
    SendModifierState({VK_LSHIFT}, false);
    CHECK(!Down(VK_LSHIFT));

    // Win pressed and released with nothing between must not open Start.
    HWND before = GetForegroundWindow();
    SendModifierState({VK_LWIN}, true);
    CHECK(Down(VK_LWIN));
    SendModifierState({VK_LWIN}, false);
    CHECK(!Down(VK_LWIN));
    Sleep(600);
    wchar_t cls[128] = L"";
    HWND fg = GetForegroundWindow();
    GetClassNameW(fg, cls, 128);
    bool start = fg != before && wcscmp(cls, L"Windows.UI.Core.CoreWindow") == 0;
    CHECK(!start);
    if (start)
        keybd_event(VK_ESCAPE, 0, 0, 0), keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);

    printf(g_fail ? "%d check(s) failed\n" : "all checks passed\n", g_fail);
    return g_fail ? 1 : 0;
}
