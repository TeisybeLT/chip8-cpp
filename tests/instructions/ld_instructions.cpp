#include "doctest.h"
#include "test_helpers.hpp"

#include "instructions.hpp"

using namespace chip8;

TEST_CASE("LD reg byte instruction")
{
	static constexpr auto test_byte = std::byte{0x8F};

	auto regs = registers(0);
	auto instr = instr_t{std::byte{0x00}, test_byte};

	for (size_t reg_idx = 0; reg_idx < regs.v.size(); ++reg_idx)
	{
		instr[0] = std::byte(reg_idx);
		instructions::ld_reg_byte(regs, instr);
	}

	REQUIRE(std::all_of(regs.v.begin(), regs.v.end(), [](std::byte reg)
	{
		return reg == test_byte;
	}));
}

TEST_CASE("LD reg reg instruction")
{
	auto regs = registers(0);
	std::fill_n(regs.v.begin(), helpers::half_reg_count, std::byte{0xFF});
	auto instr = helpers::get_zero_instruction();

	for (size_t reg_idx = 0; reg_idx < helpers::half_reg_count; ++reg_idx)
	{
		instr[0] = std::byte(helpers::half_reg_count + reg_idx);
		instr[1] = std::byte(reg_idx << 4);
		instructions::ld_reg_reg(regs, instr);
	}

	REQUIRE(std::all_of(regs.v.begin(), regs.v.end(), [](std::byte reg)
	{
		return reg == std::byte{0xFF};
	}));
}

TEST_CASE("LD I addr instruction")
{
	auto regs = registers(0);
	auto instr = helpers::get_zero_instruction();

	SUBCASE("Load 0x001")
	{
		instr[1] = std::byte{0x01};
		instructions::ld_i_addr(regs, instr);
		REQUIRE_EQ(regs.i, uint16_t{0x01});
	}

	SUBCASE("Load 0xFFF")
	{
		instr.fill(std::byte{0xFF});
		instructions::ld_i_addr(regs, instr);
		REQUIRE_EQ(regs.i, uint16_t{0xFFF});
	}

	SUBCASE("Load 0x8F0")
	{
		instr[0] = std::byte{0x08};
		instr[1] = std::byte{0xF0};
		instructions::ld_i_addr(regs, instr);
		REQUIRE_EQ(regs.i, uint16_t{0x8F0});
	}
}

TEST_CASE("LD reg dt instruction")
{
	auto regs = registers(0);
	auto instr = helpers::get_zero_instruction();

	for (size_t reg_idx = 0; reg_idx < regs.v.size(); ++reg_idx)
	{
		instr[0] = std::byte(reg_idx);
		regs.delay = uint8_t(reg_idx);
		instructions::ld_reg_dt(regs, instr);
		CHECK_EQ(std::to_integer<size_t>(regs.v[reg_idx]), reg_idx);
	}
}

TEST_CASE("LD dt reg instruction")
{
	auto regs = registers(0);
	auto instr = helpers::get_zero_instruction();
	std::generate(regs.v.begin(), regs.v.end(), [cnt = size_t{0}]() mutable
	{
		return std::byte(cnt++);
	});

	for (size_t reg_idx = 0; reg_idx < regs.v.size(); ++reg_idx)
	{
		instr[0] = std::byte(reg_idx);
		instructions::ld_dt_reg(regs, instr);
		CHECK_EQ(regs.delay, reg_idx);
	}
}

TEST_CASE("LD st reg instruction")
{
	auto regs = registers(0);
	auto instr = helpers::get_zero_instruction();
	std::generate(regs.v.begin(), regs.v.end(), [cnt = size_t{0}]() mutable
	{
		return std::byte(cnt++);
	});

	for (size_t reg_idx = 0; reg_idx < regs.v.size(); ++reg_idx)
	{
		instr[0] = std::byte(reg_idx);
		instructions::ld_st_reg(regs, instr);
		CHECK_EQ(regs.sound, reg_idx);
	}
}

TEST_CASE("LD reg reg instruction")
{
	auto regs = registers(0);
	std::fill_n(regs.v.begin(), helpers::half_reg_count, std::byte{0xFF});
	auto instr = helpers::get_zero_instruction();

	for (size_t reg_idx = 0; reg_idx < helpers::half_reg_count; ++reg_idx)
	{
		instr[0] = std::byte(helpers::half_reg_count + reg_idx);
		instr[1] = std::byte(reg_idx << 4);
		instructions::ld_reg_reg(regs, instr);
	}

	REQUIRE(std::all_of(regs.v.begin(), regs.v.end(), [](std::byte reg)
	{
		return reg == std::byte{0xFF};
	}));
}

TEST_CASE("LD B reg instruction")
{
	auto regs = registers(0);
	auto instr = instr_t{};
	auto mem = std::array<std::byte, 6>{};
	mem.fill(std::byte{0xFF});

	SUBCASE("Single digit")
	{
		static constexpr auto default_i_reg = uint16_t{2};
		regs.i = default_i_reg;

		std::generate(regs.v.begin(), regs.v.end(), [cnt = size_t{0}]() mutable
		{
			return std::byte(cnt++);
		});

		for (size_t reg_idx = 0; reg_idx < 10; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instructions::ld_b_reg(regs, mem, instr);

			CHECK(std::all_of(mem.begin() + default_i_reg, mem.begin() + default_i_reg + 1,
				[](auto data) -> bool
				{
					return data == std::byte{0x00};
				}));

			CHECK_EQ(mem[default_i_reg + 2], std::byte(reg_idx));
		}
	}

	SUBCASE("Three digit")
	{
		static constexpr auto default_i_reg = uint16_t{3};
		regs.i = default_i_reg;
		regs.v.fill(std::byte(123));

		for (size_t reg_idx = 0; reg_idx < regs.v.size(); ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instructions::ld_b_reg(regs, mem, instr);

			CHECK_EQ(mem[default_i_reg], std::byte{1});
			CHECK_EQ(mem[default_i_reg + 1], std::byte{2});
			CHECK_EQ(mem[default_i_reg + 2], std::byte{3});
		}
	}
}

TEST_CASE("LD [I] reg instruction")
{
	auto regs = registers(0);
	auto instr = instr_t{std::byte{0x09}, std::byte{0x00}};
	auto mem = std::array<std::byte, 11>{};
	mem.fill(std::byte{0xFF});

	static constexpr auto test_regs = size_t{9};
	regs.i = 1;
	std::generate(regs.v.begin(), regs.v.begin() + test_regs, [cnt = size_t{0}]() mutable
	{
		return std::byte(cnt++);
	});

	instructions::str_i_reg(regs, mem, instr);
	REQUIRE_EQ(mem[0], std::byte{0xFF});
	for (size_t idx = 0; idx < test_regs; ++idx)
		CHECK_EQ(mem[idx + 1], std::byte(idx));
}

TEST_CASE("LD reg [I] instruction")
{
	auto regs = registers(0);
	auto instr = instr_t{std::byte{0x09}, std::byte{0x00}};
	auto mem = std::array<std::byte, 11>{};
	mem.fill(std::byte{0xFF});
	regs.v.fill(std::byte{0xFF});

	static constexpr auto test_regs = size_t{9};
	regs.i = 1;
	std::generate(mem.begin() + 1, mem.begin() + test_regs + 1, [cnt = size_t{0}]() mutable
	{
		return std::byte(cnt++);
	});

	instructions::str_reg_i(regs, mem, instr);
	REQUIRE_EQ(mem[0], std::byte{0xFF});
	for (size_t idx = 0; idx < test_regs; ++idx)
		CHECK_EQ(regs.v[idx], std::byte(idx));

	for (auto idx = test_regs; idx < regs.v.size(); ++idx)
		CHECK_EQ(regs.v[idx], std::byte{0xFF});
}
