#pragma once
#include <gb/apu.hpp>
#include <SDL.h>
#include <vector>

namespace SunBoy
{

	class AudioSystem
	{
		bool opened = false;
		struct AudioSample
		{
			float left = 0.0f;
			float right = 0.0f;
		};

	public:
		SDL_AudioSpec obtained{};
		SDL_AudioDeviceID audio_device = 0;
		std::vector<AudioSample> samples;

		void open_device();
		void close_device();
		void operator()(SampleResult result);
		void prep_for_playback(APU &apu);
	};
}
