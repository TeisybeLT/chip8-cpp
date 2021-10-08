#include "doctest.h"
#include "instructions.hpp"

using namespace chip8;

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
		CHECK_EQ(instructions::detail::get_lower_nibble<T>(std::byte{cnt}), T(cnt & 0x0F));
}

TEST_CASE_TEMPLATE("Extracting upper nibble", T, uint16_t, int32_t, uint32_t, int32_t, size_t)
{
	for (uint8_t cnt = 0; cnt < 0xFF; ++cnt)
		CHECK_EQ(instructions::detail::get_upper_nibble<T>(std::byte{cnt}), T((cnt & 0xF0) >> 4));
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
