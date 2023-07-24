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
		std::string successful_rom_loaded;

		if (BeginMenu("System"))
		{
			if (MenuItem("Load Cartridge"))
			{
				NFD::UniquePath out_path;

				if (NFD::OpenDialog(out_path) == nfdresult_t::NFD_OKAY)
				{
					if (state.try_play(out_path.get()))
						successful_rom_loaded = out_path.get();
				}
			}

			if (BeginMenu("Load Recent Cartridge", !config.recent_rom_paths.empty()))
			{
				for (const auto &path : config.recent_rom_paths)
				{
					if (MenuItem(path.c_str()))
					{
						if (state.try_play(path))
							successful_rom_loaded = path;
					}
				}
				EndMenu();
			}
			EndMenu();
		}

		if (!successful_rom_loaded.empty())
			config.add_rom_path(std::move(successful_rom_loaded));
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
			{
				state.status = Status::Stopped;
				state.cart->save_sram_to_file();
				state.cart.reset();
			}

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
			if (MenuItem("Enable Background", nullptr, state.core.ppu.render_flags & DisplayRenderFlags::Background))
				state.core.ppu.render_flags ^= DisplayRenderFlags::Background;
			if (MenuItem("Enable Window", nullptr, state.core.ppu.render_flags & DisplayRenderFlags::Window))
				state.core.ppu.render_flags ^= DisplayRenderFlags::Window;
			if (MenuItem("Enable Objects", nullptr, state.core.ppu.render_flags & DisplayRenderFlags::Objects))
				state.core.ppu.render_flags ^= DisplayRenderFlags::Objects;
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