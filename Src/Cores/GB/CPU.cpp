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

#include "CPU.hpp"
#include "Constants.hpp"

namespace GB {
    CPU::CPU(std::function<void(uint16_t, uint8_t)> write_fn,
             std::function<uint8_t(uint16_t)> read_fn)
        : write(std::move(write_fn)), read(std::move(read_fn)) {}

    auto CPU::force_next_opcode(const uint8_t opcode) -> void { ir = opcode; }

    auto CPU::flag_state() const -> uint8_t {
        uint8_t out_flags = flags.cy << 4;
        out_flags |= flags.hc << 5;
        out_flags |= flags.n << 6;
        out_flags |= flags.z << 7;
        return out_flags;
    }

    auto CPU::set_flags(uint8_t in_flags) -> void {
        in_flags = in_flags & 0xF0;
        flags.cy = (in_flags >> 4) & 0x1;
        flags.hc = (in_flags >> 5) & 0x1;
        flags.n = (in_flags >> 6) & 0x1;
        flags.z = (in_flags >> 7) & 0x1;
    }

    auto CPU::double_speed() const -> bool { return double_speed_; }

    auto CPU::get_rp(const RegisterPair index) const -> uint16_t {
        switch (index) {
        case RegisterPair::BC: return (b << 8) | c;
        case RegisterPair::DE: return (d << 8) | e;
        case RegisterPair::HL: return (h << 8) | l;
        case RegisterPair::SP: return sp;
        case RegisterPair::AF: return (a << 8) | flag_state();
        }
        return 0;
    }

    auto CPU::set_rp(const RegisterPair index, const uint16_t value) -> void {
        switch (index) {
        case RegisterPair::BC: {
            b = static_cast<uint8_t>((value & 0xFF00) >> 8);
            c = static_cast<uint8_t>(value & 0x00FF);
            return;
        }
        case RegisterPair::DE: {
            d = static_cast<uint8_t>((value & 0xFF00) >> 8);
            e = static_cast<uint8_t>(value & 0x00FF);
            return;
        }
        case RegisterPair::HL: {
            h = static_cast<uint8_t>((value & 0xFF00) >> 8);
            l = static_cast<uint8_t>(value & 0x00FF);
            return;
        }
        case RegisterPair::SP: {
            sp = value;
            return;
        }
        case RegisterPair::AF: {
            a = static_cast<uint8_t>((value & 0xFF00) >> 8);
            set_flags(static_cast<uint8_t>(value & 0x00FF));
            return;
        }
        }
    }

    auto CPU::reset(const uint16_t new_pc, const bool with_dmg_values) -> void {
        dmg_mode = with_dmg_values;
        ime = false;
        double_speed_ = false;
        exec = ExecutionMode::NormalBank;
        ie = 0;
        if_ = 0;

        if (dmg_mode) {
            flags.cy = true;
            flags.hc = true;
            flags.n = false;
            flags.z = true;

            a = 0x01;
            b = 0x00;
            c = 0x13;
            d = 0x00;
            e = 0xD8;
            h = 0x01;
            l = 0x4D;

            pc = new_pc;
            sp = 0xFFFE;
        } else {
            flags.cy = false;
            flags.hc = false;
            flags.n = false;
            flags.z = true;

            a = 0x11;
            b = 0x00;
            c = 0x00;
            d = 0xFF;
            e = 0x56;
            h = 0x00;
            l = 0x0D;

            pc = new_pc;
            sp = 0xFFFE;
        }
    }

    auto CPU::request_interrupt(const uint8_t interrupt) -> void { if_ |= interrupt; }

    auto CPU::clock() -> void {
        // TODO: Probably need a state for HALT and STOP
        switch (exec) {
        case ExecutionMode::NormalBank: decode_execute(); return;
        case ExecutionMode::BitOpsBank: decode_execute_bitops(); return;
        case ExecutionMode::Interrupt: isr(); return;
        case ExecutionMode::Halted: {
            m_cycle = 1;

            if (if_ & ie) {
                if (!ime) {
                    exec = ExecutionMode::NormalBank;
                } else {
                    exec = ExecutionMode::Interrupt;
                }

                ime = false;
                ir = read(pc++);
            } else {
                ir = read(pc);
            }
            return;
        }
        }
    }

    auto CPU::get_register(const Operand reg) const -> uint8_t {
        switch (reg) {
        case Operand::B: return b;
        case Operand::C: return c;
        case Operand::D: return d;
        case Operand::E: return e;
        case Operand::H: return h;
        case Operand::L: return l;
        case Operand::Memory: return z;
        case Operand::A: return a;
        default:;
        }
        return 0;
    }

    auto CPU::set_register(const Operand reg, const uint8_t value) -> void {
        switch (reg) {
        case Operand::B: b = value; return;
        case Operand::C: c = value; return;
        case Operand::D: d = value; return;
        case Operand::E: e = value; return;
        case Operand::H: h = value; return;
        case Operand::L: l = value; return;
        case Operand::Memory: z = value; return;
        case Operand::A: a = value; return;
        default:;
        }
    }

    auto CPU::fetch(const uint16_t where) -> void {
        exec = ExecutionMode::NormalBank;
        ir = read(where);
        pc = where + 1;
        m_cycle = 1;

        const uint8_t interrupt_pending = if_ & ie;
        if (interrupt_pending && ime) {
            exec = ExecutionMode::Interrupt;
            ime = false;
        }
    }

    auto CPU::isr() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            pc--;
            break;
        }
        case 3: {
            sp--;
            break;
        }
        case 4: {
            write(sp--, pc >> 8);
            break;
        }
        case 5: {
            write(sp, pc & 0xFF);

            const uint8_t interrupt_pending = if_ & ie;
            if (interrupt_pending & INT_VBLANK_BIT) {
                pc = 0x40;
                if_ &= ~INT_VBLANK_BIT;
            } else if (interrupt_pending & INT_LCD_STAT_BIT) {
                pc = 0x48;
                if_ &= ~INT_LCD_STAT_BIT;
            } else if (interrupt_pending & INT_TIMER_BIT) {
                pc = 0x50;
                if_ &= ~INT_TIMER_BIT;
            } else if (interrupt_pending & INT_SERIAL_PORT_BIT) {
                pc = 0x58;
                if_ &= ~INT_SERIAL_PORT_BIT;
            } else if (interrupt_pending & INT_JOYPAD_BIT) {
                pc = 0x60;
                if_ &= ~INT_JOYPAD_BIT;
            }

            break;
        }
        case 6: {
            fetch(pc);
            break;
        }
        default:;
        }
    }

    auto CPU::nop() -> void {
        m_cycle++;
        fetch(pc);
    }

    auto CPU::stop() -> void {
        m_cycle++;
        if (dmg_mode) {
            return;
        }

        if (KEY1 & 0x1) {
            double_speed_ = !double_speed_;
            KEY1 = double_speed_ << 7;
        }
        fetch(pc + 1);
    }

    auto CPU::halt() -> void { exec = ExecutionMode::Halted; }

    auto CPU::illegal_instruction() -> void {
        m_cycle = 2;
        // never fetches
    }

    auto CPU::ei() -> void {
        m_cycle++;
        fetch(pc);
        ime = true;
    }

    auto CPU::di() -> void {
        exec = ExecutionMode::NormalBank;
        ir = read(pc++);
        m_cycle = 1;
        ime = false;
    }

    auto CPU::bitops_bank_switch() -> void {
        exec = ExecutionMode::BitOpsBank;
        ir = read(pc++);
        m_cycle = 1;
    }

    template <CPU::Operand dst, CPU::Operand src> auto CPU::ld() -> void {
        m_cycle++;

        // ld (hl), reg
        if constexpr (dst == Operand::Memory) {
            switch (m_cycle) {
            case 2: {
                write(get_rp(RegisterPair::HL), get_register(src));
                break;
            }
            case 3: {
                fetch(pc);
                break;
            }
            default:;
            }
            return;
        }

        // ld reg, (hl)
        if constexpr (src == Operand::Memory) {
            switch (m_cycle) {
            case 2: {
                z = read(get_rp(RegisterPair::HL));
                break;
            }
            case 3: {
                set_register(dst, z);
                fetch(pc);
                break;
            }
            default:;
            }
            return;
        }

        // ld reg, reg
        const auto src_value = get_register(src);
        set_register(dst, src_value);
        fetch(pc);
    }

    auto CPU::op_ld_r_indirect(const uint16_t address) -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = read(address);
            break;
        }
        case 3: {
            set_register(static_cast<Operand>((ir >> 3) & 0x7), z);
            fetch(pc);
            break;
        }
        default:;
        }
    }

    template <CPU::Operand reg> auto CPU::ld_immediate() -> void {
        m_cycle++;

        if constexpr (reg == Operand::Memory) {
            switch (m_cycle) {
            case 2: {
                z = read(pc++);
                break;
            }
            case 3: {
                write(get_rp(RegisterPair::HL), z);
                break;
            }
            case 4: {
                fetch(pc);
                break;
            }
            default:;
            }
            return;
        }

        switch (m_cycle) {
        case 2: {
            z = read(pc++);
            break;
        }
        case 3: {
            set_register(reg, z);
            fetch(pc);
            break;
        }
        default:;
        }
    }

    auto CPU::op_ld_indirect_r(const uint16_t address) -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            const auto src_value = get_register(static_cast<Operand>(ir & 0x7));
            write(address, src_value);
            break;
        }
        case 3: {
            fetch(pc);
            break;
        }
        default:;
        }
    }

    template <CPU::Operand2 rp> auto CPU::ld_indirect_accumulator() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            uint16_t address = 0;
            switch (rp) {
            case Operand2::BC: {
                address = get_rp(RegisterPair::BC);
                break;
            }
            case Operand2::DE: {
                address = get_rp(RegisterPair::DE);
                break;
            }
            case Operand2::HLIncrement: {
                address = get_rp(RegisterPair::HL);
                set_rp(RegisterPair::HL, address + 1);
                break;
            }
            case Operand2::HLDecrement: {
                address = get_rp(RegisterPair::HL);
                set_rp(RegisterPair::HL, address - 1);
                break;
            }
            }

            write(address, a);
            break;
        }
        case 3: {
            fetch(pc);
            break;
        }
        default:;
        }
    }

    template <CPU::Operand2 rp> auto CPU::ld_accumulator_indirect() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            uint16_t address = 0;
            switch (rp) {
            case Operand2::BC: {
                address = get_rp(RegisterPair::BC);
                break;
            }
            case Operand2::DE: {
                address = get_rp(RegisterPair::DE);
                break;
            }
            case Operand2::HLIncrement: {
                address = get_rp(RegisterPair::HL);
                set_rp(RegisterPair::HL, address + 1);
                break;
            }
            case Operand2::HLDecrement: {
                address = get_rp(RegisterPair::HL);
                set_rp(RegisterPair::HL, address - 1);
                break;
            }
            }
            a = read(address);
            break;
        }
        case 3: {
            fetch(pc);
            break;
        }
        default:;
        }
    }

    auto CPU::ld_direct_a() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = read(pc++);
            break;
        }
        case 3: {
            w = read(pc++);
            break;
        }
        case 4: {
            write(z | (w << 8), a);
            break;
        }
        case 5: {
            fetch(pc);
            break;
        }
        default:;
        }
    }

    auto CPU::ld_a_direct() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = read(pc++);
            break;
        }
        case 3: {
            w = read(pc++);
            break;
        }
        case 4: {
            a = read(z | (w << 8));
            break;
        }
        case 5: {
            fetch(pc);
            break;
        }
        default:;
        }
    }

    auto CPU::op_ld_indirect_immediate(const uint16_t address) -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = read(pc++);
            break;
        }
        case 3: {
            write(address, z);
            break;
        }
        case 4: {
            fetch(pc);
            break;
        }
        default:;
        }
    }

    template <RegisterPair rp> auto CPU::ld_rp_immediate() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = read(pc++);
            break;
        }
        case 3: {
            w = read(pc++);
            break;
        }
        case 4: {
            set_rp(rp, z | (w << 8));
            fetch(pc);
            break;
        }
        default:;
        }
    }

    auto CPU::ld_direct_sp() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = read(pc++);
            break;
        }
        case 3: {
            w = read(pc++);
            break;
        }
        case 4: {
            const auto address = z | (w << 8);
            write(address, sp & 0xFF);
            z++;
            break;
        }
        case 5: {
            const auto address = z | (w << 8);
            write(address, (sp >> 8) & 0xFF);
            break;
        }
        case 6: {
            fetch(pc);
            break;
        }
        default:;
        }
    }

    auto CPU::ld_hl_sp_i8() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = read(pc++);
            break;
        }
        case 3: {
            w = (z & 0x80) ? 255 : 0;

            const int32_t spl = (sp & 0xFF);
            const int32_t result = z + spl;

            flags.cy = result > 0xFF;
            flags.hc = ((z & 0xF) + (spl & 0xF)) > 0xF;
            flags.n = false;
            flags.z = false;
            z = static_cast<uint8_t>(result & 0xFF);

            break;
        }
        case 4: {
            w += static_cast<uint8_t>(sp >> 8) + static_cast<uint8_t>(flags.cy);
            set_rp(RegisterPair::HL, (w << 8) | z);
            fetch(pc);
            break;
        }
        default:;
        }
    }

    auto CPU::ld_sp_hl() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            sp = get_rp(RegisterPair::HL);
            break;
        }
        case 3: {
            fetch(pc);
            break;
        }
        default:;
        }
    }

    auto CPU::ldh_offset_a() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = read(pc++);
            break;
        }
        case 3: {
            write(0xFF00 + z, a);
            break;
        }
        case 4: {
            fetch(pc);
            break;
        }
        default:;
        }
    }

    auto CPU::ldh_a_offset() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = read(pc++);
            break;
        }
        case 3: {
            a = read(0xFF00 + z);
            break;
        }
        case 4: {
            fetch(pc);
            break;
        }
        default:;
        }
    }

    auto CPU::ldh_c_a() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            write(0xFF00 + c, a);
            break;
        }
        case 3: {
            fetch(pc);
            break;
        }
        default:;
        }
    }

    auto CPU::ldh_a_c() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            a = read(0xFF00 + c);
            break;
        }
        case 3: {
            fetch(pc);
            break;
        }
        default:;
        }
    }

    auto CPU::add_sp_i8() -> void {
        m_cycle++;

        switch (m_cycle) {
        case 2: {
            z = read(pc++);
            break;
        }
        case 3: {
            w = (z & 0x80) ? 255 : 0;

            const int32_t spl = (sp & 0xFF);
            const int32_t result = z + spl;

            flags.cy = result > 0xFF;
            flags.hc = ((z & 0xF) + (spl & 0xF)) > 0xF;
            flags.n = false;
            flags.z = false;
            z = static_cast<uint8_t>(result & 0xFF);
            break;
        }
        case 4: {
            w += static_cast<uint8_t>(sp >> 8) + static_cast<uint8_t>(flags.cy);
            break;
        }
        case 5: {
            sp = (w << 8) | z;
            fetch(pc);
            break;
        }
        default:;
        }
    }

    template <RegisterPair src> auto CPU::add_hl_rp() -> void {
        m_cycle++;
        auto add_8bit = [this](const int32_t left, const int32_t right,
                               const int32_t cy) -> uint8_t {
            const int32_t result = left + right + cy;
            const auto masked_result = static_cast<uint8_t>(result & 0xFF);

            flags.cy = result > 0xFF;
            flags.hc = ((left & 0xF) + (right & 0xF) + (cy & 0xF)) > 0xF;
            flags.n = false;
            return masked_result;
        };

        switch (m_cycle) {
        case 2: {
            const auto rp = get_rp(src);
            l = add_8bit(l, rp & 0xFF, 0);
            break;
        }
        case 3: {
            const auto rp = get_rp(src);
            h = add_8bit(h, rp >> 8, flags.cy);
            fetch(pc);
            break;
        }
        default:;
        }
    }

    template <CPU::Operand operand, bool with_carry> auto CPU::adc() -> void {
        const int32_t left = a;
        const int32_t right = (operand == Operand::Memory) ? z : get_register(operand);

        const int32_t cy = with_carry ? flags.cy : 0;
        const int32_t result = left + right + cy;
        const auto masked_result = static_cast<uint8_t>(result & 0xFF);

        flags.cy = result > 0xFF;
        flags.hc = ((left & 0xF) + (right & 0xF) + (cy & 0xF)) > 0xF;
        flags.n = false;
        flags.z = masked_result == 0;

        a = masked_result;
    }

    template <CPU::Operand operand, bool with_carry> auto CPU::sbc() -> void {
        const int32_t left = a;
        const int32_t right = operand == Operand::Memory ? z : get_register(operand);

        const int32_t cy = with_carry ? flags.cy : 0;
        const int32_t result = left - right - cy;
        const auto masked_result = static_cast<uint8_t>(result & 0xFF);

        flags.cy = result < 0;
        flags.hc = ((left & 0xF) - (right & 0xF) - (cy & 0xF)) < 0;
        flags.n = true;
        flags.z = masked_result == 0;

        a = masked_result;
    }

    template <CPU::Operand operand> auto CPU::and_op() -> void {
        const int32_t left = a;
        const int32_t right = operand == Operand::Memory ? z : get_register(operand);
        const int32_t result = left & right;

        flags.cy = false;
        flags.hc = true;
        flags.n = false;
        flags.z = result == 0;

        a = static_cast<uint8_t>(result & 0xFF);
    }

    template <CPU::Operand operand> auto CPU::xor_op() -> void {
        const int32_t left = a;
        const int32_t right = operand == Operand::Memory ? z : get_register(operand);
        const int32_t result = left ^ right;

        flags.cy = false;
        flags.hc = false;
        flags.n = false;
        flags.z = result == 0;

        a = static_cast<uint8_t>(result & 0xFF);
    }

    template <CPU::Operand operand> auto CPU::or_op() -> void {
        const int32_t left = a;
        const int32_t right = operand == Operand::Memory ? z : get_register(operand);
        const int32_t result = left | right;

        flags.cy = false;
        flags.hc = false;
        flags.n = false;
        flags.z = result == 0;

        a = static_cast<uint8_t>(result & 0xFF);
    }

    template <CPU::Operand operand> auto CPU::cp() -> void {
        const uint8_t temp = a;
        sbc<operand, false>();
        a = temp;
    }

    template <CPU::Operand operand> auto CPU::inc_r() -> void {
        const int32_t left = (operand == Operand::Memory) ? z : get_register(operand);
        constexpr int32_t right = 1;

        const int32_t result = left + right;
        const auto masked_result = static_cast<uint8_t>(result & 0xFF);

        flags.hc = ((left & 0xF) + (right & 0xF)) > 0xF;
        flags.n = false;
        flags.z = masked_result == 0;

        set_register(operand, masked_result);
    }

    template <CPU::Operand operand> auto CPU::dec_r() -> void {
        const int32_t left = (operand == Operand::Memory) ? z : get_register(operand);
        constexpr int32_t right = 1;

        const int32_t result = left - right;
        const auto masked_result = static_cast<uint8_t>(result & 0xFF);

        flags.hc = ((left & 0xF) - (right & 0xF)) < 0;
        flags.n = true;
        flags.z = masked_result == 0;

        set_register(operand, masked_result);
    }

    template <RegisterPair rp, int32_t adjustment> auto CPU::adjust_rp() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            set_rp(rp, get_rp(rp) + adjustment);
            break;
        }
        case 3: {
            fetch(pc);
            break;
        }
        default:;
        }
    }

    auto CPU::rlca() -> void {
        m_cycle++;
        int32_t temp = a;
        const int32_t bit7 = (temp & 0x80) >> 7;

        flags.cy = static_cast<bool>(bit7);
        flags.hc = false;
        flags.n = false;
        flags.z = false;

        temp = (temp << 1) | bit7;
        a = static_cast<uint8_t>(temp & 0xFF);
        fetch(pc);
    }

    auto CPU::rrca() -> void {
        m_cycle++;
        int32_t temp = a;
        const int32_t bit0 = (temp & 0x01);

        flags.cy = static_cast<bool>(bit0);
        flags.hc = false;
        flags.n = false;
        flags.z = false;

        temp = (temp >> 1) | (bit0 << 7);
        a = static_cast<uint8_t>(temp & 0xFF);
        fetch(pc);
    }

    auto CPU::rla() -> void {
        m_cycle++;
        const auto cy = static_cast<uint8_t>(flags.cy);

        flags.cy = static_cast<bool>(a & 0x80);
        flags.hc = false;
        flags.n = false;
        flags.z = false;

        a = (a << 1) | cy;
        fetch(pc);
    }

    auto CPU::rra() -> void {
        m_cycle++;
        const uint8_t cy = static_cast<uint8_t>(flags.cy) << 7;

        flags.cy = static_cast<bool>(a & 0x01);
        flags.hc = false;
        flags.n = false;
        flags.z = false;

        a = (a >> 1) | cy;
        fetch(pc);
    }

    auto CPU::daa() -> void {
        // Implementation adapted from: https://ehaskins.com/2018-01-30%20Z80%20DAA/
        m_cycle++;
        uint8_t correct = 0;
        auto temp = static_cast<uint16_t>(a);
        bool cy = false;

        if (flags.hc || (!flags.n && (temp & 0xF) > 9)) {
            correct |= 6;
        }

        if (flags.cy || (!flags.n && temp > 0x99)) {
            correct |= 0x60;
            cy = true;
        }

        temp += flags.n ? -correct : correct;
        temp &= 0xFF;

        flags.cy = cy;
        flags.hc = false;
        flags.z = temp == 0;

        a = static_cast<uint8_t>(temp);
        fetch(pc);
    }

    auto CPU::cpl() -> void {
        m_cycle++;

        a = ~a;
        flags.hc = true;
        flags.n = true;
        fetch(pc);
    }

    auto CPU::scf() -> void {
        m_cycle++;

        flags.cy = true;
        flags.hc = false;
        flags.n = false;
        fetch(pc);
    }

    auto CPU::ccf() -> void {
        m_cycle++;

        flags.cy = !flags.cy;
        flags.hc = false;
        flags.n = false;
        fetch(pc);
    }

    template <CPU::Operand operand> auto CPU::rlc() -> void {
        const int32_t temp = (operand == Operand::Memory) ? z : get_register(operand);
        const int32_t bit7 = temp >> 7;

        flags.cy = static_cast<bool>(bit7);
        flags.hc = false;
        flags.n = false;
        flags.z = temp == 0;

        set_register(operand, static_cast<uint8_t>(((temp << 1) | bit7) & 0xFF));
    }

    template <CPU::Operand operand> auto CPU::rrc() -> void {
        const int32_t temp = (operand == Operand::Memory) ? z : get_register(operand);
        const int32_t bit0 = (temp & 0x01) << 7;

        flags.cy = static_cast<bool>(temp & 0x01);
        flags.hc = false;
        flags.n = false;
        flags.z = temp == 0;

        set_register(operand, static_cast<uint8_t>(((temp >> 1) | bit0) & 0xFF));
    }

    template <CPU::Operand operand> auto CPU::rl() -> void {
        const int32_t temp = (operand == Operand::Memory) ? z : get_register(operand);
        const auto cy = static_cast<int32_t>(flags.cy);
        const auto result = static_cast<uint8_t>(((temp << 1) | cy) & 0xFF);

        flags.cy = static_cast<bool>((temp & 0x80) >> 7);
        flags.hc = false;
        flags.n = false;
        flags.z = result == 0;

        set_register(operand, result);
    }

    template <CPU::Operand operand> auto CPU::rr() -> void {
        const int32_t temp = (operand == Operand::Memory) ? z : get_register(operand);
        const auto cy = static_cast<int32_t>(flags.cy) << 7;
        const auto result = static_cast<uint8_t>(((temp >> 1) | cy) & 0xFF);

        flags.cy = static_cast<bool>(temp & 0x01);
        flags.hc = false;
        flags.n = false;
        flags.z = result == 0;

        set_register(operand, result);
    }

    template <CPU::Operand operand> auto CPU::sla() -> void {
        const int32_t temp = (operand == Operand::Memory) ? z : get_register(operand);
        const auto result = static_cast<uint8_t>((temp << 1) & 0xFF);

        flags.cy = static_cast<bool>((temp & 0x80) >> 7);
        flags.hc = false;
        flags.n = false;
        flags.z = result == 0;

        set_register(operand, result);
    }

    template <CPU::Operand operand> auto CPU::sra() -> void {
        const int32_t temp = (operand == Operand::Memory) ? z : get_register(operand);
        const auto result = static_cast<uint8_t>(((temp >> 1) | (temp & 0x80)) & 0xFF);

        flags.cy = static_cast<bool>(temp & 0x01);
        flags.hc = false;
        flags.n = false;
        flags.z = result == 0;

        set_register(operand, result);
    }

    template <CPU::Operand operand> auto CPU::swap() -> void {
        const int32_t temp = (operand == Operand::Memory) ? z : get_register(operand);
        const auto result =
            static_cast<uint8_t>(((temp & 0xF0) >> 4) | ((temp & 0x0F) << 4) & 0xFF);

        flags.cy = false;
        flags.hc = false;
        flags.n = false;
        flags.z = result == 0;

        set_register(operand, result);
    }

    template <CPU::Operand operand> auto CPU::srl() -> void {
        const int32_t temp = (operand == Operand::Memory) ? z : get_register(operand);
        const auto result = static_cast<uint8_t>((temp >> 1) & 0xFF);

        flags.cy = static_cast<bool>(temp & 0x01);
        flags.hc = false;
        flags.n = false;
        flags.z = result == 0;

        set_register(operand, result);
    }

    template <CPU::Operand operand, uint8_t bit_num> auto CPU::bit() -> void {
        const int32_t temp = (operand == Operand::Memory) ? z : get_register(operand);

        flags.hc = true;
        flags.n = false;
        flags.z = !(temp & (1 << bit_num));
    }

    template <CPU::Operand operand, uint8_t bit_num> auto CPU::res() -> void {
        const int32_t temp = (operand == Operand::Memory) ? z : get_register(operand);
        set_register(operand, static_cast<uint8_t>((temp & ~(1 << bit_num)) & 0xFF));
    }

    template <CPU::Operand operand, uint8_t bit_num> auto CPU::set() -> void {
        const int32_t temp = (operand == Operand::Memory) ? z : get_register(operand);
        set_register(operand, static_cast<uint8_t>((temp | (1 << bit_num)) & 0xFF));
    }

    template <CPU::ConditionCode cc, bool is_set> auto CPU::jr() -> void {
        m_cycle++;

        auto calc_address = [this]() {
            const int32_t pcl = (pc & 0xFF);
            const int32_t result = z + pcl;
            const bool cy = result > 0xFF;

            w = (z & 0x80) ? 255 : 0;
            z = static_cast<uint8_t>(result & 0xFF);
            w += static_cast<uint8_t>(pc >> 8) + static_cast<uint8_t>(cy);
        };

        switch (m_cycle) {
        case 2: {
            z = read(pc++);
            break;
        }
        case 3: {
            if constexpr (cc == ConditionCode::Always) {
                calc_address();
            }

            if constexpr (cc == ConditionCode::IfZero) {
                if (flags.z == is_set) {
                    calc_address();
                } else {
                    fetch(pc);
                }
            }

            if constexpr (cc == ConditionCode::IfCarry) {
                if (flags.cy == is_set) {
                    calc_address();
                } else {
                    fetch(pc);
                }
            }

            break;
        }

        case 4: {
            fetch((w << 8) | z);
            break;
        }
        default:;
        }
    }

    template <CPU::ConditionCode cc, bool is_set, bool from_hl> auto CPU::jp() -> void {
        m_cycle++;

        if constexpr (from_hl) {
            fetch(get_rp(RegisterPair::HL));
        } else {
            switch (m_cycle) {
            case 2: {
                z = read(pc++);
                break;
            }
            case 3: {
                w = read(pc++);
                break;
            }
            case 4: {
                if constexpr (cc == ConditionCode::Always) {
                    pc = (w << 8) | z;
                }

                if constexpr (cc == ConditionCode::IfZero) {
                    if (flags.z == is_set) {
                        pc = (w << 8) | z;
                    } else {
                        fetch(pc);
                    }
                }

                if constexpr (cc == ConditionCode::IfCarry) {
                    if (flags.cy == is_set) {
                        pc = (w << 8) | z;
                    } else {
                        fetch(pc);
                    }
                }

                break;
            }
            case 5: {
                fetch(pc);
                break;
            }

            default:;
            }
        }
    }

    template <CPU::ConditionCode cc, bool is_set> auto CPU::call() -> void {
        m_cycle++;

        switch (m_cycle) {
        case 2: {
            z = read(pc++);
            break;
        }
        case 3: {
            w = read(pc++);
            break;
        }
        case 4: {
            if constexpr (cc == ConditionCode::Always) {
                sp--;
            }

            if constexpr (cc == ConditionCode::IfZero) {
                if (flags.z == is_set) {
                    sp--;
                } else {
                    fetch(pc);
                }
            }

            if constexpr (cc == ConditionCode::IfCarry) {
                if (flags.cy == is_set) {
                    sp--;
                } else {
                    fetch(pc);
                }
            }

            break;
        }
        case 5: {
            write(sp--, pc >> 8);
            break;
        }
        case 6: {
            write(sp, pc & 0xFF);
            pc = (w << 8) | z;
            break;
        }
        case 7: {
            fetch(pc);
            break;
        }
        default:;
        }
    }

    template <bool enable_interrupts> auto CPU::ret() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = read(sp++);
            break;
        }
        case 3: {
            w = read(sp++);
            break;
        }
        case 4: {
            pc = (w << 8) | z;
            if constexpr (enable_interrupts) {
                ime = true;
            }
            break;
        }
        case 5: {
            fetch(pc);
            break;
        }

        default:;
        }
    }

    template <CPU::ConditionCode cc, bool is_set> auto CPU::ret_cc() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            if constexpr (cc == ConditionCode::IfZero) {
                if (flags.z != is_set) {
                    m_cycle = 5;
                }
            }

            if constexpr (cc == ConditionCode::IfCarry) {
                if (flags.cy != is_set) {
                    m_cycle = 5;
                }
            }

            break;
        }
        case 3: {
            z = read(sp++);
            break;
        }
        case 4: {
            w = read(sp++);
            break;
        }
        case 5: {
            pc = (w << 8) | z;
            break;
        }
        case 6: {
            fetch(pc);
            break;
        }

        default:;
        }
    }

    auto CPU::rst() -> void {
        m_cycle++;

        switch (m_cycle) {
        case 2: {
            sp--;
            break;
        }
        case 3: {
            write(sp--, pc >> 8);
            break;
        }
        case 4: {
            write(sp, pc & 0xFF);
            pc = ir & 0x38;
            break;
        }
        case 5: {
            fetch(pc);
            break;
        }

        default:;
        }
    }

    template <RegisterPair rp> auto CPU::push() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            sp--;
            break;
        }
        case 3: {
            write(sp--, get_rp(rp) >> 8);
            break;
        }
        case 4: {
            write(sp, get_rp(rp) & 0xFF);
            break;
        }
        case 5: {
            fetch(pc);
            break;
        }
        default:;
        }
    }

    template <RegisterPair rp> auto CPU::pop() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = read(sp++);
            break;
        }
        case 3: {
            w = read(sp++);
            break;
        }
        case 4: {
            set_rp(rp, (w << 8) | z);
            fetch(pc);
            break;
        }
        default:;
        }
    }

    template <typename Fn> auto CPU::immediate_addr(Fn &&func) -> void {
        m_cycle++;
        (this->*func)();
        fetch(pc);
    }

    template <CPU::MemRead address, typename Fn> auto CPU::mem_read_addr(Fn &&func) -> void {
        m_cycle++;

        switch (m_cycle) {
        case 2: {
            if constexpr (address == MemRead::PC) {
                z = read(pc++);
            } else {
                z = read(get_rp(RegisterPair::HL));
            }
            break;
        }
        case 3: {
            (this->*func)();
            fetch(pc);
            break;
        }
        default:;
        }
    }

    template <typename Fn> auto CPU::mem_write_addr(Fn &&func) -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            write(get_rp(RegisterPair::HL), (this->*func)());
            break;
        }
        case 3: {
            fetch(pc);
            break;
        }
        default:;
        }
    }

    template <typename Fn> auto CPU::read_modify_write(Fn &&func) -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = read(get_rp(RegisterPair::HL));
            break;
        }
        case 3: {
            (this->*func)();
            write(get_rp(RegisterPair::HL), z);
            break;
        }
        case 4: {
            fetch(pc);
            break;
        }
        default:;
        }
    }

    auto CPU::decode_execute() -> void {
        switch (ir) {
        case 0x00: nop(); return;
        case 0x01: ld_rp_immediate<RegisterPair::BC>(); return;
        case 0x02: ld_indirect_accumulator<Operand2::BC>(); return;
        case 0x03: adjust_rp<RegisterPair::BC, 1>(); return;
        case 0x04: immediate_addr(&CPU::inc_r<Operand::B>); return;
        case 0x05: immediate_addr(&CPU::dec_r<Operand::B>); return;
        case 0x06: ld_immediate<Operand::B>(); return;
        case 0x07: rlca(); return;
        case 0x08: ld_direct_sp(); return;
        case 0x09: add_hl_rp<RegisterPair::BC>(); return;
        case 0x0A: ld_accumulator_indirect<Operand2::BC>(); return;
        case 0x0B: adjust_rp<RegisterPair::BC, -1>(); return;
        case 0x0C: immediate_addr(&CPU::inc_r<Operand::C>); return;
        case 0x0D: immediate_addr(&CPU::dec_r<Operand::C>); return;
        case 0x0E: ld_immediate<Operand::C>(); return;
        case 0x0F: rrca(); return;

        case 0x10: stop(); return;
        case 0x11: ld_rp_immediate<RegisterPair::DE>(); return;
        case 0x12: ld_indirect_accumulator<Operand2::DE>(); return;
        case 0x13: adjust_rp<RegisterPair::DE, 1>(); return;
        case 0x14: immediate_addr(&CPU::inc_r<Operand::D>); return;
        case 0x15: immediate_addr(&CPU::dec_r<Operand::D>); return;
        case 0x16: ld_immediate<Operand::D>(); return;
        case 0x17: rla(); return;
        case 0x18: jr<ConditionCode::Always, false>(); return;
        case 0x19: add_hl_rp<RegisterPair::DE>(); return;
        case 0x1A: ld_accumulator_indirect<Operand2::DE>(); return;
        case 0x1B: adjust_rp<RegisterPair::DE, -1>(); return;
        case 0x1C: immediate_addr(&CPU::inc_r<Operand::E>); return;
        case 0x1D: immediate_addr(&CPU::dec_r<Operand::E>); return;
        case 0x1E: ld_immediate<Operand::E>(); return;
        case 0x1F: rra(); return;

        case 0x20: jr<ConditionCode::IfZero, false>(); return;
        case 0x21: ld_rp_immediate<RegisterPair::HL>(); return;
        case 0x22: ld_indirect_accumulator<Operand2::HLIncrement>(); return;
        case 0x23: adjust_rp<RegisterPair::HL, 1>(); return;
        case 0x24: immediate_addr(&CPU::inc_r<Operand::H>); return;
        case 0x25: immediate_addr(&CPU::dec_r<Operand::H>); return;
        case 0x26: ld_immediate<Operand::H>(); return;
        case 0x27: daa(); return;
        case 0x28: jr<ConditionCode::IfZero, true>(); return;
        case 0x29: add_hl_rp<RegisterPair::HL>(); return;
        case 0x2A: ld_accumulator_indirect<Operand2::HLIncrement>(); return;
        case 0x2B: adjust_rp<RegisterPair::HL, -1>(); return;
        case 0x2C: immediate_addr(&CPU::inc_r<Operand::L>); return;
        case 0x2D: immediate_addr(&CPU::dec_r<Operand::L>); return;
        case 0x2E: ld_immediate<Operand::L>(); return;
        case 0x2F: cpl(); return;

        case 0x30: jr<ConditionCode::IfCarry, false>(); return;
        case 0x31: ld_rp_immediate<RegisterPair::SP>(); return;
        case 0x32: ld_indirect_accumulator<Operand2::HLDecrement>(); return;
        case 0x33: adjust_rp<RegisterPair::SP, 1>(); return;
        case 0x34: read_modify_write(&CPU::inc_r<Operand::Memory>); return;
        case 0x35: read_modify_write(&CPU::dec_r<Operand::Memory>); return;
        case 0x36: ld_immediate<Operand::Memory>(); return;
        case 0x37: scf(); return;
        case 0x38: jr<ConditionCode::IfCarry, true>(); return;
        case 0x39: add_hl_rp<RegisterPair::SP>(); return;
        case 0x3A: ld_accumulator_indirect<Operand2::HLDecrement>(); return;
        case 0x3B: adjust_rp<RegisterPair::SP, -1>(); return;
        case 0x3C: immediate_addr(&CPU::inc_r<Operand::A>); return;
        case 0x3D: immediate_addr(&CPU::dec_r<Operand::A>); return;
        case 0x3E: ld_immediate<Operand::A>(); return;
        case 0x3F: ccf(); return;

        case 0x40: ld<Operand::B, Operand::B>(); return;
        case 0x41: ld<Operand::B, Operand::C>(); return;
        case 0x42: ld<Operand::B, Operand::D>(); return;
        case 0x43: ld<Operand::B, Operand::E>(); return;
        case 0x44: ld<Operand::B, Operand::H>(); return;
        case 0x45: ld<Operand::B, Operand::L>(); return;
        case 0x46: ld<Operand::B, Operand::Memory>(); return;
        case 0x47: ld<Operand::B, Operand::A>(); return;
        case 0x48: ld<Operand::C, Operand::B>(); return;
        case 0x49: ld<Operand::C, Operand::C>(); return;
        case 0x4A: ld<Operand::C, Operand::D>(); return;
        case 0x4B: ld<Operand::C, Operand::E>(); return;
        case 0x4C: ld<Operand::C, Operand::H>(); return;
        case 0x4D: ld<Operand::C, Operand::L>(); return;
        case 0x4E: ld<Operand::C, Operand::Memory>(); return;
        case 0x4F: ld<Operand::C, Operand::A>(); return;

        case 0x50: ld<Operand::D, Operand::B>(); return;
        case 0x51: ld<Operand::D, Operand::C>(); return;
        case 0x52: ld<Operand::D, Operand::D>(); return;
        case 0x53: ld<Operand::D, Operand::E>(); return;
        case 0x54: ld<Operand::D, Operand::H>(); return;
        case 0x55: ld<Operand::D, Operand::L>(); return;
        case 0x56: ld<Operand::D, Operand::Memory>(); return;
        case 0x57: ld<Operand::D, Operand::A>(); return;
        case 0x58: ld<Operand::E, Operand::B>(); return;
        case 0x59: ld<Operand::E, Operand::C>(); return;
        case 0x5A: ld<Operand::E, Operand::D>(); return;
        case 0x5B: ld<Operand::E, Operand::E>(); return;
        case 0x5C: ld<Operand::E, Operand::H>(); return;
        case 0x5D: ld<Operand::E, Operand::L>(); return;
        case 0x5E: ld<Operand::E, Operand::Memory>(); return;
        case 0x5F: ld<Operand::E, Operand::A>(); return;

        case 0x60: ld<Operand::H, Operand::B>(); return;
        case 0x61: ld<Operand::H, Operand::C>(); return;
        case 0x62: ld<Operand::H, Operand::D>(); return;
        case 0x63: ld<Operand::H, Operand::E>(); return;
        case 0x64: ld<Operand::H, Operand::H>(); return;
        case 0x65: ld<Operand::H, Operand::L>(); return;
        case 0x66: ld<Operand::H, Operand::Memory>(); return;
        case 0x67: ld<Operand::H, Operand::A>(); return;
        case 0x68: ld<Operand::L, Operand::B>(); return;
        case 0x69: ld<Operand::L, Operand::C>(); return;
        case 0x6A: ld<Operand::L, Operand::D>(); return;
        case 0x6B: ld<Operand::L, Operand::E>(); return;
        case 0x6C: ld<Operand::L, Operand::H>(); return;
        case 0x6D: ld<Operand::L, Operand::L>(); return;
        case 0x6E: ld<Operand::L, Operand::Memory>(); return;
        case 0x6F: ld<Operand::L, Operand::A>(); return;

        case 0x70: ld<Operand::Memory, Operand::B>(); return;
        case 0x71: ld<Operand::Memory, Operand::C>(); return;
        case 0x72: ld<Operand::Memory, Operand::D>(); return;
        case 0x73: ld<Operand::Memory, Operand::E>(); return;
        case 0x74: ld<Operand::Memory, Operand::H>(); return;
        case 0x75: ld<Operand::Memory, Operand::L>(); return;
        case 0x76: halt(); return;
        case 0x77: ld<Operand::Memory, Operand::A>(); return;
        case 0x78: ld<Operand::A, Operand::B>(); return;
        case 0x79: ld<Operand::A, Operand::C>(); return;
        case 0x7A: ld<Operand::A, Operand::D>(); return;
        case 0x7B: ld<Operand::A, Operand::E>(); return;
        case 0x7C: ld<Operand::A, Operand::H>(); return;
        case 0x7D: ld<Operand::A, Operand::L>(); return;
        case 0x7E: ld<Operand::A, Operand::Memory>(); return;
        case 0x7F: ld<Operand::A, Operand::A>(); return;

        case 0x80: immediate_addr(&CPU::adc<Operand::B, false>); return;
        case 0x81: immediate_addr(&CPU::adc<Operand::C, false>); return;
        case 0x82: immediate_addr(&CPU::adc<Operand::D, false>); return;
        case 0x83: immediate_addr(&CPU::adc<Operand::E, false>); return;
        case 0x84: immediate_addr(&CPU::adc<Operand::H, false>); return;
        case 0x85: immediate_addr(&CPU::adc<Operand::L, false>); return;
        case 0x86: mem_read_addr<MemRead::HL>(&CPU::adc<Operand::Memory, false>); return;
        case 0x87: immediate_addr(&CPU::adc<Operand::A, false>); return;
        case 0x88: immediate_addr(&CPU::adc<Operand::B, true>); return;
        case 0x89: immediate_addr(&CPU::adc<Operand::C, true>); return;
        case 0x8A: immediate_addr(&CPU::adc<Operand::D, true>); return;
        case 0x8B: immediate_addr(&CPU::adc<Operand::E, true>); return;
        case 0x8C: immediate_addr(&CPU::adc<Operand::H, true>); return;
        case 0x8D: immediate_addr(&CPU::adc<Operand::L, true>); return;
        case 0x8E: mem_read_addr<MemRead::HL>(&CPU::adc<Operand::Memory, true>); return;
        case 0x8F: immediate_addr(&CPU::adc<Operand::A, true>); return;

        case 0x90: immediate_addr(&CPU::sbc<Operand::B, false>); return;
        case 0x91: immediate_addr(&CPU::sbc<Operand::C, false>); return;
        case 0x92: immediate_addr(&CPU::sbc<Operand::D, false>); return;
        case 0x93: immediate_addr(&CPU::sbc<Operand::E, false>); return;
        case 0x94: immediate_addr(&CPU::sbc<Operand::H, false>); return;
        case 0x95: immediate_addr(&CPU::sbc<Operand::L, false>); return;
        case 0x96: mem_read_addr<MemRead::HL>(&CPU::sbc<Operand::Memory, false>); return;
        case 0x97: immediate_addr(&CPU::sbc<Operand::A, false>); return;
        case 0x98: immediate_addr(&CPU::sbc<Operand::B, true>); return;
        case 0x99: immediate_addr(&CPU::sbc<Operand::C, true>); return;
        case 0x9A: immediate_addr(&CPU::sbc<Operand::D, true>); return;
        case 0x9B: immediate_addr(&CPU::sbc<Operand::E, true>); return;
        case 0x9C: immediate_addr(&CPU::sbc<Operand::H, true>); return;
        case 0x9D: immediate_addr(&CPU::sbc<Operand::L, true>); return;
        case 0x9E: mem_read_addr<MemRead::HL>(&CPU::sbc<Operand::Memory, true>); return;
        case 0x9F: immediate_addr(&CPU::sbc<Operand::A, true>); return;

        case 0xA0: immediate_addr(&CPU::and_op<Operand::B>); return;
        case 0xA1: immediate_addr(&CPU::and_op<Operand::C>); return;
        case 0xA2: immediate_addr(&CPU::and_op<Operand::D>); return;
        case 0xA3: immediate_addr(&CPU::and_op<Operand::E>); return;
        case 0xA4: immediate_addr(&CPU::and_op<Operand::H>); return;
        case 0xA5: immediate_addr(&CPU::and_op<Operand::L>); return;
        case 0xA6: mem_read_addr<MemRead::HL>(&CPU::and_op<Operand::Memory>); return;
        case 0xA7: immediate_addr(&CPU::and_op<Operand::A>); return;
        case 0xA8: immediate_addr(&CPU::xor_op<Operand::B>); return;
        case 0xA9: immediate_addr(&CPU::xor_op<Operand::C>); return;
        case 0xAA: immediate_addr(&CPU::xor_op<Operand::D>); return;
        case 0xAB: immediate_addr(&CPU::xor_op<Operand::E>); return;
        case 0xAC: immediate_addr(&CPU::xor_op<Operand::H>); return;
        case 0xAD: immediate_addr(&CPU::xor_op<Operand::L>); return;
        case 0xAE: mem_read_addr<MemRead::HL>(&CPU::xor_op<Operand::Memory>); return;
        case 0xAF: immediate_addr(&CPU::xor_op<Operand::A>); return;

        case 0xB0: immediate_addr(&CPU::or_op<Operand::B>); return;
        case 0xB1: immediate_addr(&CPU::or_op<Operand::C>); return;
        case 0xB2: immediate_addr(&CPU::or_op<Operand::D>); return;
        case 0xB3: immediate_addr(&CPU::or_op<Operand::E>); return;
        case 0xB4: immediate_addr(&CPU::or_op<Operand::H>); return;
        case 0xB5: immediate_addr(&CPU::or_op<Operand::L>); return;
        case 0xB6: mem_read_addr<MemRead::HL>(&CPU::or_op<Operand::Memory>); return;
        case 0xB7: immediate_addr(&CPU::or_op<Operand::A>); return;
        case 0xB8: immediate_addr(&CPU::cp<Operand::B>); return;
        case 0xB9: immediate_addr(&CPU::cp<Operand::C>); return;
        case 0xBA: immediate_addr(&CPU::cp<Operand::D>); return;
        case 0xBB: immediate_addr(&CPU::cp<Operand::E>); return;
        case 0xBC: immediate_addr(&CPU::cp<Operand::H>); return;
        case 0xBD: immediate_addr(&CPU::cp<Operand::L>); return;
        case 0xBE: mem_read_addr<MemRead::HL>(&CPU::cp<Operand::Memory>); return;
        case 0xBF: immediate_addr(&CPU::cp<Operand::A>); return;

        case 0xC0: ret_cc<ConditionCode::IfZero, false>(); return;
        case 0xC1: pop<RegisterPair::BC>(); return;
        case 0xC2: jp<ConditionCode::IfZero, false, false>(); return;
        case 0xC3: jp<ConditionCode::Always, false, false>(); return;
        case 0xC4: call<ConditionCode::IfZero, false>(); return;
        case 0xC5: push<RegisterPair::BC>(); return;
        case 0xC6: mem_read_addr<MemRead::PC>(&CPU::adc<Operand::Memory, false>); return;
        case 0xC7: rst(); return;
        case 0xC8: ret_cc<ConditionCode::IfZero, true>(); return;
        case 0xC9: ret<false>(); return;
        case 0xCA: jp<ConditionCode::IfZero, true, false>(); return;
        case 0xCB: bitops_bank_switch(); return;
        case 0xCC: call<ConditionCode::IfZero, true>(); return;
        case 0xCD: call<ConditionCode::Always, false>(); return;
        case 0xCE: mem_read_addr<MemRead::PC>(&CPU::adc<Operand::Memory, true>); return;
        case 0xCF: rst(); return;

        case 0xD0: ret_cc<ConditionCode::IfCarry, false>(); return;
        case 0xD1: pop<RegisterPair::DE>(); return;
        case 0xD2: jp<ConditionCode::IfCarry, false, false>(); return;
        case 0xD3: illegal_instruction(); return;
        case 0xD4: call<ConditionCode::IfCarry, false>(); return;
        case 0xD5: push<RegisterPair::DE>(); return;
        case 0xD6: mem_read_addr<MemRead::PC>(&CPU::sbc<Operand::Memory, false>); return;
        case 0xD7: rst(); return;
        case 0xD8: ret_cc<ConditionCode::IfCarry, true>(); return;
        case 0xD9: ret<true>(); return;
        case 0xDA: jp<ConditionCode::IfCarry, true, false>(); return;
        case 0xDB: illegal_instruction(); return;
        case 0xDC: call<ConditionCode::IfCarry, true>(); return;
        case 0xDD: illegal_instruction(); return;
        case 0xDE: mem_read_addr<MemRead::PC>(&CPU::sbc<Operand::Memory, true>); return;
        case 0xDF: rst(); return;

        case 0xE0: ldh_offset_a(); return;
        case 0xE1: pop<RegisterPair::HL>(); return;
        case 0xE2: ldh_c_a(); return;
        case 0xE3:
        case 0xE4: illegal_instruction(); return;
        case 0xE5: push<RegisterPair::HL>(); return;
        case 0xE6: mem_read_addr<MemRead::PC>(&CPU::and_op<Operand::Memory>); return;
        case 0xE7: rst(); return;
        case 0xE8: add_sp_i8(); return;
        case 0xE9: jp<ConditionCode::Always, false, true>(); return;
        case 0xEA: ld_direct_a(); return;
        case 0xEB:
        case 0xEC:
        case 0xED: illegal_instruction(); return;
        case 0xEE: mem_read_addr<MemRead::PC>(&CPU::xor_op<Operand::Memory>); return;
        case 0xEF: rst(); return;

        case 0xF0: ldh_a_offset(); return;
        case 0xF1: pop<RegisterPair::AF>(); return;
        case 0xF2: ldh_a_c(); return;
        case 0xF3: di(); return;
        case 0xF4: illegal_instruction(); return;
        case 0xF5: push<RegisterPair::AF>(); return;
        case 0xF6: mem_read_addr<MemRead::PC>(&CPU::or_op<Operand::Memory>); return;
        case 0xF7: rst(); return;
        case 0xF8: ld_hl_sp_i8(); return;
        case 0xF9: ld_sp_hl(); return;
        case 0xFA: ld_a_direct(); return;
        case 0xFB: ei(); return;
        case 0xFC:
        case 0xFD: illegal_instruction(); return;
        case 0xFE: mem_read_addr<MemRead::PC>(&CPU::cp<Operand::Memory>); return;
        case 0xFF: rst(); return;
        default:;
        }
    }

    auto CPU::decode_execute_bitops() -> void {
        switch (ir) {
        case 0x00: immediate_addr(&CPU::rlc<Operand::B>); return;
        case 0x01: immediate_addr(&CPU::rlc<Operand::C>); return;
        case 0x02: immediate_addr(&CPU::rlc<Operand::D>); return;
        case 0x03: immediate_addr(&CPU::rlc<Operand::E>); return;
        case 0x04: immediate_addr(&CPU::rlc<Operand::H>); return;
        case 0x05: immediate_addr(&CPU::rlc<Operand::L>); return;
        case 0x06: read_modify_write(&CPU::rlc<Operand::Memory>); return;
        case 0x07: immediate_addr(&CPU::rlc<Operand::A>); return;
        case 0x08: immediate_addr(&CPU::rrc<Operand::B>); return;
        case 0x09: immediate_addr(&CPU::rrc<Operand::C>); return;
        case 0x0A: immediate_addr(&CPU::rrc<Operand::D>); return;
        case 0x0B: immediate_addr(&CPU::rrc<Operand::E>); return;
        case 0x0C: immediate_addr(&CPU::rrc<Operand::H>); return;
        case 0x0D: immediate_addr(&CPU::rrc<Operand::L>); return;
        case 0x0E: read_modify_write(&CPU::rrc<Operand::Memory>); return;
        case 0x0F: immediate_addr(&CPU::rrc<Operand::A>); return;

        case 0x10: immediate_addr(&CPU::rl<Operand::B>); return;
        case 0x11: immediate_addr(&CPU::rl<Operand::C>); return;
        case 0x12: immediate_addr(&CPU::rl<Operand::D>); return;
        case 0x13: immediate_addr(&CPU::rl<Operand::E>); return;
        case 0x14: immediate_addr(&CPU::rl<Operand::H>); return;
        case 0x15: immediate_addr(&CPU::rl<Operand::L>); return;
        case 0x16: read_modify_write(&CPU::rl<Operand::Memory>); return;
        case 0x17: immediate_addr(&CPU::rl<Operand::A>); return;
        case 0x18: immediate_addr(&CPU::rr<Operand::B>); return;
        case 0x19: immediate_addr(&CPU::rr<Operand::C>); return;
        case 0x1A: immediate_addr(&CPU::rr<Operand::D>); return;
        case 0x1B: immediate_addr(&CPU::rr<Operand::E>); return;
        case 0x1C: immediate_addr(&CPU::rr<Operand::H>); return;
        case 0x1D: immediate_addr(&CPU::rr<Operand::L>); return;
        case 0x1E: read_modify_write(&CPU::rr<Operand::Memory>); return;
        case 0x1F: immediate_addr(&CPU::rr<Operand::A>); return;

        case 0x20: immediate_addr(&CPU::sla<Operand::B>); return;
        case 0x21: immediate_addr(&CPU::sla<Operand::C>); return;
        case 0x22: immediate_addr(&CPU::sla<Operand::D>); return;
        case 0x23: immediate_addr(&CPU::sla<Operand::E>); return;
        case 0x24: immediate_addr(&CPU::sla<Operand::H>); return;
        case 0x25: immediate_addr(&CPU::sla<Operand::L>); return;
        case 0x26: read_modify_write(&CPU::sla<Operand::Memory>); return;
        case 0x27: immediate_addr(&CPU::sla<Operand::A>); return;
        case 0x28: immediate_addr(&CPU::sra<Operand::B>); return;
        case 0x29: immediate_addr(&CPU::sra<Operand::C>); return;
        case 0x2A: immediate_addr(&CPU::sra<Operand::D>); return;
        case 0x2B: immediate_addr(&CPU::sra<Operand::E>); return;
        case 0x2C: immediate_addr(&CPU::sra<Operand::H>); return;
        case 0x2D: immediate_addr(&CPU::sra<Operand::L>); return;
        case 0x2E: read_modify_write(&CPU::sra<Operand::Memory>); return;
        case 0x2F: immediate_addr(&CPU::sra<Operand::A>); return;

        case 0x30: immediate_addr(&CPU::swap<Operand::B>); return;
        case 0x31: immediate_addr(&CPU::swap<Operand::C>); return;
        case 0x32: immediate_addr(&CPU::swap<Operand::D>); return;
        case 0x33: immediate_addr(&CPU::swap<Operand::E>); return;
        case 0x34: immediate_addr(&CPU::swap<Operand::H>); return;
        case 0x35: immediate_addr(&CPU::swap<Operand::L>); return;
        case 0x36: read_modify_write(&CPU::swap<Operand::Memory>); return;
        case 0x37: immediate_addr(&CPU::swap<Operand::A>); return;
        case 0x38: immediate_addr(&CPU::srl<Operand::B>); return;
        case 0x39: immediate_addr(&CPU::srl<Operand::C>); return;
        case 0x3A: immediate_addr(&CPU::srl<Operand::D>); return;
        case 0x3B: immediate_addr(&CPU::srl<Operand::E>); return;
        case 0x3C: immediate_addr(&CPU::srl<Operand::H>); return;
        case 0x3D: immediate_addr(&CPU::srl<Operand::L>); return;
        case 0x3E: read_modify_write(&CPU::srl<Operand::Memory>); return;
        case 0x3F: immediate_addr(&CPU::srl<Operand::A>); return;

        case 0x40: immediate_addr(&CPU::bit<Operand::B, 0>); return;
        case 0x41: immediate_addr(&CPU::bit<Operand::C, 0>); return;
        case 0x42: immediate_addr(&CPU::bit<Operand::D, 0>); return;
        case 0x43: immediate_addr(&CPU::bit<Operand::E, 0>); return;
        case 0x44: immediate_addr(&CPU::bit<Operand::H, 0>); return;
        case 0x45: immediate_addr(&CPU::bit<Operand::L, 0>); return;
        case 0x46: mem_read_addr<MemRead::HL>(&CPU::bit<Operand::Memory, 0>); return;
        case 0x47: immediate_addr(&CPU::bit<Operand::A, 0>); return;
        case 0x48: immediate_addr(&CPU::bit<Operand::B, 1>); return;
        case 0x49: immediate_addr(&CPU::bit<Operand::C, 1>); return;
        case 0x4A: immediate_addr(&CPU::bit<Operand::D, 1>); return;
        case 0x4B: immediate_addr(&CPU::bit<Operand::E, 1>); return;
        case 0x4C: immediate_addr(&CPU::bit<Operand::H, 1>); return;
        case 0x4D: immediate_addr(&CPU::bit<Operand::L, 1>); return;
        case 0x4E: mem_read_addr<MemRead::HL>(&CPU::bit<Operand::Memory, 1>); return;
        case 0x4F: immediate_addr(&CPU::bit<Operand::A, 1>); return;

        case 0x50: immediate_addr(&CPU::bit<Operand::B, 2>); return;
        case 0x51: immediate_addr(&CPU::bit<Operand::C, 2>); return;
        case 0x52: immediate_addr(&CPU::bit<Operand::D, 2>); return;
        case 0x53: immediate_addr(&CPU::bit<Operand::E, 2>); return;
        case 0x54: immediate_addr(&CPU::bit<Operand::H, 2>); return;
        case 0x55: immediate_addr(&CPU::bit<Operand::L, 2>); return;
        case 0x56: mem_read_addr<MemRead::HL>(&CPU::bit<Operand::Memory, 2>); return;
        case 0x57: immediate_addr(&CPU::bit<Operand::A, 2>); return;
        case 0x58: immediate_addr(&CPU::bit<Operand::B, 3>); return;
        case 0x59: immediate_addr(&CPU::bit<Operand::C, 3>); return;
        case 0x5A: immediate_addr(&CPU::bit<Operand::D, 3>); return;
        case 0x5B: immediate_addr(&CPU::bit<Operand::E, 3>); return;
        case 0x5C: immediate_addr(&CPU::bit<Operand::H, 3>); return;
        case 0x5D: immediate_addr(&CPU::bit<Operand::L, 3>); return;
        case 0x5E: mem_read_addr<MemRead::HL>(&CPU::bit<Operand::Memory, 3>); return;
        case 0x5F: immediate_addr(&CPU::bit<Operand::A, 3>); return;

        case 0x60: immediate_addr(&CPU::bit<Operand::B, 4>); return;
        case 0x61: immediate_addr(&CPU::bit<Operand::C, 4>); return;
        case 0x62: immediate_addr(&CPU::bit<Operand::D, 4>); return;
        case 0x63: immediate_addr(&CPU::bit<Operand::E, 4>); return;
        case 0x64: immediate_addr(&CPU::bit<Operand::H, 4>); return;
        case 0x65: immediate_addr(&CPU::bit<Operand::L, 4>); return;
        case 0x66: mem_read_addr<MemRead::HL>(&CPU::bit<Operand::Memory, 4>); return;
        case 0x67: immediate_addr(&CPU::bit<Operand::A, 4>); return;
        case 0x68: immediate_addr(&CPU::bit<Operand::B, 5>); return;
        case 0x69: immediate_addr(&CPU::bit<Operand::C, 5>); return;
        case 0x6A: immediate_addr(&CPU::bit<Operand::D, 5>); return;
        case 0x6B: immediate_addr(&CPU::bit<Operand::E, 5>); return;
        case 0x6C: immediate_addr(&CPU::bit<Operand::H, 5>); return;
        case 0x6D: immediate_addr(&CPU::bit<Operand::L, 5>); return;
        case 0x6E: mem_read_addr<MemRead::HL>(&CPU::bit<Operand::Memory, 5>); return;
        case 0x6F: immediate_addr(&CPU::bit<Operand::A, 5>); return;

        case 0x70: immediate_addr(&CPU::bit<Operand::B, 6>); return;
        case 0x71: immediate_addr(&CPU::bit<Operand::C, 6>); return;
        case 0x72: immediate_addr(&CPU::bit<Operand::D, 6>); return;
        case 0x73: immediate_addr(&CPU::bit<Operand::E, 6>); return;
        case 0x74: immediate_addr(&CPU::bit<Operand::H, 6>); return;
        case 0x75: immediate_addr(&CPU::bit<Operand::L, 6>); return;
        case 0x76: mem_read_addr<MemRead::HL>(&CPU::bit<Operand::Memory, 6>); return;
        case 0x77: immediate_addr(&CPU::bit<Operand::A, 6>); return;
        case 0x78: immediate_addr(&CPU::bit<Operand::B, 7>); return;
        case 0x79: immediate_addr(&CPU::bit<Operand::C, 7>); return;
        case 0x7A: immediate_addr(&CPU::bit<Operand::D, 7>); return;
        case 0x7B: immediate_addr(&CPU::bit<Operand::E, 7>); return;
        case 0x7C: immediate_addr(&CPU::bit<Operand::H, 7>); return;
        case 0x7D: immediate_addr(&CPU::bit<Operand::L, 7>); return;
        case 0x7E: mem_read_addr<MemRead::HL>(&CPU::bit<Operand::Memory, 7>); return;
        case 0x7F: immediate_addr(&CPU::bit<Operand::A, 7>); return;

        case 0x80: immediate_addr(&CPU::res<Operand::B, 0>); return;
        case 0x81: immediate_addr(&CPU::res<Operand::C, 0>); return;
        case 0x82: immediate_addr(&CPU::res<Operand::D, 0>); return;
        case 0x83: immediate_addr(&CPU::res<Operand::E, 0>); return;
        case 0x84: immediate_addr(&CPU::res<Operand::H, 0>); return;
        case 0x85: immediate_addr(&CPU::res<Operand::L, 0>); return;
        case 0x86: read_modify_write(&CPU::res<Operand::Memory, 0>); return;
        case 0x87: immediate_addr(&CPU::res<Operand::A, 0>); return;
        case 0x88: immediate_addr(&CPU::res<Operand::B, 1>); return;
        case 0x89: immediate_addr(&CPU::res<Operand::C, 1>); return;
        case 0x8A: immediate_addr(&CPU::res<Operand::D, 1>); return;
        case 0x8B: immediate_addr(&CPU::res<Operand::E, 1>); return;
        case 0x8C: immediate_addr(&CPU::res<Operand::H, 1>); return;
        case 0x8D: immediate_addr(&CPU::res<Operand::L, 1>); return;
        case 0x8E: read_modify_write(&CPU::res<Operand::Memory, 1>); return;
        case 0x8F: immediate_addr(&CPU::res<Operand::A, 1>); return;

        case 0x90: immediate_addr(&CPU::res<Operand::B, 2>); return;
        case 0x91: immediate_addr(&CPU::res<Operand::C, 2>); return;
        case 0x92: immediate_addr(&CPU::res<Operand::D, 2>); return;
        case 0x93: immediate_addr(&CPU::res<Operand::E, 2>); return;
        case 0x94: immediate_addr(&CPU::res<Operand::H, 2>); return;
        case 0x95: immediate_addr(&CPU::res<Operand::L, 2>); return;
        case 0x96: read_modify_write(&CPU::res<Operand::Memory, 2>); return;
        case 0x97: immediate_addr(&CPU::res<Operand::A, 2>); return;
        case 0x98: immediate_addr(&CPU::res<Operand::B, 3>); return;
        case 0x99: immediate_addr(&CPU::res<Operand::C, 3>); return;
        case 0x9A: immediate_addr(&CPU::res<Operand::D, 3>); return;
        case 0x9B: immediate_addr(&CPU::res<Operand::E, 3>); return;
        case 0x9C: immediate_addr(&CPU::res<Operand::H, 3>); return;
        case 0x9D: immediate_addr(&CPU::res<Operand::L, 3>); return;
        case 0x9E: read_modify_write(&CPU::res<Operand::Memory, 3>); return;
        case 0x9F: immediate_addr(&CPU::res<Operand::A, 3>); return;

        case 0xA0: immediate_addr(&CPU::res<Operand::B, 4>); return;
        case 0xA1: immediate_addr(&CPU::res<Operand::C, 4>); return;
        case 0xA2: immediate_addr(&CPU::res<Operand::D, 4>); return;
        case 0xA3: immediate_addr(&CPU::res<Operand::E, 4>); return;
        case 0xA4: immediate_addr(&CPU::res<Operand::H, 4>); return;
        case 0xA5: immediate_addr(&CPU::res<Operand::L, 4>); return;
        case 0xA6: read_modify_write(&CPU::res<Operand::Memory, 4>); return;
        case 0xA7: immediate_addr(&CPU::res<Operand::A, 4>); return;
        case 0xA8: immediate_addr(&CPU::res<Operand::B, 5>); return;
        case 0xA9: immediate_addr(&CPU::res<Operand::C, 5>); return;
        case 0xAA: immediate_addr(&CPU::res<Operand::D, 5>); return;
        case 0xAB: immediate_addr(&CPU::res<Operand::E, 5>); return;
        case 0xAC: immediate_addr(&CPU::res<Operand::H, 5>); return;
        case 0xAD: immediate_addr(&CPU::res<Operand::L, 5>); return;
        case 0xAE: read_modify_write(&CPU::res<Operand::Memory, 5>); return;
        case 0xAF: immediate_addr(&CPU::res<Operand::A, 5>); return;

        case 0xB0: immediate_addr(&CPU::res<Operand::B, 6>); return;
        case 0xB1: immediate_addr(&CPU::res<Operand::C, 6>); return;
        case 0xB2: immediate_addr(&CPU::res<Operand::D, 6>); return;
        case 0xB3: immediate_addr(&CPU::res<Operand::E, 6>); return;
        case 0xB4: immediate_addr(&CPU::res<Operand::H, 6>); return;
        case 0xB5: immediate_addr(&CPU::res<Operand::L, 6>); return;
        case 0xB6: read_modify_write(&CPU::res<Operand::Memory, 6>); return;
        case 0xB7: immediate_addr(&CPU::res<Operand::A, 6>); return;
        case 0xB8: immediate_addr(&CPU::res<Operand::B, 7>); return;
        case 0xB9: immediate_addr(&CPU::res<Operand::C, 7>); return;
        case 0xBA: immediate_addr(&CPU::res<Operand::D, 7>); return;
        case 0xBB: immediate_addr(&CPU::res<Operand::E, 7>); return;
        case 0xBC: immediate_addr(&CPU::res<Operand::H, 7>); return;
        case 0xBD: immediate_addr(&CPU::res<Operand::L, 7>); return;
        case 0xBE: read_modify_write(&CPU::res<Operand::Memory, 7>); return;
        case 0xBF: immediate_addr(&CPU::res<Operand::A, 7>); return;

        case 0xC0: immediate_addr(&CPU::set<Operand::B, 0>); return;
        case 0xC1: immediate_addr(&CPU::set<Operand::C, 0>); return;
        case 0xC2: immediate_addr(&CPU::set<Operand::D, 0>); return;
        case 0xC3: immediate_addr(&CPU::set<Operand::E, 0>); return;
        case 0xC4: immediate_addr(&CPU::set<Operand::H, 0>); return;
        case 0xC5: immediate_addr(&CPU::set<Operand::L, 0>); return;
        case 0xC6: read_modify_write(&CPU::set<Operand::Memory, 0>); return;
        case 0xC7: immediate_addr(&CPU::set<Operand::A, 0>); return;
        case 0xC8: immediate_addr(&CPU::set<Operand::B, 1>); return;
        case 0xC9: immediate_addr(&CPU::set<Operand::C, 1>); return;
        case 0xCA: immediate_addr(&CPU::set<Operand::D, 1>); return;
        case 0xCB: immediate_addr(&CPU::set<Operand::E, 1>); return;
        case 0xCC: immediate_addr(&CPU::set<Operand::H, 1>); return;
        case 0xCD: immediate_addr(&CPU::set<Operand::L, 1>); return;
        case 0xCE: read_modify_write(&CPU::set<Operand::Memory, 1>); return;
        case 0xCF: immediate_addr(&CPU::set<Operand::A, 1>); return;

        case 0xD0: immediate_addr(&CPU::set<Operand::B, 2>); return;
        case 0xD1: immediate_addr(&CPU::set<Operand::C, 2>); return;
        case 0xD2: immediate_addr(&CPU::set<Operand::D, 2>); return;
        case 0xD3: immediate_addr(&CPU::set<Operand::E, 2>); return;
        case 0xD4: immediate_addr(&CPU::set<Operand::H, 2>); return;
        case 0xD5: immediate_addr(&CPU::set<Operand::L, 2>); return;
        case 0xD6: read_modify_write(&CPU::set<Operand::Memory, 2>); return;
        case 0xD7: immediate_addr(&CPU::set<Operand::A, 2>); return;
        case 0xD8: immediate_addr(&CPU::set<Operand::B, 3>); return;
        case 0xD9: immediate_addr(&CPU::set<Operand::C, 3>); return;
        case 0xDA: immediate_addr(&CPU::set<Operand::D, 3>); return;
        case 0xDB: immediate_addr(&CPU::set<Operand::E, 3>); return;
        case 0xDC: immediate_addr(&CPU::set<Operand::H, 3>); return;
        case 0xDD: immediate_addr(&CPU::set<Operand::L, 3>); return;
        case 0xDE: read_modify_write(&CPU::set<Operand::Memory, 3>); return;
        case 0xDF: immediate_addr(&CPU::set<Operand::A, 3>); return;

        case 0xE0: immediate_addr(&CPU::set<Operand::B, 4>); return;
        case 0xE1: immediate_addr(&CPU::set<Operand::C, 4>); return;
        case 0xE2: immediate_addr(&CPU::set<Operand::D, 4>); return;
        case 0xE3: immediate_addr(&CPU::set<Operand::E, 4>); return;
        case 0xE4: immediate_addr(&CPU::set<Operand::H, 4>); return;
        case 0xE5: immediate_addr(&CPU::set<Operand::L, 4>); return;
        case 0xE6: read_modify_write(&CPU::set<Operand::Memory, 4>); return;
        case 0xE7: immediate_addr(&CPU::set<Operand::A, 4>); return;
        case 0xE8: immediate_addr(&CPU::set<Operand::B, 5>); return;
        case 0xE9: immediate_addr(&CPU::set<Operand::C, 5>); return;
        case 0xEA: immediate_addr(&CPU::set<Operand::D, 5>); return;
        case 0xEB: immediate_addr(&CPU::set<Operand::E, 5>); return;
        case 0xEC: immediate_addr(&CPU::set<Operand::H, 5>); return;
        case 0xED: immediate_addr(&CPU::set<Operand::L, 5>); return;
        case 0xEE: read_modify_write(&CPU::set<Operand::Memory, 5>); return;
        case 0xEF: immediate_addr(&CPU::set<Operand::A, 5>); return;

        case 0xF0: immediate_addr(&CPU::set<Operand::B, 6>); return;
        case 0xF1: immediate_addr(&CPU::set<Operand::C, 6>); return;
        case 0xF2: immediate_addr(&CPU::set<Operand::D, 6>); return;
        case 0xF3: immediate_addr(&CPU::set<Operand::E, 6>); return;
        case 0xF4: immediate_addr(&CPU::set<Operand::H, 6>); return;
        case 0xF5: immediate_addr(&CPU::set<Operand::L, 6>); return;
        case 0xF6: read_modify_write(&CPU::set<Operand::Memory, 6>); return;
        case 0xF7: immediate_addr(&CPU::set<Operand::A, 6>); return;
        case 0xF8: immediate_addr(&CPU::set<Operand::B, 7>); return;
        case 0xF9: immediate_addr(&CPU::set<Operand::C, 7>); return;
        case 0xFA: immediate_addr(&CPU::set<Operand::D, 7>); return;
        case 0xFB: immediate_addr(&CPU::set<Operand::E, 7>); return;
        case 0xFC: immediate_addr(&CPU::set<Operand::H, 7>); return;
        case 0xFD: immediate_addr(&CPU::set<Operand::L, 7>); return;
        case 0xFE: read_modify_write(&CPU::set<Operand::Memory, 7>); return;
        case 0xFF: immediate_addr(&CPU::set<Operand::A, 7>); return;
        default:;
        }
    }

} // GB