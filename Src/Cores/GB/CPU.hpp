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
#include <cinttypes>
#include <functional>

namespace GB {
    enum class RegisterPair {
        BC,
        DE,
        HL,
        SP,
        AF,
    };

    enum class ExecutionMode {
        NormalBank,
        BitOpsBank,
        Interrupt,
        Halted,
    };

    class CPU {
        enum class Operand {
            B,
            C,
            D,
            E,
            H,
            L,
            Memory,
            A,
        };

        enum class Operand2 {
            BC,
            DE,
            HLIncrement,
            HLDecrement,
        };

        enum class ConditionCode {
            Always,
            IfZero,
            IfCarry,
        };

        enum class MemRead {
            HL,
            PC,
        };

    public:
        uint8_t b = 0;
        uint8_t c = 0;
        uint8_t d = 0;
        uint8_t e = 0;
        uint8_t h = 0;
        uint8_t l = 0;
        uint8_t a = 0;

        uint8_t if_ = 0;
        uint8_t ie = 0;
        uint8_t KEY1 = 0;

        uint16_t pc = 0;
        uint16_t sp = 0xFFFF;

        CPU(std::function<void(uint16_t, uint8_t)> write_fn,
            std::function<uint8_t(uint16_t)> read_fn);

        auto force_next_opcode(uint8_t opcode) -> void;
        auto flag_state() const -> uint8_t;
        auto set_flags(uint8_t in_flags) -> void;
        auto double_speed() const -> bool;
        auto get_rp(RegisterPair index) const -> uint16_t;
        auto set_rp(RegisterPair index, uint16_t value) -> void;

        auto reset(uint16_t new_pc, bool with_dmg_values) -> void;
        auto request_interrupt(uint8_t interrupt) -> void;
        auto clock() -> void;

    private:
        auto get_register(Operand reg) const -> uint8_t;
        auto set_register(Operand reg, uint8_t value) -> void;

        auto fetch(uint16_t where) -> void;
        auto isr() -> void;
        auto nop() -> void;
        auto stop() -> void;
        auto halt() -> void;
        auto illegal_instruction() -> void;
        auto ei() -> void;
        auto di() -> void;
        auto bitops_bank_switch() -> void;

        template <Operand dst, Operand src> auto ld() -> void;
        auto op_ld_r_indirect(uint16_t address) -> void;
        template <Operand reg> auto ld_immediate() -> void;
        auto op_ld_indirect_r(uint16_t address) -> void;
        template <Operand2 rp> auto ld_indirect_accumulator() -> void;
        template <Operand2 rp> auto ld_accumulator_indirect() -> void;
        auto ld_direct_a() -> void;
        auto ld_a_direct() -> void;
        auto op_ld_indirect_immediate(uint16_t address) -> void;
        template <RegisterPair rp> auto ld_rp_immediate() -> void;
        auto ld_direct_sp() -> void;
        auto ld_hl_sp_i8() -> void;
        auto ld_sp_hl() -> void;
        auto ldh_offset_a() -> void;
        auto ldh_a_offset() -> void;
        auto ldh_c_a() -> void;
        auto ldh_a_c() -> void;

        template <RegisterPair src> auto add_hl_rp() -> void;
        auto add_sp_i8() -> void;
        template <Operand operand, bool with_carry> auto adc() -> void;
        template <Operand operand, bool with_carry> auto sbc() -> void;
        template <Operand operand> auto and_op() -> void;
        template <Operand operand> auto xor_op() -> void;
        template <Operand operand> auto or_op() -> void;
        template <Operand operand> auto cp() -> void;
        template <Operand operand> auto inc_r() -> void;
        template <Operand operand> auto dec_r() -> void;
        template <RegisterPair rp, int32_t adjustment> auto adjust_rp() -> void;

        auto rlca() -> void;
        auto rrca() -> void;
        auto rla() -> void;
        auto rra() -> void;
        auto daa() -> void;
        auto cpl() -> void;
        auto scf() -> void;
        auto ccf() -> void;

        template <Operand operand> auto rlc() -> void;
        template <Operand operand> auto rrc() -> void;
        template <Operand operand> auto rl() -> void;
        template <Operand operand> auto rr() -> void;
        template <Operand operand> auto sla() -> void;
        template <Operand operand> auto sra() -> void;
        template <Operand operand> auto swap() -> void;
        template <Operand operand> auto srl() -> void;
        template <Operand operand, uint8_t bit_num> auto bit() -> void;
        template <Operand operand, uint8_t bit_num> auto res() -> void;
        template <Operand operand, uint8_t bit_num> auto set() -> void;

        template <ConditionCode cc, bool is_set> auto jr() -> void;
        template <ConditionCode cc, bool is_set, bool from_hl> auto jp() -> void;
        template <ConditionCode cc, bool is_set> auto call() -> void;
        template <bool enable_interrupts> auto ret() -> void;
        template <ConditionCode cc, bool is_set> auto ret_cc() -> void;
        auto rst() -> void;
        template <RegisterPair rp> auto push() -> void;
        template <RegisterPair rp> auto pop() -> void;

        template <typename Fn> auto immediate_addr(Fn &&) -> void;
        template <MemRead address, typename Fn> auto mem_read_addr(Fn &&) -> void;
        template <typename Fn> auto mem_write_addr(Fn &&) -> void;
        template <typename Fn> auto read_modify_write(Fn &&) -> void;

        auto decode_execute() -> void;
        auto decode_execute_bitops() -> void;

        struct {
            bool cy = false;
            bool hc = false;
            bool n = false;
            bool z = false;
        } flags;

        bool ime = false;
        bool double_speed_ = false;
        bool dmg_mode = true;

        uint8_t ir = 0;
        uint8_t z = 0;
        uint8_t w = 0;
        uint8_t m_cycle = 1;

        ExecutionMode exec = ExecutionMode::NormalBank;

        std::function<void(uint16_t, uint8_t)> write;
        std::function<uint8_t(uint16_t)> read;
    };

}
