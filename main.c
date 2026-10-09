/*
 * IDWatermark - 身份证（证件照片）平铺文字水印工具
 * 纯 C + Win32 API + GDI+ flat API（Windows 自带），无 C++、无第三方库。
 *
 * 构建: 见 Makefile (MinGW-w64) 或 build.bat (MSVC / MinGW)
 * 命令行: IDWatermark.exe [输入图片]                  打开图片
 *         IDWatermark.exe 输入图片 -o 输出.png|jpg|bmp  按已保存的设置直接加水印并保存（无界面）
 */
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#ifdef _MSC_VER
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "gdiplus.lib")
#endif

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0
#endif

/* ======================================================================
 * GDI+ flat API —— 手工声明，避免 C++ 头文件，MSVC / MinGW 均可用纯 C 编译
 * ====================================================================== */
typedef int   GpStatus;
typedef float GpReal;
typedef DWORD GpARGB;
typedef void  GpImage;
typedef void  GpBitmap;
typedef void  GpGraphics;
typedef void  GpBrush;
typedef void  GpFont;
typedef void  GpFontFamily;
typedef void  GpStringFormat;
typedef struct { GpReal X, Y, Width, Height; } GpRectF;
typedef struct {
    UINT32 GdiplusVersion;
    void  *DebugEventCallback;
    BOOL   SuppressBackgroundThread;
    BOOL   SuppressExternalCodecs;
} GpStartupInput;
typedef struct {
    CLSID Clsid; GUID FormatID;
    const WCHAR *CodecName, *DllName, *FormatDescription, *FilenameExtension, *MimeType;
    DWORD Flags, Version, SigCount, SigSize;
    const BYTE *SigPattern, *SigMask;
} GpImageCodecInfo;
typedef struct { GUID Guid; ULONG NumberOfValues; ULONG Type; void *Value; } GpEncoderParameter;
typedef struct { UINT Count; GpEncoderParameter Parameter[1]; } GpEncoderParameters;
typedef struct { ULONG id; ULONG length; WORD type; void *value; } GpPropertyItem;

#define GP_OK                       0
#define GP_PIXFMT_24RGB             0x00021808
#define GP_PIXFMT_32ARGB            0x0026200A
#define GP_SMOOTHING_ANTIALIAS      4
#define GP_TEXTHINT_ANTIALIAS       4
#define GP_COMPOSITING_SOURCECOPY   1
#define GP_INTERP_HQBICUBIC         7
#define GP_MATRIX_PREPEND           0
#define GP_UNIT_PIXEL               2
#define GP_FONTSTYLE_BOLD           1
#define GP_FONTSTYLE_ITALIC         2
#define GP_SFF_NOWRAP               0x00001000
#define GP_ENCVAL_LONG              4
#define GP_PROPTAG_ORIENTATION      0x0112

GpStatus WINAPI GdiplusStartup(ULONG_PTR *token, const GpStartupInput *input, void *output);
VOID     WINAPI GdiplusShutdown(ULONG_PTR token);
GpStatus WINAPI GdipCreateBitmapFromFile(const WCHAR *file, GpBitmap **bmp);
GpStatus WINAPI GdipCreateBitmapFromScan0(INT w, INT h, INT stride, INT fmt, BYTE *scan0, GpBitmap **bmp);
GpStatus WINAPI GdipCreateHBITMAPFromBitmap(GpBitmap *bmp, HBITMAP *hbm, GpARGB background);
GpStatus WINAPI GdipBitmapSetResolution(GpBitmap *bmp, GpReal xdpi, GpReal ydpi);
GpStatus WINAPI GdipGetImageHorizontalResolution(GpImage *img, GpReal *res);
GpStatus WINAPI GdipGetImageVerticalResolution(GpImage *img, GpReal *res);
GpStatus WINAPI GdipGetImageWidth(GpImage *img, UINT *w);
GpStatus WINAPI GdipGetImageHeight(GpImage *img, UINT *h);
GpStatus WINAPI GdipCloneImage(GpImage *img, GpImage **clone);
GpStatus WINAPI GdipDisposeImage(GpImage *img);
GpStatus WINAPI GdipImageRotateFlip(GpImage *img, INT type);
GpStatus WINAPI GdipGetPropertyItemSize(GpImage *img, ULONG propId, UINT *size);
GpStatus WINAPI GdipGetPropertyItem(GpImage *img, ULONG propId, UINT size, GpPropertyItem *buf);
GpStatus WINAPI GdipGetPropertyCount(GpImage *img, UINT *count);
GpStatus WINAPI GdipGetPropertySize(GpImage *img, UINT *totalSize, UINT *count);
GpStatus WINAPI GdipGetAllPropertyItems(GpImage *img, UINT totalSize, UINT count, GpPropertyItem *items);
GpStatus WINAPI GdipCreateFromHDC(HDC hdc, GpGraphics **g);
GpStatus WINAPI GdipCreatePath(INT fillMode, void **path);
GpStatus WINAPI GdipDeletePath(void *path);
GpStatus WINAPI GdipAddPathArcI(void *path, INT x, INT y, INT w, INT h, GpReal start, GpReal sweep);
GpStatus WINAPI GdipClosePathFigure(void *path);
GpStatus WINAPI GdipFillPath(GpGraphics *g, GpBrush *brush, void *path);
GpStatus WINAPI GdipGetImageGraphicsContext(GpImage *img, GpGraphics **g);
GpStatus WINAPI GdipDeleteGraphics(GpGraphics *g);
GpStatus WINAPI GdipGraphicsClear(GpGraphics *g, GpARGB color);
GpStatus WINAPI GdipSetSmoothingMode(GpGraphics *g, INT mode);
GpStatus WINAPI GdipSetTextRenderingHint(GpGraphics *g, INT hint);
GpStatus WINAPI GdipSetCompositingMode(GpGraphics *g, INT mode);
GpStatus WINAPI GdipSetInterpolationMode(GpGraphics *g, INT mode);
GpStatus WINAPI GdipTranslateWorldTransform(GpGraphics *g, GpReal dx, GpReal dy, INT order);
GpStatus WINAPI GdipRotateWorldTransform(GpGraphics *g, GpReal angle, INT order);
GpStatus WINAPI GdipDrawImageRectI(GpGraphics *g, GpImage *img, INT x, INT y, INT w, INT h);
GpStatus WINAPI GdipCreateSolidFill(GpARGB color, GpBrush **brush);
GpStatus WINAPI GdipDeleteBrush(GpBrush *brush);
GpStatus WINAPI GdipCreateFontFamilyFromName(const WCHAR *name, void *collection, GpFontFamily **family);
GpStatus WINAPI GdipDeleteFontFamily(GpFontFamily *family);
GpStatus WINAPI GdipCreateFont(const GpFontFamily *family, GpReal emSize, INT style, INT unit, GpFont **font);
GpStatus WINAPI GdipDeleteFont(GpFont *font);
GpStatus WINAPI GdipCreateStringFormat(INT flags, LANGID lang, GpStringFormat **fmt);
GpStatus WINAPI GdipDeleteStringFormat(GpStringFormat *fmt);
GpStatus WINAPI GdipMeasureString(GpGraphics *g, const WCHAR *str, INT len, const GpFont *font,
                                  const GpRectF *layout, const GpStringFormat *fmt, GpRectF *bound,
                                  INT *chars, INT *lines);
GpStatus WINAPI GdipDrawString(GpGraphics *g, const WCHAR *str, INT len, const GpFont *font,
                               const GpRectF *layout, const GpStringFormat *fmt, const GpBrush *brush);
GpStatus WINAPI GdipGetImageEncodersSize(UINT *num, UINT *size);
GpStatus WINAPI GdipGetImageEncoders(UINT num, UINT size, GpImageCodecInfo *encoders);
GpStatus WINAPI GdipSaveImageToFile(GpImage *img, const WCHAR *file, const CLSID *encoder,
                                    const GpEncoderParameters *params);

static const GUID GP_ENCODER_QUALITY =
    { 0x1d5be4b5, 0xfa4a, 0x452d, { 0x9c, 0xdd, 0x5d, 0xb3, 0x51, 0x05, 0xe7, 0xeb } };

/* ======================================================================
 * 常量 / 全局状态
 * ====================================================================== */
#define APP_TITLE     L"身份证水印工具"
#define MAIN_CLASS    L"IDWatermarkMain"
#define PREVIEW_CLASS L"IDWatermarkPreview"
#define PANEL_CLASS   L"IDWatermarkPanel"
#define INI_SECTION   L"Watermark"
#define TIMER_RENDER  1
#define IDI_APPICON   1   /* 与 app.rc 一致 */

enum {
    IDC_OPEN = 101, IDC_SAVE, IDC_FIT, IDC_ACTUAL, IDC_TEXT, IDC_FONT, IDC_COLOR, IDC_SWATCH,
    IDC_STAGGER, IDC_RESET, IDC_INFO, IDC_LBL_TEXT, IDC_LBL_FONT, IDC_LBL_COLOR, IDC_PREVIEW,
    IDC_ROTL, IDC_ROTR, IDC_PANEL,
    IDC_TRACK0 = 1100, IDC_EDIT0 = 1200, IDC_LABEL0 = 1300
};

enum { R_SIZE, R_ANGLE, R_OPACITY, R_HSPACE, R_VSPACE, R_ZOOM, NROWS };

typedef struct { const WCHAR *label; const WCHAR *key; int minv, maxv, def; } RowDef;
static const RowDef g_rowDef[NROWS] = {
    { L"字号(px)",   L"FontSize", 8,   600,  48  },
    { L"旋转角度",   L"Angle",    -90, 90,   -30 },
    { L"透明度 %",   L"Opacity",  0,   100,  35  },
    { L"水平间距",   L"HSpacing", 0,   1500, 120 },
    { L"行间距",     L"VSpacing", 0,   1500, 100 },
    { L"预览缩放 %", NULL,        5,   1600, 100 },
};

typedef struct {
    WCHAR    text[256];
    WCHAR    face[LF_FACESIZE];
    BOOL     bold, italic, stagger;
    COLORREF color;
    int      val[NROWS];
} Settings;

static Settings  g_set;
static HINSTANCE g_hInst;
static HWND      g_hwnd, g_hPreview, g_hText, g_hFontBtn, g_hColorBtn, g_hSwatch, g_hStagger, g_hInfo;
static HWND      g_hOpen, g_hSave, g_hFit, g_hActual, g_hReset, g_hLblText, g_hLblFont, g_hLblColor;
static HWND      g_hRotL, g_hRotR, g_hPanel, g_hTip;
static int       g_scrollY, g_contentH, g_wheelAcc;   /* 右侧面板滚动状态 */
#define EXIF_MAX 24
static WCHAR     g_exifLbl[EXIF_MAX][16], g_exifVal[EXIF_MAX][128];   /* Tab 叠加层显示的 EXIF 信息 */
static int       g_exifN;
static BOOL      g_showExif;
static int       g_rotation;                         /* 当前图片已顺时针旋转的 90° 次数 (0..3)，打开新图时清零 */
static HWND      g_hRowLbl[NROWS], g_hRowTrk[NROWS], g_hRowEdt[NROWS];
static HFONT     g_hUiFont;
static HBRUSH    g_hSwatchBrush;
static UINT      g_dpi = 96;
static ULONG_PTR g_gdipToken;
static GpBitmap *g_src, *g_out;          /* 原图(32bppARGB) / 加水印后的全分辨率结果 */
static HBITMAP   g_hbmView;              /* 预览用 DIB */
static int       g_imgW, g_imgH;
static WCHAR     g_path[MAX_PATH];
static WCHAR     g_iniPath[MAX_PATH];
static double    g_zoom = 1.0, g_offX, g_offY;
static BOOL      g_fitMode = TRUE;
static BOOL      g_drag;
static POINT     g_dragPt;
static double    g_dragOffX, g_dragOffY;
static BOOL      g_sync, g_renderPending, g_headless;
static COLORREF  g_custColors[16];

static int S(int v) { return MulDiv(v, (int)g_dpi, 96); }
static int ClampI(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

/* ======================================================================
 * 设置 (ini 位于 exe 同目录, UTF-16 编码以支持中文)
 * ====================================================================== */
static void DefaultSettings(Settings *s, BOOL keepText)
{
    WCHAR text[256];
    int i;
    lstrcpynW(text, s->text, 256);
    ZeroMemory(s, sizeof(*s));
    lstrcpynW(s->text, keepText ? text : L"仅供XX使用 他用无效", 256);
    lstrcpynW(s->face, L"Microsoft YaHei", LF_FACESIZE);
    s->bold = TRUE;
    s->italic = FALSE;
    s->stagger = TRUE;
    s->color = RGB(128, 128, 128);
    for (i = 0; i < NROWS; i++) s->val[i] = g_rowDef[i].def;
}

static void InitIniPath(void)
{
    WCHAR *p;
    GetModuleFileNameW(NULL, g_iniPath, MAX_PATH);
    p = wcsrchr(g_iniPath, L'\\');
    if (p) p[1] = 0; else g_iniPath[0] = 0;
    if (lstrlenW(g_iniPath) + 20 < MAX_PATH) lstrcatW(g_iniPath, L"IDWatermark.ini");
}

static void LoadSettings(void)
{
    WCHAR buf[256];
    int i;
    DefaultSettings(&g_set, FALSE);
    if (GetFileAttributesW(g_iniPath) == INVALID_FILE_ATTRIBUTES) return;
    GetPrivateProfileStringW(INI_SECTION, L"Text", g_set.text, buf, 256, g_iniPath);
    lstrcpynW(g_set.text, buf, 256);
    GetPrivateProfileStringW(INI_SECTION, L"Font", g_set.face, buf, 256, g_iniPath);
    if (buf[0]) lstrcpynW(g_set.face, buf, LF_FACESIZE);
    g_set.bold    = GetPrivateProfileIntW(INI_SECTION, L"Bold", g_set.bold, g_iniPath) != 0;
    g_set.italic  = GetPrivateProfileIntW(INI_SECTION, L"Italic", g_set.italic, g_iniPath) != 0;
    g_set.stagger = GetPrivateProfileIntW(INI_SECTION, L"Stagger", g_set.stagger, g_iniPath) != 0;
    g_set.color   = (COLORREF)GetPrivateProfileIntW(INI_SECTION, L"Color", (int)g_set.color, g_iniPath) & 0xFFFFFF;
    for (i = 0; i < NROWS; i++) {
        if (!g_rowDef[i].key) continue;
        g_set.val[i] = ClampI((int)GetPrivateProfileIntW(INI_SECTION, g_rowDef[i].key, g_rowDef[i].def, g_iniPath),
                              g_rowDef[i].minv, g_rowDef[i].maxv);
    }
    for (i = 0; i < 16; i++) {
        WCHAR key[16];
        swprintf(key, 16, L"Custom%d", i);
        g_custColors[i] = (COLORREF)GetPrivateProfileIntW(L"Colors", key, 0xFFFFFF, g_iniPath);
    }
}

static void WriteIniInt(const WCHAR *sec, const WCHAR *key, int v)
{
    WCHAR buf[32];
    swprintf(buf, 32, L"%d", v);
    WritePrivateProfileStringW(sec, key, buf, g_iniPath);
}

static void SaveSettings(void)
{
    WCHAR buf[300];
    int i;
    if (GetFileAttributesW(g_iniPath) == INVALID_FILE_ATTRIBUTES) {
        /* 先写入 UTF-16LE BOM, 之后 WritePrivateProfileStringW 会保持 Unicode */
        HANDLE h = CreateFileW(g_iniPath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h == INVALID_HANDLE_VALUE) return;
        { DWORD n; static const BYTE bom[2] = { 0xFF, 0xFE }; WriteFile(h, bom, 2, &n, NULL); }
        CloseHandle(h);
    }
    swprintf(buf, 300, L"\"%ls\"", g_set.text);
    WritePrivateProfileStringW(INI_SECTION, L"Text", buf, g_iniPath);
    WritePrivateProfileStringW(INI_SECTION, L"Font", g_set.face, g_iniPath);
    WriteIniInt(INI_SECTION, L"Bold", g_set.bold);
    WriteIniInt(INI_SECTION, L"Italic", g_set.italic);
    WriteIniInt(INI_SECTION, L"Stagger", g_set.stagger);
    WriteIniInt(INI_SECTION, L"Color", (int)g_set.color);
    for (i = 0; i < NROWS; i++)
        if (g_rowDef[i].key) WriteIniInt(INI_SECTION, g_rowDef[i].key, g_set.val[i]);
    for (i = 0; i < 16; i++) {
        WCHAR key[16];
        swprintf(key, 16, L"Custom%d", i);
        WriteIniInt(L"Colors", key, (int)g_custColors[i]);
    }
}

/* ======================================================================
 * 图像处理
 * ====================================================================== */
/* ---------------- EXIF 信息 ---------------- */
typedef struct { const GpPropertyItem *items; UINT count; } PropSet;

static const GpPropertyItem *FindProp(const PropSet *ps, ULONG id)
{
    UINT i;
    for (i = 0; i < ps->count; i++)
        if (ps->items[i].id == id && ps->items[i].value && ps->items[i].length) return &ps->items[i];
    return NULL;
}

static BOOL PropAscii(const PropSet *ps, ULONG id, WCHAR *out, int cch)
{
    const GpPropertyItem *p = FindProp(ps, id);
    char tmp[256];
    int n, k;
    if (!p || p->type != 2) return FALSE;
    n = (int)p->length < 255 ? (int)p->length : 255;
    memcpy(tmp, p->value, (size_t)n);
    tmp[n] = 0;
    n = (int)strlen(tmp);
    while (n > 0 && (tmp[n - 1] == ' ')) tmp[--n] = 0;
    if (!n) return FALSE;
    k = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, tmp, -1, out, cch);
    if (!k) k = MultiByteToWideChar(CP_ACP, 0, tmp, -1, out, cch);
    out[cch - 1] = 0;
    return k > 0;
}

static BOOL PropRational(const PropSet *ps, ULONG id, int idx, double *v)
{
    const GpPropertyItem *p = FindProp(ps, id);
    if (!p || (p->type != 5 && p->type != 10) || p->length < (ULONG)(8 * (idx + 1))) return FALSE;
    if (p->type == 5) {
        const ULONG *r = (const ULONG *)p->value + 2 * idx;
        if (!r[1]) return FALSE;
        *v = (double)r[0] / (double)r[1];
    } else {
        const LONG *r = (const LONG *)p->value + 2 * idx;
        if (!r[1]) return FALSE;
        *v = (double)r[0] / (double)r[1];
    }
    return TRUE;
}

static BOOL PropInt(const PropSet *ps, ULONG id, long *v)
{
    const GpPropertyItem *p = FindProp(ps, id);
    if (!p) return FALSE;
    if (p->type == 3 && p->length >= 2) { *v = *(const WORD *)p->value; return TRUE; }
    if ((p->type == 4 || p->type == 9) && p->length >= 4) { *v = *(const LONG *)p->value; return TRUE; }
    if (p->type == 1 && p->length >= 1) { *v = *(const BYTE *)p->value; return TRUE; }
    return FALSE;
}

static void ExifAdd(const WCHAR *lbl, const WCHAR *val)
{
    if (g_exifN >= EXIF_MAX) return;
    lstrcpynW(g_exifLbl[g_exifN], lbl, 16);
    lstrcpynW(g_exifVal[g_exifN], val, 128);
    g_exifN++;
}

static BOOL GpsCoord(const PropSet *ps, ULONG refId, ULONG valId, double *out)
{
    double d, m, s;
    WCHAR ref[8] = L"";
    if (!PropRational(ps, valId, 0, &d) || !PropRational(ps, valId, 1, &m)) return FALSE;
    if (!PropRational(ps, valId, 2, &s)) s = 0;
    *out = d + m / 60.0 + s / 3600.0;
    PropAscii(ps, refId, ref, 8);
    if (ref[0] == L'S' || ref[0] == L'W') *out = -*out;
    return TRUE;
}

/* 读取文件信息 + EXIF，填充 g_exifLbl/g_exifVal（w/h 为按 EXIF 方向校正后的尺寸） */
static void BuildExifInfo(GpImage *img, const WCHAR *path, UINT w, UINT h, GpReal dpiX, GpReal dpiY)
{
    WCHAR buf[128];
    WIN32_FILE_ATTRIBUTE_DATA fa;
    const WCHAR *name = wcsrchr(path, L'\\');
    UINT total = 0, count = 0;
    GpPropertyItem *items = NULL;
    PropSet ps = { NULL, 0 };
    double v, v2;
    long n;
    int before;

    g_exifN = 0;
    ExifAdd(L"文件名", name ? name + 1 : path);
    if (GetFileAttributesExW(path, GetFileExInfoStandard, &fa)) {
        ULONGLONG sz = ((ULONGLONG)fa.nFileSizeHigh << 32) | fa.nFileSizeLow;
        if (sz >= 1024 * 1024) swprintf(buf, 128, L"%.2f MB (%llu 字节)", sz / 1048576.0, sz);
        else swprintf(buf, 128, L"%.1f KB (%llu 字节)", sz / 1024.0, sz);
        ExifAdd(L"文件大小", buf);
    }
    swprintf(buf, 128, L"%u × %u 像素", w, h);
    ExifAdd(L"尺寸", buf);
    swprintf(buf, 128, L"%.0f × %.0f DPI", (double)dpiX, (double)dpiY);
    ExifAdd(L"分辨率", buf);

    if (GdipGetPropertyCount(img, &count) == GP_OK && count &&
        GdipGetPropertySize(img, &total, &count) == GP_OK && total) {
        items = (GpPropertyItem *)malloc(total);
        if (items && GdipGetAllPropertyItems(img, total, count, items) == GP_OK) {
            ps.items = items;
            ps.count = count;
        }
    }
    before = g_exifN;
    if (ps.count) {
        if (PropAscii(&ps, 0x010F, buf, 128)) ExifAdd(L"相机厂商", buf);
        if (PropAscii(&ps, 0x0110, buf, 128)) ExifAdd(L"相机型号", buf);
        if (PropAscii(&ps, 0x9003, buf, 128) || PropAscii(&ps, 0x0132, buf, 128)) {
            /* "YYYY:MM:DD HH:MM:SS" -> "YYYY-MM-DD HH:MM:SS" */
            if (lstrlenW(buf) >= 10 && buf[4] == L':' && buf[7] == L':') buf[4] = buf[7] = L'-';
            ExifAdd(L"拍摄时间", buf);
        }
        if (PropRational(&ps, 0x829A, 0, &v) && v > 0) {
            if (v < 1.0) swprintf(buf, 128, L"1/%.0f 秒", 1.0 / v);
            else swprintf(buf, 128, L"%.1f 秒", v);
            ExifAdd(L"曝光时间", buf);
        }
        if (PropRational(&ps, 0x829D, 0, &v) && v > 0) { swprintf(buf, 128, L"f/%.1f", v); ExifAdd(L"光圈", buf); }
        if (PropInt(&ps, 0x8827, &n) && n > 0) { swprintf(buf, 128, L"ISO %ld", n); ExifAdd(L"ISO", buf); }
        if (PropRational(&ps, 0x920A, 0, &v) && v > 0) {
            long f35;
            if (PropInt(&ps, 0xA405, &f35) && f35 > 0) swprintf(buf, 128, L"%.1f mm (等效 %ld mm)", v, f35);
            else swprintf(buf, 128, L"%.1f mm", v);
            ExifAdd(L"焦距", buf);
        }
        if (PropInt(&ps, 0x9209, &n)) ExifAdd(L"闪光灯", (n & 1) ? L"已闪光" : L"未闪光");
        if (PropInt(&ps, 0x0112, &n)) {
            static const WCHAR *ori[9] = { L"未知", L"正常", L"水平镜像", L"旋转 180°", L"垂直镜像",
                                          L"镜像 + 顺时针 90°", L"顺时针 90°", L"镜像 + 逆时针 90°", L"逆时针 90°" };
            ExifAdd(L"方向", (n >= 1 && n <= 8) ? ori[n] : ori[0]);
        }
        if (PropAscii(&ps, 0x0131, buf, 128)) ExifAdd(L"软件", buf);
        if (GpsCoord(&ps, 0x0001, 0x0002, &v) && GpsCoord(&ps, 0x0003, 0x0004, &v2)) {
            swprintf(buf, 128, L"%.6f°%lc, %.6f°%lc", fabs(v), v < 0 ? L'S' : L'N', fabs(v2), v2 < 0 ? L'W' : L'E');
            ExifAdd(L"GPS", buf);
        }
    }
    if (g_exifN == before) ExifAdd(L"EXIF", L"无 EXIF 信息");
    free(items);
}

static void ApplyExifOrientation(GpImage *img)
{
    UINT size = 0;
    GpPropertyItem *pi;
    if (GdipGetPropertyItemSize(img, GP_PROPTAG_ORIENTATION, &size) != GP_OK || size < sizeof(GpPropertyItem))
        return;
    pi = (GpPropertyItem *)malloc(size);
    if (!pi) return;
    if (GdipGetPropertyItem(img, GP_PROPTAG_ORIENTATION, size, pi) == GP_OK && pi->type == 3 && pi->value) {
        /* EXIF orientation -> RotateFlipType */
        static const int map[9] = { 0, 0, 4, 2, 6, 5, 1, 7, 3 };
        WORD o = *(WORD *)pi->value;
        if (o >= 2 && o <= 8) GdipImageRotateFlip(img, map[o]);
    }
    free(pi);
}

static BOOL GetEncoderClsid(const WCHAR *mime, CLSID *clsid)
{
    UINT num = 0, size = 0, i;
    GpImageCodecInfo *info;
    BOOL ok = FALSE;
    if (GdipGetImageEncodersSize(&num, &size) != GP_OK || size == 0) return FALSE;
    info = (GpImageCodecInfo *)malloc(size);
    if (!info) return FALSE;
    if (GdipGetImageEncoders(num, size, info) == GP_OK) {
        for (i = 0; i < num; i++) {
            if (info[i].MimeType && lstrcmpiW(info[i].MimeType, mime) == 0) {
                *clsid = info[i].Clsid;
                ok = TRUE;
                break;
            }
        }
    }
    free(info);
    return ok;
}

static GpFontFamily *CreateFamily(const WCHAR *face)
{
    static const WCHAR *fallback[] = { L"Microsoft YaHei", L"微软雅黑", L"SimHei", L"SimSun",
                                       L"Arial", L"Tahoma", L"DejaVu Sans" };
    GpFontFamily *fam = NULL;
    size_t i;
    if (face && face[0] && GdipCreateFontFamilyFromName(face, NULL, &fam) == GP_OK && fam) return fam;
    for (i = 0; i < sizeof(fallback) / sizeof(fallback[0]); i++)
        if (GdipCreateFontFamilyFromName(fallback[i], NULL, &fam) == GP_OK && fam) return fam;
    return NULL;
}

/* 在 g 上（W x H 像素的图像）平铺绘制水印 */
static void DrawWatermark(GpGraphics *g, int W, int H)
{
    GpFontFamily *fam;
    GpFont *font = NULL;
    GpStringFormat *fmt = NULL;
    GpBrush *brush = NULL;
    GpRectF layout = { 0, 0, 0, 0 }, box = { 0, 0, 0, 0 };
    int style = 0, i, j, nx, ny, alpha;
    double tw, th, cw, ch, R, lim;
    COLORREF c = g_set.color;

    if (!g_set.text[0]) return;
    fam = CreateFamily(g_set.face);
    if (!fam) return;
    if (g_set.bold) style |= GP_FONTSTYLE_BOLD;
    if (g_set.italic) style |= GP_FONTSTYLE_ITALIC;
    if (GdipCreateFont(fam, (GpReal)g_set.val[R_SIZE], style, GP_UNIT_PIXEL, &font) != GP_OK) {
        /* 某些字体不支持粗体/斜体组合，退回常规 */
        if (GdipCreateFont(fam, (GpReal)g_set.val[R_SIZE], 0, GP_UNIT_PIXEL, &font) != GP_OK) font = NULL;
    }
    GdipCreateStringFormat(GP_SFF_NOWRAP, 0, &fmt);
    alpha = g_set.val[R_OPACITY] * 255 / 100;
    GdipCreateSolidFill(((GpARGB)alpha << 24) | ((GpARGB)GetRValue(c) << 16) |
                        ((GpARGB)GetGValue(c) << 8) | GetBValue(c), &brush);
    if (!font || !fmt || !brush) goto done;

    GdipSetSmoothingMode(g, GP_SMOOTHING_ANTIALIAS);
    GdipSetTextRenderingHint(g, GP_TEXTHINT_ANTIALIAS);
    GdipMeasureString(g, g_set.text, -1, font, &layout, fmt, &box, NULL, NULL);
    tw = box.Width;
    th = box.Height;
    if (tw < 1) tw = 1;
    if (th < 1) th = 1;
    cw = tw + g_set.val[R_HSPACE];
    ch = th + g_set.val[R_VSPACE];

    /* 以图像中心为原点旋转坐标系；在旋转后的坐标中铺满半径 R 的圆即可覆盖整张图的四角 */
    GdipTranslateWorldTransform(g, (GpReal)(W / 2.0), (GpReal)(H / 2.0), GP_MATRIX_PREPEND);
    GdipRotateWorldTransform(g, (GpReal)g_set.val[R_ANGLE], GP_MATRIX_PREPEND);
    R = sqrt((double)W * W + (double)H * H) / 2.0;
    nx = (int)ceil((R + cw) / cw) + 1;
    ny = (int)ceil((R + ch) / ch) + 1;
    lim = R + sqrt(tw * tw + th * th) / 2.0 + cw / 2.0;   /* 文字中心离原点超过 lim 的格子不可能落在图内 */
    lim *= lim;
    for (j = -ny; j <= ny; j++) {
        double rowOff = (g_set.stagger && (j & 1)) ? cw / 2.0 : 0.0;
        double cy = j * ch;
        for (i = -nx; i <= nx; i++) {
            double cx = i * cw + rowOff;
            GpRectF rc;
            if (cx * cx + cy * cy > lim) continue;
            rc.X = (GpReal)(cx - tw / 2.0);
            rc.Y = (GpReal)(cy - th / 2.0);
            rc.Width = 0;
            rc.Height = 0;
            GdipDrawString(g, g_set.text, -1, font, &rc, fmt, brush);
        }
    }
done:
    if (brush) GdipDeleteBrush(brush);
    if (fmt) GdipDeleteStringFormat(fmt);
    if (font) GdipDeleteFont(font);
    GdipDeleteFontFamily(fam);
}

static void RenderNow(void)
{
    GpGraphics *g = NULL;
    g_renderPending = FALSE;
    if (g_hwnd) KillTimer(g_hwnd, TIMER_RENDER);
    if (!g_src) return;
    if (g_out) { GdipDisposeImage(g_out); g_out = NULL; }
    if (GdipCloneImage(g_src, &g_out) != GP_OK || !g_out) { g_out = NULL; return; }
    if (GdipGetImageGraphicsContext(g_out, &g) == GP_OK && g) {
        DrawWatermark(g, g_imgW, g_imgH);
        GdipDeleteGraphics(g);
    }
    if (!g_headless) {
        if (g_hbmView) { DeleteObject(g_hbmView); g_hbmView = NULL; }
        GdipCreateHBITMAPFromBitmap(g_out, &g_hbmView, 0xFFFFFFFF);
    }
}

static void ScheduleRender(void)
{
    if (!g_src) return;
    g_renderPending = TRUE;
    SetTimer(g_hwnd, TIMER_RENDER, 30, NULL);
}

/* 读取图片 -> 统一转为 32bppARGB（并释放文件锁，允许覆盖原文件） */
static BOOL LoadImageFile(const WCHAR *path)
{
    GpBitmap *tmp = NULL, *nb = NULL;
    GpGraphics *g = NULL;
    UINT w = 0, h = 0;
    GpReal dx = 96, dy = 96;

    if (GdipCreateBitmapFromFile(path, &tmp) != GP_OK || !tmp) {
        if (!g_headless) MessageBoxW(g_hwnd, L"无法打开该图片，请选择 JPG / PNG / BMP 文件。", APP_TITLE, MB_ICONERROR);
        return FALSE;
    }
    {
        /* 先读 EXIF（旋转前），再按方向校正 */
        UINT ow = 0, oh = 0, t;
        long orient = 1;
        GdipGetImageWidth(tmp, &ow);
        GdipGetImageHeight(tmp, &oh);
        GdipGetImageHorizontalResolution(tmp, &dx);
        GdipGetImageVerticalResolution(tmp, &dy);
        {
            UINT sz = 0;
            if (GdipGetPropertyItemSize(tmp, GP_PROPTAG_ORIENTATION, &sz) == GP_OK && sz >= sizeof(GpPropertyItem)) {
                GpPropertyItem *pi = (GpPropertyItem *)malloc(sz);
                if (pi && GdipGetPropertyItem(tmp, GP_PROPTAG_ORIENTATION, sz, pi) == GP_OK && pi->type == 3 && pi->value)
                    orient = *(WORD *)pi->value;
                free(pi);
            }
        }
        if (orient >= 5 && orient <= 8) { t = ow; ow = oh; oh = t; }
        BuildExifInfo(tmp, path, ow, oh, dx, dy);
    }
    ApplyExifOrientation(tmp);
    GdipGetImageWidth(tmp, &w);
    GdipGetImageHeight(tmp, &h);
    if (w == 0 || h == 0 || w > 30000 || h > 30000 ||
        GdipCreateBitmapFromScan0((INT)w, (INT)h, 0, GP_PIXFMT_32ARGB, NULL, &nb) != GP_OK || !nb) {
        GdipDisposeImage(tmp);
        if (!g_headless) MessageBoxW(g_hwnd, L"图片尺寸无效或内存不足。", APP_TITLE, MB_ICONERROR);
        return FALSE;
    }
    if (dx > 1 && dy > 1) GdipBitmapSetResolution(nb, dx, dy);   /* 保留原 DPI，打印尺寸不变 */
    if (GdipGetImageGraphicsContext(nb, &g) == GP_OK && g) {
        GdipSetCompositingMode(g, GP_COMPOSITING_SOURCECOPY);
        GdipSetInterpolationMode(g, GP_INTERP_HQBICUBIC);
        GdipDrawImageRectI(g, tmp, 0, 0, (INT)w, (INT)h);
        GdipDeleteGraphics(g);
    }
    GdipDisposeImage(tmp);

    if (g_src) GdipDisposeImage(g_src);
    g_src = nb;
    g_imgW = (int)w;
    g_imgH = (int)h;
    lstrcpynW(g_path, path, MAX_PATH);
    g_rotation = 0;
    RenderNow();
    return TRUE;
}

/* 保存全分辨率结果；格式由扩展名决定 (.jpg/.jpeg/.bmp/其余为 png) */
static BOOL SaveImageTo(const WCHAR *path)
{
    const WCHAR *ext = wcsrchr(path, L'.');
    const WCHAR *mime = L"image/png";
    CLSID clsid;
    BOOL ok = FALSE;
    if (!g_out) return FALSE;
    if (ext && (!lstrcmpiW(ext, L".jpg") || !lstrcmpiW(ext, L".jpeg") || !lstrcmpiW(ext, L".jpe")))
        mime = L"image/jpeg";
    else if (ext && !lstrcmpiW(ext, L".bmp"))
        mime = L"image/bmp";
    if (!GetEncoderClsid(mime, &clsid)) return FALSE;

    if (!lstrcmpW(mime, L"image/png")) {
        ok = GdipSaveImageToFile(g_out, path, &clsid, NULL) == GP_OK;
    } else {
        /* JPG/BMP 无透明通道：先合成到白底 24 位图 */
        GpBitmap *rgb = NULL;
        GpGraphics *g = NULL;
        GpReal dx = 96, dy = 96;
        if (GdipCreateBitmapFromScan0(g_imgW, g_imgH, 0, GP_PIXFMT_24RGB, NULL, &rgb) != GP_OK || !rgb) return FALSE;
        GdipGetImageHorizontalResolution(g_out, &dx);
        GdipGetImageVerticalResolution(g_out, &dy);
        GdipBitmapSetResolution(rgb, dx, dy);
        if (GdipGetImageGraphicsContext(rgb, &g) == GP_OK && g) {
            GdipGraphicsClear(g, 0xFFFFFFFF);
            GdipDrawImageRectI(g, g_out, 0, 0, g_imgW, g_imgH);
            GdipDeleteGraphics(g);
        }
        if (!lstrcmpW(mime, L"image/jpeg")) {
            GpEncoderParameters ep;
            ULONG quality = 95;
            ep.Count = 1;
            ep.Parameter[0].Guid = GP_ENCODER_QUALITY;
            ep.Parameter[0].NumberOfValues = 1;
            ep.Parameter[0].Type = GP_ENCVAL_LONG;
            ep.Parameter[0].Value = &quality;
            ok = GdipSaveImageToFile(rgb, path, &clsid, &ep) == GP_OK;
        } else {
            ok = GdipSaveImageToFile(rgb, path, &clsid, NULL) == GP_OK;
        }
        GdipDisposeImage(rgb);
    }
    return ok;
}

/* ======================================================================
 * 预览视图（缩放 / 平移）
 * ====================================================================== */
static void UpdateInfo(void)
{
    WCHAR buf[512];
    const WCHAR *tips =
        L"操作提示：\r\n"
        L"· 滚轮缩放预览，左键拖动平移\r\n"
        L"· 双击预览区 = 适应窗口\r\n"
        L"· 可直接把图片拖进窗口\r\n"
        L"· Ctrl+O 打开，Ctrl+S 保存\r\n"
        L"· Ctrl+L / Ctrl+R 左/右旋转 90°\r\n"
        L"· Tab 显示/隐藏 EXIF 信息（编辑框外）\r\n"
        L"· 字号、间距单位均为原图像素";
    if (!g_hInfo) return;
    if (g_src)
        swprintf(buf, 512, L"图片尺寸：%d × %d 像素\r\n预览缩放：%d%%\r\n已旋转：%d°\r\n\r\n%ls",
                 g_imgW, g_imgH, (int)(g_zoom * 100 + 0.5), g_rotation * 90, tips);
    else
        swprintf(buf, 512, L"尚未打开图片\r\n\r\n%ls", tips);
    SetWindowTextW(g_hInfo, buf);
}

static void ClampView(void)
{
    RECT rc;
    double sw, sh;
    if (!g_hPreview) return;
    GetClientRect(g_hPreview, &rc);
    sw = g_imgW * g_zoom;
    sh = g_imgH * g_zoom;
    if (sw <= rc.right) g_offX = (rc.right - sw) / 2.0;
    else { if (g_offX > 0) g_offX = 0; if (g_offX < rc.right - sw) g_offX = rc.right - sw; }
    if (sh <= rc.bottom) g_offY = (rc.bottom - sh) / 2.0;
    else { if (g_offY > 0) g_offY = 0; if (g_offY < rc.bottom - sh) g_offY = rc.bottom - sh; }
}

static void SetRowEditText(int i, int v)
{
    WCHAR buf[16];
    swprintf(buf, 16, L"%d", v);
    SetWindowTextW(g_hRowEdt[i], buf);
}

static void SyncZoomRow(void)
{
    int p = ClampI((int)(g_zoom * 100 + 0.5), g_rowDef[R_ZOOM].minv, g_rowDef[R_ZOOM].maxv);
    g_set.val[R_ZOOM] = p;
    g_sync = TRUE;
    SendMessageW(g_hRowTrk[R_ZOOM], TBM_SETPOS, TRUE, p);
    if (GetFocus() != g_hRowEdt[R_ZOOM]) SetRowEditText(R_ZOOM, p);
    g_sync = FALSE;
    UpdateInfo();
}

static void SetZoomAt(double z, int px, int py, BOOL fromRow)
{
    double ix, iy;
    if (!g_src) return;
    if (z < 0.05) z = 0.05;
    if (z > 16.0) z = 16.0;
    ix = (px - g_offX) / g_zoom;
    iy = (py - g_offY) / g_zoom;
    g_zoom = z;
    g_offX = px - ix * z;
    g_offY = py - iy * z;
    g_fitMode = FALSE;
    ClampView();
    if (fromRow) UpdateInfo(); else SyncZoomRow();
    InvalidateRect(g_hPreview, NULL, FALSE);
}

static void SetZoomCenter(double z, BOOL fromRow)
{
    RECT rc;
    GetClientRect(g_hPreview, &rc);
    SetZoomAt(z, rc.right / 2, rc.bottom / 2, fromRow);
}

static void FitView(void)
{
    RECT rc;
    double zx, zy, m;
    if (!g_src) return;
    GetClientRect(g_hPreview, &rc);
    if (rc.right <= 0 || rc.bottom <= 0) return;
    m = S(12);
    zx = (rc.right - 2 * m) / g_imgW;
    zy = (rc.bottom - 2 * m) / g_imgH;
    g_zoom = zx < zy ? zx : zy;
    if (g_zoom < 0.05) g_zoom = 0.05;
    if (g_zoom > 16.0) g_zoom = 16.0;
    g_fitMode = TRUE;
    ClampView();
    SyncZoomRow();
    InvalidateRect(g_hPreview, NULL, FALSE);
}

/* 原图无损旋转 90°（dir>0 顺时针 / 向右，dir<0 逆时针 / 向左），水印在旋转后的图上重新平铺 */
static void RotateSource(int dir)
{
    int t;
    if (!g_src) return;
    if (GdipImageRotateFlip(g_src, dir > 0 ? 1 /* Rotate90FlipNone */ : 3 /* Rotate270FlipNone */) != GP_OK)
        return;
    t = g_imgW; g_imgW = g_imgH; g_imgH = t;
    g_rotation = (g_rotation + (dir > 0 ? 1 : 3)) % 4;
    RenderNow();
    FitView();
    UpdateInfo();
}

/* 预览区左上角的 EXIF 叠加层（固定在屏幕上，不随缩放/平移，不进入保存结果） */
static void DrawExifOverlay(HDC dc)
{
    HFONT of = (HFONT)SelectObject(dc, g_hUiFont);
    TEXTMETRICW tm;
    SIZE sz;
    int i, lw = 0, vw = 0, lineH, pad = S(12), colGap = S(14), x, y, bw, bh, r = S(10);
    const WCHAR *title = L"图片信息（Tab 关闭）";
    GpGraphics *g = NULL;
    void *path = NULL;
    GpBrush *br = NULL;
    WCHAR rot[32];
    int n = g_exifN;

    GetTextMetricsW(dc, &tm);
    lineH = tm.tmHeight + S(3);
    swprintf(rot, 32, L"%d°", g_rotation * 90);
    for (i = 0; i <= n; i++) {
        const WCHAR *l = i < n ? g_exifLbl[i] : L"当前旋转";
        const WCHAR *v = i < n ? g_exifVal[i] : rot;
        GetTextExtentPoint32W(dc, l, lstrlenW(l), &sz); if (sz.cx > lw) lw = sz.cx;
        GetTextExtentPoint32W(dc, v, lstrlenW(v), &sz); if (sz.cx > vw) vw = sz.cx;
    }
    GetTextExtentPoint32W(dc, title, lstrlenW(title), &sz);
    bw = lw + colGap + vw;
    if (sz.cx > bw) bw = sz.cx;
    bw += 2 * pad;
    bh = 2 * pad + lineH * (n + 2) + S(6);
    x = S(12);
    y = S(12);

    if (GdipCreateFromHDC(dc, &g) == GP_OK && g) {
        GdipSetSmoothingMode(g, GP_SMOOTHING_ANTIALIAS);
        if (GdipCreatePath(0, &path) == GP_OK && path) {
            GdipAddPathArcI(path, x, y, 2 * r, 2 * r, 180, 90);
            GdipAddPathArcI(path, x + bw - 2 * r, y, 2 * r, 2 * r, 270, 90);
            GdipAddPathArcI(path, x + bw - 2 * r, y + bh - 2 * r, 2 * r, 2 * r, 0, 90);
            GdipAddPathArcI(path, x, y + bh - 2 * r, 2 * r, 2 * r, 90, 90);
            GdipClosePathFigure(path);
            if (GdipCreateSolidFill(0xB4101418, &br) == GP_OK && br) {
                GdipFillPath(g, br, path);
                GdipDeleteBrush(br);
            }
            GdipDeletePath(path);
        }
        GdipDeleteGraphics(g);
    }
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(255, 214, 102));
    TextOutW(dc, x + pad, y + pad, title, lstrlenW(title));
    for (i = 0; i <= n; i++) {
        const WCHAR *l = i < n ? g_exifLbl[i] : L"当前旋转";
        const WCHAR *v = i < n ? g_exifVal[i] : rot;
        int yy = y + pad + lineH * (i + 1) + S(6);
        SetTextColor(dc, RGB(170, 180, 190));
        TextOutW(dc, x + pad, yy, l, lstrlenW(l));
        SetTextColor(dc, RGB(255, 255, 255));
        TextOutW(dc, x + pad + lw + colGap, yy, v, lstrlenW(v));
    }
    SelectObject(dc, of);
}

static void PaintPreview(HWND hwnd, HDC hdc)
{
    RECT rc;
    HDC mem;
    HBITMAP bb, oldbb;
    HBRUSH bg;
    GetClientRect(hwnd, &rc);
    if (rc.right <= 0 || rc.bottom <= 0) return;
    mem = CreateCompatibleDC(hdc);
    bb = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    oldbb = (HBITMAP)SelectObject(mem, bb);
    bg = CreateSolidBrush(RGB(60, 63, 65));
    FillRect(mem, &rc, bg);
    DeleteObject(bg);

    if (g_hbmView) {
        double z = g_zoom;
        int sx0 = (int)floor(-g_offX / z), sy0 = (int)floor(-g_offY / z);
        int sx1 = (int)ceil((rc.right - g_offX) / z), sy1 = (int)ceil((rc.bottom - g_offY) / z);
        if (sx0 < 0) sx0 = 0;
        if (sy0 < 0) sy0 = 0;
        if (sx1 > g_imgW) sx1 = g_imgW;
        if (sy1 > g_imgH) sy1 = g_imgH;
        if (sx1 > sx0 && sy1 > sy0) {
            HDC src = CreateCompatibleDC(hdc);
            HBITMAP oldsrc = (HBITMAP)SelectObject(src, g_hbmView);
            int dx0 = (int)floor(g_offX + sx0 * z + 0.5), dy0 = (int)floor(g_offY + sy0 * z + 0.5);
            int dx1 = (int)floor(g_offX + sx1 * z + 0.5), dy1 = (int)floor(g_offY + sy1 * z + 0.5);
            if (z < 1.0) { SetStretchBltMode(mem, HALFTONE); SetBrushOrgEx(mem, 0, 0, NULL); }
            else SetStretchBltMode(mem, COLORONCOLOR);
            StretchBlt(mem, dx0, dy0, dx1 - dx0, dy1 - dy0, src, sx0, sy0, sx1 - sx0, sy1 - sy0, SRCCOPY);
            SelectObject(src, oldsrc);
            DeleteDC(src);
        }
    } else {
        HFONT of = (HFONT)SelectObject(mem, g_hUiFont);
        SetBkMode(mem, TRANSPARENT);
        SetTextColor(mem, RGB(200, 200, 200));
        DrawTextW(mem, L"点击「打开图片」或把图片拖到这里", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(mem, of);
    }
    if (g_showExif && g_src) DrawExifOverlay(mem);
    BitBlt(hdc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
    SelectObject(mem, oldbb);
    DeleteObject(bb);
    DeleteDC(mem);
}

static LRESULT CALLBACK PreviewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        PaintPreview(hwnd, hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_SIZE:
        if (g_fitMode) FitView(); else ClampView();
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    case WM_MOUSEWHEEL: {
        POINT pt;
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        ScreenToClient(hwnd, &pt);
        SetZoomAt(g_zoom * pow(1.15, delta / 120.0), pt.x, pt.y, FALSE);
        return 0;
    }
    case WM_LBUTTONDOWN:
        SetFocus(hwnd);
        if (g_src) {
            g_drag = TRUE;
            g_dragPt.x = GET_X_LPARAM(lParam);
            g_dragPt.y = GET_Y_LPARAM(lParam);
            g_dragOffX = g_offX;
            g_dragOffY = g_offY;
            SetCapture(hwnd);
        }
        return 0;
    case WM_MOUSEMOVE:
        if (g_drag) {
            g_offX = g_dragOffX + (GET_X_LPARAM(lParam) - g_dragPt.x);
            g_offY = g_dragOffY + (GET_Y_LPARAM(lParam) - g_dragPt.y);
            g_fitMode = FALSE;
            ClampView();
            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
        }
        return 0;
    case WM_LBUTTONUP:
        if (g_drag) ReleaseCapture();
        g_drag = FALSE;
        return 0;
    case WM_CAPTURECHANGED:
        g_drag = FALSE;
        return 0;
    case WM_LBUTTONDBLCLK:
        FitView();
        return 0;
    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT && g_src) {
            SetCursor(LoadCursor(NULL, g_drag ? IDC_SIZEALL : IDC_HAND));
            return TRUE;
        }
        break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

/* ======================================================================
 * 主窗口 / 控件
 * ====================================================================== */
static UINT GetWindowDpiCompat(HWND hwnd)
{
    typedef UINT (WINAPI *PFN)(HWND);
    static PFN pGetDpiForWindow;
    static BOOL tried;
    UINT dpi = 0;
    if (!tried) {
        HMODULE u = GetModuleHandleW(L"user32.dll");
        tried = TRUE;
        if (u) pGetDpiForWindow = (PFN)(void (*)(void))GetProcAddress(u, "GetDpiForWindow");
    }
    if (pGetDpiForWindow && hwnd) dpi = pGetDpiForWindow(hwnd);
    if (!dpi) {
        HDC dc = GetDC(NULL);
        dpi = (UINT)GetDeviceCaps(dc, LOGPIXELSX);
        ReleaseDC(NULL, dc);
    }
    return dpi ? dpi : 96;
}

static BOOL CALLBACK SetFontProc(HWND child, LPARAM lParam)
{
    SendMessageW(child, WM_SETFONT, (WPARAM)lParam, TRUE);
    return TRUE;
}

static void RecreateUiFont(void)
{
    HFONT old = g_hUiFont;
    g_hUiFont = CreateFontW(-MulDiv(9, (int)g_dpi, 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                            L"Microsoft YaHei UI");
    if (g_hwnd) EnumChildWindows(g_hwnd, SetFontProc, (LPARAM)g_hUiFont);
    if (old) DeleteObject(old);
}

static HWND Ctl(DWORD exStyle, const WCHAR *cls, const WCHAR *text, DWORD style, int id)
{
    HWND h = CreateWindowExW(exStyle, cls, text, WS_CHILD | WS_VISIBLE | style, 0, 0, 10, 10,
                             g_hPanel, (HMENU)(INT_PTR)id, g_hInst, NULL);
    SendMessageW(h, WM_SETFONT, (WPARAM)g_hUiFont, FALSE);
    return h;
}

static void UpdateFontButton(void)
{
    WCHAR buf[LF_FACESIZE + 32];
    swprintf(buf, LF_FACESIZE + 32, L"%ls%ls%ls…", g_set.face, g_set.bold ? L" 粗体" : L"",
             g_set.italic ? L" 斜体" : L"");
    SetWindowTextW(g_hFontBtn, buf);
}

static void UpdateSwatch(void)
{
    if (g_hSwatchBrush) DeleteObject(g_hSwatchBrush);
    g_hSwatchBrush = CreateSolidBrush(g_set.color);
    InvalidateRect(g_hSwatch, NULL, TRUE);
}

static void ApplySettingsToControls(void)
{
    int i;
    g_sync = TRUE;
    SetWindowTextW(g_hText, g_set.text);
    for (i = 0; i < NROWS; i++) {
        SendMessageW(g_hRowTrk[i], TBM_SETPOS, TRUE, g_set.val[i]);
        SetRowEditText(i, g_set.val[i]);
    }
    Button_SetCheck(g_hStagger, g_set.stagger ? BST_CHECKED : BST_UNCHECKED);
    g_sync = FALSE;
    UpdateFontButton();
    UpdateSwatch();
}

static void CreateControls(void)
{
    int i;
    g_hPreview = CreateWindowExW(0, PREVIEW_CLASS, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                                 0, 0, 10, 10, g_hwnd, (HMENU)(INT_PTR)IDC_PREVIEW, g_hInst, NULL);
    /* 右侧参数面板：带竖直滚动条的子容器，所有控件都放在它里面 */
    g_hPanel = CreateWindowExW(WS_EX_CONTROLPARENT, PANEL_CLASS, NULL,
                               WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                               0, 0, 10, 10, g_hwnd, (HMENU)(INT_PTR)IDC_PANEL, g_hInst, NULL);
    g_hOpen   = Ctl(0, WC_BUTTONW, L"打开图片…", WS_TABSTOP | BS_PUSHBUTTON, IDC_OPEN);
    g_hSave   = Ctl(0, WC_BUTTONW, L"保存图片…", WS_TABSTOP | BS_PUSHBUTTON, IDC_SAVE);
    g_hFit    = Ctl(0, WC_BUTTONW, L"适应窗口", WS_TABSTOP | BS_PUSHBUTTON, IDC_FIT);
    g_hActual = Ctl(0, WC_BUTTONW, L"原始大小 1:1", WS_TABSTOP | BS_PUSHBUTTON, IDC_ACTUAL);
    g_hRotL   = Ctl(0, WC_BUTTONW, L"向左旋转 90°", WS_TABSTOP | BS_PUSHBUTTON, IDC_ROTL);
    g_hRotR   = Ctl(0, WC_BUTTONW, L"向右旋转 90°", WS_TABSTOP | BS_PUSHBUTTON, IDC_ROTR);
    g_hLblText = Ctl(0, WC_STATICW, L"水印文字", 0, IDC_LBL_TEXT);
    g_hText   = Ctl(WS_EX_CLIENTEDGE, WC_EDITW, L"", WS_TABSTOP | ES_AUTOHSCROLL, IDC_TEXT);
    SendMessageW(g_hText, EM_SETLIMITTEXT, 255, 0);
    for (i = 0; i < NROWS; i++) {
        g_hRowLbl[i] = Ctl(0, WC_STATICW, g_rowDef[i].label, 0, IDC_LABEL0 + i);
        g_hRowTrk[i] = Ctl(0, TRACKBAR_CLASSW, L"", WS_TABSTOP | TBS_HORZ | TBS_NOTICKS, IDC_TRACK0 + i);
        SendMessageW(g_hRowTrk[i], TBM_SETRANGEMIN, FALSE, g_rowDef[i].minv);
        SendMessageW(g_hRowTrk[i], TBM_SETRANGEMAX, TRUE, g_rowDef[i].maxv);
        SendMessageW(g_hRowTrk[i], TBM_SETPAGESIZE, 0, (g_rowDef[i].maxv - g_rowDef[i].minv) / 20 + 1);
        g_hRowEdt[i] = Ctl(WS_EX_CLIENTEDGE, WC_EDITW, L"", WS_TABSTOP | ES_AUTOHSCROLL | ES_CENTER, IDC_EDIT0 + i);
        SendMessageW(g_hRowEdt[i], EM_SETLIMITTEXT, 5, 0);
    }
    g_hLblFont  = Ctl(0, WC_STATICW, L"字体", 0, IDC_LBL_FONT);
    g_hFontBtn  = Ctl(0, WC_BUTTONW, L"", WS_TABSTOP | BS_PUSHBUTTON, IDC_FONT);
    g_hLblColor = Ctl(0, WC_STATICW, L"颜色", 0, IDC_LBL_COLOR);
    g_hSwatch   = Ctl(WS_EX_CLIENTEDGE, WC_STATICW, L"", SS_NOTIFY, IDC_SWATCH);
    g_hColorBtn = Ctl(0, WC_BUTTONW, L"选择颜色…", WS_TABSTOP | BS_PUSHBUTTON, IDC_COLOR);
    g_hStagger  = Ctl(0, WC_BUTTONW, L"交错排列（隔行错开半格）", WS_TABSTOP | BS_AUTOCHECKBOX, IDC_STAGGER);
    g_hReset    = Ctl(0, WC_BUTTONW, L"恢复默认参数", WS_TABSTOP | BS_PUSHBUTTON, IDC_RESET);
    g_hInfo     = Ctl(0, WC_STATICW, L"", 0, IDC_INFO);

    /* 工具提示 */
    g_hTip = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
                             CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, g_hwnd, NULL, g_hInst, NULL);
    if (g_hTip) {
        static const struct { int id; const WCHAR *tip; } tips[] = {
            { IDC_OPEN,   L"打开 JPG / PNG / BMP 图片 (Ctrl+O)" },
            { IDC_SAVE,   L"按原图分辨率保存加水印后的图片 (Ctrl+S)" },
            { IDC_FIT,    L"缩放预览以完整显示图片 (Ctrl+0)" },
            { IDC_ACTUAL, L"按 100% 原始像素显示" },
            { IDC_ROTL,   L"将图片逆时针旋转 90°（无损，保存结果也会旋转）(Ctrl+L)" },
            { IDC_ROTR,   L"将图片顺时针旋转 90°（无损，保存结果也会旋转）(Ctrl+R)" },
        };
        size_t k;
        SendMessageW(g_hTip, TTM_SETMAXTIPWIDTH, 0, 600);
        for (k = 0; k < sizeof(tips) / sizeof(tips[0]); k++) {
            TOOLINFOW ti;
            ZeroMemory(&ti, sizeof(ti));
            ti.cbSize = sizeof(ti);
            ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
            ti.hwnd = g_hPanel;
            ti.uId = (UINT_PTR)GetDlgItem(g_hPanel, tips[k].id);
            ti.lpszText = (LPWSTR)tips[k].tip;
            SendMessageW(g_hTip, TTM_ADDTOOLW, 0, (LPARAM)&ti);
        }
    }
}

static void PlaceRow(int i, int x0, int y, int w)
{
    int lw = S(78), ew = S(54), gap = S(4);
    int tw = w - lw - ew - 2 * gap;
    MoveWindow(g_hRowLbl[i], x0, y + S(6), lw, S(20), TRUE);
    MoveWindow(g_hRowTrk[i], x0 + lw + gap, y, tw, S(30), TRUE);
    MoveWindow(g_hRowEdt[i], x0 + w - ew, y + S(3), ew, S(24), TRUE);
}

/* 在面板内按滚动偏移 top 摆放控件，返回内容总高度（与偏移无关） */
static int LayoutPanelAt(int top)
{
    RECT rc;
    int pad = S(10), gap = S(6), bh = S(28);
    int x0 = pad, w, y, bw, lw = S(78) + S(4), i;
    GetClientRect(g_hPanel, &rc);
    w = rc.right - 2 * pad;
    if (w < S(200)) w = S(200);
    bw = (w - gap) / 2;
    y = pad - top;
    MoveWindow(g_hOpen, x0, y, bw, bh, TRUE);
    MoveWindow(g_hSave, x0 + bw + gap, y, w - bw - gap, bh, TRUE);
    y += bh + gap;
    MoveWindow(g_hFit, x0, y, bw, bh, TRUE);
    MoveWindow(g_hActual, x0 + bw + gap, y, w - bw - gap, bh, TRUE);
    y += bh + gap;
    MoveWindow(g_hRotL, x0, y, bw, bh, TRUE);
    MoveWindow(g_hRotR, x0 + bw + gap, y, w - bw - gap, bh, TRUE);
    y += bh + gap;
    PlaceRow(R_ZOOM, x0, y, w);
    y += S(42);
    MoveWindow(g_hLblText, x0, y, w, S(20), TRUE);
    y += S(22);
    MoveWindow(g_hText, x0, y, w, S(26), TRUE);
    y += S(26) + S(10);
    for (i = 0; i < R_ZOOM; i++) { PlaceRow(i, x0, y, w); y += S(34); }
    y += S(4);
    MoveWindow(g_hLblFont, x0, y + S(5), lw, S(20), TRUE);
    MoveWindow(g_hFontBtn, x0 + lw, y, w - lw, bh, TRUE);
    y += bh + gap;
    MoveWindow(g_hLblColor, x0, y + S(5), lw, S(20), TRUE);
    MoveWindow(g_hSwatch, x0 + lw, y + S(2), S(40), bh - S(4), TRUE);
    MoveWindow(g_hColorBtn, x0 + lw + S(46), y, w - lw - S(46), bh, TRUE);
    y += bh + gap;
    MoveWindow(g_hStagger, x0, y, w, S(24), TRUE);
    y += S(24) + gap;
    MoveWindow(g_hReset, x0, y, w, bh, TRUE);
    y += bh + S(14);
    MoveWindow(g_hInfo, x0, y, w, S(200), TRUE);
    y += S(200) + pad;
    return y + top;
}

static void UpdatePanelScrollBar(void)
{
    RECT rc;
    SCROLLINFO si;
    GetClientRect(g_hPanel, &rc);
    ZeroMemory(&si, sizeof(si));
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_DISABLENOSCROLL;   /* 滚动条常驻，避免显隐导致宽度抖动 */
    si.nMin = 0;
    si.nMax = g_contentH > 0 ? g_contentH - 1 : 0;
    si.nPage = (UINT)(rc.bottom > 0 ? rc.bottom : 0);
    si.nPos = g_scrollY;
    SetScrollInfo(g_hPanel, SB_VERT, &si, TRUE);
}

static int MaxPanelScroll(void)
{
    RECT rc;
    int m;
    GetClientRect(g_hPanel, &rc);
    m = g_contentH - rc.bottom;
    return m > 0 ? m : 0;
}

static void LayoutPanel(void)
{
    int maxs;
    if (!g_hPanel) return;
    g_contentH = LayoutPanelAt(g_scrollY);
    maxs = MaxPanelScroll();
    if (g_scrollY > maxs) {
        g_scrollY = maxs;
        LayoutPanelAt(g_scrollY);
    }
    UpdatePanelScrollBar();
    RedrawWindow(g_hPanel, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
}

static void ScrollPanelTo(int pos)
{
    int dy;
    pos = ClampI(pos, 0, MaxPanelScroll());
    dy = g_scrollY - pos;
    if (!dy) return;
    g_scrollY = pos;
    ScrollWindowEx(g_hPanel, 0, dy, NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN | SW_INVALIDATE | SW_ERASE);
    UpdatePanelScrollBar();
    UpdateWindow(g_hPanel);
}

/* Tab 切换焦点到面板可视区外的控件时，自动滚动使其可见 */
static void EnsureFocusVisible(void)
{
    HWND f = GetFocus();
    RECT rc, cr;
    if (!f || GetParent(f) != g_hPanel) return;
    GetWindowRect(f, &rc);
    MapWindowPoints(NULL, g_hPanel, (POINT *)&rc, 2);
    GetClientRect(g_hPanel, &cr);
    if (rc.top < 0) ScrollPanelTo(g_scrollY + rc.top - S(6));
    else if (rc.bottom > cr.bottom) ScrollPanelTo(g_scrollY + rc.bottom - cr.bottom + S(6));
}

static LRESULT CALLBACK PanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_SIZE:
        LayoutPanel();
        return 0;
    case WM_VSCROLL: {
        SCROLLINFO si;
        int pos = g_scrollY, line = S(30);
        RECT rc;
        GetClientRect(hwnd, &rc);
        ZeroMemory(&si, sizeof(si));
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
        GetScrollInfo(hwnd, SB_VERT, &si);
        switch (LOWORD(wParam)) {
        case SB_LINEUP:        pos -= line; break;
        case SB_LINEDOWN:      pos += line; break;
        case SB_PAGEUP:        pos -= rc.bottom - line; break;
        case SB_PAGEDOWN:      pos += rc.bottom - line; break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION: pos = si.nTrackPos; break;
        case SB_TOP:           pos = 0; break;
        case SB_BOTTOM:        pos = g_contentH; break;
        default: return 0;
        }
        ScrollPanelTo(pos);
        return 0;
    }
    case WM_MOUSEWHEEL: {
        UINT lines = 3;
        int step;
        SystemParametersInfoW(SPI_GETWHEELSCROLLLINES, 0, &lines, 0);
        if (lines == 0 || lines > 20) lines = 3;
        g_wheelAcc += GET_WHEEL_DELTA_WPARAM(wParam);
        step = g_wheelAcc * (int)lines * S(14) / WHEEL_DELTA;
        if (step) {
            g_wheelAcc -= step * WHEEL_DELTA / ((int)lines * S(14));
            ScrollPanelTo(g_scrollY - step);
        }
        return 0;
    }
    /* 子控件通知转交主窗口统一处理 */
    case WM_COMMAND:
    case WM_HSCROLL:
    case WM_CTLCOLORSTATIC:
    case WM_NOTIFY:
        return SendMessageW(g_hwnd, msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static void Layout(void)
{
    RECT rc;
    int pw;
    if (!g_hPanel) return;
    GetClientRect(g_hwnd, &rc);
    pw = S(320) + GetSystemMetrics(SM_CXVSCROLL) * (int)g_dpi / 96;
    if (pw > rc.right) pw = rc.right;
    MoveWindow(g_hPreview, 0, 0, rc.right - pw, rc.bottom, TRUE);
    MoveWindow(g_hPanel, rc.right - pw, 0, pw, rc.bottom, TRUE);
    LayoutPanel();
}

static void SetTitleFromPath(void)
{
    WCHAR buf[MAX_PATH + 64];
    const WCHAR *name = wcsrchr(g_path, L'\\');
    name = name ? name + 1 : g_path;
    if (g_path[0]) swprintf(buf, MAX_PATH + 64, L"%ls - %ls", name, APP_TITLE);
    else lstrcpynW(buf, APP_TITLE, MAX_PATH + 64);
    SetWindowTextW(g_hwnd, buf);
}

static void OpenPath(const WCHAR *path)
{
    if (LoadImageFile(path)) {
        SetTitleFromPath();
        FitView();
        UpdateInfo();
    }
}

static void DoOpen(void)
{
    WCHAR file[MAX_PATH] = L"";
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hwnd;
    ofn.lpstrFilter = L"图片文件 (*.jpg;*.jpeg;*.png;*.bmp;*.tif;*.gif)\0*.jpg;*.jpeg;*.jpe;*.png;*.bmp;*.tif;*.tiff;*.gif\0所有文件 (*.*)\0*.*\0";
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = L"打开证件图片";
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    if (GetOpenFileNameW(&ofn)) OpenPath(file);
}

static void DoSave(void)
{
    WCHAR file[MAX_PATH] = L"";
    OPENFILENAMEW ofn;
    const WCHAR *srcExt;
    BOOL jpg;
    if (!g_src) {
        MessageBoxW(g_hwnd, L"请先打开一张图片。", APP_TITLE, MB_ICONINFORMATION);
        return;
    }
    if (g_renderPending) RenderNow();
    srcExt = wcsrchr(g_path, L'.');
    jpg = srcExt && (!lstrcmpiW(srcExt, L".jpg") || !lstrcmpiW(srcExt, L".jpeg"));
    {
        /* 默认文件名: 原名_水印.png/.jpg */
        const WCHAR *name = wcsrchr(g_path, L'\\');
        WCHAR base[MAX_PATH], *dot;
        lstrcpynW(base, name ? name + 1 : g_path, MAX_PATH);
        dot = wcsrchr(base, L'.');
        if (dot) *dot = 0;
        swprintf(file, MAX_PATH, L"%ls_水印.%ls", base, jpg ? L"jpg" : L"png");
    }
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hwnd;
    ofn.lpstrFilter = L"PNG 图片 (*.png)\0*.png\0JPEG 图片 (*.jpg)\0*.jpg;*.jpeg\0BMP 图片 (*.bmp)\0*.bmp\0";
    ofn.nFilterIndex = jpg ? 2 : 1;
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrDefExt = jpg ? L"jpg" : L"png";
    ofn.lpstrTitle = L"保存加水印的图片";
    ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    if (!GetSaveFileNameW(&ofn)) return;
    {
        /* 用户没写扩展名时按所选类型补上 */
        const WCHAR *ext = wcsrchr(file, L'.');
        const WCHAR *slash = wcsrchr(file, L'\\');
        if ((!ext || (slash && ext < slash)) && lstrlenW(file) + 5 < MAX_PATH)
            lstrcatW(file, ofn.nFilterIndex == 2 ? L".jpg" : ofn.nFilterIndex == 3 ? L".bmp" : L".png");
    }
    SetCursor(LoadCursor(NULL, IDC_WAIT));
    if (SaveImageTo(file)) {
        WCHAR msg[MAX_PATH + 32];
        swprintf(msg, MAX_PATH + 32, L"已保存：\r\n%ls", file);
        MessageBoxW(g_hwnd, msg, APP_TITLE, MB_ICONINFORMATION);
    } else {
        MessageBoxW(g_hwnd, L"保存失败，请检查路径是否可写。", APP_TITLE, MB_ICONERROR);
    }
}

static void DoChooseFont(void)
{
    LOGFONTW lf;
    CHOOSEFONTW cf;
    ZeroMemory(&lf, sizeof(lf));
    lstrcpynW(lf.lfFaceName, g_set.face, LF_FACESIZE);
    lf.lfHeight = -MulDiv(14, (int)g_dpi, 72);
    lf.lfWeight = g_set.bold ? FW_BOLD : FW_NORMAL;
    lf.lfItalic = (BYTE)(g_set.italic ? 1 : 0);
    lf.lfCharSet = DEFAULT_CHARSET;
    ZeroMemory(&cf, sizeof(cf));
    cf.lStructSize = sizeof(cf);
    cf.hwndOwner = g_hwnd;
    cf.lpLogFont = &lf;
    cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_TTONLY | CF_NOVERTFONTS | CF_NOSIZESEL | CF_NOSCRIPTSEL;
    if (ChooseFontW(&cf)) {
        lstrcpynW(g_set.face, lf.lfFaceName, LF_FACESIZE);
        g_set.bold = lf.lfWeight >= FW_SEMIBOLD;
        g_set.italic = lf.lfItalic != 0;
        UpdateFontButton();
        ScheduleRender();
    }
}

static void DoChooseColor(void)
{
    CHOOSECOLORW cc;
    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = g_hwnd;
    cc.rgbResult = g_set.color;
    cc.lpCustColors = g_custColors;
    cc.Flags = CC_RGBINIT | CC_FULLOPEN | CC_ANYCOLOR;
    if (ChooseColorW(&cc)) {
        g_set.color = cc.rgbResult;
        UpdateSwatch();
        ScheduleRender();
    }
}

static void OnRowChanged(int i, int v, BOOL updTrack, BOOL updEdit)
{
    v = ClampI(v, g_rowDef[i].minv, g_rowDef[i].maxv);
    g_set.val[i] = v;
    g_sync = TRUE;
    if (updTrack) SendMessageW(g_hRowTrk[i], TBM_SETPOS, TRUE, v);
    if (updEdit) SetRowEditText(i, v);
    g_sync = FALSE;
    if (i == R_ZOOM) SetZoomCenter(v / 100.0, TRUE);
    else ScheduleRender();
}

static LRESULT CALLBACK MainProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        RECT rc;
        g_hwnd = hwnd;
        g_dpi = GetWindowDpiCompat(hwnd);
        RecreateUiFont();
        CreateControls();
        ApplySettingsToControls();
        UpdateInfo();
        DragAcceptFiles(hwnd, TRUE);
        /* 按当前 DPI 调整初始大小 */
        GetWindowRect(hwnd, &rc);
        SetWindowPos(hwnd, NULL, 0, 0, S(1180), S(780), SWP_NOMOVE | SWP_NOZORDER);
        Layout();
        return 0;
    }
    case WM_SIZE:
        Layout();
        return 0;
    case WM_GETMINMAXINFO: {
        MINMAXINFO *mmi = (MINMAXINFO *)lParam;
        mmi->ptMinTrackSize.x = S(760);
        mmi->ptMinTrackSize.y = S(480);
        return 0;
    }
    case WM_DPICHANGED: {
        RECT *r = (RECT *)lParam;
        UINT oldDpi = g_dpi;
        g_dpi = HIWORD(wParam);
        g_scrollY = MulDiv(g_scrollY, (int)g_dpi, (int)oldDpi);
        RecreateUiFont();
        SetWindowPos(hwnd, NULL, r->left, r->top, r->right - r->left, r->bottom - r->top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
        Layout();
        if (g_fitMode) FitView();
        return 0;
    }
    case WM_HSCROLL: {
        HWND tb = (HWND)lParam;
        int id = tb ? GetDlgCtrlID(tb) : 0;
        if (id >= IDC_TRACK0 && id < IDC_TRACK0 + NROWS && !g_sync)
            OnRowChanged(id - IDC_TRACK0, (int)SendMessageW(tb, TBM_GETPOS, 0, 0), FALSE, TRUE);
        return 0;
    }
    case WM_COMMAND: {
        int id = LOWORD(wParam), code = HIWORD(wParam);
        switch (id) {
        case IDC_OPEN:   DoOpen(); return 0;
        case IDC_SAVE:   DoSave(); return 0;
        case IDC_FIT:    FitView(); return 0;
        case IDC_ACTUAL: SetZoomCenter(1.0, FALSE); return 0;
        case IDC_ROTL:   RotateSource(-1); return 0;
        case IDC_ROTR:   RotateSource(+1); return 0;
        case IDC_FONT:   DoChooseFont(); return 0;
        case IDC_COLOR:  DoChooseColor(); return 0;
        case IDC_SWATCH: if (code == STN_CLICKED) DoChooseColor(); return 0;
        case IDC_TEXT:
            if (code == EN_CHANGE && !g_sync) {
                GetWindowTextW(g_hText, g_set.text, 256);
                ScheduleRender();
            }
            return 0;
        case IDC_STAGGER:
            if (code == BN_CLICKED) {
                g_set.stagger = Button_GetCheck(g_hStagger) == BST_CHECKED;
                ScheduleRender();
            }
            return 0;
        case IDC_RESET: {
            int z = g_set.val[R_ZOOM];
            DefaultSettings(&g_set, TRUE);
            g_set.val[R_ZOOM] = z;
            ApplySettingsToControls();
            ScheduleRender();
            return 0;
        }
        }
        if (id >= IDC_EDIT0 && id < IDC_EDIT0 + NROWS) {
            int i = id - IDC_EDIT0;
            if (code == EN_CHANGE && !g_sync) {
                WCHAR buf[16];
                GetWindowTextW(g_hRowEdt[i], buf, 16);
                if (buf[0] && !(buf[0] == L'-' && buf[1] == 0))
                    OnRowChanged(i, _wtoi(buf), TRUE, FALSE);
            } else if (code == EN_KILLFOCUS) {
                g_sync = TRUE;
                SetRowEditText(i, g_set.val[i]);
                g_sync = FALSE;
            }
            return 0;
        }
        break;
    }
    case WM_CTLCOLORSTATIC:
        if ((HWND)lParam == g_hSwatch && g_hSwatchBrush) return (LRESULT)g_hSwatchBrush;
        break;
    case WM_TIMER:
        if (wParam == TIMER_RENDER) {
            RenderNow();
            InvalidateRect(g_hPreview, NULL, FALSE);
        }
        return 0;
    case WM_DROPFILES: {
        WCHAR file[MAX_PATH];
        HDROP hd = (HDROP)wParam;
        if (DragQueryFileW(hd, 0, file, MAX_PATH)) OpenPath(file);
        DragFinish(hd);
        return 0;
    }
    case WM_CLOSE:
        SaveSettings();
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

/* ======================================================================
 * 入口
 * ====================================================================== */
static int RunHeadless(const WCHAR *in, const WCHAR *out)
{
    g_headless = TRUE;
    if (!LoadImageFile(in)) return 2;
    return SaveImageTo(out) ? 0 : 3;
}

static BOOL IsEditFocused(void)
{
    WCHAR cls[16];
    HWND f = GetFocus();
    return f && GetClassNameW(f, cls, 16) && !lstrcmpiW(cls, L"Edit");
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrev, PWSTR cmdLine, int nShow)
{
    GpStartupInput gsi;
    INITCOMMONCONTROLSEX icc;
    WNDCLASSEXW wc;
    MSG msg;
    HACCEL hAccel;
    ACCEL acc[5];
    LPWSTR *argv;
    int argc = 0, ret = 0;
    (void)hPrev;
    (void)cmdLine;

    g_hInst = hInst;
    InitIniPath();
    LoadSettings();

    ZeroMemory(&gsi, sizeof(gsi));
    gsi.GdiplusVersion = 1;
    if (GdiplusStartup(&g_gdipToken, &gsi, NULL) != GP_OK) {
        MessageBoxW(NULL, L"GDI+ 初始化失败。", APP_TITLE, MB_ICONERROR);
        return 1;
    }

    argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv && argc >= 4 && !lstrcmpiW(argv[2], L"-o")) {
        ret = RunHeadless(argv[1], argv[3]);
        LocalFree(argv);
        if (g_out) GdipDisposeImage(g_out);
        if (g_src) GdipDisposeImage(g_src);
        GdiplusShutdown(g_gdipToken);
        return ret;
    }

    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);

    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_APPICON), IMAGE_ICON,
                                 GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
    wc.hIconSm = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_APPICON), IMAGE_ICON,
                                   GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
    wc.lpfnWndProc = MainProc;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = MAIN_CLASS;
    RegisterClassExW(&wc);
    wc.lpfnWndProc = PreviewProc;
    wc.hIcon = NULL;
    wc.hIconSm = NULL;
    wc.hbrBackground = NULL;
    wc.style = CS_DBLCLKS;
    wc.lpszClassName = PREVIEW_CLASS;
    RegisterClassExW(&wc);
    wc.lpfnWndProc = PanelProc;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.style = 0;
    wc.lpszClassName = PANEL_CLASS;
    RegisterClassExW(&wc);

    if (!CreateWindowExW(0, MAIN_CLASS, APP_TITLE, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                         CW_USEDEFAULT, CW_USEDEFAULT, 1180, 780, NULL, NULL, hInst, NULL)) {
        GdiplusShutdown(g_gdipToken);
        return 1;
    }
    ShowWindow(g_hwnd, nShow);
    UpdateWindow(g_hwnd);
    if (argv && argc >= 2) OpenPath(argv[1]);
    if (argv) LocalFree(argv);

    acc[0].fVirt = FCONTROL | FVIRTKEY; acc[0].key = 'O'; acc[0].cmd = IDC_OPEN;
    acc[1].fVirt = FCONTROL | FVIRTKEY; acc[1].key = 'S'; acc[1].cmd = IDC_SAVE;
    acc[2].fVirt = FCONTROL | FVIRTKEY; acc[2].key = '0'; acc[2].cmd = IDC_FIT;
    acc[3].fVirt = FCONTROL | FVIRTKEY; acc[3].key = 'L'; acc[3].cmd = IDC_ROTL;
    acc[4].fVirt = FCONTROL | FVIRTKEY; acc[4].key = 'R'; acc[4].cmd = IDC_ROTR;
    hAccel = CreateAcceleratorTableW(acc, 5);

    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_MOUSEWHEEL) {
            /* 滚轮按光标位置分发，无论焦点在哪 */
            POINT pt;
            HWND under;
            pt.x = GET_X_LPARAM(msg.lParam);
            pt.y = GET_Y_LPARAM(msg.lParam);
            under = WindowFromPoint(pt);
            /* 预览区：缩放；面板及其子控件（滑块/编辑框等）上：滚动面板，避免误改参数 */
            if (under == g_hPreview) msg.hwnd = g_hPreview;
            else if (under == g_hPanel || IsChild(g_hPanel, under)) msg.hwnd = g_hPanel;
        }
        {
            /* 焦点在编辑框内时：Tab 照常切换焦点，Ctrl+L/Ctrl+R 交给编辑框；否则 Tab 切换 EXIF 叠加层 */
            BOOL inEdit = IsEditFocused();
            if (msg.message == WM_KEYDOWN && msg.wParam == VK_TAB && !inEdit &&
                GetAncestor(msg.hwnd, GA_ROOT) == g_hwnd &&
                !(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_MENU) & 0x8000)) {
                g_showExif = !g_showExif;
                InvalidateRect(g_hPreview, NULL, FALSE);
                continue;
            }
            if (msg.message == WM_CHAR && msg.wParam == L'\t' && !inEdit) continue;
            if (!(inEdit && (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN) &&
                  (msg.wParam == 'L' || msg.wParam == 'R')) &&
                TranslateAcceleratorW(g_hwnd, hAccel, &msg)) continue;
        }
        if (IsDialogMessageW(g_hwnd, &msg)) {
            if (msg.message == WM_KEYDOWN && msg.wParam == VK_TAB) EnsureFocusVisible();
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    DestroyAcceleratorTable(hAccel);

    if (g_hbmView) DeleteObject(g_hbmView);
    if (g_out) GdipDisposeImage(g_out);
    if (g_src) GdipDisposeImage(g_src);
    if (g_hUiFont) DeleteObject(g_hUiFont);
    if (g_hSwatchBrush) DeleteObject(g_hSwatchBrush);
    GdiplusShutdown(g_gdipToken);
    return (int)msg.wParam;
}
