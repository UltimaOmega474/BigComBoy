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
        case 0x03: adjust_rp<RegisterPair::BC, 1>(); return;
        case 0x04: immediate_addr(&CPU::inc_r<COperand2::B>); return;
        case 0x05: immediate_addr(&CPU::dec_r<COperand2::B>); return;
        case 0x06: ld_immediate<COperand2::B>(); return;
        case 0x07: rlca(); return;
        case 0x08: ld_direct_sp(); return;
        case 0x09: add_hl_rp<RegisterPair::BC>(); return;
        case 0x0A: ld_accumulator_indirect<COperand3::BC>(); return;
        case 0x0B: adjust_rp<RegisterPair::BC, -1>(); return;
        case 0x0C: immediate_addr(&CPU::inc_r<COperand2::C>); return;
        case 0x0D: immediate_addr(&CPU::dec_r<COperand2::C>); return;
        case 0x0E: ld_immediate<COperand2::C>(); return;
        case 0x0F: rrca(); return;

        case 0x10: stop(); return;
        case 0x11: ld_rp_immediate<RegisterPair::DE>(); return;
        case 0x12: ld_indirect_accumulator<COperand3::DE>(); return;
        case 0x13: adjust_rp<RegisterPair::DE, 1>(); return;
        case 0x14: immediate_addr(&CPU::inc_r<COperand2::D>); return;
        case 0x15: immediate_addr(&CPU::dec_r<COperand2::D>); return;
        case 0x16: ld_immediate<COperand2::D>(); return;
        case 0x17: rla(); return;
        case 0x18: jr<ConditionCode::Always, false>(); return;
        case 0x19: add_hl_rp<RegisterPair::DE>(); return;
        case 0x1A: ld_accumulator_indirect<COperand3::DE>(); return;
        case 0x1B: adjust_rp<RegisterPair::DE, -1>(); return;
        case 0x1C: immediate_addr(&CPU::inc_r<COperand2::E>); return;
        case 0x1D: immediate_addr(&CPU::dec_r<COperand2::E>); return;
        case 0x1E: ld_immediate<COperand2::E>(); return;
        case 0x1F: rra(); return;

        case 0x20: jr<ConditionCode::IfZero, false>(); return;
        case 0x21: ld_rp_immediate<RegisterPair::HL>(); return;
        case 0x22: ld_indirect_accumulator<COperand3::HLIncrement>(); return;
        case 0x23: adjust_rp<RegisterPair::HL, 1>(); return;
        case 0x24: immediate_addr(&CPU::inc_r<COperand2::H>); return;
        case 0x25: immediate_addr(&CPU::dec_r<COperand2::H>); return;
        case 0x26: ld_immediate<COperand2::H>(); return;
        case 0x27: daa(); return;
        case 0x28: jr<ConditionCode::IfZero, true>(); return;
        case 0x29: add_hl_rp<RegisterPair::HL>(); return;
        case 0x2A: ld_accumulator_indirect<COperand3::HLIncrement>(); return;
        case 0x2B: adjust_rp<RegisterPair::HL, -1>(); return;
        case 0x2C: immediate_addr(&CPU::inc_r<COperand2::L>); return;
        case 0x2D: immediate_addr(&CPU::dec_r<COperand2::L>); return;
        case 0x2E: ld_immediate<COperand2::L>(); return;
        case 0x2F: cpl(); return;

        case 0x30: jr<ConditionCode::IfCarry, false>(); return;
        case 0x31: ld_rp_immediate<RegisterPair::SP>(); return;
        case 0x32: ld_indirect_accumulator<COperand3::HLDecrement>(); return;
        case 0x33: adjust_rp<RegisterPair::SP, 1>(); return;
        case 0x34: read_modify_write(&CPU::inc_r<COperand2::Memory>); return;
        case 0x35: read_modify_write(&CPU::dec_r<COperand2::Memory>); return;
        case 0x36: ld_immediate<COperand2::Memory>(); return;
        case 0x37: scf(); return;
        case 0x38: jr<ConditionCode::IfCarry, true>(); return;
        case 0x39: add_hl_rp<RegisterPair::SP>(); return;
        case 0x3A: ld_accumulator_indirect<COperand3::HLDecrement>(); return;
        case 0x3B: adjust_rp<RegisterPair::SP, -1>(); return;
        case 0x3C: immediate_addr(&CPU::inc_r<COperand2::A>); return;
        case 0x3D: immediate_addr(&CPU::dec_r<COperand2::A>); return;
        case 0x3E: ld_immediate<COperand2::A>(); return;
        case 0x3F: ccf(); return;

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

        case 0x80: immediate_addr(&CPU::adc<COperand2::B, false>); return;
        case 0x81: immediate_addr(&CPU::adc<COperand2::C, false>); return;
        case 0x82: immediate_addr(&CPU::adc<COperand2::D, false>); return;
        case 0x83: immediate_addr(&CPU::adc<COperand2::E, false>); return;
        case 0x84: immediate_addr(&CPU::adc<COperand2::H, false>); return;
        case 0x85: immediate_addr(&CPU::adc<COperand2::L, false>); return;
        case 0x86: mem_read_addr<MemRead::HL>(&CPU::adc<COperand2::Memory, false>); return;
        case 0x87: immediate_addr(&CPU::adc<COperand2::A, false>); return;
        case 0x88: immediate_addr(&CPU::adc<COperand2::B, true>); return;
        case 0x89: immediate_addr(&CPU::adc<COperand2::C, true>); return;
        case 0x8A: immediate_addr(&CPU::adc<COperand2::D, true>); return;
        case 0x8B: immediate_addr(&CPU::adc<COperand2::E, true>); return;
        case 0x8C: immediate_addr(&CPU::adc<COperand2::H, true>); return;
        case 0x8D: immediate_addr(&CPU::adc<COperand2::L, true>); return;
        case 0x8E: mem_read_addr<MemRead::HL>(&CPU::adc<COperand2::Memory, true>); return;
        case 0x8F: immediate_addr(&CPU::adc<COperand2::A, true>); return;

        case 0x90: immediate_addr(&CPU::sbc<COperand2::B, false>); return;
        case 0x91: immediate_addr(&CPU::sbc<COperand2::C, false>); return;
        case 0x92: immediate_addr(&CPU::sbc<COperand2::D, false>); return;
        case 0x93: immediate_addr(&CPU::sbc<COperand2::E, false>); return;
        case 0x94: immediate_addr(&CPU::sbc<COperand2::H, false>); return;
        case 0x95: immediate_addr(&CPU::sbc<COperand2::L, false>); return;
        case 0x96: mem_read_addr<MemRead::HL>(&CPU::sbc<COperand2::Memory, false>); return;
        case 0x97: immediate_addr(&CPU::sbc<COperand2::A, false>); return;
        case 0x98: immediate_addr(&CPU::sbc<COperand2::B, true>); return;
        case 0x99: immediate_addr(&CPU::sbc<COperand2::C, true>); return;
        case 0x9A: immediate_addr(&CPU::sbc<COperand2::D, true>); return;
        case 0x9B: immediate_addr(&CPU::sbc<COperand2::E, true>); return;
        case 0x9C: immediate_addr(&CPU::sbc<COperand2::H, true>); return;
        case 0x9D: immediate_addr(&CPU::sbc<COperand2::L, true>); return;
        case 0x9E: mem_read_addr<MemRead::HL>(&CPU::sbc<COperand2::Memory, true>); return;
        case 0x9F: immediate_addr(&CPU::sbc<COperand2::A, true>); return;

        case 0xA0: immediate_addr(&CPU::and_op<COperand2::B>); return;
        case 0xA1: immediate_addr(&CPU::and_op<COperand2::C>); return;
        case 0xA2: immediate_addr(&CPU::and_op<COperand2::D>); return;
        case 0xA3: immediate_addr(&CPU::and_op<COperand2::E>); return;
        case 0xA4: immediate_addr(&CPU::and_op<COperand2::H>); return;
        case 0xA5: immediate_addr(&CPU::and_op<COperand2::L>); return;
        case 0xA6: mem_read_addr<MemRead::HL>(&CPU::and_op<COperand2::Memory>); return;
        case 0xA7: immediate_addr(&CPU::and_op<COperand2::A>); return;
        case 0xA8: immediate_addr(&CPU::xor_op<COperand2::B>); return;
        case 0xA9: immediate_addr(&CPU::xor_op<COperand2::C>); return;
        case 0xAA: immediate_addr(&CPU::xor_op<COperand2::D>); return;
        case 0xAB: immediate_addr(&CPU::xor_op<COperand2::E>); return;
        case 0xAC: immediate_addr(&CPU::xor_op<COperand2::H>); return;
        case 0xAD: immediate_addr(&CPU::xor_op<COperand2::L>); return;
        case 0xAE: mem_read_addr<MemRead::HL>(&CPU::xor_op<COperand2::Memory>); return;
        case 0xAF: immediate_addr(&CPU::xor_op<COperand2::A>); return;

        case 0xB0: immediate_addr(&CPU::or_op<COperand2::B>); return;
        case 0xB1: immediate_addr(&CPU::or_op<COperand2::C>); return;
        case 0xB2: immediate_addr(&CPU::or_op<COperand2::D>); return;
        case 0xB3: immediate_addr(&CPU::or_op<COperand2::E>); return;
        case 0xB4: immediate_addr(&CPU::or_op<COperand2::H>); return;
        case 0xB5: immediate_addr(&CPU::or_op<COperand2::L>); return;
        case 0xB6: mem_read_addr<MemRead::HL>(&CPU::or_op<COperand2::Memory>); return;
        case 0xB7: immediate_addr(&CPU::or_op<COperand2::A>); return;
        case 0xB8: immediate_addr(&CPU::cp<COperand2::B>); return;
        case 0xB9: immediate_addr(&CPU::cp<COperand2::C>); return;
        case 0xBA: immediate_addr(&CPU::cp<COperand2::D>); return;
        case 0xBB: immediate_addr(&CPU::cp<COperand2::E>); return;
        case 0xBC: immediate_addr(&CPU::cp<COperand2::H>); return;
        case 0xBD: immediate_addr(&CPU::cp<COperand2::L>); return;
        case 0xBE: mem_read_addr<MemRead::HL>(&CPU::cp<COperand2::Memory>); return;
        case 0xBF: immediate_addr(&CPU::cp<COperand2::A>); return;

        case 0xC0: ret_cc<ConditionCode::IfZero, false>(); return;
        case 0xC2: jp<ConditionCode::IfZero, false, false>(); return;
        case 0xC3: jp<ConditionCode::Always, false, false>(); return;
        case 0xC7: rst(); return;
        case 0xC8: ret_cc<ConditionCode::IfZero, true>(); return;
        case 0xC9: ret<false>(); return;
        case 0xC4: call<ConditionCode::IfZero, false>(); return;
        case 0xCA: jp<ConditionCode::IfZero, true, false>(); return;
        case 0xC6: mem_read_addr<MemRead::PC>(&CPU::adc<COperand2::Memory, false>); return;
        case 0xCC: call<ConditionCode::IfZero, true>(); return;
        case 0xCD: call<ConditionCode::Always, false>(); return;
        case 0xCE: mem_read_addr<MemRead::PC>(&CPU::adc<COperand2::Memory, true>); return;
        case 0xCF: rst(); return;

        case 0xD0: ret_cc<ConditionCode::IfCarry, false>(); return;
        case 0xD2: jp<ConditionCode::IfCarry, false, false>(); return;
        case 0xD3: illegal(); return;
        case 0xD4: call<ConditionCode::IfCarry, false>(); return;
        case 0xD6: mem_read_addr<MemRead::PC>(&CPU::sbc<COperand2::Memory, false>); return;
        case 0xD7: rst(); return;
        case 0xD8: ret_cc<ConditionCode::IfCarry, true>(); return;
        case 0xD9: ret<true>(); return;
        case 0xDA: jp<ConditionCode::IfCarry, true, false>(); return;
        case 0xDB: illegal(); return;
        case 0xDC: call<ConditionCode::IfCarry, true>(); return;
        case 0xDD: illegal(); return;
        case 0xDE: mem_read_addr<MemRead::PC>(&CPU::sbc<COperand2::Memory, true>); return;
        case 0xDF: rst(); return;

        case 0xE0: ldh_offset_a(); return;
        case 0xE2: ldh_c_a(); return;
        case 0xE3:
        case 0xE4: illegal(); return;
        case 0xE6: mem_read_addr<MemRead::PC>(&CPU::and_op<COperand2::Memory>); return;
        case 0xE7: rst(); return;
        case 0xE8: add_sp_i8(); return;
        case 0xE9: jp<ConditionCode::Always, false, true>(); return;
        case 0xEA: ld_direct_a(); return;
        case 0xEB:
        case 0xEC:
        case 0xED: illegal(); return;
        case 0xEE: mem_read_addr<MemRead::PC>(&CPU::xor_op<COperand2::Memory>); return;
        case 0xEF: rst(); return;

        case 0xF0: ldh_a_offset(); return;
        case 0xF2: ldh_a_c(); return;
        case 0xF3: di(); return;
        case 0xF4: illegal(); return;
        case 0xF6: mem_read_addr<MemRead::PC>(&CPU::or_op<COperand2::Memory>); return;
        case 0xF7: rst(); return;
        case 0xF8: ld_hl_sp_i8(); return;
        case 0xF9: ld_sp_hl(); return;
        case 0xFA: ld_a_direct(); return;
        case 0xFB: ei(); return;
        case 0xFC:
        case 0xFD: illegal(); return;
        case 0xFE: mem_read_addr<MemRead::PC>(&CPU::cp<COperand2::Memory>); return;
        case 0xFF: rst(); return;

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
        case COperand2::Memory: return z;
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
        case COperand2::Memory: z = value; return;
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
    auto CPU::stop() -> void {
        m_cycle++;
        // TODO: implement this again
    }

    auto CPU::halt() -> void {
        m_cycle++;
        // TODO: implement this again
    }
    auto CPU::illegal() -> void {
        m_cycle = 2;
        // never fetches
    }

    auto CPU::ei() -> void {
        // TODO: implement this again
    }

    auto CPU::di() -> void {
        // TODO: implement this again
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
            w = (z & 0x80) ? 255 : 0;

            const int32_t spl = (stack_pointer & 0xFF);
            const int32_t result = z + spl;

            alu_flags.hc = ((z & 0xF) + (spl & 0xF)) > 0xF;
            alu_flags.cy = result > 0xFF;
            alu_flags.n = false;
            alu_flags.z = false;
            z = static_cast<uint8_t>(result & 0xFF);

            break;
        }
        case 4: {
            w += static_cast<uint8_t>(stack_pointer >> 8) + static_cast<uint8_t>(alu_flags.cy);
            set_rp(RegisterPair::HL, (w << 8) | z);
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

    auto CPU::add_sp_i8() -> void {
        m_cycle++;

        switch (m_cycle) {
        case 2: {
            z = bus_read_fn(program_counter++);
            break;
        }
        case 3: {
            w = (z & 0x80) ? 255 : 0;

            const int32_t spl = (stack_pointer & 0xFF);
            const int32_t result = z + spl;

            alu_flags.hc = ((z & 0xF) + (spl & 0xF)) > 0xF;
            alu_flags.cy = result > 0xFF;
            alu_flags.n = false;
            alu_flags.z = false;
            z = static_cast<uint8_t>(result & 0xFF);
            break;
        }
        case 4: {
            w += static_cast<uint8_t>(stack_pointer >> 8) + static_cast<uint8_t>(alu_flags.cy);
            break;
        }
        case 5: {
            stack_pointer = (w << 8) | z;
            fetch(program_counter);
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

            alu_flags.hc = ((left & 0xF) + (right & 0xF) + (cy & 0xF)) > 0xF;
            alu_flags.cy = result > 0xFF;
            alu_flags.n = false;
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
            h = add_8bit(h, rp >> 8, alu_flags.cy);
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    template <COperand2 operand, bool with_carry> auto CPU::adc() -> void {
        const int32_t left = a;
        const int32_t right = (operand == COperand2::Memory) ? z : get_register(operand);

        const int32_t cy = with_carry ? alu_flags.cy : 0;
        const int32_t result = left + right + cy;
        const auto masked_result = static_cast<uint8_t>(result & 0xFF);

        alu_flags.hc = ((left & 0xF) + (right & 0xF) + (cy & 0xF)) > 0xF;
        alu_flags.cy = result > 0xFF;
        alu_flags.z = masked_result == 0;
        alu_flags.n = false;

        a = masked_result;
    }

    template <COperand2 operand, bool with_carry> auto CPU::sbc() -> void {
        const int32_t left = a;
        const int32_t right = operand == COperand2::Memory ? z : get_register(operand);

        const int32_t cy = with_carry ? alu_flags.cy : 0;
        const int32_t result = left - right - cy;
        const auto masked_result = static_cast<uint8_t>(result & 0xFF);

        alu_flags.hc = ((left & 0xF) - (right & 0xF) - (cy & 0xF)) < 0;
        alu_flags.cy = result < 0;
        alu_flags.z = masked_result == 0;
        alu_flags.n = true;

        a = masked_result;
    }

    template <COperand2 operand> auto CPU::and_op() -> void {
        const int32_t left = a;
        const int32_t right = operand == COperand2::Memory ? z : get_register(operand);
        const int32_t result = left & right;

        alu_flags.hc = true;
        alu_flags.cy = false;
        alu_flags.z = result == 0;
        alu_flags.n = false;

        a = static_cast<uint8_t>(result & 0xFF);
    }

    template <COperand2 operand> auto CPU::xor_op() -> void {
        const int32_t left = a;
        const int32_t right = operand == COperand2::Memory ? z : get_register(operand);
        const int32_t result = left ^ right;

        alu_flags.hc = false;
        alu_flags.cy = false;
        alu_flags.z = result == 0;
        alu_flags.n = false;

        a = static_cast<uint8_t>(result & 0xFF);
    }

    template <COperand2 operand> auto CPU::or_op() -> void {
        const int32_t left = a;
        const int32_t right = operand == COperand2::Memory ? z : get_register(operand);
        const int32_t result = left | right;

        alu_flags.hc = false;
        alu_flags.cy = false;
        alu_flags.z = result == 0;
        alu_flags.n = false;

        a = static_cast<uint8_t>(result & 0xFF);
    }

    template <COperand2 operand> auto CPU::cp() -> void {
        const uint8_t temp = a;
        sbc<operand, false>();
        a = temp;
    }

    template <COperand2 operand> auto CPU::inc_r() -> void {
        auto op = operand;
        const int32_t left = (operand == COperand2::Memory) ? z : get_register(operand);
        constexpr int32_t right = 1;

        const int32_t result = left + right;
        const auto masked_result = static_cast<uint8_t>(result & 0xFF);

        alu_flags.hc = ((left & 0xF) + (right & 0xF)) > 0xF;
        alu_flags.z = masked_result == 0;
        alu_flags.n = false;
        set_register(operand, masked_result);
    }

    template <COperand2 operand> auto CPU::dec_r() -> void {
        const int32_t left = (operand == COperand2::Memory) ? z : get_register(operand);
        constexpr int32_t right = 1;

        const int32_t result = left - right;
        const auto masked_result = static_cast<uint8_t>(result & 0xFF);

        alu_flags.hc = ((left & 0xF) - (right & 0xF)) < 0;
        alu_flags.z = masked_result == 0;
        alu_flags.n = true;
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
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    auto CPU::rlca() -> void {
        m_cycle++;
        int32_t temp = a;
        const int32_t bit7 = (temp & 0x80) >> 7;

        alu_flags.cy = static_cast<bool>(bit7);
        alu_flags.z = false;
        alu_flags.n = false;
        alu_flags.hc = false;

        temp = (temp << 1) | bit7;
        a = static_cast<uint8_t>(temp & 0xFF);
        fetch(program_counter);
    }

    auto CPU::rrca() -> void {
        m_cycle++;
        int32_t temp = a;
        const int32_t bit0 = (temp & 0x01);

        alu_flags.cy = static_cast<bool>(bit0);
        alu_flags.z = false;
        alu_flags.n = false;
        alu_flags.hc = false;

        temp = (temp >> 1) | (bit0 << 7);
        a = static_cast<uint8_t>(temp & 0xFF);
        fetch(program_counter);
    }

    auto CPU::rla() -> void {
        m_cycle++;
        const auto cy = static_cast<uint8_t>(alu_flags.cy);

        alu_flags.cy = static_cast<bool>(a & 0x80);
        alu_flags.z = false;
        alu_flags.n = false;
        alu_flags.hc = false;

        a = (a << 1) | cy;
        fetch(program_counter);
    }

    auto CPU::rra() -> void {
        m_cycle++;
        const uint8_t cy = static_cast<uint8_t>(alu_flags.cy) << 7;

        alu_flags.cy = static_cast<bool>(a & 0x01);
        alu_flags.z = false;
        alu_flags.n = false;
        alu_flags.hc = false;

        a = (a >> 1) | cy;
        fetch(program_counter);
    }

    auto CPU::daa() -> void {
        // Implementation adapted from: https://ehaskins.com/2018-01-30%20Z80%20DAA/
        m_cycle++;
        uint8_t correct = 0;
        auto temp = static_cast<uint16_t>(a);
        bool cy = false;

        if (alu_flags.hc || (!alu_flags.n && (temp & 0xF) > 9)) {
            correct |= 6;
        }

        if (alu_flags.cy || (!alu_flags.n && temp > 0x99)) {
            correct |= 0x60;
            cy = true;
        }

        temp += alu_flags.n ? -correct : correct;
        temp &= 0xFF;

        alu_flags.z = temp == 0;
        alu_flags.hc = false;
        alu_flags.cy = cy;

        a = static_cast<uint8_t>(temp);
        fetch(program_counter);
    }

    auto CPU::cpl() -> void {
        m_cycle++;
        a = ~a;
        alu_flags.n = true;
        alu_flags.hc = true;
        fetch(program_counter);
    }

    auto CPU::scf() -> void {
        m_cycle++;
        alu_flags.n = false;
        alu_flags.hc = false;
        alu_flags.cy = true;
        fetch(program_counter);
    }

    auto CPU::ccf() -> void {
        m_cycle++;
        alu_flags.n = false;
        alu_flags.hc = false;
        alu_flags.cy = !alu_flags.cy;
        fetch(program_counter);
    }

    template <ConditionCode cc, bool is_set> auto CPU::jr() -> void {
        m_cycle++;

        auto calc_address = [this]() {
            const int32_t pcl = (program_counter & 0xFF);
            const int32_t result = z + pcl;
            const bool cy = result > 0xFF;

            w = (z & 0x80) ? 255 : 0;
            z = static_cast<uint8_t>(result & 0xFF);
            w += static_cast<uint8_t>(program_counter >> 8) + static_cast<uint8_t>(cy);
        };

        switch (m_cycle) {
        case 2: {
            z = bus_read_fn(program_counter++);
            break;
        }
        case 3: {
            if constexpr (cc == ConditionCode::Always) {
                calc_address();
            }

            if constexpr (cc == ConditionCode::IfZero) {
                if (alu_flags.z == is_set) {
                    calc_address();
                } else {
                    fetch(program_counter);
                }
            }

            if constexpr (cc == ConditionCode::IfCarry) {
                if (alu_flags.cy == is_set) {
                    calc_address();
                } else {
                    fetch(program_counter);
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

    template <ConditionCode cc, bool is_set, bool from_hl> auto CPU::jp() -> void {
        m_cycle++;

        if constexpr (from_hl) {
            fetch(get_rp(RegisterPair::HL));
        } else {
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
                if constexpr (cc == ConditionCode::Always) {
                    program_counter = (w << 8) | z;
                }

                if constexpr (cc == ConditionCode::IfZero) {
                    if (alu_flags.z == is_set) {
                        program_counter = (w << 8) | z;
                    } else {
                        fetch(program_counter);
                    }
                }

                if constexpr (cc == ConditionCode::IfCarry) {
                    if (alu_flags.cy == is_set) {
                        program_counter = (w << 8) | z;
                    } else {
                        fetch(program_counter);
                    }
                }

                break;
            }
            case 5: {
                fetch(program_counter);
                break;
            }

            default:;
            }
        }
    }

    template <ConditionCode cc, bool is_set> auto CPU::call() -> void {
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
            if constexpr (cc == ConditionCode::Always) {
                stack_pointer--;
            }

            if constexpr (cc == ConditionCode::IfZero) {
                if (alu_flags.z == is_set) {
                    stack_pointer--;
                } else {
                    fetch(program_counter);
                }
            }

            if constexpr (cc == ConditionCode::IfCarry) {
                if (alu_flags.cy == is_set) {
                    stack_pointer--;
                } else {
                    fetch(program_counter);
                }
            }

            break;
        }
        case 5: {
            bus_write_fn(stack_pointer--, program_counter >> 8);
            break;
        }
        case 6: {
            bus_write_fn(stack_pointer, program_counter & 0xFF);
            program_counter = (w << 8) | z;
            break;
        }
        case 7: {
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    template <bool enable_interrupts> auto CPU::ret() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = bus_read_fn(stack_pointer++);
            break;
        }
        case 3: {
            w = bus_read_fn(stack_pointer++);
            break;
        }
        case 4: {
            program_counter = (w << 8) | z;
            if constexpr (enable_interrupts) {
                master_interrupt_enable_ = true;
            }
            break;
        }
        case 5: {
            fetch(program_counter);
            break;
        }

        default:;
        }
    }

    template <ConditionCode cc, bool is_set> auto CPU::ret_cc() -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            if constexpr (cc == ConditionCode::IfZero) {
                if (alu_flags.z != is_set) {
                    m_cycle = 5;
                }
            }

            if constexpr (cc == ConditionCode::IfCarry) {
                if (alu_flags.cy != is_set) {
                    m_cycle = 5;
                }
            }

            break;
        }
        case 3: {
            z = bus_read_fn(stack_pointer++);
            break;
        }
        case 4: {
            w = bus_read_fn(stack_pointer++);
            break;
        }
        case 5: {
            program_counter = (w << 8) | z;
            break;
        }
        case 6: {
            fetch(program_counter);
            break;
        }

        default:;
        }
    }

    auto CPU::rst() -> void {
        m_cycle++;

        switch (m_cycle) {
        case 2: {
            stack_pointer--;
            break;
        }
        case 3: {
            bus_write_fn(stack_pointer--, program_counter >> 8);
            break;
        }
        case 4: {
            bus_write_fn(stack_pointer, program_counter & 0xFF);
            program_counter = ir & 0x38;
            break;
        }
        case 5: {
            fetch(program_counter);
            break;
        }

        default:;
        }
    }

    template <typename Fn> auto CPU::immediate_addr(Fn &&func) -> void {
        m_cycle++;
        (this->*func)();
        fetch(program_counter);
    }

    template <MemRead address, typename Fn> auto CPU::mem_read_addr(Fn &&func) -> void {
        m_cycle++;

        switch (m_cycle) {
        case 2: {
            if constexpr (address == MemRead::PC) {
                z = bus_read_fn(program_counter++);
            } else {
                z = bus_read_fn(get_rp(RegisterPair::HL));
            }
            break;
        }
        case 3: {
            (this->*func)();
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    template <typename Fn> auto CPU::mem_write_addr(Fn &&func) -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            bus_write_fn(get_rp(RegisterPair::HL), (this->*func)());
            break;
        }
        case 3: {
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

    template <typename Fn> auto CPU::read_modify_write(Fn &&func) -> void {
        m_cycle++;
        switch (m_cycle) {
        case 2: {
            z = bus_read_fn(get_rp(RegisterPair::HL));
            break;
        }
        case 3: {
            (this->*func)();
            bus_write_fn(get_rp(RegisterPair::HL), z);
            break;
        }
        case 4: {
            fetch(program_counter);
            break;
        }
        default:;
        }
    }

} // GB