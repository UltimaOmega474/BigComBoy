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

#pragma once
#include "Constants.hpp"
#include <array>
#include <cinttypes>
#include <tuple>

namespace GB
{
    class MainBus;
    class PPU;

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

    enum LCDStatusFlags
    {
        EnableLYCandLYInt = 0x40,
        EnableOAMInt = 0x20,
        EnableVBlankInt = 0x10,
        EnableHBlankInt = 0x8,
        LYCandLYCompareType = 0x4,
        ModeFlag = 0x3
    };

    enum OBJAttributeFlags
    {
        Priority = 0x80,
        FlipY = 0x40,
        FlipX = 0x20,
        Palette = 0x10,
        BankSelect = 0x08,
        PaletteBits = 0x7,
    };

    enum RenderFlags
    {
        Background = 0x1,
        Objects = 0x2,
    };

    enum class FetchState
    {
        GetTileID,
        TileLow,
        TileHigh,
        Push,
    };

    enum class FetchMode
    {
        Background,
        Window,
    };

    struct Object
    {
        uint8_t y = 0;
        uint8_t x = 0;
        uint8_t tile = 0;
        uint8_t attributes = 0;
    };

    class BackgroundFIFO
    {
        uint8_t shift_count = 0;
        uint8_t pixels_low = 0, pixels_high = 0;

    public:
        uint8_t attribute = 0;
        uint8_t pixels_left() const;

        void clear();
        void load(uint8_t low, uint8_t high, uint8_t attribute);
        void force_shift(uint8_t amount);
        uint8_t clock();
    };

    class BackgroundFetcher
    {
        bool first_fetch = true;
        uint8_t substep = 0, tile_id = 0, attribute_id = 0;
        uint8_t queued_pixels_low = 0, queued_pixels_high = 0;
        uint16_t x_pos = 0, address = 0;
        FetchState state = FetchState::GetTileID;
        FetchMode mode = FetchMode::Background;

    public:
        FetchState get_state() const;
        FetchMode get_mode() const;

        void reset();
        void clear_with_mode(FetchMode new_mode);
        void clock(PPU &ppu);
        void get_tile_id(PPU &ppu);
        void get_tile_data(PPU &ppu, uint8_t bit_plane);
        void push_pixels(PPU &ppu);
    };

    class PPU
    {
        MainBus &bus;

    public:
        bool window_draw_flag = false, previously_disabled = false;
        uint8_t num_obj_on_scanline = 0, objs_processed = 0;
        uint8_t lcd_control = 0, status = 0;
        uint8_t screen_scroll_y = 0, screen_scroll_x = 0;
        uint8_t line_y = 0, line_y_compare = 0;
        uint8_t line_x = 0;
        uint8_t window_y = 0, window_x = 0;
        uint8_t window_line_y = 0, background_palette = 0;
        uint8_t object_palette_0 = 0, object_palette_1 = 0;

        // CGB
        uint8_t vram_bank_select = 0;
        uint8_t bg_palette_select = 0;
        uint8_t obj_palette_select = 0;
        uint8_t object_priority_mode = 0;

        uint16_t HDMA_src = 0, HDMA_dst = 0;

        uint8_t render_flags = RenderFlags::Background | RenderFlags::Objects;
        uint32_t cycles = 0, extra_cycles = 0;
        PPUState mode = PPUState::HBlank;

        std::array<uint8_t, 64> obj_cram{};
        std::array<uint8_t, 64> bg_cram{};
        std::array<uint8_t, 16384> vram{};
        std::array<uint8_t, 256> oam{};
        std::array<Object, 10> objects_on_scanline{};
        std::array<uint8_t, LCD_WIDTH * LCD_HEIGHT> bg_color_table{};
        std::array<uint8_t, LCD_WIDTH * LCD_HEIGHT * 4> framebuffer{};
        std::array<uint8_t, LCD_WIDTH * LCD_HEIGHT * 4> framebuffer_complete{};
        std::array<std::array<uint8_t, 4>, 4> color_table = LCD_GRAY_PALETTE;

        BackgroundFetcher fetcher;
        BackgroundFIFO bg_fifo;

        PPU(MainBus &bus);

        void reset(bool hard_reset);
        void set_post_boot_state();
        void step(uint32_t accumulated_cycles);

        void write_bg_palette(uint8_t value);
        uint8_t read_bg_palette() const;
        void write_obj_palette(uint8_t value);
        uint8_t read_obj_palette() const;
        void write_vram(uint16_t address, uint8_t value);
        void write_oam(uint16_t address, uint8_t value);
        void instant_dma(uint8_t address);
        void instant_hdma(uint8_t length);
        uint8_t hdma_blocks_remain() const;

        uint8_t read_vram(uint16_t address) const;
        uint8_t read_oam(uint16_t address) const;
        bool check_stat(uint8_t flags) const;
        void set_stat(uint8_t flags, bool value);
        bool stat_any() const;

    private:
        void render_scanline();
        void render_objects();
        void plot_pixel(uint8_t x_pos, uint8_t final_pixel, uint8_t palette);
        void plot_cgb_pixel(uint8_t x_pos, uint8_t final_pixel, uint8_t palette, bool is_obj);

        void scan_oam();
        void check_ly_lyc(bool allow_interrupts);
    };

}