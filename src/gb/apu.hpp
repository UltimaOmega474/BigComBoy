#pragma once
#include "constants.hpp"
#include <cinttypes>
#include <array>
#include <functional>
namespace SunBoy
{
	class Timer;

	class LengthCounter
	{
	public:
		bool is_wave = false;
		uint8_t initial_length_time = 0; // (Write Only)
		uint8_t sound_length_enable = 0; // (Read/Write) (1=Stop output when length in NR11 expires)
		uint16_t length_counter = 0;

		LengthCounter(bool wave) : is_wave(wave) {}

		void step_length(bool &channel_on);
	};

	class EnvelopeCounter
	{
	public:
		bool envelope_enabled = false;
		uint8_t envelope_counter = 0;
		uint8_t envelope_sweep_pace = 0;	 // (0=No Sweep)
		uint8_t envelope_direction = 0;		 // (0=Decrease, 1=Increase)
		uint8_t initial_envelope_volume = 0; // (0-F) (0=No Sound)

		void step_envelope_sweep(uint8_t &volume_output);
	};

	class PulseChannel
	{
	public:
		bool channel_on = false;
		bool has_sweep, sweep_enabled = false;
		bool left_out_enabled = true, right_out_enabled = true;
		uint8_t sweep_slope = 0;	 // (Read/Write)
		uint8_t sweep_direction = 0; // 0: Addition (period increases) 1: Subtraction (period decreases)
		uint8_t sweep_pace = 0;
		uint8_t wave_duty = 0; // (Read/Write)
		uint8_t duty_position = 0;
		uint8_t period_low = 0;		 // (Write Only)
		uint8_t period_high = 0;	 // (Write Only)
		uint8_t trigger_channel = 0; // (Write Only) (1=Restart channel)
		uint8_t volume_output = 0;

		uint16_t sweep_pace_counter = 0;
		uint16_t period_shadow = 0;
		uint16_t period_counter = 0;

		LengthCounter length{false};
		EnvelopeCounter envelope;

		PulseChannel(bool has_sweep)
			: has_sweep(has_sweep)
		{
		}

		void step_frequency_sweep();
		void step_frequency();
		uint8_t sample(uint8_t side);
		void trigger();

		void write_sweep(uint8_t nr10);
		void write_length_timer(uint8_t nr11);
		void write_volume_envelope(uint8_t nr12);
		void write_period_low_bits(uint8_t nr13);
		void write_control_period(uint8_t nr14);

		uint8_t read_sweep() const;
		uint8_t read_length_timer() const;
		uint8_t read_volume_envelope() const;
		uint8_t read_control_period() const;

		uint16_t get_combined_period() const;
		void update_split_period(uint16_t value);
		uint16_t calculate_period();
	};

	class WaveChannel
	{
	public:
		bool channel_on = false;
		bool left_out_enabled = true, right_out_enabled = true;
		uint8_t dac_enabled = 0; // (0=Off, 1=On)
		uint8_t output_level = 0;
		uint8_t period_low = 0;		 // (Write Only)
		uint8_t period_high = 0;	 // (Write Only)
		uint8_t trigger_channel = 0; // (Write Only)
		std::array<uint8_t, 16> wave_table{};
		uint16_t position_counter = 0;
		uint16_t period_counter = 0;

		LengthCounter length{true};

		uint8_t buffer = 0;
		void step();
		uint8_t sample(uint8_t side);
		void trigger();
		void write_dac_enable(uint8_t nr30);
		void write_length_timer(uint8_t nr31);
		void write_output_level(uint8_t nr32);
		void write_period_low_bits(uint8_t nr33);
		void write_control_period(uint8_t nr34);

		uint8_t read_dac_enable() const;
		uint8_t read_output_level() const;
		uint8_t read_control_period() const;
		uint16_t get_combined_period() const;
	};

	class NoiseChannel
	{
	public:
		bool channel_on = false;
		bool left_out_enabled = true, right_out_enabled = true;
		uint8_t clock_divider = 0;	 // (r)
		uint8_t LFSR_width = 0;		 // (0=15 bits, 1=7 bits)
		uint8_t clock_shift = 0;	 // (s)
		uint8_t trigger_channel = 0; // (Write Only)
		uint8_t volume_output = 0;
		uint16_t LFSR = 0;
		uint16_t period_counter = 0;

		LengthCounter length{false};
		EnvelopeCounter envelope;

		void step();
		uint8_t sample(uint8_t side);
		void trigger();
		void write_length_timer(uint8_t nr41);
		void write_volume_envelope(uint8_t nr42);
		void write_frequency_randomness(uint8_t nr43);
		void write_control(uint8_t nr44);

		uint8_t read_volume_envelope() const;
		uint8_t read_frequency_randomness() const;
		uint8_t read_control() const;
	};

	struct SampleResult
	{
		struct
		{
			uint8_t master_volume = 0, vin = 0;
			uint8_t pulse_1 = 0, pulse_2 = 0;
			uint8_t wave = 0, noise = 0;
		} left_channel;
		struct
		{
			uint8_t master_volume = 0, vin = 0;
			uint8_t pulse_1 = 0, pulse_2 = 0;
			uint8_t wave = 0, noise = 0;
		} right_channel;
	};

	class APU
	{
	public:
		bool mix_vin_left = false, mix_vin_right = false;
		bool power = 0; // (0: turn the APU off) (Read/Write)
		uint8_t stereo_left_volume = 0, stereo_right_volume = 0;
		int32_t sample_counter = 0, sample_rate = 0;

		PulseChannel pulse_1{true};
		PulseChannel pulse_2{false};
		WaveChannel wave;
		NoiseChannel noise;
		std::function<void(SampleResult result)> samples_ready_func = nullptr;

		void reset();
		void write_sound_power(uint8_t value);
		uint8_t read_sound_power() const;
		void write_master_volume(uint8_t value);
		void write_channel_pan(uint8_t value);
		uint8_t read_master_volume() const;
		uint8_t read_channel_pan() const;
		void step(uint32_t cycles);
		void step_counters(uint8_t apu_div);
	};
}