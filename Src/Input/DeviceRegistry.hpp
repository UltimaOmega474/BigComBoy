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
#include <optional>
#include <string_view>
#include <unordered_set>

namespace Input {
    std::unordered_set<InputDevice *> &devices();

    std::optional<InputDevice *> try_find_by_name(std::string_view name);
    std::optional<InputDevice *> try_find_by_index(int32_t index);
    std::optional<int32_t> try_get_index_by_name(std::string_view name);

    void register_device(InputDevice *device);
    void remove_device(InputDevice *device);

    auto update_state() -> void;
}