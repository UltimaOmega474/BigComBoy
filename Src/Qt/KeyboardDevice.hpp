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
#include "Input/InputDevice.hpp"
#include <set>

namespace QtFrontend {
    class KeyboardDevice : public Input::InputDevice {
    public:
        std::string_view name() const override { return "Keyboard"; }

        void update_internal_state() override{};
        void key_down(int32_t key) override;
        void key_up(int32_t key) override;

        bool is_key_down(const Input::InputSource &key) override;
        std::optional<Input::InputSource> get_input_for_any_key() override;

        std::string key_to_str(const Input::InputSource &key) const override;
        int32_t str_to_key(std::string_view str) const override;

    private:
        std::set<int32_t> pressed_keys;
    };
}