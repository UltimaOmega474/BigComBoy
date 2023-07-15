#include "timer.hpp"
#include "core.hpp"

namespace Angbe
{
	Timer::Timer(Core &core)
		: core(core)
	{
		set_tac(0);
	}

	void Timer::set_tac(uint8_t rate)
	{
		constexpr uint32_t tac_table[4] = {1024, 16, 64, 256};

		tac_rate = tac_table[rate & 0x3];
		tac = rate;
	}

	void Timer::reset_div()
	{
		div_cycles = 0;
		div = 0;
	}

	uint8_t Timer::read_div()
	{
		return (div_cycles >> 8) & 0xFF;
	}

	void Timer::update(uint32_t addCycles)
	{

		div_cycles += addCycles;

		if (div_cycles == 256)
		{
			// not sure if this was supposed to be like that
			++div;
			div_cycles = 0;
		}

		if (timer_enabled())
		{
			tima_cycles += addCycles;
			if (tima_cycles >= tac_rate)
			{
				tima_cycles = 0;
				++tima;
				if (tima == 0)
				{
					tima = tma;
					// signal interrupt
					core.request_interrupt(0x04);
				}
			}
		}
	}

	bool Timer::timer_enabled() const
	{
		return tac & 0b100;
	}
}