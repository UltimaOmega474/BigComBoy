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
    }

    void PPU::set_post_boot_state()
    {
        //   window_draw_flag = true;
        previously_disabled = false;
        //    cycles = 420;
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
                if (cycles == (204 - penalty))
                {
                    cycles = 0;
                    ++line_y;

                    write_x = 0;

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

                    mode = PPUState::DrawScanline;
                    bg_fifo.pixels_high = 0;
                    bg_fifo.pixels_high = 0;
                    bg_fifo.shift_count = 0;
                    penalty = 0;
                    first_fetch = true;
                    should_discard = true;
                    fetcher.mode = FetchMode::Background;
                    fetcher.state = FetchState::ID;
                    fetcher.substep = 0;
                    fetcher.x_pos = 0;
                    continue;
                }

                break;
            }

            case PPUState::DrawScanline:
            {
                if (cycles == 172 + penalty)
                {
                    cycles = 0;

                    mode = PPUState::HBlank;

                    if (fetcher.mode == FetchMode::Window)
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

    void PPU::write_vram(uint16_t address, uint8_t value) { vram[address] = value; }

    void PPU::write_oam(uint16_t address, uint8_t value) { oam[address] = value; }

    void PPU::instant_dma(uint8_t address)
    {
        uint16_t addr = address << 8;
        for (auto i = 0; i < 160; ++i)
        {
            oam[i] = bus.read((addr) + i);
        }
    }

    uint8_t PPU::read_vram(uint16_t address) const { return vram[address]; }

    uint8_t PPU::read_oam(uint16_t address) const { return oam[address]; }

    bool PPU::check_stat(uint8_t flags) const { return status & flags; }

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

    void PPU::render_scanline()
    {
        switch (fetcher.state)
        {
        case FetchState::Idle:
        {
            fetcher.state = FetchState::ID;
            return;
        }
        case FetchState::ID:
        {
            get_tile_id();
            break;
        }
        case FetchState::TileLow:
        {
            get_tile_data(0);
            break;
        }
        case FetchState::TileHigh:
        {
            get_tile_data(1);
            break;
        }
        case FetchState::Push:
        {
            push_pixels();
            break;
        }
        }

        if ((write_x < 160) && (bg_fifo.shift_count > 0))
        {
            clock_fifo();
            bg_fifo.shift_count--;
            write_x++;
        }

        if ((lcd_control & LCDControlFlags::WindowEnable) && window_draw_flag &&
            (write_x >= (window_x - 7)) && fetcher.mode == FetchMode::Background)
        {
            fetcher.x_pos = 0;
            fetcher.mode = FetchMode::Window;
            fetcher.state = FetchState::ID;
            fetcher.substep = 0;
            bg_fifo.pixels_high = 0;
            bg_fifo.pixels_low = 0;
            bg_fifo.shift_count = 0;
            penalty += 6;
        }
    }

    void PPU::get_tile_id()
    {
        switch (fetcher.substep)
        {
        case 0:
        {
            uint16_t address = 0b10011 << 11;

            switch (fetcher.mode)
            {
            case FetchMode::Background:
            {
                address |= ((lcd_control & LCDControlFlags::BGTileMap) ? 1 : 0) << 10;

                uint16_t xoffset = ((fetcher.x_pos / 8) + (screen_scroll_x / 8)) & 31;
                uint16_t yoffset = ((line_y + screen_scroll_y) / 8) & 31;
                xoffset &= 0x3FF;
                yoffset &= 0x3FF;

                address |= xoffset;
                address |= (yoffset << 5);

                break;
            }
            case FetchMode::Window:
            {
                address |= ((lcd_control & LCDControlFlags::WindowTileMap) ? 1 : 0) << 10;

                uint16_t xoffset = ((fetcher.x_pos) / 8) & 31;
                uint16_t yoffset = ((window_line_y) / 8) & 31;
                xoffset &= 0x3FF;
                yoffset &= 0x3FF;

                address |= xoffset;
                address |= (yoffset << 5);

                break;
            }
            default:
            {
                return;
            }
            }

            fetcher.address = address & 0x1FFF;
            fetcher.substep++;
            break;
        }

        case 1:
        {
            fetcher.tile_id = vram[fetcher.address];
            fetcher.state = FetchState::TileLow;

            fetcher.substep = 0;
            break;
        }
        }
    }

    void PPU::get_tile_data(uint8_t bit_plane)
    {
        switch (fetcher.substep)
        {
        // compute address
        case 0:
        {
            uint16_t address = (0b1 << 15) + bit_plane;

            switch (fetcher.mode)
            {
            case FetchMode::Background:
            {
                uint16_t bit12 = !((lcd_control & LCDControlFlags::BGWindowTileData) ||
                                   (fetcher.tile_id & 0x80));
                uint16_t yoffset = (line_y + screen_scroll_y) & 7;

                address |= bit12 ? (1 << 12) : 0;
                address |= fetcher.tile_id << 4;
                address |= (yoffset << 1);

                break;
            }
            case FetchMode::Window:
            {
                uint16_t bit12 = !((lcd_control & LCDControlFlags::BGWindowTileData) ||
                                   (fetcher.tile_id & 0x80));
                uint16_t yoffset = window_line_y & 7;

                address |= bit12 ? (1 << 12) : 0;
                address |= fetcher.tile_id << 4;
                address |= (yoffset << 1);

                break;
            }
            default:
            {
                return;
            }
            }

            fetcher.address = address & 0x1FFF;
            fetcher.substep++;
            break;
        }

        // read from address
        case 1:
        {
            fetcher.substep = 0;

            if (bit_plane == 1)
            {
                fetcher.state = FetchState::Push;
                fetcher.queued_pixels_high = vram[fetcher.address];

                if (first_fetch)
                {
                    fetcher.state = FetchState::ID;
                    first_fetch = false;
                }
            }
            else
            {
                fetcher.state = FetchState::TileHigh;
                fetcher.queued_pixels_low = vram[fetcher.address];
            }

            break;
        }
        }
    }

    void PPU::push_pixels()
    {
        switch (fetcher.mode)
        {
        case FetchMode::Background:
        {
            if (bg_fifo.shift_count)
            {
                return;
            }
            fetcher.x_pos += 8;
            bg_fifo.pixels_low = fetcher.queued_pixels_low;
            bg_fifo.pixels_high = fetcher.queued_pixels_high;
            bg_fifo.shift_count = 8;

            if (should_discard)
            {
                should_discard = false;
                auto scx = screen_scroll_x & 7;

                bg_fifo.pixels_low <<= scx;
                bg_fifo.pixels_high <<= scx;
                bg_fifo.shift_count -= scx;
                penalty += scx;
            }

            break;
        }
        case FetchMode::Window:
        {
            if (bg_fifo.shift_count)
            {
                return;
            }
            fetcher.x_pos += 8;
            bg_fifo.pixels_low = fetcher.queued_pixels_low;
            bg_fifo.pixels_high = fetcher.queued_pixels_high;
            bg_fifo.shift_count = 8;
            break;
        }
        case FetchMode::OBJ:
        {

            break;
        }
        }

        fetcher.state = FetchState::ID;
        fetcher.substep = 0;
        fetcher.address = 0;
        fetcher.tile_id = 0;
        fetcher.queued_pixels_low = 0;
        fetcher.queued_pixels_high = 0;
    }

    void PPU::clock_fifo()
    {
        uint8_t low_bit = (bg_fifo.pixels_low >> 7) & 0x1;
        uint8_t high_bit = (bg_fifo.pixels_high >> 7) & 0x1;
        uint8_t pixel = (high_bit << 1) | (low_bit);

        bg_fifo.pixels_low <<= 1;
        bg_fifo.pixels_high <<= 1;

        size_t framebuffer_line_y = line_y * LCD_WIDTH;

        std::array<uint8_t, 4> color = color_table[(background_palette >> (int)(2 * pixel)) & 3];

        if (fetcher.mode == FetchMode::Background)
        {
            if (!(lcd_control & LCDControlFlags::BGEnable) ||
                !(render_flags & DisplayRenderFlags::Background))
            {
                color = color_table[(background_palette) & 3];
                bg_color_table[framebuffer_line_y + write_x] = 0;
            }
            else
            {
                bg_color_table[framebuffer_line_y + write_x] = pixel;
            }
        }
        else
        {
            //    framebuffer_line_y = window_line_y * LCD_WIDTH;
            bg_color_table[framebuffer_line_y + write_x] = pixel;
        }

        auto fb_pixel =
            std::span<uint8_t>{&framebuffer[(framebuffer_line_y + write_x) * COLOR_DEPTH], 4};

        std::copy(color.begin(), color.end(), fb_pixel.begin());
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
            int16_t corrected_y_position = (int16_t)sprite->y - 16;

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
            {
                bus.request_interrupt(INT_LCD_STAT_BIT);
            }
        }
    }
}