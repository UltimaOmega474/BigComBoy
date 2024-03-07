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

namespace GB
{
    Core::Core() : bus(*this), ppu(bus), timer(*this), cpu(*this, bus), dma(*this) {}

    void Core::set_cartridge(Cartridge *cart, bool skip_boot_rom)
    {
        ready_to_run = cart ? true : false;

        bus.cart = cart;

        if (skip_boot_rom || !boot_rom_loaded)
        {
            if (cart->header.cgb_support)
            {
                bus.KEY0 = cart->header.cgb_support;
                ppu.object_priority_mode = 0;
            }
            else
            {
                bus.KEY0 = 0x04;
                ppu.object_priority_mode = 1;

                ppu.set_compatibility_palette(PaletteID::BG, LCD_GRAY);
                ppu.set_compatibility_palette(PaletteID::OBJ1, LCD_GRAY);
                ppu.set_compatibility_palette(PaletteID::OBJ2, LCD_GRAY);
            }
            bus.boot_rom_enabled = false;
            cpu.reset(0x100);
            ppu.set_post_boot_state();
        }
        else
        {
            cpu.reset(0x0);
        }
    }

    void Core::reset()
    {
        apu.reset();
        ppu.reset();
        timer.reset();
        pad.reset();
        bus.reset();
        cpu.reset(0);
        dma.reset();

        ready_to_run = false;
        boot_rom_loaded = false;
    }

    void Core::run_for_frames(uint32_t frames)
    {
        while (frames-- && ready_to_run)
        {
            while (cycle_count < CYCLES_PER_FRAME && !cpu.stopped)
            {
                dma.tick();
                cpu.step();
            }

            if (cycle_count >= CYCLES_PER_FRAME)
                cycle_count -= CYCLES_PER_FRAME;
        }
    }

    void Core::run_for_cycles(uint32_t cycles)
    {
        cycle_count = 0;
        while (cycle_count < cycles && !cpu.stopped)
            cpu.step();
    }

    void Core::tick_subcomponents(uint8_t cycles)
    {
        auto adjusted_cycles = cpu.double_speed ? 2 : 4;

        while (cycles > 0)
        {
            timer.update(4);
            ppu.step(adjusted_cycles);
            apu.step(adjusted_cycles);
            bus.cart->tick(adjusted_cycles);
            cycle_count += adjusted_cycles;
            cycles -= 4;
        }
    }

    void Core::load_boot_rom_from_file(std::filesystem::path path)
    {
        std::ifstream rom(path, std::ios::binary | std::ios::ate);

        boot_rom_loaded = false;

        if (rom)
        {
            auto len = rom.tellg();

            if (len == 0)
                return;

            bus.boot_rom.clear();
            bus.boot_rom.resize(len);

            rom.seekg(0);
            rom.read(reinterpret_cast<char *>(bus.boot_rom.data()), len);
            rom.close();

            boot_rom_loaded = true;
        }
    }
}