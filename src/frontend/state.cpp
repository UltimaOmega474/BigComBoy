#include "state.hpp"
#include "config.hpp"
#include <SDL.h>
#include <intrin.h>
#include <cmath>
namespace SunBoy
{
	EmulationState EmulationState::current{};

	void EmulationState::initialize(SDL_Renderer *renderer)
	{
		audio_system.open_device();
		create_texture(renderer);
	}

	void EmulationState::close()
	{
		audio_system.close_device();
		if (texture)
		{
			SDL_DestroyTexture(texture);
			texture = nullptr;
		}
	}

	void EmulationState::create_texture(SDL_Renderer *renderer)
	{
		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SunBoy::LCD_WIDTH, SunBoy::LCD_HEIGHT);
		auto &config = Configuration::get();

		if (config.linear_filtering)
			SDL_SetTextureScaleMode(texture, SDL_ScaleModeLinear);
		else
			SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest);

		core.ppu.color_table[3] = 0x553840FF;
		core.ppu.color_table[2] = 0x9b6859FF;
		core.ppu.color_table[1] = 0xbebc6aFF;
		core.ppu.color_table[0] = 0xedf8c8FF;
	}

	void EmulationState::change_filter_mode(bool use_linear_filter)
	{
		auto &config = Configuration::get();
		config.linear_filtering = use_linear_filter;

		if (config.linear_filtering)
			SDL_SetTextureScaleMode(texture, SDL_ScaleModeLinear);
		else
			SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest);
	}

	bool EmulationState::try_play(std::string_view path)
	{
		cart = SunBoy::Cartridge::from_file(path);
		auto &config = Configuration::get();
		if (cart)
		{
			paused = false;
			core.settings.skip_boot_rom = config.skip_boot_rom;
			core.settings.boot_rom_path = config.boot_rom_path;
			core.start(cart);
			core.ppu.color_table = config.color_table;
			status = Status::Running;
			audio_system.prep_for_playback(core.apu);

			return true;
		}
		return false;
	}

	void EmulationState::reset()
	{
		auto &config = Configuration::get();
		if (cart)
		{
			paused = false;
			core.settings.skip_boot_rom = config.skip_boot_rom;
			core.settings.boot_rom_path = config.boot_rom_path;
			core.start(cart);
			core.ppu.color_table = config.color_table;
			status = Status::Running;
			audio_system.prep_for_playback(core.apu);
		}
	}

	void EmulationState::toggle_pause()
	{
		if (status == Status::Running)
			paused = !paused;
	}

	void EmulationState::step_frame()
	{
		if (status == Status::Running && !paused)
			core.run_for_frames(1);
	}

	void EmulationState::draw_frame(SDL_Window *window, SDL_Renderer *renderer)
	{
		if (status == Status::Running && texture)
		{
			const auto &framebuffer = core.ppu.framebuffer_complete;
			SDL_UpdateTexture(texture, nullptr, framebuffer.data(), SunBoy::LCD_WIDTH * sizeof(uint32_t));

			int w = 0, h = 0;
			SDL_GetWindowSize(window, &w, &h);

			h = h - MENU_HEIGHT;

			if (Configuration::get().keep_aspect_ratio)
			{
				auto final_width = w, final_height = h;
				auto scaled_w = static_cast<int32_t>(static_cast<float>(w) * (9.0f / 10.0f));
				auto scaled_h = static_cast<int32_t>(static_cast<float>(h) * (10.0f / 9.0f));

				if (scaled_h <= w)
				{
					final_width = scaled_h;
				}
				else if (scaled_w <= h)
				{
					final_height = scaled_w;
				}

				SDL_Rect rect{(w / 2) - (final_width / 2), MENU_HEIGHT, final_width, final_height};
				SDL_RenderCopy(renderer, texture, nullptr, &rect);
			}
			else
			{
				SDL_Rect rect{0, MENU_HEIGHT, w, h};
				SDL_RenderCopy(renderer, texture, nullptr, &rect);
			}
		}
	}

	EmulationState &EmulationState::current_state()
	{
		return current;
	}
}