#ifndef SDL_WINDOW_HPP
#define SDL_WINDOW_HPP

#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_video.h>

#include <string>

namespace sdl
{
	struct window
	{
		window(const std::u8string& title, const SDL_Rect& window_rect);
		~window();

		window(const window&) = delete;
		window& operator=(const window&) = delete;

		window(window&&) = delete;
		window& operator=(window&&) = delete;

		[[nodiscard]] SDL_Surface& get_window_surface() const;

		void update();

	private:
		SDL_Window* m_window;
	};
}

#endif /* SDL_WINDOW_HPP */
