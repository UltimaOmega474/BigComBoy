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
#include <functional>

namespace GB {
    namespace Register {
        constexpr int32_t B = 0;
        constexpr int32_t C = 1;
        constexpr int32_t D = 2;
        constexpr int32_t E = 3;
        constexpr int32_t H = 4;
        constexpr int32_t L = 5;
        constexpr int32_t A = 6;
        constexpr int32_t F = 7;
        constexpr int32_t HL_ADDRESS = 8;
        constexpr int32_t C8BIT_IMMEDIATE = 9;
    }

    enum class RegisterPair {
        BC,
        DE,
        HL,
        SP,
        AF,
    };

    constexpr uint8_t FLAG_Z = 128;
    constexpr uint8_t FLAG_N = 64;
    constexpr uint8_t FLAG_HC = 32;
    constexpr uint8_t FLAG_CY = 16;

    class SM83 {
    public:
        uint8_t interrupt_flag = 0, interrupt_enable = 0;
        uint8_t KEY1 = 0;
        uint16_t program_counter = 0;
        uint16_t stack_pointer = 0xFFFF;

        union {
            std::array<uint8_t, 8> registers{};

            struct {
                uint8_t b;
                uint8_t c;
                uint8_t d;
                uint8_t e;
                uint8_t h;
                uint8_t l;
                uint8_t a;
                uint8_t f;
            };
        };

        std::function<void(int32_t)> run_external_state_fn;
        std::function<void(uint16_t, uint8_t)> bus_write_fn;
        std::function<uint8_t(uint16_t)> bus_read_fn;

        SM83(std::function<void(int32_t)> run_external_state_fn,
             std::function<void(uint16_t, uint8_t)> bus_write_fn,
             std::function<uint8_t(uint16_t)> bus_read_fn);

        bool stopped() const;
        bool double_speed() const;
        bool ime() const;
        bool halted() const;

        void reset(uint16_t new_pc, bool with_DMG_values);
        void request_interrupt(uint8_t interrupt);
        void step();

    private:
        void service_interrupts();

        uint16_t read_uint16(uint16_t address) const;
        void write_uint16(uint16_t address, uint16_t value) const;

        void push_sp(uint16_t value);
        uint16_t pop_sp();
        void set_flags(uint8_t flags, bool set);
        bool get_flag(uint8_t flag) const;
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
        template <uint8_t cc, bool boolean_ver> void op_jr_cc_i8();
        template <RegisterPair rp> void op_add_hl_rp();

        template <int32_t r> void op_inc_r();
        template <int32_t r> void op_dec_r();

        template <int32_t r> void op_ld_r_u8();
        template <RegisterPair rp, int16_t displacement> void op_ld_a_rp();
        template <int32_t r, int32_t r2> void op_ld_r_r();

        template <int32_t r, bool with_carry> void op_add_a_r();
        template <int32_t r, bool with_carry> void op_sub_a_r();
        template <int32_t r> void op_and_a_r();
        template <int32_t r> void op_xor_a_r();
        template <int32_t r> void op_or_a_r();
        template <int32_t r> void op_cp_a_r();

        template <bool enable_interrupts> void op_ret();
        template <uint8_t cc, bool boolean_ver> void op_ret_cc();
        template <uint8_t cc, bool boolean_ver> void op_jp_cc_u16();
        template <uint8_t cc, bool boolean_ver> void op_call_cc_u16();

        template <RegisterPair rp> void op_pop_rp();
        template <RegisterPair rp> void op_push_rp();

        template <uint16_t page> void op_rst_n();

        template <int32_t r> void op_rlc();
        template <int32_t r> void op_rrc();
        template <int32_t r> void op_rl();
        template <int32_t r> void op_rr();
        template <int32_t r> void op_sla();
        template <int32_t r> void op_sra();
        template <int32_t r> void op_swap();
        template <int32_t r> void op_srl();

        template <uint8_t bit, int32_t r> void op_bit();
        template <uint8_t bit, int32_t r> void op_res();
        template <uint8_t bit, int32_t r> void op_set();

        void gen_opcodes();
        void gen_bitwise_opcodes();

        bool master_interrupt_enable_ = true;
        bool halted_ = false;
        bool ei_delay_ = false;
        bool stopped_ = false;
        bool double_speed_ = false;
        bool dmg_mode = true;

        using OpcodeFunction = void (SM83::*)();
        std::array<OpcodeFunction, 256> opcodes{};
        std::array<OpcodeFunction, 256> bitwise_opcodes{};
    };
}