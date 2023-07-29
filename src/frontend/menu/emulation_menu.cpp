#include "emulation_menu.hpp"
#include "../config.hpp"
#include <imgui.h>
#include <nfd.hpp>
namespace SunBoy
{
	void EmulationMenu::open()
	{
		auto &config = Configuration::get();
		path_buffer.clear();

		path_buffer.resize(config.emulation.boot_rom_path.size() + 1, 0);
		std::copy(config.emulation.boot_rom_path.begin(), config.emulation.boot_rom_path.end(), path_buffer.begin());
	}

	void EmulationMenu::draw()
	{
		using namespace ImGui;
		auto &config = Configuration::get();

		auto cb = [](ImGuiInputTextCallbackData *data) -> int32_t
		{
			if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
			{
				auto str = reinterpret_cast<std::vector<char> *>(data->UserData);
				str->resize(data->BufSize);
				data->Buf = str->data();
			}
			return 0;
		};

		if (BeginTable("Emulation Toggles", 2, ImGuiTableFlags_SizingFixedFit))
		{
			TableNextColumn();
			Checkbox("Allow RAM saving", &config.emulation.allow_sram_saving);
			TableNextColumn();
			SetNextItemWidth(100);
			InputInt("Save Interval (Seconds)", reinterpret_cast<int32_t *>(&config.emulation.sram_save_interval), 5, 100, config.emulation.allow_sram_saving ? 0 : ImGuiInputTextFlags_ReadOnly);
			TableNextRow();
			TableNextColumn();
			Checkbox("Skip Boot Sequence", &config.emulation.skip_boot_rom);
			EndTable();
		}
		AlignTextToFramePadding();
		Text("Boot Rom Path:");
		SameLine();
		SetNextItemWidth(300);
		PushID("Boot Rom Path Input");
		if (InputText("", path_buffer.data(), path_buffer.size(), ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_AutoSelectAll, cb, &path_buffer))
		{
			config.emulation.boot_rom_path = path_buffer.data();
		}
		PopID();
		SameLine();
		if (Button("Browse"))
		{
			NFD::UniquePath out_path;

			if (NFD::OpenDialog(out_path) == nfdresult_t::NFD_OKAY)
			{
				config.emulation.boot_rom_path = out_path.get();
				open();
			}
		}
	}
}