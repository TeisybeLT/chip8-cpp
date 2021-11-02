#ifndef INPUT_HPP
#define INPUT_HPP

#include <SDL_keycode.h>

#include <array>
#include <bitset>

namespace chip8
{
	static constexpr auto key_count = size_t{16};

	using keyboard_state = std::bitset<key_count>;

	/*	Default keyboard map
	 *		---------
	 *		|2|3|4|5|
	 *		|W|E|R|T|
	 *		|A|S|D|F|
	 *		|Z|X|C|V|
	 *		---------
	*/
	static constexpr auto default_map = std::array<int, key_count>
	{
		SDL_SCANCODE_X, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4,
		SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_R, SDL_SCANCODE_A,
		SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_Z, SDL_SCANCODE_C,
		SDL_SCANCODE_5, SDL_SCANCODE_T, SDL_SCANCODE_F, SDL_SCANCODE_V
	};

	// Ensure that chip8::get_keyboard_state() is called after all events have been processed
	keyboard_state get_keyboard_state() noexcept;
}

#endif /* INPUT_HPP */
