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

namespace GB {
    auto CPU::force_next_opcode(const uint8_t opcode) -> void { ir = opcode; }

    auto CPU::flags() const -> uint8_t {
        uint8_t flags = alu_flags.cy << 4;
        flags |= alu_flags.hc << 5;
        flags |= alu_flags.n << 6;
        flags |= alu_flags.z << 7;
        return flags;
    }

    auto CPU::set_flags(uint8_t flags) -> void {
        flags = flags & 0xF0;
        alu_flags.cy = (flags >> 4) & 0x1;
        alu_flags.hc = (flags >> 5) & 0x1;
        alu_flags.n = (flags >> 6) & 0x1;
        alu_flags.z = (flags >> 7) & 0x1;
    }

    auto CPU::clock() -> void {
        switch (ir) {

        case 0x00: nop(); return;
        case 0x01: ld_rp_immediate<RegisterPair::BC>(); return;
        case 0x02: ld_indirect_accumulator<COperand3::BC>(); return;
        case 0x06: ld_immediate<COperand2::B>(); return;
        case 0x08: ld_direct_sp(); return;
        case 0x0A: ld_accumulator_indirect<COperand3::BC>(); return;
        case 0x0E: ld_immediate<COperand2::C>(); return;

        case 0x11: ld_rp_immediate<RegisterPair::DE>(); return;
        case 0x12: ld_indirect_accumulator<COperand3::DE>(); return;
        case 0x16: ld_immediate<COperand2::D>(); return;
        case 0x1A: ld_accumulator_indirect<COperand3::DE>(); return;
        case 0x1E: ld_immediate<COperand2::E>(); return;

        case 0x21: ld_rp_immediate<RegisterPair::HL>(); return;
        case 0x22: ld_indirect_accumulator<COperand3::HLIncrement>(); return;
        case 0x26: ld_immediate<COperand2::H>(); return;
        case 0x2A: ld_accumulator_indirect<COperand3::HLIncrement>(); return;
        case 0x2E: ld_immediate<COperand2::L>(); return;

        case 0x31: ld_rp_immediate<RegisterPair::SP>(); return;
        case 0x32: ld_indirect_accumulator<COperand3::HLDecrement>(); return;
        case 0x36: ld_immediate<COperand2::Memory>(); return;
        case 0x3A: ld_accumulator_indirect<COperand3::HLDecrement>(); return;
        case 0x3E: ld_immediate<COperand2::A>(); return;

        case 0x40: ld<COperand2::B, COperand2::B>(); return;
        case 0x41: ld<COperand2::B, COperand2::C>(); return;
        case 0x42: ld<COperand2::B, COperand2::D>(); return;
        case 0x43: ld<COperand2::B, COperand2::E>(); return;
        case 0x44: ld<COperand2::B, COperand2::H>(); return;
        case 0x45: ld<COperand2::B, COperand2::L>(); return;
        case 0x46: ld<COperand2::B, COperand2::Memory>(); return;
        case 0x47: ld<COperand2::B, COperand2::A>(); return;
        case 0x48: ld<COperand2::C, COperand2::B>(); return;
        case 0x49: ld<COperand2::C, COperand2::C>(); return;
        case 0x4A: ld<COperand2::C, COperand2::D>(); return;
        case 0x4B: ld<COperand2::C, COperand2::E>(); return;
        case 0x4C: ld<COperand2::C, COperand2::H>(); return;
        case 0x4D: ld<COperand2::C, COperand2::L>(); return;
        case 0x4E: ld<COperand2::C, COperand2::Memory>(); return;
        case 0x4F: ld<COperand2::C, COperand2::A>(); return;

        case 0x50: ld<COperand2::D, COperand2::B>(); return;
        case 0x51: ld<COperand2::D, COperand2::C>(); return;
        case 0x52: ld<COperand2::D, COperand2::D>(); return;
        case 0x53: ld<COperand2::D, COperand2::E>(); return;
        case 0x54: ld<COperand2::D, COperand2::H>(); return;
        case 0x55: ld<COperand2::D, COperand2::L>(); return;
        case 0x56: ld<COperand2::D, COperand2::Memory>(); return;
        case 0x57: ld<COperand2::D, COperand2::A>(); return;
        case 0x58: ld<COperand2::E, COperand2::B>(); return;
        case 0x59: ld<COperand2::E, COperand2::C>(); return;
        case 0x5A: ld<COperand2::E, COperand2::D>(); return;
        case 0x5B: ld<COperand2::E, COperand2::E>(); return;
        case 0x5C: ld<COperand2::E, COperand2::H>(); return;
        case 0x5D: ld<COperand2::E, COperand2::L>(); return;
        case 0x5E: ld<COperand2::E, COperand2::Memory>(); return;
        case 0x5F: ld<COperand2::E, COperand2::A>(); return;

        case 0x60: ld<COperand2::H, COperand2::B>(); return;
        case 0x61: ld<COperand2::H, COperand2::C>(); return;
        case 0x62: ld<COperand2::H, COperand2::D>(); return;
        case 0x63: ld<COperand2::H, COperand2::E>(); return;
        case 0x64: ld<COperand2::H, COperand2::H>(); return;
        case 0x65: ld<COperand2::H, COperand2::L>(); return;
        case 0x66: ld<COperand2::H, COperand2::Memory>(); return;
        case 0x67: ld<COperand2::H, COperand2::A>(); return;
        case 0x68: ld<COperand2::L, COperand2::B>(); return;
        case 0x69: ld<COperand2::L, COperand2::C>(); return;
        case 0x6A: ld<COperand2::L, COperand2::D>(); return;
        case 0x6B: ld<COperand2::L, COperand2::E>(); return;
        case 0x6C: ld<COperand2::L, COperand2::H>(); return;
        case 0x6D: ld<COperand2::L, COperand2::L>(); return;
        case 0x6E: ld<COperand2::L, COperand2::Memory>(); return;
        case 0x6F: ld<COperand2::L, COperand2::A>(); return;

        case 0x70: ld<COperand2::Memory, COperand2::B>(); return;
        case 0x71: ld<COperand2::Memory, COperand2::C>(); return;
        case 0x72: ld<COperand2::Memory, COperand2::D>(); return;
        case 0x73: ld<COperand2::Memory, COperand2::E>(); return;
        case 0x74: ld<COperand2::Memory, COperand2::H>(); return;
        case 0x75: ld<COperand2::Memory, COperand2::L>(); return;
        case 0x76: halt(); return;
        case 0x77: ld<COperand2::Memory, COperand2::A>(); return;
        case 0x78: ld<COperand2::A, COperand2::B>(); return;
        case 0x79: ld<COperand2::A, COperand2::C>(); return;
        case 0x7A: ld<COperand2::A, COperand2::D>(); return;
        case 0x7B: ld<COperand2::A, COperand2::E>(); return;
        case 0x7C: ld<COperand2::A, COperand2::H>(); return;
        case 0x7D: ld<COperand2::A, COperand2::L>(); return;
        case 0x7E: ld<COperand2::A, COperand2::Memory>(); return;
        case 0x7F: ld<COperand2::A, COperand2::A>(); return;

        case 0x80: add<COperand2::B, false>(); return;
        case 0x81: add<COperand2::C, false>(); return;
        case 0x82: add<COperand2::D, false>(); return;
        case 0x83: add<COperand2::E, false>(); return;
        case 0x84: add<COperand2::H, false>(); return;
        case 0x85: add<COperand2::L, false>(); return;
        case 0x86: add<COperand2::Memory, false>(); return;
        case 0x87: add<COperand2::A, false>(); return;
        case 0x88: add<COperand2::B, true>(); return;
        case 0x89: add<COperand2::C, true>(); return;
        case 0x8A: add<COperand2::D, true>(); return;
        case 0x8B: add<COperand2::E, true>(); return;
        case 0x8C: add<COperand2::H, true>(); return;
        case 0x8D: add<COperand2::L, true>(); return;
        case 0x8E: add<COperand2::Memory, true>(); return;
        case 0x8F: add<COperand2::A, true>(); return;

        case 0xE0: ldh_offset_a(); return;
        case 0xE2: ldh_c_a(); return;
        case 0xEA: ld_direct_a(); return;

        case 0xF0: ldh_a_offset(); return;
        case 0xF2: ldh_a_c(); return;
        case 0xF8: ld_hl_sp_i8(); return;
        case 0xF9: ld_sp_hl(); return;
        case 0xFA: ld_a_direct(); return;

        default: throw "Unknown opcode";
        }
    }

    auto CPU::get_rp(const RegisterPair index) const -> uint16_t {
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
            low = flags();

            break;
        }
        }

        return (high << 8) | (low);
    }

    auto CPU::set_rp(const RegisterPair index, const uint16_t temp) -> void {
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
            set_flags(static_cast<uint8_t>(temp & 0x00FF) & 0xF0);
            return;
        }
        }
    }

    auto CPU::get_register(const COperand2 reg) const -> uint8_t {
        switch (reg) {
        case COperand2::B: return b;
        case COperand2::C: return c;
        case COperand2::D: return d;
        case COperand2::E: return e;
        case COperand2::H: return h;
        case COperand2::L: return l;
        case COperand2::A: return a;
        default:;
        }
        return 0;
    }

    auto CPU::set_register(const COperand2 reg, const uint8_t value) -> void {
        switch (reg) {
        case COperand2::B: b = value; return;
        case COperand2::C: c = value; return;
        case COperand2::D: d = value; return;
        case COperand2::E: e = value; return;
        case COperand2::H: h = value; return;
        case COperand2::L: l = value; return;
        case COperand2::A: a = value; return;
        default:;
        }
    }

    auto CPU::fetch(const uint16_t where) -> void {
        ir = bus_read_fn(where);
        program_counter = where + 1;
        m_cycle = 1;
        // TODO: check interrupts here
    }
    auto CPU::nop() -> void {
        m_cycle++;
        fetch(program_counter);
    }

    auto CPU::halt() -> void {
        m_cycle++;
        fetch(program_counter);
    }

    template <COperand2 dst, COperand2 src> auto CPU::ld() -> void {
        m_cycle++;

        // ld (hl), reg
        if constexpr (dst == COperand2::Memory) {
            switch (m_cycle) {
            case 2: {
                bus_write_fn(get_rp(RegisterPair::HL), get_register(src));
                break;
            }
            case 3: {
                fetch(program_counter);
                break;
            }
            default:;
            }
            return;
        }

        // ld reg, (hl)
        if constexpr (src == COperand2::Memory) {
            switch (m_cycle) {
            case 2: {
                z = bus_read_fn(get_rp(RegisterPair::HL));
                break;
            }
            case 3: {
                set_register(dst, z);
                fetch(program_counter);
                break;
            }
            default:;
            }
            return;
        }

        // ld reg, reg
        const auto src_value = get_register(src);
        set_register(dst, src_value);
        fetch(program_counter);
    }

    auto CPU::op_ld_r_indirect(const uint16_t address) -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = bus_read_fn(address);
            break;
        }
        case 3: {
            set_register(static_cast<COperand2>((ir >> 3) & 0x7), z);
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    template <COperand2 reg> auto CPU::ld_immediate() -> void {
        m_cycle++;

        if constexpr (reg == COperand2::Memory) {
            switch (m_cycle) {
            case 2: {
                z = bus_read_fn(program_counter++);
                break;
            }
            case 3: {
                bus_write_fn(get_rp(RegisterPair::HL), z);
                break;
            }
            case 4: {
                fetch(program_counter);
                break;
            }
            default:;
            }
            return;
        }

        switch (m_cycle) {
        case 2: {
            z = bus_read_fn(program_counter++);
            break;
        }
        case 3: {
            set_register(reg, z);
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    auto CPU::op_ld_indirect_r(const uint16_t address) -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            const auto src_value = get_register(static_cast<COperand2>(ir & 0x7));
            bus_write_fn(address, src_value);
            break;
        }
        case 3: {
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    template <COperand3 rp> auto CPU::ld_indirect_accumulator() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            uint16_t address = 0;
            switch (rp) {
            case COperand3::BC: {
                address = get_rp(RegisterPair::BC);
                break;
            }
            case COperand3::DE: {
                address = get_rp(RegisterPair::DE);
                break;
            }
            case COperand3::HLIncrement: {
                address = get_rp(RegisterPair::HL);
                set_rp(RegisterPair::HL, address + 1);
                break;
            }
            case COperand3::HLDecrement: {
                address = get_rp(RegisterPair::HL);
                set_rp(RegisterPair::HL, address - 1);
                break;
            }
            }

            bus_write_fn(address, a);
            break;
        }
        case 3: {
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    template <COperand3 rp> auto CPU::ld_accumulator_indirect() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            uint16_t address = 0;
            switch (rp) {
            case COperand3::BC: {
                address = get_rp(RegisterPair::BC);
                break;
            }
            case COperand3::DE: {
                address = get_rp(RegisterPair::DE);
                break;
            }
            case COperand3::HLIncrement: {
                address = get_rp(RegisterPair::HL);
                set_rp(RegisterPair::HL, address + 1);
                break;
            }
            case COperand3::HLDecrement: {
                address = get_rp(RegisterPair::HL);
                set_rp(RegisterPair::HL, address - 1);
                break;
            }
            }
            a = bus_read_fn(address);
            break;
        }
        case 3: {
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    auto CPU::ld_direct_a() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = bus_read_fn(program_counter++);
            break;
        }
        case 3: {
            w = bus_read_fn(program_counter++);
            break;
        }
        case 4: {
            bus_write_fn(z | (w << 8), a);
            break;
        }
        case 5: {
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    auto CPU::ld_a_direct() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = bus_read_fn(program_counter++);
            break;
        }
        case 3: {
            w = bus_read_fn(program_counter++);
            break;
        }
        case 4: {
            a = bus_read_fn(z | (w << 8));
            break;
        }
        case 5: {
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    auto CPU::op_ld_indirect_immediate(const uint16_t address) -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = bus_read_fn(program_counter++);
            break;
        }
        case 3: {
            bus_write_fn(address, z);
            break;
        }
        case 4: {
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    template <RegisterPair rp> auto CPU::ld_rp_immediate() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = bus_read_fn(program_counter++);
            break;
        }
        case 3: {
            w = bus_read_fn(program_counter++);
            break;
        }
        case 4: {
            set_rp(rp, z | (w << 8));
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    auto CPU::ld_direct_sp() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = bus_read_fn(program_counter++);
            break;
        }
        case 3: {
            w = bus_read_fn(program_counter++);
            break;
        }
        case 4: {
            const auto address = z | (w << 8);
            bus_write_fn(address, stack_pointer & 0xFF);
            z++;
            break;
        }
        case 5: {
            const auto address = z | (w << 8);
            bus_write_fn(address, (stack_pointer >> 8) & 0xFF);
            break;
        }
        case 6: {
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    auto CPU::ld_hl_sp_i8() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = bus_read_fn(program_counter++);
            break;
        }
        case 3: {
            const uint16_t result = stack_pointer + static_cast<int8_t>(z);
            set_rp(RegisterPair::HL, result);
            alu_flags.hc = ((stack_pointer & 0xF) + (z & 0xF)) > 0xF;
            alu_flags.cy = ((stack_pointer & 0xFF) + (z & 0xFF)) > 0xFF;
            alu_flags.n = false;
            alu_flags.z = false;
            break;
        }
        case 4: {
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    auto CPU::ld_sp_hl() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            stack_pointer = get_rp(RegisterPair::HL);
            break;
        }
        case 3: {
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    auto CPU::ldh_offset_a() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = bus_read_fn(program_counter++);
            break;
        }
        case 3: {
            bus_write_fn(0xFF00 + z, a);
            break;
        }
        case 4: {
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    auto CPU::ldh_a_offset() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = bus_read_fn(program_counter++);
            break;
        }
        case 3: {
            a = bus_read_fn(0xFF00 + z);
            break;
        }
        case 4: {
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    auto CPU::ldh_c_a() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            bus_write_fn(0xFF00 + c, a);
            break;
        }
        case 3: {
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    auto CPU::ldh_a_c() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            a = bus_read_fn(0xFF00 + c);
            break;
        }
        case 3: {
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    template <COperand2 operand, bool with_carry> auto CPU::add() -> void {
        m_cycle++;
        auto do_add = [this](const uint8_t left, const uint8_t right, const uint8_t cy) -> uint8_t {
            const int32_t result = left + right + cy;
            const auto masked_result = static_cast<uint8_t>(result & 0xFF);

            alu_flags.hc = ((left & 0xF) + (right & 0xF) + (cy & 0xF)) > 0xF;
            alu_flags.cy = result > 0xFF;
            alu_flags.z = masked_result == 0;
            alu_flags.n = false;
            return masked_result;
        };

        switch (m_cycle) {
        case 2: {
            if constexpr (operand == COperand2::Memory) {
                z = bus_read_fn(get_rp(RegisterPair::HL));
            } else {
                a = do_add(a, get_register(operand),
                           with_carry ? static_cast<uint8_t>(alu_flags.cy) : 0);
                fetch(program_counter);
            }
            break;
        }
        case 3: {
            a = do_add(a, z, with_carry ? static_cast<uint8_t>(alu_flags.cy) : 0);
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

} // GB