#pragma once

namespace SunBoy
{
	class EmulationState;
	class MenuController;
	class MenuBar
	{
	public:
		bool shown = true;
		bool ready_to_exit = false;
		float height = 0.0f;
		void draw(EmulationState &state, MenuController &menu_controller);

		void system_menu(EmulationState &state, MenuController &menu_controller);
		void emulation_menu(EmulationState &state);
		void view_menu(EmulationState &state);
	};

}