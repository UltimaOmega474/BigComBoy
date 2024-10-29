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

#include "Core.hpp"
#include "Constants.hpp"
#include "PPU.hpp"
#include <fstream>

namespace GB {
    Core::Core() : bus(this), ppu(this), timer(this), cpu(bus_write_fn, bus_read_fn), dma(this) {}

    auto Core::initialize(Cartridge *cart) -> void {
        ready_to_run = cart ? true : false;

        if (ready_to_run) {
            cart->reset();
            bootstrap.clear();
            apu.reset();
            ppu.reset();
            timer.reset();
            pad.reset();
            bus.reset(cart);
            dma.reset();

            if (cart->header().cgb_support == 0x80 || cart->header().cgb_support == 0xC0) {
                bus.KEY0 = cart->header().cgb_support;
                ppu.write_register(0x6C, 0);
            } else {
                bus.KEY0 = DISABLE_CGB_FUNCTIONS;
                ppu.write_register(0x6C, 1);

                ppu.set_compatibility_palette(PaletteID::BG, LCD_GRAY);
                ppu.set_compatibility_palette(PaletteID::OBJ1, LCD_GRAY);
                ppu.set_compatibility_palette(PaletteID::OBJ2, LCD_GRAY);
            }

            bus.bootstrap_mapped_ = false;

            cpu.reset(0x0100, bus.is_compatibility_mode());
            ppu.set_post_boot_state();
        }
    }

    auto Core::initialize_with_bootstrap(Cartridge *cart, const ConsoleType console,
                                         const std::filesystem::path &bootstrap_path) -> void {
        ready_to_run = cart ? true : false;

        if (ready_to_run) {
            cart->reset();
            bootstrap.clear();
            apu.reset();
            ppu.reset();
            timer.reset();
            pad.reset();
            bus.reset(cart);
            dma.reset();

            load_bootstrap(bootstrap_path);

            if (console == ConsoleType::DMG) {
                bus.KEY0 = DISABLE_CGB_FUNCTIONS;
                ppu.write_register(0x6C, 1);

                ppu.set_compatibility_palette(PaletteID::BG, LCD_GRAY);
                ppu.set_compatibility_palette(PaletteID::OBJ1, LCD_GRAY);
                ppu.set_compatibility_palette(PaletteID::OBJ2, LCD_GRAY);
            }

            cpu.reset(0x0, bus.is_compatibility_mode());
        }
    }

    auto Core::run_for_frames(int32_t frames) -> void {
        while (frames-- && ready_to_run) {
            while (cycle_count < CYCLES_PER_FRAME) {
                const int32_t adjusted_cycles = cpu.double_speed() ? 2 : 4;

                ppu.clock(adjusted_cycles);
                apu.clock(adjusted_cycles);
                bus.cart->clock(adjusted_cycles);

                timer.clock(4);
                if (dma.is_transfer_active()) {
                    dma.clock();
                } else {
                    cpu.clock(pad.get_pad_state());
                }

                cycle_count += adjusted_cycles;
            }

            if (cycle_count >= CYCLES_PER_FRAME) {
                cycle_count -= CYCLES_PER_FRAME;
            }
        }
    }

    auto Core::load_bootstrap(const std::filesystem::path &path) -> void {
        std::ifstream rom(path, std::ios::binary | std::ios::ate);

        if (rom) {
            const auto len = rom.tellg();

            if (len == 0) {
                return;
            }

            bootstrap.clear();
            bootstrap.resize(len);

            rom.seekg(0);
            rom.read(reinterpret_cast<char *>(bootstrap.data()), len);
            rom.close();
        }
    }

    auto Core::read_bootstrap(const uint16_t address) const -> uint8_t {
        return bootstrap[address];
    }
}