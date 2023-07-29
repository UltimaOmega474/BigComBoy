#pragma once
#include <gb/pad.hpp>
#include <SDL.h>
#include <vector>
#include <tuple>
#include <cinttypes>
#include <string>
#include <string_view>
namespace SunBoy
{
	enum class InputSourceType
	{
		Keyboard,
		ControllerButton,
		ControllerAxis
	};

	enum class AxisDirection
	{
		Positive,
		Negative,
	};

	struct InputSource
	{
		InputSourceType type = InputSourceType::Keyboard;
		SDL_Scancode key = SDL_SCANCODE_UNKNOWN;
		struct Controller
		{
			std::string device_name;
			SDL_GameControllerButton button = SDL_CONTROLLER_BUTTON_INVALID;
			SDL_GameControllerAxis axis = SDL_CONTROLLER_AXIS_INVALID;
			AxisDirection axis_direction = AxisDirection::Positive;
		} controller;
	};

	struct InputBindingProfile
	{
		std::string name;
		InputSource left, right, up, down;
		InputSource a, b, select, start;
	};

	class ControllerHandler
	{
		static std::vector<SDL_GameController *> controllers;

	public:
		static void open();
		static void close();
		static const std::vector<SDL_GameController *> &get_controllers();

	private:
		SDL_GameController *get_controller(std::string_view name, int32_t player_index);
	};

	class InputHandler
	{
	public:
		void update_state(Gamepad &pad);

	private:
		void do_input(Gamepad &pad, PadButton btn, const InputSource &source, const Uint8 *keyboard);
		void do_keyboard_input(Gamepad &pad, PadButton btn, SDL_Scancode key, const Uint8 *keyboard);
		void do_controller_button_input(Gamepad &pad, PadButton btn, const InputSource &controller_btn);
		void do_controller_axis_input(Gamepad &pad, PadButton btn, const InputSource &controller_btn);
	};

}