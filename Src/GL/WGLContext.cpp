/*
    Big ComBoy
    Copyright (C) 2023-2024 UltimaOmega474

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "WGLContext.hpp"
#include <GL/glew.h>
#include <array>

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB 0x2093
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_DEBUG_BIT_ARB 0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define ERROR_INVALID_VERSION_ARB 0x2095
#define ERROR_INVALID_PROFILE_ARB 0x2096

namespace GL
{
    bool WGLContext::create(void *window)
    {
        hwnd = reinterpret_cast<HWND>(window);
        hdc = GetDC(hwnd);

        PIXELFORMATDESCRIPTOR pfd = {
            sizeof(PIXELFORMATDESCRIPTOR), // size of this pfd
            1,                             // version number
            PFD_DRAW_TO_WINDOW |           // support window
                PFD_SUPPORT_OPENGL |       // support OpenGL
                PFD_DOUBLEBUFFER,          // double buffered
            PFD_TYPE_RGBA,                 // RGBA type
            24,                            // 24-bit color depth
            0,
            0,
            0,
            0,
            0,
            0, // color bits ignored
            0, // no alpha buffer
            0, // shift bit ignored
            0, // no accumulation buffer
            0,
            0,
            0,
            0,              // accum bits ignored
            32,             // 32-bit z-buffer
            0,              // no stencil buffer
            0,              // no auxiliary buffer
            PFD_MAIN_PLANE, // main layer
            0,              // reserved
            0,
            0,
            0 // layer masks ignored
        };

        int pixel_format = ChoosePixelFormat(hdc, &pfd);

        SetPixelFormat(hdc, pixel_format, &pfd);

        HGLRC temp_context = wglCreateContext(hdc);

        if (temp_context)
        {
            wglMakeCurrent(hdc, temp_context);

            typedef HGLRC(wglCreateContextAttribsARB__)(HDC hDC, HGLRC hShareContext,
                                                        int *attribList);

            wglCreateContextAttribsARB__ *wglCreateContextAttribsARB =
                reinterpret_cast<wglCreateContextAttribsARB__ *>(
                    wglGetProcAddress("wglCreateContextAttribsARB"));

            if (wglCreateContextAttribsARB)
            {
                int flags = 0;

#ifdef _DEBUG
                flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
#endif

                std::array<int, 9> attribs{
                    WGL_CONTEXT_PROFILE_MASK_ARB,
                    WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                    WGL_CONTEXT_MAJOR_VERSION_ARB,
                    4,
                    WGL_CONTEXT_MINOR_VERSION_ARB,
                    1,
                    WGL_CONTEXT_FLAGS_ARB,
                    flags,
                    NULL,
                };

                context = wglCreateContextAttribsARB(hdc, nullptr, attribs.data());
                wglMakeCurrent(hdc, nullptr);
                wglDeleteContext(temp_context);
                temp_context = nullptr;

                bool success = context != nullptr;

                if (success)
                {
                    wglMakeCurrent(hdc, context);

                    wglSwapIntervalEXT = reinterpret_cast<wglSwapIntervalEXT__ *>(
                        wglGetProcAddress("wglSwapIntervalEXT"));

                    glewInit();
                }

                return success;
            }
        }

        return false;
    }

    void WGLContext::destroy()
    {
        wglMakeCurrent(hdc, nullptr);
        wglDeleteContext(context);
    }

    void WGLContext::set_swap_interval(int value) { wglSwapIntervalEXT(value); }

    void WGLContext::update(int w, int h)
    {
        RECT rect{};
        GetClientRect(hwnd, &rect);
        rect.right = w;
        rect.bottom = h;

        int client_width = rect.right - rect.left;
        int client_height = rect.bottom - rect.top;
        SetWindowPos(hwnd, nullptr, rect.left, rect.top, client_width, client_height,
                     SWP_NOZORDER | SWP_NOACTIVATE);

        UpdateWindow(hwnd);

        glViewport(0, 0, client_width, client_height);
    }

    void WGLContext::swap_buffers() { SwapBuffers(hdc); }

    void WGLContext::make_current() { wglMakeCurrent(hdc, context); }

    void WGLContext::done_current() { wglMakeCurrent(hdc, nullptr); }
}