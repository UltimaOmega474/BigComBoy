#include "timer.hpp"
#include "core.hpp"

namespace SunBoy
{
	Timer::Timer(Core &core)
		: core(core)
	{
		set_tac(0);
	}

	void Timer::reset()
	{
		div_cycles = 0;
		tima_cycles = 0;
		div = 0;
		tima = 0;
		tma = 0;
		set_tac(0);
		apu_div = 0;
	}

	void Timer::set_tac(uint8_t rate)
	{
		constexpr uint32_t tac_table[4] = {1024, 16, 64, 256};

		tac_rate = tac_table[rate & 0x3];
		tac = rate;
	}

	void Timer::reset_div()
	{
		change_div(0);
	}

	uint8_t Timer::read_div()
	{
		return div;
	}

	void Timer::update(uint32_t addCycles)
	{
		if (core.cpu.stopped)
		{
			change_div(0);
		}
		else
		{
			div_cycles += addCycles;
		}

		if (div_cycles >= 256)
		{
			change_div(div + 1);
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

	void Timer::change_div(uint8_t new_div)
	{
		if ((div & 0b10000) && ((new_div & 0b10000) == 0))
		{
			apu_div++;
			core.apu.step_counters(apu_div);
		}

		if (apu_div == 8)
			apu_div = 0;

		div = new_div;
		div_cycles = 0;
	}
}