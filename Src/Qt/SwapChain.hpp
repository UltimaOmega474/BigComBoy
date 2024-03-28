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
#include <array>
#include <atomic>

namespace QtFrontend
{
    template <size_t Size> class SwapChain
    {
        int32_t rendering_index = 0;
        std::atomic_int32_t ready_index = 1;
        int32_t drawing_index = 2;

        std::array<std::array<uint8_t, Size>, 3> buffers{};

    public:
        std::array<uint8_t, Size> &next_rendering_image();

        void swap_image();
        std::array<uint8_t, Size> &next_drawing_image();
    };

    template <size_t Size> inline std::array<uint8_t, Size> &SwapChain<Size>::next_rendering_image()
    {
        auto &image = buffers[rendering_index];
        rendering_index = ready_index.exchange(rendering_index);
        return image;
    }

    template <size_t Size> inline std::array<uint8_t, Size> &SwapChain<Size>::next_drawing_image()
    {
        drawing_index = ready_index.exchange(drawing_index);
        return buffers[drawing_index];
    }
}