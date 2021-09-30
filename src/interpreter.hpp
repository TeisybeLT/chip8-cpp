#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "display.hpp"

#include <SDL_events.h>

#include <array>
#include <filesystem>

namespace chip8
{
	struct interpreter
	{
		static constexpr auto c_mem_size = size_t {4096};
		static constexpr auto c_code_start = uint16_t {0x200};

		interpreter(const std::filesystem::path& rom_path, sdl::window& interpreter_window);

		void run();

	private:
		void process_events();

		bool m_is_running;
		sdl::window& m_interpreter_window;
		display m_display;
		SDL_Event m_evt;

		std::array<std::byte, c_mem_size> m_mem;
	};
}

#endif /* INTERPRETER_HPP */
