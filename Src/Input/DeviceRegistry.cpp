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

#include "DeviceRegistry.hpp"

namespace Input {
    static std::unordered_set<InputDevice *> devices_;

    std::unordered_set<InputDevice *> &devices() { return devices_; }

    void register_device(InputDevice *device) {
        if (!devices_.contains(device)) {
            devices_.insert(device);
        }
    }

    void remove_device(InputDevice *device) { devices_.erase(device); }

    std::optional<InputDevice *> try_find_by_name(std::string_view name) {
        for (auto device : devices_) {
            if (device->name() == name) {
                return device;
            }
        }

        return {};
    }
    std::optional<InputDevice *> try_find_by_index(int32_t index) {
        int32_t i = 0;

        for (auto device : devices_) {
            if (i == index) {
                return device;
            }

            ++i;
        }

        return {};
    }
    std::optional<int32_t> try_get_index_by_name(std::string_view name) {
        int32_t i = 0;

        for (auto device : devices_) {
            if (device->name() == name)
                return i;

            ++i;
        }

        return {};
    }
}