#include "sdl_environment.hpp"
#include "interpreter.hpp"

#include "cxxopts.hpp"
#include <SDL_log.h>
#include <SDL_version.h>

using namespace std::literals::string_literals;

int main(int argc, char* argv[]) try
{
	SDL_Log("Chip8-cpp interpreter");
	SDL_Log("SDL Version: %d.%d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);

	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG); 

	// Build SDL related stuff
	auto sdl_game = sdl::environment();
	auto& interpreter_window = sdl_game.create_window(u8"Chip8-cpp interpreter"s, SDL_Rect{0, 0, 1280, 640});
	auto& beeper = sdl_game.create_beeper(440, 128);

	// Start interpreter
	auto chip8_interpreter = chip8::interpreter("/home/teisybe/Devel/tmp/chip8/roms/IBM Logo.ch8", interpreter_window, beeper, std::chrono::duration_cast<std::chrono::nanoseconds>(1s) / 500);

	chip8_interpreter.run();

	return EXIT_SUCCESS;
}
catch(std::exception& e)
{
	SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unhandled exception: %s", e.what());
	return EXIT_FAILURE;
}
catch(...)
{
	SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unhandled unknown exception");
	return EXIT_FAILURE;
}
