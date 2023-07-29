#pragma once
#include "input_menu.hpp"
#include <gb/pad.hpp>
#include <cinttypes>
#include <array>
#include <string_view>
#include <vector>
#include <imgui.h>
namespace SunBoy
{
	enum class MenuSelect
	{
		Emulation,
		Video,
		Audio,
		Input
	};

	class SettingsMenu
	{
	public:
		bool show = false;
		MenuSelect selected_menu = MenuSelect::Emulation;
		InputMenu input_menu;
		void open(MenuSelect menu);
		void draw_menu(float height);
		bool button2(const char *label, bool selected = false);
	};

}