// RenderOpenGL.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <gl\gl.h>

HWND g_hwndScreen = (HWND) INVALID_HANDLE_VALUE;

int g_SourceWidth = 0;
int g_SourceHeight = 0;

HGLRC g_hRC = NULL;

DWORD * g_TextureBuffer = NULL;


BOOL CALLBACK RenderSelectMode(int newMode)
{
    //TODO

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

    HDC hdc = ::GetDC(g_hwndScreen);

    PIXELFORMATDESCRIPTOR pfd;
    ::memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;
    int iFormat = ChoosePixelFormat(hdc, &pfd);
    int iOldFormat = ::GetPixelFormat(hdc);
    if (iOldFormat != iFormat)
        ::SetPixelFormat(hdc, iFormat, &pfd);
    
    g_hRC = wglCreateContext(hdc);

    wglMakeCurrent(NULL, NULL);

    ::ReleaseDC(g_hwndScreen, hdc);

    if (g_hRC == NULL)
        return FALSE;

    g_TextureBuffer = (DWORD*) ::malloc(1024 * 512 * 4);
    ::memset(g_TextureBuffer, 0, 1024 * 512 * 4);

    RenderSelectMode(1);  // Select the default mode

    return TRUE;
}

void CALLBACK RenderDone()
{
    wglMakeCurrent(NULL, NULL);

    if (g_hRC != NULL)
    {
        wglDeleteContext(g_hRC);
        g_hRC = NULL;
    }

    if (g_TextureBuffer != NULL)
    {
        ::free(g_TextureBuffer);
        g_TextureBuffer = NULL;
    }
}

BOOL CALLBACK RenderEnumModes()
{
    return FALSE;
}

void CALLBACK RenderDraw(const void * pixels, HDC hdc)
{
    if (pixels == NULL) return;

    for (int line = 0; line < g_SourceHeight; line++)
    {
        DWORD* pSrc = ((DWORD*)pixels) + g_SourceWidth * line;
        DWORD* pDest = g_TextureBuffer + 1024 * (line + (512 - 288) / 2) + (1024 - 640) / 2;
        ::memcpy(pDest, pSrc, g_SourceWidth * 4);
    }

    RECT rc;  ::GetClientRect(g_hwndScreen, &rc);

    wglMakeCurrent(hdc, g_hRC);
    
    glViewport(0, 0, rc.right, rc.bottom);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, rc.right, rc.bottom, 0, 1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //glTranslatef(0.0f,0.0f,-5.0f);

    //glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //glClear(GL_COLOR_BUFFER_BIT);

    //glShadeModel(GL_SMOOTH);
    //glRasterPos2i(0, 0);
    //glPixelZoom(1.5f, -1.5);

    //glDrawPixels(640, 288, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);

    glEnable(GL_TEXTURE_2D);
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 512, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, g_TextureBuffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, texture);
    float dx = 1024 / ((float)g_SourceWidth);
    float dy = 512 / ((float)g_SourceHeight);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-dx, -dy,  1.0f);  // Bottom Left Of The Texture and Quad
        glTexCoord2f(1.0f, 1.0f); glVertex3f( dx, -dy,  1.0f);  // Bottom Right Of The Texture and Quad
        glTexCoord2f(1.0f, 0.0f); glVertex3f( dx,  dy,  1.0f);  // Top Right Of The Texture and Quad
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-dx,  dy,  1.0f);
    glEnd();

    glFlush();
    SwapBuffers(hdc);

    glDeleteTextures(1, &texture);

    wglMakeCurrent(NULL, NULL);
}


//////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved)
{
    return TRUE;
}
