#pragma once
#include "constants.hpp"
#include <cinttypes>
#include <array>
namespace SunBoy
{
	enum PPUState
	{
		HBlank,
		VBlank,
		OAMSearch,
		DrawScanline
	};

	enum LCDControlFlags
	{
		DisplayEnable = 0x80,
		WindowTileMap = 0x40,
		WindowEnable = 0x20,
		BGWindowTileData = 0x10,
		BGTileMap = 0x8,
		SpriteSize = 0x4,
		SpriteEnable = 0x2,
		BGEnable = 0x1
	};

	enum StatusFlags
	{
		EnableLYCandLYInt = 0x40,
		EnableOAMInt = 0x20,
		EnableVBlankInt = 0x10,
		EnableHBlankInt = 0x8,
		LYCandLYCompareType = 0x4,
		ModeFlag = 0x3
	};

	enum ObjectAttributeFlags
	{
		Priority = 0x80,
		FlipY = 0x40,
		FlipX = 0x20,
		Palette = 0x10
	};

	enum DisplayRenderFlags
	{
		Background = 0x1,
		Window = 0x2,
		Objects = 0x4,
	};

	struct Object
	{
		uint8_t y = 0;
		uint8_t x = 0;
		uint8_t tile = 0;
		uint8_t attributes = 0;
	};

	class Core;
	class PPU
	{
		Core &core;

	public:
		bool window_draw_flag = false, previously_disabled = false;
		uint8_t num_obj_on_scanline = 0;
		uint8_t lcd_control = 0, status = 0;
		uint8_t screen_scroll_y = 0, screen_scroll_x = 0;
		uint8_t line_y = 0, line_y_compare = 0;
		uint8_t window_y = 0, window_x = 0;
		uint8_t window_line_y = 0, background_palette = 0;
		uint8_t object_palette_0 = 0, object_palette_1 = 0;
		uint8_t render_flags = DisplayRenderFlags::Background | DisplayRenderFlags::Window | DisplayRenderFlags::Objects;
		PPUState mode = PPUState::HBlank;
		uint32_t cycles = 0;
		std::array<uint8_t, 8193> vram{};
		std::array<uint8_t, 256> oam{};
		std::array<Object, 10> objects_on_scanline{};
		std::array<uint8_t, LCD_WIDTH * LCD_HEIGHT> bg_color_table{};

		std::array<uint32_t, LCD_WIDTH * LCD_HEIGHT> framebuffer{};
		std::array<uint32_t, LCD_WIDTH * LCD_HEIGHT> framebuffer_complete{};
		std::array<uint32_t, 4> color_table = LCD_GRAY_PALETTE;

		PPU(Core &core);

		void reset(bool hard_reset);
		void set_post_boot_state();
		void step(uint32_t accumulated_cycles);

		void write_vram(uint16_t address, uint8_t value);
		void write_oam(uint16_t address, uint8_t value);
		void instant_dma(uint8_t address);

		uint8_t read_vram(uint16_t address) const;
		uint8_t read_oam(uint16_t address) const;
		bool check_stat(uint8_t flags) const;
		void set_stat(uint8_t flags, bool value);
		bool stat_any() const;

	private:
		void render_scanline();
		void render_bg_layer();
		void render_window_layer();
		void scan_oam();
		void render_sprite_layer();
		void check_ly_lyc(bool allow_interrupts);

		uint32_t palette_index_to_color(uint8_t palette, uint8_t bitIndex) const;
	};
}