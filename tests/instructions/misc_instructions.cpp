#include "doctest.h"
#include "test_helpers.hpp"

#include "instructions.hpp"

#include <limits>

using namespace chip8;

TEST_CASE("RND Vx, byte")
{
	auto regs = registers(0);
	auto instr = helpers::get_zero_instruction();

	SUBCASE("Lower nibble mask")
	{
		instr[1] = std::byte{0x0F};
		for (size_t reg_idx = 0; reg_idx < regs.v.size(); ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instructions::rnd_reg_byte(regs, instr);
		}

		REQUIRE(std::all_of(regs.v.begin(), regs.v.end() - 1, [](std::byte reg)
		{
			return (reg & std::byte{0xF0}) == std::byte{0x00};
		}));
	}

	SUBCASE("Lower upper mask")
	{
		instr[1] = std::byte{0xF0};
		for (size_t reg_idx = 0; reg_idx < regs.v.size(); ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instructions::rnd_reg_byte(regs, instr);
		}

		REQUIRE(std::all_of(regs.v.begin(), regs.v.end() - 1, [](std::byte reg)
		{
			return (reg & std::byte{0x0F}) == std::byte{0x00};
		}));
	}
}
