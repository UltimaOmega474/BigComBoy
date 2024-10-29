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

namespace GB {
    class LengthCounter {
    public:
        auto clock_length(bool &channel_on) -> void;

    private:
        uint8_t sound_length_enable = 0; // (Read/Write) (1=Stop output when length in NR11 expires)
        uint16_t initial_length_time = 0; // (Write Only)
        uint16_t length_counter = 0;

        friend class PulseChannel;
        friend class NoiseChannel;
        friend class WaveChannel;
    };

    class EnvelopeCounter {
    public:
        auto clock_envelope_sweep(uint8_t &volume_output) -> void;

    private:
        bool envelope_enabled = false;
        uint8_t envelope_counter = 0;
        uint8_t envelope_sweep_pace = 0;     // (0=No Sweep)
        uint8_t envelope_direction = 0;      // (0=Decrease, 1=Increase)
        uint8_t initial_envelope_volume = 0; // (0-F) (0=No Sound)

        friend class PulseChannel;
        friend class NoiseChannel;
    };

    class PulseChannel {
    public:
        explicit PulseChannel(const bool has_sweep) : has_sweep(has_sweep) {}

        auto clock_frequency_sweep() -> void;
        auto clock_frequency() -> void;
        auto sample(uint8_t side) const -> uint8_t;
        auto trigger(uint8_t frame_sequencer_counter) -> void;

        auto write_nr10(uint8_t nr10) -> void;
        auto write_nrX1(bool apu_on, uint8_t nr11) -> void;
        auto write_nrX2(uint8_t nr12) -> void;
        auto write_nrX3(uint8_t nr13) -> void;
        auto write_nrX4(uint8_t nr14, uint8_t frame_sequencer_counter) -> void;

        auto read_nr10() const -> uint8_t;
        auto read_nrX1() const -> uint8_t;
        auto read_nrX2() const -> uint8_t;
        auto read_nrX4() const -> uint8_t;

        auto get_combined_period() const -> uint16_t;
        auto update_split_period(uint16_t value) -> void;
        auto calculate_period() -> uint16_t;

    private:
        bool frequency_too_high = false;
        bool channel_on = false;
        bool has_sweep, sweep_enabled = false;
        bool left_out_enabled = false;
        bool right_out_enabled = false;
        uint8_t sweep_shift = 0; // (Read/Write)
        // 0: Addition (period increases) 1: Subtraction (period decreases)
        uint8_t sweep_direction = 0;
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

        LengthCounter length{};
        EnvelopeCounter envelope{};

        friend class APU;
    };

    class WaveChannel {
    public:
        auto clock(const std::array<uint8_t, 16> &wave_table) -> void;
        auto sample(uint8_t side) const -> uint8_t;
        auto trigger(uint8_t frame_sequencer_counter) -> void;
        auto write_nr30(uint8_t nr30) -> void;
        auto write_nr31(uint8_t nr31) -> void;
        auto write_nr32(uint8_t nr32) -> void;
        auto write_nr33(uint8_t nr33) -> void;
        auto write_nr34(uint8_t nr34, uint8_t frame_sequencer_counter) -> void;

        auto read_nr30() const -> uint8_t;
        auto read_nr32() const -> uint8_t;
        auto read_nr34() const -> uint8_t;
        auto get_combined_period() const -> uint16_t;

    private:
        bool frequency_too_high = false;
        bool channel_on = false;
        bool left_out_enabled = false;
        bool right_out_enabled = false;

        uint8_t dac_enabled = 0; // (0=Off, 1=On)
        uint8_t output_level = 0;
        uint8_t period_low = 0;      // (Write Only)
        uint8_t period_high = 0;     // (Write Only)
        uint8_t trigger_channel = 0; // (Write Only)
        uint8_t buffer = 0;
        uint16_t position_counter = 0;
        uint16_t period_counter = 0;

        LengthCounter length;

        friend class APU;
    };

    class NoiseChannel {
    public:
        auto clock() -> void;
        auto sample(uint8_t side) const -> uint8_t;
        auto trigger(uint8_t frame_sequencer_counter) -> void;
        auto write_nr41(uint8_t nr41) -> void;
        auto write_nr42(uint8_t nr42) -> void;
        auto write_nr43(uint8_t nr43) -> void;
        auto write_nr44(uint8_t nr44, uint8_t frame_sequencer_counter) -> void;

        auto read_nr42() const -> uint8_t;
        auto read_nr43() const -> uint8_t;
        auto read_nr44() const -> uint8_t;

    private:
        bool channel_on = false;
        bool left_out_enabled = false;
        bool right_out_enabled = false;

        uint8_t clock_divider = 0;   // (r)
        uint8_t LFSR_width = 0;      // (0=15 bits, 1=7 bits)
        uint8_t clock_shift = 0;     // (s)
        uint8_t trigger_channel = 0; // (Write Only)
        uint8_t volume_output = 0;
        uint16_t LFSR = 0;
        uint16_t period_counter = 0;

        LengthCounter length;
        EnvelopeCounter envelope;

        friend class APU;
    };

    struct SampleResult {
        struct {
            uint8_t master_volume = 0, vin = 0;
            uint8_t pulse_1 = 0, pulse_2 = 0;
            uint8_t wave = 0, noise = 0;
        } left_channel;
        struct {
            uint8_t master_volume = 0, vin = 0;
            uint8_t pulse_1 = 0, pulse_2 = 0;
            uint8_t wave = 0, noise = 0;
        } right_channel;
    };

    class APU {
    public:
        auto reset() -> void;
        auto set_samples_callback(int32_t rate, std::function<void(SampleResult result)> cb)
            -> void;

        auto read_register(uint8_t address) const -> uint8_t;
        auto write_register(uint8_t address, uint8_t value) -> void;

        auto read_wave_ram(uint8_t address) const -> uint8_t;
        auto write_wave_ram(uint8_t address, uint8_t value) -> void;

        auto write_nr52(uint8_t value) -> void;
        auto read_nr52() const -> uint8_t;

        auto write_nr50(uint8_t value) -> void;
        auto write_nr51(uint8_t value) -> void;

        auto read_nr50() const -> uint8_t;
        auto read_nr51() const -> uint8_t;

        auto clock(int32_t cycles) -> void;
        auto clock_frame_sequencer() -> void;

    private:
        bool mix_vin_left = false;
        bool mix_vin_right = false;
        bool power = false;

        uint8_t stereo_left_volume = 0;
        uint8_t stereo_right_volume = 0;
        uint8_t frame_sequencer_counter = 0;

        std::array<uint8_t, 16> wave_table{};

        PulseChannel pulse_1{true};
        PulseChannel pulse_2{false};
        WaveChannel wave;
        NoiseChannel noise;

        std::function<void(SampleResult result)> samples_ready_func = nullptr;
        int32_t sample_counter = 0;
        int32_t sample_rate = 0;
    };
}