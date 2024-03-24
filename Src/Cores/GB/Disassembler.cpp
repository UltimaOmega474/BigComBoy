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
#include "Bus.hpp"
#include "SM83.hpp"
#include <format>

namespace GB
{
    std::array<uint8_t, 256> OPCODE_SIZES{
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

    std::string Disassembler::DecodeInstruction(const std::array<uint8_t, 3> &bytes)
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

    const std::deque<Instruction> &Disassembler::get_history() const { return history; }

    const std::vector<Instruction> &Disassembler::get_upcoming() const { return future; }

    void Disassembler::set_limits(uint8_t history_limit, uint8_t future_limit)
    {
        max_history = history_limit;
        max_future = future_limit;

        while (history.size() > max_history)
            history.pop_front();

        if (future.size() > max_future)
            future.resize(max_future);
    }

    void Disassembler::clear()
    {
        while (history.size())
            history.pop_front();

        future.clear();
    }

    void Disassembler::push_instruction(uint16_t pc, uint8_t len,
                                        const std::array<uint8_t, 3> &bytes)
    {
        Instruction ins{
            .len = len,
            .bytes = bytes,
            .program_counter = pc,
        };

        history.push_back(std::move(ins));

        if (history.size() > max_history)
            history.pop_front();
    }

    void Disassembler::scan_next_instructions(uint16_t pc, MainBus &bus)
    {
        future.clear();

        for (size_t i = 0; i < max_future; ++i)
        {
            uint16_t starting_pc = pc;
            std::array<uint8_t, 3> bytes{
                bus.read(pc++),
                0,
                0,
            };

            uint8_t len = OPCODE_SIZES[bytes[0]];

            for (size_t j = 1; j < len; j++)
                bytes[j] = bus.read(pc++);

            Instruction ins{
                .len = len,
                .bytes = bytes,
                .program_counter = starting_pc,
            };

            future.push_back(std::move(ins));
        }
    }

    std::string Disassembler::DecodeX0(const std::array<uint8_t, 3> &bytes)
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

    std::string Disassembler::DecodeX1(const std::array<uint8_t, 3> &bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;
        uint8_t z = bytes[0] & 0x7;

        if (z == y)
            return "halt";

        return std::format("ld {}, {}", RToStr(y), RToStr(z));
    }

    std::string Disassembler::DecodeX2(const std::array<uint8_t, 3> &bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;
        uint8_t z = bytes[0] & 0x7;

        return std::format("{} {}", ALUToStr(y), RToStr(z));
    }

    std::string Disassembler::DecodeX3(const std::array<uint8_t, 3> &bytes)
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

    std::string Disassembler::DecodeX0Z0(const std::array<uint8_t, 3> &bytes)
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

    std::string Disassembler::DecodeX0Z1(const std::array<uint8_t, 3> &bytes)
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

    std::string Disassembler::DecodeX0Z2(const std::array<uint8_t, 3> &bytes)
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

    std::string Disassembler::DecodeX0Z3(const std::array<uint8_t, 3> &bytes)
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

    std::string Disassembler::DecodeX0Z4(const std::array<uint8_t, 3> &bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;

        return std::format("inc {}", RToStr(y));
    }

    std::string Disassembler::DecodeX0Z5(const std::array<uint8_t, 3> &bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;

        return std::format("dec {}", RToStr(y));
    }

    std::string Disassembler::DecodeX0Z6(const std::array<uint8_t, 3> &bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;

        return std::format("ld {}, {}", RToStr(y), NToStr(bytes[1]));
    }

    std::string Disassembler::DecodeX0Z7(const std::array<uint8_t, 3> &bytes)
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

    std::string Disassembler::DecodeX3Z0(const std::array<uint8_t, 3> &bytes)
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

    std::string Disassembler::DecodeX3Z1(const std::array<uint8_t, 3> &bytes)
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

    std::string Disassembler::DecodeX3Z2(const std::array<uint8_t, 3> &bytes)
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

    std::string Disassembler::DecodeX3Z3(const std::array<uint8_t, 3> &bytes)
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

    std::string Disassembler::DecodeX3Z4(const std::array<uint8_t, 3> &bytes)
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

    std::string Disassembler::DecodeX3Z5(const std::array<uint8_t, 3> &bytes)
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

    std::string Disassembler::DecodeX3Z6(const std::array<uint8_t, 3> &bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;

        return std::format("{} {}", ALUToStr(y), NToStr(bytes[1]));
    }

    std::string Disassembler::DecodeX3Z7(const std::array<uint8_t, 3> &bytes)
    {
        uint8_t y = (bytes[0] >> 3) & 0x7;
        return std::format("rst {}", NToStr(y * 8));
    }

    std::string Disassembler::DecodeCB(uint8_t opcode)
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

    std::string Disassembler::RToStr(uint8_t r)
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

    std::string Disassembler::RPToStr(uint8_t rp, bool is_table2)
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

    std::string Disassembler::CCToStr(uint8_t cc)
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

    std::string Disassembler::ALUToStr(uint8_t alu)
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

    std::string Disassembler::ROTToStr(uint8_t rot)
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

    std::string Disassembler::NNToStr(const std::array<uint8_t, 3> &bytes)
    {
        uint16_t low = bytes[1];
        uint16_t high = bytes[2];

        uint16_t combined = low | (high << 8);

        return std::format("${:04X}", combined);
    }

    std::string Disassembler::NToStr(uint8_t n) { return std::format("{:02X}", n); }

    std::string Disassembler::DToStr(uint8_t d)
    {
        return std::format("{:02X}", static_cast<int8_t>(d));
    }
}