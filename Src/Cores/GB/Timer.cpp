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

namespace GB {
    Timer::Timer(Core &core) : core(core) { set_tac(0); }

    void Timer::reset() {
        div_cycles = 0xAB00;
        tima = 0;
        tma = 0;
        set_tac(0xF8);
    }

    void Timer::set_tac(uint8_t rate) {
        constexpr uint32_t tac_table[4] = {512, 8, 32, 128};

        tac_rate = tac_table[rate & 0x3];
        tac = rate;
    }

    void Timer::reset_div() { change_div(0); }

    uint8_t Timer::read_div() { return div_cycles >> 8; }

    void Timer::update(uint32_t cycles) {
        if (core.cpu.stopped()) {
            change_div(0);
        } else {
            change_div(div_cycles + cycles);
        }
    }

    bool Timer::timer_enabled() const { return tac & 0b100; }

    void Timer::change_div(uint16_t new_div) {
        if (EdgeFell(div_cycles >> 8, new_div >> 8, core.cpu.double_speed() ? 0b100000 : 0b10000)) {
            core.apu.step_frame_sequencer();
        }

        if (timer_enabled() && EdgeFell(div_cycles, new_div, tac_rate)) {
            ++tima;
            if (tima == 0) {
                tima = tma;
                core.bus.request_interrupt(0x04);
            }
        }

        div_cycles = new_div;
    }
}
