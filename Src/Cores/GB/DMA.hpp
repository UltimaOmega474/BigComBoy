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

    enum class DMAType { GDMA, HDMA };

    class DMAController {
    public:
        DMAController(Core *core);

        uint8_t get_dma_status() const;
        uint8_t get_hdma1() const;
        uint8_t get_hdma2() const;
        uint8_t get_hdma3() const;
        uint8_t get_hdma4() const;

        void reset();
        void set_dma_control(uint8_t ctrl);
        void set_hdma1(uint8_t high);
        void set_hdma2(uint8_t low);
        void set_hdma3(uint8_t high);
        void set_hdma4(uint8_t low);

        void tick();

    private:
        void transfer_block();

        bool active = false, is_mode0 = false;
        uint8_t current_length = 0x7F;
        uint16_t src_address = 0, dst_address = 0;
        DMAType type = DMAType::GDMA;

        Core *core;
    };
}
