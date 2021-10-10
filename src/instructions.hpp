#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "registers.hpp"
#include "interpreter.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <concepts>

namespace chip8::instructions
{
	using instruction = std::array<std::byte, 2>;
	
	[[nodiscard]] constexpr std::byte extract_instruction_class(instruction instr) noexcept;

	template <size_t array_size>
	[[nodiscard]] instruction fetch(const std::array<std::byte, array_size>& mem, uint16_t pc);

	template <std::integral T>
	[[nodiscard]] constexpr T get_lower_nibble(std::byte byte) noexcept;

	namespace detail
	{
		template <std::integral T>
		[[nodiscard]] constexpr T get_lower_12_bits(instruction instr) noexcept;

		template <std::integral T>
		[[nodiscard]] constexpr T get_upper_nibble(std::byte byte) noexcept;
	}

	constexpr void ret(chip8::registers& regs, std::array<uint16_t, interpreter::c_stack_size>& stack) noexcept;
	constexpr void jp(chip8::registers& regs, instruction instr) noexcept;
	constexpr void call(chip8::registers& regs, std::array<uint16_t, interpreter::c_stack_size>& stack, instruction instr) noexcept;
	constexpr void se_reg_byte(chip8::registers& regs, instruction instr) noexcept;
	constexpr void sne_reg_byte(chip8::registers& regs, instruction instr) noexcept;
	constexpr void se_reg_reg(chip8::registers& regs, instruction instr) noexcept;
	constexpr void ld_reg_byte(chip8::registers& regs, instruction instr) noexcept;
	constexpr void add_reg_byte(chip8::registers& regs, instruction instr) noexcept;
	constexpr void ld_reg_reg(chip8::registers& regs, instruction instr) noexcept;
}

namespace chip8::instructions
{
	template <std::integral T>
	constexpr T detail::get_lower_12_bits(instructions::instruction instr) noexcept
	{
		return std::to_integer<T>(instr[0] & std::byte{0x0F}) << 8 | std::to_integer<T>(instr[1]);
	}

	template <std::integral T>
	constexpr T detail::get_upper_nibble(std::byte byte) noexcept
	{
		return std::to_integer<T>(byte >> 4);
	}
}

namespace chip8
{
	template <size_t array_size>
	instructions::instruction instructions::fetch(const std::array<std::byte, array_size>& mem, uint16_t pc)
	{
		static constexpr auto instruction_size = std::tuple_size<instructions::instruction>::value;

		if (array_size < pc + instruction_size)
			throw std::runtime_error("Out of bounds memory access");

		instructions::instruction instr;
		std::copy_n(mem.begin() + pc, instruction_size, instr.begin());
		return instr;
	}

	constexpr std::byte instructions::extract_instruction_class(instructions::instruction instr) noexcept
	{
		return instr[1] >> 4;
	}

	template <std::integral T>
	constexpr T instructions::get_lower_nibble(std::byte byte) noexcept
	{
		return std::to_integer<T>(byte & std::byte{0x0F});
	}

	constexpr void instructions::ret(chip8::registers& regs, std::array<uint16_t, interpreter::c_stack_size>& stack) noexcept
	{
		regs.pc = stack[regs.sp];
		--regs.sp;
	}

	constexpr void instructions::jp(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		regs.pc = detail::get_lower_12_bits<decltype(regs.pc)>(instr);
	}

	constexpr void instructions::call(chip8::registers& regs, std::array<uint16_t, interpreter::c_stack_size>& stack, instructions::instruction instr) noexcept
	{
		++regs.sp;
		stack[regs.sp] = regs.pc;
		regs.pc = detail::get_lower_12_bits<decltype(regs.pc)>(instr);
	}

	constexpr void instructions::se_reg_byte(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		if (regs.v[instructions::get_lower_nibble<size_t>(instr[0])] == instr[1])
			++regs.pc;
	}

	constexpr void instructions::sne_reg_byte(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		if (regs.v[instructions::get_lower_nibble<size_t>(instr[0])] != instr[1])
			++regs.pc;
	}

	constexpr void instructions::se_reg_reg(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		if (regs.v[instructions::get_lower_nibble<size_t>(instr[0])] ==
			regs.v[instructions::detail::get_upper_nibble<size_t>(instr[1])])
				++regs.pc;
	}

	constexpr void instructions::ld_reg_byte(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		regs.v[instructions::get_lower_nibble<size_t>(instr[0])] = instr[1];
	}

	constexpr void instructions::add_reg_byte(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		const auto reg_idx = instructions::get_lower_nibble<size_t>(instr[0]);
		regs.v[reg_idx] = std::byte(std::to_integer<uint8_t>(regs.v[reg_idx])
			+ std::to_integer<uint8_t>(instr[1]));
	}

	constexpr void instructions::ld_reg_reg(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		regs.v[instructions::get_lower_nibble<size_t>(instr[0])] =
			regs.v[instructions::detail::get_upper_nibble<size_t>(instr[1])];
	}
}

#endif /* INSTRUCTIONS_HPP */