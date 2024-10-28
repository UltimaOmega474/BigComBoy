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

namespace GB {
    DMAController::DMAController(Core *core) : core(core) {}

    auto DMAController::is_transfer_active() -> bool {
        const bool mode0_now = (core->ppu.read_register(0x41) & 0x3) == HBLANK;
        const bool previously_mode0 = is_mode0;
        is_mode0 = mode0_now;

        if (active) {
            switch (type) {
            case DMAType::GDMA: {
                return true;
            }
            case DMAType::HDMA: {
                return (!previously_mode0 && mode0_now) ||
                       (block_progress > 0 && block_progress < 16);
            }
            }
        }

        return false;
    }

    auto DMAController::get_hdma1() const -> uint8_t { return src_address >> 8; }

    auto DMAController::get_hdma2() const -> uint8_t { return src_address & 0xFF; }

    auto DMAController::get_hdma3() const -> uint8_t { return dst_address >> 8; }

    auto DMAController::get_hdma4() const -> uint8_t { return dst_address & 0xFF; }

    auto DMAController::reset() -> void {
        active = false;
        is_mode0 = false;
        src_address = 0;
        dst_address = 0;
        current_length = 0x7F;
        block_progress = 16;
        type = DMAType::GDMA;
    }

    auto DMAController::set_dma_control(const uint8_t ctrl) -> void {
        if (!active) {
            type = (ctrl & 0x80) ? DMAType::HDMA : DMAType::GDMA;
            current_length = (ctrl & 0x7F);
            block_progress = 16;
            active = true;
        } else {
            active = ctrl & 0x80;
        }
    }

    auto DMAController::set_hdma1(const uint8_t high) -> void {
        src_address &= ~0xFF00;
        src_address |= high << 8;
    }

    auto DMAController::set_hdma2(const uint8_t low) -> void {
        src_address &= ~0xFF;
        src_address |= low & 0xF0;
    }

    auto DMAController::set_hdma3(const uint8_t high) -> void {
        dst_address &= ~0xFF00;
        dst_address |= (high & 0x1F) << 8;
    }

    auto DMAController::set_hdma4(const uint8_t low) -> void {
        dst_address &= ~0xFF;
        dst_address |= low & 0xF0;
    }

    auto DMAController::get_dma_status() const -> uint8_t {
        const uint8_t stat = !static_cast<uint8_t>(active) << 7;
        return stat | (current_length & 0x7F);
    }

    auto DMAController::clock() -> void {
        for (int i = 0; i < 2; ++i) {
            const uint8_t data = core->bus.read(src_address);
            core->ppu.write_vram(dst_address & 0x1FFF, data);

            src_address++;
            dst_address++;
            block_progress--;
        }

        if (block_progress == 0) {
            if (current_length) {
                current_length--;
                block_progress = 16;
            } else {
                current_length = 0x7F;
                active = false;
            }
        }
    }
}