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
#include <chrono>
#include <tuple>

namespace Common::Math {
    std::tuple<float, float> fit_aspect_ratio(float window_width, float window_height,
                                              float target_width, float target_height);

    void ortho_projection(std::array<float, 16> &matrix, float left, float right, float top,
                          float bottom, float near, float far);

    consteval std::chrono::nanoseconds freq_to_nanoseconds(int32_t hz) {
        using namespace std::chrono_literals;
        constexpr std::chrono::nanoseconds nanoseconds_per_hertz = 1000000000ns;
        // framerate was inconsistent without this adjustment
        return nanoseconds_per_hertz / hz + 1ns;
    }

}