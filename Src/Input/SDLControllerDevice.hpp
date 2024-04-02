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
#include "InputDevice.hpp"
#include <set>
#include <vector>

struct _SDL_GameController;
typedef _SDL_GameController SDL_GameController;

namespace Input {
    struct ControllerAxis {
        int32_t axis = 0;
        AxisDirection direction = AxisDirection::Positive;
    };

    class SDLControllerDevice : public InputDevice {
    public:
        explicit SDLControllerDevice(int32_t index);
        ~SDLControllerDevice();
        SDLControllerDevice(SDLControllerDevice &&) = delete;
        SDLControllerDevice(const SDLControllerDevice &) = delete;
        SDLControllerDevice &operator=(SDLControllerDevice &&) = delete;
        SDLControllerDevice &operator=(const SDLControllerDevice &) = delete;

        bool is_open() const;
        std::string_view name() const override;

        void key_down(int32_t key) override;
        void key_up(int32_t key) override;

        bool is_key_down(const InputSource &key) override;
        std::optional<InputSource> get_input_for_any_key() override;

        std::string key_to_str(const InputSource &key) const override;
        int32_t str_to_key(std::string_view str) const override;

        void update_internal_state() override;

    private:
        void check_button_pressed(int32_t btn);
        void check_axis_pressed(int32_t axis);

        SDL_GameController *controller = nullptr;
        std::set<int32_t> pressed_buttons{};
        std::vector<ControllerAxis> pressed_axis{};
    };
}