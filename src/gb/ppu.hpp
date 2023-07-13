#pragma once

#include <cinttypes>
#include <array>
namespace GameBoy
{
	struct Color
	{
		uint8_t red, green, blue, transparent;
	};

	enum PPUState
	{
		HBlank,
		VBlank,
		OAMSearch,
		DrawScanline
	};

	enum LCDControl
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

	enum SpriteAttrib
	{
		Priority = 0x80,
		FlipY = 0x40,
		FlipX = 0x20,
		Palette = 0x10
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
		bool window_draw_flag = false;
		uint32_t cycles = 0;
		PPUState mode = PPUState::HBlank;
		std::array<uint8_t, 8193> vram{}; // 8000h - 9fffh
		std::array<uint8_t, 256> oam{};	  // fe00h - fe9fh
		std::array<Object, 10> objects_on_scanline{};
		std::array<uint8_t, 160 * 144 * 3> framebuffer{};
		std::array<uint8_t, 160 * 144> framebuffer_bg_color{};
		uint8_t num_obj_on_scanline = 0;
		bool previously_disabled = false;
		Core &core;

	public:
		uint8_t lcd_control = 0;
		uint8_t status = 0;
		uint8_t screen_scroll_y = 0, screen_scroll_x = 0;
		uint8_t line_y = 0;
		uint8_t line_y_compare = 0;
		uint8_t window_y = 0, window_x = 0;
		uint8_t background_palette = 0;
		uint8_t object_palette_0 = 0, object_palette_1 = 0;
		uint8_t window_line_y = 0;

		PPU(Core &core);

		void reset(bool hard_reset);
		void step(uint32_t accumulated_cycles);

		const std::array<uint8_t, 160 * 144 * 3> &get_framebuffer() const;

		void write_vram(uint16_t address, uint8_t value);
		void write_oam(uint16_t address, uint8_t value);
		void instant_dma(uint8_t address);

		uint8_t read_vram(uint16_t address) const;
		uint8_t read_oam(uint16_t address) const;
		bool check_stat(uint8_t flags) const;
		void set_stat(uint8_t flags, bool value);
		bool stat_any() const;
		bool check_lcdc(uint8_t flags) const;

	private:
		void render_scanline();
		void render_bg_layer();
		void render_window_layer();
		void scan_oam();
		void render_sprite_layer();

		bool get_sprite_attrib(const Object &spr, uint8_t attrib);
		void check_ly_lyc(bool allow_interrupts);

		Color palette_index_to_color(uint8_t palette, uint8_t bitIndex) const;
	};
}