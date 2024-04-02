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

#include "Config.hpp"
#include <filesystem>
#include <fstream>
#include <toml.hpp>
#include <vector>

namespace Common {
    static Config config;

    Config &Config::current() { return config; }

    void Config::add_rom_to_history(const std::string &path) {
        auto it = std::find(recent_roms.begin(), recent_roms.end(), path);
        if (it != recent_roms.end()) {
            recent_roms.erase(it);
        }

        recent_roms.push_front(std::move(path));
        if (recent_roms.size() > 10) {
            recent_roms.pop_back();
        }
    }

    void Config::write_to_file(std::filesystem::path path) {
        using namespace Input;
        toml::value gb{
            {"console", static_cast<int32_t>(gameboy.emulation.console)},
            {"dmg_bootstrap", gameboy.emulation.dmg_bootstrap.string()},
            {"cgb_bootstrap", gameboy.emulation.cgb_bootstrap.string()},
            {"allow_sram_saving", gameboy.emulation.allow_sram_saving},
            {"sram_save_interval", gameboy.emulation.sram_save_interval},
            {"frame_blending", gameboy.video.frame_blending},
            {"smooth_scaling", gameboy.video.smooth_scaling},
            {"screen_filter", gameboy.video.screen_filter},
            {"volume", gameboy.audio.volume},
            {"square1", gameboy.audio.square1},
            {"square2", gameboy.audio.square2},
            {"wave", gameboy.audio.wave},
            {"noise", gameboy.audio.noise},
        };

        std::vector<toml::value> devices;
        for (const auto &mapping : gameboy.input_mappings) {
            const auto &source_to_toml = [](const InputSource &src) {
                return toml::value{
                    {"type", (int32_t)src.type},
                    {"keyboard", src.keyboard},
                    {"button", src.button},
                    {"axis", src.axis},
                    {"direction", (int32_t)src.axis_direction},
                };
            };

            std::vector<toml::value> buttons;
            for (const auto &button : mapping.buttons) {
                buttons.push_back(source_to_toml(button));
            }

            toml::value device{
                {"device_name", mapping.device_name},
            };

            device["buttons"] = buttons;

            devices.push_back(device);
        }

        gb["devices"] = devices;

        toml::value data{
            {"wsize_x", wsize_x},
            {"wsize_y", wsize_y},
            {"recent_roms", recent_roms},
            {"gameboy", gb},
        };

        std::ofstream ofs{path};

        if (ofs) {
            ofs << data;
            ofs.close();
        }
    }

    void Config::read_from_file(std::filesystem::path path) {
        using namespace Input;

        if (!std::filesystem::exists(path)) {
            return;
        }

        auto data = toml::parse(path);
        wsize_x = toml::find_or(data, "wsize_x", wsize_x);
        wsize_y = toml::find_or(data, "wsize_y", wsize_y);

        recent_roms = toml::find_or(data, "recent_roms", recent_roms);

        auto &gb = data["gameboy"];

        gameboy.emulation.console = static_cast<GB::ConsoleType>(
            toml::find_or(gb, "console", static_cast<int32_t>(gameboy.emulation.console)));
        gameboy.emulation.dmg_bootstrap =
            toml::find_or(gb, "dmg_bootstrap", gameboy.emulation.dmg_bootstrap.string());
        gameboy.emulation.cgb_bootstrap =
            toml::find_or(gb, "cgb_bootstrap", gameboy.emulation.cgb_bootstrap.string());

        gameboy.emulation.allow_sram_saving =
            toml::find_or(gb, "allow_sram_saving", gameboy.emulation.allow_sram_saving);
        gameboy.emulation.sram_save_interval =
            toml::find_or(gb, "sram_save_interval", gameboy.emulation.sram_save_interval);

        gameboy.video.frame_blending =
            toml::find_or(gb, "frame_blending", gameboy.video.frame_blending);
        gameboy.video.smooth_scaling =
            toml::find_or(gb, "smooth_scaling", gameboy.video.smooth_scaling);
        gameboy.video.screen_filter =
            toml::find_or(gb, "screen_filter", gameboy.video.screen_filter);

        gameboy.audio.volume = toml::find_or(gb, "volume", gameboy.audio.volume);
        gameboy.audio.square1 = toml::find_or(gb, "square1", gameboy.audio.square1);
        gameboy.audio.square2 = toml::find_or(gb, "square2", gameboy.audio.square2);
        gameboy.audio.wave = toml::find_or(gb, "wave", gameboy.audio.wave);
        gameboy.audio.noise = toml::find_or(gb, "noise", gameboy.audio.noise);

        if (gb["devices"].is_array()) {
            auto gb_devices = gb["devices"].as_array();

            const auto &toml_to_source = [](toml::value &src) {
                return InputSource{
                    .type = (SourceType)toml::find_or(src, "type", -1),
                    .keyboard = toml::find_or(src, "keyboard", -1),
                    .button = toml::find_or(src, "button", -1),
                    .axis = toml::find_or(src, "axis", -1),
                    .axis_direction = (AxisDirection)toml::find_or(src, "axis_direction", 0),
                };
            };

            for (int i = 0; i < gameboy.input_mappings.size(); ++i) {
                auto &device = gb_devices[i];
                auto &mapping = gameboy.input_mappings[i];

                mapping.device_name = toml::find_or(device, "device_name", mapping.device_name);

                if (device["buttons"].is_array()) {
                    auto &buttons = device["buttons"].as_array();
                    for (int i = 0; i < buttons.size(); ++i) {
                        mapping.buttons[i] = toml_to_source(buttons[i]);
                    }
                }
            }
        }
    }
}