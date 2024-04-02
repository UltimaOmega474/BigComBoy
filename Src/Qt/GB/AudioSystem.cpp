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

#include "AudioSystem.hpp"
#include "Common/Config.hpp"
#include "Cores/GB/Constants.hpp"

namespace QtFrontend {
    constexpr bool SYNC_TO_AUDIO = true;
    constexpr float MAX_LAG = 0.04f;
    constexpr float VOLUME_SCALE = 255.0f;

    AudioSystem::AudioSystem() { open_device(); }

    AudioSystem::~AudioSystem() { close_device(); }

    void AudioSystem::open_device() {
        if (opened) {
            return;
        }

        SDL_AudioSpec audio_spec{};
        audio_spec.freq = 48000;
        audio_spec.format = AUDIO_F32SYS;
        audio_spec.channels = 2;
        audio_spec.samples = 512;
        audio_spec.callback = NULL;
        audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, &obtained, 0);

        opened = true;
    }

    void AudioSystem::close_device() {
        SDL_CloseAudioDevice(audio_device);
        samples.clear();
        opened = false;
        audio_device = 0;
    }

    bool AudioSystem::should_continue() {
        float samples_remain =
            static_cast<float>(SDL_GetQueuedAudioSize(audio_device) / (sizeof(AudioSample)));
        float lag_threshold = static_cast<float>(obtained.freq) * MAX_LAG;

        if (samples_remain > lag_threshold) {
            if (SYNC_TO_AUDIO) {
                return false;
            } else {
                return true;
            }
        }

        return true;
    }

    void AudioSystem::operator()(GB::SampleResult result) {
        const auto &config = Common::Config::current().gameboy;

        float left_vol = static_cast<float>(128 * result.left_channel.master_volume) / 7;
        float right_vol = static_cast<float>(128 * result.right_channel.master_volume) / 7;

        float volume = static_cast<float>(config.audio.volume) / 100.0f;
        float square1 = static_cast<float>(config.audio.square1) / 100.0f;
        float square2 = static_cast<float>(config.audio.square2) / 100.0f;
        float wave = static_cast<float>(config.audio.wave) / 100.0f;
        float noise = static_cast<float>(config.audio.noise) / 100.0f;

        float sample_left = 0;
        float input = static_cast<float>(result.left_channel.pulse_1) / VOLUME_SCALE;
        SDL_MixAudioFormat(reinterpret_cast<Uint8 *>(&sample_left),
                           reinterpret_cast<Uint8 *>(&input), AUDIO_F32SYS, sizeof(float),
                           static_cast<int>(left_vol * square1));
        input = static_cast<float>(result.left_channel.pulse_2) / VOLUME_SCALE;
        SDL_MixAudioFormat(reinterpret_cast<Uint8 *>(&sample_left),
                           reinterpret_cast<Uint8 *>(&input), AUDIO_F32SYS, sizeof(float),
                           static_cast<int>(left_vol * square2));
        input = static_cast<float>(result.left_channel.wave) / VOLUME_SCALE;
        SDL_MixAudioFormat(reinterpret_cast<Uint8 *>(&sample_left),
                           reinterpret_cast<Uint8 *>(&input), AUDIO_F32SYS, sizeof(float),
                           static_cast<int>(left_vol * wave));
        input = static_cast<float>(result.left_channel.noise) / VOLUME_SCALE;
        SDL_MixAudioFormat(reinterpret_cast<Uint8 *>(&sample_left),
                           reinterpret_cast<Uint8 *>(&input), AUDIO_F32SYS, sizeof(float),
                           static_cast<int>(left_vol * noise));

        float sample_right = 0;
        input = static_cast<float>(result.right_channel.pulse_1) / VOLUME_SCALE;
        SDL_MixAudioFormat(reinterpret_cast<Uint8 *>(&sample_right),
                           reinterpret_cast<Uint8 *>(&input), AUDIO_F32SYS, sizeof(float),
                           static_cast<int>(right_vol * square1));
        input = static_cast<float>(result.right_channel.pulse_2) / VOLUME_SCALE;
        SDL_MixAudioFormat(reinterpret_cast<Uint8 *>(&sample_right),
                           reinterpret_cast<Uint8 *>(&input), AUDIO_F32SYS, sizeof(float),
                           static_cast<int>(right_vol * square2));
        input = static_cast<float>(result.right_channel.wave) / VOLUME_SCALE;
        SDL_MixAudioFormat(reinterpret_cast<Uint8 *>(&sample_right),
                           reinterpret_cast<Uint8 *>(&input), AUDIO_F32SYS, sizeof(float),
                           static_cast<int>(right_vol * wave));
        input = static_cast<float>(result.right_channel.noise) / VOLUME_SCALE;
        SDL_MixAudioFormat(reinterpret_cast<Uint8 *>(&sample_right),
                           reinterpret_cast<Uint8 *>(&input), AUDIO_F32SYS, sizeof(float),
                           static_cast<int>(right_vol * noise));

        samples.push_back({.left = sample_left * volume, .right = sample_right * volume});

        if (samples.size() == obtained.samples) {
            SDL_QueueAudio(audio_device, samples.data(), samples.size() * sizeof(AudioSample));
            samples.clear();
        }
    }

    void AudioSystem::prep_for_playback(GB::APU &apu) {
        if (!opened) {
            return;
        }

        SDL_PauseAudioDevice(audio_device, 0);
        samples.clear();
        samples.reserve(obtained.samples);

        apu.set_samples_callback(GB::CPU_CLOCK_RATE / obtained.freq,
                                 [this](GB::SampleResult samples) { this->operator()(samples); });
    }
}