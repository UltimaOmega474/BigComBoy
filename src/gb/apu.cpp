#include "apu.hpp"
#include "timer.hpp"
#include "constants.hpp"
#include <cmath>
#include <array>
namespace SunBoy
{
	void LengthCounter::step_length(bool &channel_on)
	{
		if (!channel_on || !sound_length_enable)
			return;

		if (length_counter == (is_wave ? 256 : 64))
		{
			channel_on = false;
		}
		else
		{
			length_counter++;
		}
	}

	void EnvelopeCounter::step_envelope_sweep(uint8_t &volume_output)
	{
		if (envelope_sweep_pace != 0 && envelope_enabled)
		{
			if (envelope_counter == 0)
			{
				uint8_t new_volume = volume_output;
				if (envelope_direction == 0)
				{
					new_volume--;
				}
				else
				{
					new_volume++;
				}

				if (within_range(new_volume, 0, 15))
				{
					volume_output = new_volume;
					envelope_counter = envelope_sweep_pace;
				}
				else
				{
					envelope_enabled = false;
				}
			}

			envelope_counter--;
		}
	}

	void PulseChannel::step_frequency_sweep()
	{
		if (!channel_on)
			return;

		if (sweep_pace_counter > 0)
			sweep_pace_counter--;

		if (sweep_pace_counter == 0)
		{
			if (sweep_pace > 0)
			{
				sweep_pace_counter = sweep_pace;
			}
			else
			{
				sweep_pace_counter = 8;
			}

			if (sweep_enabled && sweep_pace > 0)
			{
				auto nf = calculate_period();

				if (nf <= 2047 && sweep_slope > 0)
				{
					update_split_period(nf);
					period_shadow = nf;
					calculate_period();
				}
			}
		}
	}

	void PulseChannel::step_frequency()
	{
		if (!channel_on)
			return;
		if (period_counter == 0)
		{
			period_counter = (0x800 - get_combined_period()) * 4;
			duty_position = (duty_position + 1) & 0x7;
		}
		else
		{
			period_counter--;
		}
	}

	uint8_t PulseChannel::sample(uint8_t side)
	{
		auto volume = (DUTY_TABLE[wave_duty][duty_position] * volume_output);

		if (!channel_on)
			return 0;

		if (side == 0 && left_out_enabled)
		{

			return volume;
		}
		else if (side == 1 && right_out_enabled)
		{
			return volume;
		}

		return 0;
	}

	void PulseChannel::trigger()
	{
		channel_on = true;

		if (length.length_counter == 64)
			length.length_counter = 0;
		uint16_t old_period = get_combined_period();

		period_counter = (0x800 - old_period) * 4;

		period_shadow = get_combined_period();
		sweep_pace_counter = sweep_pace;
		sweep_enabled = get_combined_period() || sweep_slope ? true : false;

		if (sweep_slope)
			calculate_period();

		envelope.envelope_counter = envelope.envelope_sweep_pace;
		volume_output = envelope.initial_envelope_volume;
		envelope.envelope_enabled = true;
		// turn DAC off
		if (envelope.initial_envelope_volume == 0)
			channel_on = false;
	}

	void PulseChannel::write_sweep(uint8_t nr10)
	{
		if (!has_sweep)
			return;

		sweep_slope = nr10 & 0b111;
		sweep_direction = (nr10 & 0b1000) >> 3;
		sweep_pace = (nr10 & 0b01110000) >> 4;
	}

	void PulseChannel::write_length_timer(uint8_t nr11)
	{
		length.initial_length_time = nr11 & 0b00111111;
		wave_duty = (nr11 & 0b11000000) >> 6;

		length.length_counter = length.initial_length_time;
	}

	void PulseChannel::write_volume_envelope(uint8_t nr12)
	{
		envelope.envelope_sweep_pace = nr12 & 0b00000111;
		envelope.envelope_direction = (nr12 & 0b00001000) >> 3;
		envelope.initial_envelope_volume = (nr12 & 0b11110000) >> 4;

		// if channel is on, trigger again
		if (channel_on)
			trigger();
	}

	void PulseChannel::write_period_low_bits(uint8_t nr13)
	{
		period_low = nr13;
	}

	void PulseChannel::write_control_period(uint8_t nr14)
	{
		period_high = nr14 & 0b00000111;
		length.sound_length_enable = (nr14 & 0b01000000) >> 6;
		trigger_channel = (nr14 & 0b10000000) >> 7;

		// trigger the channel if the bit is 1
		if (trigger_channel)
			trigger();
	}

	uint8_t PulseChannel::read_sweep() const
	{
		if (!has_sweep)
			return 0xFF;

		uint8_t nr10 = 0;
		nr10 |= sweep_slope;
		nr10 |= sweep_direction << 3;
		nr10 |= sweep_pace << 4;

		return nr10;
	}

	uint8_t PulseChannel::read_length_timer() const
	{
		uint8_t nr11 = wave_duty << 6;

		return nr11;
	}

	uint8_t PulseChannel::read_volume_envelope() const
	{
		uint8_t nr12 = 0;
		nr12 |= envelope.envelope_sweep_pace;
		nr12 |= envelope.envelope_direction << 3;
		nr12 |= envelope.initial_envelope_volume << 4;
		return nr12;
	}

	uint8_t PulseChannel::read_control_period() const
	{
		uint8_t nr14 = length.sound_length_enable << 6;
		return nr14;
	}

	uint16_t PulseChannel::get_combined_period() const
	{
		return (period_high << 8) | period_low;
	}

	void PulseChannel::update_split_period(uint16_t value)
	{
		period_low = value & 0xFF;
		period_high = (value & 0b11100000000) >> 8;
	}

	uint16_t PulseChannel::calculate_period()
	{
		uint16_t new_period = period_shadow >> sweep_slope;

		if (sweep_direction)
		{
			new_period = period_shadow - new_period;
		}
		else
		{
			new_period = period_shadow + new_period;
		}

		if (new_period > 2048)
			channel_on = false;

		return new_period;
	}

	void WaveChannel::step()
	{
		if (channel_on && dac_enabled)
		{
			if (period_counter <= 0)
			{
				period_counter = (0x800 - get_combined_period()) * 2;

				position_counter = (position_counter + 1) % 32;
				buffer = wave_table[position_counter / 2];

				if ((position_counter & 1) == 0)
					buffer >>= 4;
			}
			else
			{
				period_counter--;
			}
		}
	}

	uint8_t WaveChannel::sample(uint8_t side)
	{
		if (!channel_on || !dac_enabled)
			return 0;

		uint8_t volume = (buffer & 0xF) >> WAVE_VOLUME[output_level];

		if (side == 0 && left_out_enabled)
		{
			return volume;
		}
		else if (side == 1 && right_out_enabled)
		{
			return volume;
		}

		return 0;
	}

	void WaveChannel::trigger()
	{
		channel_on = true;

		if (length.length_counter == (length.is_wave ? 256 : 64))
			length.length_counter = 0;

		position_counter = 0;

		period_counter = (0x800 - get_combined_period()) * 2;
	}

	void WaveChannel::write_dac_enable(uint8_t nr30)
	{
		dac_enabled = (nr30 & 0b10000000) >> 7;
	}

	void WaveChannel::write_length_timer(uint8_t nr31)
	{
		length.initial_length_time = nr31;
		length.length_counter = length.initial_length_time;
	}

	void WaveChannel::write_output_level(uint8_t nr32)
	{
		output_level = (nr32 & 0b01100000) >> 5;
	}

	void WaveChannel::write_period_low_bits(uint8_t nr33)
	{
		period_low = nr33;
	}

	void WaveChannel::write_control_period(uint8_t nr34)
	{
		period_high = nr34 & 0b00000111;
		length.sound_length_enable = (nr34 & 0b01000000) >> 6;
		trigger_channel = (nr34 & 0b10000000) >> 7;

		// trigger the channel if the bit is 1
		if (trigger_channel)
			trigger();
	}

	uint8_t WaveChannel::read_dac_enable() const
	{
		return dac_enabled << 7;
	}

	uint8_t WaveChannel::read_output_level() const
	{
		return output_level << 5;
	}

	uint8_t WaveChannel::read_control_period() const
	{
		uint8_t nr34 = length.sound_length_enable << 6;
		return nr34;
	}

	uint16_t WaveChannel::get_combined_period() const
	{
		return (period_high << 8) | period_low;
	}

	void NoiseChannel::step()
	{
		if (!channel_on)
			return;

		if (period_counter == 0)
		{
			period_counter = (NOISE_DIV[clock_divider] << clock_shift) * 4;

			uint16_t _xor = ((LFSR & 1) ^ ((LFSR & 2) >> 1));
			LFSR |= _xor << 15;

			if (LFSR_width == 1)
			{
				// clear first
				LFSR &= ~(0x80);
				LFSR |= _xor << 7;
			}
			LFSR >>= 1;
		}
		else
		{
			period_counter--;
		}
	}

	uint8_t NoiseChannel::sample(uint8_t side)
	{
		if (!channel_on)
			return 0;

		auto volume = (LFSR & 1) * volume_output;

		if (side == 0 && left_out_enabled)
		{

			return volume;
		}
		else if (side == 1 && right_out_enabled)
		{
			return volume;
		}

		return 0;
	}

	void NoiseChannel::trigger()
	{
		channel_on = true;

		if (length.length_counter == 64)
			length.length_counter = 0;

		period_counter = (NOISE_DIV[clock_divider] << clock_shift) * 4;
		LFSR = 0x7FFF;

		envelope.envelope_counter = envelope.envelope_sweep_pace;
		volume_output = envelope.initial_envelope_volume;
		envelope.envelope_enabled = true;
		// turn DAC off
		if (envelope.initial_envelope_volume == 0)
			channel_on = false;
	}

	void NoiseChannel::write_length_timer(uint8_t nr41)
	{
		length.length_counter = length.initial_length_time = nr41 & 0b00111111;
	}

	void NoiseChannel::write_volume_envelope(uint8_t nr42)
	{
		envelope.envelope_sweep_pace = nr42 & 0b00000111;
		envelope.envelope_direction = (nr42 & 0b00001000) >> 3;
		envelope.initial_envelope_volume = (nr42 & 0b11110000) >> 4;
	}

	uint8_t NoiseChannel::read_volume_envelope() const
	{
		uint8_t nr42 = 0;
		nr42 |= envelope.envelope_sweep_pace;
		nr42 |= envelope.envelope_direction << 3;
		nr42 |= envelope.initial_envelope_volume << 4;
		return nr42;
	}

	void NoiseChannel::write_frequency_randomness(uint8_t nr43)
	{
		clock_divider = nr43 & 0b00000111;
		LFSR_width = (nr43 & 0b00001000) >> 3;
		clock_shift = (nr43 & 0b11110000) >> 4;
	}

	void NoiseChannel::write_control(uint8_t nr44)
	{
		length.sound_length_enable = (nr44 & 0b01000000) >> 6;
		trigger_channel = (nr44 & 0b10000000) >> 7;

		// triggers channel if 1 for bit7
		if (trigger_channel)
			trigger();
	}

	uint8_t NoiseChannel::read_frequency_randomness() const
	{
		uint8_t nr43 = 0;
		nr43 |= clock_divider;
		nr43 |= LFSR_width << 3;
		nr43 |= clock_shift << 4;
		return nr43;
	}

	uint8_t NoiseChannel::read_control() const
	{
		return length.sound_length_enable << 6;
	}

	void APU::reset()
	{
		stereo_left_volume = 0;
		stereo_right_volume = 0;
		mix_vin_left = false;
		mix_vin_right = false;
		sample_counter = 0;
		power = false;

		pulse_1 = PulseChannel(true);
		pulse_2 = PulseChannel(false);
		wave = WaveChannel();
		noise = NoiseChannel();
	}

	void APU::write_sound_power(uint8_t value)
	{
		power = (value & 0b10000000) > 0;
		if (power == 0)
		{
			pulse_1 = PulseChannel(true);
			pulse_2 = PulseChannel(false);
			wave = WaveChannel();
			noise = NoiseChannel();
		}
	}

	uint8_t APU::read_sound_power() const
	{
		uint8_t nr52 = 0;
		nr52 |= power ? 0b10000000 : 0;
		nr52 |= pulse_1.channel_on ? 0b00000001 : 0;
		nr52 |= pulse_2.channel_on ? 0b00000010 : 0;
		nr52 |= wave.channel_on ? 0b00000100 : 0;
		nr52 |= noise.channel_on ? 0b00001000 : 0;

		return nr52;
	}

	void APU::write_master_volume(uint8_t value)
	{
		stereo_right_volume = value & 0b00000111;
		stereo_left_volume = (value & 0b01110000) >> 4;

		mix_vin_left = (value & 0b10000000) > 0;
		mix_vin_right = (value & 0b00001000) > 0;
	}

	void APU::write_channel_pan(uint8_t value)
	{
		pulse_1.left_out_enabled = (value & 0b00010000) > 0;
		pulse_1.right_out_enabled = value & 0b00000001;

		pulse_2.left_out_enabled = (value & 0b00100000) > 0;
		pulse_2.right_out_enabled = value & 0b00000010;

		wave.left_out_enabled = (value & 0b01000000) > 0;
		wave.right_out_enabled = value & 0b00000100;

		noise.left_out_enabled = (value & 0b10000000) > 0;
		noise.right_out_enabled = value & 0b00001000;
	}

	uint8_t APU::read_master_volume() const
	{
		uint8_t nr50 = 0;
		nr50 |= stereo_right_volume & 0b00000111;
		nr50 |= (stereo_left_volume & 0b01110000) << 4;
		nr50 |= mix_vin_right ? 0b00001000 : 0;
		nr50 |= mix_vin_left ? 0b10000000 : 0;
		return nr50;
	}

	uint8_t APU::read_channel_pan() const
	{
		uint8_t nr51 = 0;
		nr51 |= static_cast<uint8_t>(pulse_1.left_out_enabled) << 4;
		nr51 |= static_cast<uint8_t>(pulse_1.right_out_enabled);

		nr51 |= static_cast<uint8_t>(pulse_2.left_out_enabled) << 5;
		nr51 |= static_cast<uint8_t>(pulse_2.right_out_enabled) << 1;

		nr51 |= static_cast<uint8_t>(wave.left_out_enabled) << 6;
		nr51 |= static_cast<uint8_t>(wave.right_out_enabled) << 2;

		nr51 |= static_cast<uint8_t>(noise.left_out_enabled) << 7;
		nr51 |= static_cast<uint8_t>(noise.right_out_enabled) << 3;
		return nr51;
	}

	void APU::step(uint32_t cycles)
	{
		for (auto i = 0; i < cycles; ++i)
		{
			pulse_1.step_frequency();
			pulse_2.step_frequency();
			wave.step();
			noise.step();

			if (sample_counter <= 0 && sample_rate > 0)
			{
				sample_counter = sample_rate;

				if (samples_ready_func)
				{
					SampleResult result;
					result.left_channel.master_volume = stereo_left_volume;
					result.left_channel.pulse_1 = pulse_1.sample(0);
					result.left_channel.pulse_2 = pulse_2.sample(0);
					result.left_channel.wave = wave.sample(0);
					result.left_channel.noise = noise.sample(0);

					result.right_channel.master_volume = stereo_right_volume;
					result.right_channel.pulse_1 = pulse_1.sample(1);
					result.right_channel.pulse_2 = pulse_2.sample(1);
					result.right_channel.wave = wave.sample(1);
					result.right_channel.noise = noise.sample(1);
					samples_ready_func(result);
				}
			}

			sample_counter--;
		}
	}
	void APU::step_counters(uint8_t apu_div)
	{
		switch (apu_div)
		{
		case 0:
		case 4:
		{
			pulse_1.length.step_length(pulse_1.channel_on);
			pulse_2.length.step_length(pulse_2.channel_on);
			if (wave.dac_enabled)
				wave.length.step_length(wave.channel_on);
			noise.length.step_length(noise.channel_on);
			break;
		}

		case 2:
		case 6:
		{
			pulse_1.step_frequency_sweep();
			pulse_1.length.step_length(pulse_1.channel_on);
			pulse_2.length.step_length(pulse_2.channel_on);
			if (wave.dac_enabled)
				wave.length.step_length(wave.channel_on);
			noise.length.step_length(noise.channel_on);
			break;
		}

		case 7:
		{
			if (pulse_1.channel_on)
				pulse_1.envelope.step_envelope_sweep(pulse_1.volume_output);
			if (pulse_2.channel_on)
				pulse_2.envelope.step_envelope_sweep(pulse_2.volume_output);
			if (noise.channel_on)
				noise.envelope.step_envelope_sweep(noise.volume_output);
			break;
		}
		}
	}

}