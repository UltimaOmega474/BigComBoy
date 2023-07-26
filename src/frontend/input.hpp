#pragma once
#include <gb/pad.hpp>
#include <SDL.h>
#include <vector>
#include <tuple>
#include <cinttypes>

namespace SunBoy
{
	struct KeyboardMapping
	{
		SDL_Scancode left = SDL_SCANCODE_LEFT, right = SDL_SCANCODE_RIGHT;
		SDL_Scancode up = SDL_SCANCODE_UP, down = SDL_SCANCODE_DOWN;
		SDL_Scancode a = SDL_SCANCODE_Z, b = SDL_SCANCODE_X;
		SDL_Scancode select = SDL_SCANCODE_RSHIFT, start = SDL_SCANCODE_RETURN;
	};

	struct GamepadMapping
	{
		int32_t controller_id = -1;
		SDL_GameController *controller = nullptr;

		SDL_GameControllerButton left = SDL_CONTROLLER_BUTTON_DPAD_LEFT, right = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
		SDL_GameControllerButton up = SDL_CONTROLLER_BUTTON_DPAD_UP, down = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
		SDL_GameControllerButton a = SDL_CONTROLLER_BUTTON_A, b = SDL_CONTROLLER_BUTTON_B;
		SDL_GameControllerButton select = SDL_CONTROLLER_BUTTON_BACK, start = SDL_CONTROLLER_BUTTON_START;
	};

	class ControllerHandler
	{
		int32_t controller_id = -1;
		SDL_GameController *controller = nullptr;

	public:
		std::vector<GamepadMapping> gamepad_maps{};
		ControllerHandler() = default;
		ControllerHandler(const ControllerHandler &) = delete;
		ControllerHandler(ControllerHandler &&other);
		~ControllerHandler();
		ControllerHandler &operator=(const ControllerHandler &) = delete;
		ControllerHandler &operator=(ControllerHandler &&);

		bool open(int32_t id);
		bool open_with_existing_id();
		void close();
		int32_t get_id() const;
		void update_state(Gamepad &pad);
	};

	class KeyboardHandler
	{
	public:
		std::vector<KeyboardMapping> keyboard_maps{KeyboardMapping()};

		void update_state(Gamepad &pad);
	};

}