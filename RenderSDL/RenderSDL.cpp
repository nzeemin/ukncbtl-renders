/*  This file is part of UKNCBTL.
    UKNCBTL is free software: you can redistribute it and/or modify it under the terms
of the GNU Lesser General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.
    UKNCBTL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License along with
UKNCBTL. If not, see <http://www.gnu.org/licenses/>. */

// RenderSDL.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <SDL.h>

/// \brief Definition for render mode enumeration procedure.
typedef void (CALLBACK* RENDER_MODE_ENUM_PROC)(int modeNum, LPCTSTR modeDesc, int modeWidth, int modeHeight);

HWND g_hwndScreen = (HWND) INVALID_HANDLE_VALUE;

int g_SourceWidth = 0;
int g_SourceHeight = 0;
int m_ScreenMode = 0;

SDL_Window* g_Window = NULL;
SDL_Renderer* g_Renderer = NULL;


void CALLBACK RenderEnumModes(RENDER_MODE_ENUM_PROC enumProc)
{
    enumProc(1, _T("Free Scale Mode"), -1, -1);
    //enumProc(2, _T("Fixed aspect ratio 4:3"), -1, -1);
}

BOOL CALLBACK RenderSelectMode(int newMode)
{
    m_ScreenMode = newMode;

    return TRUE;
}

BOOL CALLBACK RenderInit(int width, int height, HWND hwndTarget)
{
    if (hwndTarget == INVALID_HANDLE_VALUE)
        return FALSE;
    //TODO: Check values
    g_SourceWidth = width;
    g_SourceHeight = height;
    g_hwndScreen = hwndTarget;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        //Log("Unable to Init SDL: %s", SDL_GetError());
        return FALSE;
    }

    g_Window = SDL_CreateWindowFrom(hwndTarget);
    if (g_Window == NULL)
    {
        SDL_Quit();
        return FALSE;
    }

    g_Renderer = SDL_CreateRenderer(g_Window, -1, SDL_RENDERER_ACCELERATED);
    if (g_Renderer == NULL)
    {
        g_Window = NULL;
        SDL_Quit();
        return FALSE;
    }

    //TODO

    RenderSelectMode(1);  // Select the default mode

    return TRUE;
}

void CALLBACK RenderDone()
{
    if (g_Renderer != NULL)
    {
        SDL_DestroyRenderer(g_Renderer);
        g_Renderer = NULL;
    }
        
    if (g_Window != NULL)
    {
        g_Window = NULL;
    }

    //TODO

    SDL_Quit();
}

void CALLBACK RenderDraw(const void * pixels, HDC hdc)
{
    if (pixels == NULL) return;

    RECT rc;  ::GetClientRect(g_hwndScreen, &rc);

    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
        (void*)pixels, g_SourceWidth, g_SourceHeight, 32, 4 * g_SourceWidth,
        0x00ff0000, 0x0000ff00, 0x000000ff, 0);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(g_Renderer, surface);

    SDL_Rect srcrect = { 0, 0, g_SourceWidth, g_SourceHeight };
    SDL_Rect dstrect = { 0, 0, rc.right, rc.bottom };

    SDL_RenderCopy(g_Renderer, texture, &srcrect, &dstrect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);

    //DEBUG
    //SDL_SetRenderDrawColor(g_Renderer, 0x00, 0x00, 0x00, 0xFF);
    //SDL_RenderDrawLine(g_Renderer, 0, 0, 640, 320);
    //SDL_RenderClear(g_Renderer);
    SDL_RenderPresent(g_Renderer);
    //SDL_UpdateWindowSurface(g_Window);

    //TODO
}


//////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved)
{
    return TRUE;
}
