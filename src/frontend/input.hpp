#pragma once
#include <gb/pad.hpp>
#include <SDL.h>
#include <vector>
#include <tuple>

namespace SunBoy
{
	class KeyboardInput
	{
	public:
		std::vector<std::tuple<SunBoy::Button, SDL_Scancode>> key_map{
			std::make_tuple(SunBoy::Button::Left, SDL_SCANCODE_LEFT),
			std::make_tuple(SunBoy::Button::Right, SDL_SCANCODE_RIGHT),
			std::make_tuple(SunBoy::Button::Up, SDL_SCANCODE_UP),
			std::make_tuple(SunBoy::Button::Down, SDL_SCANCODE_DOWN),

			std::make_tuple(SunBoy::Button::A, SDL_SCANCODE_Z),
			std::make_tuple(SunBoy::Button::B, SDL_SCANCODE_X),
			std::make_tuple(SunBoy::Button::Select, SDL_SCANCODE_SPACE),
			std::make_tuple(SunBoy::Button::Start, SDL_SCANCODE_RETURN),
		};

		void update_state(SunBoy::Gamepad &pad);
	};
}