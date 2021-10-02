#ifndef REGISTERS_HPP
#define REGISTERS_HPP

#include <array>
#include <cstdint>

namespace chip8
{
	struct registers
	{
		std::array<std::byte, 0xF> v;

		uint16_t i;

		uint16_t pc;
		uint8_t sp;

		uint8_t delay;
		uint8_t sound;
	};
}

#endif /* REGISTERS_HPP */
