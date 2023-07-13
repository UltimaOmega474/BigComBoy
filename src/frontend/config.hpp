#pragma once
#include "gui_constants.hpp"
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
		bool keep_aspect_ratio = true;
		bool allow_sram_saving = true;
		bool skip_boot_rom = false;
		uint32_t sram_save_interval = 30;
		std::array<int, 4> dmg_palette{0, 0, 0, 0};

		Configuration() = default;
		static Configuration &get();
	};
}