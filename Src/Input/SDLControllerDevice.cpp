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

#include "SDLControllerDevice.hpp"
#include <SDL.h>
#include <array>
#include <string>

namespace Input {
    const char *GetButtonNameFromKnownController(std::string_view name, int32_t button) {
        if (name == "DualSense Wireless Controller") {
            std::array<const char *, 16> MapList{
                "Cross", "Circle", "Square", "Triangle", "Share", "PS",   "Start", "L3",
                "R3",    "L1",     "R1",     "Up",       "Down",  "Left", "Right", NULL,
            };

            if (button == -1) {
                return "";
            }

            return MapList[button];
        } else {
            if (button < 0) {
                return "";
            }

            return SDL_GameControllerGetStringForButton((SDL_GameControllerButton)button);
        }
    }

    SDLControllerDevice::SDLControllerDevice(int32_t index)
        : controller(SDL_GameControllerOpen(index)) {}

    SDLControllerDevice::~SDLControllerDevice() {
        SDL_GameControllerClose(controller);
        controller = nullptr;
    }

    bool SDLControllerDevice::is_open() const { return controller != nullptr; }

    std::string_view SDLControllerDevice::name() const {
        return SDL_GameControllerName(controller);
    }

    void SDLControllerDevice::update_internal_state() {
        check_button_pressed(SDL_CONTROLLER_BUTTON_A);
        check_button_pressed(SDL_CONTROLLER_BUTTON_B);
        check_button_pressed(SDL_CONTROLLER_BUTTON_X);
        check_button_pressed(SDL_CONTROLLER_BUTTON_Y);
        check_button_pressed(SDL_CONTROLLER_BUTTON_BACK);
        check_button_pressed(SDL_CONTROLLER_BUTTON_GUIDE);
        check_button_pressed(SDL_CONTROLLER_BUTTON_START);
        check_button_pressed(SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        check_button_pressed(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
        check_button_pressed(SDL_CONTROLLER_BUTTON_LEFTSTICK);
        check_button_pressed(SDL_CONTROLLER_BUTTON_RIGHTSTICK);
        check_button_pressed(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        check_button_pressed(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        check_button_pressed(SDL_CONTROLLER_BUTTON_DPAD_UP);
        check_button_pressed(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        check_button_pressed(SDL_CONTROLLER_BUTTON_MISC1);
        check_button_pressed(SDL_CONTROLLER_BUTTON_TOUCHPAD);
    }

    void SDLControllerDevice::key_down(int32_t key) {}

    void SDLControllerDevice::key_up(int32_t key) {}

    bool SDLControllerDevice::is_key_down(const InputSource &key) {
        return pressed_buttons.contains(key.button);
    }

    std::optional<InputSource> SDLControllerDevice::get_input_for_any_key() {
        if (pressed_buttons.size() > 0) {
            int32_t key = *pressed_buttons.begin();
            return InputSource{
                .type = SourceType::ControllerButton,
                .keyboard = -1,
                .button = key,
                .axis = -1,
                .axis_direction = AxisDirection::Positive,
            };
        }

        return {};
    }

    std::string SDLControllerDevice::key_to_str(const InputSource &key) const {
        return GetButtonNameFromKnownController(name(), key.button);
    }

    int32_t SDLControllerDevice::str_to_key(std::string_view str) const { return 0; }

    void SDLControllerDevice::check_button_pressed(int32_t btn) {
        if (SDL_GameControllerGetButton(controller, (SDL_GameControllerButton)btn)) {
            pressed_buttons.insert(btn);
        } else if (pressed_buttons.contains(btn)) {
            pressed_buttons.erase(btn);
        }
    }

    void SDLControllerDevice::check_axis_pressed(int32_t axis_type) {
        Sint16 pressed_amount =
            SDL_GameControllerGetAxis(controller, (SDL_GameControllerAxis)axis_type);

        ControllerAxis axis{
            .axis = axis_type,
            .direction = pressed_amount > 0 ? AxisDirection::Positive : AxisDirection::Negative,
        };

        if (pressed_amount > 10000 || pressed_amount < -10000) {
            pressed_axis.push_back(axis);
        }
    }
}