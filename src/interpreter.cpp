#include "interpreter.hpp"

#include "chip8_font.hpp"
#include "instructions.hpp"

#include <SDL_timer.h>
#include <SDL_log.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <functional>
#include <fstream>
#include <iomanip>
#include <sstream>

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

	[[nodiscard]] inline auto calculate_tick_delta(std::chrono::time_point<std::chrono::high_resolution_clock>&
		tick_time) noexcept
	{
		const auto last_tick_time = tick_time;
		tick_time = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::nanoseconds>(tick_time - last_tick_time);
	}
}

interpreter::interpreter(const std::filesystem::path& rom_path, sdl::window& interpreter_window, sdl::beeper& beeper,
	std::chrono::nanoseconds tick_period) :
	m_is_running{true}, m_interpreter_window{interpreter_window}, m_display{m_interpreter_window, 64, 32},
	m_machine_tick_period{tick_period}, m_registers{c_code_start}, m_video_mem(64 * 32)
{
	// Set up timers
	// Timer index 0 - delay
	// Timer index 1 - sound
	this->m_timers.emplace_back(this->m_registers.delay, c_timer_tick_freq);
	this->m_timers.emplace_back(this->m_registers.sound, c_timer_tick_freq,
		std::bind(&sdl::beeper::play, &beeper),
		std::bind(&sdl::beeper::pause, &beeper)
	);

	// Set up memory
	std::copy_n(chip8::font::raw_data.begin(), chip8::font::raw_data.size(), this->m_mem.begin());	
	load_program_to_mem(rom_path, this->m_mem);
}

void interpreter::run()
{
	size_t test_count = 0;
	bool test_tick = false;
	auto tick_time = std::chrono::high_resolution_clock::now();
	auto machine_tick_count = 0ns;

	while (this->m_is_running)
	{
		const auto tick_delta = calculate_tick_delta(tick_time);

		// Process everything needed for interpreter
		this->process_events();
		this->process_timers(tick_delta);

		// Calculate and process machine tick
		machine_tick_count += tick_delta;
		if (machine_tick_count >= this->m_machine_tick_period)
		{
			machine_tick_count -= this->m_machine_tick_period;
			this->process_machine_tick();

			++test_count;
			if (test_count >= 250)
			{
				test_count = 0;
				// Generate test checker board pattern
				auto test_vec = std::vector<bool>(64*32, true);
				for (size_t y = 0; y < 32; ++y)
					for (size_t x = 0; x < 64; ++x)
						test_vec[y * 64 + x] = (x % 2 == 0) ^ (y % 2 == test_tick);

				m_display.draw(test_vec);
				test_tick = !test_tick;
				SDL_Log("tick");
			}
		}
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

void interpreter::process_timers(const std::chrono::nanoseconds& delta)
{
	for (auto& timer : this->m_timers)
		timer.update(delta);
}

void interpreter::process_machine_tick()
{
	const auto instr = instructions::fetch(this->m_mem, this->m_registers.pc);

	auto throw_illegal_instruction = [&instr]
	{
		auto str = std::ostringstream{"Illegal instruction: 0x"};
		str << std::hex << std::to_integer<int>(instr[1]) << std::to_integer<int>(instr[0]);
		throw std::runtime_error(str.str()); 
	};

	switch(instructions::extract_instruction_class(instr))
	{
		case std::byte{0x0}:
		{
			if (instr[0] == std::byte{0xE0}) // CLS
			{
				std::fill(this->m_video_mem.begin(), this->m_video_mem.end(), false);
				this->m_display.draw(this->m_video_mem);
			}
			else if (instr[0] == std::byte{0xEE}) // RET
			{
				instructions::ret(this->m_registers, this->m_stack);
				return;
			}
			else
				throw_illegal_instruction();

			break;
		}

		case std::byte{0x1}: // JP addr
			instructions::jp(this->m_registers, instr);
			return;

		case std::byte{0x2}: // CALL addr
			instructions::call(this->m_registers, this->m_stack, instr);
			return;

		case std::byte{0x3}: // SE Vx, byte
			instructions::se_reg_byte(this->m_registers, instr);
			break;

		case std::byte{0x4}: // SNE Vx, byte
			instructions::sne_reg_byte(this->m_registers, instr);
			break;

		case std::byte{0x5}: // SE Vx, Vy
			instructions::se_reg_reg(this->m_registers, instr);
			break;

		case std::byte{0x6}: // LD Vx, byte
//			this->m_registers.v[instruction::get_lower_nibble<size_t>(instruction[1])] = instruction[0];
			break;

		case std::byte{0x7}: // ADD Vx, byte

			break;
//		case std::byte{0x8}:
	}

	// If not returned before, PC was not changed by instruction, so increment it here
	++this->m_registers.pc;
}
