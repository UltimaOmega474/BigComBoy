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

namespace GB {
    class Cartridge;
    class Core;

    class MainBus {
    public:
        explicit MainBus(Core *core);

        auto bootstrap_mapped() const -> bool;
        auto is_compatibility_mode() const -> bool;

        auto reset(Cartridge *new_cart) -> void;

        auto read(uint16_t address) const -> uint8_t;
        auto write(uint16_t address, uint8_t value) -> void;

    private:
        bool bootstrap_mapped_ = true;
        uint8_t wram_bank_num = 1;
        uint8_t KEY0 = 0x0;

        std::array<uint8_t, 32768> wram{};
        std::array<uint8_t, 127> hram{};

        Cartridge *cart = nullptr;
        Core *core;

        friend class Core;
    };
}