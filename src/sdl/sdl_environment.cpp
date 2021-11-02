#include "sdl_environment.hpp"
#include "errors/sdl_exception.hpp"

#include <SDL.h>

using namespace sdl;
using namespace std::literals::string_literals;

environment::environment()
{
	SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "SDL Initializing");

	// Initialize video
	sdl::sdl_check_error(SDL_InitSubSystem(SDL_INIT_VIDEO), "Unable to initialize SDL video"s);
	SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "SDL video init OK!");

	// Initialize audio
	if (SDL_InitSubSystem(SDL_INIT_AUDIO))
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize SDL audio: %s", SDL_GetError());
	else
		SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "SDL audio init OK!");
}

environment::~environment()
{
	SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "De-initializing SDL");

	// Close created items
	m_windows.clear();
	m_beepers.clear();

	// Close SDL stuff
	SDL_VideoQuit();
	SDL_AudioQuit();
}

window& environment::create_window(const std::u8string& title, const SDL_Rect& window_rect)
{
	return this->m_windows.emplace_back(title, window_rect);
}

beeper& environment::create_beeper(uint16_t freq, uint8_t amplitude)
{
	return this->m_beepers.emplace_back(freq, amplitude);
}
