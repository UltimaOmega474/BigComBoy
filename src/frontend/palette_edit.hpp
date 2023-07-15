#pragma once
#include <cinttypes>
#include <array>
#include <string>
namespace AngbeGui
{

	std::array<uint32_t, 4> load_palette_from_file(const std::string &path);

}