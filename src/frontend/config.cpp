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
			auto emulation = parse_result["emulation"].as_table();
			if (emulation)
				load_emulation_settings(*emulation);

			auto audio = parse_result["audio"].as_table();
			if (audio)
				load_audio_settings(*audio);

			auto video = parse_result["video"].as_table();
			if (video)
				load_video_settings(*video);

			auto input_mapping = parse_result["input"].as_array();
			if (input_mapping)
				load_input_mapping(*input_mapping);

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
		toml::table root{
			{"emulation", emulation_settings_as_toml()},
			{"video", video_settings_as_toml()},
			{"audio", audio_settings_as_toml()},
			{"input", input_profiles_as_toml()},
		};

		toml::array toml_rom_paths;
		for (const auto &path : recent_rom_paths)
			toml_rom_paths.insert(toml_rom_paths.end(), path);
		root.insert("recent_rom_paths", toml_rom_paths);

		return (std::stringstream{} << root).str();
	}

	Configuration &Configuration::get()
	{
		return current;
	}

	void Configuration::load_emulation_settings(toml::table &emulation_table)
	{
		emulation.boot_rom_path = emulation_table["boot_rom"].value_or(emulation.boot_rom_path);
		emulation.skip_boot_rom = emulation_table["skip_boot_rom"].value_or(emulation.skip_boot_rom);
		emulation.allow_sram_saving = emulation_table["allow_sram_saving"].value_or(emulation.allow_sram_saving);
		emulation.sram_save_interval = emulation_table["sram_save_interval"].value_or(emulation.sram_save_interval);
	}

	toml::table Configuration::emulation_settings_as_toml() const
	{
		return toml::table{
			{"boot_rom", emulation.boot_rom_path},
			{"skip_boot_rom", emulation.skip_boot_rom},
			{"allow_sram_saving", emulation.allow_sram_saving},
			{"sram_save_interval", emulation.sram_save_interval},
		};
	}

	void Configuration::load_video_settings(toml::table &video_table)
	{
		video.keep_aspect_ratio = video_table["keep_aspect_ratio"].value_or(video.keep_aspect_ratio);
		video.linear_filtering = video_table["linear_filtering"].value_or(video.linear_filtering);
		auto ct = video_table["color_table"].as_array();
		if (ct)
		{
			for (auto i = 0; (i < ct->size() && i < video.color_table.size()); ++i)
			{
				video.color_table[i] = (*ct)[i].value_or(video.color_table[i]);
			}
		}
	}

	toml::table Configuration::video_settings_as_toml() const
	{
		return toml::table{
			{"keep_aspect_ratio", video.keep_aspect_ratio},
			{"linear_filtering", video.linear_filtering},
			{
				"color_table",
				toml::array{
					video.color_table[0],
					video.color_table[1],
					video.color_table[2],
					video.color_table[3],
				},
			},
		};
	}

	void Configuration::load_audio_settings(toml::table &audio_table)
	{
		audio.latency_select = audio_table["latency_select"].value_or(audio.latency_select);
		audio.master_volume = audio_table["master_volume"].value_or(audio.master_volume);
		audio.master_volume = std::min<uint32_t>(audio.master_volume, 100);
	}

	toml::table Configuration::audio_settings_as_toml() const
	{
		return toml::table{
			{"latency_select", audio.latency_select},
			{"master_volume", audio.master_volume},
		};
	}

	void Configuration::load_input_mapping(toml::array &profiles)
	{
		input_profiles.clear();
		auto deserialize_input_source = [](const toml::table *source_toml_ptr) -> InputSource
		{
			const auto &source_toml = *source_toml_ptr;
			return InputSource{
				static_cast<InputSourceType>(source_toml["type"].value_or(0)),
				SDL_GetScancodeFromName(source_toml["key"].value_or("")),
				{
					source_toml["device_name"].value_or(""),
					SDL_GameControllerGetButtonFromString(source_toml["button"].value_or("")),
					SDL_GameControllerGetAxisFromString(source_toml["axis"].value_or("")),
					static_cast<AxisDirection>(source_toml["axis_direction"].value_or(0)),
				},
			};
		};

		for (auto i = 0; i < profiles.size(); ++i)
		{
			const toml::table *profile_toml_ptr = profiles[i].as_table();
			if (profile_toml_ptr)
			{
				const toml::table &profile_toml = *profile_toml_ptr;

				InputBindingProfile profile{
					profile_toml["name"].value_or(""),
					deserialize_input_source(profile_toml["left"].as_table()),
					deserialize_input_source(profile_toml["right"].as_table()),
					deserialize_input_source(profile_toml["up"].as_table()),
					deserialize_input_source(profile_toml["down"].as_table()),
					deserialize_input_source(profile_toml["a"].as_table()),
					deserialize_input_source(profile_toml["b"].as_table()),
					deserialize_input_source(profile_toml["select"].as_table()),
					deserialize_input_source(profile_toml["start"].as_table()),
				};

				input_profiles.push_back(std::move(profile));
			}
		}
	}

	toml::array Configuration::input_profiles_as_toml() const
	{
		toml::array profile_toml_array;
		auto serialize_input_source = [](const InputSource &source) -> toml::table
		{
			const auto button_string = SDL_GameControllerGetStringForButton(source.controller.button);
			const auto axis_string = SDL_GameControllerGetStringForAxis(source.controller.axis);
			const auto scancode_string = SDL_GetScancodeName(source.key);
			return toml::table{
				{"type", static_cast<uint8_t>(source.type)},
				{"key", scancode_string ? scancode_string : ""},
				{"device_name", source.controller.device_name},
				{"button", button_string ? button_string : ""},
				{"axis", axis_string ? axis_string : ""},
				{"axis_direction", static_cast<uint8_t>(source.controller.axis_direction)},
			};
		};
		for (const auto &profile : input_profiles)
		{
			profile_toml_array.push_back(toml::table{
				{"name", profile.name},
				{"left", serialize_input_source(profile.left)},
				{"right", serialize_input_source(profile.right)},
				{"up", serialize_input_source(profile.up)},
				{"down", serialize_input_source(profile.down)},
				{"a", serialize_input_source(profile.a)},
				{"b", serialize_input_source(profile.b)},
				{"select", serialize_input_source(profile.select)},
				{"start", serialize_input_source(profile.start)},
			});
		}
		return profile_toml_array;
	}
}