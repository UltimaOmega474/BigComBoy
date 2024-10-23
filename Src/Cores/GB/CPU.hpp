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
#include "SM83.hpp"
#include <cinttypes>
#include <filesystem>

namespace GB {

    enum class COperand2 {
        B,
        C,
        D,
        E,
        H,
        L,
        Memory,
        A,
    };

    enum class COperand3 {
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

    class CPU {
    public:
        uint8_t b = 0;
        uint8_t c = 0;
        uint8_t d = 0;
        uint8_t e = 0;
        uint8_t h = 0;
        uint8_t l = 0;
        uint8_t a = 0;

        bool master_interrupt_enable_ = true;
        uint8_t interrupt_flag = 0;
        uint8_t interrupt_enable = 0;
        uint8_t KEY1 = 0;

        uint16_t program_counter = 0;
        uint16_t stack_pointer = 0xFFFF;

        std::function<void(uint16_t, uint8_t)> bus_write_fn;
        std::function<uint8_t(uint16_t)> bus_read_fn;

        auto force_next_opcode(uint8_t opcode) -> void;
        auto flags() const -> uint8_t;
        auto set_flags(uint8_t flags) -> void;
        auto clock() -> void;

    private:
        auto get_register(COperand2 reg) const -> uint8_t;
        auto set_register(COperand2 reg, uint8_t value) -> void;
        auto get_rp(RegisterPair index) const -> uint16_t;
        auto set_rp(RegisterPair index, uint16_t value) -> void;

        auto fetch(uint16_t where) -> void;
        auto nop() -> void;
        auto stop() -> void;
        auto halt() -> void;
        auto illegal() -> void;
        auto ei() -> void;
        auto di() -> void;

        template <COperand2 dst, COperand2 src> auto ld() -> void;
        auto op_ld_r_indirect(uint16_t address) -> void;
        template <COperand2 reg> auto ld_immediate() -> void;
        auto op_ld_indirect_r(uint16_t address) -> void;
        template <COperand3 rp> auto ld_indirect_accumulator() -> void;
        template <COperand3 rp> auto ld_accumulator_indirect() -> void;
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
        template <COperand2 operand, bool with_carry> auto adc() -> void;
        template <COperand2 operand, bool with_carry> auto sbc() -> void;
        template <COperand2 operand> auto and_op() -> void;
        template <COperand2 operand> auto xor_op() -> void;
        template <COperand2 operand> auto or_op() -> void;
        template <COperand2 operand> auto cp() -> void;
        template <COperand2 operand> auto inc_r() -> void;
        template <COperand2 operand> auto dec_r() -> void;
        template <RegisterPair rp, int32_t adjustment> auto adjust_rp() -> void;

        auto rlca() -> void;
        auto rrca() -> void;
        auto rla() -> void;
        auto rra() -> void;
        auto daa() -> void;
        auto cpl() -> void;
        auto scf() -> void;
        auto ccf() -> void;

        template <ConditionCode cc, bool is_set> auto jr() -> void;
        template <ConditionCode cc, bool is_set, bool from_hl> auto jp() -> void;
        template <ConditionCode cc, bool is_set> auto call() -> void;
        template <bool enable_interrupts> auto ret() -> void;
        template <ConditionCode cc, bool is_set> auto ret_cc() -> void;
        auto rst() -> void;

        template <typename Fn> auto immediate_addr(Fn &&) -> void;
        template <MemRead address, typename Fn> auto mem_read_addr(Fn &&) -> void;
        template <typename Fn> auto mem_write_addr(Fn &&) -> void;
        template <typename Fn> auto read_modify_write(Fn &&) -> void;

        struct {
            bool cy = false;
            bool hc = false;
            bool n = false;
            bool z = false;
        } alu_flags;

        uint8_t ir = 0;
        uint8_t z = 0;
        uint8_t w = 0;

        uint8_t m_cycle = 1;
    };

}
