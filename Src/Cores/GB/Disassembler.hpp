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
#include <span>
#include <string>

namespace GB
{
    constexpr std::array<uint8_t, 256> OPCODE_SIZES{
        1, 3, 1, 1, 1, 1, 2, 1, 3, 1, 1, 1, 1, 1, 2, 1, // 0x00 - 0x0F
        1, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1, // 0x10 - 0x1F
        2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1, // 0x20 - 0x2F
        2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1, // 0x30 - 0x3F
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40 - 0x4F
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x50 - 0x5F
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60 - 0x6F
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x70 - 0x7F
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x80 - 0x8F
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x90 - 0x9F
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xA0 - 0xAF
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xB0 - 0xBF
        1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 2, 3, 3, 2, 1, // 0xC0 - 0xCF
        1, 1, 3, 0, 3, 1, 2, 1, 1, 1, 3, 0, 3, 0, 2, 1, // 0xD0 - 0xDF
        2, 1, 1, 0, 0, 1, 2, 1, 2, 1, 3, 0, 0, 0, 2, 1, // 0xE0 - 0xEF
        2, 1, 1, 1, 0, 1, 2, 1, 2, 1, 3, 1, 0, 0, 2, 1, // 0xF0 - 0xFF
    };

    std::string DecodeInstruction(std::span<uint8_t, 3> bytes);
    std::string DecodeX0(std::span<uint8_t, 3> bytes);
    std::string DecodeX1(std::span<uint8_t, 3> bytes);
    std::string DecodeX2(std::span<uint8_t, 3> bytes);
    std::string DecodeX3(std::span<uint8_t, 3> bytes);

    std::string DecodeX0Z0(std::span<uint8_t, 3> bytes);
    std::string DecodeX0Z1(std::span<uint8_t, 3> bytes);
    std::string DecodeX0Z2(std::span<uint8_t, 3> bytes);
    std::string DecodeX0Z3(std::span<uint8_t, 3> bytes);
    std::string DecodeX0Z4(std::span<uint8_t, 3> bytes);
    std::string DecodeX0Z5(std::span<uint8_t, 3> bytes);
    std::string DecodeX0Z6(std::span<uint8_t, 3> bytes);
    std::string DecodeX0Z7(std::span<uint8_t, 3> bytes);

    std::string DecodeX3Z0(std::span<uint8_t, 3> bytes);
    std::string DecodeX3Z1(std::span<uint8_t, 3> bytes);
    std::string DecodeX3Z2(std::span<uint8_t, 3> bytes);
    std::string DecodeX3Z3(std::span<uint8_t, 3> bytes);
    std::string DecodeX3Z4(std::span<uint8_t, 3> bytes);
    std::string DecodeX3Z5(std::span<uint8_t, 3> bytes);
    std::string DecodeX3Z6(std::span<uint8_t, 3> bytes);
    std::string DecodeX3Z7(std::span<uint8_t, 3> bytes);

    std::string DecodeCB(uint8_t opcode);

    std::string RToStr(uint8_t r);
    std::string RPToStr(uint8_t rp, bool is_table2);
    std::string CCToStr(uint8_t cc);
    std::string ALUToStr(uint8_t alu);
    std::string ROTToStr(uint8_t rot);
    std::string NNToStr(std::span<uint8_t, 3> bytes);
    std::string NToStr(uint8_t n);
    std::string DToStr(uint8_t d);
}