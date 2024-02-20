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

namespace GL
{
    class Context
    {
    public:
        virtual ~Context() {}
        virtual bool create(void *window) = 0;
        virtual void destroy() = 0;
        virtual void set_swap_interval(int value) = 0;
        virtual void update(int w, int h) = 0;
        virtual void swap_buffers() = 0;
        virtual void make_current() = 0;
        virtual void done_current() = 0;
    };
}