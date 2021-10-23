#include "interpreter.hpp"

#include "chip8_font.hpp"
#include "instructions.hpp"

#include <SDL_timer.h>
#include <SDL_log.h>

#include <algorithm>
#include <bitset>
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
		auto str = std::ostringstream{"Illegal instruction: 0x", std::ios_base::ate};
		str << std::hex << std::to_integer<int>(instr[0]) << std::to_integer<int>(instr[1]);
		throw std::runtime_error(str.str()); 
	};

	switch(instructions::extract_instruction_class(instr))
	{
		case std::byte{0x0}: // Instruction starting with 0 are further split by their second byte
		{
			switch (instr[1])
			{
				case std::byte{0xE0}: // CLS
					std::fill(this->m_video_mem.begin(), this->m_video_mem.end(), false);
					this->m_display.draw(this->m_video_mem);
					break;

				case std::byte{0xEE}: // RET
					instructions::ret(this->m_registers, this->m_stack);
					return;

				default:
					throw_illegal_instruction();
			}

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
			instructions::ld_reg_byte(this->m_registers, instr);
			break;

		case std::byte{0x7}: // ADD Vx, byte
			instructions::add_reg_byte(this->m_registers, instr);
			break;

		case std::byte{0x8}: // Instructions starting with 0x8 are further split by their lowest nibble
		{
			switch (instructions::get_lower_nibble<uint8_t>(instr[1]))
			{
				case uint8_t{0x00}: // LD Vx, Vy
					instructions::ld_reg_reg(this->m_registers, instr);
					break;

				case uint8_t{0x01}: // OR Vx, Vy
					instructions::or_reg_reg(this->m_registers, instr);
					break;

				case uint8_t{0x02}: // AND Vx, Vy
					instructions::and_reg_reg(this->m_registers, instr);
					break;

				case uint8_t{0x03}: // XOR Vx, Vy
					instructions::xor_reg_reg(this->m_registers, instr);
					break;

				case uint8_t{0x04}: // ADD Vx, Vy
					instructions::add_reg_reg(this->m_registers, instr);
					break;

				case uint8_t{0x05}: // SUB Vx, Vy
					instructions::sub_reg_reg(this->m_registers, instr);
					break;

				case uint8_t{0x06}: // SHR Vx, Vy
					instructions::shr_reg_reg(this->m_registers, instr);
					break;

				case uint8_t{0x07}: // SUBN Vx, Vy
					instructions::subn_reg_reg(this->m_registers, instr);
					break;

				case uint8_t{0x0E}: // SHL Vx, Vy
					instructions::subn_reg_reg(this->m_registers, instr);
					break;

				default:
					throw_illegal_instruction();
			}

			break;
		}

		case std::byte{0x9}: // SNE Vx, Vy
			instructions::sne_reg_reg(this->m_registers, instr);
			break;

		case std::byte{0xA}: // LD I, addr
			instructions::ld_i_addr(this->m_registers, instr);
			break;

		case std::byte{0xB}: // JP V0, addr
			instructions::jp_v0_addr(this->m_registers, instr);
			break;

		case std::byte{0xC}: // RND Vx, byte
			instructions::rnd_reg_byte(this->m_registers, instr);
			break;

		case std::byte{0xD}: // DRW Vx, Vy, nibble
		{
			auto wrap = [](size_t val, size_t limit)
			{
				return (val >= limit) ? (val - limit) : val;
			};

			this->m_registers.v[0xF] = std::byte{0x00};
			const auto x_offset = std::to_integer<uint8_t>(this->m_registers.v[instructions::get_lower_nibble<size_t>(instr[0])]);
			auto y_offset = std::to_integer<uint8_t>(this->m_registers.v[instructions::get_upper_nibble<size_t>(instr[1])]);
			const auto n_end = this->m_registers.i + instructions::get_lower_nibble<uint16_t>(instr[1]);

			for (size_t n_idx = this->m_registers.i; n_idx < n_end; ++n_idx)
			{
				auto sprite_line = std::bitset<8>(std::to_integer<uint8_t>(this->m_mem[n_idx]));
				auto x_offset_line = x_offset;

				for (int bit_idx = sprite_line.size() - 1; bit_idx >= 0; --bit_idx)
				{
					const auto cur_idx = y_offset * 64 + x_offset_line;
					const auto prev_bit = bool{this->m_video_mem[cur_idx]};
					const auto new_bit = bool{sprite_line[bit_idx]};

					this->m_video_mem[cur_idx] = prev_bit ^ new_bit;
					if (prev_bit && new_bit)
						this->m_registers.v[0xF] = std::byte{0x01};

					x_offset_line = wrap(x_offset_line + 1, 64);
				}

				y_offset = wrap(y_offset + 1, 32);
			}

			this->m_display.draw(this->m_video_mem);
			break;
		}

		case std::byte{0xE}: // Instructions starting with 0xE are further split by their lowest byte
		{
			switch (instr[1])
			{
				case std::byte{0x9E}: // Ex9E - SKP Vx
					instructions::skp_reg(this->m_registers, instr);
					break;

				case std::byte{0xA1}: // ExA1 - SKNP Vx
					instructions::sknp_reg(this->m_registers, instr);
					break;

				default:
					throw_illegal_instruction();
			}

			break;
		}

		case std::byte{0xF}: // Instructions starting with 0xF are further split by their lowest byte
		{
			switch (instr[1])
			{
				case std::byte{0x07}: // Fx07 - LD Vx, DT
					instructions::ld_reg_dt(this->m_registers, instr);
					break;

				case std::byte{0x0A}: // LD Vx, K
					if (!instructions::ld_reg_k(this->m_registers, instr))
						return;

					break;

				case std::byte{0x15}: // Fx15 - LD DT, Vx
					instructions::ld_dt_reg(this->m_registers, instr);
					this->m_timers[0].report_change();
					break;

				case std::byte{0x18}: // Fx18 - LD ST, Vx
					instructions::ld_st_reg(this->m_registers, instr);
					this->m_timers[1].report_change();
					break;

				case std::byte{0x1E}: // Fx1E - ADD I, Vx
					instructions::add_i_reg(this->m_registers, instr);
					break;

				default:
					throw_illegal_instruction();
			}

			break;
		}

		default:
			throw_illegal_instruction();
	}

	// If not returned before, PC was not changed by instruction, so increment it here
	this->m_registers.pc += 2;
}
