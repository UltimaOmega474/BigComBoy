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

    void Core::initialize(Cartridge *cart)
    {
        ready_to_run = cart ? true : false;
        bootstrap.clear();
        apu.reset();
        ppu.reset();
        timer.reset();
        pad.reset();
        bus.reset(cart);
        dma.reset();

        if (ready_to_run)
        {
            bus.bootstrap_mapped = false;

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

            cpu.reset(0x0100);
            ppu.set_post_boot_state();
        }
    }

    void Core::initialize_with_bootstrap(Cartridge *cart, ConsoleType console,
                                         std::filesystem::path bootstrap_path)
    {
        ready_to_run = cart ? true : false;
        bootstrap.clear();
        apu.reset();
        ppu.reset();
        timer.reset();
        pad.reset();
        bus.reset(cart);
        dma.reset();

        if (ready_to_run)
        {
            load_bootstrap(bootstrap_path);
            switch (console)
            {
            case ConsoleType::DMG:
            {
                bus.KEY0 = 0x04;
                ppu.object_priority_mode = 1;

                ppu.set_compatibility_palette(PaletteID::BG, LCD_GRAY);
                ppu.set_compatibility_palette(PaletteID::OBJ1, LCD_GRAY);
                ppu.set_compatibility_palette(PaletteID::OBJ2, LCD_GRAY);
                break;
            }
            case ConsoleType::CGB:
            {
                bus.KEY0 = cart->header.cgb_support;
                ppu.object_priority_mode = 0;
                break;
            }
            default:
                return;
            }

            cpu.reset(0x0);
        }
    }

    void Core::run_for_frames(uint32_t frames)
    {
        if (!ready_to_run)
            return;
        while (frames--)
        {
            while (cycle_count < CYCLES_PER_FRAME)
                step_instruction(1);

            if (cycle_count >= CYCLES_PER_FRAME)
                cycle_count -= CYCLES_PER_FRAME;
        }
    }

    void Core::step_instruction(uint32_t times)
    {
        for (size_t i = 0; i < times; ++i)
        {
            if (!cpu.stopped)
            {
                dma.tick();
                cpu.step();

                if (enable_debug_tools)
                {
                    disassembler.push_instruction(cpu.pc - cpu.fetched_count, cpu.fetched_count,
                                                  cpu.fetched_bytes);
                    disassembler.scan_next_instructions(cpu.pc, bus);
                }
            }
        }
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

    void Core::load_bootstrap(std::filesystem::path path)
    {
        std::ifstream rom(path, std::ios::binary | std::ios::ate);

        if (rom)
        {
            auto len = rom.tellg();

            if (len == 0)
                return;

            bootstrap.clear();
            bootstrap.resize(len);

            rom.seekg(0);
            rom.read(reinterpret_cast<char *>(bootstrap.data()), len);
            rom.close();
        }
    }

    uint8_t Core::read_bootrom(uint16_t address) { return bootstrap[address]; }
}