// Checks overhaulded-alt-tab-fork's selection jumps, Up/Down window cycling
// and icon-row geometry against the mod's ACTUAL code, sliced out of the
// source so this cannot test a stale copy:
//
//   python - <<'EOF'
//   s = open('mods/overhaulded-alt-tab-fork/overhaulded-alt-tab-fork.wh.cpp', encoding='utf-8').read()
//   cut = lambda a, b: s[s.index(a):s.index(b, s.index(a))]
//   open('scripts/nav.inc', 'w', encoding='utf-8').write(
//       cut('// The icon row under the carousel', 'static RECT GetCloseButtonRect') +
//       cut('// Moves the selection to an app', '// Runs on the UI thread'))
//   EOF
//   & 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe' -std=c++20 -static `
//       '-Wl,--subsystem,console' -Iscripts `
//       scripts/probes/probe-alttab-nav.cpp -o scripts/nav.exe -luser32
//
// Exit code 0 means every check passed.

#include <windows.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

// Just the pieces of the mod the sliced code touches.
struct AppGroup
{
    std::vector<HWND> windows;
    HWND representativeWindow;
};
static std::vector<AppGroup> g_groups;
static int g_selected = 0;
static int g_lastSteps = 0;
static const int kSelectorWidth = 1220;
static int ScaleLayoutPx(float value) { return static_cast<int>(roundf(value)); }
static void StartSlide(int steps) { g_lastSteps = steps; }
static void UpdateSelectorControls() {}

#include "nav.inc"

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

static void SetGroupCount(int count)
{
    g_groups.assign(count, AppGroup{});
}

static int Jump(int count, int from, int to)
{
    SetGroupCount(count);
    g_selected = from;
    g_lastSteps = 999;
    JumpToGroup(to);
    return g_lastSteps;
}

int main()
{
    // Shortest way round, never more than half the ring.
    CHECK(Jump(20, 0, 1) == 1);
    CHECK(Jump(20, 0, 10) == 10);
    CHECK(Jump(20, 0, 11) == -9);
    CHECK(Jump(20, 0, 19) == -1);
    CHECK(Jump(20, 15, 0) == 5);   // End -> Home wraps forward
    CHECK(Jump(20, 3, 3) == 0);
    CHECK(Jump(2, 0, 1) == 1);
    CHECK(Jump(3, 2, 0) == 1);
    CHECK(Jump(20, 0, 20) == 999); // out of range: no slide at all
    CHECK(Jump(20, 0, -1) == 999);
    for (int count = 1; count <= 25; ++count)
        for (int from = 0; from < count; ++from)
            for (int to = 0; to < count; ++to)
            {
                int steps = Jump(count, from, to);
                CHECK(((from + steps) % count + count) % count == to);
                CHECK(2 * std::abs(steps) <= count);
            }

    // Up / Down step through the app's windows and wrap both ways.
    HWND a = reinterpret_cast<HWND>(1), b = reinterpret_cast<HWND>(2),
         c = reinterpret_cast<HWND>(3);
    g_groups = { AppGroup{ { a, b, c }, a } };
    g_selected = 0;
    CycleGroupWindow(1);
    CHECK(g_groups[0].representativeWindow == b);
    CycleGroupWindow(1);
    CycleGroupWindow(1);
    CHECK(g_groups[0].representativeWindow == a);
    CycleGroupWindow(-1);
    CHECK(g_groups[0].representativeWindow == c);
    g_groups = { AppGroup{ { a }, a } };
    CycleGroupWindow(1);
    CHECK(g_groups[0].representativeWindow == a); // single window: no-op

    // Icon row: inside the selector, centred, cells touching, hits exact.
    const int counts[] = { 1, 5, 20, 33, 60 };
    for (int count : counts)
    {
        SetGroupCount(count);
        RECT first = GetStripRect(0, true);
        RECT last = GetStripRect(count - 1, true);
        CHECK(first.left >= 47 && last.right <= kSelectorWidth - 47);
        CHECK(std::abs((first.left + last.right) - kSelectorWidth) <= 1);
        for (int i = 0; i < count; ++i)
        {
            RECT icon = GetStripRect(i, false);
            RECT cell = GetStripRect(i, true);
            CHECK(icon.left >= cell.left && icon.right <= cell.right);
            CHECK(icon.right > icon.left && icon.bottom > icon.top);
            if (i + 1 < count)
                CHECK(std::abs(GetStripRect(i + 1, true).left - cell.right) <= 1);
            POINT centre = { (icon.left + icon.right) / 2, (icon.top + icon.bottom) / 2 };
            CHECK(GetStripIndexAtPoint(centre) == i);
        }
        POINT above = { (first.left + first.right) / 2, first.top - 2 };
        CHECK(GetStripIndexAtPoint(above) == -1);
    }

    printf(g_fail ? "%d check(s) failed\n" : "all checks passed\n", g_fail);
    return g_fail ? 1 : 0;
}
