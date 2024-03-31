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
#include "Constants.hpp"
#include "Core.hpp"
#include <algorithm>
#include <span>
#include <stdexcept>

namespace GB {
    uint8_t BackgroundFIFO::pixel_attribute() const { return attribute; }

    uint8_t BackgroundFIFO::pixels_left() const { return shift_count; }

    void BackgroundFIFO::clear() {
        shift_count = 0;
        pixels_low = 0;
        pixels_high = 0;
        attribute = 0;
    }

    void BackgroundFIFO::load(uint8_t low, uint8_t high, uint8_t attribute) {
        pixels_low = low;
        pixels_high = high;
        shift_count = 8;
        this->attribute = attribute;

        if (attribute & TILE_FLIP_X_BIT) {
            auto byte_reverse = [](uint8_t input) -> uint8_t {
                input = ((input & 0b11110000) >> 4) | ((input & 0b00001111) << 4);
                input = ((input & 0b11001100) >> 2) | ((input & 0b00110011) << 2);
                input = ((input & 0b10101010) >> 1) | ((input & 0b01010101) << 1);
                return input;
            };

            pixels_low = byte_reverse(pixels_low);
            pixels_high = byte_reverse(pixels_high);
        }
    }

    void BackgroundFIFO::force_shift(uint8_t amount) {
        pixels_low <<= amount;
        pixels_high <<= amount;
        shift_count -= amount;
    }

    uint8_t BackgroundFIFO::clock() {
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

    void BackgroundFetcher::reset() {
        first_fetch = true;
        clear_with_mode(FetchMode::Background);
    }

    void BackgroundFetcher::clear_with_mode(FetchMode new_mode) {
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

    void BackgroundFetcher::clock(PPU &ppu) {
        switch (state) {
        case FetchState::GetTileID: {
            get_tile_id(ppu);
            break;
        }
        case FetchState::TileLow: {
            get_tile_data(ppu, 0);
            break;
        }
        case FetchState::TileHigh: {
            get_tile_data(ppu, 1);
            break;
        }
        case FetchState::Push: {
            push_pixels(ppu);
            break;
        }
        }
    }

    void BackgroundFetcher::get_tile_id(PPU &ppu) {
        switch (substep) {
        case 0: {
            uint16_t computed_address = 0b10011 << 11;

            switch (mode) {
            case FetchMode::Background: {
                computed_address |= ((ppu.lcd_control & BG_TILE_MAP_BIT) ? 1 : 0) << 10;

                uint16_t xoffset = ((x_pos / 8) + (ppu.screen_scroll_x / 8)) & 31;
                uint16_t yoffset = ((ppu.line_y + ppu.screen_scroll_y) / 8) & 31;
                xoffset &= 0x3FF;
                yoffset &= 0x3FF;

                computed_address |= xoffset;
                computed_address |= (yoffset << 5);

                break;
            }
            case FetchMode::Window: {
                computed_address |= ((ppu.lcd_control & WND_TILE_MAP_BIT) ? 1 : 0) << 10;

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

        case 1: {
            tile_id = ppu.vram[address];
            attribute_id = ppu.vram[0x2000 + address];
            state = FetchState::TileLow;

            substep = 0;
            break;
        }
        }
    }

    void BackgroundFetcher::get_tile_data(PPU &ppu, uint8_t bit_plane) {
        switch (substep) {
        case 0: {
            uint16_t computed_address = (0b1 << 15) + bit_plane;

            switch (mode) {
            case FetchMode::Background: {
                uint16_t bit12 = !((ppu.lcd_control & TILE_DATA_LOC_BIT) || (tile_id & 0x80));
                uint16_t yoffset = (ppu.line_y + ppu.screen_scroll_y) & 7;

                if (attribute_id & TILE_FLIP_Y_BIT) {
                    yoffset = (7 - (ppu.line_y + ppu.screen_scroll_y)) & 7;
                }

                computed_address |= bit12 ? (1 << 12) : 0;
                computed_address |= tile_id << 4;
                computed_address |= (yoffset << 1);

                break;
            }
            case FetchMode::Window: {
                uint16_t bit12 = !((ppu.lcd_control & TILE_DATA_LOC_BIT) || (tile_id & 0x80));
                uint16_t yoffset = ppu.window_line_y & 7;

                if (attribute_id & TILE_FLIP_Y_BIT) {
                    yoffset = (7 - ppu.window_line_y) & 7;
                }

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
        case 1: {
            substep = 0;

            if (bit_plane == 1) {
                state = FetchState::Push;

                auto bank = (attribute_id & 0x8) >> 3;
                queued_pixels_high = ppu.vram[(0x2000 * bank) + address];

                if (first_fetch) {
                    state = FetchState::GetTileID;
                    first_fetch = false;
                }
            } else {
                state = FetchState::TileHigh;
                auto bank = (attribute_id & 0x8) >> 3;
                queued_pixels_low = ppu.vram[(0x2000 * bank) + address];
            }

            break;
        }
        }
    }

    void BackgroundFetcher::push_pixels(PPU &ppu) {
        if (ppu.bg_fifo.pixels_left())
            return;

        x_pos += 8;
        ppu.bg_fifo.load(queued_pixels_low, queued_pixels_high, attribute_id);

        if (mode == FetchMode::Background) {
            if (ppu.line_x == 0) {
                auto scx = ppu.screen_scroll_x & 7;

                ppu.bg_fifo.force_shift(scx);
                ppu.extra_cycles += scx;
            }
        }

        substep = 0;
        state = FetchState::GetTileID;
    }

    PPU::PPU(Core *core) : core(core) {
        if (!core) {
            throw std::invalid_argument("Core cannot be null.");
        }
    }

    std::span<uint8_t, LCD_WIDTH * LCD_HEIGHT * 4> PPU::framebuffer() {
        return framebuffer_complete;
    }

    void PPU::reset() {
        fetcher.reset();
        bg_fifo.clear();

        window_draw_flag = false;
        previously_disabled = false;
        num_obj_on_scanline = 0;
        line_x = 0;
        cycles = 0;
        extra_cycles = 0;

        lcd_control = 0;
        status = 0;
        screen_scroll_y = 0;
        screen_scroll_x = 0;
        line_y = 0;
        line_y_compare = 0;

        window_y = 0;
        window_x = 0;
        line_y_compare = 0;

        background_palette = 0;
        object_palette_0 = 0;
        object_palette_1 = 0;

        vram_bank_select = 0;
        bg_palette_select = 0;
        obj_palette_select = 0;
        object_priority_mode = 0;

        obj_cram.fill(0);
        bg_cram.fill(0);

        vram.fill(0);

        oam.fill(0);
        objects_on_scanline.fill(Object{});

        bg_color_table.fill(0);
        internal_framebuffer.fill(0);
        framebuffer_complete.fill(0);
    }

    void PPU::set_post_boot_state() {
        previously_disabled = false;
        status = 0x85;
        lcd_control = 0x91;
        screen_scroll_x = 0;
        screen_scroll_y = 0;
        line_y = 0;
        background_palette = 0xFC;
    }

    void PPU::set_compatibility_palette(PaletteID palette_type,
                                        const std::span<const uint16_t> colors) {
        switch (palette_type) {
        case PaletteID::BG: {
            auto palette = std::span(reinterpret_cast<uint16_t *>(bg_cram.data()), 4);
            std::copy(colors.begin(), colors.end(), palette.begin());

            break;
        }
        case PaletteID::OBJ1: {
            auto palette = std::span(reinterpret_cast<uint16_t *>(obj_cram.data()), 4);
            std::copy(colors.begin(), colors.end(), palette.begin());
            break;
        }
        case PaletteID::OBJ2: {
            auto palette = std::span(reinterpret_cast<uint16_t *>(obj_cram.data() + 8), 4);
            std::copy(colors.begin(), colors.end(), palette.begin());
            break;
        }
        }
    }

    void PPU::step(uint32_t accumulated_cycles) {
        if (!(lcd_control & LCD_ENABLED_BIT)) {
            set_mode(HBLANK);
            previously_disabled = true;
            return;
        }

        if (previously_disabled) {
            window_draw_flag = false;
            num_obj_on_scanline = 0;
            cycles = 0;
            extra_cycles = 0;
            line_y = 0;
            window_line_y = 0;

            fetcher.reset();
            bg_fifo.clear();

            previously_disabled = false;
        }

        while (accumulated_cycles) {
            bool allow_interrupt = !stat_any();

            if (window_y == line_y) {
                window_draw_flag = true;
            }

            switch (status & 0x3) {

            case HBLANK: {

                if (cycles == (204 - extra_cycles)) {
                    cycles = 0;
                    ++line_y;

                    line_x = 0;

                    if (line_y == 144) {
                        set_mode(VBLANK);

                        core->cpu.request_interrupt(INT_VBLANK_BIT);
                        if ((status & VBLANK_STAT_INT_BIT) && allow_interrupt) {
                            core->cpu.request_interrupt(INT_LCD_STAT_BIT);
                        }
                    } else {

                        set_mode(OAM_SEARCH);

                        if ((status & OAM_STAT_INT_BIT) && allow_interrupt) {
                            core->cpu.request_interrupt(INT_LCD_STAT_BIT);
                        }
                    }

                    continue;
                }
                break;
            }

            case VBLANK: {
                if (cycles == 456) {
                    ++line_y;
                    cycles = 0;

                    if (line_y > 153) {
                        framebuffer_complete = internal_framebuffer;
                        set_mode(OAM_SEARCH);

                        if ((status & OAM_STAT_INT_BIT) && allow_interrupt) {
                            core->cpu.request_interrupt(INT_LCD_STAT_BIT);
                        }

                        line_y = 0;
                        window_line_y = 0;
                        window_draw_flag = false;

                        continue;
                    }
                }
                break;
            }

            case OAM_SEARCH: {
                if (cycles == 80) {
                    scan_oam();
                    cycles = 0;
                    extra_cycles = 0;

                    set_mode(PIXEL_TRANSFER);
                    fetcher.reset();
                    bg_fifo.clear();
                    continue;
                }

                break;
            }

            case PIXEL_TRANSFER: {
                if (cycles == 172 + extra_cycles) {
                    render_objects();
                    cycles = 0;

                    set_mode(HBLANK);
                    if (fetcher.get_mode() == FetchMode::Window) {
                        window_line_y++;
                    }

                    if ((status & HBLANK_STAT_INT_BIT) && allow_interrupt) {
                        core->cpu.request_interrupt(INT_LCD_STAT_BIT);
                    }

                    continue;
                } else {
                    render_scanline();
                }
                break;
            }
            }

            accumulated_cycles--;
            cycles++;

            check_ly_lyc(allow_interrupt);
        }
    }

    void PPU::write_register(uint8_t reg, uint8_t value) {
        switch (reg) {
        case 0x40: {
            lcd_control = value;
            return;
        }
        case 0x41: {
            status &= 0x3;
            status |= value & 0xF8;
            return;
        }
        case 0x42: {
            screen_scroll_y = value;
            return;
        }
        case 0x43: {
            screen_scroll_x = value;
            return;
        }
        case 0x45: {
            line_y_compare = value;
            return;
        }
        case 0x46: {
            instant_dma(value);
            return;
        }
        case 0x47: {
            background_palette = value;
            return;
        }
        case 0x48: {
            object_palette_0 = value;
            return;
        }
        case 0x49: {
            object_palette_1 = value;
            return;
        }
        case 0x4A: {
            window_y = value;
            return;
        }
        case 0x4B: {
            window_x = value;
            return;
        }

        case 0x4F: {
            vram_bank_select = value & 0x1;
            break;
        }
        case 0x68: {
            bg_palette_select = value;
            break;
        }
        case 0x69: {
            write_bg_palette(value);
            break;
        }
        case 0x6A: {
            obj_palette_select = value;
            break;
        }
        case 0x6B: {
            write_obj_palette(value);
            break;
        }
        case 0x6C: {
            if (core->bus.bootstrap_mapped()) {
                object_priority_mode = value & 0x1;
            }
            break;
        }
        }
    }

    uint8_t PPU::read_register(uint8_t reg) const {
        switch (reg) {
        case 0x40: {
            return lcd_control;
        }
        case 0x41: {
            return status;
        }
        case 0x42: {
            return screen_scroll_y;
        }
        case 0x43: {
            return screen_scroll_x;
        }
        case 0x44: {
            return line_y;
        }
        case 0x45: {
            return line_y_compare;
        }
        case 0x46: {
            return 0xFF;
        }
        case 0x47: {
            return background_palette;
        }
        case 0x48: {
            return object_palette_0;
        }
        case 0x49: {
            return object_palette_1;
        }
        case 0x4A: {
            return window_y;
        }
        case 0x4B: {
            return window_x;
        }
        case 0x4F: {
            return vram_bank_select | 0xFE;
        }
        case 0x68: {
            return bg_palette_select;
        }
        case 0x69: {
            return read_bg_palette();
        }
        case 0x6A: {
            return obj_palette_select;
        }
        case 0x6B: {
            return read_obj_palette();
        }
        case 0x6C: {
            return object_priority_mode;
        }
        }
        return 0;
    }

    void PPU::write_vram(uint16_t address, uint8_t value) {
        vram[(vram_bank_select * 0x2000) + address] = value;
    }

    uint8_t PPU::read_vram(uint16_t address) const {
        return vram[(vram_bank_select * 0x2000) + address];
    }

    void PPU::write_oam(uint16_t address, uint8_t value) { oam[address] = value; }

    uint8_t PPU::read_oam(uint16_t address) const { return oam[address]; }

    void PPU::write_bg_palette(uint8_t value) {
        bg_cram[bg_palette_select & 0x3F] = value;

        if (bg_palette_select & 0x80) {
            bg_palette_select = ((bg_palette_select + 1) & 0x3F) | 0x80;
        }
    }

    uint8_t PPU::read_bg_palette() const { return bg_cram[bg_palette_select & 0x3F]; }

    void PPU::write_obj_palette(uint8_t value) {
        obj_cram[obj_palette_select & 0x3F] = value;

        if (obj_palette_select & 0x80) {
            obj_palette_select = ((obj_palette_select + 1) & 0x3F) | 0x80;
        }
    }

    uint8_t PPU::read_obj_palette() const { return obj_cram[obj_palette_select & 0x3F]; }

    void PPU::instant_dma(uint8_t address) {
        uint16_t addr = address << 8;
        for (int i = 0; i < 160; ++i) {
            oam[i] = core->bus.read(addr + i);
        }
    }

    void PPU::set_stat(uint8_t flags, bool value) {
        if (value) {
            status |= flags;
        } else {
            status &= ~flags;
        }
    }

    bool PPU::stat_any() const {
        uint8_t mode = status & 0x3;

        if (status & LYC_LY_STAT_INT_BIT) {
            if (status & LYC_LY_COMPARE_MODE_BIT) {
                return true;
            }
        }

        if (status & OAM_STAT_INT_BIT) {
            if (mode == OAM_SEARCH) {
                return true;
            }
        }

        if (status & VBLANK_STAT_INT_BIT) {
            if (mode == VBLANK) {
                return true;
            }
        }

        if (status & HBLANK_STAT_INT_BIT) {
            if (mode == HBLANK) {
                return true;
            }
        }

        return false;
    }

    void PPU::render_scanline() {
        fetcher.clock(*this);

        if ((line_x < 160) && bg_fifo.pixels_left()) {
            uint8_t final_pixel = 0, final_palette = bg_fifo.pixel_attribute() & 0x7;
            uint8_t final_dmg_palette = background_palette;
            uint8_t bg_pixel = bg_fifo.clock();

            bool bg_enabled =
                core->bus.is_compatibility_mode() ? (lcd_control & BG_ENABLED_BIT) : true;

            if (!bg_enabled) {
                final_pixel = 0;
                final_palette = 0;
                final_dmg_palette = 0;
            } else {
                final_pixel = bg_pixel;
            }

            bg_color_table[(line_y * LCD_WIDTH) + line_x] =
                final_pixel | (static_cast<uint16_t>(bg_fifo.pixel_attribute()) << 8);

            if (core->bus.is_compatibility_mode()) {
                uint8_t cgb_pixel = (final_dmg_palette >> (int)(2 * final_pixel)) & 3;

                plot_cgb_pixel(line_x, cgb_pixel, 0, false);
            } else {
                plot_cgb_pixel(line_x, final_pixel, final_palette, false);
            }

            line_x++;
        }

        if ((lcd_control & WND_ENABLED_BIT) && window_draw_flag && (line_x >= (window_x - 7)) &&
            fetcher.get_mode() == FetchMode::Background) {

            fetcher.clear_with_mode(FetchMode::Window);
            bg_fifo.clear();
            extra_cycles += 6;
        }
    }

    void PPU::render_objects() {
        if (!(lcd_control & OBJECTS_ENABLED_BIT)) {
            return;
        }

        uint8_t height = (lcd_control & OBJECT_SIZE_BIT) ? 16 : 8;

        for (int i = 0; i < num_obj_on_scanline; ++i) {
            auto &object = objects_on_scanline[(num_obj_on_scanline - 1) - i];

            uint8_t cgb_palette_idx = 0;
            uint8_t palette = object_palette_0;

            if (object.attributes & OBJ_PALETTE_SELECT_BIT) {
                cgb_palette_idx = 1;
                palette = object_palette_1;
            }

            uint8_t cgb_palette = (object.attributes & CGB_PALETTE_NUM_MASK);
            uint16_t bank = 0x2000 * ((object.attributes & VRAM_BANK_SELECT_BIT) >> 3);

            uint16_t tile_index = 0;

            if (height == 16) {
                object.tile &= ~1;
            }

            int32_t obj_y = static_cast<int32_t>(object.y) - 16;

            if (object.attributes & TILE_FLIP_Y_BIT) {
                tile_index =
                    (0x8000 & 0x1FFF) + (object.tile * 16) + ((height - (line_y - obj_y) - 1) * 2);
            } else {
                tile_index =
                    (0x8000 & 0x1FFF) + (object.tile * 16) + ((line_y - obj_y) % height * 2);
            }

            int32_t adjusted_x = static_cast<int32_t>(object.x) - 8;
            size_t framebuffer_line_y = line_y * LCD_WIDTH;

            for (int x = 0; x < 8; ++x) {
                size_t framebuffer_line_x = adjusted_x + x;

                if (framebuffer_line_x >= 0 && framebuffer_line_x < 160) {
                    uint8_t low_byte = vram[bank + tile_index];
                    uint8_t high_byte = vram[bank + (tile_index + 1)];
                    uint8_t bit = 7 - (x & 7);

                    if (object.attributes & TILE_FLIP_X_BIT) {
                        bit = x & 7;
                    }

                    uint8_t low_bit = (low_byte >> bit) & 0x01;
                    uint8_t high_bit = (high_byte >> bit) & 0x01;
                    uint8_t pixel = (high_bit << 1) | low_bit;

                    if (pixel == 0) {
                        continue;
                    }

                    uint16_t bg_pixel =
                        bg_color_table[framebuffer_line_y + framebuffer_line_x] & 0xFF;
                    uint16_t bg_attribute =
                        bg_color_table[framebuffer_line_y + framebuffer_line_x] >> 8;

                    bool bg_priority = bg_attribute & PRIORITY_BIT;
                    bool oam_priority = object.attributes & PRIORITY_BIT;
                    bool master_priority = lcd_control & MASTER_PRIORITY_BIT;

                    bool bg_has_priority = false;

                    if (core->bus.is_compatibility_mode()) {
                        bg_has_priority = bg_pixel && (object.attributes & PRIORITY_BIT);

                        if (!bg_has_priority) {
                            uint8_t cgb_pixel = (palette >> (int)(2 * pixel)) & 3;
                            plot_cgb_pixel(framebuffer_line_x, cgb_pixel, cgb_palette_idx, true);
                        }
                    } else {
                        if (master_priority && bg_pixel) {
                            bg_has_priority = oam_priority || bg_priority;
                        }

                        if (!bg_has_priority) {
                            plot_cgb_pixel(framebuffer_line_x, pixel, cgb_palette, true);
                        }
                    }
                }
            }
        }
    }

    void PPU::plot_cgb_pixel(uint8_t x_pos, uint8_t final_pixel, uint8_t palette, bool is_obj) {
        size_t framebuffer_line_y = line_y * LCD_WIDTH;
        size_t select_color = (palette * 8) + (final_pixel * 2);

        uint16_t color = 0;

        if (is_obj) {
            color = obj_cram[select_color];
            color |= obj_cram[select_color + 1] << 8;
        } else {
            color = bg_cram[select_color];
            color |= bg_cram[select_color + 1] << 8;
        }

        auto fb_pixel = std::span<uint8_t>{
            &internal_framebuffer[(framebuffer_line_y + x_pos) * FRAMEBUFFER_COLOR_CHANNELS], 4};

        auto r = color & 0x1F;
        auto g = (color >> 5) & 0x1F;
        auto b = (color >> 10) & 0x1F;

        fb_pixel[0] = (r * 255) / 31;
        fb_pixel[1] = (g * 255) / 31;
        fb_pixel[2] = (b * 255) / 31;
        fb_pixel[3] = 255;
    }

    void PPU::scan_oam() {
        uint8_t height = (lcd_control & OBJECT_SIZE_BIT) ? 16 : 8;

        num_obj_on_scanline = 0;
        objects_on_scanline.fill({});
        for (int i = 0, total = 0; i < 40; ++i) {
            if (total == 10) {
                break;
            }

            const Object *sprite = reinterpret_cast<const Object *>((&oam[i * 4]));
            int32_t corrected_y_position = static_cast<int32_t>(sprite->y) - 16;

            if (corrected_y_position <= line_y && (corrected_y_position + height) > line_y) {
                objects_on_scanline[total] = *sprite;
                total++;
                num_obj_on_scanline++;
            }
        }

        if (object_priority_mode & 0x1) {
            std::stable_sort(
                objects_on_scanline.begin(), objects_on_scanline.begin() + num_obj_on_scanline,
                [=](const Object &left, const Object &right) { return (left.x) < (right.x); });
        }
    }

    void PPU::set_mode(uint8_t mode) {
        mode &= 0x3;
        status &= ~0x3;
        status |= mode;
    }

    void PPU::check_ly_lyc(bool allow_interrupts) {
        set_stat(LYC_LY_COMPARE_MODE_BIT, false);

        if (line_y == line_y_compare) {
            set_stat(LYC_LY_COMPARE_MODE_BIT, true);

            if ((status & LYC_LY_STAT_INT_BIT) && allow_interrupts) {
                core->cpu.request_interrupt(INT_LCD_STAT_BIT);
            }
        }
    }
}