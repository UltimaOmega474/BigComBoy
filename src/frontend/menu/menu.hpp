#pragma once
#include "settings_menu.hpp"
#include "menu_bar.hpp"

namespace SunBoy
{
	class EmulationState;
	class MenuController
	{
	public:
		MenuBar menu_bar;
		SettingsMenu settings;
		void toggle_settings(MenuSelect menu);
		void draw(EmulationState &state);
	};

}