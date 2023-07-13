#include "menu_bar.hpp"
#include "config.hpp"
#include "state.hpp"
#include "gui_constants.hpp"
#include <imgui.h>
#include <nfd.hpp>

namespace AngbeGui
{
	void MenuBar::draw()
	{
		using namespace ImGui;
		auto &config = Configuration::get();
		auto &state = EmulationState::current_state();

		if (BeginMainMenuBar() && shown)
		{
			if (BeginMenu("System"))
			{
				if (MenuItem("Load Cartridge"))
				{
					NFD::UniquePathN out_path;

					if (NFD::OpenDialog(out_path) == nfdresult_t::NFD_OKAY)
						state.try_play(out_path.get());
				}

				if (MenuItem("Use Boot Rom", nullptr, !config.skip_boot_rom))
					config.skip_boot_rom = !config.skip_boot_rom;

				if (BeginMenu("Load Recent Cartridge", !config.recent_rom_paths.empty()))
				{
					for (const auto &path : config.recent_rom_paths)
						MenuItem(path.c_str());

					EndMenu();
				}

				Separator();

				if (MenuItem("Reset"))
					state.reset();

				if (MenuItem("Pause/Play", nullptr, state.paused))
					state.toggle_pause();

				if (MenuItem("Stop"))
					state.status = Status::Stopped;

				EndMenu();
			}
			EndMainMenuBar();
		}
	}
}