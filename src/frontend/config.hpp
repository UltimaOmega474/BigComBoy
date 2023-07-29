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

		struct
		{
			bool allow_sram_saving = true, skip_boot_rom = true;
			uint32_t sram_save_interval = 15;
			std::string boot_rom_path = get_full_path("/dmg_boot.bin");
		} emulation;

		struct
		{
			bool keep_aspect_ratio = true, linear_filtering = false;
			std::array<uint32_t, 4> color_table = SunBoy::LCD_GRAY_PALETTE;
		} video;

		struct
		{
			uint8_t master_volume = 70, latency_select = 1;
		} audio;

		std::vector<InputBindingProfile> input_profiles;

		std::deque<std::string> recent_rom_paths;

		Configuration(std::string path);

		void save_as_toml_file();
		void add_rom_path(std::string path);
		std::string as_toml_string() const;
		static Configuration &get();

	private:
		void load_emulation_settings(toml::table &emulation);
		toml::table emulation_settings_as_toml() const;

		void load_video_settings(toml::table &video);
		toml::table video_settings_as_toml() const;

		void load_audio_settings(toml::table &audio);
		toml::table audio_settings_as_toml() const;

		void load_input_mapping(toml::array &profiles);
		toml::array input_profiles_as_toml() const;
	};
}