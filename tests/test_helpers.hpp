#include "constants.hpp"
#include "instructions.hpp"

using namespace chip8;

namespace helpers
{
	static constexpr auto half_reg_count = constants::v_reg_count / 2;

	constexpr auto get_zero_instruction() noexcept
	{
		return instr_t{std::byte{0x00}, std::byte{0x00}};
	}
}
