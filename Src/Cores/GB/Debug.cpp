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

#include "Debug.hpp"
#include "SM83.hpp"
#include <format>
#include <utility>

namespace GB
{
    void Debugger::push_op_to_history(std::array<uint8_t, 3> &buffer)
    {
        uint8_t x = buffer[0] >> 6;

        std::string ins;

        switch (x)
        {
        case 0:
            ins = decode_x0(buffer);
            break;
        case 1:
            ins = decode_x1(buffer);
            break;
        case 2:
            ins = decode_x2(buffer);
            break;
        case 3:
            ins = decode_x3(buffer);
            break;
        default:
            return;
        }

        instruction_history.push_back(std::move(ins));
    }

    std::string Debugger::decode_x0(std::array<uint8_t, 3> &buffer)
    {
        uint8_t z = buffer[0] & 0x7;

        switch (z)
        {
        case 0:
            return decode_x0_z0(buffer);
        case 1:
            return decode_x0_z1(buffer);
        case 2:
            return decode_x0_z2(buffer);
        case 3:
            return decode_x0_z3(buffer);
        case 4:
            return decode_x0_z4(buffer);
        case 5:
            return decode_x0_z5(buffer);
        case 6:
            return decode_x0_z6(buffer);
        case 7:
            return decode_x0_z7(buffer);
        default:
            return {};
        }
    }

    std::string Debugger::decode_x1(std::array<uint8_t, 3> &buffer)
    {
        uint8_t y = (buffer[0] >> 3) & 0x7;
        uint8_t z = buffer[0] & 0x7;

        if (z == y)
            return std::format("halt");

        return std::format("ld {}, {}", r_to_str(y), r_to_str(z));
    }

    std::string Debugger::decode_x2(std::array<uint8_t, 3> &buffer)
    {
        uint8_t y = (buffer[0] >> 3) & 0x7;
        uint8_t z = buffer[0] & 0x7;

        return std::format("{} {}", alu_to_str(y), r_to_str(z));
    }

    std::string Debugger::decode_x3(std::array<uint8_t, 3> &buffer)
    {
        uint8_t z = buffer[0] & 0x7;

        switch (z)
        {
        case 0:
            return decode_x3_z0(buffer);
        case 1:
            return decode_x3_z1(buffer);
        case 2:
            return decode_x3_z2(buffer);
        case 3:
            return decode_x3_z3(buffer);
        case 4:
            return decode_x3_z4(buffer);
        case 5:
            return decode_x3_z5(buffer);
        case 6:
            return decode_x3_z6(buffer);
        case 7:
            return decode_x3_z7(buffer);
        default:
            return {};
        }
    }

    std::string Debugger::decode_x0_z0(std::array<uint8_t, 3> &buffer)
    {
        uint8_t y = (buffer[0] >> 3) & 0x7;

        switch (y)
        {
        case 0:
            return std::format("nop");
        case 1:
            return std::format("ld ({}), sp", nn_to_str(buffer));
        case 2:
            return std::format("stop");
        case 3:
            return std::format("jr {}", d_to_str(buffer[1]));
        case 4:
        case 5:
        case 6:
        case 7:
            return std::format("jr {}, {}", cc_to_str(y - 4), d_to_str(buffer[1]));
        default:
            return {};
        }
    }

    std::string Debugger::decode_x0_z1(std::array<uint8_t, 3> &buffer)
    {
        uint8_t y = (buffer[0] >> 3) & 0x7;
        uint8_t p = y >> 1;
        uint8_t q = y & 0x1;

        switch (q)
        {
        case 0:
            return std::format("ld {}, {}", rp_to_str(p, false), nn_to_str(buffer));
        case 1:
            return std::format("add hl, {}", rp_to_str(p, false));
        default:
            return {};
        }
    }

    std::string Debugger::decode_x0_z2(std::array<uint8_t, 3> &buffer)
    {
        uint8_t y = (buffer[0] >> 3) & 0x7;
        uint8_t p = y >> 1;
        uint8_t q = y & 0x1;

        switch (q)
        {
        case 0:
        {
            switch (p)
            {
            case 0:
                return std::format("ld (bc), a");
            case 1:
                return std::format("ld (de), a");
            case 2:
                return std::format("ld (hl+), a");
            case 3:
                return std::format("ld (hl-), a");
            default:
                return {};
            }
        }
        case 1:
        {
            switch (p)
            {
            case 0:
                return std::format("ld a, (bc)");
            case 1:
                return std::format("ld a, (de)");
            case 2:
                return std::format("ld a, (hl+)");
            case 3:
                return std::format("ld a, (hl-)");
            default:
                return {};
            }
        }
        default:
            return {};
        }
    }

    std::string Debugger::decode_x0_z3(std::array<uint8_t, 3> &buffer)
    {
        uint8_t y = (buffer[0] >> 3) & 0x7;
        uint8_t p = y >> 1;
        uint8_t q = y & 0x1;

        switch (q)
        {
        case 0:
            return std::format("inc {}", rp_to_str(p, false));
        case 1:
            return std::format("dec {}", rp_to_str(p, false));
        default:
            return {};
        }
    }

    std::string Debugger::decode_x0_z4(std::array<uint8_t, 3> &buffer)
    {
        uint8_t y = (buffer[0] >> 3) & 0x7;

        return std::format("inc {}", r_to_str(y));
    }

    std::string Debugger::decode_x0_z5(std::array<uint8_t, 3> &buffer)
    {
        uint8_t y = (buffer[0] >> 3) & 0x7;

        return std::format("dec {}", r_to_str(y));
    }

    std::string Debugger::decode_x0_z6(std::array<uint8_t, 3> &buffer)
    {
        uint8_t y = (buffer[0] >> 3) & 0x7;

        return std::format("ld {}, {}", r_to_str(y), n_to_str(buffer[1]));
    }

    std::string Debugger::decode_x0_z7(std::array<uint8_t, 3> &buffer)
    {
        uint8_t y = (buffer[0] >> 3) & 0x7;

        switch (y)
        {
        case 0:
            return std::format("rlca");
        case 1:
            return std::format("rrca");
        case 2:
            return std::format("rla");
        case 3:
            return std::format("rra");
        case 4:
            return std::format("daa");
        case 5:
            return std::format("cpl");
        case 6:
            return std::format("scf");
        case 7:
            return std::format("ccf");
        default:
            return {};
        }
    }

    std::string Debugger::decode_x3_z0(std::array<uint8_t, 3> &buffer)
    {
        uint8_t y = (buffer[0] >> 3) & 0x7;

        switch (y)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            return std::format("ret {}", cc_to_str(y));
        case 4:
            return std::format("ld ($FF00 + {}), a", n_to_str(buffer[1]));
        case 5:
            return std::format("add sp, {}", d_to_str(buffer[1]));
        case 6:
            return std::format("ld a, ($FF00 + {})", n_to_str(buffer[1]));
        case 7:
            return std::format("ld hl, sp+{}", d_to_str(buffer[1]));
        default:
            return {};
        }
    }

    std::string Debugger::decode_x3_z1(std::array<uint8_t, 3> &buffer)
    {
        uint8_t y = (buffer[0] >> 3) & 0x7;
        uint8_t p = y >> 1;
        uint8_t q = y & 0x1;

        switch (q)
        {
        case 0:
        {
            return std::format("pop {}", rp_to_str(p, true));
        }
        case 1:
        {
            switch (p)
            {
            case 0:
                return std::format("ret");
            case 1:
                return std::format("reti");
            case 2:
                return std::format("jp hl");
            case 3:
                return std::format("ld sp, hl");
            default:
                return {};
            }
        }
        default:
            return {};
        }
    }

    std::string Debugger::decode_x3_z2(std::array<uint8_t, 3> &buffer)
    {
        uint8_t y = (buffer[0] >> 3) & 0x7;

        switch (y)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            return std::format("jp {}, {}", cc_to_str(y), nn_to_str(buffer));

        case 4:
            return std::format("ld ($FF00+c), a");
        case 5:
            return std::format("ld ({}), a", nn_to_str(buffer));
        case 6:
            return std::format("ld a, ($FF00+c)");
        case 7:
            return std::format("ld a, ({})", nn_to_str(buffer));
        default:
            return {};
        }
    }

    std::string Debugger::decode_x3_z3(std::array<uint8_t, 3> &buffer)
    {
        uint8_t y = (buffer[0] >> 3) & 0x7;

        switch (y)
        {
        case 0:
            return std::format("jp {}", nn_to_str(buffer));
        case 1:
            return decode_cb(buffer[0]);
        case 2:
        case 3:
        case 4:
        case 5:
            return std::format("illegal opcode {}", n_to_str(buffer[0]));
        case 6:
            return std::format("di");
        case 7:
            return std::format("ei");
        default:
            return {};
        }
    }

    std::string Debugger::decode_x3_z4(std::array<uint8_t, 3> &buffer)
    {
        uint8_t y = (buffer[0] >> 3) & 0x7;

        switch (y)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            return std::format("call {}, {}", cc_to_str(y), nn_to_str(buffer));
        case 4:
        case 5:
        case 6:
        case 7:
            return std::format("illegal opcode {}", n_to_str(buffer[0]));
        default:
            return {};
        }
    }

    std::string Debugger::decode_x3_z5(std::array<uint8_t, 3> &buffer)
    {
        uint8_t y = (buffer[0] >> 3) & 0x7;
        uint8_t p = y >> 1;
        uint8_t q = y & 0x1;

        switch (q)
        {
        case 0:
        {
            return std::format("push {}", rp_to_str(p, true));
        }
        case 1:
        {
            switch (p)
            {
            case 0:
                return std::format("call {}", nn_to_str(buffer));
            case 1:
            case 2:
            case 3:
                return std::format("illegal opcode {}", n_to_str(buffer[0]));
            default:
                return {};
            }
        }
        default:
            return {};
        }
    }

    std::string Debugger::decode_x3_z6(std::array<uint8_t, 3> &buffer)
    {
        uint8_t y = (buffer[0] >> 3) & 0x7;

        return std::format("{} {}", alu_to_str(y), n_to_str(buffer[1]));
    }

    std::string Debugger::decode_x3_z7(std::array<uint8_t, 3> &buffer)
    {
        uint8_t y = (buffer[0] >> 3) & 0x7;
        return std::format("rst {}", n_to_str(y * 8));
    }

    std::string Debugger::decode_cb(uint8_t opcode)
    {
        uint8_t x = opcode >> 6;
        uint8_t y = (opcode >> 3) & 0x7;
        uint8_t z = opcode & 0x7;

        switch (x)
        {
        case 0:
            return std::format("{} {}", rot_to_str(y), r_to_str(z));
        case 1:
            return std::format("bit {} {}", y, r_to_str(z));
        case 2:
            return std::format("res {} {}", y, r_to_str(z));
        case 3:
            return std::format("set {} {}", y, r_to_str(z));
        default:
            return {};
        }
    }

    std::string Debugger::r_to_str(uint8_t r)
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

    std::string Debugger::rp_to_str(uint8_t rp, bool is_table2)
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

    std::string Debugger::cc_to_str(uint8_t cc)
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

    std::string Debugger::alu_to_str(uint8_t alu)
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

    std::string Debugger::rot_to_str(uint8_t rot)
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

    std::string Debugger::nn_to_str(std::array<uint8_t, 3> &buffer)
    {
        uint16_t low = buffer[1];
        uint16_t high = buffer[2];

        uint16_t combined = low | (high << 8);

        return std::format("${:x}", combined);
    }

    std::string Debugger::n_to_str(uint8_t n) { return std::format("{:x}", n); }

    std::string Debugger::d_to_str(uint8_t d)
    {
        return std::format("{:x}", static_cast<int8_t>(d));
    }
}