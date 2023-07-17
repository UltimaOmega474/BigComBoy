#pragma once
#include "input.hpp"
#include <string>
#include <gb/core.hpp>
#include <memory>

struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Window;

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
		Status status = Status::Stopped;
		bool paused = false;
		std::shared_ptr<SunBoy::Cartridge> cart;

		~EmulationState();

		void create_texture(SDL_Renderer *renderer);

		bool try_play(std::string path);
		void reset();
		void toggle_pause();
		void step_frame();
		void draw_frame(SDL_Window *window, SDL_Renderer *renderer);

		static EmulationState &current_state();
	};
}