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

namespace GL
{
    class NullContext : public Context
    {
    public:
        ~NullContext() override;
        bool create(void *window) override;
        void destroy() override;
        void set_swap_interval(int value) override;
        void update(int w, int h) override;
        void swap_buffers() override;
        void make_current() override;
        void done_current() override;
    };
}