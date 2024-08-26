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
#include "Constants.hpp"
#include <stdexcept>
#include <utility>

namespace GB {
    SM83::SM83(std::function<void(int32_t)> run_external_state_fn,
               std::function<void(uint16_t, uint8_t)> bus_write_fn,
               std::function<uint8_t(uint16_t)> bus_read_fn)
        : bus_read_fn(std::move(bus_read_fn)), bus_write_fn(std::move(bus_write_fn)),
          run_external_state_fn(std::move(run_external_state_fn)) {
        gen_opcodes();
        gen_bitwise_opcodes();

        reset(0x0000, true);
    }

    bool SM83::stopped() const { return stopped_; }

    bool SM83::double_speed() const { return double_speed_; }

    bool SM83::ime() const { return master_interrupt_enable_; }

    bool SM83::halted() const { return halted_; }

    void SM83::reset(uint16_t new_pc, bool with_DMG_values) {
        dmg_mode = with_DMG_values;
        master_interrupt_enable_ = false;
        double_speed_ = false;
        halted_ = false;
        stopped_ = false;
        ei_delay_ = false;
        interrupt_enable = 0;
        interrupt_flag = 0;

        if (dmg_mode) {
            set_flags(FLAG_N, false);
            set_flags(FLAG_Z, true);
            set_flags(FLAG_HC | FLAG_CY, true);

            a = 0x01;
            b = 0x00;
            c = 0x13;
            d = 0x00;
            e = 0xD8;
            h = 0x01;
            l = 0x4D;

            program_counter = new_pc;
            stack_pointer = 0xFFFE;
        } else {
            set_flags(FLAG_N | FLAG_HC | FLAG_CY, false);
            set_flags(FLAG_Z, true);

            a = 0x11;
            b = 0x00;
            c = 0x00;
            d = 0xFF;
            e = 0x56;
            h = 0x00;
            l = 0x0D;

            program_counter = new_pc;
            stack_pointer = 0xFFFE;
        }
    }

    void SM83::request_interrupt(uint8_t interrupt) { interrupt_flag |= interrupt; }

    void SM83::step() {
        service_interrupts();

        if (ei_delay_) {
            master_interrupt_enable_ = true;
            ei_delay_ = false;
        }

        uint8_t opcode = bus_read_fn(program_counter);

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
                This happens regardless of IME which only controls whether the pending
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
                run_external_state_fn(8);
                push_sp(program_counter);
                run_external_state_fn(4);
                program_counter = 0x40;

                interrupt_flag &= ~INT_VBLANK_BIT;
            } else if (interrupt_pending & INT_LCD_STAT_BIT) {
                run_external_state_fn(8);
                push_sp(program_counter);
                run_external_state_fn(4);
                program_counter = 0x48;

                interrupt_flag &= ~INT_LCD_STAT_BIT;
            } else if (interrupt_pending & INT_TIMER_BIT) {
                run_external_state_fn(8);
                push_sp(program_counter);
                run_external_state_fn(4);
                program_counter = 0x50;

                interrupt_flag &= ~INT_TIMER_BIT;
            } else if (interrupt_pending & INT_SERIAL_PORT_BIT) {
                run_external_state_fn(8);
                push_sp(program_counter);
                run_external_state_fn(4);
                program_counter = 0x58;

                interrupt_flag &= ~INT_SERIAL_PORT_BIT;
            } else if (interrupt_pending & INT_JOYPAD_BIT) {
                run_external_state_fn(8);
                push_sp(program_counter);
                run_external_state_fn(4);
                program_counter = 0x60;

                interrupt_flag &= ~INT_JOYPAD_BIT;
            }
        }
    }

    uint16_t SM83::read_uint16(uint16_t address) const {
        uint8_t low = bus_read_fn(address);
        uint8_t high = bus_read_fn(address + 1);

        return (static_cast<uint16_t>(high) << 8) | static_cast<uint16_t>(low);
    }

    void SM83::write_uint16(uint16_t address, uint16_t value) const {
        bus_write_fn(address, value & 0x00FF);
        bus_write_fn(address + 1, (value & 0xFF00) >> 8);
    }

    void SM83::push_sp(uint16_t temp) {
        bus_write_fn(--stack_pointer, (temp & 0xFF00) >> 8);
        bus_write_fn(--stack_pointer, (temp & 0xFF));
    }

    uint16_t SM83::pop_sp() {
        uint8_t low = bus_read_fn(stack_pointer++);
        uint8_t high = bus_read_fn(stack_pointer++);
        return (high << 8) | low;
    }

    void SM83::set_flags(uint8_t flags, bool set) {
        if (set) {
            f |= flags;
        } else {
            f &= ~flags;
        }
    }

    bool SM83::get_flag(uint8_t flag) const { return f & flag; }

    uint16_t SM83::get_rp(RegisterPair index) const {
        uint16_t high;
        uint16_t low;
        switch (index) {
        case RegisterPair::BC: {
            high = b;
            low = c;
            break;
        }
        case RegisterPair::DE: {
            high = d;
            low = e;
            break;
        }
        case RegisterPair::HL: {
            high = h;
            low = l;
            break;
        }
        case RegisterPair::SP: {
            return stack_pointer;
        }
        case RegisterPair::AF: {
            high = a;
            low = f;
            break;
        }
        }

        return (high << 8) | (low);
    }

    void SM83::set_rp(RegisterPair index, uint16_t temp) {
        switch (index) {
        case RegisterPair::BC: {
            b = static_cast<uint8_t>((temp & 0xFF00) >> 8);
            c = static_cast<uint8_t>(temp & 0x00FF);
            return;
        }
        case RegisterPair::DE: {
            d = static_cast<uint8_t>((temp & 0xFF00) >> 8);
            e = static_cast<uint8_t>(temp & 0x00FF);
            return;
        }
        case RegisterPair::HL: {
            h = static_cast<uint8_t>((temp & 0xFF00) >> 8);
            l = static_cast<uint8_t>(temp & 0x00FF);
            return;
        }
        case RegisterPair::SP: {
            stack_pointer = temp;
            return;
        }
        case RegisterPair::AF: {
            a = static_cast<uint8_t>((temp & 0xFF00) >> 8);
            f = static_cast<uint8_t>(temp & 0x00FF);
            set_flags(0xF, false);
            return;
        }
        }
    }

    uint8_t SM83::get_register(Operand reg) const {
        switch (reg) {
        case Operand::B: return b;
        case Operand::C: return c;
        case Operand::D: return d;
        case Operand::E: return e;
        case Operand::H: return h;
        case Operand::L: return l;
        case Operand::A: return a;
        case Operand::F: return f;
        default: return 0;
        }
    }

    void SM83::set_register(Operand reg, uint8_t value) {
        switch (reg) {
        case Operand::B: b = value; return;
        case Operand::C: c = value; return;
        case Operand::D: d = value; return;
        case Operand::E: e = value; return;
        case Operand::H: h = value; return;
        case Operand::L: l = value; return;
        case Operand::A: a = value; return;
        case Operand::F: f = value; return;
        default: return;
        }
    }

    void SM83::op_ld_u16_sp() {
        auto addr = read_uint16(program_counter + 1);

        write_uint16(addr, stack_pointer);
        program_counter += 3;
    }

    void SM83::op_stop() {
        if (dmg_mode) {
            stopped_ = true;
            return;
        }

        if (KEY1 & 0x1) {
            double_speed_ = !double_speed_;
            KEY1 = double_speed_ << 7;
        }

        program_counter += 2;
    }

    void SM83::op_jr_i8() {
        auto offset = static_cast<int8_t>(bus_read_fn(program_counter + 1));

        program_counter += 2;
        program_counter += offset;
        run_external_state_fn(4);
    }

    void SM83::op_rlca() {
        auto temp = static_cast<uint16_t>(a);
        auto bit7 = temp & 0x80 ? 1 : 0;

        set_flags(FLAG_CY, bit7);
        set_flags(FLAG_Z | FLAG_N | FLAG_HC, false);
        temp = (temp << 1) | bit7;

        a = static_cast<uint8_t>(temp & 0xFF);
        ++program_counter;
    }

    void SM83::op_rrca() {
        auto temp = static_cast<uint16_t>(a);
        uint8_t bit0 = (temp & 1) ? 0x80 : 0;

        set_flags(FLAG_CY, (temp & 1));
        set_flags(FLAG_Z | FLAG_N | FLAG_HC, false);
        temp = (temp >> 1) | bit0;

        a = static_cast<uint8_t>(temp & 0xFF);
        ++program_counter;
    }

    void SM83::op_rla() {
        auto temp = static_cast<uint16_t>(a);
        uint16_t cy = get_flag(FLAG_CY);

        set_flags(FLAG_CY, (temp & 0x80));
        set_flags(FLAG_Z | FLAG_N | FLAG_HC, false);
        temp = (temp << 1) | cy;

        a = static_cast<uint8_t>(temp & 0xFF);
        ++program_counter;
    }

    void SM83::op_rra() {
        auto temp = static_cast<uint16_t>(a);
        uint16_t cy = get_flag(FLAG_CY);

        cy <<= 7;
        set_flags(FLAG_CY, (temp & 1));
        set_flags(FLAG_Z | FLAG_N | FLAG_HC, false);
        temp = (temp >> 1) | cy;

        a = static_cast<uint8_t>(temp & 0xFF);
        ++program_counter;
    }

    void SM83::op_daa() {
        // Implementation adapted from: https://ehaskins.com/2018-01-30%20Z80%20DAA/
        uint8_t correct = 0;
        auto temp = static_cast<uint16_t>(a);
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

        a = static_cast<uint8_t>(temp);
        ++program_counter;
    }

    void SM83::op_cpl() {
        a = ~a;
        set_flags(FLAG_N | FLAG_HC, true);
        ++program_counter;
    }

    void SM83::op_scf() {
        set_flags(FLAG_N | FLAG_HC, false);
        set_flags(FLAG_CY, true);
        ++program_counter;
    }

    void SM83::op_ccf() {
        set_flags(FLAG_N | FLAG_HC, false);
        set_flags(FLAG_CY, !get_flag(FLAG_CY));
        ++program_counter;
    }

    void SM83::op_jp_u16() {
        program_counter = read_uint16(program_counter + 1);
        run_external_state_fn(4);
    }

    void SM83::op_call_u16() {
        uint16_t saved_pc = program_counter + 3;
        auto addr = read_uint16(program_counter + 1);
        run_external_state_fn(4);
        push_sp(saved_pc);
        program_counter = addr;
    }

    void SM83::op_cb() {
        uint8_t opcode = bus_read_fn(program_counter + 1);

        (this->*bitwise_opcodes.at(opcode))();
        program_counter += 2;
    }

    void SM83::op_ld_ff00_u8_a() {
        uint8_t offset = bus_read_fn(program_counter + 1);
        bus_write_fn(0xFF00 + offset, a);
        program_counter += 2;
    }

    void SM83::op_ld_ff00_c_a() {
        bus_write_fn(0xFF00 + c, a);
        ++program_counter;
    }

    void SM83::op_add_sp_i8() {
        auto offset = static_cast<int8_t>(bus_read_fn(program_counter + 1));
        uint32_t sp32 = stack_pointer;
        uint32_t res32 = (sp32 + offset);

        // internal operation?
        run_external_state_fn(4);
        set_rp(RegisterPair::SP, res32 & 0xFFFF);
        // SP update is visible
        run_external_state_fn(4);
        set_flags(FLAG_HC, ((sp32 & 0xF) + (offset & 0xF)) > 0xF);
        set_flags(FLAG_CY, ((sp32 & 0xFF) + (offset & 0xFF)) > 0xFF);
        set_flags(FLAG_Z | FLAG_N, false);
        program_counter += 2;
    }

    void SM83::op_jp_hl() { program_counter = get_rp(RegisterPair::HL); }

    void SM83::op_ld_u16_a() {
        auto addr = read_uint16(program_counter + 1);
        bus_write_fn(addr, a);
        program_counter += 3;
    }

    void SM83::op_ld_a_ff00_u8() {
        uint16_t offset = bus_read_fn(program_counter + 1);
        a = bus_read_fn(0xFF00 + offset);
        program_counter += 2;
    }

    void SM83::op_ld_a_ff00_c() {
        a = bus_read_fn(0xFF00 + c);
        ++program_counter;
    }

    void SM83::op_di() {
        master_interrupt_enable_ = false;
        ei_delay_ = false;
        ++program_counter;
    }

    void SM83::op_ei() {
        ei_delay_ = true;
        ++program_counter;
    }

    void SM83::op_ld_hl_sp_i8() {
        auto offset = static_cast<int8_t>(bus_read_fn(program_counter + 1));
        uint16_t sp32 = stack_pointer;
        uint16_t res32 = (sp32 + offset);
        set_rp(RegisterPair::HL, static_cast<uint16_t>(res32 & 0xFFFF));
        set_flags(FLAG_HC, ((sp32 & 0xF) + (offset & 0xF)) > 0xF);
        set_flags(FLAG_CY, ((sp32 & 0xFF) + (offset & 0xFF)) > 0xFF);
        set_flags(FLAG_Z | FLAG_N, false);
        run_external_state_fn(4);
        program_counter += 2;
    }

    void SM83::op_ld_sp_hl() {
        stack_pointer = get_rp(RegisterPair::HL);
        run_external_state_fn(4);
        ++program_counter;
    }

    void SM83::op_ld_a_u16() {
        auto addr = read_uint16(program_counter + 1);
        a = bus_read_fn(addr);
        program_counter += 3;
    }

    template <bool is_illegal_op, uint8_t illegal_op> void SM83::op_nop() {
        if constexpr (is_illegal_op) {
            stopped_ = true;
            return;
        }
        ++program_counter;
    }

    template <RegisterPair rp> inline void SM83::op_ld_rp_u16() {
        uint16_t combine = read_uint16(program_counter + 1);

        set_rp(rp, combine);
        program_counter += 3;
    }

    template <RegisterPair rp> inline void SM83::op_inc_rp() {
        auto increment = [this](uint8_t &high, uint8_t &low) {
            uint16_t res = static_cast<uint16_t>(high) << 8 | static_cast<uint16_t>(low);

            ++res;
            high = static_cast<uint8_t>((res & 0xFF00) >> 8);
            low = static_cast<uint8_t>(res & 0xFF);

            run_external_state_fn(4);
            ++program_counter;
        };

        switch (rp) {
        case RegisterPair::BC: {
            increment(b, c);
            return;
        }
        case RegisterPair::DE: {
            increment(d, e);
            return;
        }
        case RegisterPair::HL: {
            increment(h, l);
            return;
        }
        case RegisterPair::SP: {
            stack_pointer++;
            run_external_state_fn(4);
            ++program_counter;
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

            run_external_state_fn(4);
            ++program_counter;
        };

        switch (rp) {
        case RegisterPair::BC: {
            decrement(b, c);
            return;
        }
        case RegisterPair::DE: {
            decrement(d, e);
            return;
        }
        case RegisterPair::HL: {
            decrement(h, l);
            return;
        }
        case RegisterPair::SP: {
            stack_pointer--;
            run_external_state_fn(4);
            ++program_counter;
            return;
        }
        }
    }

    template <RegisterPair rp, int16_t displacement> inline void SM83::op_ld_rp_a() {
        uint16_t addr = get_rp(rp);

        bus_write_fn(addr, a);

        if constexpr (displacement != 0) {
            set_rp(rp, addr + displacement);
        }
        ++program_counter;
    }

    template <uint8_t cc, bool boolean_ver> inline void SM83::op_jr_cc_i8() {
        if (get_flag(cc) == boolean_ver) {
            auto offset = static_cast<int8_t>(bus_read_fn(program_counter + 1));

            program_counter += 2;
            program_counter += offset;
            run_external_state_fn(4);
            return;
        }
        run_external_state_fn(4);
        program_counter += 2;
    }

    template <RegisterPair rp> inline void SM83::op_add_hl_rp() {
        int32_t left = get_rp(RegisterPair::HL);
        int32_t right = get_rp(rp);
        int32_t res32 = left + right;

        set_flags(FLAG_N, false);
        set_flags(FLAG_HC, ((left & 0xFFF) + (right & 0xFFF)) > 0xFFF);
        set_flags(FLAG_CY, res32 > 0xFFFF);
        set_rp(RegisterPair::HL, static_cast<uint16_t>(res32 & 0xFFFF));

        this->run_external_state_fn(4);
        ++program_counter;
    }

    template <Operand r> void SM83::op_inc_r() {
        uint16_t left;
        uint16_t right = 1;

        if constexpr (r == Operand::HLAddress) {
            left = bus_read_fn(get_rp(RegisterPair::HL));
        } else {
            left = get_register(r);
        }

        auto result = left + right;
        auto masked_result = static_cast<uint8_t>(result & 0xFF);

        set_flags(FLAG_N, false);
        set_flags(FLAG_Z, masked_result == 0);
        set_flags(FLAG_HC, ((left & 0xF) + (right & 0xF)) > 0xF);

        if constexpr (r == Operand::HLAddress) {
            bus_write_fn(get_rp(RegisterPair::HL), masked_result);
        } else {
            set_register(r, masked_result);
        }
        ++program_counter;
    }

    template <Operand r> void SM83::op_dec_r() {
        int16_t left;
        int16_t right = 1;

        if constexpr (r == Operand::HLAddress) {
            left = bus_read_fn(get_rp(RegisterPair::HL));
        } else {
            left = get_register(r);
        }

        auto result = left - right;
        auto masked_result = static_cast<uint8_t>(result & 0xFF);

        set_flags(FLAG_HC, ((left & 0xF) - (right & 0xF)) < 0);
        set_flags(FLAG_Z, masked_result == 0);
        set_flags(FLAG_N, true); // only set if subtraction

        if constexpr (r == Operand::HLAddress) {
            bus_write_fn(get_rp(RegisterPair::HL), masked_result);
        } else {
            set_register(r, masked_result);
        }
        ++program_counter;
    }

    template <Operand r> void SM83::op_ld_r_u8() {
        if constexpr (r == Operand::HLAddress) {
            bus_write_fn(get_rp(RegisterPair::HL), bus_read_fn(program_counter + 1));
        } else {
            set_register(r, bus_read_fn(program_counter + 1));
        }
        program_counter += 2;
    }

    template <RegisterPair rp, int16_t displacement> void SM83::op_ld_a_rp() {
        auto addr = get_rp(rp);

        a = bus_read_fn(addr);

        if constexpr (displacement != 0) {
            set_rp(rp, addr + displacement);
        }
        ++program_counter;
    }

    template <Operand r, Operand r2> void SM83::op_ld_r_r() {
        // r = destination
        // r2 = source
        if constexpr (r == Operand::HLAddress && r2 != Operand::HLAddress) {
            bus_write_fn(get_rp(RegisterPair::HL), get_register(r2));
        } else if constexpr (r != Operand::HLAddress && r2 == Operand::HLAddress) {
            set_register(r, bus_read_fn(get_rp(RegisterPair::HL)));
        } else if constexpr (r == Operand::HLAddress && r2 == Operand::HLAddress) {
            halted_ = true;
        } else if constexpr (r != Operand::HLAddress && r2 != Operand::HLAddress) {
            set_register(r, get_register(r2));
        }
        ++program_counter;
    }

    template <Operand r, bool with_carry> void SM83::op_add_a_r() {
        auto left = static_cast<uint16_t>(a);
        uint16_t right;

        if constexpr (r == Operand::HLAddress) {
            right = static_cast<uint16_t>(bus_read_fn(get_rp(RegisterPair::HL)));
        } else if constexpr (r == Operand::Immediate) {
            right = static_cast<uint16_t>(bus_read_fn(program_counter + 1));
            ++program_counter;
        } else {
            right = static_cast<uint16_t>(get_register(r));
        }

        uint16_t cy = with_carry ? get_flag(FLAG_CY) : 0;

        uint16_t result = left + right + cy;
        auto masked_result = static_cast<uint8_t>(result & 0xFF);

        set_flags(FLAG_HC, ((left & 0xF) + (right & 0xF) + (cy & 0xF)) > 0xF);
        set_flags(FLAG_CY, result > 0xFF);
        set_flags(FLAG_Z, masked_result == 0);
        set_flags(FLAG_N, false);

        a = masked_result;
        ++program_counter;
    }

    template <Operand r, bool with_carry> void SM83::op_sub_a_r() {
        auto left = static_cast<int16_t>(a);
        int16_t right;

        if constexpr (r == Operand::HLAddress) {
            right = static_cast<int16_t>(bus_read_fn(get_rp(RegisterPair::HL)));
        } else if constexpr (r == Operand::Immediate) {
            right = static_cast<int16_t>(bus_read_fn(program_counter + 1));
            ++program_counter;
        } else {
            right = static_cast<int16_t>(get_register(r));
        }

        int16_t cy = with_carry ? get_flag(FLAG_CY) : 0;

        auto result = static_cast<int16_t>(left - right - cy);
        auto masked_result = static_cast<uint8_t>(result & 0xFF);

        set_flags(FLAG_HC, ((left & 0xF) - (right & 0xF) - (cy & 0xF)) < 0);
        set_flags(FLAG_CY, result < 0);
        set_flags(FLAG_Z, masked_result == 0);
        set_flags(FLAG_N, true);

        a = masked_result;
        ++program_counter;
    }

    template <Operand r> void SM83::op_and_a_r() {
        uint8_t right;

        if constexpr (r == Operand::HLAddress) {
            right = bus_read_fn(get_rp(RegisterPair::HL));
        } else if constexpr (r == Operand::Immediate) {
            right = bus_read_fn(program_counter + 1);
            ++program_counter;
        } else {
            right = get_register(r);
        }

        uint8_t result = a & right;
        set_flags(FLAG_Z, result == 0);
        set_flags(FLAG_N, false);
        set_flags(FLAG_HC, true);
        set_flags(FLAG_CY, false);

        a = result;
        ++program_counter;
    }

    template <Operand r> void SM83::op_xor_a_r() {
        uint8_t right;

        if constexpr (r == Operand::HLAddress) {
            right = bus_read_fn(get_rp(RegisterPair::HL));
        } else if constexpr (r == Operand::Immediate) {
            right = bus_read_fn(program_counter + 1);
            ++program_counter;
        } else {
            right = get_register(r);
        }

        uint8_t result = a ^ right;
        set_flags(FLAG_Z, result == 0);
        set_flags(FLAG_N, false);
        set_flags(FLAG_HC, false);
        set_flags(FLAG_CY, false);

        a = result;
        ++program_counter;
    }

    template <Operand r> void SM83::op_or_a_r() {
        uint8_t right;

        if constexpr (r == Operand::HLAddress) {
            right = bus_read_fn(get_rp(RegisterPair::HL));
        } else if constexpr (r == Operand::Immediate) {
            right = bus_read_fn(program_counter + 1);
            ++program_counter;
        } else {
            right = get_register(r);
        }

        uint8_t result = a | right;
        set_flags(FLAG_Z, result == 0);
        set_flags(FLAG_N, false);
        set_flags(FLAG_HC, false);
        set_flags(FLAG_CY, false);

        a = result;
        ++program_counter;
    }

    template <Operand r> void SM83::op_cp_a_r() {
        uint8_t right;

        if constexpr (r == Operand::HLAddress) {
            right = bus_read_fn(get_rp(RegisterPair::HL));
        } else if constexpr (r == Operand::Immediate) {
            right = bus_read_fn(program_counter + 1);
            ++program_counter;
        } else {
            right = get_register(r);
        }

        int32_t result = a - right;
        auto masked_result = static_cast<uint8_t>(result & 0xFF);

        set_flags(FLAG_HC, ((a & 0xF) - (right & 0xF)) < 0);
        set_flags(FLAG_CY, result < 0);
        set_flags(FLAG_Z, masked_result == 0);
        set_flags(FLAG_N, true);

        // Operand::A is not modified, same as subtract without carry
        ++program_counter;
    }

    template <bool enable_interrupts> void SM83::op_ret() {
        if constexpr (enable_interrupts) {
            master_interrupt_enable_ = true;
        }

        program_counter = pop_sp();
        run_external_state_fn(4);
    }

    template <uint8_t cc, bool boolean_ver> void SM83::op_ret_cc() {
        run_external_state_fn(4);

        if (get_flag(cc) == boolean_ver) {
            program_counter = pop_sp();
            run_external_state_fn(4);
            return;
        }
        ++program_counter;
    }

    template <uint8_t cc, bool boolean_ver> void SM83::op_jp_cc_u16() {
        if (get_flag(cc) == boolean_ver) {
            program_counter = read_uint16(program_counter + 1);
            run_external_state_fn(4);
            return;
        }

        run_external_state_fn(8);
        program_counter += 3;
    }

    template <uint8_t cc, bool boolean_ver> void SM83::op_call_cc_u16() {
        if (get_flag(cc) == boolean_ver) {
            auto saved_pc = program_counter + 3;
            auto addr = read_uint16(program_counter + 1);
            run_external_state_fn(4);
            push_sp(saved_pc);
            program_counter = addr;
            return;
        }

        run_external_state_fn(8);
        program_counter += 3;
    }

    template <RegisterPair rp> void SM83::op_pop_rp() {
        uint16_t temp = pop_sp();
        set_rp(rp, temp);
        ++program_counter;
    }

    template <RegisterPair rp> void SM83::op_push_rp() {
        run_external_state_fn(4);
        push_sp(get_rp(rp));
        ++program_counter;
    }

    template <uint16_t page> void SM83::op_rst_n() {
        run_external_state_fn(4);
        push_sp(program_counter + 1);
        program_counter = page;
    }

    template <Operand r> void SM83::op_rlc() {
        uint8_t temp = get_register(r);

        if constexpr (r == Operand::HLAddress) {
            temp = bus_read_fn(get_rp(RegisterPair::HL));
        }

        uint8_t bit7 = (temp & 0x80) ? 1 : 0;

        set_flags(FLAG_CY, bit7);
        set_flags(FLAG_Z, temp == 0);
        set_flags(FLAG_N | FLAG_HC, false);
        temp = temp << 1;
        temp |= bit7;

        if constexpr (r == Operand::HLAddress) {
            bus_write_fn(get_rp(RegisterPair::HL), temp);
        } else {
            set_register(r, temp);
        }
    }

    template <Operand r> void SM83::op_rrc() {
        uint8_t temp = get_register(r);

        if constexpr (r == Operand::HLAddress) {
            temp = bus_read_fn(get_rp(RegisterPair::HL));
        }

        uint8_t bit0 = (temp & 0x01) ? 0x80 : 0;
        set_flags(FLAG_CY, bit0);
        set_flags(FLAG_Z, temp == 0);
        set_flags(FLAG_N | FLAG_HC, false);
        temp = temp >> 1;
        temp |= bit0;

        if constexpr (r == Operand::HLAddress) {
            bus_write_fn(get_rp(RegisterPair::HL), temp);
        } else {
            set_register(r, temp);
        }
    }

    template <Operand r> void SM83::op_rl() {
        uint16_t temp = get_register(r);

        if constexpr (r == Operand::HLAddress) {
            temp = bus_read_fn(get_rp(RegisterPair::HL));
        }

        uint16_t cy = get_flag(FLAG_CY);
        set_flags(FLAG_CY, (temp & 0x80));

        temp = temp << 1;
        temp = temp | cy;

        auto result = static_cast<uint8_t>(temp & 0xFF);
        set_flags(FLAG_Z, result == 0);
        set_flags(FLAG_N | FLAG_HC, false);

        if constexpr (r == Operand::HLAddress) {
            bus_write_fn(get_rp(RegisterPair::HL), result);
        } else {
            set_register(r, result);
        }
    }

    template <Operand r> void SM83::op_rr() {
        uint16_t temp = get_register(r);

        if constexpr (r == Operand::HLAddress) {
            temp = bus_read_fn(get_rp(RegisterPair::HL));
        }

        uint16_t cy = get_flag(FLAG_CY);
        cy = cy << 7;
        set_flags(FLAG_CY, (temp & 0x01));
        temp = temp >> 1;
        temp = temp | cy;

        auto result = static_cast<uint8_t>(temp & 0xFF);
        set_flags(FLAG_Z, result == 0);
        set_flags(FLAG_N | FLAG_HC, false);

        if constexpr (r == Operand::HLAddress) {
            bus_write_fn(get_rp(RegisterPair::HL), result);
        } else {
            set_register(r, result);
        }
    }

    template <Operand r> void SM83::op_sla() {
        uint8_t temp = get_register(r);

        if constexpr (r == Operand::HLAddress) {
            temp = bus_read_fn(get_rp(RegisterPair::HL));
        }

        set_flags(FLAG_CY, (temp & 0x80));
        temp = temp << 1;
        set_flags(FLAG_Z, temp == 0);
        set_flags(FLAG_N | FLAG_HC, false);

        if constexpr (r == Operand::HLAddress) {
            bus_write_fn(get_rp(RegisterPair::HL), temp);
        } else {
            set_register(r, temp);
        }
    }

    template <Operand r> void SM83::op_sra() {
        uint8_t temp = get_register(r);

        if constexpr (r == Operand::HLAddress) {
            temp = bus_read_fn(get_rp(RegisterPair::HL));
        }

        uint8_t bit7 = (temp & 0x80);
        set_flags(FLAG_CY, (temp & 0x01));
        temp = temp >> 1;
        temp |= bit7; // bit7 is left unchanged
        set_flags(FLAG_Z, temp == 0);
        set_flags(FLAG_N | FLAG_HC, false);

        if constexpr (r == Operand::HLAddress) {
            bus_write_fn(get_rp(RegisterPair::HL), temp);
        } else {
            set_register(r, temp);
        }
    }

    template <Operand r> void SM83::op_swap() {
        uint8_t temp = get_register(r);

        if constexpr (r == Operand::HLAddress) {
            temp = bus_read_fn(get_rp(RegisterPair::HL));
        }

        uint8_t high = (temp & 0xF0) >> 4;
        uint8_t low = (temp & 0x0F) << 4;
        temp = (low) | high;

        set_flags(FLAG_Z, temp == 0);
        set_flags(FLAG_N | FLAG_HC | FLAG_CY, false);

        if constexpr (r == Operand::HLAddress) {
            bus_write_fn(get_rp(RegisterPair::HL), temp);
        } else {
            set_register(r, temp);
        }
    }

    template <Operand r> void SM83::op_srl() {
        uint8_t temp = get_register(r);

        if constexpr (r == Operand::HLAddress) {
            temp = bus_read_fn(get_rp(RegisterPair::HL));
        }

        set_flags(FLAG_CY, (temp & 0x01));
        temp = temp >> 1;
        set_flags(FLAG_Z, temp == 0);
        set_flags(FLAG_N | FLAG_HC, false);

        if constexpr (r == Operand::HLAddress) {
            bus_write_fn(get_rp(RegisterPair::HL), temp);
        } else {
            set_register(r, temp);
        }
    }

    template <uint8_t bit, Operand r> void SM83::op_bit() {
        uint8_t temp = get_register(r);

        if constexpr (r == Operand::HLAddress) {
            temp = bus_read_fn(get_rp(RegisterPair::HL));
        }

        set_flags(FLAG_Z, !(temp & (1 << bit)));

        set_flags(FLAG_N, false);
        set_flags(FLAG_HC, true);
    }

    template <uint8_t bit, Operand r> void SM83::op_res() {
        uint8_t temp = get_register(r);

        if constexpr (r == Operand::HLAddress) {
            temp = bus_read_fn(get_rp(RegisterPair::HL));
        }

        temp &= ~(1 << bit);

        if constexpr (r == Operand::HLAddress) {
            bus_write_fn(get_rp(RegisterPair::HL), temp);
        } else {
            set_register(r, temp);
        }
    }

    template <uint8_t bit, Operand r> void SM83::op_set() {
        uint8_t temp = get_register(r);

        if constexpr (r == Operand::HLAddress) {
            temp = bus_read_fn(get_rp(RegisterPair::HL));
        }

        temp |= (1 << bit);

        if constexpr (r == Operand::HLAddress) {
            bus_write_fn(get_rp(RegisterPair::HL), temp);
        } else {
            set_register(r, temp);
        }
    }

    void SM83::gen_opcodes() {
        constexpr int16_t NoDisplacement = 0;
        constexpr int16_t Increment = 1;
        constexpr int16_t Decrement = -1;
        constexpr bool WithCarry = true;
        constexpr bool WithoutCarry = false;
        constexpr bool IgnoreIME = false;
        constexpr bool SetIME = true;

        opcodes = {
            // 0x00 - 0x0F
            &SM83::op_nop<false, 0>,
            &SM83::op_ld_rp_u16<RegisterPair::BC>,
            &SM83::op_ld_rp_a<RegisterPair::BC, NoDisplacement>,
            &SM83::op_inc_rp<RegisterPair::BC>,
            &SM83::op_inc_r<Operand::B>,
            &SM83::op_dec_r<Operand::B>,
            &SM83::op_ld_r_u8<Operand::B>,
            &SM83::op_rlca,
            &SM83::op_ld_u16_sp,
            &SM83::op_add_hl_rp<RegisterPair::BC>,
            &SM83::op_ld_a_rp<RegisterPair::BC, NoDisplacement>,
            &SM83::op_dec_rp<RegisterPair::BC>,
            &SM83::op_inc_r<Operand::C>,
            &SM83::op_dec_r<Operand::C>,
            &SM83::op_ld_r_u8<Operand::C>,
            &SM83::op_rrca,

            // 0x10 - 0x1F
            &SM83::op_stop,
            &SM83::op_ld_rp_u16<RegisterPair::DE>,
            &SM83::op_ld_rp_a<RegisterPair::DE, NoDisplacement>,
            &SM83::op_inc_rp<RegisterPair::DE>,
            &SM83::op_inc_r<Operand::D>,
            &SM83::op_dec_r<Operand::D>,
            &SM83::op_ld_r_u8<Operand::D>,
            &SM83::op_rla,
            &SM83::op_jr_i8,
            &SM83::op_add_hl_rp<RegisterPair::DE>,
            &SM83::op_ld_a_rp<RegisterPair::DE, NoDisplacement>,
            &SM83::op_dec_rp<RegisterPair::DE>,
            &SM83::op_inc_r<Operand::E>,
            &SM83::op_dec_r<Operand::E>,
            &SM83::op_ld_r_u8<Operand::E>,
            &SM83::op_rra,

            // 0x20 - 0x2F
            &SM83::op_jr_cc_i8<FLAG_Z, false>,
            &SM83::op_ld_rp_u16<RegisterPair::HL>,
            &SM83::op_ld_rp_a<RegisterPair::HL, Increment>,
            &SM83::op_inc_rp<RegisterPair::HL>,
            &SM83::op_inc_r<Operand::H>,
            &SM83::op_dec_r<Operand::H>,
            &SM83::op_ld_r_u8<Operand::H>,
            &SM83::op_daa,
            &SM83::op_jr_cc_i8<FLAG_Z, true>,
            &SM83::op_add_hl_rp<RegisterPair::HL>,
            &SM83::op_ld_a_rp<RegisterPair::HL, Increment>,
            &SM83::op_dec_rp<RegisterPair::HL>,
            &SM83::op_inc_r<Operand::L>,
            &SM83::op_dec_r<Operand::L>,
            &SM83::op_ld_r_u8<Operand::L>,
            &SM83::op_cpl,

            // 0x30 - 0x3F
            &SM83::op_jr_cc_i8<FLAG_CY, false>,
            &SM83::op_ld_rp_u16<RegisterPair::SP>,
            &SM83::op_ld_rp_a<RegisterPair::HL, Decrement>,
            &SM83::op_inc_rp<RegisterPair::SP>,
            &SM83::op_inc_r<Operand::HLAddress>,
            &SM83::op_dec_r<Operand::HLAddress>,
            &SM83::op_ld_r_u8<Operand::HLAddress>,
            &SM83::op_scf,
            &SM83::op_jr_cc_i8<FLAG_CY, true>,
            &SM83::op_add_hl_rp<RegisterPair::SP>,
            &SM83::op_ld_a_rp<RegisterPair::HL, Decrement>,
            &SM83::op_dec_rp<RegisterPair::SP>,
            &SM83::op_inc_r<Operand::A>,
            &SM83::op_dec_r<Operand::A>,
            &SM83::op_ld_r_u8<Operand::A>,
            &SM83::op_ccf,

            // 0x40 - 0x4F
            &SM83::op_ld_r_r<Operand::B, Operand::B>,
            &SM83::op_ld_r_r<Operand::B, Operand::C>,
            &SM83::op_ld_r_r<Operand::B, Operand::D>,
            &SM83::op_ld_r_r<Operand::B, Operand::E>,
            &SM83::op_ld_r_r<Operand::B, Operand::H>,
            &SM83::op_ld_r_r<Operand::B, Operand::L>,
            &SM83::op_ld_r_r<Operand::B, Operand::HLAddress>,
            &SM83::op_ld_r_r<Operand::B, Operand::A>,
            &SM83::op_ld_r_r<Operand::C, Operand::B>,
            &SM83::op_ld_r_r<Operand::C, Operand::C>,
            &SM83::op_ld_r_r<Operand::C, Operand::D>,
            &SM83::op_ld_r_r<Operand::C, Operand::E>,
            &SM83::op_ld_r_r<Operand::C, Operand::H>,
            &SM83::op_ld_r_r<Operand::C, Operand::L>,
            &SM83::op_ld_r_r<Operand::C, Operand::HLAddress>,
            &SM83::op_ld_r_r<Operand::C, Operand::A>,

            // 0x50 - 0x5F
            &SM83::op_ld_r_r<Operand::D, Operand::B>,
            &SM83::op_ld_r_r<Operand::D, Operand::C>,
            &SM83::op_ld_r_r<Operand::D, Operand::D>,
            &SM83::op_ld_r_r<Operand::D, Operand::E>,
            &SM83::op_ld_r_r<Operand::D, Operand::H>,
            &SM83::op_ld_r_r<Operand::D, Operand::L>,
            &SM83::op_ld_r_r<Operand::D, Operand::HLAddress>,
            &SM83::op_ld_r_r<Operand::D, Operand::A>,
            &SM83::op_ld_r_r<Operand::E, Operand::B>,
            &SM83::op_ld_r_r<Operand::E, Operand::C>,
            &SM83::op_ld_r_r<Operand::E, Operand::D>,
            &SM83::op_ld_r_r<Operand::E, Operand::E>,
            &SM83::op_ld_r_r<Operand::E, Operand::H>,
            &SM83::op_ld_r_r<Operand::E, Operand::L>,
            &SM83::op_ld_r_r<Operand::E, Operand::HLAddress>,
            &SM83::op_ld_r_r<Operand::E, Operand::A>,

            // 0x60 - 0x6F
            &SM83::op_ld_r_r<Operand::H, Operand::B>,
            &SM83::op_ld_r_r<Operand::H, Operand::C>,
            &SM83::op_ld_r_r<Operand::H, Operand::D>,
            &SM83::op_ld_r_r<Operand::H, Operand::E>,
            &SM83::op_ld_r_r<Operand::H, Operand::H>,
            &SM83::op_ld_r_r<Operand::H, Operand::L>,
            &SM83::op_ld_r_r<Operand::H, Operand::HLAddress>,
            &SM83::op_ld_r_r<Operand::H, Operand::A>,
            &SM83::op_ld_r_r<Operand::L, Operand::B>,
            &SM83::op_ld_r_r<Operand::L, Operand::C>,
            &SM83::op_ld_r_r<Operand::L, Operand::D>,
            &SM83::op_ld_r_r<Operand::L, Operand::E>,
            &SM83::op_ld_r_r<Operand::L, Operand::H>,
            &SM83::op_ld_r_r<Operand::L, Operand::L>,
            &SM83::op_ld_r_r<Operand::L, Operand::HLAddress>,
            &SM83::op_ld_r_r<Operand::L, Operand::A>,

            // 0x70 - 0x7F
            &SM83::op_ld_r_r<Operand::HLAddress, Operand::B>,
            &SM83::op_ld_r_r<Operand::HLAddress, Operand::C>,
            &SM83::op_ld_r_r<Operand::HLAddress, Operand::D>,
            &SM83::op_ld_r_r<Operand::HLAddress, Operand::E>,
            &SM83::op_ld_r_r<Operand::HLAddress, Operand::H>,
            &SM83::op_ld_r_r<Operand::HLAddress, Operand::L>,
            // HALT
            &SM83::op_ld_r_r<Operand::HLAddress, Operand::HLAddress>,
            &SM83::op_ld_r_r<Operand::HLAddress, Operand::A>,
            &SM83::op_ld_r_r<Operand::A, Operand::B>,
            &SM83::op_ld_r_r<Operand::A, Operand::C>,
            &SM83::op_ld_r_r<Operand::A, Operand::D>,
            &SM83::op_ld_r_r<Operand::A, Operand::E>,
            &SM83::op_ld_r_r<Operand::A, Operand::H>,
            &SM83::op_ld_r_r<Operand::A, Operand::L>,
            &SM83::op_ld_r_r<Operand::A, Operand::HLAddress>,
            &SM83::op_ld_r_r<Operand::A, Operand::A>,

            // 0x80 - 0x8F
            &SM83::op_add_a_r<Operand::B, WithoutCarry>,
            &SM83::op_add_a_r<Operand::C, WithoutCarry>,
            &SM83::op_add_a_r<Operand::D, WithoutCarry>,
            &SM83::op_add_a_r<Operand::E, WithoutCarry>,
            &SM83::op_add_a_r<Operand::H, WithoutCarry>,
            &SM83::op_add_a_r<Operand::L, WithoutCarry>,
            &SM83::op_add_a_r<Operand::HLAddress, WithoutCarry>,
            &SM83::op_add_a_r<Operand::A, WithoutCarry>,
            &SM83::op_add_a_r<Operand::B, WithCarry>,
            &SM83::op_add_a_r<Operand::C, WithCarry>,
            &SM83::op_add_a_r<Operand::D, WithCarry>,
            &SM83::op_add_a_r<Operand::E, WithCarry>,
            &SM83::op_add_a_r<Operand::H, WithCarry>,
            &SM83::op_add_a_r<Operand::L, WithCarry>,
            &SM83::op_add_a_r<Operand::HLAddress, WithCarry>,
            &SM83::op_add_a_r<Operand::A, WithCarry>,

            // 0x90 - 0x9F
            &SM83::op_sub_a_r<Operand::B, WithoutCarry>,
            &SM83::op_sub_a_r<Operand::C, WithoutCarry>,
            &SM83::op_sub_a_r<Operand::D, WithoutCarry>,
            &SM83::op_sub_a_r<Operand::E, WithoutCarry>,
            &SM83::op_sub_a_r<Operand::H, WithoutCarry>,
            &SM83::op_sub_a_r<Operand::L, WithoutCarry>,
            &SM83::op_sub_a_r<Operand::HLAddress, WithoutCarry>,
            &SM83::op_sub_a_r<Operand::A, WithoutCarry>,
            &SM83::op_sub_a_r<Operand::B, WithCarry>,
            &SM83::op_sub_a_r<Operand::C, WithCarry>,
            &SM83::op_sub_a_r<Operand::D, WithCarry>,
            &SM83::op_sub_a_r<Operand::E, WithCarry>,
            &SM83::op_sub_a_r<Operand::H, WithCarry>,
            &SM83::op_sub_a_r<Operand::L, WithCarry>,
            &SM83::op_sub_a_r<Operand::HLAddress, WithCarry>,
            &SM83::op_sub_a_r<Operand::A, WithCarry>,

            // 0xA0 - 0xAF
            &SM83::op_and_a_r<Operand::B>,
            &SM83::op_and_a_r<Operand::C>,
            &SM83::op_and_a_r<Operand::D>,
            &SM83::op_and_a_r<Operand::E>,
            &SM83::op_and_a_r<Operand::H>,
            &SM83::op_and_a_r<Operand::L>,
            &SM83::op_and_a_r<Operand::HLAddress>,
            &SM83::op_and_a_r<Operand::A>,
            &SM83::op_xor_a_r<Operand::B>,
            &SM83::op_xor_a_r<Operand::C>,
            &SM83::op_xor_a_r<Operand::D>,
            &SM83::op_xor_a_r<Operand::E>,
            &SM83::op_xor_a_r<Operand::H>,
            &SM83::op_xor_a_r<Operand::L>,
            &SM83::op_xor_a_r<Operand::HLAddress>,
            &SM83::op_xor_a_r<Operand::A>,

            // 0xB0 - 0xBF
            &SM83::op_or_a_r<Operand::B>,
            &SM83::op_or_a_r<Operand::C>,
            &SM83::op_or_a_r<Operand::D>,
            &SM83::op_or_a_r<Operand::E>,
            &SM83::op_or_a_r<Operand::H>,
            &SM83::op_or_a_r<Operand::L>,
            &SM83::op_or_a_r<Operand::HLAddress>,
            &SM83::op_or_a_r<Operand::A>,
            &SM83::op_cp_a_r<Operand::B>,
            &SM83::op_cp_a_r<Operand::C>,
            &SM83::op_cp_a_r<Operand::D>,
            &SM83::op_cp_a_r<Operand::E>,
            &SM83::op_cp_a_r<Operand::H>,
            &SM83::op_cp_a_r<Operand::L>,
            &SM83::op_cp_a_r<Operand::HLAddress>,
            &SM83::op_cp_a_r<Operand::A>,

            // 0xC0 - 0xCF
            &SM83::op_ret_cc<FLAG_Z, false>,
            &SM83::op_pop_rp<RegisterPair::BC>,
            &SM83::op_jp_cc_u16<FLAG_Z, false>,
            &SM83::op_jp_u16,
            &SM83::op_call_cc_u16<FLAG_Z, false>,
            &SM83::op_push_rp<RegisterPair::BC>,
            &SM83::op_add_a_r<Operand::Immediate, WithoutCarry>,
            &SM83::op_rst_n<0x00>,
            &SM83::op_ret_cc<FLAG_Z, true>,
            &SM83::op_ret<IgnoreIME>,
            &SM83::op_jp_cc_u16<FLAG_Z, true>,
            &SM83::op_cb,
            &SM83::op_call_cc_u16<FLAG_Z, true>,
            &SM83::op_call_u16,
            &SM83::op_add_a_r<Operand::Immediate, WithCarry>,
            &SM83::op_rst_n<0x08>,

            // 0xD0 - 0xDF
            &SM83::op_ret_cc<FLAG_CY, false>,
            &SM83::op_pop_rp<RegisterPair::DE>,
            &SM83::op_jp_cc_u16<FLAG_CY, false>,
            &SM83::op_nop<true, 0xD3>,
            &SM83::op_call_cc_u16<FLAG_CY, false>,
            &SM83::op_push_rp<RegisterPair::DE>,
            &SM83::op_sub_a_r<Operand::Immediate, WithoutCarry>,
            &SM83::op_rst_n<0x10>,
            &SM83::op_ret_cc<FLAG_CY, true>,
            &SM83::op_ret<SetIME>,
            &SM83::op_jp_cc_u16<FLAG_CY, true>,
            &SM83::op_nop<true, 0xDB>,
            &SM83::op_call_cc_u16<FLAG_CY, true>,
            &SM83::op_nop<true, 0xDD>,
            &SM83::op_sub_a_r<Operand::Immediate, WithCarry>,
            &SM83::op_rst_n<0x18>,

            // 0xE0 - 0xEF
            &SM83::op_ld_ff00_u8_a,
            &SM83::op_pop_rp<RegisterPair::HL>,
            &SM83::op_ld_ff00_c_a,
            &SM83::op_nop<true, 0xE3>,
            &SM83::op_nop<true, 0xE4>,
            &SM83::op_push_rp<RegisterPair::HL>,
            &SM83::op_and_a_r<Operand::Immediate>,
            &SM83::op_rst_n<0x20>,
            &SM83::op_add_sp_i8,
            &SM83::op_jp_hl,
            &SM83::op_ld_u16_a,
            &SM83::op_nop<true, 0xEB>,
            &SM83::op_nop<true, 0xEC>,
            &SM83::op_nop<true, 0xED>,
            &SM83::op_xor_a_r<Operand::Immediate>,
            &SM83::op_rst_n<0x28>,

            // 0xF0 - 0xFF
            &SM83::op_ld_a_ff00_u8,
            &SM83::op_pop_rp<RegisterPair::AF>,
            &SM83::op_ld_a_ff00_c,
            &SM83::op_di,
            &SM83::op_nop<true, 0xF4>,
            &SM83::op_push_rp<RegisterPair::AF>,
            &SM83::op_or_a_r<Operand::Immediate>,
            &SM83::op_rst_n<0x30>,
            &SM83::op_ld_hl_sp_i8,
            &SM83::op_ld_sp_hl,
            &SM83::op_ld_a_u16,
            &SM83::op_ei,
            &SM83::op_nop<true, 0xFC>,
            &SM83::op_nop<true, 0xFD>,
            &SM83::op_cp_a_r<Operand::Immediate>,
            &SM83::op_rst_n<0x38>,
        };
    }

    void SM83::gen_bitwise_opcodes() {
        bitwise_opcodes = {
            // 0x00 - 0x0F
            &SM83::op_rlc<Operand::B>,
            &SM83::op_rlc<Operand::C>,
            &SM83::op_rlc<Operand::D>,
            &SM83::op_rlc<Operand::E>,
            &SM83::op_rlc<Operand::H>,
            &SM83::op_rlc<Operand::L>,
            &SM83::op_rlc<Operand::HLAddress>,
            &SM83::op_rlc<Operand::A>,

            &SM83::op_rrc<Operand::B>,
            &SM83::op_rrc<Operand::C>,
            &SM83::op_rrc<Operand::D>,
            &SM83::op_rrc<Operand::E>,
            &SM83::op_rrc<Operand::H>,
            &SM83::op_rrc<Operand::L>,
            &SM83::op_rrc<Operand::HLAddress>,
            &SM83::op_rrc<Operand::A>,

            // 0x10 - 0x1F
            &SM83::op_rl<Operand::B>,
            &SM83::op_rl<Operand::C>,
            &SM83::op_rl<Operand::D>,
            &SM83::op_rl<Operand::E>,
            &SM83::op_rl<Operand::H>,
            &SM83::op_rl<Operand::L>,
            &SM83::op_rl<Operand::HLAddress>,
            &SM83::op_rl<Operand::A>,

            &SM83::op_rr<Operand::B>,
            &SM83::op_rr<Operand::C>,
            &SM83::op_rr<Operand::D>,
            &SM83::op_rr<Operand::E>,
            &SM83::op_rr<Operand::H>,
            &SM83::op_rr<Operand::L>,
            &SM83::op_rr<Operand::HLAddress>,
            &SM83::op_rr<Operand::A>,

            // 0x20 - 0x2F
            &SM83::op_sla<Operand::B>,
            &SM83::op_sla<Operand::C>,
            &SM83::op_sla<Operand::D>,
            &SM83::op_sla<Operand::E>,
            &SM83::op_sla<Operand::H>,
            &SM83::op_sla<Operand::L>,
            &SM83::op_sla<Operand::HLAddress>,
            &SM83::op_sla<Operand::A>,

            &SM83::op_sra<Operand::B>,
            &SM83::op_sra<Operand::C>,
            &SM83::op_sra<Operand::D>,
            &SM83::op_sra<Operand::E>,
            &SM83::op_sra<Operand::H>,
            &SM83::op_sra<Operand::L>,
            &SM83::op_sra<Operand::HLAddress>,
            &SM83::op_sra<Operand::A>,

            // 0x30 - 0x3F
            &SM83::op_swap<Operand::B>,
            &SM83::op_swap<Operand::C>,
            &SM83::op_swap<Operand::D>,
            &SM83::op_swap<Operand::E>,
            &SM83::op_swap<Operand::H>,
            &SM83::op_swap<Operand::L>,
            &SM83::op_swap<Operand::HLAddress>,
            &SM83::op_swap<Operand::A>,

            &SM83::op_srl<Operand::B>,
            &SM83::op_srl<Operand::C>,
            &SM83::op_srl<Operand::D>,
            &SM83::op_srl<Operand::E>,
            &SM83::op_srl<Operand::H>,
            &SM83::op_srl<Operand::L>,
            &SM83::op_srl<Operand::HLAddress>,
            &SM83::op_srl<Operand::A>,

            // 0x40 - 0x4F
            &SM83::op_bit<0, Operand::B>,
            &SM83::op_bit<0, Operand::C>,
            &SM83::op_bit<0, Operand::D>,
            &SM83::op_bit<0, Operand::E>,
            &SM83::op_bit<0, Operand::H>,
            &SM83::op_bit<0, Operand::L>,
            &SM83::op_bit<0, Operand::HLAddress>,
            &SM83::op_bit<0, Operand::A>,

            &SM83::op_bit<1, Operand::B>,
            &SM83::op_bit<1, Operand::C>,
            &SM83::op_bit<1, Operand::D>,
            &SM83::op_bit<1, Operand::E>,
            &SM83::op_bit<1, Operand::H>,
            &SM83::op_bit<1, Operand::L>,
            &SM83::op_bit<1, Operand::HLAddress>,
            &SM83::op_bit<1, Operand::A>,

            // 0x50 - 0x5F
            &SM83::op_bit<2, Operand::B>,
            &SM83::op_bit<2, Operand::C>,
            &SM83::op_bit<2, Operand::D>,
            &SM83::op_bit<2, Operand::E>,
            &SM83::op_bit<2, Operand::H>,
            &SM83::op_bit<2, Operand::L>,
            &SM83::op_bit<2, Operand::HLAddress>,
            &SM83::op_bit<2, Operand::A>,

            &SM83::op_bit<3, Operand::B>,
            &SM83::op_bit<3, Operand::C>,
            &SM83::op_bit<3, Operand::D>,
            &SM83::op_bit<3, Operand::E>,
            &SM83::op_bit<3, Operand::H>,
            &SM83::op_bit<3, Operand::L>,
            &SM83::op_bit<3, Operand::HLAddress>,
            &SM83::op_bit<3, Operand::A>,

            // 0x60 - 0x6F
            &SM83::op_bit<4, Operand::B>,
            &SM83::op_bit<4, Operand::C>,
            &SM83::op_bit<4, Operand::D>,
            &SM83::op_bit<4, Operand::E>,
            &SM83::op_bit<4, Operand::H>,
            &SM83::op_bit<4, Operand::L>,
            &SM83::op_bit<4, Operand::HLAddress>,
            &SM83::op_bit<4, Operand::A>,

            &SM83::op_bit<5, Operand::B>,
            &SM83::op_bit<5, Operand::C>,
            &SM83::op_bit<5, Operand::D>,
            &SM83::op_bit<5, Operand::E>,
            &SM83::op_bit<5, Operand::H>,
            &SM83::op_bit<5, Operand::L>,
            &SM83::op_bit<5, Operand::HLAddress>,
            &SM83::op_bit<5, Operand::A>,

            // 0x70 - 0x7F
            &SM83::op_bit<6, Operand::B>,
            &SM83::op_bit<6, Operand::C>,
            &SM83::op_bit<6, Operand::D>,
            &SM83::op_bit<6, Operand::E>,
            &SM83::op_bit<6, Operand::H>,
            &SM83::op_bit<6, Operand::L>,
            &SM83::op_bit<6, Operand::HLAddress>,
            &SM83::op_bit<6, Operand::A>,

            &SM83::op_bit<7, Operand::B>,
            &SM83::op_bit<7, Operand::C>,
            &SM83::op_bit<7, Operand::D>,
            &SM83::op_bit<7, Operand::E>,
            &SM83::op_bit<7, Operand::H>,
            &SM83::op_bit<7, Operand::L>,
            &SM83::op_bit<7, Operand::HLAddress>,
            &SM83::op_bit<7, Operand::A>,

            // 0x80 - 0x8F
            &SM83::op_res<0, Operand::B>,
            &SM83::op_res<0, Operand::C>,
            &SM83::op_res<0, Operand::D>,
            &SM83::op_res<0, Operand::E>,
            &SM83::op_res<0, Operand::H>,
            &SM83::op_res<0, Operand::L>,
            &SM83::op_res<0, Operand::HLAddress>,
            &SM83::op_res<0, Operand::A>,

            &SM83::op_res<1, Operand::B>,
            &SM83::op_res<1, Operand::C>,
            &SM83::op_res<1, Operand::D>,
            &SM83::op_res<1, Operand::E>,
            &SM83::op_res<1, Operand::H>,
            &SM83::op_res<1, Operand::L>,
            &SM83::op_res<1, Operand::HLAddress>,
            &SM83::op_res<1, Operand::A>,

            // 0x90 - 0x9F
            &SM83::op_res<2, Operand::B>,
            &SM83::op_res<2, Operand::C>,
            &SM83::op_res<2, Operand::D>,
            &SM83::op_res<2, Operand::E>,
            &SM83::op_res<2, Operand::H>,
            &SM83::op_res<2, Operand::L>,
            &SM83::op_res<2, Operand::HLAddress>,
            &SM83::op_res<2, Operand::A>,

            &SM83::op_res<3, Operand::B>,
            &SM83::op_res<3, Operand::C>,
            &SM83::op_res<3, Operand::D>,
            &SM83::op_res<3, Operand::E>,
            &SM83::op_res<3, Operand::H>,
            &SM83::op_res<3, Operand::L>,
            &SM83::op_res<3, Operand::HLAddress>,
            &SM83::op_res<3, Operand::A>,

            // 0xA0 - 0xAF
            &SM83::op_res<4, Operand::B>,
            &SM83::op_res<4, Operand::C>,
            &SM83::op_res<4, Operand::D>,
            &SM83::op_res<4, Operand::E>,
            &SM83::op_res<4, Operand::H>,
            &SM83::op_res<4, Operand::L>,
            &SM83::op_res<4, Operand::HLAddress>,
            &SM83::op_res<4, Operand::A>,

            &SM83::op_res<5, Operand::B>,
            &SM83::op_res<5, Operand::C>,
            &SM83::op_res<5, Operand::D>,
            &SM83::op_res<5, Operand::E>,
            &SM83::op_res<5, Operand::H>,
            &SM83::op_res<5, Operand::L>,
            &SM83::op_res<5, Operand::HLAddress>,
            &SM83::op_res<5, Operand::A>,

            // 0xB0 - 0xBF
            &SM83::op_res<6, Operand::B>,
            &SM83::op_res<6, Operand::C>,
            &SM83::op_res<6, Operand::D>,
            &SM83::op_res<6, Operand::E>,
            &SM83::op_res<6, Operand::H>,
            &SM83::op_res<6, Operand::L>,
            &SM83::op_res<6, Operand::HLAddress>,
            &SM83::op_res<6, Operand::A>,

            &SM83::op_res<7, Operand::B>,
            &SM83::op_res<7, Operand::C>,
            &SM83::op_res<7, Operand::D>,
            &SM83::op_res<7, Operand::E>,
            &SM83::op_res<7, Operand::H>,
            &SM83::op_res<7, Operand::L>,
            &SM83::op_res<7, Operand::HLAddress>,
            &SM83::op_res<7, Operand::A>,

            // 0xC0 - 0xCF
            &SM83::op_set<0, Operand::B>,
            &SM83::op_set<0, Operand::C>,
            &SM83::op_set<0, Operand::D>,
            &SM83::op_set<0, Operand::E>,
            &SM83::op_set<0, Operand::H>,
            &SM83::op_set<0, Operand::L>,
            &SM83::op_set<0, Operand::HLAddress>,
            &SM83::op_set<0, Operand::A>,

            &SM83::op_set<1, Operand::B>,
            &SM83::op_set<1, Operand::C>,
            &SM83::op_set<1, Operand::D>,
            &SM83::op_set<1, Operand::E>,
            &SM83::op_set<1, Operand::H>,
            &SM83::op_set<1, Operand::L>,
            &SM83::op_set<1, Operand::HLAddress>,
            &SM83::op_set<1, Operand::A>,

            // 0xD0 - 0xDF
            &SM83::op_set<2, Operand::B>,
            &SM83::op_set<2, Operand::C>,
            &SM83::op_set<2, Operand::D>,
            &SM83::op_set<2, Operand::E>,
            &SM83::op_set<2, Operand::H>,
            &SM83::op_set<2, Operand::L>,
            &SM83::op_set<2, Operand::HLAddress>,
            &SM83::op_set<2, Operand::A>,

            &SM83::op_set<3, Operand::B>,
            &SM83::op_set<3, Operand::C>,
            &SM83::op_set<3, Operand::D>,
            &SM83::op_set<3, Operand::E>,
            &SM83::op_set<3, Operand::H>,
            &SM83::op_set<3, Operand::L>,
            &SM83::op_set<3, Operand::HLAddress>,
            &SM83::op_set<3, Operand::A>,

            // 0xE0 - 0xEF
            &SM83::op_set<4, Operand::B>,
            &SM83::op_set<4, Operand::C>,
            &SM83::op_set<4, Operand::D>,
            &SM83::op_set<4, Operand::E>,
            &SM83::op_set<4, Operand::H>,
            &SM83::op_set<4, Operand::L>,
            &SM83::op_set<4, Operand::HLAddress>,
            &SM83::op_set<4, Operand::A>,

            &SM83::op_set<5, Operand::B>,
            &SM83::op_set<5, Operand::C>,
            &SM83::op_set<5, Operand::D>,
            &SM83::op_set<5, Operand::E>,
            &SM83::op_set<5, Operand::H>,
            &SM83::op_set<5, Operand::L>,
            &SM83::op_set<5, Operand::HLAddress>,
            &SM83::op_set<5, Operand::A>,

            // 0xF0 - 0xFF
            &SM83::op_set<6, Operand::B>,
            &SM83::op_set<6, Operand::C>,
            &SM83::op_set<6, Operand::D>,
            &SM83::op_set<6, Operand::E>,
            &SM83::op_set<6, Operand::H>,
            &SM83::op_set<6, Operand::L>,
            &SM83::op_set<6, Operand::HLAddress>,
            &SM83::op_set<6, Operand::A>,

            &SM83::op_set<7, Operand::B>,
            &SM83::op_set<7, Operand::C>,
            &SM83::op_set<7, Operand::D>,
            &SM83::op_set<7, Operand::E>,
            &SM83::op_set<7, Operand::H>,
            &SM83::op_set<7, Operand::L>,
            &SM83::op_set<7, Operand::HLAddress>,
            &SM83::op_set<7, Operand::A>,
        };
    }

}
