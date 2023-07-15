#pragma once
#include "gui_constants.hpp"
#include <gb/constants.hpp>
#include <string>
#include <array>
#include <vector>

namespace AngbeGui
{
	class Configuration
	{
		static Configuration current;

	public:
		std::vector<std::string> recent_rom_paths;
		std::string boot_rom_path = get_full_path("/dmg_boot.bin");
		bool keep_aspect_ratio = true, linear_filtering = false;
		bool allow_sram_saving = true, skip_boot_rom = true;
		uint32_t sram_save_interval = 30;
		std::array<uint32_t, 4> color_table = Angbe::LCD_GRAY_PALETTE;

		static Configuration &
		get();
	};
}