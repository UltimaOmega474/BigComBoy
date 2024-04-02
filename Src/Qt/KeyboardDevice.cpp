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

#include "KeyboardDevice.hpp"
#include <QKeySequence>
#include <QString>

namespace QtFrontend {
    void KeyboardDevice::key_down(int32_t key) { pressed_keys.insert(key); }

    void KeyboardDevice::key_up(int32_t key) { pressed_keys.erase(key); }

    bool KeyboardDevice::is_key_down(const Input::InputSource &key) {
        return pressed_keys.contains(key.keyboard);
    }

    std::optional<Input::InputSource> KeyboardDevice::get_input_for_any_key() {
        if (pressed_keys.size() > 0) {
            int32_t key = *pressed_keys.begin();
            return Input::InputSource{
                .type = Input::SourceType::Invalid,
                .keyboard = key,
                .button = -1,
                .axis = -1,
                .axis_direction = Input::AxisDirection::Positive,
            };
        }

        return {};
    }

    std::string KeyboardDevice::key_to_str(const Input::InputSource &key) const {
        return QKeySequence(key.keyboard).toString().toUpper().toStdString();
    }

    int32_t KeyboardDevice::str_to_key(std::string_view str) const { return 0; }

    void KeyboardDevice::update_internal_state() {}
}