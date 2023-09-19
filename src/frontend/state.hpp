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
		SDL_Texture *texture = nullptr;
		SDL_Window *_window = nullptr;

	public:
		Status status = Status::Stopped;
		bool paused = false;
		int32_t menu_bar_height = 0.0f;
		SunBoy::Core core;
		InputHandler user_input;
		ControllerHandler controllers;
		AudioSystem audio_system;
		std::unique_ptr<SunBoy::Cartridge> cart;

		EmulationState() = default;
		EmulationState(SDL_Window *window);
		EmulationState(const EmulationState &) = delete;
		EmulationState(EmulationState &&) = delete;
		~EmulationState();
		EmulationState &operator=(const EmulationState &) = delete;
		EmulationState &operator=(EmulationState &&) = delete;

		void initialize(SDL_Renderer *renderer);
		void close();
		SDL_Window *window() const;
		void create_texture(SDL_Renderer *renderer);
		void change_filter_mode(bool use_linear_filter);
		bool try_play(std::string_view path);
		void reset();
		void toggle_pause();
		void poll_input();
		void step_frame();
		void draw_frame(SDL_Window *window, SDL_Renderer *renderer);
	};
}