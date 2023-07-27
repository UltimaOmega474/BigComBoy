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
	struct KeyboardMapping
	{
		std::vector<SDL_Scancode> left{SDL_SCANCODE_LEFT}, right{SDL_SCANCODE_RIGHT}, up{SDL_SCANCODE_UP}, down{SDL_SCANCODE_DOWN};
		std::vector<SDL_Scancode> a{SDL_SCANCODE_Z}, b{SDL_SCANCODE_X}, select{SDL_SCANCODE_RSHIFT}, start{SDL_SCANCODE_RETURN};
	};

	struct ControllerMapping
	{
		std::string device_name;
		int32_t player_index = -1; // not all controllers support this

		std::vector<SDL_GameControllerButton> left, right, up, down;
		std::vector<SDL_GameControllerButton> a, b, select, start;
	};

	class ControllerHandler
	{
		std::vector<std::tuple<int32_t, SDL_GameController *>> controllers;

	public:
		ControllerHandler();
		ControllerHandler(const ControllerHandler &) = delete;
		ControllerHandler(ControllerHandler &&other);
		~ControllerHandler();
		ControllerHandler &operator=(const ControllerHandler &) = delete;
		ControllerHandler &operator=(ControllerHandler &&);
		void open();
		void close();

		void update_state(Gamepad &pad);

	private:
		SDL_GameController *get_controller(std::string_view name, int32_t player_index);
	};

	class KeyboardHandler
	{
	public:
		void update_state(Gamepad &pad);
	};

}