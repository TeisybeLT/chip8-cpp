#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include "sdl/sdl_window.hpp"

#include <SDL_surface.h>

#include <vector>

namespace chip8
{
	struct display
	{
		display(sdl::window& window, size_t game_width, size_t game_height);
		~display();

		void draw(const std::vector<bool>& pixels);

		size_t get_pixel_count() const noexcept;
		int get_width() const noexcept;
		int get_height() const noexcept;

	private:
		const size_t m_pixel_count;
		sdl::window& m_window;
		SDL_Surface* m_surface;
	};
}

#endif /* DISPLAY_HPP */
