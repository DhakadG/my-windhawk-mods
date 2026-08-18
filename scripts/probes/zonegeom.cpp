// Checks the two pieces of zone geometry that are easy to get quietly wrong.
//
//  1. The sixteen dashboard zones must tile the border of the screen box with
//     no overlap and no gap, at any aspect ratio.
//  2. The edge-segment coalescing must turn neighbouring identical segments
//     into one span - AAA is one edge-wide zone, not three that each re-arm as
//     the pointer crosses a seam.
//
// Build (32-bit, matching windhawk.exe):
//   clang++ --target=i686-w64-mingw32 -std=c++20 -static \
//       -Wl,--subsystem,console scripts/zonegeom.cpp -o zonegeom.exe -luser32
#define NOMINMAX
#include <windows.h>
#include <stdio.h>
#include <vector>

enum Zone {
    ZONE_TOP_LEFT = 0, ZONE_TOP_RIGHT, ZONE_BOTTOM_LEFT, ZONE_BOTTOM_RIGHT,
    ZONE_TOP_START, ZONE_TOP_MIDDLE, ZONE_TOP_END,
    ZONE_BOTTOM_START, ZONE_BOTTOM_MIDDLE, ZONE_BOTTOM_END,
    ZONE_LEFT_START, ZONE_LEFT_MIDDLE, ZONE_LEFT_END,
    ZONE_RIGHT_START, ZONE_RIGHT_MIDDLE, ZONE_RIGHT_END,
    ZONE_COUNT = 16,
};

// --- verbatim copy of the mod's function ---
static RECT ZoneRectInDiagram(Zone z, const RECT &d)
{
    int w = d.right - d.left, h = d.bottom - d.top;
    int c = (w < h ? w : h) / 5;
    int t = c * 62 / 100;
    int cw = w * 22 / 100, ch = h * 22 / 100;
    int cx0 = d.left + w / 2 - cw / 2, cx1 = cx0 + cw;
    int cy0 = d.top + h / 2 - ch / 2, cy1 = cy0 + ch;

    switch (z)
    {
    case ZONE_TOP_LEFT:      return {d.left, d.top, d.left + c, d.top + c};
    case ZONE_TOP_RIGHT:     return {d.right - c, d.top, d.right, d.top + c};
    case ZONE_BOTTOM_LEFT:   return {d.left, d.bottom - c, d.left + c, d.bottom};
    case ZONE_BOTTOM_RIGHT:  return {d.right - c, d.bottom - c, d.right, d.bottom};
    case ZONE_TOP_START:     return {d.left + c, d.top, cx0, d.top + t};
    case ZONE_TOP_MIDDLE:    return {cx0, d.top, cx1, d.top + t};
    case ZONE_TOP_END:       return {cx1, d.top, d.right - c, d.top + t};
    case ZONE_BOTTOM_START:  return {d.left + c, d.bottom - t, cx0, d.bottom};
    case ZONE_BOTTOM_MIDDLE: return {cx0, d.bottom - t, cx1, d.bottom};
    case ZONE_BOTTOM_END:    return {cx1, d.bottom - t, d.right - c, d.bottom};
    case ZONE_LEFT_START:    return {d.left, d.top + c, d.left + t, cy0};
    case ZONE_LEFT_MIDDLE:   return {d.left, cy0, d.left + t, cy1};
    case ZONE_LEFT_END:      return {d.left, cy1, d.left + t, d.bottom - c};
    case ZONE_RIGHT_START:   return {d.right - t, d.top + c, d.right, cy0};
    case ZONE_RIGHT_MIDDLE:  return {d.right - t, cy0, d.right, cy1};
    case ZONE_RIGHT_END:     return {d.right - t, cy1, d.right, d.bottom - c};
    default:                 return {0, 0, 0, 0};
    }
}
// --- end copy ---

static int checkTiling(int w, int h, const char *label)
{
    RECT box = {0, 0, w, h};
    std::vector<RECT> pieces;
    std::vector<int> owner;
    for (int z = 0; z < ZONE_COUNT; z++) {
        RECT r = ZoneRectInDiagram((Zone)z, box);
        if (r.right > r.left && r.bottom > r.top) {
            pieces.push_back(r);
            owner.push_back(z);
        }
    }

    int bad = 0;
    for (size_t i = 0; i < pieces.size(); i++)
        if (pieces[i].left < 0 || pieces[i].top < 0 || pieces[i].right > w ||
            pieces[i].bottom > h) {
            printf("  [%s] zone %d escapes the box\n", label, owner[i]);
            bad++;
        }

    for (size_t i = 0; i < pieces.size(); i++)
        for (size_t j = i + 1; j < pieces.size(); j++) {
            RECT o;
            if (IntersectRect(&o, &pieces[i], &pieces[j])) {
                printf("  [%s] zones %d and %d overlap by %ldx%ld\n", label,
                       owner[i], owner[j], o.right - o.left, o.bottom - o.top);
                bad++;
            }
        }

    auto covered = [&](int x, int y) {
        POINT pt = {x, y};
        for (auto &r : pieces) if (PtInRect(&r, pt)) return true;
        return false;
    };
    int gaps = 0;
    for (int x = 0; x < w; x++) { if (!covered(x, 0)) gaps++; if (!covered(x, h - 1)) gaps++; }
    for (int y = 0; y < h; y++) { if (!covered(0, y)) gaps++; if (!covered(w - 1, y)) gaps++; }
    if (gaps) { printf("  [%s] %d uncovered border pixels\n", label, gaps); bad++; }

    printf("%-22s %4dx%-4d  %2zu pieces  %s\n", label, w, h, pieces.size(),
           bad ? "FAIL" : "ok");
    return bad;
}

// --- the coalescing loop from BuildZoneSet, over plain ints ---
// 0 means "not configured"; equal non-zero values mean identical config.
static std::vector<std::pair<int, int>> coalesce(const int seg[3],
                                                 const int bound[4])
{
    std::vector<std::pair<int, int>> spans;
    int i = 0;
    while (i < 3) {
        int a = seg[i];
        int j = i + 1;
        while (j < 3 && seg[j] == a) j++;
        if (a && bound[j] > bound[i]) spans.push_back({bound[i], bound[j]});
        i = j;
    }
    return spans;
}

static int checkPattern(const char *name, int a, int b, int c, int wantSpans,
                        int wantCoverStart, int wantCoverEnd)
{
    const int seg[3] = {a, b, c};
    const int bound[4] = {0, 40, 60, 100};
    auto spans = coalesce(seg, bound);

    int bad = 0;
    if ((int)spans.size() != wantSpans) {
        printf("  [%s] expected %d span(s), got %zu\n", name, wantSpans,
               spans.size());
        bad++;
    }
    if (wantSpans > 0 && !spans.empty()) {
        if (spans.front().first != wantCoverStart ||
            spans.back().second != wantCoverEnd) {
            printf("  [%s] expected cover %d..%d, got %d..%d\n", name,
                   wantCoverStart, wantCoverEnd, spans.front().first,
                   spans.back().second);
            bad++;
        }
    }
    // Spans must never overlap each other.
    for (size_t i = 1; i < spans.size(); i++)
        if (spans[i].first < spans[i - 1].second) {
            printf("  [%s] spans overlap\n", name);
            bad++;
        }

    printf("%-22s %zu span(s)   %s\n", name, spans.size(), bad ? "FAIL" : "ok");
    return bad;
}

int main()
{
    int bad = 0;

    printf("-- dashboard tiling --\n");
    bad += checkTiling(560, 315, "16:9 landscape");
    bad += checkTiling(560, 236, "21:9 ultrawide");
    bad += checkTiling(300, 300, "1:1 square");
    bad += checkTiling(169, 300, "9:16 portrait");
    bad += checkTiling(560, 350, "16:10");
    bad += checkTiling(120, 68,  "tiny");

    printf("\n-- edge segment coalescing --\n");
    bad += checkPattern("AAA -> one edge",   1, 1, 1, 1, 0, 100);
    bad += checkPattern("ABC -> three",      1, 2, 3, 3, 0, 100);
    bad += checkPattern("AAB -> two",        1, 1, 2, 2, 0, 100);
    bad += checkPattern("ABB -> two",        1, 2, 2, 2, 0, 100);
    bad += checkPattern("ABA -> three",      1, 2, 1, 3, 0, 100);
    bad += checkPattern("A_A (gap) -> two",  1, 0, 1, 2, 0, 100);
    bad += checkPattern("_B_ -> centre only",0, 2, 0, 1, 40, 60);
    bad += checkPattern("___ -> nothing",    0, 0, 0, 0, 0, 0);

    printf("\n%s\n", bad ? "BROKEN" : "all checks pass");
    return bad ? 1 : 0;
}
