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
#include <stdexcept>

namespace GB {
    constexpr bool EdgeFell(uint16_t previous, uint16_t next, uint16_t mask) {
        return (previous & mask) && (!(next & mask));
    }

    Timer::Timer(Core *core) : core(core) {
        if (!core) {
            throw std::invalid_argument("Core cannot be null.");
        }

        reset();
    }

    void Timer::write_register(uint8_t reg, uint8_t value) {
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
        }
    }

    uint8_t Timer::read_register(uint8_t reg) {
        switch (reg) {
        case 0x04:
            return read_div();
        case 0x05:
            return tima;
        case 0x06:
            return tma;
        case 0x07:
            return tac;
        }
        return 0xFF;
    }

    void Timer::reset() {
        div_cycles = 0xAB00;
        tima = 0;
        tma = 0;
        set_tac(0xF8);
    }

    void Timer::set_tac(uint8_t rate) {
        static constexpr std::array<uint16_t, 4> tac_table = {512, 8, 32, 128};

        tac_rate = tac_table[rate & 0x3];
        tac = rate;
    }

    void Timer::reset_div() { change_div(0); }

    uint8_t Timer::read_div() { return div_cycles >> 8; }

    void Timer::update(int32_t cycles) {
        if (core->cpu.stopped()) {
            change_div(0);
        } else {
            change_div(div_cycles + cycles);
        }
    }

    bool Timer::timer_enabled() const { return tac & 0b100; }

    void Timer::change_div(uint16_t new_div) {
        if (EdgeFell(div_cycles >> 8, new_div >> 8,
                     core->cpu.double_speed() ? 0b100000 : 0b10000)) {
            core->apu.step_frame_sequencer();
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
