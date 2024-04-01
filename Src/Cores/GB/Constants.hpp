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

namespace GB {
    constexpr int32_t DISABLE_CGB_FUNCTIONS = 0x4;
    constexpr int32_t LCD_WIDTH = 160;
    constexpr int32_t LCD_HEIGHT = 144;
    constexpr int32_t FRAMEBUFFER_COLOR_CHANNELS = 4;
    constexpr uint8_t INT_VBLANK_BIT = 0x01;
    constexpr uint8_t INT_LCD_STAT_BIT = 0x02;
    constexpr uint8_t INT_TIMER_BIT = 0x04;
    constexpr uint8_t INT_SERIAL_PORT_BIT = 0x08;
    constexpr uint8_t INT_JOYPAD_BIT = 0x10;
    constexpr int32_t CPU_CLOCK_RATE = 4194304;
    constexpr int32_t CYCLES_PER_FRAME = 70224;

    enum class ConsoleType {
        DMG,
        CGB,
        AutoSelect,
    };

    consteval uint16_t RGB555ToUInt(uint16_t r, uint16_t g, uint16_t b) {
        r &= 0x1F;
        g &= 0x1F;
        b &= 0x1F;

        return (r) | (g << 5) | (b << 10);
    }

    constexpr std::array<uint16_t, 4> LCD_GRAY{
        RGB555ToUInt(31, 31, 31),
        RGB555ToUInt(21, 21, 21),
        RGB555ToUInt(10, 10, 10),
        RGB555ToUInt(0, 0, 0),
    };
}