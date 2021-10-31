#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "chip8_font.hpp"
#include "constants.hpp"
#include "registers.hpp"

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

	template <std::integral T>
	[[nodiscard]] constexpr T get_upper_nibble(std::byte byte) noexcept;

	namespace detail
	{
		template <std::integral T>
		[[nodiscard]] constexpr T get_lower_12_bits(instruction instr) noexcept;
	}

	constexpr void ret(chip8::registers& regs, std::array<uint16_t, constants::stack_size>& stack) noexcept;
	constexpr void jp(chip8::registers& regs, instruction instr) noexcept;
	constexpr void call(chip8::registers& regs, std::array<uint16_t, constants::stack_size>& stack, instruction instr) noexcept;
	constexpr void se_reg_byte(chip8::registers& regs, instruction instr) noexcept;
	constexpr void sne_reg_byte(chip8::registers& regs, instruction instr) noexcept;
	constexpr void se_reg_reg(chip8::registers& regs, instruction instr) noexcept;
	constexpr void ld_reg_byte(chip8::registers& regs, instruction instr) noexcept;
	constexpr void add_reg_byte(chip8::registers& regs, instruction instr) noexcept;
	constexpr void ld_reg_reg(chip8::registers& regs, instruction instr) noexcept;
	constexpr void or_reg_reg(chip8::registers& regs, instruction instr) noexcept;
	constexpr void and_reg_reg(chip8::registers& regs, instruction instr) noexcept;
	constexpr void xor_reg_reg(chip8::registers& regs, instruction instr) noexcept;
	constexpr void add_reg_reg(chip8::registers& regs, instruction instr) noexcept;
	constexpr void sub_reg_reg(chip8::registers& regs, instruction instr) noexcept;
	constexpr void shr_reg_reg(chip8::registers& regs, instruction instr) noexcept;
	constexpr void subn_reg_reg(chip8::registers& regs, instruction instr) noexcept;
	constexpr void shl_reg_reg(chip8::registers& regs, instruction instr) noexcept;
	constexpr void sne_reg_reg(chip8::registers& regs, instruction instr) noexcept;
	constexpr void ld_i_addr(chip8::registers& regs, instruction instr) noexcept;
	constexpr void jp_v0_addr(chip8::registers& regs, instruction instr) noexcept;
	void rnd_reg_byte(chip8::registers& regs, instruction instr) noexcept;
	void skp_reg(chip8::registers& regs, instruction instr) noexcept;
	void sknp_reg(chip8::registers& regs, instruction instr) noexcept;
	constexpr void ld_reg_dt(chip8::registers& regs, instruction instr) noexcept;
	[[nodiscard]] bool ld_reg_k(chip8::registers& regs, instruction instr) noexcept;
	constexpr void ld_dt_reg(chip8::registers& regs, instruction instr) noexcept;
	constexpr void ld_st_reg(chip8::registers& regs, instruction instr) noexcept;
	constexpr void add_i_reg(chip8::registers& regs, instruction instr) noexcept;
	constexpr void ld_f_reg(chip8::registers& regs, instruction instr) noexcept;

	template <size_t array_size>
	constexpr void ld_b_reg(chip8::registers& regs, std::array<std::byte, array_size>& mem, instruction instr);

	template <size_t array_size>
	constexpr void str_i_reg(chip8::registers& regs, std::array<std::byte, array_size>& mem, instruction instr);
	template <size_t array_size>
	constexpr void str_reg_i(chip8::registers& regs, std::array<std::byte, array_size>& mem, instruction instr);

}

namespace chip8::instructions
{
	template <std::integral T>
	constexpr T detail::get_lower_12_bits(instructions::instruction instr) noexcept
	{
		return std::to_integer<T>(instr[0] & std::byte{0x0F}) << 8 | std::to_integer<T>(instr[1]);
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
		return instr[0] >> 4;
	}

	template <std::integral T>
	constexpr T instructions::get_lower_nibble(std::byte byte) noexcept
	{
		return std::to_integer<T>(byte & std::byte{0x0F});
	}

	template <std::integral T>
	constexpr T instructions::get_upper_nibble(std::byte byte) noexcept
	{
		return std::to_integer<T>(byte >> 4);
	}

	constexpr void instructions::ret(chip8::registers& regs, std::array<uint16_t, constants::stack_size>& stack) noexcept
	{
		regs.pc = stack[regs.sp];
		--regs.sp;
	}

	constexpr void instructions::jp(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		regs.pc = detail::get_lower_12_bits<decltype(regs.pc)>(instr);
	}

	constexpr void instructions::call(chip8::registers& regs, std::array<uint16_t, constants::stack_size>& stack, instructions::instruction instr) noexcept
	{
		++regs.sp;
		stack[regs.sp] = regs.pc;
		regs.pc = detail::get_lower_12_bits<decltype(regs.pc)>(instr);
	}

	constexpr void instructions::se_reg_byte(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		if (regs.v[instructions::get_lower_nibble<size_t>(instr[0])] == instr[1])
			regs.pc += 2;
	}

	constexpr void instructions::sne_reg_byte(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		if (regs.v[instructions::get_lower_nibble<size_t>(instr[0])] != instr[1])
			regs.pc += 2;
	}

	constexpr void instructions::se_reg_reg(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		if (regs.v[instructions::get_lower_nibble<size_t>(instr[0])] ==
			regs.v[instructions::get_upper_nibble<size_t>(instr[1])])
				regs.pc += 2;
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
			regs.v[instructions::get_upper_nibble<size_t>(instr[1])];
	}

	constexpr void instructions::or_reg_reg(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		regs.v[instructions::get_lower_nibble<size_t>(instr[0])] |=
			regs.v[instructions::get_upper_nibble<size_t>(instr[1])];
	}

	constexpr void instructions::and_reg_reg(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		regs.v[instructions::get_lower_nibble<size_t>(instr[0])] &=
			regs.v[instructions::get_upper_nibble<size_t>(instr[1])];
	}

	constexpr void instructions::xor_reg_reg(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		regs.v[instructions::get_lower_nibble<size_t>(instr[0])] ^=
			regs.v[instructions::get_upper_nibble<size_t>(instr[1])];
	}

	constexpr void instructions::add_reg_reg(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		const auto x_offset = instructions::get_lower_nibble<size_t>(instr[0]);
		const auto y_offset = instructions::get_upper_nibble<size_t>(instr[1]);
	
		auto tmp = std::to_integer<uint8_t>(regs.v[x_offset]) + std::to_integer<uint8_t>(regs.v[y_offset]);

		if (tmp > 255)
		{
			regs.v[x_offset] = std::byte(tmp & 0xFF);
			regs.v[0xF] = std::byte{0x01};
		}
		else
		{
			regs.v[x_offset] = std::byte(tmp);
			regs.v[0xF] = std::byte{0x00};
		}
	}

	constexpr void instructions::sub_reg_reg(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		const auto x_offset = instructions::get_lower_nibble<size_t>(instr[0]);
		const auto y_offset = instructions::get_upper_nibble<size_t>(instr[1]);

		regs.v[0xF] = (regs.v[x_offset] > regs.v[y_offset]) ? std::byte{0x01} : std::byte{0x00};
		auto res = std::to_integer<uint8_t>(regs.v[x_offset]) - std::to_integer<uint8_t>(regs.v[y_offset]);

		regs.v[x_offset] = (res >= 0) ? std::byte(res) : std::byte(res & 0xFF);
	}

	constexpr void instructions::shr_reg_reg(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		const auto x_offset = instructions::get_lower_nibble<size_t>(instr[0]);
		regs.v[0xF] = regs.v[x_offset] & std::byte{0x01};
		regs.v[x_offset] >>= 1;
	}

	constexpr void instructions::subn_reg_reg(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		const auto x_offset = instructions::get_lower_nibble<size_t>(instr[0]);
		const auto y_offset = instructions::get_upper_nibble<size_t>(instr[1]);

		regs.v[0xF] = (regs.v[y_offset] > regs.v[x_offset]) ? std::byte{0x01} : std::byte{0x00};
		auto res = std::to_integer<uint8_t>(regs.v[y_offset]) - std::to_integer<uint8_t>(regs.v[x_offset]);

		regs.v[x_offset] = (res >= 0) ? std::byte(res) : std::byte(res & 0xFF);
	}

	constexpr void instructions::shl_reg_reg(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		const auto x_offset = instructions::get_lower_nibble<size_t>(instr[0]);
		regs.v[0xF] = (regs.v[x_offset] & std::byte{0x80}) != std::byte{0x00} ?
			std::byte{0x01} : std::byte{0x00};
		regs.v[x_offset] <<= 1;
	}

	constexpr void instructions::sne_reg_reg(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		if (regs.v[instructions::get_lower_nibble<size_t>(instr[0])] !=
			regs.v[instructions::get_upper_nibble<size_t>(instr[1])])
				regs.pc += 2;
	}

	constexpr void instructions::ld_i_addr(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		regs.i = instructions::detail::get_lower_12_bits<decltype(regs.i)>(instr);
	}

	constexpr void instructions::jp_v0_addr(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		regs.pc = uint16_t(instructions::detail::get_lower_12_bits<decltype(regs.pc)>(instr) + 
			std::to_integer<uint16_t>(regs.v[0x00])) & uint16_t{0xFFF};
	}

	constexpr void instructions::ld_reg_dt(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		regs.v[instructions::get_lower_nibble<size_t>(instr[0])] = std::byte{regs.delay};
	}

	constexpr void instructions::ld_dt_reg(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		regs.delay = std::to_integer<uint8_t>(regs.v[instructions::get_lower_nibble<size_t>(instr[0])]);
	}

	constexpr void instructions::ld_st_reg(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		regs.sound = std::to_integer<uint8_t>(regs.v[instructions::get_lower_nibble<size_t>(instr[0])]);
	}

	constexpr void instructions::add_i_reg(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		regs.i += std::to_integer<uint8_t>(regs.v[instructions::get_lower_nibble<size_t>(instr[0])]);
	}

	constexpr void instructions::ld_f_reg(chip8::registers& regs, instructions::instruction instr) noexcept
	{
		const auto digit = std::to_integer<uint8_t>(regs.v[instructions::get_lower_nibble<size_t>(instr[0])]);
		regs.i = uint16_t(chip8::font::c_font_offset + digit * chip8::font::c_bytes_per_symbol);
	}

	template <size_t array_size>
	constexpr void instructions::ld_b_reg(chip8::registers& regs, std::array<std::byte, array_size>& mem, instructions::instruction instr)
	{
		if (array_size < size_t{regs.i} + 3)
			throw std::runtime_error("Out of bounds memory access");

		auto number = std::to_integer<int>(regs.v[instructions::get_lower_nibble<size_t>(instr[0])]);
		for (int idx = 2; idx >= 0; --idx)
		{
			mem[regs.i + idx] = std::byte(number % 10);
			number /= 10;
		}
	}

	template <size_t array_size>
	constexpr void instructions::str_i_reg(chip8::registers& regs, std::array<std::byte, array_size>& mem, instructions::instruction instr)
	{
		const auto last_reg = instructions::get_lower_nibble<size_t>(instr[0]);
		if (array_size < size_t{regs.i} + last_reg)
			throw std::runtime_error("Out of bounds memory access");

		std::copy(regs.v.begin(), regs.v.begin() + last_reg + 1, mem.begin() + size_t{regs.i});
	}

	template <size_t array_size>
	constexpr void instructions::str_reg_i(chip8::registers& regs, std::array<std::byte, array_size>& mem, instructions::instruction instr)
	{
		const auto last_reg = instructions::get_lower_nibble<size_t>(instr[0]);
		if (array_size < size_t{regs.i} + last_reg)
			throw std::runtime_error("Out of bounds memory access");

		std::copy(mem.begin() + size_t{regs.i}, mem.begin() + size_t{regs.i} + last_reg + 1, regs.v.begin());
	}
}

#endif /* INSTRUCTIONS_HPP */
