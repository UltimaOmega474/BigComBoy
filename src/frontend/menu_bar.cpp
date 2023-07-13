#include "menu_bar.hpp"
#include "config.hpp"
#include "gui_constants.hpp"
#include <imgui.h>

void MenuBar::draw()
{
	using namespace ImGui;
	auto &config = Configuration::get_current();

	if (BeginMainMenuBar())
	{
		if (BeginMenu("System"))
		{
			MenuItem("Load Cartridge");

			if (MenuItem("Use Boot Rom", nullptr, config.allow_boot_rom))
				config.allow_boot_rom = !config.allow_boot_rom;
			if (BeginMenu("Load Recent Cartridge", !config.recent_rom_paths.empty()))
			{
				for (const auto &path : config.recent_rom_paths)
				{
					MenuItem(path.c_str());
				}

				EndMenu();
			}
			Separator();
			MenuItem("Reset");
			MenuItem("Pause/Play");
			MenuItem("Stop");

			EndMenu();
		}
		EndMainMenuBar();
	}
}
