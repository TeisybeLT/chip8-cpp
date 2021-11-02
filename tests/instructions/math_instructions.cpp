#include "doctest.h"
#include "test_helpers.hpp"

#include "instructions.hpp"

using namespace chip8;

TEST_CASE("ADD reg byte instruction")
{
	auto regs = registers(0);
	regs.v.fill(std::byte{0x03});
	auto instr = instructions::instruction{};

	SUBCASE("Regular addition")
	{
		instr[1] = std::byte{0x02};
		for (size_t reg_idx = 0; reg_idx < regs.v.size(); ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instructions::add_reg_byte(regs, instr);
		}

		REQUIRE(std::all_of(regs.v.begin(), regs.v.end(), [](std::byte reg)
		{
			return reg == std::byte{0x05};
		}));
	}

	SUBCASE("Overflow addition")
	{
		instr[1] = std::byte{0xFF};
		for (size_t reg_idx = 0; reg_idx < regs.v.size(); ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instructions::add_reg_byte(regs, instr);
		}

		REQUIRE(std::all_of(regs.v.begin(), regs.v.end(), [](std::byte reg)
		{
			return reg == std::byte{0x02};
		}));
	}
}

TEST_CASE("OR reg reg instruction")
{
	auto regs = registers(0);
	std::fill_n(regs.v.begin(), helpers::half_reg_count, std::byte{0x0F});
	std::fill_n(regs.v.begin() + helpers::half_reg_count, helpers::half_reg_count, std::byte{0xF0});
	auto instr = helpers::get_zero_instruction();

	for (size_t reg_idx = 0; reg_idx < helpers::half_reg_count; ++reg_idx)
	{
		instr[0] = std::byte(reg_idx);
		instr[1] = std::byte((helpers::half_reg_count + reg_idx) << 4);
		instructions::or_reg_reg(regs, instr);
	}

	REQUIRE(std::all_of(regs.v.begin(), regs.v.begin() + helpers::half_reg_count, [](std::byte reg)
	{
		return reg == std::byte{0xFF};
	}));

	REQUIRE(std::all_of(regs.v.begin() + helpers::half_reg_count, regs.v.end(), [](std::byte reg)
	{
		return reg == std::byte{0xF0};
	}));
}

TEST_CASE("AND reg reg instruction")
{
	auto regs = registers(0);
	std::fill_n(regs.v.begin(), helpers::half_reg_count, std::byte{0xFF});
	std::fill_n(regs.v.begin() + helpers::half_reg_count, helpers::half_reg_count, std::byte{0xF0});
	auto instr = helpers::get_zero_instruction();

	for (size_t reg_idx = 0; reg_idx < helpers::half_reg_count; ++reg_idx)
	{
		instr[0] = std::byte(reg_idx);
		instr[1] = std::byte((helpers::half_reg_count + reg_idx) << 4);
		instructions::and_reg_reg(regs, instr);
	}

	REQUIRE(std::all_of(regs.v.begin(), regs.v.end(), [](std::byte reg)
	{
		return reg == std::byte{0xF0};
	}));
}

TEST_CASE("XOR reg reg instruction")
{
	auto regs = registers(0);
	std::fill_n(regs.v.begin(), helpers::half_reg_count, std::byte{0xFF});
	std::fill_n(regs.v.begin() + helpers::half_reg_count, helpers::half_reg_count, std::byte{0xFA});
	auto instr = helpers::get_zero_instruction();

	for (size_t reg_idx = 0; reg_idx < helpers::half_reg_count; ++reg_idx)
	{
		instr[0] = std::byte(reg_idx);
		instr[1] = std::byte((helpers::half_reg_count + reg_idx) << 4);
		instructions::xor_reg_reg(regs, instr);
	}

	REQUIRE(std::all_of(regs.v.begin(), regs.v.begin() + helpers::half_reg_count, [](std::byte reg)
	{
		return reg == std::byte{0x05};
	}));

	REQUIRE(std::all_of(regs.v.begin() + helpers::half_reg_count, regs.v.end(), [](std::byte reg)
	{
		return reg == std::byte{0xFA};
	}));

}

TEST_CASE("ADD reg reg instruction")
{
	auto regs = registers(0);
	std::fill_n(regs.v.begin(), helpers::half_reg_count, std::byte{0x09});
	auto instr = helpers::get_zero_instruction();

	SUBCASE("Non-overflowing")
	{
		std::fill_n(regs.v.begin() + helpers::half_reg_count, helpers::half_reg_count - 1, std::byte{0x01});
		for (size_t reg_idx = 0; reg_idx < helpers::half_reg_count - 1; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((helpers::half_reg_count + reg_idx) << 4);
			instructions::add_reg_reg(regs, instr);

			CHECK_EQ(regs.v[0xF], std::byte{0x00});
			CHECK_EQ(regs.v[reg_idx], std::byte{0x0A});
			CHECK_EQ(regs.v[helpers::half_reg_count + reg_idx], std::byte{0x01});
		}
		CHECK_EQ(regs.v[helpers::half_reg_count - 1], std::byte{0x09});
	}

	SUBCASE("Overflowing")
	{
		std::fill_n(regs.v.begin() + helpers::half_reg_count, helpers::half_reg_count - 1, std::byte{0xFF});
		for (size_t reg_idx = 0; reg_idx < helpers::half_reg_count - 1; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((helpers::half_reg_count + reg_idx) << 4);
			instructions::add_reg_reg(regs, instr);

			CHECK_EQ(regs.v[0xF], std::byte{0x01});
			CHECK_EQ(regs.v[reg_idx], std::byte{0x08});
			CHECK_EQ(regs.v[helpers::half_reg_count + reg_idx], std::byte{0xFF});
		}
		CHECK_EQ(regs.v[helpers::half_reg_count - 1], std::byte{0x09});
	}
}

TEST_CASE("SUB reg reg instruction")
{
	auto regs = registers(0);
	std::fill_n(regs.v.begin(), helpers::half_reg_count, std::byte{0x09});
	auto instr = helpers::get_zero_instruction();

	SUBCASE("Non-borrowing")
	{
		std::fill_n(regs.v.begin() + helpers::half_reg_count, helpers::half_reg_count - 1, std::byte{0x01});
		for (size_t reg_idx = 0; reg_idx < helpers::half_reg_count - 1; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((helpers::half_reg_count + reg_idx) << 4);
			instructions::sub_reg_reg(regs, instr);

			CHECK_EQ(regs.v[0xF], std::byte{0x01});
			CHECK_EQ(regs.v[reg_idx], std::byte{0x08});
			CHECK_EQ(regs.v[helpers::half_reg_count + reg_idx], std::byte{0x01});
		}
		CHECK_EQ(regs.v[helpers::half_reg_count - 1], std::byte{0x09});
	}

	SUBCASE("Borrowing")
	{
		std::fill_n(regs.v.begin() + helpers::half_reg_count, helpers::half_reg_count - 1, std::byte{0x0A});
		for (size_t reg_idx = 0; reg_idx < helpers::half_reg_count - 1; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((helpers::half_reg_count + reg_idx) << 4);
			instructions::sub_reg_reg(regs, instr);

			CHECK_EQ(regs.v[0xF], std::byte{0x00});
			CHECK_EQ(regs.v[reg_idx], std::byte{0xFF});
			CHECK_EQ(regs.v[helpers::half_reg_count + reg_idx], std::byte{0x0A});
		}
		CHECK_EQ(regs.v[helpers::half_reg_count - 1], std::byte{0x09});
	}
}

TEST_CASE("SHR reg reg instruction")
{
	auto regs = registers(0);
	auto instr = helpers::get_zero_instruction();

	SUBCASE("LSB is 0")
	{
		std::fill_n(regs.v.begin(), regs.v.size() - 1, std::byte{0x02});
		for (size_t reg_idx = 0; reg_idx < regs.v.size() - 1; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instructions::shr_reg_reg(regs, instr);
			CHECK_EQ(regs.v[0xF], std::byte{0x00});
		}

		REQUIRE(std::all_of(regs.v.begin(), regs.v.end() - 1, [](std::byte reg)
		{
			return reg == std::byte{0x01};
		}));
	}

	SUBCASE("LSB is 1")
	{
		std::fill_n(regs.v.begin(), regs.v.size() - 1, std::byte{0x05});
		for (size_t reg_idx = 0; reg_idx < regs.v.size() - 1; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instructions::shr_reg_reg(regs, instr);
			CHECK_EQ(regs.v[0xF], std::byte{0x01});
		}

		REQUIRE(std::all_of(regs.v.begin(), regs.v.end() - 1, [](std::byte reg)
		{
			return reg == std::byte{0x02};
		}));
	}
}

TEST_CASE("SUBN reg reg instruction")
{
	auto regs = registers(0);
	auto instr = helpers::get_zero_instruction();

	SUBCASE("Non-borrowing")
	{
		std::fill_n(regs.v.begin(), helpers::half_reg_count, std::byte{0x01});
		std::fill_n(regs.v.begin() + helpers::half_reg_count, helpers::half_reg_count - 1, std::byte{0x09});
		for (size_t reg_idx = 0; reg_idx < helpers::half_reg_count - 1; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((helpers::half_reg_count + reg_idx) << 4);
			instructions::subn_reg_reg(regs, instr);

			CHECK_EQ(regs.v[0xF], std::byte{0x01});
			CHECK_EQ(regs.v[reg_idx], std::byte{0x08});
			CHECK_EQ(regs.v[helpers::half_reg_count + reg_idx], std::byte{0x09});
		}
		CHECK_EQ(regs.v[helpers::half_reg_count - 1], std::byte{0x01});
	}

	SUBCASE("Borrowing")
	{
		std::fill_n(regs.v.begin(), helpers::half_reg_count, std::byte{0x0A});
		std::fill_n(regs.v.begin() + helpers::half_reg_count, helpers::half_reg_count - 1, std::byte{0x09});
		for (size_t reg_idx = 0; reg_idx < helpers::half_reg_count - 1; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((helpers::half_reg_count + reg_idx) << 4);
			instructions::subn_reg_reg(regs, instr);

			CHECK_EQ(regs.v[0xF], std::byte{0x00});
			CHECK_EQ(regs.v[reg_idx], std::byte{0xFF});
			CHECK_EQ(regs.v[helpers::half_reg_count + reg_idx], std::byte{0x09});
		}
		CHECK_EQ(regs.v[helpers::half_reg_count - 1], std::byte{0x0A});
	}
}

TEST_CASE("SHL reg reg instruction")
{
	auto regs = registers(0);
	auto instr = helpers::get_zero_instruction();

	SUBCASE("MSB is 0")
	{
		std::fill_n(regs.v.begin(), regs.v.size() - 1, std::byte{0x01});
		for (size_t reg_idx = 0; reg_idx < regs.v.size() - 1; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instructions::shl_reg_reg(regs, instr);
			CHECK_EQ(regs.v[0xF], std::byte{0x00});
		}

		REQUIRE(std::all_of(regs.v.begin(), regs.v.end() - 1, [](std::byte reg)
		{
			return reg == std::byte{0x02};
		}));
	}

	SUBCASE("MSB is 1")
	{
		std::fill_n(regs.v.begin(), regs.v.size() - 1, std::byte{0x82});
		for (size_t reg_idx = 0; reg_idx < regs.v.size() - 1; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instructions::shl_reg_reg(regs, instr);
			CHECK_EQ(regs.v[0xF], std::byte{0x01});
		}

		REQUIRE(std::all_of(regs.v.begin(), regs.v.end() - 1, [](std::byte reg)
		{
			return reg == std::byte{0x04};
		}));
	}
}

TEST_CASE("ADD i reg instruction")
{
	auto regs = registers(0);
	auto instr = instructions::instruction{};

	SUBCASE("Regular addition")
	{
		regs.i = 10;
		regs.v.fill(std::byte{0x03});

		for (size_t reg_idx = 0; reg_idx < regs.v.size(); ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instructions::add_i_reg(regs, instr);
		}

		REQUIRE_EQ(regs.i, 3 * regs.v.size() + 10);
	}

	SUBCASE("Overflow addition")
	{
		regs.i = std::numeric_limits<decltype(regs.i)>::max() - 5;
		regs.v[0] = std::byte{0x07};
		instructions::add_i_reg(regs, instr);
		REQUIRE_EQ(regs.i, uint16_t{1});
	}
}

TEST_CASE("ADD F reg instruction")
{
	auto regs = registers(0);
	auto instr = instructions::instruction{};
	std::generate(regs.v.begin(), regs.v.end(), [cnt = size_t{0}]() mutable
	{
		return std::byte(cnt++);
	});

	for (uint8_t digit = 0; digit < 0x0F; ++digit)
	{
		instr[0] = std::byte{digit};
		instructions::ld_f_reg(regs, instr);
		CHECK_EQ(regs.i, digit * 5);
	}
}
