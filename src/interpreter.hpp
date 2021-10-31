#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "constants.hpp"
#include "display.hpp"
#include "registers.hpp"
#include "timer.hpp"
#include "sdl_beeper.hpp"

#include <SDL_events.h>

#include <array>
#include <filesystem>

namespace chip8
{
	struct interpreter
	{
		interpreter(
			const std::filesystem::path& rom_path,
			sdl::window& interpreter_window,
			sdl::beeper& beeper,
			std::chrono::nanoseconds tick_period);

		void run();

	private:
		void process_events();
		void process_timers(const std::chrono::nanoseconds& delta);
		void process_machine_tick();

		bool m_is_running;
		sdl::window& m_interpreter_window;
		display m_display;
		const std::chrono::nanoseconds m_machine_tick_period;
		SDL_Event m_evt;

		std::vector<chip8::timer> m_timers;
		registers m_registers;

		std::array<std::byte, constants::mem_size> m_mem;
		std::array<uint16_t, constants::stack_size> m_stack;
		std::vector<bool> m_video_mem;
	};
}

#endif /* INTERPRETER_HPP */
