#ifndef SDL_BEEPER_HPP
#define SDL_BEEPER_HPP

#include <SDL_audio.h>

#include <vector>

namespace sdl
{
	struct beeper
	{
		beeper(uint16_t freq, uint8_t amplitude);
		~beeper();

		beeper(const beeper&) = delete;
		beeper& operator=(const beeper&) = delete;

		beeper(beeper&&) = delete;
		beeper& operator=(beeper&&) = delete;

		void play() noexcept;
		void pause() noexcept;
		bool is_playing() const noexcept;

	private:
		SDL_AudioDeviceID m_audio_device;
		std::vector<uint8_t> m_sample_cache;
	};
}

#endif /* BEEPER_HPP */
