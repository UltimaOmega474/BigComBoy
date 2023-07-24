#include "audio.hpp"
#include "config.hpp"
#include "gui_constants.hpp"
namespace SunBoy
{
	void AudioSystem::open_device()
	{
		if (opened)
			return;

		SDL_AudioSpec audio_spec{};
		audio_spec.freq = 44100;
		audio_spec.format = AUDIO_F32SYS;
		audio_spec.channels = 2;

		float latency = AUDIO_LATENCY_TABLE[Configuration::get().audio_latency_select];
		float target_samples = std::floor(static_cast<float>(audio_spec.freq) * latency / 1000.0f);

		audio_spec.samples = (int)target_samples / audio_spec.channels;
		audio_spec.callback = NULL;
		audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, &obtained, 0);

		opened = true;
	}

	void AudioSystem::close_device()
	{
		SDL_CloseAudioDevice(audio_device);
	}

	void AudioSystem::operator()(SampleResult result)
	{
		constexpr float VOLUME_SCALE = 256.0f;
		int32_t left_vol = (128 * result.left_channel.master_volume) / 7;
		int32_t right_vol = (128 * result.right_channel.master_volume) / 7;
		uint8_t volume_uint = Configuration::get().audio_master_volume;
		float volume = static_cast<float>(volume_uint) / 100.0f;

		if (volume_uint == 0)
		{
			samples.clear();
			return;
		}

		float sample_left = 0;
		float input = static_cast<float>(result.left_channel.pulse_1) / VOLUME_SCALE;
		SDL_MixAudioFormat(reinterpret_cast<Uint8 *>(&sample_left), reinterpret_cast<Uint8 *>(&input), AUDIO_F32SYS, sizeof(float), left_vol);
		input = static_cast<float>(result.left_channel.pulse_2) / VOLUME_SCALE;
		SDL_MixAudioFormat(reinterpret_cast<Uint8 *>(&sample_left), reinterpret_cast<Uint8 *>(&input), AUDIO_F32SYS, sizeof(float), left_vol);
		input = static_cast<float>(result.left_channel.wave) / VOLUME_SCALE;
		SDL_MixAudioFormat(reinterpret_cast<Uint8 *>(&sample_left), reinterpret_cast<Uint8 *>(&input), AUDIO_F32SYS, sizeof(float), left_vol);
		input = static_cast<float>(result.left_channel.noise) / VOLUME_SCALE;
		SDL_MixAudioFormat(reinterpret_cast<Uint8 *>(&sample_left), reinterpret_cast<Uint8 *>(&input), AUDIO_F32SYS, sizeof(float), left_vol);

		float sample_right = 0;
		input = static_cast<float>(result.right_channel.pulse_1) / VOLUME_SCALE;
		SDL_MixAudioFormat(reinterpret_cast<Uint8 *>(&sample_right), reinterpret_cast<Uint8 *>(&input), AUDIO_F32SYS, sizeof(float), right_vol);
		input = static_cast<float>(result.right_channel.pulse_2) / VOLUME_SCALE;
		SDL_MixAudioFormat(reinterpret_cast<Uint8 *>(&sample_right), reinterpret_cast<Uint8 *>(&input), AUDIO_F32SYS, sizeof(float), right_vol);
		input = static_cast<float>(result.right_channel.wave) / VOLUME_SCALE;
		SDL_MixAudioFormat(reinterpret_cast<Uint8 *>(&sample_right), reinterpret_cast<Uint8 *>(&input), AUDIO_F32SYS, sizeof(float), right_vol);
		input = static_cast<float>(result.right_channel.noise) / VOLUME_SCALE;
		SDL_MixAudioFormat(reinterpret_cast<Uint8 *>(&sample_right), reinterpret_cast<Uint8 *>(&input), AUDIO_F32SYS, sizeof(float), right_vol);

		if (samples.size() < obtained.samples)
			samples.push_back({sample_left * volume, sample_right * volume});

		if (samples.size() >= obtained.samples)
		{
			if (SDL_GetQueuedAudioSize(audio_device) > samples.size() * sizeof(AudioSample))
				return;

			SDL_QueueAudio(audio_device, samples.data(), samples.size() * sizeof(AudioSample));
			samples.clear();
		}
	}

	void AudioSystem::prep_for_playback(APU &apu)
	{
		if (!opened)
			return;
		SDL_PauseAudioDevice(audio_device, 0);
		apu.sample_rate = CPU_CLOCK_RATE / obtained.freq;
		samples.clear();
		samples.reserve(obtained.samples);
		apu.samples_ready_func = *this;
	}
}