/*
    Big ComBoy
    Copyright (C) 2023-2024 UltimaOmega474

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once
#include <array>
#include <cinttypes>
#include <functional>

namespace GB
{
    class Timer;

    class LengthCounter
    {
    public:
        bool is_wave = false;
        uint16_t initial_length_time = 0; // (Write Only)
        uint8_t sound_length_enable = 0; // (Read/Write) (1=Stop output when length in NR11 expires)
        uint16_t length_counter = 0;
        uint16_t clocks = 0;
        uint16_t init = 0;
        LengthCounter(bool wave) : is_wave(wave) {}

        void step_length(bool &channel_on);
    };

    class EnvelopeCounter
    {
    public:
        bool envelope_enabled = false;
        uint8_t envelope_counter = 0;
        uint8_t envelope_sweep_pace = 0;     // (0=No Sweep)
        uint8_t envelope_direction = 0;      // (0=Decrease, 1=Increase)
        uint8_t initial_envelope_volume = 0; // (0-F) (0=No Sound)

        void step_envelope_sweep(uint8_t &volume_output);
    };

    class PulseChannel
    {
        bool frequency_too_high = false;

    public:
        bool channel_on = false;
        bool has_sweep, sweep_enabled = false;
        bool left_out_enabled = false, right_out_enabled = false;
        uint8_t sweep_slope = 0; // (Read/Write)
        uint8_t sweep_direction =
            0; // 0: Addition (period increases) 1: Subtraction (period decreases)
        uint8_t sweep_pace = 0;
        uint8_t wave_duty = 0; // (Read/Write)
        uint8_t duty_position = 0;
        uint8_t period_low = 0;      // (Write Only)
        uint8_t period_high = 0;     // (Write Only)
        uint8_t trigger_channel = 0; // (Write Only) (1=Restart channel)
        uint8_t volume_output = 0;

        uint16_t sweep_pace_counter = 0;
        uint16_t period_shadow = 0;
        uint16_t period_counter = 0;

        LengthCounter length{false};
        EnvelopeCounter envelope;

        PulseChannel(bool has_sweep) : has_sweep(has_sweep) {}

        void step_frequency_sweep();
        void step_frequency();
        uint8_t sample(uint8_t side);
        void trigger(uint8_t frame_sequencer_counter);

        void write_nr10(uint8_t nr10);
        void write_nrX1(bool apu_on, uint8_t nr11);
        void write_nrX2(uint8_t nr12);
        void write_nrX3(uint8_t nr13);
        void write_nrX4(uint8_t nr14, uint8_t frame_sequencer_counter);

        uint8_t read_nr10() const;
        uint8_t read_nrX1() const;
        uint8_t read_nrX2() const;
        uint8_t read_nrX4() const;

        uint16_t get_combined_period() const;
        void update_split_period(uint16_t value);
        uint16_t calculate_period();
    };

    class WaveChannel
    {
        bool frequency_too_high = false;

    public:
        bool channel_on = false;
        bool left_out_enabled = false, right_out_enabled = false;
        uint8_t dac_enabled = 0; // (0=Off, 1=On)
        uint8_t output_level = 0;
        uint8_t period_low = 0;      // (Write Only)
        uint8_t period_high = 0;     // (Write Only)
        uint8_t trigger_channel = 0; // (Write Only)
        uint16_t position_counter = 0;
        uint16_t period_counter = 0;

        LengthCounter length{true};

        uint8_t buffer = 0;
        void step(const std::array<uint8_t, 16> &wave_table);
        uint8_t sample(uint8_t side);
        void trigger(uint8_t frame_sequencer_counter);
        void write_nr30(uint8_t nr30);
        void write_nr31(uint8_t nr31);
        void write_nr32(uint8_t nr32);
        void write_nr33(uint8_t nr33);
        void write_nr34(uint8_t nr34, uint8_t frame_sequencer_counter);

        uint8_t read_nr30() const;
        uint8_t read_nr32() const;
        uint8_t read_nr34() const;
        uint16_t get_combined_period() const;
    };

    class NoiseChannel
    {
    public:
        bool channel_on = false;
        bool left_out_enabled = false, right_out_enabled = false;
        uint8_t clock_divider = 0;   // (r)
        uint8_t LFSR_width = 0;      // (0=15 bits, 1=7 bits)
        uint8_t clock_shift = 0;     // (s)
        uint8_t trigger_channel = 0; // (Write Only)
        uint8_t volume_output = 0;
        uint16_t LFSR = 0;
        uint16_t period_counter = 0;

        LengthCounter length{false};
        EnvelopeCounter envelope;

        void step();
        uint8_t sample(uint8_t side);
        void trigger(uint8_t frame_sequencer_counter);
        void write_nr41(uint8_t nr41);
        void write_nr42(uint8_t nr42);
        void write_nr43(uint8_t nr43);
        void write_nr44(uint8_t nr44, uint8_t frame_sequencer_counter);

        uint8_t read_nr42() const;
        uint8_t read_nr43() const;
        uint8_t read_nr44() const;
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
        uint8_t frame_sequencer_counter = 0;
        uint32_t sample_counter = 0, sample_rate = 0;
        std::array<uint8_t, 16> wave_table{};
        PulseChannel pulse_1{true};
        PulseChannel pulse_2{false};
        WaveChannel wave;
        NoiseChannel noise;

        std::function<void(SampleResult result)> samples_ready_func = nullptr;

        uint8_t read_register(uint8_t address);
        void write_register(uint8_t address, uint8_t value);

        uint8_t read_wave_ram(uint8_t address);
        void write_wave_ram(uint8_t address, uint8_t value);
        void reset();
        void write_nr52(uint8_t value);
        uint8_t read_nr52() const;
        void write_nr50(uint8_t value);
        void write_nr51(uint8_t value);
        uint8_t read_nr50() const;
        uint8_t read_nr51() const;
        void step(uint32_t cycles);
        void step_frame_sequencer();
    };
}