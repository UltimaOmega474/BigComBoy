#pragma once

#include <cinttypes>
#include <array>
namespace Angbe
{
	constexpr int32_t LCD_WIDTH = 160;
	constexpr int32_t LCD_HEIGHT = 144;
	constexpr uint8_t INT_VBLANK_BIT = 0x01;
	constexpr uint8_t INT_LCD_STAT_BIT = 0x02;
	constexpr uint8_t INT_TIMER_BIT = 0x04;
	constexpr uint8_t INT_SERIAL_PORT_BIT = 0x08;
	constexpr uint8_t INT_JOYPAD_BIT = 0x10;
	constexpr uint32_t CPU_CLOCK_RATE = 4194304;
	constexpr uint32_t CYCLES_PER_FRAME = static_cast<uint32_t>((CPU_CLOCK_RATE / 59.7275));
	constexpr int16_t NO_DISPLACEMENT = 0;
	constexpr int16_t INCREMENT = 1;
	constexpr int16_t DECREMENT = -1;
	constexpr auto WITH_CARRY = true;
	constexpr auto WITHOUT_CARRY = false;
	constexpr auto DONT_SET_IME = false;
	constexpr auto SET_IME = true;

	constexpr uint8_t duty_table[4][8]{
		{0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 1, 1, 1},
		{0, 1, 1, 1, 1, 1, 1, 0}};
}