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

#pragma once
#include "Context.hpp"

#ifdef __OBJC__
#include <AppKit/AppKit.h>
#else
class NSOpenGLPixelFormat;
class NSOpenGLContext;
#endif

namespace GL
{
    class CocoaContext : public Context
    {
        NSOpenGLPixelFormat *pixel_format;
        NSOpenGLContext *context;

    public:
        CocoaContext() = default;
        ~CocoaContext() override;
        bool create(void *window) override;
        void destroy() override;
        void update(int w, int h) override;
        void set_swap_interval(int value) override;

        void swap_buffers() override;
        void make_current() override;
        void done_current() override;
    };

}