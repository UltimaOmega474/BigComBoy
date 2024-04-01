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
#include "Cores/GB/APU.hpp"
#include <SDL.h>
#include <vector>

namespace QtFrontend {
    class AudioSystem {
        struct AudioSample {
            float left = 0.0f;
            float right = 0.0f;
        };

    public:
        AudioSystem();
        ~AudioSystem();
        AudioSystem(const AudioSystem &) = delete;
        AudioSystem(AudioSystem &&other) noexcept;
        AudioSystem &operator=(const AudioSystem &) = delete;
        AudioSystem &operator=(AudioSystem &&other) noexcept;

        void open_device();
        void close_device();
        bool should_continue();
        void operator()(GB::SampleResult result);
        void prep_for_playback(GB::APU &apu);

    private:
        bool opened = false;
        SDL_AudioSpec obtained{};
        SDL_AudioDeviceID audio_device = 0;
        std::vector<AudioSample> samples{};
    };
}
