#pragma once

namespace SunBoy
{
	class EmulationState;
	class MenuBar
	{
	public:
		bool shown = true;
		void draw(EmulationState &state);

		void draw_system_menu(EmulationState &state);
		void emulation_menu(EmulationState &state);
		void view_menu(EmulationState &state);
	};
}