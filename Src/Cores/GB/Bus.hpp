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

#pragma once
#include <array>
#include <cinttypes>

namespace GB
{
    class Cartridge;
    class Core;
    class MainBus
    {
        Core &core;

    public:
        bool boot_rom_enabled = true;
        uint8_t KEY1 = 0;

        std::array<uint8_t, 8192> wram{};
        std::array<uint8_t, 127> hram{};
        std::array<uint8_t, 256> boot_rom{};

        Cartridge *cart = nullptr;
        MainBus(Core &core);

        void reset();
        void request_interrupt(uint8_t interrupt);

        // bus functions
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t value);
    };
}