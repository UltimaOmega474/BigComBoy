#pragma once
#include "input.hpp"
#include "audio.hpp"
#include <gb/core.hpp>
#include <memory>
#include <SDL.h>
#include <vector>
#include <string_view>

namespace SunBoy
{
	enum class Status
	{
		Stopped,
		Running,
	};

	class EmulationState
	{
		static EmulationState current;
		SDL_Texture *texture = nullptr;

	public:
		SunBoy::Core core;
		KeyboardInput keyboard_input;
		AudioSystem audio_system;
		Status status = Status::Stopped;
		bool paused = false;
		std::shared_ptr<SunBoy::Cartridge> cart;

		void initialize(SDL_Renderer *renderer);
		void close();
		void create_texture(SDL_Renderer *renderer);
		void change_filter_mode(bool use_linear_filter);
		bool try_play(std::string_view path);
		void reset();
		void toggle_pause();
		void step_frame();
		void draw_frame(SDL_Window *window, SDL_Renderer *renderer);
		void init_audio();
		static EmulationState &current_state();
	};
}