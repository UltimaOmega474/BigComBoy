#pragma once
#include "gui_constants.hpp"
#include <gb/constants.hpp>
#include <string>
#include <array>
#include <deque>
namespace SunBoy
{
	class Configuration
	{
		static Configuration current;
		std::string config_path = get_full_path("/settings.toml");

	public:
		bool keep_aspect_ratio = true, linear_filtering = false;
		bool allow_sram_saving = true, skip_boot_rom = true;

		uint32_t sram_save_interval = 15, audio_latency_select = 1;
		float audio_master_volume = 0.70f;

		std::array<uint32_t, 4> color_table = SunBoy::LCD_GRAY_PALETTE;
		std::deque<std::string> recent_rom_paths;
		std::string boot_rom_path = get_full_path("/dmg_boot.bin");
		Configuration(std::string path);

		void save_as_toml_file();
		void add_rom_path(std::string path);
		std::string as_toml_string() const;
		static Configuration &get();
	};
}