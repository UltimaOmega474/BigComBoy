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

#include "Pad.hpp"

namespace GB {
    auto Gamepad::reset() -> void {
        dpad = action = 0x0F;
        mode = 0x30;
    }

    auto Gamepad::clear_buttons() -> void { dpad = action = 0x0F; }

    auto Gamepad::set_pad_state(const PadButton btn, const bool pressed) -> void {
        if (pressed) {
            switch (btn) {
            case PadButton::Right: {
                dpad &= ~0x1;
                break;
            }
            case PadButton::Left: {
                dpad &= ~0x2;
                break;
            }
            case PadButton::Up: {
                dpad &= ~0x4;
                break;
            }
            case PadButton::Down: {
                dpad &= ~0x8;
                break;
            }

            case PadButton::A: {
                action &= ~0x1;
                break;
            }
            case PadButton::B: {
                action &= ~0x2;
                break;
            }
            case PadButton::Select: {
                action &= ~0x4;
                break;
            }
            case PadButton::Start: {
                action &= ~0x8;
                break;
            }
            }
        } else {
            switch (btn) {
            case PadButton::Right: {
                dpad |= 0x1;
                break;
            }
            case PadButton::Left: {
                dpad |= 0x2;
                break;
            }
            case PadButton::Up: {
                dpad |= 0x4;
                break;
            }
            case PadButton::Down: {
                dpad |= 0x8;
                break;
            }

            case PadButton::A: {
                action |= 0x1;
                break;
            }
            case PadButton::B: {
                action |= 0x2;
                break;
            }
            case PadButton::Select: {
                action |= 0x4;
                break;
            }
            case PadButton::Start: {
                action |= 0x8;
                break;
            }
            }
        }
    }

    auto Gamepad::select_button_mode(const uint8_t value) -> void { mode = value; }

    uint8_t Gamepad::get_pad_state() const {

        if (mode & 0x10) {
            return action;
        }

        if(mode & 0x20) {
            return dpad;
        }

        return 0x0F;
    }
}