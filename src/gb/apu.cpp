#include "apu.hpp"

namespace Angbe
{
	void Square::step()
	{
		frequency_timer--;
		if (frequency_timer == 0)
		{
			frequency_timer = (2048 - frequency) * 4;
			duty_position = (duty_position + 1) & 0x7;
		}
	}

	uint16_t Square::sample(uint8_t side) const
	{
		constexpr uint8_t duty_table[4][8]{
			{0, 0, 0, 0, 0, 0, 0, 1},
			{1, 0, 0, 0, 0, 0, 0, 1},
			{1, 0, 0, 0, 0, 1, 1, 1},
			{0, 1, 1, 1, 1, 1, 1, 0}};
		auto volume = (duty_table[duty_select][duty_position] * volume_out);

		if (side == 0 && stereo_left_enabled)
		{

			return volume;
		}
		else if (side == 1 && stereo_right_enabled)
		{
			return volume;
		}

		return 0;
	}

	void Square::length_step()
	{

		if (length_enabled && length_load > 0)
		{
			length_load--;
			if (length_load == 0)
			{
				enabled = false;
			}
		}
	}

	void Square::envelope_step()
	{
		if (envelope_period != 0)
		{
			if (envelope_timer > 0)
				envelope_timer--;

			if (envelope_timer == 0)
			{
				envelope_timer = envelope_period;

				if (volume_out < 0xF && envelope_direction)
				{
					volume_out++;
				}
				else if (volume_out > 0 && !envelope_direction)
				{
					volume_out--;
				}
			}
		}
	}

	void Square::write_NRX0(uint8_t value)
	{
	}

	void Square::write_NRX1(uint8_t value)
	{
		duty_select = (value >> 6) & 0x3;

		length_load = 64 - (value & 0x3F);
	}

	void Square::write_NRX2(uint8_t value)
	{
		volume = (value >> 4) & 0xF;
		envelope_direction = (value >> 3) & 0x01;
		envelope_period = value & 0x7;
	}

	void Square::write_NRX3(uint8_t value)
	{
		frequency = (frequency & 0x700) | value;
	}

	void Square::write_NRX4(uint8_t value)
	{
		uint16_t freq_msb = value & 0b111;
		freq_msb <<= 8;
		frequency &= 0xFF;
		frequency = frequency | freq_msb;
		enabled = (value & 0x80) >> 7;

		length_enabled = (value & 0x40) >> 6;

		if (enabled)
		{
			trigger_channel();
		}
	}

	void Square::trigger_channel()
	{
		if (length_load == 0)
			length_load = 64;
		frequency_timer = frequency;
		envelope_timer = envelope_period;
		volume_out = volume;
		envelope_enable = true;
	}

	uint8_t Square::read_NRX0() const
	{
		return 0;
	}

	uint8_t Square::read_NRX1() const
	{
		uint8_t result = 0;
		result |= length_load & 0x1F;
		result |= (duty_select & 0x3) << 6;
		return result;
	}

	uint8_t Square::read_NRX2() const
	{
		uint8_t result = 0;
		result |= envelope_period & 0x7;
		result |= (envelope_direction & 0x1) << 3;
		result |= (volume & 0xF) << 4;

		return result;
	}

	uint8_t Square::read_NRX3() const
	{
		uint8_t result = 0;
		result |= frequency & 0xFF;
		return result;
	}

	uint8_t Square::read_NRX4() const
	{
		uint8_t result = 0;
		result |= (frequency & 0x700) >> 8;
		result |= enabled >> 7;
		result |= length_enabled >> 6;
		return result;
	}

	void SweepSquare::trigger_channel()
	{
		Square::trigger_channel();
		frequency_shadow = frequency;
		sweep_timer = sweep_period;
		sweep_enabled = sweep_period || sweep_shift ? true : false;
		if (sweep_shift)
			calc_frequency();
	}

	void SweepSquare::write_NRX0(uint8_t value)
	{
		Square::write_NRX0(value);
		sweep_period = (value >> 4) & 0x07;
		sweep_direction = (value >> 3) & 0x1;
		sweep_shift = (value & 0x7);
	}

	void SweepSquare::write_NRX4(uint8_t value)
	{
		Square::write_NRX4(value);
	}

	void SweepSquare::sweep_step()
	{
		if (sweep_timer > 0)
		{
			sweep_timer--;
		}

		if (sweep_timer == 0)
		{
			if (sweep_period > 0)
			{
				sweep_timer = sweep_period;
			}
			else
			{
				sweep_timer = 8;
			}

			if (sweep_enabled && sweep_period > 0)
			{
				auto nFreq = calc_frequency();

				if (nFreq <= 2047 && sweep_shift > 0)
				{
					frequency = nFreq;
					frequency_shadow = nFreq;
					calc_frequency();
				}
			}
		}
	}

	uint16_t SweepSquare::calc_frequency()
	{
		uint16_t nFreq = frequency_shadow >> sweep_shift;

		if (sweep_direction)
		{
			nFreq = frequency_shadow - nFreq;
		}
		else
		{
			nFreq = frequency_shadow + nFreq;
		}

		if (nFreq > 2048)
			enabled = false;

		return nFreq;
	}

	void APU::write_power(uint8_t value)
	{
		power = value >> 7;
		if (!power)
		{
			square_2 = SweepSquare();
			square_1 = Square();
		}
	}

	uint8_t APU::read_power() const
	{
		return power << 7;
	}

	void APU::write_NR50(uint8_t value)
	{
		stereo_right_volume = value & 0b0111;
		stereo_left_volume = (value & 0b01110000) >> 4;
	}

	void APU::write_NR51(uint8_t value)
	{
		square_2.stereo_left_enabled = (value & 0b00010000) >> 4;
		square_2.stereo_right_enabled = value & 0b00000001;

		square_1.stereo_left_enabled = (value & 0b00100000) >> 4;
		square_1.stereo_right_enabled = value & 0b00000010;
	}

	void APU::step(uint32_t cpuCycles)
	{
		while (cpuCycles && power)
		{
			sequencer_count--;
			if (sequencer_count <= 0)
			{
				sequencer_count = 8192;
				switch (frame_sequence)
				{
				case 0:
					// length
					square_2.length_step();
					square_1.length_step();

					break;
				case 2:
					// length & sweep
					square_2.length_step();
					square_1.length_step();

					square_2.sweep_step();
					break;
				case 4:
					// length
					square_2.length_step();
					square_1.length_step();

					break;
				case 6:
					// length & sweep
					square_2.length_step();
					square_1.length_step();

					square_2.sweep_step();
					break;
				case 7:
					// volume env
					square_2.envelope_step();
					square_1.envelope_step();

					break;
				default:
					break;
				}

				frame_sequence++;
				if (frame_sequence == 8)
				{
					frame_sequence = 0;
				}
			}

			square_2.step();
			square_1.step();
			if (sample_counter == 0)
			{
				float buffer1 = 0, buffer2 = 0;
				sample_counter = output_sample_rate;
				// Implementation for audio generation temporarily removed
			}

			--sample_counter;
			--cpuCycles;
		}
	}

}