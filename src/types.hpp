#ifndef TYPES_HPP
#define TYPES_HPP

#include "constants.hpp"

#include <array>

namespace chip8
{
	using memory_t = std::array<std::byte, constants::mem_size>;
	using stack_t = std::array<uint16_t, constants::stack_size>;
}

#endif /* TYPES_HPP */
