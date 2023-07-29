#include "input_menu.hpp"
#include "../config.hpp"
#include <imgui.h>

namespace SunBoy
{
	void InputMenu::open()
	{
		const auto &config = Configuration::get();
		selected_binding = (config.input_profiles.size() > 0) ? 0 : -1;
		input_key_modal = false;
		name_buffer.fill('\0');
		if (selected_binding == -1)
			name_buffer = std::array<char, 40>{'D', 'e', 'f', 'a', 'u', 'l', 't', '\0'};
	}

	void InputMenu::remove_selected_mapping()
	{
		if (selected_binding == -1)
			return;
		auto &config = Configuration::get();

		config.input_profiles.erase(config.input_profiles.begin() + selected_binding);

		selected_binding--;
	}

	void InputMenu::add_new_mapping()
	{
		if (name_buffer[0] == '\0')
			return;
		auto &config = Configuration::get();
		InputBindingProfile map;
		map.name = name_buffer.data();
		config.input_profiles.push_back(std::move(map));
		selected_binding = config.input_profiles.size() - 1;
	}

	void InputMenu::draw()
	{
		using namespace ImGui;
		auto &config = Configuration::get();
		bool valid_index = (selected_binding >= 0) && (selected_binding < config.input_profiles.size());

		{
			Text("Binding Profile");
			SameLine();
			PushID("Binding Profile");
			SetNextItemWidth(160.0f);
			if (BeginCombo("", valid_index ? config.input_profiles[selected_binding].name.c_str() : ""))
			{
				for (auto i = 0; i < config.input_profiles.size(); i++)
				{
					PushID(i);
					if (Selectable(config.input_profiles[i].name.c_str(), selected_binding == i))
					{
						selected_binding = i;
					}
					PopID();
				}

				EndCombo();
			}
			PopID();

			SameLine();
			if (ImGui::Button("Remove", ImVec2(100, 0)))
				remove_selected_mapping();
		}

		{
			ImVec2 profile_text_size = CalcTextSize("Binding Profile");
			ImVec2 name_text_size = CalcTextSize("Name  ");
			Text("Name");
			SameLine();
			Dummy(ImVec2(profile_text_size.x - name_text_size.x, 0));
			SameLine();
			SetNextItemWidth(160.0f);
			PushID("Name Field");
			InputText("", name_buffer.data(), name_buffer.size());
			PopID();
			SameLine();
			if (ImGui::Button("Add", ImVec2(100, 0)))
				add_new_mapping();
		}

		for (auto i = 0; i < 4; ++i)
			Spacing();

		if (valid_index)
		{
			draw_keyboard_map(config.input_profiles[selected_binding]);
		}
		if (input_key_modal)
			OpenPopup("Get Key Modal");

		if (BeginPopupModal("Get Key Modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration))
		{
			if (IsMouseClicked(ImGuiMouseButton_Left) && !IsWindowHovered())
			{
				input_key_modal = false;
				CloseCurrentPopup();
			}
			Text("Press any key...");
			Spacing();
			TextColored(MENU_TEXT_DARK, "(Press Escape/Click outside this box to cancel)");
			any_key_pressed();
			EndPopup();
		}
	}

	bool InputMenu::draw_input_button(std::string_view unique_name, InputSource &source)
	{
		using namespace ImGui;
		bool pushed = false;

		PushID(unique_name.data());
		switch (source.type)
		{
		case InputSourceType::Keyboard:
		{
			pushed = Button((source.key == SDL_SCANCODE_UNKNOWN) ? "Not Set" : SDL_GetScancodeName(source.key), ImVec2(80, 0));
			break;
		}
		case InputSourceType::ControllerButton:
		{
			const char *btn_string = SDL_GameControllerGetStringForButton(source.controller.button);

			std::string name = "Pad: ";
			name += btn_string ? btn_string : "";
			pushed = Button((source.controller.button == SDL_CONTROLLER_BUTTON_INVALID) ? "Not Set" : name.c_str(), ImVec2(80, 0));
			break;
		}
		case InputSourceType::ControllerAxis:
		{
			const char *axis_string = SDL_GameControllerGetStringForAxis(source.controller.axis);

			std::string name = "Pad: ";
			name += axis_string ? axis_string : "";
			name += source.controller.axis_direction == AxisDirection::Positive ? "+" : "-";

			pushed = Button((source.controller.axis == SDL_CONTROLLER_AXIS_INVALID) ? "Not Set" : name.c_str(), ImVec2(80, 0));
			break;
		}
		}
		PopID();

		return pushed;
	}

	void InputMenu::draw_keyboard_map(InputBindingProfile &map)
	{
		using namespace ImGui;
		if (BeginTable("Keyboard Bindings", 2, ImGuiTableFlags_SizingStretchProp))
		{
			TableNextColumn();
			AlignTextToFramePadding();
			Text("Left");
			if (draw_input_button("Left Button", map.left) && !input_key_modal)
			{
				input_key_modal = true;
				input_key_target = PadButton::Left;
			}
			TableNextColumn();
			Text("A");
			if (draw_input_button("A Button", map.a) && !input_key_modal)
			{
				input_key_modal = true;
				input_key_target = PadButton::A;
			}
			TableNextRow();
			TableNextColumn();
			Text("Right");
			if (draw_input_button("Right Button", map.right) && !input_key_modal)
			{
				input_key_modal = true;
				input_key_target = PadButton::Right;
			}
			TableNextColumn();
			Text("B");
			if (draw_input_button("B Button", map.b) && !input_key_modal)
			{
				input_key_modal = true;
				input_key_target = PadButton::B;
			}
			TableNextRow();
			TableNextColumn();
			Text("Up");
			if (draw_input_button("Up Button", map.up) && !input_key_modal)
			{
				input_key_modal = true;
				input_key_target = PadButton::Up;
			}
			TableNextColumn();
			Text("Select");
			if (draw_input_button("Select Button", map.select) && !input_key_modal)
			{
				input_key_modal = true;
				input_key_target = PadButton::Select;
			}
			TableNextRow();
			TableNextColumn();
			Text("Down");
			if (draw_input_button("Down Button", map.down) && !input_key_modal)
			{
				input_key_modal = true;
				input_key_target = PadButton::Down;
			}
			TableNextColumn();
			Text("Start");
			if (draw_input_button("Start Button", map.start) && !input_key_modal)
			{
				input_key_modal = true;
				input_key_target = PadButton::Start;
			}
			EndTable();
		}
	}

	void InputMenu::any_key_pressed()
	{
		if (!input_key_modal)
			return;
		int32_t num_keys = 0;
		const Uint8 *keyboard = SDL_GetKeyboardState(&num_keys);

		for (auto i = 0; i < num_keys; ++i)
		{
			if (keyboard[i])
			{
				input_key_modal = false;
				ImGui::CloseCurrentPopup();

				if (i == SDL_SCANCODE_ESCAPE || SDL_SCANCODE_BACKSPACE)
					return;

				InputSource source;
				source.type = InputSourceType::Keyboard;
				source.key = static_cast<SDL_Scancode>(i);
				update_key_mapping(input_key_target, std::move(source));
				return;
			}
		}

		for (const auto controller : ControllerHandler::get_controllers())
		{
			for (auto i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i)
			{
				auto btn = static_cast<SDL_GameControllerButton>(i);
				if (SDL_GameControllerGetButton(controller, btn))
				{
					input_key_modal = false;
					ImGui::CloseCurrentPopup();

					InputSource source;
					source.controller.device_name = SDL_GameControllerName(controller);
					source.type = InputSourceType::ControllerButton;
					source.controller.button = btn;
					update_key_mapping(input_key_target, std::move(source));
					return;
				}
			}

			for (auto i = 0; i < SDL_CONTROLLER_AXIS_MAX; ++i)
			{
				auto axis = static_cast<SDL_GameControllerAxis>(i);
				Sint16 axis_value = SDL_GameControllerGetAxis(controller, axis);
				if (axis_value != 0)
				{

					InputSource source;
					source.controller.device_name = SDL_GameControllerName(controller);
					source.type = InputSourceType::ControllerAxis;
					source.controller.axis = axis;

					if (axis_value > 10000)
						source.controller.axis_direction = AxisDirection::Positive;
					else if (axis_value < -10000)
						source.controller.axis_direction = AxisDirection::Negative;
					else
						continue;
					input_key_modal = false;
					ImGui::CloseCurrentPopup();
					update_key_mapping(input_key_target, std::move(source));
					return;
				}
			}
		}
	}

	void InputMenu::update_key_mapping(SunBoy::PadButton btn, InputSource source)
	{
		auto &config = Configuration::get();
		auto &map = config.input_profiles[selected_binding];
		switch (btn)
		{
		case PadButton::Left:
			map.left = std::move(source);
			break;
		case PadButton::Right:
			map.right = std::move(source);
			break;
		case PadButton::Up:
			map.up = std::move(source);
			break;
		case PadButton::Down:
			map.down = std::move(source);
			break;
		case PadButton::A:
			map.a = std::move(source);
			break;
		case PadButton::B:
			map.b = std::move(source);
			break;
		case PadButton::Select:
			map.select = std::move(source);
			break;
		case PadButton::Start:
			map.start = std::move(source);
			break;
		}
	}

}