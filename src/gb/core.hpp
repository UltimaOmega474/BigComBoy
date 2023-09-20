#pragma once
#include "sm83.hpp"
#include "cartridge.hpp"
#include "timer.hpp"
#include "ppu.hpp"
#include "pad.hpp"
#include "apu.hpp"
#include "bus.hpp"
#include <memory>
#include <ostream>
namespace SunBoy
{

	struct CoreSettings
	{
		std::string boot_rom_path;
		bool skip_boot_rom = false;
	};

	class Core
	{
		uint32_t cycle_count = 0;

	public:
		Gamepad pad;
		PPU ppu;
		APU apu;
		Timer timer;
		MainBus bus;
		SM83 cpu;
		CoreSettings settings;

		Core();

		void start(Cartridge *cart);
		void reset();
		void run_for_frames(uint32_t frames);
		void run_for_cycles(uint32_t cycles);
		void tick_subcomponents(uint8_t cycles);
		void load_boot_rom_from_file();
	};

}