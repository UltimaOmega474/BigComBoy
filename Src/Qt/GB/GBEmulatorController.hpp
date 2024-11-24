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
#include "../EmulatorContext.hpp"
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

    class GBEmulatorController : public QObject, public EmulatorContext {
        Q_OBJECT

    public:
        GBEmulatorController();
        ~GBEmulatorController() override;
        GBEmulatorController(const GBEmulatorController &) = delete;
        GBEmulatorController(GBEmulatorController &&) = delete;
        GBEmulatorController &operator=(const GBEmulatorController &) = delete;
        GBEmulatorController &operator=(GBEmulatorController &&) = delete;

        EmulationState get_state() const;
        GB::Core &get_core();

        auto process_input(std::array<bool, 8> &buttons) -> void override;
        auto run() -> void override;
    private:
        void init_by_console_type();

        EmulationState state = EmulationState::Stopped;
        GB::Core core{};
        std::unique_ptr<GB::Cartridge> cart;
        AudioSystem audio_system{};

        QTimer *sram_timer = nullptr;
    };
}