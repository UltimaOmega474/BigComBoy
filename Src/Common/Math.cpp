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

#include "Math.hpp"

namespace Common::Math
{
    std::tuple<float, float> FitToAspectRatio(float window_width, float window_height,
                                              float target_width, float target_height)
    {
        float final_width = window_width, final_height = window_height;
        auto scaled_w = static_cast<float>(window_width) * (target_height / target_width);
        auto scaled_h = static_cast<float>(window_height) * (target_width / target_height);

        if (scaled_h <= window_width)
            final_width = scaled_h;
        else if (scaled_w <= window_height)
            final_height = scaled_w;

        return std::make_tuple(final_width, final_height);
    }

    void OrthroProject(std::array<float, 16> &matrix, float left, float right, float top,
                       float bottom, float near, float far)
    {
        matrix[0] = 2.0f / (right - left);
        matrix[1] = 0.0f;
        matrix[2] = 0.0f;
        matrix[3] = -((right + left) / (right - left));

        matrix[4] = 0.0f;
        matrix[5] = 2.0f / (top - bottom);
        matrix[6] = 0.0f;
        matrix[7] = -((top + bottom) / (top - bottom));

        matrix[8] = 0.0f;
        matrix[9] = 0.0f;
        matrix[10] = -2.0f / (far - near);
        matrix[11] = -((far + near) / (far - near));

        matrix[12] = 0.0f;
        matrix[13] = 0.0f;
        matrix[14] = 0.0f;
        matrix[15] = 1.0f;
    }
}