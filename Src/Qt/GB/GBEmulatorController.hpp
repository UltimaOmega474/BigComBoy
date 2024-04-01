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
#include "AudioSystem.hpp"
#include "Common/Math.hpp"
#include "Cores/GB/Core.hpp"
#include <QObject>
#include <QTimer>
#include <array>
#include <filesystem>
#include <memory>

namespace GL {
    class Renderer;
}

namespace QtFrontend {
    constexpr size_t FRAMES = 2;

    enum class EmulationState { Stopped, BreakMode, Paused, Running };

    class GBEmulatorController : public QObject {
        Q_OBJECT

    public:
        GBEmulatorController();
        ~GBEmulatorController();
        GBEmulatorController(const GBEmulatorController &) = delete;
        GBEmulatorController(GBEmulatorController &&) = delete;
        GBEmulatorController &operator=(const GBEmulatorController &) = delete;
        GBEmulatorController &operator=(GBEmulatorController &&) = delete;

        EmulationState get_state() const;
        GB::Core &get_core();

        bool try_run_frame();
        void process_input(std::array<bool, 8> &buttons);

        Q_SLOT void start_rom(std::filesystem::path path);
        Q_SLOT void copy_input(std::array<bool, 8> input);
        Q_SLOT void set_pause(bool checked);
        Q_SLOT void stop_emulation();
        Q_SLOT void reset_emulation();
        Q_SLOT void save_sram();

        Q_SIGNAL void on_load_success(const QString &message, int timeout = 0);
        Q_SIGNAL void on_load_fail(const QString &message, int timeout = 0);
        Q_SIGNAL void on_show();
        Q_SIGNAL void on_hide();

    private:
        void init_by_console_type();

        EmulationState state = EmulationState::Stopped;
        GB::Core core{};
        std::unique_ptr<GB::Cartridge> cart;
        AudioSystem audio_system{};

        QTimer *sram_timer = nullptr;
    };
}