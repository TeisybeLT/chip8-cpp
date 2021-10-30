#include "sdl_environment.hpp"
#include "interpreter.hpp"

#include "cxxopts.hpp"
#include <SDL_log.h>
#include <SDL_version.h>

using namespace std::literals::string_literals;

namespace
{
	[[nodiscard]] auto set_up_options()
	{
		auto opts = cxxopts::Options("chip8-cpp"s, "A simple chip8 interpreter written in C++"s);

		opts.add_options()
			("h, help"s, "Show help screen"s)
			("r, rom"s, "Path to chip8 (*.ch8) rom file"s, cxxopts::value<std::string>())
			("f, freq"s, "Speed of emulation", cxxopts::value<int>()->default_value("500"s))
			("d, debug"s, "Enable debug strings"s, cxxopts::value<bool>())
			("upscale-mult"s, "Resolution multiplier"s, cxxopts::value<int>()->default_value("20"));

		return opts;
	}

	void parse_debug_logging(const cxxopts::ParseResult& parse_result)
	{
		if (parse_result["debug"].count())
		{
			SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG); 
			SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Debug logging enabled");
		}
	}

	[[nodiscard]] auto parse_rom_path(const cxxopts::ParseResult& parse_result)
	{
		if (parse_result["rom"].count())
		{
			auto path = parse_result["rom"].as<std::string>();
			SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Rom file: %s", path.c_str());
			return std::filesystem::path{path};
		}

		return std::filesystem::path{};
	}

	[[nodiscard]] auto parse_machine_tick_rate(const cxxopts::ParseResult& parse_result)
	{
		auto freq = parse_result["freq"].as<int>();
		SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Interpreter frequency: %d Hz", freq);
		return std::chrono::duration_cast<std::chrono::nanoseconds>(1s) / freq;
	}

	[[nodiscard]] auto parse_upscale_multiplier(const cxxopts::ParseResult& parse_result)
	{
		auto mult = parse_result["upscale-mult"].as<int>();
		SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Upscale multiplier: %d", mult);
		return mult;
	}
}

int main(int argc, char* argv[]) try
{
	SDL_Log("Chip8-cpp interpreter");
	SDL_Log("SDL Version: %d.%d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);

	// Parse reults
	auto options = set_up_options();
	const auto parse_result = options.parse(argc, argv);
	if (parse_result.count("help"))
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, options.help().c_str());
		return EXIT_SUCCESS;
	}


	parse_debug_logging(parse_result);
	auto rom_path = parse_rom_path(parse_result);
	if (rom_path.empty())
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "No rom path provided. Please use '-r/--rom'"
			" argument to pass a valid path to a *.ch8 chip8 rom file");
		return EXIT_FAILURE;
	}
	const auto machine_tick_period = parse_machine_tick_rate(parse_result);
	const auto upscale_mult = parse_upscale_multiplier(parse_result);

	// Build SDL related stuff
	auto sdl_game = sdl::environment();
	auto& interpreter_window = sdl_game.create_window(u8"Chip8-cpp interpreter"s, SDL_Rect{0, 0, 64 * upscale_mult, 32 * upscale_mult});
	auto& beeper = sdl_game.create_beeper(440, 128);

	// Start interpreter
	chip8::interpreter(rom_path, interpreter_window, beeper, machine_tick_period).run();

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
