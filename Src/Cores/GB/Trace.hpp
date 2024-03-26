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
#include <deque>
#include <span>
#include <string>

namespace GB
{
    struct Instruction
    {
        uint16_t pc = 0;
        uint8_t len = 0;
        std::array<uint8_t, 3> bytes{};
    };

    class TraceLogger
    {
        int32_t max_history = 10000;
        std::deque<Instruction> history;

    public:
        std::deque<Instruction> &get_history();
        int32_t get_line_limit() const;
        void set_limit(int32_t lines);

        void clear();
        void push_instruction(uint16_t pc, uint8_t len, std::span<uint8_t, 3> bytes);
    };
}