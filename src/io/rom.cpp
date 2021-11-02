#include "io/rom.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>

using namespace std::literals::string_literals;
using namespace chip8;

void chip8::load_rom_from_file(const std::filesystem::path& rom_path, chip8::memory_t& mem)
{
	// Sanity check
	if (!std::filesystem::exists(rom_path))
		throw std::runtime_error("File does not exist at "s + rom_path.string());

	if (!std::filesystem::is_regular_file(rom_path))
		throw std::runtime_error(rom_path.string() + " does not point to a regular file"s);

	// Load to mem
	auto reader = std::ifstream(rom_path, std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
	if (!reader)
		throw std::runtime_error("Unable to open file "s + rom_path.string() + " for reading"s);

	static constexpr auto max_rom_size = chip8::constants::mem_size - chip8::constants::code_start;
	const auto file_byte_count = reader.tellg();
	if (file_byte_count > max_rom_size)
	{
		throw std::runtime_error("Rom file is too large. Expected up to "s
			+ std::to_string(max_rom_size) + " got "s + std::to_string(file_byte_count));
	}

	reader.seekg(0);
	reader.read(reinterpret_cast<char*>(mem.data() + chip8::constants::code_start), file_byte_count);
}
