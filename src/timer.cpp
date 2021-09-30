#include "timer.hpp"

#include <SDL_log.h>

using namespace chip8;

timer::timer(uint8_t& timer_reg, const std::chrono::nanoseconds& update_period,
	std::function<void()> start_callback, std::function<void()> stop_callback) :
	m_reg{timer_reg},
	m_update_period{update_period},
	m_start_callback{start_callback},
	m_stop_callback{stop_callback}
{}

void timer::report_change() const
{
	if (this->m_reg > 0 && this->m_start_callback)
		this->m_start_callback();
	else if (this->m_stop_callback)
		this->m_stop_callback();
}

void timer::update(const std::chrono::nanoseconds& delta)
{
	auto update_counter = size_t{0};
	this->m_accumulated_time += delta;

	while (this->m_accumulated_time >= this->m_update_period)
	{
		this->m_accumulated_time -= this->m_update_period;
		process_timer();
		++update_counter;
	}

	if (update_counter > 1)
	{
		SDL_LogWarn(SDL_LOG_CATEGORY_SYSTEM, "Timer had to do %d rounds to compensate for lag",
			update_counter);
	}
}

void timer::process_timer()
{
	if (m_reg > 0)
		if (--m_reg == 0 && this->m_stop_callback)
			this->m_stop_callback();
}
