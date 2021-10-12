#include "instructions.hpp"

#include <random>

using namespace chip8;

void instructions::rnd_reg_byte(chip8::registers& regs, instructions::instruction instr) noexcept
{
	const auto x_offset = instructions::get_lower_nibble<size_t>(instr[0]);

	auto engine = std::default_random_engine{std::random_device{}()};
	auto random_number = std::uniform_int_distribution<uint8_t>(0, 255)(engine);

	regs.v[x_offset] = std::byte(random_number) & instr[1];
}
