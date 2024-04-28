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

#include "DiscordRPC.hpp"
#include <QTimer>
#include <chrono>
#include <cstdint>
#include <discord_rpc.h>
#include <fmt/format.h>

namespace QtFrontend::DiscordRPC {
    QTimer *poll_timer = nullptr;

    int64_t get_timestamp();
    void ready(const DiscordUser *user);
    void disconnected(int error_code, const char *message);
    void errored(int error_code, const char *message);

    void initialize() {
        DiscordEventHandlers handlers{};
        handlers.ready = &ready;
        handlers.disconnected = &disconnected;
        handlers.errored = &errored;
        Discord_Initialize("1233974713458360320", &handlers, 1, nullptr);

        poll_timer = new QTimer;
        QObject::connect(poll_timer, &QTimer::timeout, []() { Discord_RunCallbacks(); });
        poll_timer->setInterval(1000);
        poll_timer->start();
    }

    void close() {
        poll_timer->stop();
        delete poll_timer;
        poll_timer = nullptr;
        Discord_Shutdown();
    }

    void new_activity(const std::string &title) {
        DiscordRichPresence rpc{};
        rpc.largeImageKey = "bcb_app_icon";
        rpc.largeImageText = "Big ComBoy " BCB_VER;
        rpc.details = title.c_str();
        rpc.startTimestamp = get_timestamp();
        Discord_UpdatePresence(&rpc);
    }

    void set_idle() {
        DiscordRichPresence rpc{};
        rpc.largeImageKey = "bcb_app_icon";
        rpc.largeImageText = "Big ComBoy" BCB_VER;
        rpc.details = "Idle";
        rpc.startTimestamp = get_timestamp();
        Discord_UpdatePresence(&rpc);
    }

    int64_t get_timestamp() {
        const auto epoch = std::chrono::system_clock::now().time_since_epoch();
        const auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(epoch).count();
        return timestamp;
    }

    void ready(const DiscordUser *user) {
        fmt::print("Connected!\nUser: {}\nID: {}\n", user->username, user->userId);
#ifdef WIN32
        fflush(stdout);
#endif
        set_idle();
    }

    void disconnected(int error_code, const char *message) {
        fmt::print("Disconnected!\nError Code: {}\nMsg: {}\n", error_code, message);
#ifdef WIN32
        fflush(stdout);
#endif
    }

    void errored(int error_code, const char *message) {
        fmt::print("An error has occured!\nError Code: {}\nMsg: {}\n", error_code, message);
#ifdef WIN32
        fflush(stdout);
#endif
    }
}
