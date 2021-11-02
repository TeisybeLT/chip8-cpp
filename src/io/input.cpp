#include "input.hpp"

#include <SDL_events.h>
#include <SDL_keyboard.h>

using namespace chip8;

keyboard_state chip8::get_keyboard_state() noexcept
{
	const auto* kbd_state = SDL_GetKeyboardState(nullptr);

	keyboard_state cur_state;
	for (size_t cnt = 0; cnt < chip8::key_count; ++cnt)
		cur_state[cnt] = kbd_state[chip8::default_map[cnt]];
	
	return cur_state;
}
