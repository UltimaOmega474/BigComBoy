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
#include <array>
#include <chrono>
#include <functional>

namespace Common
{
    class Timer
    {
        std::chrono::nanoseconds accumulator = std::chrono::nanoseconds::zero();
        std::chrono::steady_clock::time_point last_timer_time = std::chrono::steady_clock::now(),
                                              last_callback_time = std::chrono::steady_clock::now();
        std::chrono::nanoseconds interval = std::chrono::nanoseconds::zero();

        std::array<double, 100> samples{};
        size_t next = 0;
        double previous_avg = 0;
        double min = 1000000000.0, max = 0;

    public:
        void reset();
        void set_interval(std::chrono::nanoseconds target_interval);
        void run(std::function<void(Timer &)> on_timeout);
        void update_metrics();
        double min_ft() const;
        double max_ft() const;
        double average_ft() const;
        double average_fps() const;
    };
}