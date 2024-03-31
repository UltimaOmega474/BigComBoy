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

#include "DMA.hpp"
#include "Core.hpp"
#include "PPU.hpp"
#include <stdexcept>

namespace GB {
    DMAController::DMAController(Core *core) : core(core) {
        if (!core) {
            throw std::invalid_argument("Core cannot be null.");
        }
    }

    uint8_t DMAController::get_hdma1() const { return src_address >> 8; }

    uint8_t DMAController::get_hdma2() const { return src_address & 0xFF; }

    uint8_t DMAController::get_hdma3() const { return dst_address >> 8; }

    uint8_t DMAController::get_hdma4() const { return dst_address & 0xFF; }

    void DMAController::reset() {
        active = false;
        is_mode0 = false;
        src_address = 0;
        dst_address = 0;
        current_length = 0x7F;
        type = DMAType::GDMA;
    }

    void DMAController::set_dma_control(uint8_t ctrl) {
        if (!active) {
            type = (ctrl & 0x80) ? DMAType::HDMA : DMAType::GDMA;
            current_length = ctrl & 0x7F;
            active = true;
        } else {
            active = ctrl & 0x80;
        }
    }

    void DMAController::set_hdma1(uint8_t high) {
        src_address &= ~0xFF00;
        src_address |= high << 8;
    }

    void DMAController::set_hdma2(uint8_t low) {
        src_address &= ~0xFF;
        src_address |= low & 0xF0;
    }

    void DMAController::set_hdma3(uint8_t high) {
        dst_address &= ~0xFF00;
        dst_address |= (high & 0x1F) << 8;
    }

    void DMAController::set_hdma4(uint8_t low) {
        dst_address &= ~0xFF;
        dst_address |= low & 0xF0;
    }

    uint8_t DMAController::get_dma_status() const {
        uint8_t stat = !static_cast<uint8_t>(active) << 7;

        return stat | (current_length & 0x7F);
    }

    void DMAController::tick() {
        bool mode0_now = (core->ppu.read_register(0x41) & 0x3) == HBLANK;

        if (active) {
            switch (type) {
            case DMAType::GDMA: {

                for (int i = 0; i < (current_length + 1); ++i) {
                    transfer_block();
                }
                current_length = 0x7F;
                active = false;
                break;
            }
            case DMAType::HDMA: {
                if (!is_mode0 && mode0_now) {
                    transfer_block();

                    if (current_length) {
                        current_length--;
                    } else {
                        current_length = 0x7F;
                        active = false;
                    }
                }
                break;
            }
            }
        }

        is_mode0 = mode0_now;
    }

    void DMAController::transfer_block() {
        for (int i = 0; i < 16; ++i) {
            bool can_tick = (i % 4) == 0;

            if (can_tick) {
                core->tick_subcomponents(4);
            }

            uint8_t data = core->bus.read(src_address);

            if (can_tick) {
                core->tick_subcomponents(4);
            }

            core->ppu.write_vram(dst_address & 0x1FFF, data);

            src_address++;
            dst_address++;
        }
    }

}