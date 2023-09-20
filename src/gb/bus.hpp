#pragma once
#include <cinttypes>
#include <array>
namespace SunBoy
{
	class Cartridge;
	class Core;
	class MainBus
	{
		Core &core;

	public:
		bool boot_rom_enabled = true;
		std::array<uint8_t, 8192> wram{};
		std::array<uint8_t, 127> hram{};
		std::array<uint8_t, 256> boot_rom{};
		Cartridge *cart = nullptr;
		MainBus(Core &core);

		void reset();

		void request_interrupt(uint8_t interrupt);

		// bus functions
		uint8_t read_no_tick(uint16_t address);
		void write_no_tick(uint16_t address, uint8_t value);

		uint8_t read(uint16_t address);
		void write(uint16_t address, uint8_t value);
		uint16_t read_uint16(uint16_t address);
		uint16_t read_uint16_nt(uint16_t address);
		void write_uint16(uint16_t address, uint16_t value);
	};
}