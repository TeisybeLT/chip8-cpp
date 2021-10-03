#ifndef REGISTERS_HPP
#define REGISTERS_HPP

#include <array>

namespace chip8
{
	struct registers
	{
		constexpr explicit registers(uint16_t initial_pc);

		std::array<std::byte, 0xF> v;

		uint16_t i;

		uint16_t pc;
		uint8_t sp;

		uint8_t delay;
		uint8_t sound;
	};
}

constexpr chip8::registers::registers(uint16_t initial_pc)
	: i{0}, pc{initial_pc}, sp{0}, delay{0}, sound{0}
{
	this->v.fill(std::byte{0x0});
}

#endif /* REGISTERS_HPP */
