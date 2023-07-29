#include "menu.hpp"
#include "../config.hpp"
namespace SunBoy
{
	void MenuController::toggle_settings(MenuSelect menu)
	{
		if (settings.show && settings.selected_menu == menu)
		{
			Configuration::get().save_as_toml_file();
			settings.show = false;
			return;
		}
		settings.open(menu);
	}

	void MenuController::draw(EmulationState &state)
	{
		menu_bar.draw(state, *this);

		settings.draw_menu(menu_bar.height);
	}
}