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
#include "APU.hpp"
#include "Bus.hpp"
#include "CPU.hpp"
#include "Cartridge.hpp"
#include "DMA.hpp"
#include "PPU.hpp"
#include "Pad.hpp"
#include "Timer.hpp"
#include <cinttypes>
#include <filesystem>
#include <vector>

namespace GB {
    class Core {
        std::function<uint8_t(uint16_t)> bus_read_fn = [this](uint16_t address) -> uint8_t {
            return bus.read(address);
        };

        std::function<void(uint16_t, uint8_t)> bus_write_fn = [this](uint16_t address,
                                                                     uint8_t value) -> void {
            bus.write(address, value);
        };

    public:
        Gamepad pad;
        MainBus bus;
        PPU ppu;
        APU apu;
        Timer timer;
        CPU cpu;
        DMAController dma;
        Core();

        auto initialize(Cartridge *cart) -> void;
        auto initialize_with_bootstrap(Cartridge *cart, ConsoleType console,
                                       const std::filesystem::path &bootstrap_path) -> void;
        auto run_for_frames(int32_t frames) -> void;
        auto load_bootstrap(const std::filesystem::path &path) -> void;

        auto read_bootstrap(uint16_t address) const -> uint8_t;

    private:
        bool ready_to_run = false;
        int32_t cycle_count = 0;
        std::vector<uint8_t> bootstrap{};
    };
}