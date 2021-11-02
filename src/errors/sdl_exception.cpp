#include "errors/sdl_exception.hpp"

#include <SDL_error.h>

#include <sstream>
#include <utility>

using namespace sdl;
using namespace std::literals::string_literals;

void sdl::sdl_check_error(int retval, std::string error_message)
{
	if (retval < 0)
		throw sdl_exception(retval, std::move(error_message));
}

void sdl::sdl_check_null(void* retval, std::string error_message)
{
	if (!retval)
		throw sdl_exception(-1, std::move(error_message));
}

sdl_exception::sdl_exception(int error_code, std::string error_message) noexcept :
	m_error_code{error_code},
	m_sdl_error(SDL_GetError()),
	m_error_message(std::move(error_message))
{
	this->build_formatted_message();
}

void sdl_exception::build_formatted_message() noexcept
{
	// This string building is used to avoid dangling pointer
	auto stream = std::ostringstream{};
	stream << "SDL Failure: "s;
	stream << this->m_error_message << " ("s << std::to_string(this->m_error_code) << ' ';
	stream << this->m_sdl_error << ')';
	this->m_formatted_message = stream.str();
}

const char* sdl_exception::what() const noexcept
{
	return m_formatted_message.c_str();
}
