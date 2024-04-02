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

#include "SM83.hpp"
#include "Bus.hpp"
#include "Constants.hpp"
#include "Core.hpp"
#include <stdexcept>

#define GET_REG(R) registers[static_cast<size_t>(R)]

namespace GB {
    SM83::SM83(Core *core) : opcodes(gen_optable()), cb_opcodes(gen_cb_optable()), core(core) {
        if (!core) {
            throw std::invalid_argument("Core cannot be null.");
        }
    }

    bool SM83::stopped() const { return stopped_; }

    bool SM83::double_speed() const { return double_speed_; }

    void SM83::reset(uint16_t new_pc) {
        master_interrupt_enable_ = false;
        double_speed_ = false;
        halted_ = false;
        stopped_ = false;
        ei_delay_ = false;
        interrupt_enable = 0;
        interrupt_flag = 0;

        if (core->bus.is_compatibility_mode()) {
            set_flags(FLAG_N, false);
            set_flags(FLAG_Z, true);
            set_flags(FLAG_HC | FLAG_CY, true);

            GET_REG(Register::A) = 0x01;
            GET_REG(Register::B) = 0x00;
            GET_REG(Register::C) = 0x13;
            GET_REG(Register::D) = 0x00;
            GET_REG(Register::E) = 0xD8;
            GET_REG(Register::H) = 0x01;
            GET_REG(Register::L) = 0x4D;

            pc = new_pc;
            sp = 0xFFFE;
        } else {
            set_flags(FLAG_N | FLAG_HC | FLAG_CY, false);
            set_flags(FLAG_Z, true);

            GET_REG(Register::A) = 0x11;
            GET_REG(Register::B) = 0x00;
            GET_REG(Register::C) = 0x00;
            GET_REG(Register::D) = 0xFF;
            GET_REG(Register::E) = 0x56;
            GET_REG(Register::H) = 0x00;
            GET_REG(Register::L) = 0x0D;

            pc = new_pc;
            sp = 0xFFFE;
        }
    }

    void SM83::request_interrupt(uint8_t interrupt) { interrupt_flag |= interrupt; }

    void SM83::step() {
        service_interrupts();

        if (ei_delay_) {
            master_interrupt_enable_ = true;
            ei_delay_ = false;
        }

        uint8_t opcode = read(pc);

        if (halted_) {
            return;
        }

        (this->*opcodes.at(opcode))();
    }

    void SM83::service_interrupts() {
        uint8_t interrupt_pending = interrupt_flag & interrupt_enable;

        if (interrupt_pending) {
            /*
                    The CPU wakes up from HALT if any interrupt has been signaled/pending.
                    This happens regardless of IME which only controls whether or not the pending
               interrupts will be serviced.
            */

            if (halted_) {
                halted_ = false;
            }

            if (!master_interrupt_enable_) {
                return;
            }

            master_interrupt_enable_ = false;
            if (interrupt_pending & 0x01) {
                core->tick_subcomponents(8);
                push_sp(pc);
                core->tick_subcomponents(4);
                pc = 0x40;

                interrupt_flag &= ~INT_VBLANK_BIT;
            } else if (interrupt_pending & INT_LCD_STAT_BIT) {
                core->tick_subcomponents(8);
                push_sp(pc);
                core->tick_subcomponents(4);
                pc = 0x48;

                interrupt_flag &= ~INT_LCD_STAT_BIT;
            } else if (interrupt_pending & INT_TIMER_BIT) {
                core->tick_subcomponents(8);
                push_sp(pc);
                core->tick_subcomponents(4);
                pc = 0x50;

                interrupt_flag &= ~INT_TIMER_BIT;
            } else if (interrupt_pending & INT_SERIAL_PORT_BIT) {
                core->tick_subcomponents(8);
                push_sp(pc);
                core->tick_subcomponents(4);
                pc = 0x58;

                interrupt_flag &= ~INT_SERIAL_PORT_BIT;
            } else if (interrupt_pending & INT_JOYPAD_BIT) {
                core->tick_subcomponents(8);
                push_sp(pc);
                core->tick_subcomponents(4);
                pc = 0x60;

                interrupt_flag &= ~INT_JOYPAD_BIT;
            }
        }
    }

    uint8_t SM83::read(uint16_t address) {
        core->tick_subcomponents(4);
        return core->bus.read(address);
    }

    uint16_t SM83::read_uint16(uint16_t address) {
        uint8_t low = read(address);
        uint8_t hi = read(address + 1);

        return (static_cast<uint16_t>(hi) << 8) | static_cast<uint16_t>(low);
    }

    void SM83::write(uint16_t address, uint8_t value) {
        core->tick_subcomponents(4);
        core->bus.write(address, value);
    }

    void SM83::write_uint16(uint16_t address, uint16_t value) {
        write(address, value & 0x00FF);
        write(address + 1, (value & 0xFF00) >> 8);
    }

    void SM83::push_sp(uint16_t temp) {
        write(--sp, (temp & 0xFF00) >> 8);
        write(--sp, (temp & 0xFF));
    }

    uint16_t SM83::pop_sp() {
        uint8_t low = read(sp++);
        uint8_t hi = read(sp++);
        return (hi << 8) | low;
    }

    void SM83::set_flags(uint8_t flags, bool set) {
        if (set) {
            GET_REG(Register::F) |= flags;
        } else {
            GET_REG(Register::F) &= ~flags;
        }
    }

    bool SM83::get_flag(uint8_t flag) const { return GET_REG(Register::F) & flag; }

    uint16_t SM83::get_rp(RegisterPair index) const {
        uint16_t hi;
        uint16_t low;
        switch (index) {
        case RegisterPair::BC: {
            hi = GET_REG(Register::B);
            low = GET_REG(Register::C);
            break;
        }
        case RegisterPair::DE: {
            hi = GET_REG(Register::D);
            low = GET_REG(Register::E);
            break;
        }
        case RegisterPair::HL: {
            hi = GET_REG(Register::H);
            low = GET_REG(Register::L);
            break;
        }
        case RegisterPair::SP: {
            return sp;
        }
        case RegisterPair::AF: {
            hi = GET_REG(Register::A);
            low = GET_REG(Register::F);
            break;
        }
        }

        return (hi << 8) | (low);
    }

    void SM83::set_rp(RegisterPair index, uint16_t temp) {
        switch (index) {
        case RegisterPair::BC: {
            GET_REG(Register::B) = static_cast<uint8_t>((temp & 0xFF00) >> 8);
            GET_REG(Register::C) = static_cast<uint8_t>(temp & 0x00FF);
            return;
        }
        case RegisterPair::DE: {
            GET_REG(Register::D) = static_cast<uint8_t>((temp & 0xFF00) >> 8);
            GET_REG(Register::E) = static_cast<uint8_t>(temp & 0x00FF);
            return;
        }
        case RegisterPair::HL: {
            GET_REG(Register::H) = static_cast<uint8_t>((temp & 0xFF00) >> 8);
            GET_REG(Register::L) = static_cast<uint8_t>(temp & 0x00FF);
            return;
        }
        case RegisterPair::SP: {
            sp = temp;
            return;
        }
        case RegisterPair::AF: {
            GET_REG(Register::A) = static_cast<uint8_t>((temp & 0xFF00) >> 8);
            GET_REG(Register::F) = static_cast<uint8_t>(temp & 0x00FF);
            set_flags(0xF, false);
            return;
        }
        }
    }

    void SM83::op_ld_u16_sp() {
        auto addr = read_uint16(pc + 1);

        write_uint16(addr, sp);
        pc += 3;
    }

    void SM83::op_stop() {
        if (core->bus.is_compatibility_mode()) {
            stopped_ = true;
            return;
        }

        if (KEY1 & 0x1) {
            double_speed_ = !double_speed_;
            KEY1 = double_speed_ << 7;
        }

        pc += 2;
    }

    void SM83::op_jr_i8() {
        int8_t off = static_cast<int8_t>(read(pc + 1));

        pc += 2;
        pc += off;
        core->tick_subcomponents(4);
    }

    void SM83::op_rlca() {
        uint16_t temp = static_cast<uint16_t>(GET_REG(Register::A));
        auto bit7 = temp & 0x80 ? 1 : 0;

        set_flags(FLAG_CY, bit7);
        set_flags(FLAG_Z | FLAG_N | FLAG_HC, false);
        temp = (temp << 1) | bit7;

        GET_REG(Register::A) = static_cast<uint8_t>(temp & 0xFF);
        ++pc;
    }

    void SM83::op_rrca() {
        uint16_t temp = static_cast<uint16_t>(GET_REG(Register::A));
        uint8_t bit0 = (temp & 1) ? 0x80 : 0;

        set_flags(FLAG_CY, (temp & 1));
        set_flags(FLAG_Z | FLAG_N | FLAG_HC, false);
        temp = (temp >> 1) | bit0;

        GET_REG(Register::A) = static_cast<uint8_t>(temp & 0xFF);
        ++pc;
    }

    void SM83::op_rla() {
        uint16_t temp = static_cast<uint16_t>(GET_REG(Register::A));
        uint16_t cy = get_flag(FLAG_CY);

        set_flags(FLAG_CY, (temp & 0x80));
        set_flags(FLAG_Z | FLAG_N | FLAG_HC, false);
        temp = (temp << 1) | cy;

        GET_REG(Register::A) = static_cast<uint8_t>(temp & 0xFF);
        ++pc;
    }

    void SM83::op_rra() {
        uint16_t temp = static_cast<uint16_t>(GET_REG(Register::A));
        uint16_t cy = get_flag(FLAG_CY);

        cy <<= 7;
        set_flags(FLAG_CY, (temp & 1));
        set_flags(FLAG_Z | FLAG_N | FLAG_HC, false);
        temp = (temp >> 1) | cy;

        GET_REG(Register::A) = static_cast<uint8_t>(temp & 0xFF);
        ++pc;
    }

    void SM83::op_daa() {
        // Implementation adapted from: https://ehaskins.com/2018-01-30%20Z80%20DAA/
        uint8_t correct = 0;
        uint16_t temp = static_cast<uint16_t>(GET_REG(Register::A));
        bool cy = false;

        if (get_flag(FLAG_HC) || (!get_flag(FLAG_N) && (temp & 0xF) > 9)) {
            correct |= 6;
        }

        if (get_flag(FLAG_CY) || (!get_flag(FLAG_N) && temp > 0x99)) {
            correct |= 0x60;
            cy = true;
        }

        temp += get_flag(FLAG_N) ? -correct : correct;
        temp &= 0xFF;

        set_flags(FLAG_Z, temp == 0);
        set_flags(FLAG_HC, false);
        set_flags(FLAG_CY, cy);

        GET_REG(Register::A) = static_cast<uint8_t>(temp);
        ++pc;
    }

    void SM83::op_cpl() {
        GET_REG(Register::A) = ~GET_REG(Register::A);
        set_flags(FLAG_N | FLAG_HC, true);
        ++pc;
    }

    void SM83::op_scf() {
        set_flags(FLAG_N | FLAG_HC, false);
        set_flags(FLAG_CY, true);
        ++pc;
    }

    void SM83::op_ccf() {
        set_flags(FLAG_N | FLAG_HC, false);
        set_flags(FLAG_CY, !get_flag(FLAG_CY));
        ++pc;
    }

    void SM83::op_jp_u16() {
        pc = read_uint16(pc + 1);
        core->tick_subcomponents(4);
    }

    void SM83::op_call_u16() {
        uint16_t saved_pc = pc + 3;
        auto addr = read_uint16(pc + 1);
        core->tick_subcomponents(4);
        push_sp(saved_pc);
        pc = addr;
    }

    void SM83::op_cb() {
        uint8_t opcode = read(pc + 1);

        (this->*cb_opcodes.at(opcode))();
        pc += 2;
    }

    void SM83::op_ld_ff00_u8_a() {
        uint8_t off = read(pc + 1);
        write(0xFF00 + off, GET_REG(Register::A));
        pc += 2;
    }

    void SM83::op_ld_ff00_c_a() {
        write(0xFF00 + GET_REG(Register::C), GET_REG(Register::A));
        ++pc;
    }

    void SM83::op_add_sp_i8() {
        int16_t off = static_cast<int8_t>(read(pc + 1));
        uint16_t sp32 = sp;
        uint16_t res32 = (sp32 + off);

        // internal operation?
        core->tick_subcomponents(4);
        set_rp(RegisterPair::SP, res32 & 0xFFFF);
        // sp update is visible
        core->tick_subcomponents(4);
        set_flags(FLAG_HC, ((sp32 & 0xF) + (off & 0xF)) > 0xF);
        set_flags(FLAG_CY, ((sp32 & 0xFF) + (off & 0xFF)) > 0xFF);
        set_flags(FLAG_Z | FLAG_N, false);
        pc += 2;
    }

    void SM83::op_jp_hl() { pc = get_rp(RegisterPair::HL); }

    void SM83::op_ld_u16_a() {
        auto addr = read_uint16(pc + 1);
        write(addr, GET_REG(Register::A));
        pc += 3;
    }

    void SM83::op_ld_a_ff00_u8() {
        uint16_t off = read(pc + 1);
        GET_REG(Register::A) = read(0xFF00 + off);
        pc += 2;
    }

    void SM83::op_ld_a_ff00_c() {
        GET_REG(Register::A) = read(0xFF00 + GET_REG(Register::C));
        ++pc;
    }

    void SM83::op_di() {
        master_interrupt_enable_ = false;
        ei_delay_ = false;
        ++pc;
    }

    void SM83::op_ei() {
        ei_delay_ = true;
        ++pc;
    }

    void SM83::op_ld_hl_sp_i8() {
        int16_t off = static_cast<int8_t>(read(pc + 1));
        uint16_t sp32 = sp;
        uint16_t res32 = (sp32 + off);
        set_rp(RegisterPair::HL, static_cast<uint16_t>(res32 & 0xFFFF));
        set_flags(FLAG_HC, ((sp32 & 0xF) + (off & 0xF)) > 0xF);
        set_flags(FLAG_CY, ((sp32 & 0xFF) + (off & 0xFF)) > 0xFF);
        set_flags(FLAG_Z | FLAG_N, false);
        core->tick_subcomponents(4);
        pc += 2;
    }

    void SM83::op_ld_sp_hl() {
        sp = get_rp(RegisterPair::HL);
        core->tick_subcomponents(4);
        ++pc;
    }

    void SM83::op_ld_a_u16() {
        auto addr = read_uint16(pc + 1);
        GET_REG(Register::A) = read(addr);
        pc += 3;
    }

    template <bool is_illegal_op, uint8_t illegal_op> void SM83::op_nop() {
        if constexpr (is_illegal_op) {
            stopped_ = true;
            return;
        }
        ++pc;
    }

    template <RegisterPair rp> inline void SM83::op_ld_rp_u16() {
        uint16_t combine = read_uint16(pc + 1);

        set_rp(rp, combine);
        pc += 3;
    }

    template <RegisterPair rp> inline void SM83::op_inc_rp() {
        auto increment = [this](uint8_t &high, uint8_t &low) {
            uint16_t res = static_cast<uint16_t>(high) << 8 | static_cast<uint16_t>(low);

            ++res;
            high = static_cast<uint8_t>((res & 0xFF00) >> 8);
            low = static_cast<uint8_t>(res & 0xFF);

            core->tick_subcomponents(4);
            ++pc;
        };

        switch (rp) {
        case RegisterPair::BC: {
            increment(GET_REG(Register::B), GET_REG(Register::C));
            return;
        }
        case RegisterPair::DE: {
            increment(GET_REG(Register::D), GET_REG(Register::E));
            return;
        }
        case RegisterPair::HL: {
            increment(GET_REG(Register::H), GET_REG(Register::L));
            return;
        }
        case RegisterPair::SP: {
            sp++;
            core->tick_subcomponents(4);
            ++pc;
            return;
        }
        }
    }

    template <RegisterPair rp> void SM83::op_dec_rp() {
        const auto decrement = [this](uint8_t &high, uint8_t &low) {
            uint16_t res = static_cast<uint16_t>(high) << 8 | static_cast<uint16_t>(low);

            --res;
            high = static_cast<uint8_t>((res & 0xFF00) >> 8);
            low = static_cast<uint8_t>(res & 0xFF);

            core->tick_subcomponents(4);
            ++pc;
        };

        switch (rp) {
        case RegisterPair::BC: {
            decrement(GET_REG(Register::B), GET_REG(Register::C));
            return;
        }
        case RegisterPair::DE: {
            decrement(GET_REG(Register::D), GET_REG(Register::E));
            return;
        }
        case RegisterPair::HL: {
            decrement(GET_REG(Register::H), GET_REG(Register::L));
            return;
        }
        case RegisterPair::SP: {
            sp--;
            core->tick_subcomponents(4);
            ++pc;
            return;
        }
        }
    }

    template <RegisterPair rp, int16_t displacement> inline void SM83::op_ld_rp_a() {
        uint16_t addr = get_rp(rp);

        write(addr, GET_REG(Register::A));

        if constexpr (displacement != 0) {
            set_rp(rp, addr + displacement);
        }
        ++pc;
    }

    template <uint8_t cc, bool boolean_ver> inline void SM83::op_jr_cc_i8() {
        if (get_flag(cc) == boolean_ver) {
            int8_t off = static_cast<int8_t>(read(pc + 1));

            pc += 2;
            pc += off;
            core->tick_subcomponents(4);
            return;
        }
        core->tick_subcomponents(4);
        pc += 2;
    }

    template <RegisterPair rp> inline void SM83::op_add_hl_rp() {
        int32_t left = get_rp(RegisterPair::HL);
        int32_t right = get_rp(rp);
        int32_t res32 = left + right;

        set_flags(FLAG_N, false);
        set_flags(FLAG_HC, ((left & 0xFFF) + (right & 0xFFF)) > 0xFFF);
        set_flags(FLAG_CY, res32 > 0xFFFF);
        set_rp(RegisterPair::HL, static_cast<uint16_t>(res32 & 0xFFFF));

        this->core->tick_subcomponents(4);
        ++pc;
    }

    template <Register r> void SM83::op_inc_r() {
        uint16_t left;
        uint16_t right = 1;

        if constexpr (r == Register::HL_ADDR) {
            left = read(get_rp(RegisterPair::HL));
        } else {
            left = GET_REG(r);
        }

        auto result = left + right;
        auto masked_result = result & 0xFF;

        set_flags(FLAG_N, false);
        set_flags(FLAG_Z, masked_result == 0);
        set_flags(FLAG_HC, ((left & 0xF) + (right & 0xF)) > 0xF);

        if constexpr (r == Register::HL_ADDR) {
            write(get_rp(RegisterPair::HL), static_cast<uint8_t>(masked_result));
        } else {
            GET_REG(r) = static_cast<uint8_t>(masked_result);
        }
        ++pc;
    }

    template <Register r> void SM83::op_dec_r() {
        int16_t left;
        int16_t right = 1;

        if constexpr (r == Register::HL_ADDR) {
            left = read(get_rp(RegisterPair::HL));
        } else {
            left = GET_REG(r);
        }

        auto result = left - right;
        auto masked_result = result & 0xFF;

        set_flags(FLAG_HC, ((left & 0xF) - (right & 0xF)) < 0);
        set_flags(FLAG_Z, masked_result == 0);
        set_flags(FLAG_N, true); // only set if subtraction

        if constexpr (r == Register::HL_ADDR) {
            write(get_rp(RegisterPair::HL), static_cast<uint8_t>(masked_result));
        } else {
            GET_REG(r) = static_cast<uint8_t>(masked_result);
        }
        ++pc;
    }

    template <Register r> void SM83::op_ld_r_u8() {
        if constexpr (r == Register::HL_ADDR) {
            write(get_rp(RegisterPair::HL), read(pc + 1));
        } else {
            GET_REG(r) = read(pc + 1);
        }
        pc += 2;
    }

    template <RegisterPair rp, int16_t displacement> void SM83::op_ld_a_rp() {
        auto addr = get_rp(rp);

        GET_REG(Register::A) = read(addr);

        if constexpr (displacement != 0) {
            set_rp(rp, addr + displacement);
        }
        ++pc;
    }

    template <Register r, Register r2> void SM83::op_ld_r_r() {
        // r = destination
        // r2 = source
        if constexpr (r == Register::HL_ADDR && r2 != Register::HL_ADDR) {
            write(get_rp(RegisterPair::HL), GET_REG(r2));
        } else if constexpr (r != Register::HL_ADDR && r2 == Register::HL_ADDR) {
            GET_REG(r) = read(get_rp(RegisterPair::HL));
        } else if constexpr (r == Register::HL_ADDR && r2 == Register::HL_ADDR) {
            halted_ = true;
        } else if constexpr (r != Register::HL_ADDR && r2 != Register::HL_ADDR) {
            GET_REG(r) = GET_REG(r2);
        }
        ++pc;
    }

    template <Register r, bool with_carry> void SM83::op_add_a_r() {
        uint16_t left = static_cast<uint16_t>(GET_REG(Register::A));
        uint16_t right;

        if constexpr (r == Register::HL_ADDR) {
            right = static_cast<uint16_t>(read(get_rp(RegisterPair::HL)));
        } else if constexpr (r == Register::U8) {
            right = static_cast<uint16_t>(read(pc + 1));
            ++pc;
        } else {
            right = static_cast<uint16_t>(GET_REG(r));
        }

        uint16_t cy = with_carry ? get_flag(FLAG_CY) : 0;

        uint16_t result = left + right + cy;
        uint8_t masked_result = result & 0xFF;

        set_flags(FLAG_HC, ((left & 0xF) + (right & 0xF) + (cy & 0xF)) > 0xF);
        set_flags(FLAG_CY, result > 0xFF);
        set_flags(FLAG_Z, masked_result == 0);
        set_flags(FLAG_N, false);

        GET_REG(Register::A) = masked_result;
        ++pc;
    }

    template <Register r, bool with_carry> void SM83::op_sub_a_r() {
        int16_t left = static_cast<int16_t>(GET_REG(Register::A));
        int16_t right;

        if constexpr (r == Register::HL_ADDR) {
            right = static_cast<int16_t>(read(get_rp(RegisterPair::HL)));
        } else if constexpr (r == Register::U8) {
            right = static_cast<int16_t>(read(pc + 1));
            ++pc;
        } else {
            right = static_cast<int16_t>(GET_REG(r));
        }

        int16_t cy = with_carry ? get_flag(FLAG_CY) : 0;

        int16_t result = static_cast<int16_t>(left - right - cy);
        uint8_t masked_result = result & 0xFF;

        set_flags(FLAG_HC, ((left & 0xF) - (right & 0xF) - (cy & 0xF)) < 0);
        set_flags(FLAG_CY, result < 0);
        set_flags(FLAG_Z, masked_result == 0);
        set_flags(FLAG_N, true);

        GET_REG(Register::A) = masked_result;
        ++pc;
    }

    template <Register r> void SM83::op_and_a_r() {
        uint8_t right;

        if constexpr (r == Register::HL_ADDR) {
            right = read(get_rp(RegisterPair::HL));
        } else if constexpr (r == Register::U8) {
            right = read(pc + 1);
            ++pc;
        } else {
            right = GET_REG(r);
        }

        uint8_t result = GET_REG(Register::A) & right;
        set_flags(FLAG_Z, result == 0);
        set_flags(FLAG_N, false);
        set_flags(FLAG_HC, true);
        set_flags(FLAG_CY, false);

        GET_REG(Register::A) = result;
        ++pc;
    }

    template <Register r> void SM83::op_xor_a_r() {
        uint8_t right;

        if constexpr (r == Register::HL_ADDR) {
            right = read(get_rp(RegisterPair::HL));
        } else if constexpr (r == Register::U8) {
            right = read(pc + 1);
            ++pc;
        } else {
            right = GET_REG(r);
        }

        uint8_t result = GET_REG(Register::A) ^ right;
        set_flags(FLAG_Z, result == 0);
        set_flags(FLAG_N, false);
        set_flags(FLAG_HC, false);
        set_flags(FLAG_CY, false);

        GET_REG(Register::A) = result;
        ++pc;
    }

    template <Register r> void SM83::op_or_a_r() {
        uint8_t right;

        if constexpr (r == Register::HL_ADDR) {
            right = read(get_rp(RegisterPair::HL));
        } else if constexpr (r == Register::U8) {
            right = read(pc + 1);
            ++pc;
        } else {
            right = GET_REG(r);
        }

        uint8_t result = GET_REG(Register::A) | right;
        set_flags(FLAG_Z, result == 0);
        set_flags(FLAG_N, false);
        set_flags(FLAG_HC, false);
        set_flags(FLAG_CY, false);

        GET_REG(Register::A) = result;
        ++pc;
    }

    template <Register r> void SM83::op_cp_a_r() {
        uint8_t right;

        if constexpr (r == Register::HL_ADDR) {
            right = read(get_rp(RegisterPair::HL));
        } else if constexpr (r == Register::U8) {
            right = read(pc + 1);
            ++pc;
        } else {
            right = GET_REG(r);
        }

        int16_t result = GET_REG(Register::A) - right;
        uint8_t masked_result = result & 0xFF;

        set_flags(FLAG_HC, ((GET_REG(Register::A) & 0xF) - (right & 0xF)) < 0);
        set_flags(FLAG_CY, result < 0);
        set_flags(FLAG_Z, masked_result == 0);
        set_flags(FLAG_N, true);

        // Register::A is not modified, same as subtract without carry
        ++pc;
    }

    template <bool enable_interrupts> void SM83::op_ret() {
        if constexpr (enable_interrupts)
            master_interrupt_enable_ = 1;

        pc = pop_sp();
        core->tick_subcomponents(4);
    }

    template <uint8_t cc, bool boolean_ver> void SM83::op_ret_cc() {
        core->tick_subcomponents(4);

        if (get_flag(cc) == boolean_ver) {
            pc = pop_sp();
            core->tick_subcomponents(4);
            return;
        }
        ++pc;
    }

    template <uint8_t cc, bool boolean_ver> void SM83::op_jp_cc_u16() {
        if (get_flag(cc) == boolean_ver) {
            pc = read_uint16(pc + 1);
            core->tick_subcomponents(4);
            return;
        }

        core->tick_subcomponents(8);
        pc += 3;
    }

    template <uint8_t cc, bool boolean_ver> void SM83::op_call_cc_u16() {
        if (get_flag(cc) == boolean_ver) {
            auto saved_pc = pc + 3;
            auto addr = read_uint16(pc + 1);
            core->tick_subcomponents(4);
            push_sp(saved_pc);
            pc = addr;
            return;
        }

        core->tick_subcomponents(8);
        pc += 3;
    }

    template <RegisterPair rp> void SM83::op_pop_rp() {
        uint16_t temp = pop_sp();
        set_rp(rp, temp);
        ++pc;
    }

    template <RegisterPair rp> void SM83::op_push_rp() {
        core->tick_subcomponents(4);

        push_sp(get_rp(rp));
        ++pc;
    }

    template <uint16_t page> void SM83::op_rst_n() {
        core->tick_subcomponents(4);
        push_sp(pc + 1);
        pc = page;
    }

    template <Register r> void SM83::op_rlc() {
        uint8_t temp = GET_REG(r);

        if constexpr (r == Register::HL_ADDR) {
            temp = read(get_rp(RegisterPair::HL));
        }

        uint8_t bit7 = (temp & 0x80) ? 1 : 0;

        set_flags(FLAG_CY, bit7);
        set_flags(FLAG_Z, temp == 0);
        set_flags(FLAG_N | FLAG_HC, false);
        temp = temp << 1;
        temp |= bit7;

        if constexpr (r == Register::HL_ADDR) {
            write(get_rp(RegisterPair::HL), temp);
        } else {
            GET_REG(r) = temp;
        }
    }

    template <Register r> void SM83::op_rrc() {
        uint8_t temp = GET_REG(r);

        if constexpr (r == Register::HL_ADDR) {
            temp = read(get_rp(RegisterPair::HL));
        }

        uint8_t bit0 = (temp & 0x01) ? 0x80 : 0;
        set_flags(FLAG_CY, bit0);
        set_flags(FLAG_Z, temp == 0);
        set_flags(FLAG_N | FLAG_HC, false);
        temp = temp >> 1;
        temp |= bit0;

        if constexpr (r == Register::HL_ADDR) {
            write(get_rp(RegisterPair::HL), temp);
        } else {
            GET_REG(r) = temp;
        }
    }

    template <Register r> void SM83::op_rl() {
        uint16_t temp = GET_REG(r);

        if constexpr (r == Register::HL_ADDR) {
            temp = read(get_rp(RegisterPair::HL));
        }

        uint16_t cy = get_flag(FLAG_CY);
        set_flags(FLAG_CY, (temp & 0x80));

        temp = temp << 1;
        temp = temp | cy;
        uint8_t t8 = static_cast<uint8_t>(temp & 0xFF);
        set_flags(FLAG_Z, t8 == 0);
        set_flags(FLAG_N | FLAG_HC, false);

        if constexpr (r == Register::HL_ADDR) {
            write(get_rp(RegisterPair::HL), static_cast<uint8_t>(temp));
        } else {
            GET_REG(r) = t8;
        }
    }

    template <Register r> void SM83::op_rr() {
        uint16_t temp = GET_REG(r);

        if constexpr (r == Register::HL_ADDR) {
            temp = read(get_rp(RegisterPair::HL));
        }

        uint16_t cy = get_flag(FLAG_CY);
        cy = cy << 7;
        set_flags(FLAG_CY, (temp & 0x01));
        temp = temp >> 1;
        temp = temp | cy;
        uint8_t t8 = temp & 0xFF;
        set_flags(FLAG_Z, t8 == 0);
        set_flags(FLAG_N | FLAG_HC, false);

        if constexpr (r == Register::HL_ADDR) {
            write(get_rp(RegisterPair::HL), static_cast<uint8_t>(temp));
        } else {
            GET_REG(r) = t8;
        }
    }

    template <Register r> void SM83::op_sla() {
        uint8_t temp = GET_REG(r);

        if constexpr (r == Register::HL_ADDR) {
            temp = read(get_rp(RegisterPair::HL));
        }

        set_flags(FLAG_CY, (temp & 0x80));
        temp = temp << 1;
        set_flags(FLAG_Z, temp == 0);
        set_flags(FLAG_N | FLAG_HC, false);

        if constexpr (r == Register::HL_ADDR) {
            write(get_rp(RegisterPair::HL), temp);
        } else {
            GET_REG(r) = temp;
        }
    }

    template <Register r> void SM83::op_sra() {
        uint8_t temp = GET_REG(r);

        if constexpr (r == Register::HL_ADDR) {
            temp = read(get_rp(RegisterPair::HL));
        }

        uint8_t bit7 = (temp & 0x80);
        set_flags(FLAG_CY, (temp & 0x01));
        temp = temp >> 1;
        temp |= bit7; // bit7 is left unchanged
        set_flags(FLAG_Z, temp == 0);
        set_flags(FLAG_N | FLAG_HC, false);

        if constexpr (r == Register::HL_ADDR) {
            write(get_rp(RegisterPair::HL), temp);
        } else {
            GET_REG(r) = temp;
        }
    }

    template <Register r> void SM83::op_swap() {
        uint8_t temp = GET_REG(r);

        if constexpr (r == Register::HL_ADDR) {
            temp = read(get_rp(RegisterPair::HL));
        }

        uint8_t hi = (temp & 0xF0) >> 4;
        uint8_t low = (temp & 0x0F) << 4;
        temp = (low) | hi;

        set_flags(FLAG_Z, temp == 0);
        set_flags(FLAG_N | FLAG_HC | FLAG_CY, false);

        if constexpr (r == Register::HL_ADDR) {
            write(get_rp(RegisterPair::HL), temp);
        } else {
            GET_REG(r) = temp;
        }
    }

    template <Register r> void SM83::op_srl() {
        uint8_t temp = GET_REG(r);

        if constexpr (r == Register::HL_ADDR) {
            temp = read(get_rp(RegisterPair::HL));
        }

        set_flags(FLAG_CY, (temp & 0x01));
        temp = temp >> 1;
        set_flags(FLAG_Z, temp == 0);
        set_flags(FLAG_N | FLAG_HC, false);

        if constexpr (r == Register::HL_ADDR) {
            write(get_rp(RegisterPair::HL), temp);
        } else {
            GET_REG(r) = temp;
        }
    }

    template <uint8_t bit, Register r> void SM83::op_bit() {
        uint8_t temp = GET_REG(r);

        if constexpr (r == Register::HL_ADDR) {
            temp = read(get_rp(RegisterPair::HL));
        }

        set_flags(FLAG_Z, !(temp & (1 << bit)));

        set_flags(FLAG_N, false);
        set_flags(FLAG_HC, true);
    }

    template <uint8_t bit, Register r> void SM83::op_res() {
        uint8_t temp = GET_REG(r);

        if constexpr (r == Register::HL_ADDR) {
            temp = read(get_rp(RegisterPair::HL));
        }

        temp &= ~(1 << bit);

        if constexpr (r == Register::HL_ADDR) {
            write(get_rp(RegisterPair::HL), temp);
        } else {
            GET_REG(r) = temp;
        }
    }

    template <uint8_t bit, Register r> void SM83::op_set() {
        uint8_t temp = GET_REG(r);

        if constexpr (r == Register::HL_ADDR) {
            temp = read(get_rp(RegisterPair::HL));
        }

        temp |= (1 << bit);

        if constexpr (r == Register::HL_ADDR) {
            write(get_rp(RegisterPair::HL), temp);
        } else {
            GET_REG(r) = temp;
        }
    }

    std::array<SM83::opcode_function, 256> SM83::gen_optable() {
        constexpr int16_t NoDisplacement = 0;
        constexpr int16_t Increment = 1;
        constexpr int16_t Decrement = -1;
        constexpr bool WithCarry = true;
        constexpr bool WithoutCarry = false;
        constexpr bool IgnoreIME = false;
        constexpr bool SetIME = true;

        return std::array<SM83::opcode_function, 256>{

            // 0x00 - 0x0F
            &SM83::op_nop<false, 0>,
            &SM83::op_ld_rp_u16<RegisterPair::BC>,
            &SM83::op_ld_rp_a<RegisterPair::BC, NoDisplacement>,
            &SM83::op_inc_rp<RegisterPair::BC>,
            &SM83::op_inc_r<Register::B>,
            &SM83::op_dec_r<Register::B>,
            &SM83::op_ld_r_u8<Register::B>,
            &SM83::op_rlca,
            &SM83::op_ld_u16_sp,
            &SM83::op_add_hl_rp<RegisterPair::BC>,
            &SM83::op_ld_a_rp<RegisterPair::BC, NoDisplacement>,
            &SM83::op_dec_rp<RegisterPair::BC>,
            &SM83::op_inc_r<Register::C>,
            &SM83::op_dec_r<Register::C>,
            &SM83::op_ld_r_u8<Register::C>,
            &SM83::op_rrca,

            // 0x10 - 0x1F
            &SM83::op_stop,
            &SM83::op_ld_rp_u16<RegisterPair::DE>,
            &SM83::op_ld_rp_a<RegisterPair::DE, NoDisplacement>,
            &SM83::op_inc_rp<RegisterPair::DE>,
            &SM83::op_inc_r<Register::D>,
            &SM83::op_dec_r<Register::D>,
            &SM83::op_ld_r_u8<Register::D>,
            &SM83::op_rla,
            &SM83::op_jr_i8,
            &SM83::op_add_hl_rp<RegisterPair::DE>,
            &SM83::op_ld_a_rp<RegisterPair::DE, NoDisplacement>,
            &SM83::op_dec_rp<RegisterPair::DE>,
            &SM83::op_inc_r<Register::E>,
            &SM83::op_dec_r<Register::E>,
            &SM83::op_ld_r_u8<Register::E>,
            &SM83::op_rra,

            // 0x20 - 0x2F
            &SM83::op_jr_cc_i8<FLAG_Z, false>,
            &SM83::op_ld_rp_u16<RegisterPair::HL>,
            &SM83::op_ld_rp_a<RegisterPair::HL, Increment>,
            &SM83::op_inc_rp<RegisterPair::HL>,
            &SM83::op_inc_r<Register::H>,
            &SM83::op_dec_r<Register::H>,
            &SM83::op_ld_r_u8<Register::H>,
            &SM83::op_daa,
            &SM83::op_jr_cc_i8<FLAG_Z, true>,
            &SM83::op_add_hl_rp<RegisterPair::HL>,
            &SM83::op_ld_a_rp<RegisterPair::HL, Increment>,
            &SM83::op_dec_rp<RegisterPair::HL>,
            &SM83::op_inc_r<Register::L>,
            &SM83::op_dec_r<Register::L>,
            &SM83::op_ld_r_u8<Register::L>,
            &SM83::op_cpl,

            // 0x30 - 0x3F
            &SM83::op_jr_cc_i8<FLAG_CY, false>,
            &SM83::op_ld_rp_u16<RegisterPair::SP>,
            &SM83::op_ld_rp_a<RegisterPair::HL, Decrement>,
            &SM83::op_inc_rp<RegisterPair::SP>,
            &SM83::op_inc_r<Register::HL_ADDR>,
            &SM83::op_dec_r<Register::HL_ADDR>,
            &SM83::op_ld_r_u8<Register::HL_ADDR>,
            &SM83::op_scf,
            &SM83::op_jr_cc_i8<FLAG_CY, true>,
            &SM83::op_add_hl_rp<RegisterPair::SP>,
            &SM83::op_ld_a_rp<RegisterPair::HL, Decrement>,
            &SM83::op_dec_rp<RegisterPair::SP>,
            &SM83::op_inc_r<Register::A>,
            &SM83::op_dec_r<Register::A>,
            &SM83::op_ld_r_u8<Register::A>,
            &SM83::op_ccf,

            // 0x40 - 0x4F
            &SM83::op_ld_r_r<Register::B, Register::B>,
            &SM83::op_ld_r_r<Register::B, Register::C>,
            &SM83::op_ld_r_r<Register::B, Register::D>,
            &SM83::op_ld_r_r<Register::B, Register::E>,
            &SM83::op_ld_r_r<Register::B, Register::H>,
            &SM83::op_ld_r_r<Register::B, Register::L>,
            &SM83::op_ld_r_r<Register::B, Register::HL_ADDR>,
            &SM83::op_ld_r_r<Register::B, Register::A>,
            &SM83::op_ld_r_r<Register::C, Register::B>,
            &SM83::op_ld_r_r<Register::C, Register::C>,
            &SM83::op_ld_r_r<Register::C, Register::D>,
            &SM83::op_ld_r_r<Register::C, Register::E>,
            &SM83::op_ld_r_r<Register::C, Register::H>,
            &SM83::op_ld_r_r<Register::C, Register::L>,
            &SM83::op_ld_r_r<Register::C, Register::HL_ADDR>,
            &SM83::op_ld_r_r<Register::C, Register::A>,

            // 0x50 - 0x5F
            &SM83::op_ld_r_r<Register::D, Register::B>,
            &SM83::op_ld_r_r<Register::D, Register::C>,
            &SM83::op_ld_r_r<Register::D, Register::D>,
            &SM83::op_ld_r_r<Register::D, Register::E>,
            &SM83::op_ld_r_r<Register::D, Register::H>,
            &SM83::op_ld_r_r<Register::D, Register::L>,
            &SM83::op_ld_r_r<Register::D, Register::HL_ADDR>,
            &SM83::op_ld_r_r<Register::D, Register::A>,
            &SM83::op_ld_r_r<Register::E, Register::B>,
            &SM83::op_ld_r_r<Register::E, Register::C>,
            &SM83::op_ld_r_r<Register::E, Register::D>,
            &SM83::op_ld_r_r<Register::E, Register::E>,
            &SM83::op_ld_r_r<Register::E, Register::H>,
            &SM83::op_ld_r_r<Register::E, Register::L>,
            &SM83::op_ld_r_r<Register::E, Register::HL_ADDR>,
            &SM83::op_ld_r_r<Register::E, Register::A>,

            // 0x60 - 0x6F
            &SM83::op_ld_r_r<Register::H, Register::B>,
            &SM83::op_ld_r_r<Register::H, Register::C>,
            &SM83::op_ld_r_r<Register::H, Register::D>,
            &SM83::op_ld_r_r<Register::H, Register::E>,
            &SM83::op_ld_r_r<Register::H, Register::H>,
            &SM83::op_ld_r_r<Register::H, Register::L>,
            &SM83::op_ld_r_r<Register::H, Register::HL_ADDR>,
            &SM83::op_ld_r_r<Register::H, Register::A>,
            &SM83::op_ld_r_r<Register::L, Register::B>,
            &SM83::op_ld_r_r<Register::L, Register::C>,
            &SM83::op_ld_r_r<Register::L, Register::D>,
            &SM83::op_ld_r_r<Register::L, Register::E>,
            &SM83::op_ld_r_r<Register::L, Register::H>,
            &SM83::op_ld_r_r<Register::L, Register::L>,
            &SM83::op_ld_r_r<Register::L, Register::HL_ADDR>,
            &SM83::op_ld_r_r<Register::L, Register::A>,

            // 0x70 - 0x7F
            &SM83::op_ld_r_r<Register::HL_ADDR, Register::B>,
            &SM83::op_ld_r_r<Register::HL_ADDR, Register::C>,
            &SM83::op_ld_r_r<Register::HL_ADDR, Register::D>,
            &SM83::op_ld_r_r<Register::HL_ADDR, Register::E>,
            &SM83::op_ld_r_r<Register::HL_ADDR, Register::H>,
            &SM83::op_ld_r_r<Register::HL_ADDR, Register::L>,
            // HALT
            &SM83::op_ld_r_r<Register::HL_ADDR, Register::HL_ADDR>,
            &SM83::op_ld_r_r<Register::HL_ADDR, Register::A>,
            &SM83::op_ld_r_r<Register::A, Register::B>,
            &SM83::op_ld_r_r<Register::A, Register::C>,
            &SM83::op_ld_r_r<Register::A, Register::D>,
            &SM83::op_ld_r_r<Register::A, Register::E>,
            &SM83::op_ld_r_r<Register::A, Register::H>,
            &SM83::op_ld_r_r<Register::A, Register::L>,
            &SM83::op_ld_r_r<Register::A, Register::HL_ADDR>,
            &SM83::op_ld_r_r<Register::A, Register::A>,

            // 0x80 - 0x8F
            &SM83::op_add_a_r<Register::B, WithoutCarry>,
            &SM83::op_add_a_r<Register::C, WithoutCarry>,
            &SM83::op_add_a_r<Register::D, WithoutCarry>,
            &SM83::op_add_a_r<Register::E, WithoutCarry>,
            &SM83::op_add_a_r<Register::H, WithoutCarry>,
            &SM83::op_add_a_r<Register::L, WithoutCarry>,
            &SM83::op_add_a_r<Register::HL_ADDR, WithoutCarry>,
            &SM83::op_add_a_r<Register::A, WithoutCarry>,
            &SM83::op_add_a_r<Register::B, WithCarry>,
            &SM83::op_add_a_r<Register::C, WithCarry>,
            &SM83::op_add_a_r<Register::D, WithCarry>,
            &SM83::op_add_a_r<Register::E, WithCarry>,
            &SM83::op_add_a_r<Register::H, WithCarry>,
            &SM83::op_add_a_r<Register::L, WithCarry>,
            &SM83::op_add_a_r<Register::HL_ADDR, WithCarry>,
            &SM83::op_add_a_r<Register::A, WithCarry>,

            // 0x90 - 0x9F
            &SM83::op_sub_a_r<Register::B, WithoutCarry>,
            &SM83::op_sub_a_r<Register::C, WithoutCarry>,
            &SM83::op_sub_a_r<Register::D, WithoutCarry>,
            &SM83::op_sub_a_r<Register::E, WithoutCarry>,
            &SM83::op_sub_a_r<Register::H, WithoutCarry>,
            &SM83::op_sub_a_r<Register::L, WithoutCarry>,
            &SM83::op_sub_a_r<Register::HL_ADDR, WithoutCarry>,
            &SM83::op_sub_a_r<Register::A, WithoutCarry>,
            &SM83::op_sub_a_r<Register::B, WithCarry>,
            &SM83::op_sub_a_r<Register::C, WithCarry>,
            &SM83::op_sub_a_r<Register::D, WithCarry>,
            &SM83::op_sub_a_r<Register::E, WithCarry>,
            &SM83::op_sub_a_r<Register::H, WithCarry>,
            &SM83::op_sub_a_r<Register::L, WithCarry>,
            &SM83::op_sub_a_r<Register::HL_ADDR, WithCarry>,
            &SM83::op_sub_a_r<Register::A, WithCarry>,

            // 0xA0 - 0xAF
            &SM83::op_and_a_r<Register::B>,
            &SM83::op_and_a_r<Register::C>,
            &SM83::op_and_a_r<Register::D>,
            &SM83::op_and_a_r<Register::E>,
            &SM83::op_and_a_r<Register::H>,
            &SM83::op_and_a_r<Register::L>,
            &SM83::op_and_a_r<Register::HL_ADDR>,
            &SM83::op_and_a_r<Register::A>,
            &SM83::op_xor_a_r<Register::B>,
            &SM83::op_xor_a_r<Register::C>,
            &SM83::op_xor_a_r<Register::D>,
            &SM83::op_xor_a_r<Register::E>,
            &SM83::op_xor_a_r<Register::H>,
            &SM83::op_xor_a_r<Register::L>,
            &SM83::op_xor_a_r<Register::HL_ADDR>,
            &SM83::op_xor_a_r<Register::A>,

            // 0xB0 - 0xBF
            &SM83::op_or_a_r<Register::B>,
            &SM83::op_or_a_r<Register::C>,
            &SM83::op_or_a_r<Register::D>,
            &SM83::op_or_a_r<Register::E>,
            &SM83::op_or_a_r<Register::H>,
            &SM83::op_or_a_r<Register::L>,
            &SM83::op_or_a_r<Register::HL_ADDR>,
            &SM83::op_or_a_r<Register::A>,
            &SM83::op_cp_a_r<Register::B>,
            &SM83::op_cp_a_r<Register::C>,
            &SM83::op_cp_a_r<Register::D>,
            &SM83::op_cp_a_r<Register::E>,
            &SM83::op_cp_a_r<Register::H>,
            &SM83::op_cp_a_r<Register::L>,
            &SM83::op_cp_a_r<Register::HL_ADDR>,
            &SM83::op_cp_a_r<Register::A>,

            // 0xC0 - 0xCF
            &SM83::op_ret_cc<FLAG_Z, false>,
            &SM83::op_pop_rp<RegisterPair::BC>,
            &SM83::op_jp_cc_u16<FLAG_Z, false>,
            &SM83::op_jp_u16,
            &SM83::op_call_cc_u16<FLAG_Z, false>,
            &SM83::op_push_rp<RegisterPair::BC>,
            &SM83::op_add_a_r<Register::U8, WithoutCarry>,
            &SM83::op_rst_n<0x00>,
            &SM83::op_ret_cc<FLAG_Z, true>,
            &SM83::op_ret<IgnoreIME>,
            &SM83::op_jp_cc_u16<FLAG_Z, true>,
            &SM83::op_cb,
            &SM83::op_call_cc_u16<FLAG_Z, true>,
            &SM83::op_call_u16,
            &SM83::op_add_a_r<Register::U8, WithCarry>,
            &SM83::op_rst_n<0x08>,

            // 0xD0 - 0xDF
            &SM83::op_ret_cc<FLAG_CY, false>,
            &SM83::op_pop_rp<RegisterPair::DE>,
            &SM83::op_jp_cc_u16<FLAG_CY, false>,
            &SM83::op_nop<true, 0xD3>,
            &SM83::op_call_cc_u16<FLAG_CY, false>,
            &SM83::op_push_rp<RegisterPair::DE>,
            &SM83::op_sub_a_r<Register::U8, WithoutCarry>,
            &SM83::op_rst_n<0x10>,
            &SM83::op_ret_cc<FLAG_CY, true>,
            &SM83::op_ret<SetIME>,
            &SM83::op_jp_cc_u16<FLAG_CY, true>,
            &SM83::op_nop<true, 0xDB>,
            &SM83::op_call_cc_u16<FLAG_CY, true>,
            &SM83::op_nop<true, 0xDD>,
            &SM83::op_sub_a_r<Register::U8, WithCarry>,
            &SM83::op_rst_n<0x18>,

            // 0xE0 - 0xEF
            &SM83::op_ld_ff00_u8_a,
            &SM83::op_pop_rp<RegisterPair::HL>,
            &SM83::op_ld_ff00_c_a,
            &SM83::op_nop<true, 0xE3>,
            &SM83::op_nop<true, 0xE4>,
            &SM83::op_push_rp<RegisterPair::HL>,
            &SM83::op_and_a_r<Register::U8>,
            &SM83::op_rst_n<0x20>,
            &SM83::op_add_sp_i8,
            &SM83::op_jp_hl,
            &SM83::op_ld_u16_a,
            &SM83::op_nop<true, 0xEB>,
            &SM83::op_nop<true, 0xEC>,
            &SM83::op_nop<true, 0xED>,
            &SM83::op_xor_a_r<Register::U8>,
            &SM83::op_rst_n<0x28>,

            // 0xF0 - 0xFF
            &SM83::op_ld_a_ff00_u8,
            &SM83::op_pop_rp<RegisterPair::AF>,
            &SM83::op_ld_a_ff00_c,
            &SM83::op_di,
            &SM83::op_nop<true, 0xF4>,
            &SM83::op_push_rp<RegisterPair::AF>,
            &SM83::op_or_a_r<Register::U8>,
            &SM83::op_rst_n<0x30>,
            &SM83::op_ld_hl_sp_i8,
            &SM83::op_ld_sp_hl,
            &SM83::op_ld_a_u16,
            &SM83::op_ei,
            &SM83::op_nop<true, 0xFC>,
            &SM83::op_nop<true, 0xFD>,
            &SM83::op_cp_a_r<Register::U8>,
            &SM83::op_rst_n<0x38>,
        };
    }

    std::array<SM83::opcode_function, 256> SM83::gen_cb_optable() {
        return std::array<SM83::opcode_function, 256>{
            // 0x00 - 0x0F
            &SM83::op_rlc<Register::B>,
            &SM83::op_rlc<Register::C>,
            &SM83::op_rlc<Register::D>,
            &SM83::op_rlc<Register::E>,
            &SM83::op_rlc<Register::H>,
            &SM83::op_rlc<Register::L>,
            &SM83::op_rlc<Register::HL_ADDR>,
            &SM83::op_rlc<Register::A>,

            &SM83::op_rrc<Register::B>,
            &SM83::op_rrc<Register::C>,
            &SM83::op_rrc<Register::D>,
            &SM83::op_rrc<Register::E>,
            &SM83::op_rrc<Register::H>,
            &SM83::op_rrc<Register::L>,
            &SM83::op_rrc<Register::HL_ADDR>,
            &SM83::op_rrc<Register::A>,

            // 0x10 - 0x1F
            &SM83::op_rl<Register::B>,
            &SM83::op_rl<Register::C>,
            &SM83::op_rl<Register::D>,
            &SM83::op_rl<Register::E>,
            &SM83::op_rl<Register::H>,
            &SM83::op_rl<Register::L>,
            &SM83::op_rl<Register::HL_ADDR>,
            &SM83::op_rl<Register::A>,

            &SM83::op_rr<Register::B>,
            &SM83::op_rr<Register::C>,
            &SM83::op_rr<Register::D>,
            &SM83::op_rr<Register::E>,
            &SM83::op_rr<Register::H>,
            &SM83::op_rr<Register::L>,
            &SM83::op_rr<Register::HL_ADDR>,
            &SM83::op_rr<Register::A>,

            // 0x20 - 0x2F
            &SM83::op_sla<Register::B>,
            &SM83::op_sla<Register::C>,
            &SM83::op_sla<Register::D>,
            &SM83::op_sla<Register::E>,
            &SM83::op_sla<Register::H>,
            &SM83::op_sla<Register::L>,
            &SM83::op_sla<Register::HL_ADDR>,
            &SM83::op_sla<Register::A>,

            &SM83::op_sra<Register::B>,
            &SM83::op_sra<Register::C>,
            &SM83::op_sra<Register::D>,
            &SM83::op_sra<Register::E>,
            &SM83::op_sra<Register::H>,
            &SM83::op_sra<Register::L>,
            &SM83::op_sra<Register::HL_ADDR>,
            &SM83::op_sra<Register::A>,

            // 0x30 - 0x3F
            &SM83::op_swap<Register::B>,
            &SM83::op_swap<Register::C>,
            &SM83::op_swap<Register::D>,
            &SM83::op_swap<Register::E>,
            &SM83::op_swap<Register::H>,
            &SM83::op_swap<Register::L>,
            &SM83::op_swap<Register::HL_ADDR>,
            &SM83::op_swap<Register::A>,

            &SM83::op_srl<Register::B>,
            &SM83::op_srl<Register::C>,
            &SM83::op_srl<Register::D>,
            &SM83::op_srl<Register::E>,
            &SM83::op_srl<Register::H>,
            &SM83::op_srl<Register::L>,
            &SM83::op_srl<Register::HL_ADDR>,
            &SM83::op_srl<Register::A>,

            // 0x40 - 0x4F
            &SM83::op_bit<0, Register::B>,
            &SM83::op_bit<0, Register::C>,
            &SM83::op_bit<0, Register::D>,
            &SM83::op_bit<0, Register::E>,
            &SM83::op_bit<0, Register::H>,
            &SM83::op_bit<0, Register::L>,
            &SM83::op_bit<0, Register::HL_ADDR>,
            &SM83::op_bit<0, Register::A>,

            &SM83::op_bit<1, Register::B>,
            &SM83::op_bit<1, Register::C>,
            &SM83::op_bit<1, Register::D>,
            &SM83::op_bit<1, Register::E>,
            &SM83::op_bit<1, Register::H>,
            &SM83::op_bit<1, Register::L>,
            &SM83::op_bit<1, Register::HL_ADDR>,
            &SM83::op_bit<1, Register::A>,

            // 0x50 - 0x5F
            &SM83::op_bit<2, Register::B>,
            &SM83::op_bit<2, Register::C>,
            &SM83::op_bit<2, Register::D>,
            &SM83::op_bit<2, Register::E>,
            &SM83::op_bit<2, Register::H>,
            &SM83::op_bit<2, Register::L>,
            &SM83::op_bit<2, Register::HL_ADDR>,
            &SM83::op_bit<2, Register::A>,

            &SM83::op_bit<3, Register::B>,
            &SM83::op_bit<3, Register::C>,
            &SM83::op_bit<3, Register::D>,
            &SM83::op_bit<3, Register::E>,
            &SM83::op_bit<3, Register::H>,
            &SM83::op_bit<3, Register::L>,
            &SM83::op_bit<3, Register::HL_ADDR>,
            &SM83::op_bit<3, Register::A>,

            // 0x60 - 0x6F
            &SM83::op_bit<4, Register::B>,
            &SM83::op_bit<4, Register::C>,
            &SM83::op_bit<4, Register::D>,
            &SM83::op_bit<4, Register::E>,
            &SM83::op_bit<4, Register::H>,
            &SM83::op_bit<4, Register::L>,
            &SM83::op_bit<4, Register::HL_ADDR>,
            &SM83::op_bit<4, Register::A>,

            &SM83::op_bit<5, Register::B>,
            &SM83::op_bit<5, Register::C>,
            &SM83::op_bit<5, Register::D>,
            &SM83::op_bit<5, Register::E>,
            &SM83::op_bit<5, Register::H>,
            &SM83::op_bit<5, Register::L>,
            &SM83::op_bit<5, Register::HL_ADDR>,
            &SM83::op_bit<5, Register::A>,

            // 0x70 - 0x7F
            &SM83::op_bit<6, Register::B>,
            &SM83::op_bit<6, Register::C>,
            &SM83::op_bit<6, Register::D>,
            &SM83::op_bit<6, Register::E>,
            &SM83::op_bit<6, Register::H>,
            &SM83::op_bit<6, Register::L>,
            &SM83::op_bit<6, Register::HL_ADDR>,
            &SM83::op_bit<6, Register::A>,

            &SM83::op_bit<7, Register::B>,
            &SM83::op_bit<7, Register::C>,
            &SM83::op_bit<7, Register::D>,
            &SM83::op_bit<7, Register::E>,
            &SM83::op_bit<7, Register::H>,
            &SM83::op_bit<7, Register::L>,
            &SM83::op_bit<7, Register::HL_ADDR>,
            &SM83::op_bit<7, Register::A>,

            // 0x80 - 0x8F
            &SM83::op_res<0, Register::B>,
            &SM83::op_res<0, Register::C>,
            &SM83::op_res<0, Register::D>,
            &SM83::op_res<0, Register::E>,
            &SM83::op_res<0, Register::H>,
            &SM83::op_res<0, Register::L>,
            &SM83::op_res<0, Register::HL_ADDR>,
            &SM83::op_res<0, Register::A>,

            &SM83::op_res<1, Register::B>,
            &SM83::op_res<1, Register::C>,
            &SM83::op_res<1, Register::D>,
            &SM83::op_res<1, Register::E>,
            &SM83::op_res<1, Register::H>,
            &SM83::op_res<1, Register::L>,
            &SM83::op_res<1, Register::HL_ADDR>,
            &SM83::op_res<1, Register::A>,

            // 0x90 - 0x9F
            &SM83::op_res<2, Register::B>,
            &SM83::op_res<2, Register::C>,
            &SM83::op_res<2, Register::D>,
            &SM83::op_res<2, Register::E>,
            &SM83::op_res<2, Register::H>,
            &SM83::op_res<2, Register::L>,
            &SM83::op_res<2, Register::HL_ADDR>,
            &SM83::op_res<2, Register::A>,

            &SM83::op_res<3, Register::B>,
            &SM83::op_res<3, Register::C>,
            &SM83::op_res<3, Register::D>,
            &SM83::op_res<3, Register::E>,
            &SM83::op_res<3, Register::H>,
            &SM83::op_res<3, Register::L>,
            &SM83::op_res<3, Register::HL_ADDR>,
            &SM83::op_res<3, Register::A>,

            // 0xA0 - 0xAF
            &SM83::op_res<4, Register::B>,
            &SM83::op_res<4, Register::C>,
            &SM83::op_res<4, Register::D>,
            &SM83::op_res<4, Register::E>,
            &SM83::op_res<4, Register::H>,
            &SM83::op_res<4, Register::L>,
            &SM83::op_res<4, Register::HL_ADDR>,
            &SM83::op_res<4, Register::A>,

            &SM83::op_res<5, Register::B>,
            &SM83::op_res<5, Register::C>,
            &SM83::op_res<5, Register::D>,
            &SM83::op_res<5, Register::E>,
            &SM83::op_res<5, Register::H>,
            &SM83::op_res<5, Register::L>,
            &SM83::op_res<5, Register::HL_ADDR>,
            &SM83::op_res<5, Register::A>,

            // 0xB0 - 0xBF
            &SM83::op_res<6, Register::B>,
            &SM83::op_res<6, Register::C>,
            &SM83::op_res<6, Register::D>,
            &SM83::op_res<6, Register::E>,
            &SM83::op_res<6, Register::H>,
            &SM83::op_res<6, Register::L>,
            &SM83::op_res<6, Register::HL_ADDR>,
            &SM83::op_res<6, Register::A>,

            &SM83::op_res<7, Register::B>,
            &SM83::op_res<7, Register::C>,
            &SM83::op_res<7, Register::D>,
            &SM83::op_res<7, Register::E>,
            &SM83::op_res<7, Register::H>,
            &SM83::op_res<7, Register::L>,
            &SM83::op_res<7, Register::HL_ADDR>,
            &SM83::op_res<7, Register::A>,

            // 0xC0 - 0xCF
            &SM83::op_set<0, Register::B>,
            &SM83::op_set<0, Register::C>,
            &SM83::op_set<0, Register::D>,
            &SM83::op_set<0, Register::E>,
            &SM83::op_set<0, Register::H>,
            &SM83::op_set<0, Register::L>,
            &SM83::op_set<0, Register::HL_ADDR>,
            &SM83::op_set<0, Register::A>,

            &SM83::op_set<1, Register::B>,
            &SM83::op_set<1, Register::C>,
            &SM83::op_set<1, Register::D>,
            &SM83::op_set<1, Register::E>,
            &SM83::op_set<1, Register::H>,
            &SM83::op_set<1, Register::L>,
            &SM83::op_set<1, Register::HL_ADDR>,
            &SM83::op_set<1, Register::A>,

            // 0xD0 - 0xDF
            &SM83::op_set<2, Register::B>,
            &SM83::op_set<2, Register::C>,
            &SM83::op_set<2, Register::D>,
            &SM83::op_set<2, Register::E>,
            &SM83::op_set<2, Register::H>,
            &SM83::op_set<2, Register::L>,
            &SM83::op_set<2, Register::HL_ADDR>,
            &SM83::op_set<2, Register::A>,

            &SM83::op_set<3, Register::B>,
            &SM83::op_set<3, Register::C>,
            &SM83::op_set<3, Register::D>,
            &SM83::op_set<3, Register::E>,
            &SM83::op_set<3, Register::H>,
            &SM83::op_set<3, Register::L>,
            &SM83::op_set<3, Register::HL_ADDR>,
            &SM83::op_set<3, Register::A>,

            // 0xE0 - 0xEF
            &SM83::op_set<4, Register::B>,
            &SM83::op_set<4, Register::C>,
            &SM83::op_set<4, Register::D>,
            &SM83::op_set<4, Register::E>,
            &SM83::op_set<4, Register::H>,
            &SM83::op_set<4, Register::L>,
            &SM83::op_set<4, Register::HL_ADDR>,
            &SM83::op_set<4, Register::A>,

            &SM83::op_set<5, Register::B>,
            &SM83::op_set<5, Register::C>,
            &SM83::op_set<5, Register::D>,
            &SM83::op_set<5, Register::E>,
            &SM83::op_set<5, Register::H>,
            &SM83::op_set<5, Register::L>,
            &SM83::op_set<5, Register::HL_ADDR>,
            &SM83::op_set<5, Register::A>,

            // 0xF0 - 0xFF
            &SM83::op_set<6, Register::B>,
            &SM83::op_set<6, Register::C>,
            &SM83::op_set<6, Register::D>,
            &SM83::op_set<6, Register::E>,
            &SM83::op_set<6, Register::H>,
            &SM83::op_set<6, Register::L>,
            &SM83::op_set<6, Register::HL_ADDR>,
            &SM83::op_set<6, Register::A>,

            &SM83::op_set<7, Register::B>,
            &SM83::op_set<7, Register::C>,
            &SM83::op_set<7, Register::D>,
            &SM83::op_set<7, Register::E>,
            &SM83::op_set<7, Register::H>,
            &SM83::op_set<7, Register::L>,
            &SM83::op_set<7, Register::HL_ADDR>,
            &SM83::op_set<7, Register::A>,
        };
    }

}
