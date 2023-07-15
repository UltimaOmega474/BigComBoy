#include "input.hpp"
#include "state.hpp"

namespace AngbeGui
{
	void KeyboardInput::update_state(Angbe::Gamepad &pad)
	{
		auto keyboard = SDL_GetKeyboardState(nullptr);

		for (const auto &[gb_btn, key] : key_map)
		{
			pad.set_pad_state(gb_btn, static_cast<bool>(keyboard[key]));
		}
	}
}