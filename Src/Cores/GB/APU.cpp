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

#include "APU.hpp"
#include <array>

namespace GB {
    // Fixes Final Fantasy Adventure because it mutes channels by setting the frequency to max
    constexpr int HIGH_FREQUENCY_CUTOFF = 0x7FF;

    constexpr std::array<uint16_t, 8> NOISE_DIV{
        2, 4, 8, 12, 16, 20, 24, 28,
    };

    constexpr std::array<uint8_t, 4> WAVE_VOLUME{
        4,
        0,
        1,
        2,
    };

    constexpr std::array<std::array<uint8_t, 8>, 4> DUTY_TABLE{
        std::array<uint8_t, 8>{0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, 8>{1, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, 8>{1, 0, 0, 0, 0, 1, 1, 1},
        std::array<uint8_t, 8>{0, 1, 1, 1, 1, 1, 1, 0},
    };

    void LengthCounter::step_length(bool &channel_on) {
        if (sound_length_enable && length_counter > 0) {
            length_counter--;

            if (length_counter == 0) {
                channel_on = false;
            }
        }
    }

    void EnvelopeCounter::step_envelope_sweep(uint8_t &volume_output) {
        if (envelope_sweep_pace && envelope_enabled) {
            if (envelope_counter) {
                envelope_counter--;
            }

            if (envelope_counter == 0) {
                envelope_counter = envelope_sweep_pace;

                uint8_t new_volume = volume_output + (envelope_direction ? 1 : -1);

                if ((new_volume >= 0) && (new_volume <= 15)) {
                    volume_output = new_volume;
                } else {
                    envelope_enabled = false;
                }
            }
        }
    }

    void PulseChannel::step_frequency_sweep() {
        if (!channel_on || !sweep_enabled) {
            return;
        }

        if (sweep_pace_counter > 0) {
            sweep_pace_counter--;
        }

        if (sweep_pace_counter == 0) {
            sweep_pace_counter = sweep_pace ? sweep_pace : 8;

            if (sweep_pace > 0) {
                uint16_t nf = calculate_period();

                if (nf <= 2047 && sweep_shift > 0) {
                    update_split_period(nf);
                    period_shadow = nf;
                    calculate_period();
                }
            }
        }
    }

    void PulseChannel::step_frequency() {
        if (!channel_on) {
            return;
        }

        if (period_counter == 0) {
            period_counter = (0x800 - get_combined_period()) * 4;
            duty_position = (duty_position + 1) & 0x7;
        } else {
            period_counter--;
        }
    }

    uint8_t PulseChannel::sample(uint8_t side) {
        uint8_t volume = (DUTY_TABLE[wave_duty][duty_position] * volume_output);

        if (frequency_too_high) {
            volume = 0;
        }

        if (!channel_on) {
            return 0;
        }

        if (side == 0 && left_out_enabled) {
            return volume;
        } else if (side == 1 && right_out_enabled) {
            return volume;
        }

        return 0;
    }

    void PulseChannel::trigger(uint8_t frame_sequencer_counter) {
        channel_on = true;

        if (length.length_counter == 0) {
            length.length_counter = 64;

            if (frame_sequencer_counter & 1) {
                length.step_length(channel_on);
            }
        }

        if (has_sweep) {
            period_shadow = get_combined_period();
            sweep_pace_counter = sweep_pace ? sweep_pace : 8;

            sweep_enabled = (sweep_pace || sweep_shift) ? true : false;

            if (sweep_shift) {
                calculate_period();
            }
        }

        uint16_t old_period = get_combined_period();

        period_counter = (0x800 - old_period) * 4;

        envelope.envelope_counter = envelope.envelope_sweep_pace ? envelope.envelope_sweep_pace : 8;
        volume_output = envelope.initial_envelope_volume;
        envelope.envelope_enabled = true;

        if (envelope.initial_envelope_volume == 0 && envelope.envelope_direction == 0) {
            channel_on = false;
        }
    }

    void PulseChannel::write_nr10(uint8_t nr10) {
        if (!has_sweep) {
            return;
        }

        sweep_shift = nr10 & 0b111;
        sweep_direction = (nr10 & 0b1000) >> 3;
        sweep_pace = (nr10 & 0b01110000) >> 4;
    }

    void PulseChannel::write_nrX1(bool apu_on, uint8_t nr11) {
        length.initial_length_time = 64 - (nr11 & 0x3F);
        if (apu_on) {
            wave_duty = (nr11 & 0b11000000) >> 6;
        }

        length.length_counter = length.initial_length_time;
    }

    void PulseChannel::write_nrX2(uint8_t nr12) {
        uint8_t old_pace = envelope.envelope_sweep_pace;
        uint8_t old_direction = envelope.envelope_direction;

        envelope.envelope_sweep_pace = nr12 & 0b00000111;
        envelope.envelope_direction = (nr12 & 0b00001000) >> 3;
        envelope.initial_envelope_volume = (nr12 & 0b11110000) >> 4;

        /*
            "Zombie mode" from https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware
            Except incrementing by 2 in subtract mode doesn't fix the issue. Incrementing both
            situations by one fixes the problem.
            Fixes Prehistorik Man's broken audio.
        */

        if ((!old_pace && envelope.envelope_enabled) || !old_direction) {
            volume_output++;
        }

        if (old_direction != envelope.envelope_direction) {
            volume_output = 0x10 - volume_output;
        }

        volume_output &= 0x0F;

        if (envelope.initial_envelope_volume == 0 && envelope.envelope_direction == 0) {
            channel_on = false;
        }
    }

    void PulseChannel::write_nrX3(uint8_t nr13) {
        period_low = nr13;
        frequency_too_high = get_combined_period() >= HIGH_FREQUENCY_CUTOFF;
    }

    void PulseChannel::write_nrX4(uint8_t nr14, uint8_t frame_sequencer_counter) {
        period_high = nr14 & 0b00000111;

        bool old_length = length.sound_length_enable;
        length.sound_length_enable = (nr14 & 0b01000000) >> 6;

        if ((!old_length && length.sound_length_enable) && (frame_sequencer_counter & 1)) {
            length.step_length(channel_on);
        }

        trigger_channel = (nr14 & 0b10000000) >> 7;

        frequency_too_high = get_combined_period() >= HIGH_FREQUENCY_CUTOFF;

        if (trigger_channel) {
            trigger(frame_sequencer_counter);
        }
    }

    uint8_t PulseChannel::read_nr10() const {
        if (!has_sweep) {
            return 0xFF;
        }

        uint8_t nr10 = 0;
        nr10 |= sweep_shift;
        nr10 |= sweep_direction << 3;
        nr10 |= sweep_pace << 4;

        return nr10;
    }

    uint8_t PulseChannel::read_nrX1() const {
        uint8_t nr11 = wave_duty << 6;

        return nr11;
    }

    uint8_t PulseChannel::read_nrX2() const {
        uint8_t nr12 = 0;
        nr12 |= envelope.envelope_sweep_pace;
        nr12 |= envelope.envelope_direction << 3;
        nr12 |= envelope.initial_envelope_volume << 4;
        return nr12;
    }

    uint8_t PulseChannel::read_nrX4() const {
        uint8_t nr14 = length.sound_length_enable << 6;
        return nr14;
    }

    uint16_t PulseChannel::get_combined_period() const { return (period_high << 8) | period_low; }

    void PulseChannel::update_split_period(uint16_t value) {
        period_low = value & 0xFF;
        period_high = (value & 0b11100000000) >> 8;

        frequency_too_high = get_combined_period() >= HIGH_FREQUENCY_CUTOFF;
    }

    uint16_t PulseChannel::calculate_period() {
        uint16_t new_period = period_shadow >> sweep_shift;

        if (sweep_direction) {
            new_period = period_shadow - new_period;
        } else {
            new_period = period_shadow + new_period;
        }

        if (new_period > 2047) {
            channel_on = false;
        }

        return new_period;
    }

    void WaveChannel::step(const std::array<uint8_t, 16> &wave_table) {
        if (channel_on && dac_enabled) {
            if (period_counter <= 0) {
                period_counter = (0x800 - get_combined_period()) * 2;

                position_counter = (position_counter + 1) % 32;
                buffer = wave_table[position_counter / 2];

                if ((position_counter & 1) == 0) {
                    buffer >>= 4;
                }
            } else {
                period_counter--;
            }
        }
    }

    uint8_t WaveChannel::sample(uint8_t side) {
        if (!channel_on || !dac_enabled) {
            return 0;
        }

        uint8_t volume = (buffer & 0xF) >> WAVE_VOLUME[output_level];

        if (frequency_too_high) {
            volume = 0;
        }

        if (side == 0 && left_out_enabled) {
            return volume;
        } else if (side == 1 && right_out_enabled) {
            return volume;
        }

        return 0;
    }

    void WaveChannel::trigger(uint8_t frame_sequencer_counter) {
        if (dac_enabled) {
            channel_on = true;
        }

        if (length.length_counter == 0) {
            length.length_counter = 256;

            if (frame_sequencer_counter & 1) {
                length.step_length(channel_on);
            }
        }

        position_counter = 0;

        period_counter = (0x800 - get_combined_period()) * 2;
    }

    void WaveChannel::write_nr30(uint8_t nr30) {
        dac_enabled = (nr30 & 0b10000000) >> 7;

        if (!dac_enabled) {
            channel_on = false;
        }
    }

    void WaveChannel::write_nr31(uint8_t nr31) {
        length.initial_length_time = 256 - (int)nr31;
        length.length_counter = length.initial_length_time;
    }

    void WaveChannel::write_nr32(uint8_t nr32) { output_level = (nr32 & 0b01100000) >> 5; }

    void WaveChannel::write_nr33(uint8_t nr33) {
        period_low = nr33;
        frequency_too_high = get_combined_period() >= HIGH_FREQUENCY_CUTOFF;
    }

    void WaveChannel::write_nr34(uint8_t nr34, uint8_t frame_sequencer_counter) {
        period_high = nr34 & 0b00000111;

        bool old_length = length.sound_length_enable;
        length.sound_length_enable = (nr34 & 0b01000000) >> 6;

        if ((!old_length && length.sound_length_enable) && (frame_sequencer_counter & 1)) {
            length.step_length(channel_on);
        }

        trigger_channel = (nr34 & 0b10000000) >> 7;
        frequency_too_high = get_combined_period() >= HIGH_FREQUENCY_CUTOFF;

        if (trigger_channel) {
            trigger(frame_sequencer_counter);
        }
    }

    uint8_t WaveChannel::read_nr30() const { return dac_enabled << 7; }

    uint8_t WaveChannel::read_nr32() const { return output_level << 5; }

    uint8_t WaveChannel::read_nr34() const {
        uint8_t nr34 = length.sound_length_enable << 6;
        return nr34;
    }

    uint16_t WaveChannel::get_combined_period() const { return (period_high << 8) | period_low; }

    void NoiseChannel::step() {
        if (!channel_on) {
            return;
        }

        if (period_counter == 0) {
            period_counter = (NOISE_DIV[clock_divider] << clock_shift) * 4;

            uint16_t _xor = ((LFSR & 1) ^ ((LFSR & 2) >> 1));
            LFSR |= _xor << 15;

            if (LFSR_width == 1) {
                LFSR &= ~(0x80);
                LFSR |= _xor << 7;
            }
            LFSR >>= 1;
        } else {
            period_counter--;
        }
    }

    uint8_t NoiseChannel::sample(uint8_t side) {
        if (!channel_on) {
            return 0;
        }

        uint8_t volume = (LFSR & 1) * volume_output;

        if (side == 0 && left_out_enabled) {
            return volume;
        } else if (side == 1 && right_out_enabled) {
            return volume;
        }

        return 0;
    }

    void NoiseChannel::trigger(uint8_t frame_sequencer_counter) {
        channel_on = true;

        if (length.length_counter == 0) {
            length.length_counter = 64;

            if (frame_sequencer_counter & 1) {
                length.step_length(channel_on);
            }
        }

        period_counter = (NOISE_DIV[clock_divider] << clock_shift) * 4;
        LFSR = 0x7FFF;

        envelope.envelope_counter = envelope.envelope_sweep_pace ? envelope.envelope_sweep_pace : 8;
        volume_output = envelope.initial_envelope_volume;
        envelope.envelope_enabled = true;

        if (envelope.initial_envelope_volume == 0 && envelope.envelope_direction == 0) {
            channel_on = false;
        }
    }

    void NoiseChannel::write_nr41(uint8_t nr41) {
        length.length_counter = length.initial_length_time = 64 - (nr41 & 0b00111111);
    }

    void NoiseChannel::write_nr42(uint8_t nr42) {
        uint8_t old_pace = envelope.envelope_sweep_pace;
        uint8_t old_direction = envelope.envelope_direction;

        envelope.envelope_sweep_pace = nr42 & 0b00000111;
        envelope.envelope_direction = (nr42 & 0b00001000) >> 3;
        envelope.initial_envelope_volume = (nr42 & 0b11110000) >> 4;

        // see: PulseChannel::write_nrx2 for explanation

        if ((!old_pace && envelope.envelope_enabled) || !old_direction) {
            volume_output++;
        }

        if (old_direction != envelope.envelope_direction) {
            volume_output = 0x10 - volume_output;
        }

        volume_output &= 0x0F;

        if (envelope.initial_envelope_volume == 0 && envelope.envelope_direction == 0) {
            channel_on = false;
        }
    }

    uint8_t NoiseChannel::read_nr42() const {
        uint8_t nr42 = 0;
        nr42 |= envelope.envelope_sweep_pace;
        nr42 |= envelope.envelope_direction << 3;
        nr42 |= envelope.initial_envelope_volume << 4;
        return nr42;
    }

    void NoiseChannel::write_nr43(uint8_t nr43) {
        clock_divider = nr43 & 0b00000111;
        LFSR_width = (nr43 & 0b00001000) >> 3;
        clock_shift = (nr43 & 0b11110000) >> 4;
    }

    void NoiseChannel::write_nr44(uint8_t nr44, uint8_t frame_sequencer_counter) {
        bool old_length = length.sound_length_enable;
        length.sound_length_enable = (nr44 & 0b01000000) >> 6;

        if ((!old_length && length.sound_length_enable) && (frame_sequencer_counter & 1)) {
            length.step_length(channel_on);
        }

        trigger_channel = (nr44 & 0b10000000) >> 7;

        if (trigger_channel) {
            trigger(frame_sequencer_counter);
        }
    }

    uint8_t NoiseChannel::read_nr43() const {
        uint8_t nr43 = 0;
        nr43 |= clock_divider;
        nr43 |= LFSR_width << 3;
        nr43 |= clock_shift << 4;
        return nr43;
    }

    uint8_t NoiseChannel::read_nr44() const { return length.sound_length_enable << 6; }

    void APU::reset() {
        stereo_left_volume = 7;
        stereo_right_volume = 7;
        mix_vin_left = false;
        mix_vin_right = false;
        sample_counter = sample_rate = 0;
        frame_sequencer_counter = 0;
        power = true;

        pulse_1 = PulseChannel(true);
        pulse_2 = PulseChannel(false);
        wave = WaveChannel();
        noise = NoiseChannel();
        wave_table.fill(0);
    }

    void APU::set_samples_callback(int32_t rate, std::function<void(SampleResult result)> cb) {
        samples_ready_func = cb;
        sample_rate = rate;
        sample_counter = 0;
    }

    uint8_t APU::read_register(uint8_t address) {
        switch (address) {
        // Pulse 1
        case 0x10: {
            return pulse_1.read_nr10() | 0x80;
        }
        case 0x11: {
            return pulse_1.read_nrX1() | 0x3F;
        }
        case 0x12: {
            return pulse_1.read_nrX2();
        }
        case 0x13: {
            return 0xFF;
        }
        case 0x14: {
            return pulse_1.read_nrX4() | 0xBF;
        }

        // Pulse 2
        case 0x15: {
            return 0xFF;
        }
        case 0x16: {
            return pulse_2.read_nrX1() | 0x3F;
        }
        case 0x17: {
            return pulse_2.read_nrX2();
        }
        case 0x18: {
            return 0xFF;
        }
        case 0x19: {
            return pulse_2.read_nrX4() | 0xBF;
        }

        // Wave
        case 0x1A: {
            return wave.read_nr30() | 0x7F;
        }
        case 0x1B: {
            return 0xFF;
        }
        case 0x1C: {
            return wave.read_nr32() | 0x9F;
        }
        case 0x1D: {
            return 0xFF;
        }
        case 0x1E: {
            return wave.read_nr34() | 0xBF;
        }

        // Noise
        case 0x1F: {
            return 0xFF;
        }
        case 0x20: {
            return 0xFF;
        }
        case 0x21: {
            return noise.read_nr42();
        }
        case 0x22: {
            return noise.read_nr43();
        }
        case 0x23: {
            return noise.read_nr44() | 0xBF;
        }

        // APU Registers
        case 0x24: {
            return read_nr50();
        }
        case 0x25: {
            return read_nr51();
        }
        case 0x26: {
            return read_nr52();
        }
        case 0x27: {
            return 0xFF;
        }
        }
        return 0;
    }

    void APU::write_register(uint8_t address, uint8_t value) {
        if (power) {
            switch (address) {
            case 0x10: {
                pulse_1.write_nr10(value);
                return;
            }
            case 0x11: {
                pulse_1.write_nrX1(power, value);
                return;
            }
            case 0x12: {
                pulse_1.write_nrX2(value);
                return;
            }
            case 0x13: {
                pulse_1.write_nrX3(value);
                return;
            }
            case 0x14: {
                pulse_1.write_nrX4(value, frame_sequencer_counter);
                return;
            }

            case 0x15: {
                return;
            }
            case 0x16: {
                pulse_2.write_nrX1(power, value);
                return;
            }
            case 0x17: {
                pulse_2.write_nrX2(value);
                return;
            }
            case 0x18: {
                pulse_2.write_nrX3(value);
                return;
            }
            case 0x19: {
                pulse_2.write_nrX4(value, frame_sequencer_counter);
                return;
            }

            case 0x1A: {
                wave.write_nr30(value);
                return;
            }
            case 0x1B: {
                wave.write_nr31(value);
                return;
            }
            case 0x1C: {
                wave.write_nr32(value);
                return;
            }
            case 0x1D: {
                wave.write_nr33(value);
                return;
            }
            case 0x1E: {
                wave.write_nr34(value, frame_sequencer_counter);
                return;
            }

            case 0x1F: {
                return;
            }

            case 0x20: {
                noise.write_nr41(value);
                return;
            }
            case 0x21: {
                noise.write_nr42(value);
                return;
            }
            case 0x22: {
                noise.write_nr43(value);
                return;
            }
            case 0x23: {
                noise.write_nr44(value, frame_sequencer_counter);
                return;
            }

            case 0x24: {
                write_nr50(value);
                return;
            }
            case 0x25: {
                write_nr51(value);
                return;
            }
            case 0x26: {
                write_nr52(value);
                return;
            }
            case 0x27: {
                return;
            }
            }
        } else {
            // Length counters are always writable on DMG units
            switch (address) {
            case 0x11: {
                pulse_1.write_nrX1(power, value);
                return;
            }
            case 0x16: {
                pulse_2.write_nrX1(power, value);
                return;
            }
            case 0x1B: {
                wave.write_nr31(value);
                return;
            }
            case 0x20: {
                noise.write_nr41(value);
                return;
            }
            case 0x26: {
                write_nr52(value);
                return;
            }
            }
        }
    }

    uint8_t APU::read_wave_ram(uint8_t address) { return wave_table[address]; }

    void APU::write_wave_ram(uint8_t address, uint8_t value) { wave_table[address] = value; }

    void APU::write_nr52(uint8_t value) {
        power = (value & 0b10000000) > 0;
        if (power == 0) {
            pulse_1 = PulseChannel(true);
            pulse_2 = PulseChannel(false);
            wave = WaveChannel();
            noise = NoiseChannel();
            stereo_left_volume = 0;
            stereo_right_volume = 0;
            mix_vin_left = false;
            mix_vin_right = false;
        }
    }

    uint8_t APU::read_nr52() const {
        uint8_t nr52 = 0x70;
        nr52 |= power ? 0b10000000 : 0;
        nr52 |= pulse_1.channel_on ? 0b00000001 : 0;
        nr52 |= pulse_2.channel_on ? 0b00000010 : 0;
        nr52 |= wave.channel_on ? 0b00000100 : 0;
        nr52 |= noise.channel_on ? 0b00001000 : 0;

        return nr52;
    }

    void APU::write_nr50(uint8_t value) {
        stereo_right_volume = value & 0b00000111;
        stereo_left_volume = (value & 0b01110000) >> 4;

        mix_vin_left = (value & 0b10000000) > 0;
        mix_vin_right = (value & 0b00001000) > 0;
    }

    void APU::write_nr51(uint8_t value) {
        pulse_1.left_out_enabled = (value & 0b00010000) > 0;
        pulse_1.right_out_enabled = value & 0b00000001;

        pulse_2.left_out_enabled = (value & 0b00100000) > 0;
        pulse_2.right_out_enabled = value & 0b00000010;

        wave.left_out_enabled = (value & 0b01000000) > 0;
        wave.right_out_enabled = value & 0b00000100;

        noise.left_out_enabled = (value & 0b10000000) > 0;
        noise.right_out_enabled = value & 0b00001000;
    }

    uint8_t APU::read_nr50() const {
        uint8_t nr50 = 0;
        nr50 |= stereo_right_volume & 0b00000111;
        nr50 |= (stereo_left_volume & 0b00000111) << 4;
        nr50 |= mix_vin_right ? 0b00001000 : 0;
        nr50 |= mix_vin_left ? 0b10000000 : 0;
        return nr50;
    }

    uint8_t APU::read_nr51() const {
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

    void APU::step(uint32_t cycles) {
        for (int i = 0; i < cycles; ++i) {
            pulse_1.step_frequency();
            pulse_2.step_frequency();
            wave.step(wave_table);
            noise.step();
            sample_counter++;

            if (sample_counter % sample_rate == 0) {
                if (samples_ready_func) {
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
        }
    }
    void APU::step_frame_sequencer() {
        switch (frame_sequencer_counter) {
        case 0:
        case 4: {
            pulse_1.length.step_length(pulse_1.channel_on);
            pulse_2.length.step_length(pulse_2.channel_on);
            if (wave.dac_enabled) {
                wave.length.step_length(wave.channel_on);
            }
            noise.length.step_length(noise.channel_on);
            break;
        }

        case 2:
        case 6: {
            pulse_1.step_frequency_sweep();
            pulse_1.length.step_length(pulse_1.channel_on);
            pulse_2.length.step_length(pulse_2.channel_on);
            if (wave.dac_enabled) {
                wave.length.step_length(wave.channel_on);
            }
            noise.length.step_length(noise.channel_on);
            break;
        }

        case 7: {
            if (pulse_1.channel_on) {
                pulse_1.envelope.step_envelope_sweep(pulse_1.volume_output);
            }
            if (pulse_2.channel_on) {
                pulse_2.envelope.step_envelope_sweep(pulse_2.volume_output);
            }
            if (noise.channel_on) {
                noise.envelope.step_envelope_sweep(noise.volume_output);
            }
            break;
        }
        }

        frame_sequencer_counter = ++frame_sequencer_counter & 7;
    }

}