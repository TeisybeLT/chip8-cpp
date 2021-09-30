#ifndef CPU_HPP
#define CPU_HPP

#include <array>

namespace chip8
{
	struct cpu
	{
		struct registers
		{
			std::array<std::byte, 0xF> v;
			uint16_t i;
			uint16_t pc;
			uint16_t sp;
		};

		static constexpr auto c_stack_depth = size_t {16};

		void tick();

	private:
		std::array<uint16_t, c_stack_depth> m_stack;
	};
}

#endif /* CPU_HPP */
