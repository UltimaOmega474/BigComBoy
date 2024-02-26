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
#include <cstdint>
#include <span>

namespace GB
{
    bool BackgroundFIFO::empty() const { return shift_count == 0; }

    void BackgroundFIFO::clear()
    {
        shift_count = 0;
        pixels_low = 0;
        pixels_high = 0;
    }

    void BackgroundFIFO::load(uint8_t low, uint8_t high)
    {
        pixels_low = low;
        pixels_high = high;
        shift_count = 8;
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

    uint8_t ObjectFIFO::pixels_left() const { return shift_count; }

    void ObjectFIFO::clear()
    {
        shift_count = 0;
        pixels_low = 0;
        pixels_high = 0;
        palette = 0;
        priority = 0;
    }

    void ObjectFIFO::load(uint8_t pixels_avail, uint8_t low, uint8_t high, uint8_t palette,
                          uint8_t priority)
    {
        pixels_low |= low;
        pixels_high |= high;
        this->palette |= palette;
        this->priority |= priority;
        shift_count += pixels_avail;

        if (pixels_avail != 0)
            int d = 0;
    }

    std::tuple<uint8_t, uint8_t, uint8_t> ObjectFIFO::clock()
    {
        uint8_t low_bit = (pixels_low >> 7) & 0x1;
        uint8_t high_bit = (pixels_high >> 7) & 0x1;
        uint8_t out_pixel = (high_bit << 1) | (low_bit);
        uint8_t out_palette = (palette >> 7) & 1;
        uint8_t out_priority = (priority >> 7) & 1;

        pixels_low <<= 1;
        pixels_high <<= 1;
        palette <<= 1;
        priority <<= 1;

        shift_count--;
        return std::make_tuple(out_pixel, out_palette, out_priority);
    }

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
        case FetchState::Idle:
        {
            break;
        }
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
                queued_pixels_high = ppu.vram[address];

                if (first_fetch)
                {
                    state = FetchState::GetTileID;
                    first_fetch = false;
                }
            }
            else
            {
                state = FetchState::TileHigh;
                queued_pixels_low = ppu.vram[address];
            }

            break;
        }
        }
    }

    void BackgroundFetcher::push_pixels(PPU &ppu)
    {
        if (!ppu.bg_fifo.empty())
            return;

        x_pos += 8;
        ppu.bg_fifo.load(queued_pixels_low, queued_pixels_high);

        if (mode == FetchMode::Background)
        {
            if (ppu.write_x == 0)
            {
                auto scx = ppu.screen_scroll_x & 7;

                ppu.bg_fifo.force_shift(scx);
                ppu.extra_cycles += scx;
            }
        }

        substep = 0;
        state = FetchState::GetTileID;
    }

    void ObjectFetcher::reset()
    {
        ppu_obj = 0;
        substep = 0;
        tile_id = 0;
        queued_pixels_low = 0;
        queued_pixels_high = 0;
        address = 0;
        state = FetchState::GetTileID;
    }

    void ObjectFetcher::set_object(uint8_t obj_num) { ppu_obj = obj_num; }

    void ObjectFetcher::clock(PPU &ppu)
    {

        switch (state)
        {
        case FetchState::Idle:
        {
            for (int i = 0; i < ppu.num_obj_on_scanline; ++i)
            {
                const auto &obj = ppu.objects_on_scanline[i];
                if (obj.x <= (ppu.write_x + 8))
                {
                    ppu.halt_bg_fetcher = true;
                    ppu.fetcher.clear_with_mode(ppu.fetcher.get_mode());

                    ppu_obj = i;
                    state = FetchState::GetTileID;
                    substep = 0;

                    //    ppu.extra_cycles += 6;
                    break;
                }
            }

            break;
        }
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

    void ObjectFetcher::get_tile_id(PPU &ppu)
    {
        switch (substep)
        {
        case 0:
        {
            substep++;
            break;
        }

        case 1:
        {
            tile_id = ppu.objects_on_scanline[ppu_obj].tile;
            state = FetchState::TileLow;

            substep = 0;
            break;
        }
        }
    }

    void ObjectFetcher::get_tile_data(PPU &ppu, uint8_t bit_plane)
    {
        switch (substep)
        {
        case 0:
        {
            uint16_t computed_address = (0b1 << 15) + bit_plane;

            uint16_t yoffset = (ppu.line_y - ppu.objects_on_scanline[ppu_obj].y) & 7;

            auto flip_y = ppu.objects_on_scanline[ppu_obj].attributes & ObjectAttributeFlags::FlipY;

            if (flip_y)
                yoffset = ~yoffset;

            computed_address |= tile_id << 4;
            computed_address |= (yoffset << 1);

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
                queued_pixels_high = ppu.vram[address];
            }
            else
            {
                state = FetchState::TileHigh;
                queued_pixels_low = ppu.vram[address];
            }

            break;
        }
        }
    }

    void ObjectFetcher::push_pixels(PPU &ppu)
    {
        auto &obj = ppu.objects_on_scanline[ppu_obj];

        if (obj.attributes & ObjectAttributeFlags::FlipX)
        {
            auto byte_reverse = [](uint8_t input) -> uint8_t
            {
                uint8_t reversed = (input & 1) << 7;
                reversed |= (input & 2) << 6;
                reversed |= (input & 4) << 5;
                reversed |= (input & 8) << 4;
                reversed |= (input & 16) << 3;
                reversed |= (input & 32) << 2;
                reversed |= (input & 64) << 1;
                reversed |= (input & 128) >> 7;
                return reversed;
            };

            queued_pixels_low = byte_reverse(queued_pixels_low);
            queued_pixels_high = byte_reverse(queued_pixels_high);
        }

        uint8_t avail = 8;
        uint8_t palette_bits = (obj.attributes & ObjectAttributeFlags::Palette) ? 255 : 0;
        uint8_t priority_bits = (obj.attributes & ObjectAttributeFlags::Priority) ? 255 : 0;

        if (obj.x < 8)
        {
            avail = obj.x;
            auto shift_count = (8 - obj.x);

            queued_pixels_low <<= shift_count;
            queued_pixels_high <<= shift_count;
            palette_bits <<= shift_count;
            priority_bits <<= shift_count;
        }

        uint8_t already_queued = ppu.obj_fifo.pixels_left();
        if (already_queued)
        {
            queued_pixels_low <<= already_queued;
            queued_pixels_high <<= already_queued;
            palette_bits <<= already_queued;
            priority_bits <<= already_queued;
            avail -= already_queued;
        }

        ppu.obj_fifo.load(avail, queued_pixels_low, queued_pixels_high, palette_bits,
                          priority_bits);

        substep = 0;
        state = FetchState::Idle;
        ppu.halt_bg_fetcher = false;
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
                if (cycles == (204 - extra_cycles))
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
                    extra_cycles = 0;

                    mode = PPUState::DrawScanline;
                    fetcher.reset();
                    bg_fifo.clear();
                    obj_fetcher.reset();
                    obj_fifo.clear();
                    halt_bg_fetcher = false;
                    continue;
                }

                break;
            }

            case PPUState::DrawScanline:
            {
                if (cycles == 172 + extra_cycles)
                {
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
        if (!halt_bg_fetcher)
            fetcher.clock(*this);

        obj_fetcher.clock(*this);

        if ((write_x < 160))
        {
            uint8_t final_pixel = 0, final_palette = 0;
            uint8_t bg_pixel = 0;
            bool bg_clocked = false, obj_clocked = false;

            if (!bg_fifo.empty())
            {
                bg_pixel = bg_fifo.clock();
                final_pixel = bg_pixel;
                final_palette = background_palette;
                bg_clocked = true;

                if (!(lcd_control & LCDControlFlags::BGEnable))
                {
                    final_pixel = 0;
                }
            }

            if (obj_fifo.pixels_left())
            {
                auto [obj_pixel, obj_pal, obj_pri] = obj_fifo.clock();

                bool bg_has_priority =
                    bg_clocked && ((obj_pixel == 0) || (obj_pri && bg_pixel != 0));

                if (!bg_has_priority)
                {
                    final_pixel = obj_pixel;
                    final_palette = (obj_pal & ObjectAttributeFlags::Palette) ? object_palette_1
                                                                              : object_palette_0;
                }

                obj_clocked = true;
            }

            if (obj_clocked || bg_clocked)
            {
                plot_pixel(final_pixel, final_palette);
                write_x++;
            }
        }

        if ((lcd_control & LCDControlFlags::WindowEnable) && window_draw_flag &&
            (write_x >= (window_x - 7)) && fetcher.get_mode() == FetchMode::Background)
        {
            fetcher.clear_with_mode(FetchMode::Window);
            bg_fifo.clear();
            extra_cycles += 6;
        }
    }

    void PPU::plot_pixel(uint8_t final_pixel, uint8_t palette)
    {
        size_t framebuffer_line_y = line_y * LCD_WIDTH;

        std::array<uint8_t, 4> color = color_table[(palette >> (int)(2 * final_pixel)) & 3];

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