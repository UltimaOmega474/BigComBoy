#include "ppu.hpp"
#include "core.hpp"
#include "constants.hpp"
#include <algorithm>

namespace Angbe
{
	PPU::PPU(Core &memory)
		: core(memory)
	{
	}

	void PPU::reset(bool hard_reset)
	{

		if (hard_reset)
		{
			status = 0;
			lcd_control = 0;
			background_palette = 0;
			object_palette_0 = 0;
			object_palette_1 = 0;
			screen_scroll_y = 0;
			screen_scroll_x = 0;
			window_y = 0;
			window_x = 0;
			line_y_compare = 0;
			vram.fill(0);
			oam.fill(0);
			framebuffer.fill(0);
			bg_color_table.fill(0);
			objects_on_scanline.fill({});
			mode = PPUState::HBlank;
		}
		window_draw_flag = false;
		num_obj_on_scanline = 0;
		cycles = 0;
		line_y = 0;

		window_line_y = 0;
	}

	void PPU::set_post_boot_state()
	{
		window_draw_flag = true;
		previously_disabled = false;
		cycles = 420;
		status = 1;
		lcd_control = 0x91;
	}

	void PPU::step(uint32_t accumulated_cycles)
	{
		if (!check_lcdc(DisplayEnable))
		{
			mode = HBlank;
			set_stat(ModeFlag, false);
			status |= mode & 0x03;
			previously_disabled = true;
			return;
		}

		if (previously_disabled)
		{
			reset(false);
			previously_disabled = false;
		}

		cycles += accumulated_cycles;
		bool allow_interrupt = stat_any() ? false : true;

		switch (mode)
		{

		case PPUState::HBlank:
		{
			if (cycles >= 204)
			{
				cycles -= 204;
				++line_y;

				if (line_y == 144)
				{
					mode = PPUState::VBlank;

					core.request_interrupt(INT_VBLANK_BIT);
					if (check_stat(EnableVBlankInt) && allow_interrupt)
						core.request_interrupt(INT_LCD_STAT_BIT);
				}
				else
				{

					mode = PPUState::OAMSearch;

					if (check_stat(EnableOAMInt) && allow_interrupt)
						core.request_interrupt(INT_LCD_STAT_BIT);
				}
			}
			break;
		}

		case PPUState::VBlank:
		{
			if (cycles >= 456)
			{
				++line_y;
				cycles -= 456;

				if (line_y > 153)
				{

					mode = PPUState::OAMSearch;
					if (check_stat(EnableOAMInt) && allow_interrupt)
						core.request_interrupt(INT_LCD_STAT_BIT);

					line_y = 0;
					window_line_y = 0;
					window_draw_flag = false;
				}
			}
			break;
		}

		case PPUState::OAMSearch:
		{
			if (cycles >= 80)
			{
				scan_oam();
				cycles -= 80;
				mode = PPUState::DrawScanline;
				break;
			}

			break;
		}

		case PPUState::DrawScanline:
		{
			if (cycles >= 172)
			{
				cycles -= 172;
				mode = PPUState::HBlank;

				if (check_stat(EnableHBlankInt) && allow_interrupt)
					core.request_interrupt(INT_LCD_STAT_BIT);

				render_scanline();
			}
			break;
		}
		}

		check_ly_lyc(allow_interrupt);

		if (window_y == line_y)
			window_draw_flag = true;

		set_stat(ModeFlag, false);
		status |= mode & 0x03;
	}

	void PPU::write_vram(uint16_t address, uint8_t value)
	{
		vram[address] = value;
	}

	void PPU::write_oam(uint16_t address, uint8_t value)
	{
		oam[address] = value;
	}

	void PPU::instant_dma(uint8_t address)
	{
		uint16_t addr = address << 8;
		for (auto i = 0; i < 160; ++i)
		{
			oam[i] = core.read_no_tick((addr) + i);
		}
	}

	uint8_t PPU::read_vram(uint16_t address) const
	{
		return vram[address];
	}

	uint8_t PPU::read_oam(uint16_t address) const
	{
		return oam[address];
	}

	bool PPU::check_stat(uint8_t flags) const
	{
		return status & flags;
	}

	void PPU::set_stat(uint8_t flags, bool value)
	{
		if (value)
		{
			status |= flags;
		}
		else
		{
			status &= ~flags;
		}
	}

	bool PPU::stat_any() const
	{
		if (check_stat(EnableLYCandLYInt))
		{
			if (check_stat(LYCandLYCompareType))
				return true;
		}

		if (check_stat(EnableOAMInt))
		{
			if (mode == 2)
				return true;
		}

		if (check_stat(EnableVBlankInt))
		{
			if (mode == 1)
				return true;
		}

		if (check_stat(EnableHBlankInt))
		{
			if (mode == 0)
				return true;
		}

		return false;
	}

	bool PPU::check_lcdc(uint8_t flags) const
	{
		return lcd_control & flags;
	}

	void PPU::render_scanline()
	{
		render_bg_layer();
		render_window_layer();
		render_sprite_layer();
	}

	void PPU::render_bg_layer()
	{
		uint16_t tile_map_address = check_lcdc(BGTileMap) ? 0x9c00 : 0x9800;
		uint16_t tile_data_address = check_lcdc(BGWindowTileData) ? 0x8000 : 0x8800;

		for (uint8_t i = 0; i < 160; ++i)
		{
			uint8_t x_offset = screen_scroll_x + i;
			uint8_t y_offset = screen_scroll_y + line_y;
			uint8_t tile_id = vram[(tile_map_address & 0x1FFF) + ((x_offset / 8) & 31) + (((y_offset / 8) & 31) * 32)];
			uint16_t tile_index = 0;

			if (tile_data_address == 0x8800)
			{
				tile_index = ((tile_data_address + 0x800) & 0x1FFF) + (((int8_t)tile_id) * 16) + (y_offset % 8 * 2);
			}
			else
			{
				tile_index = (tile_data_address & 0x1FFF) + (tile_id * 16) + (y_offset % 8 * 2);
			}

			uint8_t low_byte = vram[tile_index];
			uint8_t high_byte = vram[tile_index + 1];
			uint8_t bit = 7 - (x_offset % 8);
			uint8_t low_bit = (low_byte >> bit) & 0x01;
			uint8_t high_bit = (high_byte >> bit) & 0x01;
			uint8_t pixel = (high_bit << 1) | low_bit;

			size_t framebuffer_line_x = i;
			size_t framebuffer_line_y = (line_y)*LCD_WIDTH;

			uint32_t pixel_color;
			if (!check_lcdc(BGEnable))
			{
				pixel_color = palette_index_to_color(background_palette, 0);
				bg_color_table[(line_y * 160) + i] = 0;
			}
			else
			{
				pixel_color = palette_index_to_color(background_palette, pixel);
				bg_color_table[(line_y * 160) + i] = pixel;
			}

			framebuffer[framebuffer_line_y + framebuffer_line_x] = pixel_color;
		}
	}

	void PPU::render_window_layer()
	{
		uint16_t tile_map_address = check_lcdc(WindowTileMap) ? 0x9c00 : 0x9800;
		uint16_t tile_data_address = check_lcdc(BGWindowTileData) ? 0x8000 : 0x8800;
		uint8_t advance_by = 0;
		if (check_lcdc(WindowEnable) && window_draw_flag)
			for (uint8_t i = 0; i < 160; ++i)
			{
				if (line_y >= window_y && i >= (window_x - 7))
				{
					uint8_t x_offset = (i - (window_x - 7));
					uint8_t y_offset = window_line_y;
					uint8_t tile_id = vram[(tile_map_address & 0x1FFF) + ((x_offset / 8) & 31) + (((y_offset / 8) & 31) * 32)];
					uint16_t tile_index = 0;

					if (tile_data_address == 0x8800)
					{
						tile_index = ((tile_data_address + 0x800) & 0x1FFF) + (((int8_t)tile_id) * 16) + (y_offset % 8 * 2);
					}
					else
					{
						tile_index = (tile_data_address & 0x1FFF) + (tile_id * 16) + (y_offset % 8 * 2);
					}

					uint8_t low_byte = vram[tile_index];
					uint8_t high_byte = vram[tile_index + 1];
					uint8_t bit = 7 - (x_offset % 8);
					uint8_t low_bit = (low_byte >> bit) & 0x01;
					uint8_t high_bit = (high_byte >> bit) & 0x01;
					uint8_t pixel = (high_bit << 1) | low_bit;

					size_t framebuffer_line_x = i;
					size_t framebuffer_line_y = (line_y)*LCD_WIDTH;
					advance_by = 1;

					bg_color_table[(line_y * 160) + i] = pixel;

					framebuffer[framebuffer_line_y + framebuffer_line_x] = palette_index_to_color(background_palette, pixel);
				}
			}

		window_line_y += advance_by;
	}

	void PPU::scan_oam()
	{
		uint8_t height = check_lcdc(LCDControl::SpriteSize) ? 16 : 8;

		num_obj_on_scanline = 0;
		objects_on_scanline.fill({});

		for (auto i = 0, total = 0; i < 40; ++i)
		{
			if (total == 10)
				break;

			const Object *sprite = reinterpret_cast<const Object *>((&oam[i * 4]));
			uint8_t corrected_y_position = sprite->y - 16;

			if (corrected_y_position <= line_y && (corrected_y_position + height) > line_y)
			{
				objects_on_scanline[total] = *sprite;
				total++;
				num_obj_on_scanline++;
			}
		}

		std::stable_sort(objects_on_scanline.begin(), objects_on_scanline.begin() + num_obj_on_scanline,
						 [=](const Object &left, const Object &right)
						 {
							 return (left.x) < (right.x);
						 });
	}

	void PPU::render_sprite_layer()
	{
		if (!check_lcdc(SpriteEnable))
			return;

		uint8_t height = check_lcdc(LCDControl::SpriteSize) ? 16 : 8;

		if (check_lcdc(SpriteEnable))
		{
			for (auto i = 0; i < num_obj_on_scanline; ++i)
			{
				auto &sprite = objects_on_scanline[(num_obj_on_scanline - 1) - i];
				sprite.y -= 16;
				sprite.x -= 8;

				uint8_t palette = get_sprite_attrib(sprite, Palette) ? object_palette_1 : object_palette_0;
				uint16_t tile_index = 0;

				if (height == 16)
				{
					sprite.tile &= ~1;
				}

				if (get_sprite_attrib(sprite, FlipY))
				{
					tile_index = (0x8000 & 0x1FFF) + (sprite.tile * 16) + ((height - (line_y - sprite.y) - 1) * 2);
				}
				else
				{
					tile_index = (0x8000 & 0x1FFF) + (sprite.tile * 16) + ((line_y - sprite.y) % height * 2);
				}

				for (auto x = 0; x < 8; ++x)
				{
					size_t framebuffer_line_x = (sprite.x + x);
					size_t framebuffer_line_y = (line_y)*LCD_WIDTH;

					if ((sprite.x + x) >= 0 && (sprite.x + x) < 160)
					{
						uint8_t low_byte = vram[tile_index];
						uint8_t high_byte = vram[tile_index + 1];
						uint8_t bit = 7 - (x % 8);

						if (get_sprite_attrib(sprite, FlipX))
							bit = x % 8;

						uint8_t low_bit = (low_byte >> bit) & 0x01;
						uint8_t high_bit = (high_byte >> bit) & 0x01;
						uint8_t pixel = (high_bit << 1) | low_bit;

						if (pixel == 0)
							continue;

						if (!get_sprite_attrib(sprite, Priority))
						{
							framebuffer[framebuffer_line_y + framebuffer_line_x] = palette_index_to_color(palette, pixel);
						}
						else
						{
							if (bg_color_table[(line_y * 160) + (sprite.x + x)] == 0)
							{
								framebuffer[framebuffer_line_y + framebuffer_line_x] = palette_index_to_color(palette, pixel);
							}
						}
					}
				}
			}
		}
	}

	bool PPU::get_sprite_attrib(const Object &spr, uint8_t attrib)
	{
		return spr.attributes & attrib;
	}

	void PPU::check_ly_lyc(bool allow_interrupts)
	{
		set_stat(LYCandLYCompareType, false);
		if (line_y == line_y_compare)
		{
			set_stat(LYCandLYCompareType, true);
			if (check_stat(EnableLYCandLYInt) && allow_interrupts)
			{
				core.request_interrupt(INT_LCD_STAT_BIT);
			}
		}
	}

	uint32_t PPU::palette_index_to_color(uint8_t palette, uint8_t bitIndex) const
	{
		switch (bitIndex)
		{
		case 0:
			return color_table[palette & 0x3];
		case 1:
			return color_table[(palette >> 2) & 0x3];
		case 2:
			return color_table[(palette >> 4) & 0x3];
		case 3:
			return color_table[(palette >> 6) & 0x3];

		default:
			// in case of a screw up, turn the pixel red
			return 0xFF0000FF;
		}
	}
}