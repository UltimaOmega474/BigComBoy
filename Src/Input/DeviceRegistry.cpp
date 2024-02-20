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

namespace Input
{

    std::unordered_set<InputDevice *> devices;

    std::unordered_set<InputDevice *> &DeviceRegistry::GetDevices() { return devices; }

    void DeviceRegistry::RegisterDevice(InputDevice *device)
    {
        if (!devices.contains(device))
            devices.insert(device);
    }

    void DeviceRegistry::RemoveDevice(InputDevice *device) { devices.erase(device); }

    std::optional<InputDevice *> DeviceRegistry::TryFindDeviceByName(std::string_view name)
    {
        for (auto device : devices)
        {
            if (device->name() == name)
                return device;
        }

        return {};
    }
    std::optional<InputDevice *> DeviceRegistry::TryFindDeviceByIndex(int32_t index)
    {
        int32_t i = 0;
        for (auto device : devices)
        {
            if (i == index)
                return device;

            ++i;
        }

        return {};
    }
    std::optional<int32_t> DeviceRegistry::TryGetDeviceIndexFromName(std::string_view name)
    {
        int32_t i = 0;
        for (auto device : devices)
        {
            if (device->name() == name)
                return i;

            ++i;
        }

        return {};
    }
}