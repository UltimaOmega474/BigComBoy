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

#include "Trace.hpp"
#include "Disassembler.hpp"
#include <format>

namespace GB
{
    std::deque<Instruction> &TraceLogger::get_history() { return history; }

    int32_t TraceLogger::get_line_limit() const { return max_history; }

    void TraceLogger::set_limit(int32_t lines)
    {
        max_history = lines;

        while (history.size() > max_history)
            history.pop_front();
    }

    void TraceLogger::clear() { history.clear(); }

    void TraceLogger::push_instruction(uint16_t pc, uint8_t len, std::span<uint8_t, 3> bytes)
    {
        std::array<uint8_t, 3> copied_bytes;

        std::copy(bytes.begin(), bytes.end(), copied_bytes.begin());

        history.push_back(Instruction{
            .pc = pc,
            .len = len,
            .bytes = std::move(copied_bytes),
        });

        if (history.size() > max_history)
            history.pop_front();
    }
}