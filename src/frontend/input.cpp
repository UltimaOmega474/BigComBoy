#include "input.hpp"
#include "state.hpp"
#include "config.hpp"
namespace SunBoy
{
	ControllerHandler::ControllerHandler()
	{
		open();
	}

	ControllerHandler::ControllerHandler(ControllerHandler &&other)
		: controllers(std::move(other.controllers))
	{
	}

	ControllerHandler::~ControllerHandler()
	{
		close();
	}

	ControllerHandler &ControllerHandler::operator=(ControllerHandler &&other)
	{
		controllers = std::move(other.controllers);
		return *this;
	}

	void ControllerHandler::open()
	{
		for (auto i = 0; i < SDL_NumJoysticks(); ++i)
		{
			if (SDL_IsGameController(i))
			{
				auto controller = SDL_GameControllerOpen(i);
				if (controller)
					controllers.push_back(std::make_tuple(i, controller));
			}
		}
	}

	void ControllerHandler::close()
	{
		for (const auto [index, controller] : controllers)
		{
			SDL_GameControllerClose(controller);
		}
		controllers.clear();
	}

	void ControllerHandler::update_state(Gamepad &pad)
	{
		const auto &config = Configuration::get();
		for (const auto &map : config.controller_maps)
		{
			auto controller = get_controller(map.device_name, map.player_index);

			for (auto button : map.left)
			{
				if (SDL_GameControllerGetButton(controller, button))
					pad.set_pad_state(Button::Left, true);
			}
			for (auto button : map.right)
			{
				if (SDL_GameControllerGetButton(controller, button))
					pad.set_pad_state(Button::Right, true);
			}
			for (auto button : map.up)
			{
				if (SDL_GameControllerGetButton(controller, button))
					pad.set_pad_state(Button::Up, true);
			}
			for (auto button : map.down)
			{
				if (SDL_GameControllerGetButton(controller, button))
					pad.set_pad_state(Button::Down, true);
			}
			for (auto button : map.a)
			{
				if (SDL_GameControllerGetButton(controller, button))
					pad.set_pad_state(Button::A, true);
			}
			for (auto button : map.b)
			{
				if (SDL_GameControllerGetButton(controller, button))
					pad.set_pad_state(Button::B, true);
			}
			for (auto button : map.select)
			{
				if (SDL_GameControllerGetButton(controller, button))
					pad.set_pad_state(Button::Select, true);
			}
			for (auto button : map.start)
			{
				if (SDL_GameControllerGetButton(controller, button))
					pad.set_pad_state(Button::Start, true);
			}
		}
	}

	SDL_GameController *ControllerHandler::get_controller(std::string_view name, int32_t player_index)
	{
		for (const auto [index, controller] : controllers)
		{
			if (SDL_GameControllerName(controller) == name && SDL_GameControllerGetPlayerIndex(controller) == player_index)
				return controller;
		}
		return nullptr;
	}

	void KeyboardHandler::update_state(Gamepad &pad)
	{
		const auto keyboard_state = SDL_GetKeyboardState(nullptr);
		const auto &config = Configuration::get();

		for (auto key : config.keyboard_maps.left)
		{
			if (keyboard_state[key])
				pad.set_pad_state(Button::Left, true);
		}
		for (auto key : config.keyboard_maps.right)
		{
			if (keyboard_state[key])
				pad.set_pad_state(Button::Right, true);
		}
		for (auto key : config.keyboard_maps.up)
		{
			if (keyboard_state[key])
				pad.set_pad_state(Button::Up, true);
		}
		for (auto key : config.keyboard_maps.down)
		{
			if (keyboard_state[key])
				pad.set_pad_state(Button::Down, true);
		}
		for (auto key : config.keyboard_maps.a)
		{
			if (keyboard_state[key])
				pad.set_pad_state(Button::A, true);
		}
		for (auto key : config.keyboard_maps.b)
		{
			if (keyboard_state[key])
				pad.set_pad_state(Button::B, true);
		}
		for (auto key : config.keyboard_maps.select)
		{
			if (keyboard_state[key])
				pad.set_pad_state(Button::Select, true);
		}
		for (auto key : config.keyboard_maps.start)
		{
			if (keyboard_state[key])
				pad.set_pad_state(Button::Start, true);
		}
	}
}