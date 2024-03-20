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

namespace GB
{
    class Core;
    class MainBus;
    enum Register
    {
        B = 0,
        C = 1,
        D = 2,
        E = 3,
        H = 4,
        L = 5,
        HL_ADDR = 6,
        F = 6,
        A = 7,
        // for ALU opcodes that take an immediate value
        U8,
    };

    enum RegisterPair
    {
        BC = 0,
        DE,
        HL,
        SP,
        AF,
    };

    enum CPUFlags
    {
        Z = 128,
        N = 64,
        HC = 32,
        CY = 16,
        // unused, always 0
        B3 = 8,
        B2 = 4,
        B1 = 2,
        B0 = 1
    };

    class SM83
    {
        using opcode_function = void (SM83::*)();
        Core &core;
        MainBus &bus;

    public:
        bool master_interrupt_enable = true, double_speed = false;
        bool halted = false, stopped = false, ei_delay = false;
        uint8_t interrupt_flag = 0, interrupt_enable = 0;
        uint16_t sp = 0xFFFE, pc = 0;
        std::array<uint8_t, 8> registers{};

        size_t fetched_count = 0;
        std::array<uint8_t, 3> fetched_bytes{};

        SM83(Core &core, MainBus &bus);

        void reset(uint16_t new_pc);
        void step();
        void service_interrupts();

    private:
        uint8_t fetch(uint16_t address);
        uint16_t fetch_uint16(uint16_t address);
        uint8_t read(uint16_t address);
        uint16_t read_uint16(uint16_t address);
        void write(uint16_t address, uint8_t value);
        void write_uint16(uint16_t address, uint16_t value);

        static std::array<SM83::opcode_function, 256> gen_optable();
        static std::array<SM83::opcode_function, 256> gen_cb_optable();
        std::array<opcode_function, 256> opcodes, cb_opcodes;
        void push_sp(uint16_t value);
        uint16_t pop_sp();

        void set_flags(uint8_t flagBits, bool set);
        bool get_flag(CPUFlags flagBit) const;
        uint16_t get_rp(RegisterPair index) const;
        void set_rp(RegisterPair index, uint16_t value);

        void op_ld_u16_sp();
        void op_stop();
        void op_jr_i8();
        void op_rlca();
        void op_rrca();
        void op_rla();
        void op_rra();
        void op_daa();
        void op_cpl();
        void op_scf();
        void op_ccf();

        void op_jp_u16();
        void op_call_u16();
        void op_cb();
        void op_ld_ff00_u8_a();
        void op_ld_ff00_c_a();
        void op_add_sp_i8();
        void op_jp_hl();
        void op_ld_u16_a();
        void op_ld_a_ff00_u8();
        void op_ld_a_ff00_c();
        void op_di();
        void op_ei();
        void op_ld_hl_sp_i8();
        void op_ld_sp_hl();
        void op_ld_a_u16();

        template <bool is_illegal_op, uint8_t illegal_op> void op_nop();
        template <RegisterPair rp> void op_ld_rp_u16();
        template <RegisterPair rp> void op_inc_rp();
        template <RegisterPair rp> void op_dec_rp();

        template <RegisterPair rp, int16_t displacement> void op_ld_rp_a();
        template <CPUFlags cc, bool boolean_ver> void op_jr_cc_i8();
        template <RegisterPair rp> void op_add_hl_rp();
        template <Register r> void op_inc_r();
        template <Register r> void op_dec_r();
        template <Register r> void op_ld_r_u8();
        template <RegisterPair rp, int16_t displacement> void op_ld_a_rp();
        template <Register r, Register r2> void op_ld_r_r();

        template <Register r, bool with_carry> void op_add_a_r();
        template <Register r, bool with_carry> void op_sub_a_r();
        template <Register r> void op_and_a_r();
        template <Register r> void op_xor_a_r();
        template <Register r> void op_or_a_r();
        template <Register r> void op_cp_a_r();

        template <bool enable_interrupts> void op_ret();
        template <CPUFlags cc, bool boolean_ver> void op_ret_cc();
        template <CPUFlags cc, bool boolean_ver> void op_jp_cc_u16();
        template <CPUFlags cc, bool boolean_ver> void op_call_cc_u16();

        template <RegisterPair rp> void op_pop_rp();
        template <RegisterPair rp> void op_push_rp();

        template <uint16_t page> void op_rst_n();

        template <Register r> void op_rlc();
        template <Register r> void op_rrc();
        template <Register r> void op_rl();
        template <Register r> void op_rr();
        template <Register r> void op_sla();
        template <Register r> void op_sra();
        template <Register r> void op_swap();
        template <Register r> void op_srl();

        template <uint8_t bit, Register r> void op_bit();
        template <uint8_t bit, Register r> void op_res();
        template <uint8_t bit, Register r> void op_set();
    };

}