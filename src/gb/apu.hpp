#pragma once
#include "constants.hpp"
#include <cinttypes>
#include <array>

namespace GameBoy
{
	class Square
	{
	public:
		uint8_t enabled;
		uint8_t stereo_left_enabled, stereo_right_enabled;
		bool envelope_enable = false;
		uint16_t frequency, frequency_shadow, frequency_timer;
		uint8_t duty_select, duty_length, duty_position;
		uint8_t length_load, length_enabled;

		uint8_t volume, envelope_direction, envelope_period, envelope_timer;
		uint16_t volume_out;

		Square();
		virtual ~Square() = default;

		void step();
		void length_step();
		void envelope_step();

		virtual void trigger_channel();

		uint16_t sample(uint8_t side) const;

		virtual void write_NRX0(uint8_t value);
		void write_NRX1(uint8_t value);
		void write_NRX2(uint8_t value);
		void write_NRX3(uint8_t value);
		virtual void write_NRX4(uint8_t value);

		virtual uint8_t read_NRX0() const;
		uint8_t read_NRX1() const;
		uint8_t read_NRX2() const;
		uint8_t read_NRX3() const;
		virtual uint8_t read_NRX4() const;
	};

	class SweepSquare : public Square
	{
	public:
		uint8_t sweep_period, sweep_direction, sweep_shift, sweep_timer;
		uint8_t sweep_enabled;
		SweepSquare();

		void trigger_channel() override;
		void write_NRX0(uint8_t value) override;
		void write_NRX4(uint8_t value) override;

		void sweep_step();
		uint16_t calc_frequency();
	};

	class APU
	{
		uint8_t power;

		uint16_t frame_sequence, sequencer_count, sample_counter;
		uint16_t buffer_index;
		uint16_t output_sample_rate;
		uint8_t stereo_left_volume, stereo_right_volume;
		std::array<float, 2048> samples_buffer;

	public:
		Square square_1;
		SweepSquare square_2;

		APU();

		void write_power(uint8_t value);
		uint8_t read_power() const;

		void write_NR50(uint8_t value);
		void write_NR51(uint8_t value);

		void step(uint32_t cpuCycles);
	};
}