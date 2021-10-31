#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <chrono>
#include <cstdint>

using namespace std::literals::chrono_literals;

namespace chip8::constants
{
	static constexpr auto v_reg_count = std::size_t {16};
	static constexpr auto mem_size = std::size_t {4096};
	static constexpr auto stack_size = std::size_t {16};
	static constexpr auto code_start = std::uint16_t {0x200};
	static constexpr auto timer_tick_freq = std::chrono::duration_cast<std::chrono::nanoseconds>(1s) / 60;
}

#endif /* CONSTANTS_HPP */
