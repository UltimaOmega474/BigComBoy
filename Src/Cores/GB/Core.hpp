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
#include "Cartridge.hpp"
#include "DMA.hpp"
#include "Debug.hpp"
#include "PPU.hpp"
#include "Pad.hpp"
#include "SM83.hpp"
#include "Timer.hpp"
#include <cinttypes>
#include <filesystem>
#include <vector>

namespace GB
{
    class Core
    {
        uint32_t cycle_count = 0;
        bool ready_to_run = false;
        std::vector<uint8_t> bootstrap{};

    public:
        Gamepad pad;
        MainBus bus;
        PPU ppu;
        APU apu;
        Timer timer;
        SM83 cpu;
        DMAController dma;
        Debugger debugger;
        Core();

        void initialize(Cartridge *cart);
        void initialize_with_bootstrap(Cartridge *cart, ConsoleType console,
                                       std::filesystem::path bootstrap_path);
        void run_for_frames(uint32_t frames);
        void run_for_cycles(uint32_t cycles);
        void tick_subcomponents(uint8_t cycles);
        void load_bootstrap(std::filesystem::path path);

        uint8_t read_bootrom(uint16_t address);
    };

}