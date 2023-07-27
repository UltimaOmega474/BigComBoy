#include "config.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>

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
			boot_rom_path = parse_result["emulation"]["boot_rom"].value_or(boot_rom_path);
			skip_boot_rom = parse_result["emulation"]["skip_boot_rom"].value_or(skip_boot_rom);
			allow_sram_saving = parse_result["emulation"]["allow_sram_saving"].value_or(allow_sram_saving);
			sram_save_interval = parse_result["emulation"]["sram_save_interval"].value_or(sram_save_interval);
			audio_latency_select = parse_result["audio"]["audio_latency_select"].value_or(audio_latency_select);
			audio_master_volume = parse_result["audio"]["audio_master_volume"].value_or(audio_master_volume);
			audio_master_volume = std::min<uint32_t>(audio_master_volume, 100);
			keep_aspect_ratio = parse_result["video"]["keep_aspect_ratio"].value_or(keep_aspect_ratio);
			linear_filtering = parse_result["video"]["linear_filtering"].value_or(linear_filtering);
			auto ct = parse_result["video"]["color_table"].as_array();
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

			auto keyboard = parse_result["keyboard-mapping"].as_table();
			if (keyboard)
				load_keyboard_mapping(*keyboard);
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
		toml::table tbl;

		toml::table video{
			{"keep_aspect_ratio", keep_aspect_ratio},
			{"linear_filtering", linear_filtering},
			{"color_table", toml::array{
								color_table[0],
								color_table[1],
								color_table[2],
								color_table[3],
							}},
		};

		tbl.insert("video", video);

		toml::table audio{
			{"audio_latency_select", audio_latency_select},
			{"audio_master_volume", audio_master_volume},
		};
		tbl.insert("audio", audio);

		toml::table emulation{
			{"boot_rom", boot_rom_path},
			{"skip_boot_rom", skip_boot_rom},
			{"allow_sram_saving", allow_sram_saving},
			{"sram_save_interval", sram_save_interval},
		};
		tbl.insert("emulation", emulation);

		toml::table keyboard;
		save_keyboard_mapping(keyboard);

		tbl.insert("keyboard-mapping", keyboard);

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

	void Configuration::load_keyboard_mapping(toml::table &keyboard)
	{
		const auto left = keyboard["left"].as_array();
		if (left)
		{
			for (auto i = 0; i < left->size(); ++i)
			{
				auto key = SDL_GetScancodeFromName(left[i].as_string()->value_or(""));
				if (key != SDL_SCANCODE_UNKNOWN)
					keyboard_maps.left.push_back(key);
			}
		}
		const auto right = keyboard["right"].as_array();
		if (right)
		{
			for (auto i = 0; i < right->size(); ++i)
			{
				auto key = SDL_GetScancodeFromName(right[i].as_string()->value_or(""));
				if (key != SDL_SCANCODE_UNKNOWN)
					keyboard_maps.right.push_back(key);
			}
		}

		const auto up = keyboard["up"].as_array();
		if (up)
		{
			for (auto i = 0; i < up->size(); ++i)
			{
				auto key = SDL_GetScancodeFromName(up[i].as_string()->value_or(""));
				if (key != SDL_SCANCODE_UNKNOWN)
					keyboard_maps.up.push_back(key);
			}
		}

		const auto down = keyboard["down"].as_array();
		if (down)
		{
			for (auto i = 0; i < down->size(); ++i)
			{
				auto key = SDL_GetScancodeFromName(down[i].as_string()->value_or(""));
				if (key != SDL_SCANCODE_UNKNOWN)
					keyboard_maps.down.push_back(key);
			}
		}

		const auto a = keyboard["a"].as_array();
		if (a)
		{
			for (auto i = 0; i < a->size(); ++i)
			{
				auto key = SDL_GetScancodeFromName(a[i].as_string()->value_or(""));
				if (key != SDL_SCANCODE_UNKNOWN)
					keyboard_maps.a.push_back(key);
			}
		}

		const auto b = keyboard["b"].as_array();
		if (b)
		{
			for (auto i = 0; i < b->size(); ++i)
			{
				auto key = SDL_GetScancodeFromName(b[i].as_string()->value_or(""));
				if (key != SDL_SCANCODE_UNKNOWN)
					keyboard_maps.b.push_back(key);
			}
		}

		const auto select = keyboard["select"].as_array();
		if (select)
		{
			for (auto i = 0; i < select->size(); ++i)
			{
				auto key = SDL_GetScancodeFromName(select[i].as_string()->value_or(""));
				if (key != SDL_SCANCODE_UNKNOWN)
					keyboard_maps.select.push_back(key);
			}
		}

		const auto start = keyboard["start"].as_array();
		if (start)
		{
			for (auto i = 0; i < start->size(); ++i)
			{
				auto key = SDL_GetScancodeFromName(start[i].as_string()->value_or(""));
				if (key != SDL_SCANCODE_UNKNOWN)
					keyboard_maps.start.push_back(key);
			}
		}
	}

	void Configuration::save_keyboard_mapping(toml::table &keyboard) const
	{
		toml::array left_keys, right_keys, up_keys, down_keys;
		toml::array a_keys, b_keys, select_keys, start_keys;
		for (auto key : keyboard_maps.left)
			left_keys.push_back(SDL_GetScancodeName(key));
		keyboard.insert("left", left_keys);

		for (auto key : keyboard_maps.right)
			right_keys.push_back(SDL_GetScancodeName(key));
		keyboard.insert("right", left_keys);

		for (auto key : keyboard_maps.up)
			up_keys.push_back(SDL_GetScancodeName(key));
		keyboard.insert("up", left_keys);

		for (auto key : keyboard_maps.down)
			down_keys.push_back(SDL_GetScancodeName(key));
		keyboard.insert("down", left_keys);

		for (auto key : keyboard_maps.a)
			a_keys.push_back(SDL_GetScancodeName(key));
		keyboard.insert("a", left_keys);

		for (auto key : keyboard_maps.b)
			b_keys.push_back(SDL_GetScancodeName(key));
		keyboard.insert("b", left_keys);

		for (auto key : keyboard_maps.select)
			select_keys.push_back(SDL_GetScancodeName(key));
		keyboard.insert("select", left_keys);

		for (auto key : keyboard_maps.start)
			start_keys.push_back(SDL_GetScancodeName(key));
		keyboard.insert("start", left_keys);
	}
}