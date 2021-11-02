#include "sdl_beeper.hpp"
#include "errors/sdl_exception.hpp"

#include <SDL_log.h>

#include <cstring>
#include <string>

using namespace sdl;
using namespace std::literals::string_literals;

namespace
{
	auto generate_sample_cache(const SDL_AudioSpec& spec, uint16_t target_freq, uint8_t ampl) noexcept
	{
		auto out = std::vector<uint8_t>(spec.samples);
		const auto sample_pulse_width = spec.freq / 2 / target_freq;

		auto is_high = false;
		for (size_t cnt = 0; cnt < out.size(); ++cnt)
		{
			if ((cnt % sample_pulse_width) == 0)
				is_high = !is_high;

			out[cnt] = is_high ? ampl : 0;
		}

		SDL_LogDebug(SDL_LOG_CATEGORY_AUDIO, "Sample cache was generated with %d samples", out.size());
		return out;
	}
}

beeper::beeper(uint16_t freq, uint8_t amplitude)
{
	SDL_LogDebug(SDL_LOG_CATEGORY_AUDIO, "Creating beeper with %hu Hz and amplitude of %hhu",
		freq, amplitude);

	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;

	std::memset(&desired, 0, sizeof(desired));

	desired.freq = freq * 2;
	desired.format = AUDIO_U8;
	desired.channels = 1;
	desired.samples = 32; // TODO: Tune this, since this is a throughput-vs-latency thing
	desired.callback = +[](void* user_data, Uint8* stream, int len) -> void
	{
		if (len == 0)
			return;

		const auto& buffer = *reinterpret_cast<std::vector<uint8_t>*>(user_data);
		if (len > buffer.size())
		{
			SDL_LogWarn(SDL_LOG_CATEGORY_AUDIO, "More samples requested than was cached, "
				"truncating with silence. Requested: %d, cached: %d", len, buffer.size());

			std::memset(stream + buffer.size(), 0, len - buffer.size());
			len = buffer.size();
		}
		std::memcpy(stream, buffer.data(), len);
	};
	desired.userdata = &this->m_sample_cache;

	this->m_audio_device = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained,
		SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
	sdl::sdl_check_error(this->m_audio_device, "Unable to open audio device"s);

	this->m_sample_cache = generate_sample_cache(obtained, freq, amplitude);
}

beeper::~beeper()
{
	SDL_CloseAudioDevice(this->m_audio_device);
}

void beeper::play() noexcept
{
	SDL_LogDebug(SDL_LOG_CATEGORY_AUDIO, "Beeper audio start");
	SDL_PauseAudioDevice(this->m_audio_device, false);
}

void beeper::pause() noexcept
{
	SDL_LogDebug(SDL_LOG_CATEGORY_AUDIO, "Beeper audio pause");
	SDL_PauseAudioDevice(this->m_audio_device, true);
}

bool beeper::is_playing() const noexcept
{
	return SDL_GetAudioDeviceStatus(this->m_audio_device) == SDL_AUDIO_PLAYING;
}
