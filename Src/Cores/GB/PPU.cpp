/*
    Big ComBoy
    Copyright (C) 2023-2024 UltimaOmega474

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "PPU.hpp"
#include "Bus.hpp"
#include <algorithm>
#include <span>

namespace GB
{
    uint8_t BackgroundFIFO::pixels_left() const { return shift_count; }

    void BackgroundFIFO::clear()
    {
        shift_count = 0;
        pixels_low = 0;
        pixels_high = 0;
        attribute = 0;
    }

    void BackgroundFIFO::load(uint8_t low, uint8_t high, uint8_t attribute)
    {
        pixels_low = low;
        pixels_high = high;
        shift_count = 8;
        this->attribute = attribute;
    }

    void BackgroundFIFO::force_shift(uint8_t amount)
    {
        pixels_low <<= amount;
        pixels_high <<= amount;
        shift_count -= amount;
    }

    uint8_t BackgroundFIFO::clock()
    {
        uint8_t low_bit = (pixels_low >> 7) & 0x1;
        uint8_t high_bit = (pixels_high >> 7) & 0x1;
        uint8_t pixel = (high_bit << 1) | (low_bit);

        pixels_low <<= 1;
        pixels_high <<= 1;

        shift_count--;
        return pixel;
    }

    FetchState BackgroundFetcher::get_state() const { return state; }

    FetchMode BackgroundFetcher::get_mode() const { return mode; }

    void BackgroundFetcher::reset()
    {
        first_fetch = true;
        clear_with_mode(FetchMode::Background);
    }

    void BackgroundFetcher::clear_with_mode(FetchMode new_mode)
    {
        substep = 0;
        tile_id = 0;
        attribute_id = 0;
        queued_pixels_low = 0;
        queued_pixels_high = 0;
        x_pos = 0;
        address = 0;
        state = FetchState::GetTileID;
        mode = new_mode;
    }

    void BackgroundFetcher::clock(PPU &ppu)
    {
        switch (state)
        {
        case FetchState::GetTileID:
        {
            get_tile_id(ppu);
            break;
        }
        case FetchState::TileLow:
        {
            get_tile_data(ppu, 0);
            break;
        }
        case FetchState::TileHigh:
        {
            get_tile_data(ppu, 1);
            break;
        }
        case FetchState::Push:
        {
            push_pixels(ppu);
            break;
        }
        }
    }

    void BackgroundFetcher::get_tile_id(PPU &ppu)
    {
        switch (substep)
        {
        case 0:
        {
            uint16_t computed_address = 0b10011 << 11;

            switch (mode)
            {
            case FetchMode::Background:
            {
                computed_address |= ((ppu.lcd_control & LCDControlFlags::BGTileMap) ? 1 : 0) << 10;

                uint16_t xoffset = ((x_pos / 8) + (ppu.screen_scroll_x / 8)) & 31;
                uint16_t yoffset = ((ppu.line_y + ppu.screen_scroll_y) / 8) & 31;
                xoffset &= 0x3FF;
                yoffset &= 0x3FF;

                computed_address |= xoffset;
                computed_address |= (yoffset << 5);

                break;
            }
            case FetchMode::Window:
            {
                computed_address |= ((ppu.lcd_control & LCDControlFlags::WindowTileMap) ? 1 : 0)
                                    << 10;

                uint16_t xoffset = ((x_pos) / 8) & 31;
                uint16_t yoffset = ((ppu.window_line_y) / 8) & 31;
                xoffset &= 0x3FF;
                yoffset &= 0x3FF;

                computed_address |= xoffset;
                computed_address |= (yoffset << 5);

                break;
            }
            }

            address = computed_address & 0x1FFF;
            substep++;
            break;
        }

        case 1:
        {
            tile_id = ppu.vram[address];
            attribute_id = ppu.vram[0x2000 + address];
            state = FetchState::TileLow;

            substep = 0;
            break;
        }
        }
    }

    void BackgroundFetcher::get_tile_data(PPU &ppu, uint8_t bit_plane)
    {
        switch (substep)
        {
        case 0:
        {
            uint16_t computed_address = (0b1 << 15) + bit_plane;

            switch (mode)
            {
            case FetchMode::Background:
            {
                uint16_t bit12 =
                    !((ppu.lcd_control & LCDControlFlags::BGWindowTileData) || (tile_id & 0x80));
                uint16_t yoffset = (ppu.line_y + ppu.screen_scroll_y) & 7;

                computed_address |= bit12 ? (1 << 12) : 0;
                computed_address |= tile_id << 4;
                computed_address |= (yoffset << 1);

                break;
            }
            case FetchMode::Window:
            {
                uint16_t bit12 =
                    !((ppu.lcd_control & LCDControlFlags::BGWindowTileData) || (tile_id & 0x80));
                uint16_t yoffset = ppu.window_line_y & 7;

                computed_address |= bit12 ? (1 << 12) : 0;
                computed_address |= tile_id << 4;
                computed_address |= (yoffset << 1);

                break;
            }
            }

            address = computed_address & 0x1FFF;
            substep++;
            break;
        }

        // read from address
        case 1:
        {
            substep = 0;

            if (bit_plane == 1)
            {
                state = FetchState::Push;

                auto bank = (attribute_id & 0x8) >> 3;
                queued_pixels_high = ppu.vram[(0x2000 * bank) + address];

                if (first_fetch)
                {
                    state = FetchState::GetTileID;
                    first_fetch = false;
                }
            }
            else
            {
                state = FetchState::TileHigh;
                auto bank = (attribute_id & 0x8) >> 3;
                queued_pixels_low = ppu.vram[(0x2000 * bank) + address];
            }

            break;
        }
        }
    }

    void BackgroundFetcher::push_pixels(PPU &ppu)
    {
        if (ppu.bg_fifo.pixels_left())
            return;

        x_pos += 8;
        ppu.bg_fifo.load(queued_pixels_low, queued_pixels_high, attribute_id);

        if (mode == FetchMode::Background)
        {
            if (ppu.line_x == 0)
            {
                auto scx = ppu.screen_scroll_x & 7;

                ppu.bg_fifo.force_shift(scx);
                ppu.extra_cycles += scx;
            }
        }

        substep = 0;
        state = FetchState::GetTileID;
    }

    PPU::PPU(MainBus &bus) : bus(bus) {}

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
            framebuffer_complete.fill(0);
            bg_color_table.fill(0);
            objects_on_scanline.fill({});
            mode = PPUState::HBlank;
        }
        window_draw_flag = false;
        num_obj_on_scanline = 0;
        cycles = 0;
        line_y = 0;
        window_line_y = 0;

        fetcher.reset();
        bg_fifo.clear();
    }

    void PPU::set_post_boot_state()
    {
        previously_disabled = false;
        status = 1;
        lcd_control = 0x91;
    }

    void PPU::step(uint32_t accumulated_cycles)
    {
        if (!(lcd_control & LCDControlFlags::DisplayEnable))
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

        while (accumulated_cycles)
        {
            bool allow_interrupt = stat_any() ? false : true;

            if (window_y == line_y)
                window_draw_flag = true;

            switch (mode)
            {

            case PPUState::HBlank:
            {
                if (cycles == (204 - extra_cycles))
                {
                    cycles = 0;
                    ++line_y;

                    line_x = 0;

                    if (line_y == 144)
                    {
                        mode = PPUState::VBlank;

                        bus.request_interrupt(INT_VBLANK_BIT);
                        if (check_stat(EnableVBlankInt) && allow_interrupt)
                            bus.request_interrupt(INT_LCD_STAT_BIT);
                    }
                    else
                    {

                        mode = PPUState::OAMSearch;

                        if (check_stat(EnableOAMInt) && allow_interrupt)
                            bus.request_interrupt(INT_LCD_STAT_BIT);
                    }

                    continue;
                }
                break;
            }

            case PPUState::VBlank:
            {
                if (cycles == 456)
                {
                    ++line_y;
                    cycles = 0;

                    if (line_y > 153)
                    {
                        framebuffer_complete = framebuffer;
                        mode = PPUState::OAMSearch;

                        if (check_stat(EnableOAMInt) && allow_interrupt)
                            bus.request_interrupt(INT_LCD_STAT_BIT);

                        line_y = 0;
                        window_line_y = 0;
                        window_draw_flag = false;

                        continue;
                    }
                }
                break;
            }

            case PPUState::OAMSearch:
            {
                if (cycles == 80)
                {
                    scan_oam();
                    cycles = 0;
                    extra_cycles = 0;

                    mode = PPUState::DrawScanline;
                    fetcher.reset();
                    bg_fifo.clear();
                    continue;
                }

                break;
            }

            case PPUState::DrawScanline:
            {
                if (cycles == 172 + extra_cycles)
                {
                    render_objects();
                    cycles = 0;

                    mode = PPUState::HBlank;

                    if (fetcher.get_mode() == FetchMode::Window)
                        window_line_y++;

                    if (check_stat(EnableHBlankInt) && allow_interrupt)
                        bus.request_interrupt(INT_LCD_STAT_BIT);

                    continue;
                }
                else
                {
                    render_scanline();
                }
                break;
            }
            }

            accumulated_cycles--;
            cycles++;

            check_ly_lyc(allow_interrupt);

            set_stat(ModeFlag, false);
            status |= mode & 0x03;
        }
    }

    void PPU::write_vram(uint16_t address, uint8_t value)
    {
        vram[(vram_bank_select * 0x2000) + address] = value;
    }

    void PPU::write_bg_palette(uint8_t value)
    {
        bg_cram[bg_palette_select & 0x3F] = value;

        if (bg_palette_select & 0x80)
        {
            bg_palette_select = ((bg_palette_select + 1) & 0x3F) | 0x80;
        }
    }

    uint8_t PPU::read_bg_palette() const { return bg_cram[bg_palette_select & 0x3F]; }

    void PPU::write_obj_palette(uint8_t value)
    {
        obj_cram[obj_palette_select & 0x3F] = value;

        if (obj_palette_select & 0x80)
        {
            obj_palette_select = ((obj_palette_select + 1) & 0x3F) | 0x80;
        }
    }

    uint8_t PPU::read_obj_palette() const { return obj_cram[obj_palette_select & 0x3F]; }

    void PPU::write_oam(uint16_t address, uint8_t value) { oam[address] = value; }

    void PPU::instant_dma(uint8_t address)
    {
        uint16_t addr = address << 8;
        for (auto i = 0; i < 160; ++i)
            oam[i] = bus.read((addr) + i);
    }

    void PPU::instant_hdma(uint8_t length)
    {
        length &= ~0x80;
        for (int i = 0; i < length; ++i)
        {
            uint8_t data = bus.read(HDMA_src + i);

            auto vram_bank = (vram_bank_select * 0x2000);
            vram[vram_bank + (HDMA_dst & 0x1FFF) + i] = data;
        }
    }

    uint8_t PPU::hdma_blocks_remain() const { return 0xFF; }

    uint8_t PPU::read_vram(uint16_t address) const
    {
        return vram[(vram_bank_select * 0x2000) + address];
    }

    uint8_t PPU::read_oam(uint16_t address) const { return oam[address]; }

    bool PPU::check_stat(uint8_t flags) const { return status & flags; }

    void PPU::set_stat(uint8_t flags, bool value)
    {
        if (value)
            status |= flags;
        else
            status &= ~flags;
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

    void PPU::render_scanline()
    {
        fetcher.clock(*this);

        if ((line_x < 160) && bg_fifo.pixels_left())
        {
            uint8_t final_pixel = 0, final_palette = bg_fifo.attribute & 0x7;
            uint8_t bg_pixel = bg_fifo.clock();

            if (!(lcd_control & LCDControlFlags::BGEnable) ||
                !(render_flags & RenderFlags::Background))
            {
                final_pixel = 0;
                final_palette = 0;
            }
            else
            {
                final_pixel = bg_pixel;
            }

            bg_color_table[(line_y * LCD_WIDTH) + line_x] = final_pixel;
            plot_cgb_pixel(line_x, final_pixel, final_palette, false);
            line_x++;
        }

        if ((lcd_control & LCDControlFlags::WindowEnable) && window_draw_flag &&
            (line_x >= (window_x - 7)) && fetcher.get_mode() == FetchMode::Background)
        {
            fetcher.clear_with_mode(FetchMode::Window);
            bg_fifo.clear();
            extra_cycles += 6;
        }
    }

    void PPU::render_objects()
    {
        if (!(lcd_control & LCDControlFlags::SpriteEnable) ||
            !(render_flags & RenderFlags::Objects))
            return;

        uint8_t height = (lcd_control & LCDControlFlags::SpriteSize) ? 16 : 8;

        for (auto i = 0; i < num_obj_on_scanline; ++i)
        {
            auto &object = objects_on_scanline[(num_obj_on_scanline - 1) - i];

            uint8_t palette = (object.attributes & OBJAttributeFlags::Palette) ? object_palette_1
                                                                               : object_palette_0;
            uint8_t cgb_palette = (object.attributes & OBJAttributeFlags::PaletteBits);
            uint8_t bank = 0x2000 * ((object.attributes & OBJAttributeFlags::BankSelect) >> 3);

            uint16_t tile_index = 0;

            if (height == 16)
                object.tile &= ~1;

            int32_t obj_y = static_cast<int32_t>(object.y) - 16;

            if (object.attributes & OBJAttributeFlags::FlipY)
                tile_index =
                    (0x8000 & 0x1FFF) + (object.tile * 16) + ((height - (line_y - obj_y) - 1) * 2);
            else
                tile_index =
                    (0x8000 & 0x1FFF) + (object.tile * 16) + ((line_y - obj_y) % height * 2);

            int32_t adjusted_x = static_cast<int32_t>(object.x) - 8;
            size_t framebuffer_line_y = line_y * LCD_WIDTH;

            for (auto x = 0; x < 8; ++x)
            {
                size_t framebuffer_line_x = adjusted_x + x;

                if (framebuffer_line_x >= 0 && framebuffer_line_x < 160)
                {
                    uint8_t low_byte = vram[bank + tile_index];
                    uint8_t high_byte = vram[bank + (tile_index + 1)];
                    uint8_t bit = 7 - (x & 7);

                    if (object.attributes & OBJAttributeFlags::FlipX)
                        bit = x & 7;

                    uint8_t low_bit = (low_byte >> bit) & 0x01;
                    uint8_t high_bit = (high_byte >> bit) & 0x01;
                    uint8_t pixel = (high_bit << 1) | low_bit;

                    if (pixel == 0)
                        continue;

                    bool bg_has_priority = (object.attributes & OBJAttributeFlags::Priority) &&
                                           bg_color_table[framebuffer_line_y + framebuffer_line_x];

                    if (!bg_has_priority)
                        plot_cgb_pixel(framebuffer_line_x, pixel, cgb_palette, true);
                }
            }
        }
    }

    void PPU::plot_pixel(uint8_t x_pos, uint8_t final_pixel, uint8_t palette)
    {
        size_t framebuffer_line_y = line_y * LCD_WIDTH;

        std::array<uint8_t, 4> color = color_table[(palette >> (int)(2 * final_pixel)) & 3];

        auto fb_pixel =
            std::span<uint8_t>{&framebuffer[(framebuffer_line_y + x_pos) * COLOR_DEPTH], 4};

        std::copy(color.begin(), color.end(), fb_pixel.begin());
    }

    void PPU::plot_cgb_pixel(uint8_t x_pos, uint8_t final_pixel, uint8_t palette, bool is_obj)
    {
        size_t framebuffer_line_y = line_y * LCD_WIDTH;
        size_t select_color = (palette * 8) + (final_pixel * 2);

        uint16_t color = 0;

        if (is_obj)
        {
            color = obj_cram[select_color];
            color |= obj_cram[select_color + 1] << 8;
        }
        else
        {
            color = bg_cram[select_color];
            color |= bg_cram[select_color + 1] << 8;
        }

        auto fb_pixel =
            std::span<uint8_t>{&framebuffer[(framebuffer_line_y + x_pos) * COLOR_DEPTH], 4};

        auto r = color & 0x1F;
        auto g = (color >> 5) & 0x1F;
        auto b = (color >> 10) & 0x1F;

        fb_pixel[0] = (r * 255) / 31;
        fb_pixel[1] = (g * 255) / 31;
        fb_pixel[2] = (b * 255) / 31;
        fb_pixel[3] = 255;
    }

    void PPU::scan_oam()
    {
        uint8_t height = (lcd_control & LCDControlFlags::SpriteSize) ? 16 : 8;

        num_obj_on_scanline = 0;
        objects_on_scanline.fill({});
        for (auto i = 0, total = 0; i < 40; ++i)
        {
            if (total == 10)
                break;

            const Object *sprite = reinterpret_cast<const Object *>((&oam[i * 4]));
            int32_t corrected_y_position = static_cast<int32_t>(sprite->y) - 16;

            if (corrected_y_position <= line_y && (corrected_y_position + height) > line_y)
            {
                objects_on_scanline[total] = *sprite;
                total++;
                num_obj_on_scanline++;
            }
        }

        std::stable_sort(
            objects_on_scanline.begin(), objects_on_scanline.begin() + num_obj_on_scanline,
            [=](const Object &left, const Object &right) { return (left.x) < (right.x); });
    }

    void PPU::check_ly_lyc(bool allow_interrupts)
    {
        set_stat(LYCandLYCompareType, false);
        if (line_y == line_y_compare)
        {
            set_stat(LYCandLYCompareType, true);
            if (check_stat(EnableLYCandLYInt) && allow_interrupts)
                bus.request_interrupt(INT_LCD_STAT_BIT);
        }
    }
}