#include "bus.hpp"
#include "core.hpp"

namespace SunBoy
{
	MainBus::MainBus(Core &core)
		: core(core)
	{
	}
	void MainBus::reset()
	{
		boot_rom_enabled = true;
		boot_rom.fill(0);
		wram.fill(0);
		hram.fill(0);
		cart = nullptr;
	}

	void MainBus::request_interrupt(uint8_t interrupt)
	{
		core.cpu.interrupt_flag |= interrupt;
	}

	uint8_t MainBus::read_no_tick(uint16_t address)
	{
		if (within_range(address, 0, 0x7FFF))
		{
			if (address < 0x100 && boot_rom_enabled && !core.settings.skip_boot_rom)
				return boot_rom[address & 0xFF];

			if (cart)
				return cart->read(address);

			return 0xFF;
		}
		else if (within_range(address, 0x8000, 0x9FFF))
		{
			return core.ppu.read_vram(address & 0x1FFF); // VRAM
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
			return core.ppu.read_oam(address & 0xFF); // Sprite Attrib Table
		}

		else if (within_range(address, 0xFF00, 0xFF7F))
		{
			switch (address & 0xFF)
			{
				// input register
			case 0x00:
				return core.pad.get_pad_state();

			// timer registers
			case 0x04:
				return core.timer.read_div();
			case 0x05:
				return core.timer.tima;
			case 0x06:
				return core.timer.tma;
			case 0x07:
				return core.timer.tac;
			case 0x0F:
				return core.cpu.interrupt_flag;

			// pulse 1
			case 0x10:
				return core.apu.pulse_1.read_sweep();
			case 0x11:
				return core.apu.pulse_1.read_length_timer();
			case 0x12:
				return core.apu.pulse_1.read_volume_envelope();
			case 0x13:
				return 0xFF;
			case 0x14:
				return core.apu.pulse_1.read_control_period();

			// pulse 2
			case 0x15:
				return core.apu.pulse_2.read_sweep();
			case 0x16:
				return core.apu.pulse_2.read_length_timer();
			case 0x17:
				return core.apu.pulse_2.read_volume_envelope();
			case 0x18:
				return 0xFF;
			case 0x19:
				return core.apu.pulse_2.read_control_period();

			// wave channel
			case 0x1A:
				return core.apu.wave.read_dac_enable();
			case 0x1B:
				return 0xFF;
			case 0x1C:
				return core.apu.wave.read_output_level();
			case 0x1D:
				return 0xFF;
			case 0x1E:
				return core.apu.wave.read_control_period();

			// noise channel
			case 0x1F:
				return 0xFF;
			case 0x20:
				return 0xFF;
			case 0x21:
				return core.apu.noise.read_volume_envelope();
			case 0x22:
				return core.apu.noise.read_frequency_randomness();
			case 0x23:
				return core.apu.noise.read_control();
			case 0x24:
				return 0xFF;

			case 0x25:
				return core.apu.read_channel_pan();
			case 0x26:
				return core.apu.read_sound_power();
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
				return core.apu.wave.wave_table[(address & 0xFF) - 0x30];

			// ppu registers
			case 0x40:
				return core.ppu.lcd_control;
			case 0x41:
				return core.ppu.status;
			case 0x42:
				return core.ppu.screen_scroll_y;
			case 0x43:
				return core.ppu.screen_scroll_x;
			case 0x44:
				return core.ppu.line_y;
			case 0x45:
				return core.ppu.line_y_compare;
			case 0x46:
				return 0xFF; // dma start address
			case 0x47:
				return core.ppu.background_palette;
			case 0x48:
				return core.ppu.object_palette_0;
			case 0x49:
				return core.ppu.object_palette_1;
			case 0x4A:
				return core.ppu.window_y;
			case 0x4B:
				return core.ppu.window_x;

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
			return core.cpu.interrupt_enable;
		}

		return 0xFF;
	}

	void MainBus::write_no_tick(uint16_t address, uint8_t value)
	{
		if (within_range(address, 0, 0x7FFF))
		{
			if (address < 0x100 && boot_rom_enabled && !core.settings.skip_boot_rom)
			{
				return;
			}

			if (cart)
				cart->write(address, value);
			return;
		}
		else if (within_range(address, 0x8000, 0x9FFF))
		{
			core.ppu.write_vram(address & 0x1FFF, value); // VRAM
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
			core.ppu.write_oam(address & 0xFF, value); // Sprite Attrib Table
			return;
		}

		else if (within_range(address, 0xFF00, 0xFF7F))
		{
			switch (address & 0xFF)
			{
			case 0x00:
				core.pad.select_button_mode(value);
				return;

			case 0x04:
				core.timer.reset_div();
				return;
			case 0x05:
				core.timer.tima = value;
				return;
			case 0x06:
				core.timer.tma = value;
				return;
			case 0x07:
				core.timer.set_tac(value);
				return;
			case 0x0F:
				core.cpu.interrupt_flag = value;
				return;

			case 0x10:
				core.apu.pulse_1.write_sweep(value);
				return;
			case 0x11:
				core.apu.pulse_1.write_length_timer(value);
				return;
			case 0x12:
				core.apu.pulse_1.write_volume_envelope(value);
				return;
			case 0x13:
				core.apu.pulse_1.write_period_low_bits(value);
				return;
			case 0x14:
				core.apu.pulse_1.write_control_period(value);
				return;

			case 0x15:
				core.apu.pulse_2.write_sweep(value);
				return;
			case 0x16:
				core.apu.pulse_2.write_length_timer(value);
				return;
			case 0x17:
				core.apu.pulse_2.write_volume_envelope(value);
				return;
			case 0x18:
				core.apu.pulse_2.write_period_low_bits(value);
				return;
			case 0x19:
				core.apu.pulse_2.write_control_period(value);
				return;

			case 0x1A:
				core.apu.wave.write_dac_enable(value);
				return;
			case 0x1B:
				core.apu.wave.write_length_timer(value);
				return;
			case 0x1C:
				core.apu.wave.write_output_level(value);
				return;
			case 0x1D:
				core.apu.wave.write_period_low_bits(value);
				return;
			case 0x1E:
				core.apu.wave.write_control_period(value);
				return;

			case 0x20:
				core.apu.noise.write_length_timer(value);
				return;
			case 0x21:
				core.apu.noise.write_volume_envelope(value);
				return;
			case 0x22:
				core.apu.noise.write_frequency_randomness(value);
				return;
			case 0x23:
				core.apu.noise.write_control(value);
				return;

			case 0x24:
				core.apu.write_master_volume(value);
				return;
			case 0x25:
				core.apu.write_channel_pan(value);
				return;
			case 0x26:
				core.apu.write_sound_power(value);
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
				core.apu.wave.wave_table[(address & 0xFF) - 0x30] = value;
				return;

			case 0x40:
				core.ppu.lcd_control = value;
				return;
			case 0x41:
				core.ppu.status = value & 0xF8;
				return;

			case 0x42:
				core.ppu.screen_scroll_y = value;
				return;
			case 0x43:
				core.ppu.screen_scroll_x = value;
				return;

			case 0x44:
				return; // ly is read only
			case 0x45:
				core.ppu.line_y_compare = value;
				return;
			case 0x46:
				core.ppu.instant_dma(value);
				return; // dma start address
			case 0x47:
				core.ppu.background_palette = value;
				return;
			case 0x48:
				core.ppu.object_palette_0 = value;
				return;
			case 0x49:
				core.ppu.object_palette_1 = value;
				return;

			case 0x4A:
				core.ppu.window_y = value;
				return;
			case 0x4B:
				core.ppu.window_x = value;
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
			core.cpu.interrupt_enable = value;
			return;
		}
	}

	uint8_t MainBus::read(uint16_t address)
	{
		core.tick_subcomponents(4);
		return read_no_tick(address);
	}

	uint16_t MainBus::read_uint16(uint16_t address)
	{
		uint8_t low = read(address);
		uint8_t hi = read(address + 1);

		return (static_cast<uint16_t>(hi) << 8) | static_cast<uint16_t>(low);
	}

	uint16_t MainBus::read_uint16_nt(uint16_t address)
	{
		uint8_t low = read_no_tick(address);
		uint8_t hi = read_no_tick(address + 1);

		return (static_cast<uint16_t>(hi) << 8) | static_cast<uint16_t>(low);
	}

	void MainBus::write(uint16_t address, uint8_t value)
	{
		core.tick_subcomponents(4);
		write_no_tick(address, value);
	}

	void MainBus::write_uint16(uint16_t address, uint16_t value)
	{
		write(address, value & 0x00FF);
		write(address + 1, (value & 0xFF00) >> 8);
	}

}