#pragma once

namespace SunBoy
{
	class MenuBar
	{
	public:
		bool shown = true;
		void draw();

		void draw_system_menu();
		void emulation_menu();
		void view_menu();
	};
}