// RenderVfw.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <vfw.h>

HWND g_hwndScreen = (HWND) INVALID_HANDLE_VALUE;

int g_SourceWidth = 0;
int g_SourceHeight = 0;
int m_ScreenHeightMode = -1;  // 1 - Normal height, 2 - Double height, 3 - Upscaled to 1.5

HDRAWDIB m_hdd = NULL;
BITMAPINFO m_bmpinfo;
HBITMAP m_hbmp = NULL;
DWORD * m_bits = NULL;

int m_cxScreenWidth;
int m_cyScreenHeight;
int m_xScreenOffset = 0;
int m_yScreenOffset = 0;

void CALLBACK PrepareScreenCopy(const void * pSrcBits, void * pDestBits);
void CALLBACK PrepareScreenUpscale(const void * pSrcBits, void * pDestBits);
void CALLBACK PrepareScreenUpscale2(const void * pSrcBits, void * pDestBits);
void CALLBACK PrepareScreenUpscale3(const void * pSrcBits, void * pDestBits);
void CALLBACK PrepareScreenUpscale4(const void * pSrcBits, void * pDestBits);

//�������� ������� �������������� ������
typedef void (CALLBACK* PREPARE_SCREEN_CALLBACK)(const void * pSrcBits, void * pDestBits);

struct ScreenModeStruct
{
    int width;
    int height;
    PREPARE_SCREEN_CALLBACK callback;
}
static ScreenModeReference[] = {
    {  640,  288, PrepareScreenCopy },  // Dummy record for absent mode 0
    {  640,  288, PrepareScreenCopy },
    {  640,  576, PrepareScreenUpscale2 },
    {  640,  432, PrepareScreenUpscale },
    {  960,  576, PrepareScreenUpscale3 },
    { 1280,  864, PrepareScreenUpscale4 },
};

void RenderGetScreenSize(int scrmode, int* pwid, int* phei)
{
    if (scrmode < 0 || scrmode >= sizeof(ScreenModeReference) / sizeof(ScreenModeStruct))
    {
        *pwid = *phei = 0;
    }
    else
    {
        ScreenModeStruct* pinfo = ScreenModeReference + scrmode;
        *pwid = pinfo->width;
        *phei = pinfo->height;
    }
}

void RenderCreateDisplay()
{
    if (m_hbmp != NULL)
    {
        DeleteObject(m_hbmp);
        m_hbmp = NULL;
    }

    RenderGetScreenSize(m_ScreenHeightMode, &m_cxScreenWidth, &m_cyScreenHeight);

    m_bmpinfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
    m_bmpinfo.bmiHeader.biWidth = m_cxScreenWidth;
    m_bmpinfo.bmiHeader.biHeight = m_cyScreenHeight;
    m_bmpinfo.bmiHeader.biPlanes = 1;
    m_bmpinfo.bmiHeader.biBitCount = 32;
    m_bmpinfo.bmiHeader.biCompression = BI_RGB;
    m_bmpinfo.bmiHeader.biSizeImage = 0;
    m_bmpinfo.bmiHeader.biXPelsPerMeter = 0;
    m_bmpinfo.bmiHeader.biYPelsPerMeter = 0;
    m_bmpinfo.bmiHeader.biClrUsed = 0;
    m_bmpinfo.bmiHeader.biClrImportant = 0;

    HDC hdc = GetDC( g_hwndScreen );
    
    m_hbmp = CreateDIBSection( hdc, &m_bmpinfo, DIB_RGB_COLORS, (void **) &m_bits, NULL, 0 );

    ReleaseDC( g_hwndScreen, hdc );
}

BOOL CALLBACK RenderSelectMode(int newMode)
{
    if (m_ScreenHeightMode == newMode) return TRUE;

    m_ScreenHeightMode = newMode;

    RenderCreateDisplay();

    int cxScreen = 100;
    int cyScreen = 100;
    RenderGetScreenSize(m_ScreenHeightMode, &cxScreen, &cyScreen);

    return TRUE;
}

BOOL CALLBACK RenderInit(int width, int height, HWND hwndTarget)
{
    //TODO: Check values
    g_SourceWidth = width;
    g_SourceHeight = height;
    g_hwndScreen = hwndTarget;

    m_hdd = DrawDibOpen();

    RenderSelectMode(1);  // Select the default mode

    return TRUE;
}

void CALLBACK RenderDone()
{
    if (m_hbmp != NULL)
    {
        DeleteObject(m_hbmp);
        m_hbmp = NULL;
    }

    DrawDibClose( m_hdd );
}

BOOL CALLBACK RenderEnumModes()
{
    return FALSE;
}

void CALLBACK RenderDraw(const void * pixels, HDC hdc)
{
    if (pixels == NULL) return;
    if (m_bits == NULL) return;

    PREPARE_SCREEN_CALLBACK callback = ScreenModeReference[m_ScreenHeightMode].callback;
    if (callback != NULL)
        callback(pixels, m_bits);

    m_xScreenOffset = 0;
    m_yScreenOffset = 0;
    RECT rc;  GetClientRect(g_hwndScreen, &rc);
    if (rc.right > m_cxScreenWidth)
    {
        m_xScreenOffset = (rc.right - m_cxScreenWidth) / 2;
        ::PatBlt(hdc, 0, 0, m_xScreenOffset, rc.bottom, BLACKNESS);
        ::PatBlt(hdc, rc.right, 0, m_cxScreenWidth + m_xScreenOffset - rc.right, rc.bottom, BLACKNESS);
    }
    if (rc.bottom > m_cyScreenHeight)
    {
        m_yScreenOffset = (rc.bottom - m_cyScreenHeight) / 2;
        ::PatBlt(hdc, m_xScreenOffset, 0, m_cxScreenWidth, m_yScreenOffset, BLACKNESS);
        int frombottom = rc.bottom - m_yScreenOffset - m_cyScreenHeight;
        ::PatBlt(hdc, m_xScreenOffset, rc.bottom, m_cxScreenWidth, -frombottom, BLACKNESS);
    }

    DrawDibDraw(m_hdd, hdc,
        m_xScreenOffset, m_yScreenOffset, -1, -1,
        &m_bmpinfo.bmiHeader, m_bits,
        0,0, m_cxScreenWidth, m_cyScreenHeight,
        0);
}


//////////////////////////////////////////////////////////////////////

#define AVERAGERGB(a, b)  ( (((a) & 0xfefefeffUL) + ((b) & 0xfefefeffUL)) >> 1 )

void CALLBACK PrepareScreenCopy(const void * pSrcBits, void * pDestBits)
{
    for (int line = 0; line < g_SourceHeight; line++)
    {
        DWORD * pSrc = ((DWORD*)pSrcBits) + g_SourceWidth * (g_SourceHeight - line - 1);
        DWORD * pDest = ((DWORD*)pDestBits) + g_SourceWidth * line;
        ::memcpy(pDest, pSrc, g_SourceWidth * 4);
    }
    //::memcpy(pDestBits, pSrcBits, g_SourceWidth * g_SourceHeight * 4);
}

// Upscale screen from height 288 to 432
void CALLBACK PrepareScreenUpscale(const void * pSrcBits, void * pDestBits)
{
    int ukncline = g_SourceHeight - 1;
    for (int line = 431; line > 0; line--)
    {
        DWORD* pdest = ((DWORD*)pDestBits) + (431 - line) * g_SourceWidth;
        if (line % 3 == 1)
        {
            DWORD* psrc1 = ((DWORD*)pSrcBits) + ukncline * g_SourceWidth;
            DWORD* psrc2 = ((DWORD*)pSrcBits) + (ukncline + 1) * g_SourceWidth;
            DWORD* pdst1 = (DWORD*)pdest;
            for (int i = 0; i < g_SourceWidth; i++)
            {
                *pdst1 = AVERAGERGB(*psrc1, *psrc2);
                psrc1++;  psrc2++;  pdst1++;
            }
        }
        else
        {
            DWORD* psrc = ((DWORD*)pSrcBits) + ukncline * g_SourceWidth;
            memcpy(pdest, psrc, g_SourceWidth * 4);
            ukncline--;
        }
    }
}

// Upscale screen to twice height with "interlaced" effect
void CALLBACK PrepareScreenUpscale2(const void * pSrcBits, void * pDestBits)
{
    for (int ukncline = 0; ukncline < g_SourceHeight; ukncline++)
    {
        DWORD* psrc = ((DWORD*)pSrcBits) + ukncline * g_SourceWidth;
        DWORD* pdest = ((DWORD*)pDestBits) + ((g_SourceHeight - ukncline - 1) * 2) * g_SourceWidth;
        memcpy(pdest, psrc, g_SourceWidth * 4);

        pdest += g_SourceWidth;
        memset(pdest, 0, g_SourceWidth * 4);
    }
}

// Upscale screen width 640->960, height 288->576 with "interlaced" effect
void CALLBACK PrepareScreenUpscale3(const void * pSrcBits, void * pDestBits)
{
    for (int ukncline = g_SourceHeight - 1; ukncline >= 0; ukncline--)
    {
        DWORD* psrc = ((DWORD*)pSrcBits) + ukncline * g_SourceWidth;
        psrc += g_SourceWidth - 1;
        DWORD* pdest = ((DWORD*)pDestBits) + ((g_SourceHeight - ukncline - 1) * 2) * 960;
        pdest += 960 - 1;
        for (int i = 0; i < g_SourceWidth / 2; i++)
        {
            DWORD c1 = *psrc;  psrc--;
            DWORD c2 = *psrc;  psrc--;
            DWORD c12 = AVERAGERGB(c1, c2);
            *pdest = c1;  pdest--;
            *pdest = c12; pdest--;
            *pdest = c2;  pdest--;
        }

        pdest += 960;
        memset(pdest, 0, 960 * 4);
    }
}

// Upscale screen width 640->1280, height 288->864 with "interlaced" effect
void CALLBACK PrepareScreenUpscale4(const void * pSrcBits, void * pDestBits)
{
    for (int ukncline = g_SourceHeight - 1; ukncline >= 0; ukncline--)
    {
        DWORD* psrc = ((DWORD*)pSrcBits) + ukncline * g_SourceWidth;
        DWORD* pdest = ((DWORD*)pDestBits) + ((g_SourceHeight - ukncline - 1) * 3) * 1280;
        psrc += g_SourceWidth - 1;
        pdest += 1280 - 1;
        DWORD* pdest2 = pdest + 1280;
        DWORD* pdest3 = pdest2 + 1280;
        for (int i = 0; i < g_SourceWidth; i++)
        {
            DWORD color = *psrc;  psrc--;
            *pdest = color;  pdest--;
            *pdest = color;  pdest--;
            *pdest2 = color;  pdest2--;
            *pdest2 = color;  pdest2--;
            *pdest3 = 0;  pdest3--;
            *pdest3 = 0;  pdest3--;
        }
    }
}


//////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved)
{
    return TRUE;
}
