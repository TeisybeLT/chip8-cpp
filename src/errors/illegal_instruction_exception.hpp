#ifndef ILLEGAL_INSTRUCTION_EXCEPTION_HPP
#define ILLEGAL_INSTRUCTION_EXCEPTION_HPP

#include "types.hpp"
#include "registers.hpp"

#include <stdexcept>

namespace chip8
{
	struct illegal_instruction : public std::exception
	{
		illegal_instruction(const registers& regs, instr_t instruction) noexcept;

		illegal_instruction() = delete;
		~illegal_instruction() = default;
		illegal_instruction(const illegal_instruction&) = default;
		illegal_instruction(illegal_instruction&&) = default;
		illegal_instruction& operator=(const illegal_instruction&) = default;
		illegal_instruction& operator=(illegal_instruction&&) = default;

		const char* what() const noexcept override;

	private:
		const instr_t m_illegal_instruction;
		const uint16_t m_offending_address;

		std::string m_formatted_message;
	};
}

#endif /* ILLEGAL_INSTRUCTION_EXCEPTION_HPP */
