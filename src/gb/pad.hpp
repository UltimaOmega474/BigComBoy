#pragma once
#include <cinttypes>
#include <vector>

namespace Angbe
{
	enum class Button
	{
		Right,
		Left,
		Up,
		Down,
		A,
		B,
		Select,
		Start
	};

	class Gamepad
	{
	public:
		uint8_t dpad = 0xFF, action = 0xFF, mode = 0;

		void set_pad_state(Button btn, bool pressed);
		void select_button_mode(uint8_t value);
		uint8_t get_pad_state();
	};
}