// RenderOpenGL.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <gl\gl.h>

HWND g_hwndScreen = (HWND) INVALID_HANDLE_VALUE;

int g_SourceWidth = 0;
int g_SourceHeight = 0;

HGLRC g_hRC = NULL;


void SetVSync(bool sync);


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

    wglMakeCurrent(hdc, g_hRC);
    SetVSync(0);
    glEnable(GL_TEXTURE_2D);
    wglMakeCurrent(NULL, NULL);

    ::ReleaseDC(g_hwndScreen, hdc);

    if (g_hRC == NULL)
        return FALSE;

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
}

BOOL CALLBACK RenderEnumModes()
{
    return FALSE;
}

void CALLBACK RenderDraw(const void * pixels, HDC hdc)
{
    if (pixels == NULL) return;

    wglMakeCurrent(hdc, g_hRC);
    
    RECT rc;  ::GetClientRect(g_hwndScreen, &rc);
    glViewport(0, 0, rc.right, rc.bottom);
    
    bool okOne2One = (rc.right == g_SourceWidth && rc.bottom == g_SourceHeight);
    GLuint texture = 0;
    if (okOne2One)  // 1:1 mode using glDrawPixels
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        //glShadeModel(GL_SMOOTH);
        glRasterPos2i(-1, 1);
        glPixelZoom(1.0f, -1.0);
        glDrawPixels(g_SourceWidth, g_SourceHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);
    }
    else  // Free scale mode using texture on quad
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, rc.right, rc.bottom, 0, 1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 512, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
        glTexSubImage2D(GL_TEXTURE_2D, 0,
            (1024 - g_SourceWidth) / 2, (512 - g_SourceHeight) / 2, g_SourceWidth, g_SourceHeight,
            GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);
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
    }

    //glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //glClear(GL_COLOR_BUFFER_BIT);

    glFlush();
    SwapBuffers(hdc);

    if (!okOne2One)
        glDeleteTextures(1, &texture);

    wglMakeCurrent(NULL, NULL);
}

// Enable or disable VBlank sync for OpenGL
void SetVSync(bool sync)
{	
	// Function pointer for the wgl extention function we need to enable/disable vsync
	typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALPROC)( int );
	PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;

	const char *extensions = (const char*) glGetString(GL_EXTENSIONS);
	if (strstr(extensions, "WGL_EXT_swap_control") == 0)
		return;

    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress("wglSwapIntervalEXT");
	if (wglSwapIntervalEXT)
		wglSwapIntervalEXT(sync);
}

//////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved)
{
    return TRUE;
}
