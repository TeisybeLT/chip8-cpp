#include "doctest.h"
#include "timer.hpp"

using namespace std::literals::chrono_literals;

TEST_CASE("Timer at zero" *
	doctest::description("Tests if timer does not change during update when it has already reached zero"))
{
	auto test_reg = uint8_t{0};
	auto timer = chip8::timer(test_reg, 10ms);

	for (size_t cnt = 0; cnt < 100; ++cnt)
	{
		timer.update(3ms);
		REQUIRE_EQ(test_reg, 0);
	}
}

TEST_CASE("Timer decrement" *
	doctest::description("Tests timer behaviour when it is decremented"))
{
	auto test_reg = uint8_t{0};

	SUBCASE("Decrement once")
	{
		test_reg = 3;
		auto timer = chip8::timer(test_reg, 5ms);
		timer.update(6ms);
		REQUIRE_EQ(test_reg, 2);
	}

	SUBCASE("Decrement five times")
	{
		test_reg = 10;
		auto timer = chip8::timer(test_reg, 1ms);

		for (size_t cnt = 0; cnt < 5; ++cnt)
		{
			timer.update(1ms);
			REQUIRE_EQ(test_reg, 9 - cnt);
		}
	}

	SUBCASE("Decrement to zero and beyond")
	{
		test_reg = 7;
		auto timer = chip8::timer(test_reg, 1ms);

		// First decrement to zero
		for (size_t cnt = 0; cnt < 7; ++cnt)
		{
			timer.update(1ms);
			REQUIRE_EQ(test_reg, 6 - cnt);
		}

		// Then continue doing so
		for (size_t cnt = 0; cnt < 10; ++cnt)
		{
			timer.update(1ms);
			REQUIRE_EQ(test_reg, 0);
		}
	}
}

TEST_CASE("Timer callbacks" *
	doctest::description("Tests callbacks issued by timer"))
{
	auto test_reg = uint8_t{0};
	auto is_start_called = false;
	auto is_stop_called = false;

	auto start_callback = [&is_start_called] {is_start_called = true;};
	auto stop_callback = [&is_stop_called] {is_stop_called = true;};

	SUBCASE("No callbacks at zero")
	{
		auto timer = chip8::timer(test_reg, 1ms, start_callback, stop_callback);
		for (size_t cnt = 0; cnt < 10; ++cnt)
		{
			timer.update(500us);
			REQUIRE_FALSE(is_start_called);
			REQUIRE_FALSE(is_start_called);
		}
	}

	SUBCASE("Stop callback at zero")
	{
		test_reg = 1;
		auto timer = chip8::timer(test_reg, 1ms, start_callback, stop_callback);
		timer.update(1ms);
		REQUIRE_FALSE(is_start_called);
		REQUIRE(is_stop_called);
	}

	SUBCASE("Stop callback at zero after multiple decrements")
	{
		test_reg = 3;
		auto timer = chip8::timer(test_reg, 1ms, start_callback, stop_callback);
		
		// First two decrements should not invoke callbacks
		for (size_t cnt = 0; cnt < 2; ++cnt)
		{
			timer.update(1ms);
			REQUIRE_FALSE(is_start_called);
			REQUIRE_FALSE(is_stop_called);
		}

		// Next decrement should trigger stop callback
		timer.update(1ms);
		REQUIRE_FALSE(is_start_called);
		REQUIRE(is_stop_called);
		is_stop_called = false;

		// Following callbacks should not call any additional callbacks
		for (size_t cnt = 0; cnt < 10; ++cnt)
		{
			timer.update(1ms);
			REQUIRE_FALSE(is_start_called);
			REQUIRE_FALSE(is_stop_called);
		}
	}

	SUBCASE("Stop callback at register update report, when register is == 0")
	{
		auto timer = chip8::timer(test_reg, 1ms, start_callback, stop_callback);
		timer.report_change();

		REQUIRE_FALSE(is_start_called);
		REQUIRE(is_stop_called);
	}

	SUBCASE("Start callback at register update report, when register is > 0")
	{
		auto timer = chip8::timer(test_reg, 1ms, start_callback, stop_callback);
		test_reg = 255;
		timer.report_change();

		REQUIRE(is_start_called);
		REQUIRE_FALSE(is_stop_called);
	}
}
