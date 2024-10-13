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
        B = 0,
        C = 1,
        D = 2,
        E = 3,
        H = 4,
        L = 5,
        A = 7,
    };

    enum class COperand3 {
        BC,
        DE,
        HLIncrement,
        HLDecrement,
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
        auto op_ld_r_r() -> void;
        auto op_ld_r_indirect(uint16_t address) -> void;
        auto op_ld_r_immediate() -> void;
        auto op_ld_indirect_r(uint16_t address) -> void;

        template <COperand3 rp> auto op_ld_indirect_rp_a() -> void;
        template <COperand3 rp> auto op_ld_a_indirect_rp() -> void;
        auto op_ld_indirect_immediate(uint16_t address) -> void;
        auto op_ld_rp_immediate() -> void;

        uint8_t ir = 0;
        uint8_t f = 0;
        uint8_t z = 0;
        uint8_t w = 0;
        uint8_t m_cycle = 1;

        friend auto run_test(const std::filesystem::path &path, uint8_t opcode) -> void;
    };

}