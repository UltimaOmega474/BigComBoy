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
#include "GL/Renderer.hpp"
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <array>
#include <filesystem>
#include <memory>

namespace GL
{
    class Renderer;
}

namespace QtFrontend
{
    constexpr size_t FRAMES = 2;

    enum class EmulationState
    {
        Stopped,
        BreakMode,
        Paused,
        Running
    };

    class GBEmulatorController : public QObject
    {
        Q_OBJECT

        EmulationState state = EmulationState::Stopped;

        QMutex core_mutex;
        GB::Core core{};
        std::unique_ptr<GB::Cartridge> cart;
        AudioSystem audio_system{};

        std::array<GLuint, FRAMES> textures{};
        std::array<std::array<uint8_t, 160 * 144 * 4>, FRAMES> framebuffers{};
        QTimer *sram_timer = nullptr;

    public:
        GBEmulatorController();
        GBEmulatorController(const GBEmulatorController &) = delete;
        GBEmulatorController(GBEmulatorController &&) = delete;
        ~GBEmulatorController() override;
        GBEmulatorController &operator=(const GBEmulatorController &) = delete;
        GBEmulatorController &operator=(GBEmulatorController &&) = delete;

        EmulationState get_state() const;
        QMutex *get_mutex();
        GB::Core &get_core();

        void update();
        void draw_scene(GL::Renderer *renderer, float screen_width, float screen_height);
        void process_input(std::array<bool, 8> &buttons);

        Q_SLOT void start_rom(std::filesystem::path path);
        Q_SLOT void copy_input(std::array<bool, 8> input);
        Q_SLOT void set_pause(bool checked);
        Q_SLOT void stop_emulation();
        Q_SLOT void reset_emulation();
        Q_SLOT void step_frame();
        Q_SLOT void step_instruction(uint32_t count);
        Q_SLOT void save_sram();
        Q_SLOT void begin_trace();
        Q_SLOT void end_trace();

        Q_SIGNAL void load_success(const QString &message, int timeout = 0);
        Q_SIGNAL void load_fail(const QString &message, int timeout = 0);
        Q_SIGNAL void show();
        Q_SIGNAL void hide();
        Q_SIGNAL void update_debuggers();
        Q_SIGNAL void upload_trace_log(std::deque<GB::Instruction> &logger);

    private:
        void init_by_console_type();
        void update_textures();
    };
}