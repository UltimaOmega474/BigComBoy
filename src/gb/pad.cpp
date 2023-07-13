#include "pad.hpp"

namespace Angbe
{
	void Gamepad::select_pad_state(uint8_t value)
	{
		info_select = value;
	}

	uint8_t Gamepad::get_pad_state()
	{
		if (info_select & 0x10)
		{
			return action;
		}
		else
		{
			return dpad;
		}
	}
}