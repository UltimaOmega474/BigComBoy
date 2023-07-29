#pragma once
#include <cinttypes>
#include <vector>
namespace SunBoy
{
	class EmulationMenu
	{
		std::vector<char> path_buffer;

	public:
		void open();
		void draw();
	};
}