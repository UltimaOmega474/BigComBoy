#include "input.hpp"
#include "state.hpp"
#include "config.hpp"
namespace SunBoy
{
	std::vector<SDL_GameController *> ControllerHandler::controllers;

	void ControllerHandler::open()
	{
		for (auto i = 0; i < SDL_NumJoysticks(); ++i)
		{
			if (SDL_IsGameController(i))
			{
				SDL_GameController *controller = SDL_GameControllerOpen(i);
				if (controller)
					controllers.push_back(controller);
			}
		}
	}

	void ControllerHandler::close()
	{
		for (const auto controller : controllers)
		{
			SDL_GameControllerClose(controller);
		}
		controllers.clear();
	}

	const std::vector<SDL_GameController *> &ControllerHandler::get_controllers()
	{
		return controllers;
	}

	void InputHandler::update_state(Gamepad &pad)
	{
		const Uint8 *keyboard = SDL_GetKeyboardState(nullptr);
		const auto &config = Configuration::get();

		for (const auto &map : config.input_profiles)
		{
			do_input(pad, PadButton::Left, map.left, keyboard);
			do_input(pad, PadButton::Right, map.right, keyboard);
			do_input(pad, PadButton::Up, map.up, keyboard);
			do_input(pad, PadButton::Down, map.down, keyboard);
			do_input(pad, PadButton::A, map.a, keyboard);
			do_input(pad, PadButton::B, map.b, keyboard);
			do_input(pad, PadButton::Select, map.select, keyboard);
			do_input(pad, PadButton::Start, map.start, keyboard);
		}
	}

	void InputHandler::do_input(Gamepad &pad, PadButton btn, const InputSource &source, const Uint8 *keyboard)
	{
		switch (source.type)
		{
		case InputSourceType::Keyboard:
			do_keyboard_input(pad, btn, source.key, keyboard);
			break;
		case InputSourceType::ControllerButton:
			do_controller_button_input(pad, btn, source);
			break;
		case InputSourceType::ControllerAxis:
			do_controller_axis_input(pad, btn, source);
			break;
		}
	}

	void InputHandler::do_keyboard_input(Gamepad &pad, PadButton btn, SDL_Scancode key, const Uint8 *keyboard)
	{
		if (keyboard[key])
			pad.set_pad_state(btn, true);
	}

	void InputHandler::do_controller_button_input(Gamepad &pad, PadButton btn, const InputSource &controller_btn)
	{
		for (const auto controller : ControllerHandler::get_controllers())
		{
			if (SDL_GameControllerName(controller) == controller_btn.controller.device_name)
			{
				if (SDL_GameControllerGetButton(controller, controller_btn.controller.button))
					pad.set_pad_state(btn, true);

				return;
			}
		}
	}

	void InputHandler::do_controller_axis_input(Gamepad &pad, PadButton btn, const InputSource &controller_btn)
	{
		for (const auto controller : ControllerHandler::get_controllers())
		{
			if (SDL_GameControllerName(controller) == controller_btn.controller.device_name)
			{
				Sint16 axis_value = SDL_GameControllerGetAxis(controller, controller_btn.controller.axis);
				if ((axis_value > 10000 && controller_btn.controller.axis_direction == AxisDirection::Positive) ||
					(axis_value < -10000 && controller_btn.controller.axis_direction == AxisDirection::Negative))
				{
					pad.set_pad_state(btn, true);
				}

				return;
			}
		}
	}
}