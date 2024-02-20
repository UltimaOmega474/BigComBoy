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
#include <cinttypes>
#include <optional>
#include <string_view>

namespace Input
{
    enum class SourceType : int32_t
    {
        Invalid = -1,
        ControllerButton = 0,
        ControllerAxis = 1,
    };

    enum class AxisDirection : int32_t
    {
        Positive,
        Negative,
    };

    struct InputSource
    {
        SourceType type = SourceType::Invalid;
        int32_t keyboard = -1;
        int32_t button = -1;
        int32_t axis = -1;
        AxisDirection axis_direction = AxisDirection::Positive;
    };

    class InputDevice
    {
    public:
        virtual ~InputDevice() = default;

        virtual std::string_view name() const = 0;
        virtual void update_internal_state() = 0;
        virtual void key_down(int32_t key) = 0;
        virtual void key_up(int32_t key) = 0;

        virtual bool is_key_down(const InputSource &key) = 0;
        virtual std::optional<InputSource> get_input_for_any_key() = 0;

        virtual std::string key_to_str(const InputSource &key) const = 0;
        virtual int32_t str_to_key(std::string_view str) const = 0;
    };
}