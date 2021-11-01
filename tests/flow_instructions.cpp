#include "doctest.h"
#include "test_helpers.hpp"
#include "instructions.hpp"

using namespace chip8;

TEST_CASE("RET instruction")
{
	for (size_t cnt = 0; cnt < constants::stack_size; ++cnt)
	{
		auto stack = stack_t();
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
	auto instr = helpers::get_zero_instruction();

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
	auto stack = stack_t();
	auto regs = registers(1337);
	auto instr = helpers::get_zero_instruction();

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

TEST_CASE("JP V0 addr instruction")
{
	auto regs = registers(0);
	auto instr = helpers::get_zero_instruction();

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
