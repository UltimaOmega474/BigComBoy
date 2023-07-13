#pragma once
#include <cinttypes>

namespace GameBoy
{
	class Core;
	class Timer
	{
		Core &core;
		uint32_t div_cycles, tima_cycles;

	public:
		uint8_t div;
		uint8_t tima;
		uint8_t tma;
		uint8_t tac;
		uint32_t tac_rate;

		Timer(Core &core);

		void set_tac(uint8_t rate);
		void reset_div();
		uint8_t read_div();
		void update(uint32_t addCycles);

		bool timer_enabled() const;
	};

}
