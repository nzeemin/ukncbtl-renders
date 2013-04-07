// RenderDX9.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <d3d9.h>

/// \brief Definition for render mode enumeration procedure.
typedef void (CALLBACK* RENDER_MODE_ENUM_PROC)(int modeNum, LPCTSTR modeDesc, int modeWidth, int modeHeight);

HWND g_hwndScreen = (HWND) INVALID_HANDLE_VALUE;

int g_SourceWidth = 0;
int g_SourceHeight = 0;

LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_direct3dDevice = NULL;
LPDIRECT3DSURFACE9 g_D3DSurface = NULL;


void CALLBACK RenderEnumModes(RENDER_MODE_ENUM_PROC enumProc)
{
    enumProc(1, _T("Free Scale Mode"), -1, -1);
}

BOOL CALLBACK RenderSelectMode(int newMode)
{
    //TODO

    return TRUE;
}

BOOL CALLBACK RenderInit(int width, int height, HWND hwndTarget)
{
    //TODO: Check values
    g_SourceWidth = width;
    g_SourceHeight = height;
    g_hwndScreen = hwndTarget;

    g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (g_pD3D == NULL)
        return FALSE;

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;
    d3dpp.BackBufferCount = 1;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    d3dpp.hDeviceWindow = g_hwndScreen;

    HRESULT hr = g_pD3D->CreateDevice(
            D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hwndScreen, D3DCREATE_HARDWARE_VERTEXPROCESSING,
            &d3dpp, &g_direct3dDevice);
    if (FAILED(hr))
        return FALSE;

    D3DFORMAT backbufferformat = d3dpp.BackBufferFormat;

    g_direct3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);

    hr = g_direct3dDevice->CreateOffscreenPlainSurface(
            g_SourceWidth, g_SourceHeight, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT,
            &g_D3DSurface, NULL);
    if (FAILED(hr))
        return FALSE;

    RenderSelectMode(1);  // Select the default mode

    return TRUE;
}

void CALLBACK RenderDone()
{
    if (g_D3DSurface != NULL)
    {
        g_D3DSurface->Release();  g_D3DSurface = NULL;
    }
    if (g_direct3dDevice != NULL)
    {
        g_direct3dDevice->Release();  g_direct3dDevice = NULL;
    }
    if (g_pD3D != NULL)
    {
        g_pD3D->Release();  g_pD3D = NULL;
    }

    //TODO: Finalize
}

void CALLBACK RenderDraw(const void * pixels, HDC hdc)
{
    if (pixels == NULL) return;
    if (g_direct3dDevice == NULL) return;

    HRESULT hr = g_direct3dDevice->Clear(0, NULL,
            D3DCLEAR_TARGET,
            D3DCOLOR_XRGB(0, 0, 0),
            1.0f, 0);
    g_direct3dDevice->BeginScene();

    //g_direct3dDevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MAXMIPLEVEL, 0);

    D3DLOCKED_RECT lockedrect;
    g_D3DSurface->LockRect(&lockedrect, NULL, 0);
    for (int line = 0; line < g_SourceHeight; line++)
    {
        DWORD * pSrc = ((DWORD*)pixels) + g_SourceWidth * line;
        BYTE * pDest = ((BYTE*)lockedrect.pBits) + lockedrect.Pitch * line;
        ::memcpy(pDest, pSrc, g_SourceWidth * 4);
    }
    g_D3DSurface->UnlockRect();

    IDirect3DSurface9* backbuffer = NULL;
    hr = g_direct3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
    //D3DSURFACE_DESC backbufferdesc;
    //hr = backbuffer->GetDesc(&backbufferdesc);

    RECT srcRect, destRect;
    srcRect.left = srcRect.top = 0;
    srcRect.right = g_SourceWidth;  srcRect.bottom = g_SourceHeight;
    ::GetClientRect(g_hwndScreen, &destRect);
    hr = g_direct3dDevice->StretchRect(g_D3DSurface, &srcRect, backbuffer, NULL,
            D3DTEXF_LINEAR);

    g_direct3dDevice->EndScene();

    g_direct3dDevice->Present(NULL, NULL, NULL, NULL);
}


//////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved)
{
    return TRUE;
}
