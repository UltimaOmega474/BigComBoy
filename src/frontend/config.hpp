#pragma once
#include <string>
#include <array>
#include <vector>
class Configuration
{
	static Configuration current;

public:
	std::vector<std::string> recent_rom_paths;
	std::string boot_rom_path;
	bool keep_aspect_ratio = true;
	bool allow_sram_saving = true;
	bool allow_boot_rom = false;
	uint32_t sram_save_interval = 30;
	std::array<int, 4> dmg_palette{0, 0, 0, 0};

	Configuration() = default;
	static Configuration &get_current();
};
