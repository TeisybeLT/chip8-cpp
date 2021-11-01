#ifndef SDL_ENVIRONMENT_HPP
#define SDL_ENVIRONMENT_HPP

#include "sdl_beeper.hpp"
#include "sdl_window.hpp"

#include <list>

namespace sdl
{
	struct environment
	{
		environment();
		~environment();

		[[nodiscard]] window& create_window(const std::u8string& title, const SDL_Rect& window_rect);
		[[nodiscard]] beeper& create_beeper(uint16_t freq, uint8_t amplitude);

	private:
		std::list<window> m_windows;
		std::list<beeper> m_beepers;
	};
}

#endif /* SDL_ENVIRONMENT_HPP */
