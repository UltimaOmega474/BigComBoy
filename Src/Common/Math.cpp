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

    void AlphaComposite(std::span<uint8_t> b, std::span<uint8_t> a, std::span<uint8_t> out,
                        size_t width, size_t height, size_t bit_depth)
    {
        auto cvt = [](int c) -> float
        {
            return (float)c / 255.0f;
        };

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                auto i = ((y * width) * 4) + (x * 4);
                auto ar = cvt(a[i]);
                auto ag = cvt(a[i + 1]);
                auto ab = cvt(a[i + 2]);
                auto aa = cvt(a[i + 3]);
                auto br = cvt(b[i]);
                auto bg = cvt(b[i + 1]);
                auto bb = cvt(b[i + 2]);
                auto ba = cvt(b[i + 3]);

                auto cr = ar + br * (1.0f - aa);
                auto cg = ag + bg * (1.0f - aa);
                auto cb = ab + bb * (1.0f - aa);
                auto ca = aa + ba * (1.0f - aa);

                out[i] = cr * 255.0f;
                out[i + 1] = cg * 255.0f;
                out[i + 2] = cb * 255.0f;
                out[i + 3] = ca * 255.0f;
            }
        }
    }
}