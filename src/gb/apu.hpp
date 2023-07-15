#pragma once
#include "constants.hpp"
#include <cinttypes>
#include <array>

namespace Angbe
{
	class Square
	{
	public:
		bool envelope_enable = false;
		uint8_t enabled = 0, stereo_left_enabled = 0, stereo_right_enabled = 0;
		uint8_t duty_select = 0, duty_length = 0, duty_position = 0;
		uint8_t length_enabled = 0, length_load = 0, volume = 0;
		uint8_t envelope_direction = 0, envelope_period = 0, envelope_timer = 0;
		uint16_t frequency = 0, frequency_shadow = 0, frequency_timer = 0;
		uint16_t volume_out = 0;

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
		uint8_t sweep_enabled = 0;
		uint8_t sweep_period = 0, sweep_direction = 0;
		uint8_t sweep_shift = 0, sweep_timer = 0;

		void trigger_channel() override;
		void write_NRX0(uint8_t value) override;
		void write_NRX4(uint8_t value) override;

		void sweep_step();
		uint16_t calc_frequency();
	};

	class APU
	{
		uint8_t power = 0;
		uint8_t stereo_left_volume = 0, stereo_right_volume = 0;
		uint16_t frame_sequence = 0, sequencer_count = 0, sample_counter = 0;
		uint16_t buffer_index = 0;
		uint16_t output_sample_rate = 0;

		std::array<float, 2048> samples_buffer{};

	public:
		Square square_1;
		SweepSquare square_2;

		void write_power(uint8_t value);
		uint8_t read_power() const;

		void write_NR50(uint8_t value);
		void write_NR51(uint8_t value);

		void step(uint32_t cpuCycles);
	};
}