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
#include <string>
#include <vector>

namespace GB
{
    struct BreakPoint
    {
        uint16_t program_counter = 0;
    };

    class Debugger
    {
        std::vector<std::string> instruction_history;

    public:
        void push_op_to_history(std::array<uint8_t, 3> &buffer);

    private:
        static std::string decode_x0(std::array<uint8_t, 3> &buffer);
        static std::string decode_x1(std::array<uint8_t, 3> &buffer);
        static std::string decode_x2(std::array<uint8_t, 3> &buffer);
        static std::string decode_x3(std::array<uint8_t, 3> &buffer);

        static std::string decode_x0_z0(std::array<uint8_t, 3> &buffer);
        static std::string decode_x0_z1(std::array<uint8_t, 3> &buffer);
        static std::string decode_x0_z2(std::array<uint8_t, 3> &buffer);
        static std::string decode_x0_z3(std::array<uint8_t, 3> &buffer);
        static std::string decode_x0_z4(std::array<uint8_t, 3> &buffer);
        static std::string decode_x0_z5(std::array<uint8_t, 3> &buffer);
        static std::string decode_x0_z6(std::array<uint8_t, 3> &buffer);
        static std::string decode_x0_z7(std::array<uint8_t, 3> &buffer);

        static std::string decode_x3_z0(std::array<uint8_t, 3> &buffer);
        static std::string decode_x3_z1(std::array<uint8_t, 3> &buffer);
        static std::string decode_x3_z2(std::array<uint8_t, 3> &buffer);
        static std::string decode_x3_z3(std::array<uint8_t, 3> &buffer);
        static std::string decode_x3_z4(std::array<uint8_t, 3> &buffer);
        static std::string decode_x3_z5(std::array<uint8_t, 3> &buffer);
        static std::string decode_x3_z6(std::array<uint8_t, 3> &buffer);
        static std::string decode_x3_z7(std::array<uint8_t, 3> &buffer);

        static std::string decode_cb(uint8_t opcode);

        static std::string r_to_str(uint8_t r);
        static std::string rp_to_str(uint8_t rp, bool is_table2);
        static std::string cc_to_str(uint8_t cc);
        static std::string alu_to_str(uint8_t alu);
        static std::string rot_to_str(uint8_t rot);
        static std::string nn_to_str(std::array<uint8_t, 3> &buffer);
        static std::string n_to_str(uint8_t n);
        static std::string d_to_str(uint8_t d);
    };

}