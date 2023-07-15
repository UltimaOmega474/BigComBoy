#pragma once
#include <gb/pad.hpp>
#include <SDL.h>
#include <vector>
#include <tuple>

namespace AngbeGui
{
	class KeyboardInput
	{
	public:
		std::vector<std::tuple<Angbe::Button, SDL_Scancode>> key_map{
			std::make_tuple(Angbe::Button::Left, SDL_SCANCODE_LEFT),
			std::make_tuple(Angbe::Button::Right, SDL_SCANCODE_RIGHT),
			std::make_tuple(Angbe::Button::Up, SDL_SCANCODE_UP),
			std::make_tuple(Angbe::Button::Down, SDL_SCANCODE_DOWN),

			std::make_tuple(Angbe::Button::A, SDL_SCANCODE_Z),
			std::make_tuple(Angbe::Button::B, SDL_SCANCODE_X),
			std::make_tuple(Angbe::Button::Select, SDL_SCANCODE_SPACE),
			std::make_tuple(Angbe::Button::Start, SDL_SCANCODE_RETURN),
		};

		void update_state(Angbe::Gamepad &pad);
	};
}