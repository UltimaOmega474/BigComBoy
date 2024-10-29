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

#include "Timer.hpp"
#include "Core.hpp"
#include <array>

namespace GB {
    constexpr auto EdgeFell(const uint16_t previous, const uint16_t next, const uint16_t mask)
        -> bool {
        return (previous & mask) && (!(next & mask));
    }

    Timer::Timer(Core *core) : core(core) { reset(); }

    auto Timer::write_register(const uint8_t reg, const uint8_t value) -> void {
        switch (reg) {
        case 0x04: {
            reset_div();
            return;
        }
        case 0x05: {
            tima = value;
            return;
        }
        case 0x06: {
            tma = value;
            return;
        }
        case 0x07: {
            set_tac(value);
            return;
        }
        default:;
        }
    }

    auto Timer::read_register(const uint8_t reg) const -> uint8_t {
        switch (reg) {
        case 0x04: return read_div();
        case 0x05: return tima;
        case 0x06: return tma;
        case 0x07: return tac;
        default:;
        }
        return 0xFF;
    }

    auto Timer::reset() -> void {
        div_cycles = 0xAB00;
        tima = 0;
        tma = 0;
        set_tac(0xF8);
    }

    auto Timer::set_tac(const uint8_t rate) -> void {
        static constexpr std::array<uint16_t, 4> tac_table{
            512,
            8,
            32,
            128,
        };

        tac_rate = tac_table[rate & 0x3];
        tac = rate;
    }

    auto Timer::reset_div() -> void { change_div(0); }

    uint8_t Timer::read_div() const { return div_cycles >> 8; }

    auto Timer::clock(const int32_t cycles) -> void {
        const uint16_t new_div =
            (core->cpu.current_state() == ExecutionMode::Stopped) ? 0 : (div_cycles + cycles);
        change_div(new_div);
    }

    auto Timer::timer_enabled() const -> bool { return tac & 0b100; }

    auto Timer::change_div(const uint16_t new_div) -> void {
        if (EdgeFell(div_cycles >> 8, new_div >> 8,
                     core->cpu.double_speed() ? 0b100000 : 0b10000)) {
            core->apu.clock_frame_sequencer();
        }

        if (timer_enabled() && EdgeFell(div_cycles, new_div, tac_rate)) {
            ++tima;
            if (tima == 0) {
                tima = tma;
                core->cpu.request_interrupt(INT_TIMER_BIT);
            }
        }

        div_cycles = new_div;
    }
}
