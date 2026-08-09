// Checks the dashboard's zone geometry: the twelve zones must tile the border
// of the screen box with no overlap and no gap, at any aspect ratio.
#define NOMINMAX
#include <windows.h>
#include <stdio.h>
#include <vector>

enum Zone {
    ZONE_TOP_LEFT = 0, ZONE_TOP_RIGHT, ZONE_BOTTOM_LEFT, ZONE_BOTTOM_RIGHT,
    ZONE_EDGE_TOP, ZONE_EDGE_BOTTOM, ZONE_EDGE_LEFT, ZONE_EDGE_RIGHT,
    ZONE_CENTER_TOP, ZONE_CENTER_BOTTOM, ZONE_CENTER_LEFT, ZONE_CENTER_RIGHT,
    ZONE_COUNT = 12,
};

// --- verbatim copy of the mod's function ---
static RECT ZoneRectInDiagram(Zone z, const RECT &d, bool secondHalf)
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
    case ZONE_EDGE_TOP:
        return secondHalf ? RECT{cx1, d.top, d.right - c, d.top + t}
                          : RECT{d.left + c, d.top, cx0, d.top + t};
    case ZONE_EDGE_BOTTOM:
        return secondHalf ? RECT{cx1, d.bottom - t, d.right - c, d.bottom}
                          : RECT{d.left + c, d.bottom - t, cx0, d.bottom};
    case ZONE_EDGE_LEFT:
        return secondHalf ? RECT{d.left, cy1, d.left + t, d.bottom - c}
                          : RECT{d.left, d.top + c, d.left + t, cy0};
    case ZONE_EDGE_RIGHT:
        return secondHalf ? RECT{d.right - t, cy1, d.right, d.bottom - c}
                          : RECT{d.right - t, d.top + c, d.right, cy0};
    case ZONE_CENTER_TOP:    return {cx0, d.top, cx1, d.top + t};
    case ZONE_CENTER_BOTTOM: return {cx0, d.bottom - t, cx1, d.bottom};
    case ZONE_CENTER_LEFT:   return {d.left, cy0, d.left + t, cy1};
    case ZONE_CENTER_RIGHT:  return {d.right - t, cy0, d.right, cy1};
    default:                 return {0, 0, 0, 0};
    }
}
static bool ZoneHasTwoParts(Zone z) {
    return z == ZONE_EDGE_TOP || z == ZONE_EDGE_BOTTOM ||
           z == ZONE_EDGE_LEFT || z == ZONE_EDGE_RIGHT;
}
// --- end copy ---

struct Piece { RECT r; int zone; };

static int check(int w, int h, const char *label)
{
    RECT box = {0, 0, w, h};
    std::vector<Piece> pieces;
    for (int z = 0; z < ZONE_COUNT; z++) {
        int parts = ZoneHasTwoParts((Zone)z) ? 2 : 1;
        for (int p = 0; p < parts; p++) {
            RECT r = ZoneRectInDiagram((Zone)z, box, p == 1);
            if (r.right > r.left && r.bottom > r.top) pieces.push_back({r, z});
        }
    }

    int bad = 0;

    // 1. Nothing escapes the screen box.
    for (auto &p : pieces)
        if (p.r.left < 0 || p.r.top < 0 || p.r.right > w || p.r.bottom > h) {
            printf("  [%s] zone %d escapes the box: (%ld,%ld)-(%ld,%ld)\n",
                   label, p.zone, p.r.left, p.r.top, p.r.right, p.r.bottom);
            bad++;
        }

    // 2. No two pieces overlap.
    for (size_t i = 0; i < pieces.size(); i++)
        for (size_t j = i + 1; j < pieces.size(); j++) {
            RECT o;
            if (IntersectRect(&o, &pieces[i].r, &pieces[j].r)) {
                printf("  [%s] zones %d and %d overlap by %ldx%ld\n", label,
                       pieces[i].zone, pieces[j].zone, o.right - o.left,
                       o.bottom - o.top);
                bad++;
            }
        }

    // 3. The border ring is fully covered: walk the top/bottom rows and the
    //    left/right columns and make sure every pixel belongs to some zone.
    auto covered = [&](int x, int y) {
        POINT pt = {x, y};
        for (auto &p : pieces) if (PtInRect(&p.r, pt)) return true;
        return false;
    };
    int gaps = 0;
    for (int x = 0; x < w; x++) {
        if (!covered(x, 0)) gaps++;
        if (!covered(x, h - 1)) gaps++;
    }
    for (int y = 0; y < h; y++) {
        if (!covered(0, y)) gaps++;
        if (!covered(w - 1, y)) gaps++;
    }
    if (gaps) {
        printf("  [%s] %d uncovered pixels on the border ring\n", label, gaps);
        bad++;
    }

    printf("%-22s %4dx%-4d  %2zu pieces  %s\n", label, w, h, pieces.size(),
           bad ? "FAIL" : "ok");
    return bad;
}

int main()
{
    int bad = 0;
    bad += check(560, 315, "16:9 landscape");
    bad += check(560, 236, "21:9 ultrawide");
    bad += check(300, 300, "1:1 square");
    bad += check(169, 300, "9:16 portrait");
    bad += check(560, 350, "16:10");
    bad += check(120, 68,  "tiny");
    printf("\n%s\n", bad ? "GEOMETRY BROKEN" : "all aspect ratios tile cleanly");
    return bad ? 1 : 0;
}
