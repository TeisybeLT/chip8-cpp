#include "sdl_window.hpp"
#include "errors/sdl_exception.hpp"

#include <SDL_log.h>

using namespace sdl;
using namespace std::literals::string_literals;

window::window(const std::u8string& title, const SDL_Rect& window_rect)
{
	SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Creating window \"%s\" at %d:%d with the size of %dx%d",
		reinterpret_cast<const char*>(title.c_str()), window_rect.x, window_rect.y,
		window_rect.w, window_rect.h);

	// Create window
	this->m_window = SDL_CreateWindow(reinterpret_cast<const char*>(title.c_str()),
		window_rect.x, window_rect.y, window_rect.w, window_rect.h, 0);

	sdl::sdl_check_null(this->m_window, "Failed to create window"s);
}

window::~window()
{
	SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Closing window");
	SDL_DestroyWindow(this->m_window);
}

SDL_Surface& window::get_window_surface() const
{
	auto window_surface = SDL_GetWindowSurface(this->m_window);
	sdl::sdl_check_null(window_surface, "Failed to get window surface");
	return *window_surface;
}

void window::update()
{
	sdl::sdl_check_error(SDL_UpdateWindowSurface(this->m_window), "Failed to update window"s);
}
