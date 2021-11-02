#ifndef ROM_HPP
#define ROM_HPP

#include "types.hpp"

#include <filesystem>

namespace chip8
{
	void load_rom_from_file(const std::filesystem::path& rom_path, chip8::memory_t& mem);
}

#endif /* ROM_HPP */
