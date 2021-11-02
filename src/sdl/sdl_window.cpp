#include "sdl_window.hpp"

#include <SDL_log.h>

#include <stdexcept>

using namespace sdl;

window::window(const std::u8string& title, const SDL_Rect& window_rect)
{
	SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Creating window \"%s\" at %d:%d with the size of %dx%d",
		reinterpret_cast<const char*>(title.c_str()), window_rect.x, window_rect.y,
		window_rect.w, window_rect.h);

	// Create window
	this->m_window = SDL_CreateWindow(reinterpret_cast<const char*>(title.c_str()),
		window_rect.x, window_rect.y, window_rect.w, window_rect.h, 0);
	if (!this->m_window)
		throw std::runtime_error("Failed to create window: " + std::string(SDL_GetError()));
}

window::~window()
{
	SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Closing window");
	SDL_DestroyWindow(this->m_window);
}

SDL_Surface& window::get_window_surface() const
{
	if (auto window_surface = SDL_GetWindowSurface(this->m_window))
		return *window_surface;
	else
		throw std::runtime_error("Failed to get window surface: " + std::string(SDL_GetError()));
}

void window::update()
{
	if (SDL_UpdateWindowSurface(this->m_window))
		throw std::runtime_error("Failed to update window: " + std::string(SDL_GetError()));
}
