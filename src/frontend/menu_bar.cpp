#include "menu_bar.hpp"
#include "config.hpp"
#include "state.hpp"
#include "gui_constants.hpp"
#include <imgui.h>
#include <nfd.hpp>

namespace SunBoy
{
	void MenuBar::draw()
	{
		if (!shown)
			return;

		if (ImGui::BeginMainMenuBar())
		{
			draw_system_menu();
			emulation_menu();
			view_menu();
			ImGui::EndMainMenuBar();
		}
	}
	void MenuBar::draw_system_menu()
	{
		using namespace ImGui;
		auto &config = Configuration::get();
		auto &state = EmulationState::current_state();

		if (BeginMenu("System"))
		{
			if (MenuItem("Load Cartridge"))
			{
				NFD::UniquePath out_path;

				if (NFD::OpenDialog(out_path) == nfdresult_t::NFD_OKAY)
					state.try_play(out_path.get());
			}

			if (BeginMenu("Load Recent Cartridge", !config.recent_rom_paths.empty()))
			{
				for (const auto &path : config.recent_rom_paths)
				{
					if (MenuItem(path.c_str()))
					{
						state.try_play(path);
					}
				}

				EndMenu();
			}
			EndMenu();
		}
	}
	void MenuBar::emulation_menu()
	{
		using namespace ImGui;
		auto &config = Configuration::get();
		auto &state = EmulationState::current_state();
		if (BeginMenu("Emulation"))
		{
			if (MenuItem("Use Boot Rom", nullptr, !config.skip_boot_rom))
				config.skip_boot_rom = !config.skip_boot_rom;

			if (MenuItem("Reset"))
				state.reset();

			if (MenuItem("Pause/Play", nullptr, state.paused))
				state.toggle_pause();

			if (MenuItem("Stop"))
				state.status = Status::Stopped;
			EndMenu();
		}
	}
	void MenuBar::view_menu()
	{
		using namespace ImGui;
		auto &config = Configuration::get();
		auto &state = EmulationState::current_state();
		if (BeginMenu("View"))
		{
			TextColored(INACTIVE_ITEM_COLOR, "PPU Overrides");
			Spacing();
			MenuItem("Enable Background");
			MenuItem("Enable Window");
			MenuItem("Enable OAM");
			Separator();
			TextColored(INACTIVE_ITEM_COLOR, "Screen");
			Spacing();
			MenuItem("Scale");

			if (MenuItem("Smooth Filter", nullptr, config.linear_filtering))
				state.change_filter_mode(!config.linear_filtering);

			MenuItem("Palette Editor");
			EndMenu();
		}
	}
}