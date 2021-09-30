#include "interpreter.hpp"
#include "chip8_font.hpp"

#include <SDL_timer.h>
#include <SDL_log.h>

#include <algorithm>
#include <chrono>
#include <fstream>

using namespace chip8;
using namespace std::literals::string_literals;

namespace
{
	void load_program_to_mem(const std::filesystem::path& rom_path,
		std::array<std::byte, chip8::interpreter::c_mem_size>& mem)
	{
		// Sanity check
		if (!std::filesystem::exists(rom_path))
			throw std::runtime_error("File does not exist at "s + rom_path.string());

		if (!std::filesystem::is_regular_file(rom_path))
			throw std::runtime_error(rom_path.string() + " does not point to a regular file"s);

		// Load to mem
		auto reader = std::ifstream(rom_path, std::ios_base::in | std::ios_base::binary |
			std::ios_base::ate);
		if (!reader)
			throw std::runtime_error("Unable to open file "s + rom_path.string() + " for reading"s);

		constexpr auto max_rom_size = chip8::interpreter::c_mem_size
			- chip8::interpreter::c_code_start;
		const auto file_byte_count = reader.tellg();
		if (file_byte_count > max_rom_size)
		{
			throw std::runtime_error("Rom file is too large. Expected up to "s
				+ std::to_string(max_rom_size) + " got "s + std::to_string(file_byte_count));
		}

		reader.seekg(0);
		reader.read(reinterpret_cast<char*>(mem.data() + chip8::interpreter::c_code_start),
			file_byte_count);
	}

	inline auto calculate_tick_delta(std::chrono::time_point<std::chrono::high_resolution_clock>&
		tick_time) noexcept
	{
		const auto last_tick_time = tick_time;
		tick_time = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::nanoseconds>(tick_time - last_tick_time);
	}
}

interpreter::interpreter(const std::filesystem::path& rom_path, sdl::window& interpreter_window) :
	m_is_running{true},
	m_interpreter_window{interpreter_window},
	m_display{chip8::display{m_interpreter_window, 64, 32}}
{
	// Set up memory
	std::copy_n(chip8::font::raw_data.begin(), chip8::font::raw_data.size(), this->m_mem.begin());	
	load_program_to_mem(rom_path, this->m_mem);
}

void interpreter::run()
{
	bool test_tick = false;
	auto tick_time = std::chrono::high_resolution_clock::now();

	while (this->m_is_running)
	{
		const auto tick_delta = calculate_tick_delta(tick_time);

		this->process_events();

		// Generate test checker board pattern
		auto test_vec = std::vector<bool>(64*32, true);
		for (size_t y = 0; y < 32; ++y)
			for (size_t x = 0; x < 64; ++x)
				test_vec[y * 64 + x] = (x % 2 == 0) ^ (y % 2 == test_tick);

		m_display.draw(test_vec);
		test_tick = !test_tick;
		SDL_Log("tick");

		SDL_Delay(500);
	}
}

void interpreter::process_events()
{
	while (SDL_PollEvent(&this->m_evt))
	{
		switch (this->m_evt.type)
		{
			case SDL_QUIT:
				SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Quit event received");
				this->m_is_running = false;
				break;
		}
	}
}
