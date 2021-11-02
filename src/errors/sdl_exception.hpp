#ifndef SDL_EXCEPTION_HPP
#define SDL_EXCEPTION_HPP

#include <stdexcept>
#include <string>

namespace sdl
{
	void sdl_check_error(int retval, std::string error_message);
	void sdl_check_null(void* retval, std::string error_message);

	struct sdl_exception : public std::exception
	{
		sdl_exception(int error_code, std::string error_message) noexcept;

		sdl_exception() = delete;
		~sdl_exception() = default;
		sdl_exception(const sdl_exception&) = default;
		sdl_exception(sdl_exception&&) = default;
		sdl_exception& operator=(const sdl_exception&) = default;
		sdl_exception& operator=(sdl_exception&&) = default;

		const char* what() const noexcept override;

	private:
		void build_formatted_message() noexcept;

		const int m_error_code;
		const std::string m_sdl_error;
		const std::string m_error_message;

		std::string m_formatted_message;
	};
}

#endif /* SDL_EXCEPTION_HPP */
