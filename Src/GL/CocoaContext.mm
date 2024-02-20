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

#include "CocoaContext.h"
#include <AppKit/AppKit.h>
#include <Foundation/Foundation.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

namespace GL
{
    CocoaContext::~CocoaContext() { destroy(); }

    bool CocoaContext::create(void *window)
    {
        NSOpenGLPixelFormatAttribute attribs[] = {
            NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
            NSOpenGLPFAColorSize,     24,
            NSOpenGLPFAAlphaSize,     8,
            NSOpenGLPFADoubleBuffer,  NSOpenGLPFAAccelerated,
            NSOpenGLPFANoRecovery,    0,
        };

        pixel_format = [[NSOpenGLPixelFormat new] initWithAttributes:attribs];

        context = [[NSOpenGLContext new] initWithFormat:pixel_format shareContext:nil];

        bool success = context != nil;

        if (success)
        {
            make_current();
            auto view = reinterpret_cast<NSView *>(window);
            [context setView:view];
            [context update];
        }

        return success;
    }

    void CocoaContext::destroy()
    {
        if (context)
        {
            [context release];
            context = nil;
        }
    }

    void CocoaContext::set_swap_interval(int value)
    {
        [context setValues:&value forParameter:NSOpenGLContextParameterSwapInterval];
    }

    void CocoaContext::update(int w, int h)
    {

        NSView *view = [context view];
        NSSize size = NSMakeSize((float)w, (float)h);
        [view setFrameSize:size];

        [context update];
    }

    void CocoaContext::swap_buffers() { [context flushBuffer]; }

    void CocoaContext::make_current() { [context makeCurrentContext]; }

    void CocoaContext::done_current() { [NSOpenGLContext clearCurrentContext]; }

}