#include "doctest.h"
#include "instructions.hpp"

#include <limits>

using namespace chip8;

namespace
{
	static constexpr auto half_reg_count = std::tuple_size<decltype(std::declval<registers>().v)>::value / 2;

	inline auto get_zero_instruction() noexcept
	{
		return instructions::instruction{std::byte{0x00}, std::byte{0x00}};
	}
}

TEST_CASE_TEMPLATE("Extracting lower 12 bits", T, uint16_t, int32_t, uint32_t, int32_t, size_t)
{
	instructions::instruction test_instr;

	SUBCASE("All bits")
	{
		test_instr.fill(std::byte{0xFF});
		REQUIRE_EQ(instructions::detail::get_lower_12_bits<T>(test_instr), T{0x0FFF});
	}

	SUBCASE("Lower byte")
	{
		test_instr[1] = std::byte{0xAF};
		test_instr[0] = std::byte{0xF0};
		REQUIRE_EQ(instructions::detail::get_lower_12_bits<T>(test_instr), T{0x00AF});
	}

	SUBCASE("Upper byte lower nibble")
	{
		test_instr[1] = std::byte{0x00};
		test_instr[0] = std::byte{0x0F};
		REQUIRE_EQ(instructions::detail::get_lower_12_bits<T>(test_instr), T{0x0F00});
	}
}

TEST_CASE_TEMPLATE("Extracting lower nibble", T, uint16_t, int32_t, uint32_t, int32_t, size_t)
{
	for (uint8_t cnt = 0; cnt < 0xFF; ++cnt)
		CHECK_EQ(instructions::get_lower_nibble<T>(std::byte{cnt}), T(cnt & 0x0F));
}

TEST_CASE_TEMPLATE("Extracting upper nibble", T, uint16_t, int32_t, uint32_t, int32_t, size_t)
{
	for (uint8_t cnt = 0; cnt < 0xFF; ++cnt)
		CHECK_EQ(instructions::get_upper_nibble<T>(std::byte{cnt}), T((cnt & 0xF0) >> 4));
}

TEST_CASE("Fetch instruction")
{
	static constexpr auto mem_test_size = size_t{16};
	auto mem = std::array<std::byte, mem_test_size>();
	mem.fill(std::byte{0x00});

	SUBCASE("At zero address")
	{
		mem[0] = std::byte{0xFF};
		mem[1] = std::byte{0xCE};
		const auto instr = instructions::fetch(mem, 0);

		REQUIRE_EQ(instr[1], std::byte{0xCE});
		REQUIRE_EQ(instr[0], std::byte{0xFF});
	}

	SUBCASE("At odd address")
	{
		mem[1] = std::byte{0xDF};
		mem[2] = std::byte{0xCA};
		const auto instr = instructions::fetch(mem, 1);

		REQUIRE_EQ(instr[0], std::byte{0xDF});
		REQUIRE_EQ(instr[1], std::byte{0xCA});
	}

	SUBCASE("At further address")
	{
		mem[10] = std::byte{0xFF};
		mem[11] = std::byte{0xCD};
		const auto instr = instructions::fetch(mem, 10);

		REQUIRE_EQ(instr[0], std::byte{0xFF});
		REQUIRE_EQ(instr[1], std::byte{0xCD});
	}

	SUBCASE("Out of bounds exception")
	{
		REQUIRE_THROWS(static_cast<void>(instructions::fetch(mem, 15)));
		REQUIRE_THROWS(static_cast<void>(instructions::fetch(mem, 16)));
	}
}

TEST_CASE("RET instruction")
{
	for (size_t cnt = 0; cnt < interpreter::c_stack_size; ++cnt)
	{
		auto stack = std::array<uint16_t, interpreter::c_stack_size>();
		auto regs = registers(0);

		regs.sp = uint16_t(cnt);
		stack[cnt] = uint16_t{1337};

		instructions::ret(regs, stack);

		CHECK_EQ(regs.sp, int16_t(cnt) - 1);
		CHECK_EQ(regs.pc, uint16_t{1337});
	}
}

TEST_CASE("JP instruction")
{
	auto regs = registers(0);
	auto instr = get_zero_instruction();

	SUBCASE("Jump to 0x001")
	{
		instr[1] = std::byte{0x01};
		instructions::jp(regs, instr);
		REQUIRE_EQ(regs.pc, uint16_t{0x01});
	}

	SUBCASE("Jump to 0xFFF")
	{
		instr.fill(std::byte{0xFF});
		instructions::jp(regs, instr);
		REQUIRE_EQ(regs.pc, uint16_t{0xFFF});
	}

	SUBCASE("Jump to 0x8F0")
	{
		instr[0] = std::byte{0x08};
		instr[1] = std::byte{0xF0};
		instructions::jp(regs, instr);
		REQUIRE_EQ(regs.pc, uint16_t{0x8F0});
	}
}

TEST_CASE("Nested CALL instruction")
{
	auto stack = std::array<uint16_t, interpreter::c_stack_size>();
	auto regs = registers(1337);
	auto instr = get_zero_instruction();

	// First call
	instr[0] = std::byte{0x0A};
	instr[1] = std::byte{0xBF};
	instructions::call(regs, stack, instr);
	REQUIRE_EQ(regs.sp, 0);
	REQUIRE_EQ(stack[0], 1337);
	REQUIRE_EQ(regs.pc, 0xABF);

	// Second call
	instr[0] = std::byte{0x08};
	instr[1] = std::byte{0xAA};
	instructions::call(regs, stack, instr);
	REQUIRE_EQ(regs.sp, 1);
	REQUIRE_EQ(stack[1], 0xABF);
	REQUIRE_EQ(regs.pc, 0x8AA);

	// Third call
	instr[0] = std::byte{0x01};
	instr[1] = std::byte{0x23};
	instructions::call(regs, stack, instr);
	REQUIRE_EQ(regs.sp, 2);
	REQUIRE_EQ(stack[2], 0x8AA);
	REQUIRE_EQ(regs.pc, 0x123);
}

TEST_CASE("SE reg byte instruction")
{
	auto regs = registers(0);
	auto instr = instructions::instruction{std::byte{0x00}, std::byte{0xFF}};

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
	auto instr = instructions::instruction{std::byte{0x00}, std::byte{0xFF}};

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
	auto instr = instructions::instruction{std::byte{0x00}, std::byte{0xFF}};

	SUBCASE("All regs are same")
	{
		for (size_t reg_idx = 0; reg_idx < half_reg_count; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((reg_idx + half_reg_count) << 4);
			instructions::se_reg_reg(regs, instr);
			REQUIRE_EQ(regs.pc, reg_idx * 2 + 2);
		}
	}

	SUBCASE("Half regs different")
	{
		std::fill_n(regs.v.begin(), half_reg_count, std::byte{0xFF});
		for (size_t reg_idx = 0; reg_idx < half_reg_count; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((reg_idx + half_reg_count) << 4);
			instructions::se_reg_reg(regs, instr);
			REQUIRE_EQ(regs.pc, 0);
		}
	}
}

TEST_CASE("LD reg byte instruction")
{
	static constexpr auto test_byte = std::byte{0x8F};

	auto regs = registers(0);
	auto instr = instructions::instruction{std::byte{0x00}, test_byte};

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

TEST_CASE("LD reg reg instruction")
{
	auto regs = registers(0);
	std::fill_n(regs.v.begin(), half_reg_count, std::byte{0xFF});
	auto instr = get_zero_instruction();

	for (size_t reg_idx = 0; reg_idx < half_reg_count; ++reg_idx)
	{
		instr[0] = std::byte(half_reg_count + reg_idx);
		instr[1] = std::byte(reg_idx << 4);
		instructions::ld_reg_reg(regs, instr);
	}

	REQUIRE(std::all_of(regs.v.begin(), regs.v.end(), [](std::byte reg)
	{
		return reg == std::byte{0xFF};
	}));
}

TEST_CASE("OR reg reg instruction")
{
	auto regs = registers(0);
	std::fill_n(regs.v.begin(), half_reg_count, std::byte{0x0F});
	std::fill_n(regs.v.begin() + half_reg_count, half_reg_count, std::byte{0xF0});
	auto instr = get_zero_instruction();

	for (size_t reg_idx = 0; reg_idx < half_reg_count; ++reg_idx)
	{
		instr[0] = std::byte(reg_idx);
		instr[1] = std::byte((half_reg_count + reg_idx) << 4);
		instructions::or_reg_reg(regs, instr);
	}

	REQUIRE(std::all_of(regs.v.begin(), regs.v.begin() + half_reg_count, [](std::byte reg)
	{
		return reg == std::byte{0xFF};
	}));

	REQUIRE(std::all_of(regs.v.begin() + half_reg_count, regs.v.end(), [](std::byte reg)
	{
		return reg == std::byte{0xF0};
	}));
}

TEST_CASE("AND reg reg instruction")
{
	auto regs = registers(0);
	std::fill_n(regs.v.begin(), half_reg_count, std::byte{0xFF});
	std::fill_n(regs.v.begin() + half_reg_count, half_reg_count, std::byte{0xF0});
	auto instr = get_zero_instruction();

	for (size_t reg_idx = 0; reg_idx < half_reg_count; ++reg_idx)
	{
		instr[0] = std::byte(reg_idx);
		instr[1] = std::byte((half_reg_count + reg_idx) << 4);
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
	std::fill_n(regs.v.begin(), half_reg_count, std::byte{0xFF});
	std::fill_n(regs.v.begin() + half_reg_count, half_reg_count, std::byte{0xFA});
	auto instr = get_zero_instruction();

	for (size_t reg_idx = 0; reg_idx < half_reg_count; ++reg_idx)
	{
		instr[0] = std::byte(reg_idx);
		instr[1] = std::byte((half_reg_count + reg_idx) << 4);
		instructions::xor_reg_reg(regs, instr);
	}

	REQUIRE(std::all_of(regs.v.begin(), regs.v.begin() + half_reg_count, [](std::byte reg)
	{
		return reg == std::byte{0x05};
	}));

	REQUIRE(std::all_of(regs.v.begin() + half_reg_count, regs.v.end(), [](std::byte reg)
	{
		return reg == std::byte{0xFA};
	}));

}

TEST_CASE("ADD reg reg instruction")
{
	auto regs = registers(0);
	std::fill_n(regs.v.begin(), half_reg_count, std::byte{0x09});
	auto instr = get_zero_instruction();

	SUBCASE("Non-overflowing")
	{
		std::fill_n(regs.v.begin() + half_reg_count, half_reg_count - 1, std::byte{0x01});
		for (size_t reg_idx = 0; reg_idx < half_reg_count - 1; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((half_reg_count + reg_idx) << 4);
			instructions::add_reg_reg(regs, instr);

			CHECK_EQ(regs.v[0xF], std::byte{0x00});
			CHECK_EQ(regs.v[reg_idx], std::byte{0x0A});
			CHECK_EQ(regs.v[half_reg_count + reg_idx], std::byte{0x01});
		}
		CHECK_EQ(regs.v[half_reg_count - 1], std::byte{0x09});
	}

	SUBCASE("Overflowing")
	{
		std::fill_n(regs.v.begin() + half_reg_count, half_reg_count - 1, std::byte{0xFF});
		for (size_t reg_idx = 0; reg_idx < half_reg_count - 1; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((half_reg_count + reg_idx) << 4);
			instructions::add_reg_reg(regs, instr);

			CHECK_EQ(regs.v[0xF], std::byte{0x01});
			CHECK_EQ(regs.v[reg_idx], std::byte{0x08});
			CHECK_EQ(regs.v[half_reg_count + reg_idx], std::byte{0xFF});
		}
		CHECK_EQ(regs.v[half_reg_count - 1], std::byte{0x09});
	}
}

TEST_CASE("SUB reg reg instruction")
{
	auto regs = registers(0);
	std::fill_n(regs.v.begin(), half_reg_count, std::byte{0x09});
	auto instr = get_zero_instruction();

	SUBCASE("Non-borrowing")
	{
		std::fill_n(regs.v.begin() + half_reg_count, half_reg_count - 1, std::byte{0x01});
		for (size_t reg_idx = 0; reg_idx < half_reg_count - 1; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((half_reg_count + reg_idx) << 4);
			instructions::sub_reg_reg(regs, instr);

			CHECK_EQ(regs.v[0xF], std::byte{0x01});
			CHECK_EQ(regs.v[reg_idx], std::byte{0x08});
			CHECK_EQ(regs.v[half_reg_count + reg_idx], std::byte{0x01});
		}
		CHECK_EQ(regs.v[half_reg_count - 1], std::byte{0x09});
	}

	SUBCASE("Borrowing")
	{
		std::fill_n(regs.v.begin() + half_reg_count, half_reg_count - 1, std::byte{0x0A});
		for (size_t reg_idx = 0; reg_idx < half_reg_count - 1; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((half_reg_count + reg_idx) << 4);
			instructions::sub_reg_reg(regs, instr);

			CHECK_EQ(regs.v[0xF], std::byte{0x00});
			CHECK_EQ(regs.v[reg_idx], std::byte{0xFF});
			CHECK_EQ(regs.v[half_reg_count + reg_idx], std::byte{0x0A});
		}
		CHECK_EQ(regs.v[half_reg_count - 1], std::byte{0x09});
	}
}

TEST_CASE("SHR reg reg instruction")
{
	auto regs = registers(0);
	auto instr = get_zero_instruction();

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
	auto instr = get_zero_instruction();

	SUBCASE("Non-borrowing")
	{
		std::fill_n(regs.v.begin(), half_reg_count, std::byte{0x01});
		std::fill_n(regs.v.begin() + half_reg_count, half_reg_count - 1, std::byte{0x09});
		for (size_t reg_idx = 0; reg_idx < half_reg_count - 1; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((half_reg_count + reg_idx) << 4);
			instructions::subn_reg_reg(regs, instr);

			CHECK_EQ(regs.v[0xF], std::byte{0x01});
			CHECK_EQ(regs.v[reg_idx], std::byte{0x08});
			CHECK_EQ(regs.v[half_reg_count + reg_idx], std::byte{0x09});
		}
		CHECK_EQ(regs.v[half_reg_count - 1], std::byte{0x01});
	}

	SUBCASE("Borrowing")
	{
		std::fill_n(regs.v.begin(), half_reg_count, std::byte{0x0A});
		std::fill_n(regs.v.begin() + half_reg_count, half_reg_count - 1, std::byte{0x09});
		for (size_t reg_idx = 0; reg_idx < half_reg_count - 1; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((half_reg_count + reg_idx) << 4);
			instructions::subn_reg_reg(regs, instr);

			CHECK_EQ(regs.v[0xF], std::byte{0x00});
			CHECK_EQ(regs.v[reg_idx], std::byte{0xFF});
			CHECK_EQ(regs.v[half_reg_count + reg_idx], std::byte{0x09});
		}
		CHECK_EQ(regs.v[half_reg_count - 1], std::byte{0x0A});
	}
}

TEST_CASE("SHL reg reg instruction")
{
	auto regs = registers(0);
	auto instr = get_zero_instruction();

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

TEST_CASE("SNE reg reg instruction")
{
	auto regs = registers(0);
	auto instr = instructions::instruction{std::byte{0x00}, std::byte{0xFF}};

	SUBCASE("All regs are same")
	{
		for (size_t reg_idx = 0; reg_idx < half_reg_count; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((reg_idx + half_reg_count) << 4);
			instructions::sne_reg_reg(regs, instr);
			CHECK_EQ(regs.pc, 0);
		}
	}

	SUBCASE("Half regs different")
	{
		std::fill_n(regs.v.begin(), half_reg_count, std::byte{0xFF});
		for (size_t reg_idx = 0; reg_idx < half_reg_count; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((reg_idx + half_reg_count) << 4);
			instructions::sne_reg_reg(regs, instr);
			CHECK_EQ(regs.pc, reg_idx * 2 + 2);
		}
	}
}

TEST_CASE("LD I addr instruction")
{
	auto regs = registers(0);
	auto instr = get_zero_instruction();

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

TEST_CASE("JP V0 addr instruction")
{
	auto regs = registers(0);
	auto instr = get_zero_instruction();

	SUBCASE("Jump to 0x001 + 0x01")
	{
		regs.v[0] = std::byte{0x01};
		instr[1] = std::byte{0x01};
		instructions::jp_v0_addr(regs, instr);
		REQUIRE_EQ(regs.pc, uint16_t{0x02});
	}

	SUBCASE("Jump to 0xF00 + 0xFF")
	{
		regs.v[0] = std::byte{0xFF};
		instr[0] = std::byte{0x0F};
		instructions::jp_v0_addr(regs, instr);
		REQUIRE_EQ(regs.pc, uint16_t{0xFFF});
	}

	SUBCASE("Jump to 0x8F0")
	{
		instr[0] = std::byte{0x08};
		regs.v[0] = std::byte{0xF0};
		instructions::jp_v0_addr(regs, instr);
		REQUIRE_EQ(regs.pc, uint16_t{0x8F0});
	}
}

TEST_CASE("RND Vx, byte")
{
	auto regs = registers(0);
	auto instr = get_zero_instruction();

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

TEST_CASE("LD reg dt instruction")
{
	auto regs = registers(0);
	auto instr = get_zero_instruction();

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
	auto instr = get_zero_instruction();
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
	auto instr = get_zero_instruction();
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
	std::fill_n(regs.v.begin(), half_reg_count, std::byte{0xFF});
	auto instr = get_zero_instruction();

	for (size_t reg_idx = 0; reg_idx < half_reg_count; ++reg_idx)
	{
		instr[0] = std::byte(half_reg_count + reg_idx);
		instr[1] = std::byte(reg_idx << 4);
		instructions::ld_reg_reg(regs, instr);
	}

	REQUIRE(std::all_of(regs.v.begin(), regs.v.end(), [](std::byte reg)
	{
		return reg == std::byte{0xFF};
	}));
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
