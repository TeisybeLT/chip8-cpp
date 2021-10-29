#include "instructions.hpp"
#include "input.hpp"

#include <SDL_log.h>

#include <random>

using namespace chip8;

namespace
{
	inline bool is_key_pressed(const registers& regs, instructions::instruction instr) noexcept
	{
		const auto x_offset = instructions::get_lower_nibble<size_t>(instr[0]);
		const auto key = std::to_integer<size_t>(regs.v[x_offset]);
		return chip8::get_keyboard_state()[key];
	}
}

void instructions::rnd_reg_byte(chip8::registers& regs, instructions::instruction instr) noexcept
{
	const auto x_offset = instructions::get_lower_nibble<size_t>(instr[0]);

	auto engine = std::default_random_engine{std::random_device{}()};
	auto random_number = std::uniform_int_distribution<uint8_t>(0, 255)(engine);

	regs.v[x_offset] = std::byte(random_number) & instr[1];
}

void instructions::skp_reg(chip8::registers& regs, instructions::instruction instr) noexcept
{
	if (is_key_pressed(regs, instr))
		regs.pc += 2;
}

void instructions::sknp_reg(chip8::registers& regs, instructions::instruction instr) noexcept
{
	if (!is_key_pressed(regs, instr))
		regs.pc += 2;
}

bool instructions::ld_reg_k(chip8::registers& regs, instructions::instruction instr) noexcept
{
	const auto kbd_state = chip8::get_keyboard_state();
	if (!kbd_state.count())
		return false;

	auto pressed_idx = std::byte{0};
	for (size_t idx = 0; idx < kbd_state.size(); ++idx)
	{
		if (kbd_state[idx])
		{
			pressed_idx = std::byte(idx);
			break;
		}
	}

	regs.v[instructions::get_lower_nibble<size_t>(instr[0])] = pressed_idx;
	return true;
}
