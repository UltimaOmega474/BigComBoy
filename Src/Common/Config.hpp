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
#include "Cores/GB/Constants.hpp"
#include "Input/InputDevice.hpp"
#include <array>
#include <cinttypes>
#include <deque>
#include <filesystem>
#include <string>

namespace Common {
    struct GBGamepadConfig {
        std::string device_name;
        std::array<Input::InputSource, 8> buttons{};
    };

    struct GBConfig {
        struct VideoData {
            std::string screen_filter = "No Filter";
            bool frame_blending = true;
            bool smooth_scaling = false;
        } video;

        struct EmulationData {
            GB::ConsoleType console = GB::ConsoleType::AutoSelect;
            std::filesystem::path dmg_bootstrap;
            std::filesystem::path cgb_bootstrap;

            bool allow_sram_saving = true;
            int32_t sram_save_interval = 30;
        } emulation;

        struct AudioData {
            int32_t volume = 70;
            int32_t square1 = 100;
            int32_t square2 = 100;
            int32_t wave = 100;
            int32_t noise = 100;
        } audio;

        std::array<GBGamepadConfig, 2> input_mappings{
            GBGamepadConfig{.device_name = "Keyboard"},
            GBGamepadConfig{.device_name = "Keyboard"},
        };
    };

    struct Config {
        int32_t wsize_x = 640, wsize_y = 480;

        std::deque<std::string> recent_roms{};
        GBConfig gameboy;

        static Config &current();
        void add_rom_to_history(const std::string &file);
        void write_to_file(std::filesystem::path path);
        void read_from_file(std::filesystem::path path);
    };
}