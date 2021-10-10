#include "doctest.h"
#include "instructions.hpp"

using namespace chip8;

namespace
{
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
			REQUIRE_EQ(regs.pc, reg_idx + 1);
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
			REQUIRE_EQ(regs.pc, reg_idx + 1);
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
	static constexpr auto half_reg_count = std::tuple_size<decltype(std::declval<registers>().v)>::value / 2;

	auto regs = registers(0);
	auto instr = instructions::instruction{std::byte{0x00}, std::byte{0xFF}};

	SUBCASE("All regs are same")
	{
		for (size_t reg_idx = 0; reg_idx < half_reg_count; ++reg_idx)
		{
			instr[0] = std::byte(reg_idx);
			instr[1] = std::byte((reg_idx + half_reg_count) << 4);
			instructions::se_reg_reg(regs, instr);
			REQUIRE_EQ(regs.pc, reg_idx + 1);
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
