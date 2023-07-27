#pragma once
#include "gui_constants.hpp"
#include "input.hpp"
#include <gb/constants.hpp>
#include <string>
#include <array>
#include <deque>
#include <map>
#define TOML_EXCEPTIONS 0
#include <toml.hpp>

namespace SunBoy
{
	class Configuration
	{
		static Configuration current;

	public:
		std::string config_path = get_full_path("/settings.toml");

		bool keep_aspect_ratio = true, linear_filtering = false;
		bool allow_sram_saving = true, skip_boot_rom = true;
		uint8_t audio_master_volume = 40;
		uint32_t sram_save_interval = 15, audio_latency_select = 1;

		std::array<uint32_t, 4> color_table = SunBoy::LCD_GRAY_PALETTE;
		std::deque<std::string> recent_rom_paths;
		std::string boot_rom_path = get_full_path("/dmg_boot.bin");

		KeyboardMapping keyboard_maps;
		std::vector<ControllerMapping> controller_maps;

		Configuration(std::string path);

		void save_as_toml_file();
		void add_rom_path(std::string path);
		std::string as_toml_string() const;
		static Configuration &get();

	private:
		void load_keyboard_mapping(toml::table &keyboard);
		void save_keyboard_mapping(toml::table &keyboard) const;
	};
}