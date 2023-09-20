#include "core.hpp"
#include "constants.hpp"
#include <fstream>

namespace SunBoy
{
	Core::Core()
		: bus(*this), cpu(*this, bus), timer(*this), ppu(bus)
	{
	}

	void Core::start(Cartridge *cart)
	{
		reset();
		bus.cart = cart;
		if (settings.skip_boot_rom)
		{
			bus.boot_rom_enabled = false;
			cpu.reset(0x100);
			ppu.set_post_boot_state();
		}
		else
		{
			bus.boot_rom_enabled = true;
			cpu.reset(0x0);
			load_boot_rom_from_file();
		}
	}

	void Core::reset()
	{
		bus.reset();
		cpu.reset(0);
		apu.reset();
		ppu.reset(true);
		timer.reset();
		pad.reset();
	}

	void Core::run_for_frames(uint32_t frames)
	{
		while (frames--)
		{
			cycle_count = 0;
			while (cycle_count <= CYCLES_PER_FRAME && !cpu.stopped)
				cpu.step();
		}
	}

	void Core::run_for_cycles(uint32_t cycles)
	{
		cycle_count = 0;
		while (cycle_count <= cycles && !cpu.stopped)
			cpu.step();
	}

	void Core::tick_subcomponents(uint8_t cycles)
	{
		timer.update(cycles);
		ppu.step(cycles);
		apu.step(cycles);
		cycle_count += cycles;
	}

	void Core::load_boot_rom_from_file()
	{
		std::ifstream rom(settings.boot_rom_path, std::ios::binary | std::ios::ate);

		if (rom)
		{
			auto len = std::min<std::streamsize>(256, rom.tellg());

			if (len == 0)
				return;

			rom.seekg(0);
			rom.read(reinterpret_cast<char *>(bus.boot_rom.data()), len);
			rom.close();
		}
	}

}