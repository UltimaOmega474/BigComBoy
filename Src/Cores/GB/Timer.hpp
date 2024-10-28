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
#include <cinttypes>

namespace GB {
    class Core;

    class Timer {
    public:
        explicit Timer(Core *core);

        auto timer_enabled() const -> bool;
        auto write_register(uint8_t reg, uint8_t value) -> void;
        auto read_register(uint8_t reg) const -> uint8_t;
        auto reset() -> void;
        auto clock(int32_t cycles) -> void;

    private:
        auto set_tac(uint8_t rate) -> void;
        auto reset_div() -> void;
        auto read_div() const -> uint8_t;
        auto change_div(uint16_t new_div) -> void;

        uint8_t tima = 0;
        uint8_t tma = 0;
        uint8_t tac = 0;
        uint16_t tac_rate = 0;
        uint16_t div_cycles = 0;

        Core *core;
    };
}
