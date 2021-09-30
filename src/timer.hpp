#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <functional>

namespace chip8
{
	struct timer
	{
		timer(
			uint8_t& timer_reg,
			const std::chrono::nanoseconds& update_period,
			std::function<void()> start_callback = std::function<void()>(),
			std::function<void()> stop_callback = std::function<void()>()
		);

		void report_change() const;
		void update(const std::chrono::nanoseconds& delta);

	private:
		void process_timer();

		uint8_t& m_reg;

		const std::chrono::nanoseconds m_update_period;
		std::chrono::nanoseconds m_accumulated_time;

		std::function<void()> m_start_callback;
		std::function<void()> m_stop_callback;
	};
}

#endif /* TIMER_HPP */
