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

#include "Disassembler.hpp"
#include <format>

namespace GB
{
    std::string DecodeInstruction(std::span<uint8_t, 3> bytes)
    {
        uint8_t x = bytes[0] >> 6;

        switch (x)
        {
        case 0:
            return DecodeX0(bytes);
        case 1:
            return DecodeX1(bytes);
        case 2:
            return DecodeX2(bytes);
        case 3:
            return DecodeX3(bytes);
        default:
            return {};
        }
    }

    std::string DecodeX0(std::span<uint8_t, 3> bytes)
    {
        uint8_t z = bytes[0] & 0x7;

        switch (z)
        {
        case 0:
            return DecodeX0Z0(bytes);
        case 1:
            return DecodeX0Z1(bytes);
        case 2:
            return DecodeX0Z2(bytes);
        case 3:
            return DecodeX0Z3(bytes);
        case 4:
            return DecodeX0Z4(bytes);
        case 5:
            return DecodeX0Z5(bytes);
        case 6:
            return DecodeX0Z6(bytes);
        case 7:
            return DecodeX0Z7(bytes);
        default:
            return {};
        }
    }

    std::string DecodeX1(std::span<uint8_t, 3> bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;
        uint8_t z = bytes[0] & 0x7;

        if (z == y)
            return "halt";

        return std::format("ld {}, {}", RToStr(y), RToStr(z));
    }

    std::string DecodeX2(std::span<uint8_t, 3> bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;
        uint8_t z = bytes[0] & 0x7;

        return std::format("{} {}", ALUToStr(y), RToStr(z));
    }

    std::string DecodeX3(std::span<uint8_t, 3> bytes)
    {
        uint8_t z = bytes[0] & 0x7;

        switch (z)
        {
        case 0:
            return DecodeX3Z0(bytes);
        case 1:
            return DecodeX3Z1(bytes);
        case 2:
            return DecodeX3Z2(bytes);
        case 3:
            return DecodeX3Z3(bytes);
        case 4:
            return DecodeX3Z4(bytes);
        case 5:
            return DecodeX3Z5(bytes);
        case 6:
            return DecodeX3Z6(bytes);
        case 7:
            return DecodeX3Z7(bytes);
        default:
            return {};
        }
    }

    std::string DecodeX0Z0(std::span<uint8_t, 3> bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;

        switch (y)
        {
        case 0:
            return "nop";
        case 1:
            return std::format("ld ({}), sp", NNToStr(bytes));
        case 2:
            return "stop";
        case 3:
            return std::format("jr {}", DToStr(bytes[1]));
        case 4:
        case 5:
        case 6:
        case 7:
            return std::format("jr {}, {}", CCToStr(y - 4), DToStr(bytes[1]));
        default:
            return {};
        }
    }

    std::string DecodeX0Z1(std::span<uint8_t, 3> bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;
        uint8_t p = y >> 1;
        uint8_t q = y & 0x1;

        switch (q)
        {
        case 0:
            return std::format("ld {}, {}", RPToStr(p, false), NNToStr(bytes));
        case 1:
            return std::format("add hl, {}", RPToStr(p, false));
        default:
            return {};
        }
    }

    std::string DecodeX0Z2(std::span<uint8_t, 3> bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;
        uint8_t p = y >> 1;
        uint8_t q = y & 0x1;

        switch (q)
        {
        case 0:
        {
            switch (p)
            {
            case 0:
                return "ld (bc), a";
            case 1:
                return "ld (de), a";
            case 2:
                return "ld (hl+), a";
            case 3:
                return "ld (hl-), a";
            default:
                return {};
            }
        }
        case 1:
        {
            switch (p)
            {
            case 0:
                return "ld a, (bc)";
            case 1:
                return "ld a, (de)";
            case 2:
                return "ld a, (hl+)";
            case 3:
                return "ld a, (hl-)";
            default:
                return {};
            }
        }
        default:
            return {};
        }
    }

    std::string DecodeX0Z3(std::span<uint8_t, 3> bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;
        uint8_t p = y >> 1;
        uint8_t q = y & 0x1;

        switch (q)
        {
        case 0:
            return std::format("inc {}", RPToStr(p, false));
        case 1:
            return std::format("dec {}", RPToStr(p, false));
        default:
            return {};
        }
    }

    std::string DecodeX0Z4(std::span<uint8_t, 3> bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;

        return std::format("inc {}", RToStr(y));
    }

    std::string DecodeX0Z5(std::span<uint8_t, 3> bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;

        return std::format("dec {}", RToStr(y));
    }

    std::string DecodeX0Z6(std::span<uint8_t, 3> bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;

        return std::format("ld {}, {}", RToStr(y), NToStr(bytes[1]));
    }

    std::string DecodeX0Z7(std::span<uint8_t, 3> bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;

        switch (y)
        {
        case 0:
            return "rlca";
        case 1:
            return "rrca";
        case 2:
            return "rla";
        case 3:
            return "rra";
        case 4:
            return "daa";
        case 5:
            return "cpl";
        case 6:
            return "scf";
        case 7:
            return "ccf";
        default:
            return {};
        }
    }

    std::string DecodeX3Z0(std::span<uint8_t, 3> bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;

        switch (y)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            return std::format("ret {}", CCToStr(y));
        case 4:
            return std::format("ldh ({}), a", NToStr(bytes[1]));
        case 5:
            return std::format("add sp, {}", DToStr(bytes[1]));
        case 6:
            return std::format("ldh a, ({})", NToStr(bytes[1]));
        case 7:
            return std::format("ld hl, sp+{}", DToStr(bytes[1]));
        default:
            return {};
        }
    }

    std::string DecodeX3Z1(std::span<uint8_t, 3> bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;
        uint8_t p = y >> 1;
        uint8_t q = y & 0x1;

        switch (q)
        {
        case 0:
        {
            return std::format("pop {}", RPToStr(p, true));
        }
        case 1:
        {
            switch (p)
            {
            case 0:
                return "ret";
            case 1:
                return "reti";
            case 2:
                return "jp hl";
            case 3:
                return "ld sp, hl";
            default:
                return {};
            }
        }
        default:
            return {};
        }
    }

    std::string DecodeX3Z2(std::span<uint8_t, 3> bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;

        switch (y)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            return std::format("jp {}, {}", CCToStr(y), NNToStr(bytes));

        case 4:
            return "ldh (c), a";
        case 5:
            return std::format("ld ({}), a", NNToStr(bytes));
        case 6:
            return "ldh a, (c)";
        case 7:
            return std::format("ld a, ({})", NNToStr(bytes));
        default:
            return {};
        }
    }

    std::string DecodeX3Z3(std::span<uint8_t, 3> bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;

        switch (y)
        {
        case 0:
            return std::format("jp {}", NNToStr(bytes));
        case 1:
            return DecodeCB(bytes[0]);
        case 2:
        case 3:
        case 4:
        case 5:
            return std::format("illegal opcode {}", NToStr(bytes[0]));
        case 6:
            return "di";
        case 7:
            return "ei";
        default:
            return {};
        }
    }

    std::string DecodeX3Z4(std::span<uint8_t, 3> bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;

        switch (y)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            return std::format("call {}, {}", CCToStr(y), NNToStr(bytes));
        case 4:
        case 5:
        case 6:
        case 7:
            return std::format("illegal opcode {}", NToStr(bytes[0]));
        default:
            return {};
        }
    }

    std::string DecodeX3Z5(std::span<uint8_t, 3> bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;
        uint8_t p = y >> 1;
        uint8_t q = y & 0x1;

        switch (q)
        {
        case 0:
        {
            return std::format("push {}", RPToStr(p, true));
        }
        case 1:
        {
            switch (p)
            {
            case 0:
                return std::format("call {}", NNToStr(bytes));
            case 1:
            case 2:
            case 3:
                return std::format("illegal opcode {}", NToStr(bytes[0]));
            default:
                return {};
            }
        }
        default:
            return {};
        }
    }

    std::string DecodeX3Z6(std::span<uint8_t, 3> bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;

        return std::format("{} {}", ALUToStr(y), NToStr(bytes[1]));
    }

    std::string DecodeX3Z7(std::span<uint8_t, 3> bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;
        return std::format("rst {}", NToStr(y * 8));
    }

    std::string DecodeCB(uint8_t opcode)
    {
        uint8_t x = opcode >> 6;
        uint8_t y = (opcode >> 3) & 0x7;
        uint8_t z = opcode & 0x7;

        switch (x)
        {
        case 0:
            return std::format("{} {}", ROTToStr(y), RToStr(z));
        case 1:
            return std::format("bit {} {}", y, RToStr(z));
        case 2:
            return std::format("res {} {}", y, RToStr(z));
        case 3:
            return std::format("set {} {}", y, RToStr(z));
        default:
            return {};
        }
    }

    std::string RToStr(uint8_t r)
    {
        switch (r & 0x7)
        {
        case 0:
            return "b";
        case 1:
            return "c";
        case 2:
            return "d";
        case 3:
            return "e";
        case 4:
            return "h";
        case 5:
            return "l";
        case 6:
            return "(hl)";
        case 7:
            return "a";

        default:
            return {};
        }
    }

    std::string RPToStr(uint8_t rp, bool is_table2)
    {
        switch (rp & 0x3)
        {
        case 0:
            return "bc";
        case 1:
            return "de";
        case 2:
            return "hl";
        case 3:
            return is_table2 ? "af" : "sp";

        default:
            return {};
        }
    }

    std::string CCToStr(uint8_t cc)
    {
        switch (cc & 0x3)
        {
        case 0:
            return "nz";
        case 1:
            return "z";
        case 2:
            return "nc";
        case 3:
            return "c";

        default:
            return {};
        }
    }

    std::string ALUToStr(uint8_t alu)
    {
        switch (alu & 0x7)
        {
        case 0:
            return "add a,";
        case 1:
            return "adc a,";
        case 2:
            return "sub a,";
        case 3:
            return "sbc a,";
        case 4:
            return "and a,";
        case 5:
            return "xor a,";
        case 6:
            return "or a,";
        case 7:
            return "cp a,";

        default:
            return {};
        }
    }

    std::string ROTToStr(uint8_t rot)
    {
        switch (rot & 0x7)
        {
        case 0:
            return "rlc";
        case 1:
            return "rrc";
        case 2:
            return "rl";
        case 3:
            return "rr";
        case 4:
            return "sla";
        case 5:
            return "sra";
        case 6:
            return "swap";
        case 7:
            return "srl";

        default:
            return {};
        }
    }

    std::string NNToStr(std::span<uint8_t, 3> bytes)
    {
        uint16_t low = bytes[1];
        uint16_t high = bytes[2];

        uint16_t combined = low | (high << 8);

        return std::format("${:04X}", combined);
    }

    std::string NToStr(uint8_t n) { return std::format("{:02X}", n); }

    std::string DToStr(uint8_t d) { return std::format("{:02X}", static_cast<int8_t>(d)); }
}