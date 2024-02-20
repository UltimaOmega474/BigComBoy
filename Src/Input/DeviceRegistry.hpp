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

namespace Input
{

    class DeviceRegistry
    {
    public:
        static std::unordered_set<InputDevice *> &GetDevices();
        static void RegisterDevice(InputDevice *device);
        static void RemoveDevice(InputDevice *device);

        static std::optional<InputDevice *> TryFindDeviceByName(std::string_view name);
        static std::optional<InputDevice *> TryFindDeviceByIndex(int32_t index);
        static std::optional<int32_t> TryGetDeviceIndexFromName(std::string_view name);
    };
}