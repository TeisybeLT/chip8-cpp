#include "doctest.h"
#include "test_helpers.hpp"

#include "instructions.hpp"

using namespace chip8;

TEST_CASE("SE reg byte instruction")
{
	auto regs = registers(0);
	auto instr = instr_t{std::byte{0x00}, std::byte{0xFF}};

	SUBCASE("All regs not equal")
	{
		for (size_t reg_idx = 0; reg_idx < regs.v.size(); ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instructions::se_reg_byte(regs, instr);
			REQUIRE_EQ(regs.pc, 0);
		}
	}

	SUBCASE("All regs equal")
	{
		regs.v.fill(std::byte{0xFF});
		for (size_t reg_idx = 0; reg_idx < regs.v.size(); ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instructions::se_reg_byte(regs, instr);
			REQUIRE_EQ(regs.pc, reg_idx * 2 + 2);
		}
	}
}

TEST_CASE("SNE reg byte instruction")
{
	auto regs = registers(0);
	auto instr = instr_t{std::byte{0x00}, std::byte{0xFF}};

	SUBCASE("All regs not equal")
	{
		for (size_t reg_idx = 0; reg_idx < regs.v.size(); ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instructions::sne_reg_byte(regs, instr);
			REQUIRE_EQ(regs.pc, reg_idx * 2 + 2);
		}
	}

	SUBCASE("All regs equal")
	{
		regs.v.fill(std::byte{0xFF});
		for (size_t reg_idx = 0; reg_idx < regs.v.size(); ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instructions::sne_reg_byte(regs, instr);
			REQUIRE_EQ(regs.pc, 0);
		}
	}
}

TEST_CASE("SE reg reg instruction")
{
	auto regs = registers(0);
	auto instr = instr_t{std::byte{0x00}, std::byte{0xFF}};

	SUBCASE("All regs are same")
	{
		for (size_t reg_idx = 0; reg_idx < helpers::half_reg_count; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((reg_idx + helpers::half_reg_count) << 4);
			instructions::se_reg_reg(regs, instr);
			REQUIRE_EQ(regs.pc, reg_idx * 2 + 2);
		}
	}

	SUBCASE("Half regs different")
	{
		std::fill_n(regs.v.begin(), helpers::half_reg_count, std::byte{0xFF});
		for (size_t reg_idx = 0; reg_idx < helpers::half_reg_count; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((reg_idx + helpers::half_reg_count) << 4);
			instructions::se_reg_reg(regs, instr);
			REQUIRE_EQ(regs.pc, 0);
		}
	}
}

TEST_CASE("SNE reg reg instruction")
{
	auto regs = registers(0);
	auto instr = instr_t{std::byte{0x00}, std::byte{0xFF}};

	SUBCASE("All regs are same")
	{
		for (size_t reg_idx = 0; reg_idx < helpers::half_reg_count; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((reg_idx + helpers::half_reg_count) << 4);
			instructions::sne_reg_reg(regs, instr);
			CHECK_EQ(regs.pc, 0);
		}
	}

	SUBCASE("Half regs different")
	{
		std::fill_n(regs.v.begin(), helpers::half_reg_count, std::byte{0xFF});
		for (size_t reg_idx = 0; reg_idx < helpers::half_reg_count; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((reg_idx + helpers::half_reg_count) << 4);
			instructions::sne_reg_reg(regs, instr);
			CHECK_EQ(regs.pc, reg_idx * 2 + 2);
		}
	}
}
