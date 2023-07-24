#pragma once
#include <cinttypes>

namespace SunBoy
{
	class Core;
	class Timer
	{
		Core &core;
		uint32_t div_cycles = 0, tima_cycles = 0;

	public:
		uint8_t div = 0;
		uint8_t tima = 0;
		uint8_t tma = 0;
		uint8_t tac = 0;
		uint32_t tac_rate = 0;
		uint8_t apu_div = 0;
		Timer(Core &core);

		void set_tac(uint8_t rate);
		void reset_div();
		uint8_t read_div();
		void update(uint32_t addCycles);

		bool timer_enabled() const;
		void change_div(uint8_t new_div);
	};

}
