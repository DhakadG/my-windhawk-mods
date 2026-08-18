// Where does a 900-escapement TextOutW actually put its glyphs relative to the
// baseline point? Renders the mod's exact call plus candidates into a BMP so
// the answer is looked at rather than deduced.

#include <windows.h>
#include <cstdio>

static void Save(HBITMAP bmp, int w, int h, const char *path)
{
    BITMAPFILEHEADER fh = {};
    BITMAPINFOHEADER ih = {};
    ih.biSize = sizeof(ih);
    ih.biWidth = w;
    ih.biHeight = -h;
    ih.biPlanes = 1;
    ih.biBitCount = 24;
    ih.biCompression = BI_RGB;
    int stride = ((w * 3 + 3) / 4) * 4;
    int sz = stride * h;
    BYTE *bits = new BYTE[sz];
    HDC dc = GetDC(nullptr);
    GetDIBits(dc, bmp, 0, h, bits, (BITMAPINFO *)&ih, DIB_RGB_COLORS);
    ReleaseDC(nullptr, dc);
    fh.bfType = 0x4D42;
    fh.bfOffBits = sizeof(fh) + sizeof(ih);
    fh.bfSize = fh.bfOffBits + sz;
    FILE *f = fopen(path, "wb");
    fwrite(&fh, sizeof(fh), 1, f);
    fwrite(&ih, sizeof(ih), 1, f);
    fwrite(bits, sz, 1, f);
    fclose(f);
    delete[] bits;
}

int main()
{
    const int W = 560, H = 340;
    HDC screen = GetDC(nullptr);
    HDC dc = CreateCompatibleDC(screen);
    HBITMAP bmp = CreateCompatibleBitmap(screen, W, H);
    SelectObject(dc, bmp);
    ReleaseDC(nullptr, screen);

    HBRUSH bg = CreateSolidBrush(RGB(40, 40, 40));
    RECT all = {0, 0, W, H};
    FillRect(dc, &all, bg);

    LOGFONTW lf = {};
    lf.lfHeight = -18;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfQuality = CLEARTYPE_QUALITY;
    lf.lfEscapement = 900;
    lf.lfOrientation = 900;
    wcscpy_s(lf.lfFaceName, LF_FACESIZE, L"Segoe UI");
    HFONT f = CreateFontIndirectW(&lf);
    SelectObject(dc, f);
    SetBkMode(dc, TRANSPARENT);

    const wchar_t *txt = L"Snap Left";
    SIZE ts = {};
    GetTextExtentPoint32W(dc, txt, 9, &ts);
    printf("extent: cx=%ld cy=%ld   (cx = along baseline, cy = line height)\n",
           ts.cx, ts.cy);

    HBRUSH zone = CreateSolidBrush(RGB(80, 190, 250));

    struct { int ox; const char *tag; int dx, dy; } cases[] = {
        {40,  "current: cx+cy/2, cy+cx/2",  +ts.cy / 2, +ts.cx / 2},
        {220, "cx-cy/2, cy+cx/2",           -ts.cy / 2, +ts.cx / 2},
        {400, "cx-cy/2, cy-cx/2",           -ts.cy / 2, -ts.cx / 2},
    };

    for (auto &c : cases) {
        // A band the shape of the left-edge strip: narrow and tall.
        RECT z = {c.ox, 60, c.ox + 46, 280};
        FillRect(dc, &z, zone);
        int cx = (z.left + z.right) / 2;
        int cy = (z.top + z.bottom) / 2;
        SetTextColor(dc, RGB(0, 0, 0));
        TextOutW(dc, cx + c.dx, cy + c.dy, txt, 9);
        printf("  band x=%d..%d  centre cx=%d  baseline x=%d  %s\n",
               (int)z.left, (int)z.right, cx, cx + c.dx, c.tag);
    }

    Save(bmp, W, H, "vtext.bmp");
    printf("wrote vtext.bmp\n");
    return 0;
}
