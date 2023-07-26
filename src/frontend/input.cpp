#include "input.hpp"
#include "state.hpp"

namespace SunBoy
{
	ControllerHandler::ControllerHandler(ControllerHandler &&other)
		: controller_id(other.controller_id), controller(other.controller), gamepad_maps(std::move(other.gamepad_maps))
	{
		other.controller = nullptr;
		other.controller_id = -1;
	}

	ControllerHandler::~ControllerHandler()
	{
		close();
	}

	ControllerHandler &ControllerHandler::operator=(ControllerHandler &&other)
	{
		controller_id = other.controller_id;
		controller = other.controller;

		other.controller = nullptr;
		other.controller_id = -1;

		gamepad_maps = std::move(other.gamepad_maps);
		return *this;
	}

	bool ControllerHandler::open(int32_t id)
	{
		auto saved = controller_id;
		auto result = open_with_existing_id();
		controller_id = id;

		if (open_with_existing_id())
			return true;

		controller_id = saved;
		return false;
	}

	bool ControllerHandler::open_with_existing_id()
	{
		if ((controller_id < 0) || (controller_id >= SDL_NumJoysticks()))
			return false;

		if (SDL_IsGameController(controller_id) && !controller)
		{
			controller = SDL_GameControllerOpen(controller_id);
			return true;
		}

		return false;
	}

	void ControllerHandler::close()
	{
		if (controller)
		{
			SDL_GameControllerClose(controller);
			controller = nullptr;
			controller_id = -1;
		}
	}

	int32_t ControllerHandler::get_id() const
	{
		return controller_id;
	}

	void ControllerHandler::update_state(Gamepad &pad)
	{
		if (controller)
		{
			for (const auto &map : gamepad_maps)
			{
				if (SDL_GameControllerGetButton(controller, map.left))
					pad.set_pad_state(Button::Left, true);
				if (SDL_GameControllerGetButton(controller, map.right))
					pad.set_pad_state(Button::Right, true);
				if (SDL_GameControllerGetButton(controller, map.up))
					pad.set_pad_state(Button::Up, true);
				if (SDL_GameControllerGetButton(controller, map.down))
					pad.set_pad_state(Button::Down, true);

				if (SDL_GameControllerGetButton(controller, map.a))
					pad.set_pad_state(Button::A, true);
				if (SDL_GameControllerGetButton(controller, map.b))
					pad.set_pad_state(Button::B, true);
				if (SDL_GameControllerGetButton(controller, map.select))
					pad.set_pad_state(Button::Select, true);
				if (SDL_GameControllerGetButton(controller, map.start))
					pad.set_pad_state(Button::Start, true);
			}
		}
	}

	void KeyboardHandler::update_state(Gamepad &pad)
	{
		const auto keyboard_state = SDL_GetKeyboardState(nullptr);
		for (const auto &map : keyboard_maps)
		{
			if (keyboard_state[map.left])
				pad.set_pad_state(Button::Left, true);
			if (keyboard_state[map.right])
				pad.set_pad_state(Button::Right, true);
			if (keyboard_state[map.up])
				pad.set_pad_state(Button::Up, true);
			if (keyboard_state[map.down])
				pad.set_pad_state(Button::Down, true);

			if (keyboard_state[map.a])
				pad.set_pad_state(Button::A, true);
			if (keyboard_state[map.b])
				pad.set_pad_state(Button::B, true);
			if (keyboard_state[map.select])
				pad.set_pad_state(Button::Select, true);
			if (keyboard_state[map.start])
				pad.set_pad_state(Button::Start, true);
		}
	}
}