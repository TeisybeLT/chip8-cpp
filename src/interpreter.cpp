#include "interpreter.hpp"

#include "chip8_font.hpp"
#include "instructions.hpp"
#include "io/rom.hpp"
#include "errors/illegal_instruction_exception.hpp"

#include <SDL_timer.h>
#include <SDL_log.h>

#include <algorithm>
#include <bitset>
#include <chrono>
#include <cstring>
#include <functional>

using namespace chip8;

namespace
{
	[[nodiscard]] inline auto calculate_tick_delta(std::chrono::time_point<std::chrono::high_resolution_clock>&
		tick_time) noexcept
	{
		const auto last_tick_time = tick_time;
		tick_time = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::nanoseconds>(tick_time - last_tick_time);
	}

	[[nodiscard]] inline auto wrap(size_t val, size_t limit)
	{
		return (val >= limit) ? (val - limit) : val;
	};
}

interpreter::interpreter(const std::filesystem::path& rom_path, sdl::window& interpreter_window, sdl::beeper& beeper,
	std::chrono::nanoseconds tick_period) :
		m_is_running{true},
		m_interpreter_window{interpreter_window},
		m_display{m_interpreter_window, constants::ch8_width, constants::ch8_height},
		m_machine_tick_period{tick_period},
		m_registers{constants::code_start},
		m_video_mem(this->m_display.get_pixel_count())
{
	// Set up timers
	// Timer index 0 - delay
	// Timer index 1 - sound
	this->m_timers.emplace_back(this->m_registers.delay, constants::timer_tick_freq);
	this->m_timers.emplace_back(this->m_registers.sound, constants::timer_tick_freq,
		std::bind(&sdl::beeper::play, &beeper),
		std::bind(&sdl::beeper::pause, &beeper)
	);

	// Set up memory
	std::copy_n(chip8::font::raw_data.begin(), chip8::font::raw_data.size(), this->m_mem.begin());
	chip8::load_rom_from_file(rom_path, this->m_mem);
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

	auto throw_illegal_instruction = [&]
	{
		throw illegal_instruction{this->m_registers, instr};
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
					break;

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
			switch (instructions::get_lower_nibble<std::byte>(instr[1]))
			{
				case std::byte{0x00}: // LD Vx, Vy
					instructions::ld_reg_reg(this->m_registers, instr);
					break;

				case std::byte{0x01}: // OR Vx, Vy
					instructions::or_reg_reg(this->m_registers, instr);
					break;

				case std::byte{0x02}: // AND Vx, Vy
					instructions::and_reg_reg(this->m_registers, instr);
					break;

				case std::byte{0x03}: // XOR Vx, Vy
					instructions::xor_reg_reg(this->m_registers, instr);
					break;

				case std::byte{0x04}: // ADD Vx, Vy
					instructions::add_reg_reg(this->m_registers, instr);
					break;

				case std::byte{0x05}: // SUB Vx, Vy
					instructions::sub_reg_reg(this->m_registers, instr);
					break;

				case std::byte{0x06}: // SHR Vx, Vy
					instructions::shr_reg_reg(this->m_registers, instr);
					break;

				case std::byte{0x07}: // SUBN Vx, Vy
					instructions::subn_reg_reg(this->m_registers, instr);
					break;

				case std::byte{0x0E}: // SHL Vx, Vy
					instructions::shl_reg_reg(this->m_registers, instr);
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
			this->m_registers.v[0xF] = std::byte{0x00};
			const auto x_offset = std::to_integer<uint8_t>(this->m_registers.v[instructions::get_lower_nibble<size_t>(instr[0])]);
			auto y_offset = std::to_integer<uint8_t>(this->m_registers.v[instructions::get_upper_nibble<size_t>(instr[1])]);
			const auto n_end = this->m_registers.i + instructions::get_lower_nibble<uint16_t>(instr[1]);

			for (size_t n_idx = this->m_registers.i; n_idx < n_end; ++n_idx)
			{
				auto sprite_line = std::bitset<8>(std::to_integer<uint8_t>(this->m_mem.at(n_idx)));
				auto x_offset_line = x_offset;

				for (int bit_idx = sprite_line.size() - 1; bit_idx >= 0; --bit_idx)
				{
					const auto cur_idx = y_offset * this->m_display.get_width() + x_offset_line;
					const auto prev_bit = bool{this->m_video_mem.at(cur_idx)};
					const auto new_bit = bool{sprite_line[bit_idx]};

					this->m_video_mem[cur_idx] = prev_bit ^ new_bit;
					if (prev_bit && new_bit)
						this->m_registers.v[0xF] = std::byte{0x01};

					x_offset_line = wrap(x_offset_line + 1, this->m_display.get_width());
				}

				y_offset = wrap(y_offset + 1, this->m_display.get_height());
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

				case std::byte{0x29}: // Fx29 - LD F, Vx
					instructions::ld_f_reg(this->m_registers, instr);
					break;

				case std::byte{0x33}: // Fx33 - LD B, Vx
					instructions::ld_b_reg(this->m_registers, this->m_mem, instr);
					break;

				case std::byte{0x55}: // Fx55 - ld [i], vx
					instructions::str_i_reg(this->m_registers, this->m_mem, instr);
					break;

				case std::byte{0x65}: // Fx65 - ld vx, [i]
					instructions::str_reg_i(this->m_registers, this->m_mem, instr);
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
