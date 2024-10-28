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
#include <cinttypes>

namespace GB {
    class Core;

    enum class DMAType {
        GDMA,
        HDMA,
    };

    class DMAController {
    public:
        explicit DMAController(Core *core);

        auto is_transfer_active() -> bool;
        auto get_dma_status() const -> uint8_t;
        auto get_hdma1() const -> uint8_t;
        auto get_hdma2() const -> uint8_t;
        auto get_hdma3() const -> uint8_t;
        auto get_hdma4() const -> uint8_t;

        auto reset() -> void;
        auto set_dma_control(uint8_t ctrl) -> void;
        auto set_hdma1(uint8_t high) -> void;
        auto set_hdma2(uint8_t low) -> void;
        auto set_hdma3(uint8_t high) -> void;
        auto set_hdma4(uint8_t low) -> void;

        auto clock() -> void;

    private:
        bool active = false;
        bool is_mode0 = false;
        uint8_t current_length = 0x7F;
        uint16_t src_address = 0;
        uint16_t dst_address = 0;
        int32_t block_progress = 16;
        DMAType type = DMAType::GDMA;

        Core *core;
    };
}
