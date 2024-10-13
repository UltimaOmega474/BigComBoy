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
    auto CPU::force_next_opcode(uint8_t opcode) -> void { ir = opcode; }

    auto CPU::flags() const -> uint8_t { return f; }

    auto CPU::set_flags(const uint8_t flags) -> void { f = flags & 0xF0; }

    auto CPU::clock() -> void {
        switch (ir) {
            // clang-format off

            // LD RP, Immediate
        case 0x01: case 0x11: case 0x21: case 0x31: op_ld_rp_immediate(); return;
            // LD (Indirect RP), A
        case 0x02: op_ld_indirect_rp_a<COperand3::BC>(); return;
        case 0x12: op_ld_indirect_rp_a<COperand3::DE>(); return;
        case 0x22: op_ld_indirect_rp_a<COperand3::HLIncrement>(); return;
        case 0x32: op_ld_indirect_rp_a<COperand3::HLDecrement>(); return;
            // LD R, Immediate
        case 0x06: case 0x16: case 0x26: case 0x0E:
        case 0x1E: case 0x2E: case 0x3E: op_ld_r_immediate(); return;
            // LD A, (Indirect RP)
        case 0x0A: op_ld_a_indirect_rp<COperand3::BC>(); return;
        case 0x1A: op_ld_a_indirect_rp<COperand3::DE>(); return;
        case 0x2A: op_ld_a_indirect_rp<COperand3::HLIncrement>(); return;
        case 0x3A: op_ld_a_indirect_rp<COperand3::HLDecrement>(); return;
            // LD (HL), Immediate
        case 0x36: op_ld_indirect_immediate(get_rp(RegisterPair::HL)); return;
            // LD B, SRC
        case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x47:
            // LD C, SRC
        case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4F:
            // LD D, SRC
        case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x57:
            // LD E, SRC
        case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5F:
            // LD H, SRC
        case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x67:
            // LD L, SRC
        case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6F:
            // LD A, SRC
        case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7F:
            op_ld_r_r(); return;
            // LD R, (HL)
        case 0x46: case 0x56: case 0x66: case 0x4E: case 0x5E: case 0x6E: case 0x7E:
            op_ld_r_indirect(get_rp(RegisterPair::HL)); return;
            // LD (HL), SRC
        case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77:
            op_ld_indirect_r(get_rp(RegisterPair::HL)); return;

            // clang-format on
        default:
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
            low = f;
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
            f = static_cast<uint8_t>(temp & 0x00FF) & 0xF0;
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
        default:
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
        default:
        }
    }

    auto CPU::fetch(const uint16_t where) -> void {
        ir = bus_read_fn(where);
        program_counter = where + 1;
        m_cycle = 1;
        // TODO: check interrupts here
    }

    auto CPU::op_ld_r_r() -> void {
        m_cycle++;
        const auto src_value = get_register(static_cast<COperand2>(ir & 0x7));
        set_register(static_cast<COperand2>((ir >> 3) & 0x7), src_value);
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
        default:
        }
    }
    auto CPU::op_ld_r_immediate() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = bus_read_fn(program_counter++);
            break;
        }
        case 3: {
            set_register(static_cast<COperand2>((ir >> 3) & 0x7), z);
            fetch(program_counter);
            break;
        }
        default:
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
        default:
        }
    }

    template <COperand3 rp> auto CPU::op_ld_indirect_rp_a() -> void {
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
        default:
        }
    }

    template <COperand3 rp> auto CPU::op_ld_a_indirect_rp() -> void {
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
        default:
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
        default:
        }
    }

    auto CPU::op_ld_rp_immediate() -> void {
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
            set_rp(static_cast<RegisterPair>((ir >> 4) & 0x3), z | (w << 8));
            fetch(program_counter);
            break;
        }
        default:
        }
    }

} // GB