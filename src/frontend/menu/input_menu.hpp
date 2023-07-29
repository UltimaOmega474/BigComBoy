#pragma once
#include "../input.hpp"
#include <gb/pad.hpp>
#include <array>
#include <string_view>
#include <cinttypes>
namespace SunBoy
{
	class InputMenu
	{
		std::array<char, 40> name_buffer{};
		bool input_key_modal = false;
		SunBoy::PadButton input_key_target = SunBoy::PadButton::Left;

	public:
		int32_t selected_binding = -1;

		void open();
		void remove_selected_mapping();
		void add_new_mapping();
		void draw();

	private:
		bool draw_input_button(std::string_view unique_name, InputSource &source);
		void draw_keyboard_map(InputBindingProfile &map);
		void any_key_pressed();
		void update_key_mapping(SunBoy::PadButton btn, InputSource source);
	};
}