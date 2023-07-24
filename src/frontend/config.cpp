#include "config.hpp"
#define TOML_EXCEPTIONS 0
#include <toml.hpp>
#include <fstream>
#include <sstream>
namespace SunBoy
{
	Configuration Configuration::current("settings.toml");

	Configuration::Configuration(std::string path)
		: config_path(std::move(path))
	{
		std::ifstream toml_file(config_path);
		if (toml_file)
		{
			auto parse_result = toml::parse(toml_file);
			boot_rom_path = parse_result["boot_rom"].value_or(boot_rom_path);
			skip_boot_rom = parse_result["skip_boot_rom"].value_or(skip_boot_rom);
			keep_aspect_ratio = parse_result["keep_aspect_ratio"].value_or(keep_aspect_ratio);
			linear_filtering = parse_result["linear_filtering"].value_or(linear_filtering);
			allow_sram_saving = parse_result["allow_sram_saving"].value_or(allow_sram_saving);
			sram_save_interval = parse_result["sram_save_interval"].value_or(sram_save_interval);
			audio_latency_select = parse_result["audio_latency_select"].value_or(audio_latency_select);
			audio_master_volume = parse_result["audio_master_volume"].value_or(audio_master_volume);
			auto ct = parse_result["color_table"].as_array();
			if (ct)
			{
				for (auto i = 0; (i < ct->size() && i < color_table.size()); ++i)
				{
					color_table[i] = (*ct)[i].value_or(color_table[i]);
				}
			}

			auto recent = parse_result["recent_rom_paths"].as_array();
			if (recent)
			{
				for (auto i = recent->size(); i > 0; --i)
				{
					add_rom_path((*recent)[i - 1].value_or(""));
				}
			}
		}
		else
		{
			toml_file.close();
			save_as_toml_file();
		}
	}

	void Configuration::save_as_toml_file()
	{
		std::ofstream out_file(config_path);
		if (out_file)
		{
			out_file << as_toml_string();
			out_file.close();
		}
	}

	void Configuration::add_rom_path(std::string path)
	{
		auto it = std::find(recent_rom_paths.begin(), recent_rom_paths.end(), path);
		if (it != recent_rom_paths.end())
		{
			recent_rom_paths.erase(it);
		}

		recent_rom_paths.push_front(std::move(path));
		if (recent_rom_paths.size() > 7)
			recent_rom_paths.pop_back();
	}

	std::string Configuration::as_toml_string() const
	{
		toml::table tbl{
			{"boot_rom", boot_rom_path},
			{"skip_boot_rom", skip_boot_rom},
			{"keep_aspect_ratio", keep_aspect_ratio},
			{"linear_filtering", linear_filtering},
			{"allow_sram_saving", allow_sram_saving},
			{"sram_save_interval", sram_save_interval},
			{"audio_latency_select", audio_latency_select},
			{"audio_master_volume", audio_master_volume},
			{"color_table", toml::array{
								color_table[0],
								color_table[1],
								color_table[2],
								color_table[3],
							}},
		};

		toml::array toml_rom_paths;
		for (const auto &path : recent_rom_paths)
			toml_rom_paths.insert(toml_rom_paths.end(), path);
		tbl.insert_or_assign("recent_rom_paths", toml_rom_paths);

		return (std::stringstream{} << tbl).str();
	}

	Configuration &Configuration::get()
	{
		return current;
	}

}