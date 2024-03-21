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
#include <queue>
#include <string>
#include <vector>

namespace GB
{
    class MainBus;

    struct Instruction
    {
        uint8_t len = 0;
        std::array<uint8_t, 3> bytes;
        uint16_t program_counter = 0;
    };

    class Disassembler
    {
        std::queue<Instruction> history;
        std::vector<Instruction> future;

        uint8_t max_history = 10, max_future = 5;

    public:
        static std::string DecodeInstruction(const std::array<uint8_t, 3> &bytes);

        void set_limits(uint8_t history_limit, uint8_t future_limit);
        void clear();
        void push_instruction(uint16_t pc, uint8_t len, const std::array<uint8_t, 3> &bytes);
        void scan_next_instructions(uint16_t pc, MainBus &bus);

    private:
        static std::string DecodeX0(const std::array<uint8_t, 3> &bytes);
        static std::string DecodeX1(const std::array<uint8_t, 3> &bytes);
        static std::string DecodeX2(const std::array<uint8_t, 3> &bytes);
        static std::string DecodeX3(const std::array<uint8_t, 3> &bytes);

        static std::string DecodeX0Z0(const std::array<uint8_t, 3> &bytes);
        static std::string DecodeX0Z1(const std::array<uint8_t, 3> &bytes);
        static std::string DecodeX0Z2(const std::array<uint8_t, 3> &bytes);
        static std::string DecodeX0Z3(const std::array<uint8_t, 3> &bytes);
        static std::string DecodeX0Z4(const std::array<uint8_t, 3> &bytes);
        static std::string DecodeX0Z5(const std::array<uint8_t, 3> &bytes);
        static std::string DecodeX0Z6(const std::array<uint8_t, 3> &bytes);
        static std::string DecodeX0Z7(const std::array<uint8_t, 3> &bytes);

        static std::string DecodeX3Z0(const std::array<uint8_t, 3> &bytes);
        static std::string DecodeX3Z1(const std::array<uint8_t, 3> &bytes);
        static std::string DecodeX3Z2(const std::array<uint8_t, 3> &bytes);
        static std::string DecodeX3Z3(const std::array<uint8_t, 3> &bytes);
        static std::string DecodeX3Z4(const std::array<uint8_t, 3> &bytes);
        static std::string DecodeX3Z5(const std::array<uint8_t, 3> &bytes);
        static std::string DecodeX3Z6(const std::array<uint8_t, 3> &bytes);
        static std::string DecodeX3Z7(const std::array<uint8_t, 3> &bytes);

        static std::string DecodeCB(uint8_t opcode);

        static std::string RToStr(uint8_t r);
        static std::string RPToStr(uint8_t rp, bool is_table2);
        static std::string CCToStr(uint8_t cc);
        static std::string ALUToStr(uint8_t alu);
        static std::string ROTToStr(uint8_t rot);
        static std::string NNToStr(const std::array<uint8_t, 3> &bytes);
        static std::string NToStr(uint8_t n);
        static std::string DToStr(uint8_t d);
    };

}