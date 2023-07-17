#include "pad.hpp"

namespace SunBoy
{
	void Gamepad::set_pad_state(Button btn, bool pressed)
	{
		if (pressed)
		{
			switch (btn)
			{
			case Button::Right:
				dpad &= ~0x1;
				break;
			case Button::Left:
				dpad &= ~0x2;
				break;
			case Button::Up:
				dpad &= ~0x4;
				break;
			case Button::Down:
				dpad &= ~0x8;
				break;

			case Button::A:
				action &= ~0x1;
				break;
			case Button::B:
				action &= ~0x2;
				break;
			case Button::Select:
				action &= ~0x4;
				break;
			case Button::Start:
				action &= ~0x8;
				break;
			}
		}
		else
		{
			switch (btn)
			{
			case Button::Right:
				dpad |= 0x1;
				break;
			case Button::Left:
				dpad |= 0x2;
				break;
			case Button::Up:
				dpad |= 0x4;
				break;
			case Button::Down:
				dpad |= 0x8;
				break;

			case Button::A:
				action |= 0x1;
				break;
			case Button::B:
				action |= 0x2;
				break;
			case Button::Select:
				action |= 0x4;
				break;
			case Button::Start:
				action |= 0x8;
				break;
			}
		}
	}

	void Gamepad::select_button_mode(uint8_t value)
	{
		mode = value;
	}

	uint8_t Gamepad::get_pad_state()
	{
		if (mode & 0x10)
		{
			return action;
		}
		else
		{
			return dpad;
		}
	}
}