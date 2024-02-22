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

#include "Bus.hpp"
#include "Core.hpp"

namespace GB
{
    MainBus::MainBus(Core &core) : core(core) {}
    void MainBus::reset()
    {
        boot_rom_enabled = true;
        boot_rom.fill(0);
        wram.fill(0);
        hram.fill(0);
        cart = nullptr;
    }

    void MainBus::request_interrupt(uint8_t interrupt) { core.cpu.interrupt_flag |= interrupt; }

    uint8_t MainBus::read(uint16_t address)
    {
        auto page = address >> 12;

        switch (page)
        {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
        {
            if (address < 0x100 && boot_rom_enabled)
                return boot_rom[address & 0xFF];

            if (cart)
                return cart->read(address);

            return 0xFF;
        }

        case 0x8:
        case 0x9:
        {
            return core.ppu.read_vram(address & 0x1FFF);
        }
        case 0xA:
        case 0xB:
        {
            if (cart)
                return cart->read_ram(address & 0x1FFF);

            return 0xFF;
        }
        case 0xC:
        case 0xD:
        case 0xE:
        {
            return wram[address & 0x1FFF];
        }
        case 0xF:
        {
            auto hram_page = address >> 8;

            switch (hram_page)
            {
            case 0xFD:
            {
                return wram[address & 0x1FFF];
            }
            case 0xFE:
            {
                return core.ppu.read_oam(address & 0xFF);
            }
            case 0xFF:
            {
                auto io_address = address & 0xFF;

                switch (io_address)
                {
                // Input
                case 0x00:
                    return core.pad.get_pad_state();

                // Serial Port
                case 0x01:
                    return 0xFF;
                case 0x02:
                    return 0xFF;

                // Timer
                case 0x04:
                    return core.timer.read_div();
                case 0x05:
                    return core.timer.tima;
                case 0x06:
                    return core.timer.tma;
                case 0x07:
                    return core.timer.tac;
                case 0x0F:
                    return core.cpu.interrupt_flag;

                // APU
                case 0x10:
                case 0x11:
                case 0x12:
                case 0x13:
                case 0x14:
                case 0x15:
                case 0x16:
                case 0x17:
                case 0x18:
                case 0x19:
                case 0x1A:
                case 0x1B:
                case 0x1C:
                case 0x1D:
                case 0x1E:
                case 0x1F:
                case 0x20:
                case 0x21:
                case 0x22:
                case 0x23:
                case 0x24:
                case 0x25:
                case 0x26:
                case 0x27:
                    return core.apu.read_register(io_address);

                // APU Wave RAM
                case 0x30:
                case 0x31:
                case 0x32:
                case 0x33:
                case 0x34:
                case 0x35:
                case 0x36:
                case 0x37:
                case 0x38:
                case 0x39:
                case 0x3A:
                case 0x3B:
                case 0x3C:
                case 0x3D:
                case 0x3E:
                case 0x3F:
                    return core.apu.read_wave_ram(io_address - 0x30);

                // ppu registers
                case 0x40:
                    return core.ppu.lcd_control;
                case 0x41:
                    return core.ppu.status;
                case 0x42:
                    return core.ppu.screen_scroll_y;
                case 0x43:
                    return core.ppu.screen_scroll_x;
                case 0x44:
                    return core.ppu.line_y;
                case 0x45:
                    return core.ppu.line_y_compare;
                case 0x46:
                    return 0xFF; // dma start address
                case 0x47:
                    return core.ppu.background_palette;
                case 0x48:
                    return core.ppu.object_palette_0;
                case 0x49:
                    return core.ppu.object_palette_1;
                case 0x4A:
                    return core.ppu.window_y;
                case 0x4B:
                    return core.ppu.window_x;

                case 0x50:
                    return boot_rom_enabled;
                }

                if (within_range(address, 0xFF80, 0xFFFE))
                {
                    return hram[address - 0xFF80]; // High Ram
                }
                else if (address == 0xFFFF)
                {
                    return core.cpu.interrupt_enable;
                }

                return 0xFF; // IO Registers
            }
            }

            return 0;
        }
        }

        return 0;
    }

    void MainBus::write(uint16_t address, uint8_t value)
    {
        auto page = address >> 12;

        switch (page)
        {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
        {
            if (address < 0x100 && boot_rom_enabled)
                return;

            if (cart)
                cart->write(address, value);

            return;
        }

        case 0x8:
        case 0x9:
        {
            core.ppu.write_vram(address & 0x1FFF, value);
            return;
        }

        case 0xA:
        case 0xB:
        {
            if (cart)
                cart->write_ram(address & 0x1FFF, value);
            return;
        }

        case 0xC:
        case 0xD:
        case 0xE:
        {
            wram[address & 0x1FFF] = value;
            return;
        }

        case 0xF:
        {
            auto hram_page = address >> 8;

            switch (hram_page)
            {
            case 0xFD:
            {
                wram[address & 0x1FFF] = value;
                return;
            }
            case 0xFE:
            {
                core.ppu.write_oam(address & 0xFF, value);
                return;
            }
            case 0xFF:
            {
                auto io_address = address & 0xFF;

                switch (io_address)
                {
                // Input
                case 0x00:
                {
                    core.pad.select_button_mode(value);
                    return;
                }

                // Timer
                case 0x04:
                {
                    core.timer.reset_div();
                    return;
                }
                case 0x05:
                {
                    core.timer.tima = value;
                    return;
                }
                case 0x06:
                {
                    core.timer.tma = value;
                    return;
                }
                case 0x07:
                {
                    core.timer.set_tac(value);
                    return;
                }
                case 0x0F:
                {
                    core.cpu.interrupt_flag = value;
                    return;
                }

                // APU
                case 0x10:
                case 0x11:
                case 0x12:
                case 0x13:
                case 0x14:
                case 0x15:
                case 0x16:
                case 0x17:
                case 0x18:
                case 0x19:
                case 0x1A:
                case 0x1B:
                case 0x1C:
                case 0x1D:
                case 0x1E:
                case 0x1F:
                case 0x20:
                case 0x21:
                case 0x22:
                case 0x23:
                case 0x24:
                case 0x25:
                case 0x26:
                case 0x27:
                {
                    core.apu.write_register(io_address, value);
                    return;
                }

                // APU Wave RAM
                case 0x30:
                case 0x31:
                case 0x32:
                case 0x33:
                case 0x34:
                case 0x35:
                case 0x36:
                case 0x37:
                case 0x38:
                case 0x39:
                case 0x3A:
                case 0x3B:
                case 0x3C:
                case 0x3D:
                case 0x3E:
                case 0x3F:
                {
                    core.apu.write_wave_ram(io_address - 0x30, value);
                    return;
                }

                // PPU Registers
                case 0x40:
                {
                    core.ppu.lcd_control = value;
                    return;
                }
                case 0x41:
                {
                    core.ppu.status = value & 0xF8;
                    return;
                }

                case 0x42:
                {
                    core.ppu.screen_scroll_y = value;
                    return;
                }
                case 0x43:
                {
                    core.ppu.screen_scroll_x = value;
                    return;
                }
                case 0x45:
                {
                    core.ppu.line_y_compare = value;
                    return;
                }
                case 0x46:
                {
                    core.ppu.instant_dma(value);
                    return; // dma start address
                }
                case 0x47:
                {
                    core.ppu.background_palette = value;
                    return;
                }
                case 0x48:
                {
                    core.ppu.object_palette_0 = value;
                    return;
                }
                case 0x49:
                {
                    core.ppu.object_palette_1 = value;
                    return;
                }

                case 0x4A:
                {
                    core.ppu.window_y = value;
                    return;
                }
                case 0x4B:
                {
                    core.ppu.window_x = value;
                    return;
                }

                case 0x50:
                {
                    boot_rom_enabled = false;
                    return;
                }
                }

                if (within_range(address, 0xFF80, 0xFFFE))
                {
                    hram[address - 0xFF80] = value; // High Ram
                    return;
                }
                else if (address == 0xFFFF)
                {
                    core.cpu.interrupt_enable = value;
                    return;
                }

                return;
            }
            }
        }
        }
    }
}