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
#include <cinttypes>

namespace GB
{
    enum class Console
    {
        DMG,
        CGB
    };

    constexpr int32_t DISABLE_CGB_FUNCTIONS = 0x4;
    constexpr int32_t LCD_WIDTH = 160;
    constexpr int32_t LCD_HEIGHT = 144;
    constexpr int32_t COLOR_DEPTH = 4;
    constexpr uint8_t INT_VBLANK_BIT = 0x01;
    constexpr uint8_t INT_LCD_STAT_BIT = 0x02;
    constexpr uint8_t INT_TIMER_BIT = 0x04;
    constexpr uint8_t INT_SERIAL_PORT_BIT = 0x08;
    constexpr uint8_t INT_JOYPAD_BIT = 0x10;
    constexpr uint32_t CPU_CLOCK_RATE = 4194304;
    constexpr uint32_t CYCLES_PER_FRAME = 70224;
    constexpr int16_t NO_DISPLACEMENT = 0;
    constexpr int16_t INCREMENT = 1;
    constexpr int16_t DECREMENT = -1;
    constexpr auto WITH_CARRY = true;
    constexpr auto WITHOUT_CARRY = false;
    constexpr auto DONT_SET_IME = false;
    constexpr auto SET_IME = true;

    constexpr std::array<uint16_t, 8> NOISE_DIV{
        2, 4, 8, 12, 16, 20, 24, 28,
    };

    constexpr std::array<uint8_t, 4> WAVE_VOLUME{
        4,
        0,
        1,
        2,
    };

    constexpr std::array<std::array<uint8_t, 8>, 4> DUTY_TABLE{
        std::array<uint8_t, 8>{0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, 8>{1, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, 8>{1, 0, 0, 0, 0, 1, 1, 1},
        std::array<uint8_t, 8>{0, 1, 1, 1, 1, 1, 1, 0},
    };

    consteval uint16_t RGB555ToUInt(uint16_t r, uint16_t g, uint16_t b)
    {
        r &= 0x1F;
        g &= 0x1F;
        b &= 0x1F;

        return (r) | (g << 5) | (b << 10);
    }
}