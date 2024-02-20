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

#include "Timer.hpp"
#include <numeric>

namespace Common
{
    void Common::Timer::set_interval(std::chrono::nanoseconds target_interval)
    {
        interval = target_interval;
    }

    void Timer::reset()
    {
        next = 0;
        samples.fill(0);
        previous_avg = 0;
        last_callback_time = std::chrono::steady_clock::now();
        last_timer_time = std::chrono::steady_clock::now();
        max = 0;
        min = 1000000000.0;
    }

    void Timer::run(std::function<void(Timer &)> on_timeout)
    {
        auto time_now = std::chrono::steady_clock::now();
        auto delta = time_now - last_timer_time;
        last_timer_time = time_now;
        accumulator += delta;

        if (accumulator >= interval)
        {
            update_metrics();
            on_timeout(*this);
            accumulator -= interval;
        }
    }

    void Timer::update_metrics()
    {
        auto time_now = std::chrono::steady_clock::now();
        auto delta = time_now - last_callback_time;
        last_callback_time = time_now;

        samples[next++] = static_cast<double>(
            std::chrono::duration_cast<std::chrono::microseconds>(delta).count());

        if (next == samples.size())
        {
            next = 0;

            previous_avg = std::accumulate(samples.begin(), samples.end(), 0) /
                           static_cast<double>(samples.size());
            previous_avg /= 1000.0;

            if (previous_avg < min)
                min = previous_avg;

            if (previous_avg > max)
                max = previous_avg;
        }
    }

    double Timer::min_ft() const { return min; }

    double Timer::max_ft() const { return max; }

    double Timer::average_ft() const { return previous_avg; }

    double Timer::average_fps() const { return 1000.0 / previous_avg; }
}