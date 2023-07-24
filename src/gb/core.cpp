#include "core.hpp"
#include "constants.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

namespace SunBoy
{
	Core::Core()
		: cpu(*this), timer(*this), ppu(*this)
	{
	}

	void Core::start(std::shared_ptr<Cartridge> cart)
	{
		this->cart = cart;
		ppu.reset(true);
		apu.reset();
		if (settings.skip_boot_rom)
		{
			boot_rom_enabled = false;
			cpu.reset(0x100);
			ppu.set_post_boot_state();
		}
		else
		{
			boot_rom_enabled = true;
			cpu.reset(0x0);
			load_boot_rom_from_file();
		}
	}

	void Core::run_for_frames(uint32_t frames)
	{
		while (frames--)
		{
			cycle_count = 0;
			while (cycle_count <= CYCLES_PER_FRAME && !cpu.stopped)
			{
				cpu.step();
			}
		}
	}

	void Core::run_for_cycles(uint32_t cycles, std::ostream &log_stream)
	{
		cycle_count = 0;
		while (cycle_count <= cycles && !cpu.stopped)
		{
			if (log_stream)
				cpu.log_state(log_stream);

			cpu.step();
		}
	}

	void Core::tick_subcomponents(uint8_t cycles)
	{
		timer.update(cycles);
		ppu.step(cycles);
		apu.step(cycles);
		cycle_count += cycles;
	}

	void Core::request_interrupt(uint8_t interrupt)
	{
		cpu.interrupt_flag |= interrupt;
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

			rom.read(reinterpret_cast<char *>(boot_rom.data()), len);

			rom.close();
		}
	}

	uint8_t Core::read_no_tick(uint16_t address)
	{
		if (within_range(address, 0, 0x7FFF))
		{
			if (address < 0x100 && boot_rom_enabled && !settings.skip_boot_rom)
				return boot_rom[address & 0xFF];

			if (cart)
				return cart->read(address);

			return 0xFF;
		}
		else if (within_range(address, 0x8000, 0x9FFF))
		{
			return ppu.read_vram(address & 0x1FFF); // VRAM
		}
		else if (within_range(address, 0xA000, 0xBFFF))
		{
			if (cart)
				return cart->read_ram(address & 0x1FFF); // External Ram

			return 0xFF;
		}
		else if (within_range(address, 0xC000, 0xDFFF))
		{
			return wram[address & 0x1FFF]; // Work Ram
		}
		else if (within_range(address, 0xE000, 0xFDFF))
		{
			return wram[address & 0x1FFF]; // Echo Ram
		}

		else if (within_range(address, 0xFE00, 0xFEFF)) // 0xFE9F
		{
			return ppu.read_oam(address & 0xFF); // Sprite Attrib Table
		}

		else if (within_range(address, 0xFF00, 0xFF7F))
		{
			switch (address & 0xFF)
			{
				// input register
			case 0x00:
				return pad.get_pad_state();

			// timer registers
			case 0x04:
				return timer.read_div();
			case 0x05:
				return timer.tima;
			case 0x06:
				return timer.tma;
			case 0x07:
				return timer.tac;
			case 0x0F:
				return cpu.interrupt_flag;

			// pulse 1
			case 0x10:
				return apu.pulse_1.read_sweep();
			case 0x11:
				return apu.pulse_1.read_length_timer();
			case 0x12:
				return apu.pulse_1.read_volume_envelope();
			case 0x13:
				return 0xFF;
			case 0x14:
				return apu.pulse_1.read_control_period();

			// pulse 2
			case 0x15:
				return apu.pulse_2.read_sweep();
			case 0x16:
				return apu.pulse_2.read_length_timer();
			case 0x17:
				return apu.pulse_2.read_volume_envelope();
			case 0x18:
				return 0xFF;
			case 0x19:
				return apu.pulse_2.read_control_period();

			// wave channel
			case 0x1A:
				return apu.wave.read_dac_enable();
			case 0x1B:
				return 0xFF;
			case 0x1C:
				return apu.wave.read_output_level();
			case 0x1D:
				return 0xFF;
			case 0x1E:
				return apu.wave.read_control_period();

			// noise channel
			case 0x1F:
				return 0xFF;
			case 0x20:
				return 0xFF;
			case 0x21:
				return apu.noise.read_volume_envelope();
			case 0x22:
				return apu.noise.read_frequency_randomness();
			case 0x23:
				return apu.noise.read_control();
			case 0x24:
				return 0xFF;

			case 0x25:
				return apu.read_channel_pan();
			case 0x26:
				return apu.read_sound_power();
			case 0x30:
			case 0x31:
			case 0x32:
			case 0x33:
			case 0x34:
			case 0x35:
			case 0x36:
			case 0x37:
			case 0x38:
			case 0x39:
			case 0x3A:
			case 0x3B:
			case 0x3C:
			case 0x3D:
			case 0x3E:
			case 0x3F:
				return apu.wave.wave_table[(address & 0xFF) - 0x30];

			// ppu registers
			case 0x40:
				return ppu.lcd_control;
			case 0x41:
				return ppu.status;
			case 0x42:
				return ppu.screen_scroll_y;
			case 0x43:
				return ppu.screen_scroll_x;
			case 0x44:
				return ppu.line_y;
			case 0x45:
				return ppu.line_y_compare;
			case 0x46:
				return 0xFF; // dma start address
			case 0x47:
				return ppu.background_palette;
			case 0x48:
				return ppu.object_palette_0;
			case 0x49:
				return ppu.object_palette_1;
			case 0x4A:
				return ppu.window_y;
			case 0x4B:
				return ppu.window_x;

			case 0x50:
				return boot_rom_enabled;
			}

			return 0xFF; // IO Registers
		}

		else if (within_range(address, 0xFF80, 0xFFFE))
		{
			return hram[address - 0xFF80]; // High Ram
		}
		else if (address == 0xFFFF)
		{
			return cpu.interrupt_enable;
		}

		return 0xFF;
	}

	void Core::write_no_tick(uint16_t address, uint8_t value)
	{
		if (within_range(address, 0, 0x7FFF))
		{
			if (address < 0x100 && boot_rom_enabled && !settings.skip_boot_rom)
			{
				return;
			}

			if (cart)
				cart->write(address, value);
			return;
		}
		else if (within_range(address, 0x8000, 0x9FFF))
		{
			ppu.write_vram(address & 0x1FFF, value); // VRAM
			return;
		}
		else if (within_range(address, 0xA000, 0xBFFF))
		{
			if (cart)
				cart->write_ram(address & 0x1FFF, value);
			return;
		}
		else if (within_range(address, 0xC000, 0xDFFF))
		{
			wram[address & 0x1FFF] = value; // Work Ram
			return;
		}
		else if (within_range(address, 0xE000, 0xFDFF))
		{
			wram[address & 0x1FFF] = value; // Echo Ram
			return;
		}

		else if (within_range(address, 0xFE00, 0xFEFF)) // 0xFE9F
		{
			ppu.write_oam(address & 0xFF, value); // Sprite Attrib Table
			return;
		}

		else if (within_range(address, 0xFF00, 0xFF7F))
		{
			switch (address & 0xFF)
			{
			case 0x00:
				pad.select_button_mode(value);
				return;

			case 0x04:
				timer.reset_div();
				return;
			case 0x05:
				timer.tima = value;
				return;
			case 0x06:
				timer.tma = value;
				return;
			case 0x07:
				timer.set_tac(value);
				return;
			case 0x0F:
				cpu.interrupt_flag = value;
				return;

			case 0x10:
				apu.pulse_1.write_sweep(value);
				return;
			case 0x11:
				apu.pulse_1.write_length_timer(value);
				return;
			case 0x12:
				apu.pulse_1.write_volume_envelope(value);
				return;
			case 0x13:
				apu.pulse_1.write_period_low_bits(value);
				return;
			case 0x14:
				apu.pulse_1.write_control_period(value);
				return;

			case 0x15:
				apu.pulse_2.write_sweep(value);
				return;
			case 0x16:
				apu.pulse_2.write_length_timer(value);
				return;
			case 0x17:
				apu.pulse_2.write_volume_envelope(value);
				return;
			case 0x18:
				apu.pulse_2.write_period_low_bits(value);
				return;
			case 0x19:
				apu.pulse_2.write_control_period(value);
				return;

			case 0x1A:
				apu.wave.write_dac_enable(value);
				return;
			case 0x1B:
				apu.wave.write_length_timer(value);
				return;
			case 0x1C:
				apu.wave.write_output_level(value);
				return;
			case 0x1D:
				apu.wave.write_period_low_bits(value);
				return;
			case 0x1E:
				apu.wave.write_control_period(value);
				return;

			case 0x20:
				apu.noise.write_length_timer(value);
				return;
			case 0x21:
				apu.noise.write_volume_envelope(value);
				return;
			case 0x22:
				apu.noise.write_frequency_randomness(value);
				return;
			case 0x23:
				apu.noise.write_control(value);
				return;

			case 0x24:
				apu.write_master_volume(value);
				return;
			case 0x25:
				apu.write_channel_pan(value);
				return;
			case 0x26:
				apu.write_sound_power(value);
				return;

			case 0x30:
			case 0x31:
			case 0x32:
			case 0x33:
			case 0x34:
			case 0x35:
			case 0x36:
			case 0x37:
			case 0x38:
			case 0x39:
			case 0x3A:
			case 0x3B:
			case 0x3C:
			case 0x3D:
			case 0x3E:
			case 0x3F:
				apu.wave.wave_table[(address & 0xFF) - 0x30] = value;
				return;

			case 0x40:
				ppu.lcd_control = value;
				return;
			case 0x41:
				ppu.status = value & 0xF8;
				return;

			case 0x42:
				ppu.screen_scroll_y = value;
				return;
			case 0x43:
				ppu.screen_scroll_x = value;
				return;

			case 0x44:
				return; // ly is read only
			case 0x45:
				ppu.line_y_compare = value;
				return;
			case 0x46:
				ppu.instant_dma(value);
				return; // dma start address
			case 0x47:
				ppu.background_palette = value;
				return;
			case 0x48:
				ppu.object_palette_0 = value;
				return;
			case 0x49:
				ppu.object_palette_1 = value;
				return;

			case 0x4A:
				ppu.window_y = value;
				return;
			case 0x4B:
				ppu.window_x = value;
				return;

			case 0x50:
				boot_rom_enabled = false;
				return;
			}
			return;
		}

		else if (within_range(address, 0xFF80, 0xFFFE))
		{
			hram[address - 0xFF80] = value; // High Ram
			return;
		}
		else if (address == 0xFFFF)
		{
			cpu.interrupt_enable = value;
			return;
		}
	}

	uint8_t Core::read(uint16_t address)
	{
		tick_subcomponents(4);
		return read_no_tick(address);
	}

	uint16_t Core::read_uint16(uint16_t address)
	{
		uint8_t low = read(address);
		uint8_t hi = read(address + 1);

		return (static_cast<uint16_t>(hi) << 8) | static_cast<uint16_t>(low);
	}

	void Core::write(uint16_t address, uint8_t value)
	{
		tick_subcomponents(4);
		write_no_tick(address, value);
	}

	void Core::write_uint16(uint16_t address, uint16_t value)
	{
		write(address, value & 0x00FF);
		write(address + 1, (value & 0xFF00) >> 8);
	}

}