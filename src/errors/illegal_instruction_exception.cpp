#include "errors/illegal_instruction_exception.hpp"

#include <cstddef>
#include <sstream>

using namespace chip8;
using namespace std::literals::string_literals;

illegal_instruction::illegal_instruction(const registers& regs, instr_t instruction) noexcept :
	m_illegal_instruction{instruction},
	m_offending_address{regs.pc}
{
	auto str = std::ostringstream{"Illegal instruction 0x"s, std::ios::ate};
	str << std::hex << std::to_integer<int>(instruction[0]) << std::to_integer<int>(instruction[1]);
	str << " at 0x"s << std::hex << int{this->m_offending_address};
	this->m_formatted_message = str.str();
}

const char* illegal_instruction::what() const noexcept
{
	return this->m_formatted_message.c_str();
}
