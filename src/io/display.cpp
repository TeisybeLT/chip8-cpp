#include "display.hpp"
#include "errors/sdl_exception.hpp"

#include <array>
#include <cstring>

#include <SDL_log.h>

using namespace chip8;
using namespace std::literals::string_literals;

display::display(sdl::window& window, size_t game_width, size_t game_height) :
	m_pixel_count{game_width * game_height}, m_window{window}
{
	this->m_surface = SDL_CreateRGBSurfaceWithFormat(0, static_cast<int>(game_width),
		static_cast<int>(game_height), 32, SDL_PIXELFORMAT_RGB888);
	sdl::sdl_check_null(this->m_surface, "Unable to allocate main game surface"s);
}

display::~display()
{
	SDL_FreeSurface(this->m_surface);
}

void display::draw(const std::vector<bool>& pixels)
{
	if (pixels.size() != this->m_pixel_count)
		throw std::runtime_error("Size mismatch");

	// Update current surface
	for (size_t cnt = 0; cnt < pixels.size(); ++cnt)
		std::memset(&static_cast<std::byte*>(this->m_surface->pixels)[cnt * 4],
			pixels[cnt] ? 0xff : 0x00, 4);

	// Blit everything to main window
	auto& window_surface = this->m_window.get_window_surface();

	sdl::sdl_check_error(SDL_BlitScaled(this->m_surface, nullptr, &window_surface, nullptr),
		"Unable to blit game surface"s);

	this->m_window.update();
}

size_t display::get_pixel_count() const noexcept
{
	return this->m_pixel_count;
}

int display::get_width() const noexcept
{
	return this->m_surface->w;
}

int display::get_height() const noexcept
{
	return this->m_surface->h;
}
