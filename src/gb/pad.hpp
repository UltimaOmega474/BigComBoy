#pragma once
#include <cinttypes>
#include <vector>

namespace GameBoy
{
	class Gamepad
	{
	public:
		uint8_t dpad = 0xFF, action = 0xFF, info_select = 0;

		void select_pad_state(uint8_t value);
		uint8_t get_pad_state();
	};
}