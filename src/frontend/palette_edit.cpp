#include "palette_edit.hpp"
#include <gb/constants.hpp>
#include <ranges>
#include <fstream>

namespace AngbeGui
{
	std::array<uint32_t, 4> load_palette_from_file(const std::string &path)
	{
		std::ifstream file(path);
		std::array<uint32_t, 4> palette = Angbe::LCD_GRAY_PALETTE;

		if (file)
		{
			auto reversed = std::ranges::reverse_view(palette);

			for (auto &color : reversed)
			{
				std::string line;
				if (std::getline(file, line))
				{
					if (line == "\r" || line == "\n" || line == "\r\n")
						break;

					color = std::stoul(line, nullptr, 16);

					if (color < 0xFFFFFF)
						color = (color << 8) | 0xFF;

					continue;
				}
				break;
			}
		}

		return palette;
	}
}