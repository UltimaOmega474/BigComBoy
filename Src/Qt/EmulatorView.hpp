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
#include "Cores/GB/Constants.hpp"
#include "SwapChain.hpp"
#include <QOpenGLWidget>
#include <QThread>
#include <QTimer>
#include <QWidget>
#include <array>
#include <atomic>
#include <chrono>
#include <mutex>

namespace GL {
    class Renderer;
    class Context;
}

namespace QtFrontend {
    class MainWindow;
    class GBEmulatorController;
    class GLFunctions;
    class Renderer;

    class EmulatorThread : public QThread {
        Q_OBJECT

    public:
        EmulatorThread(QObject *parent);
        ~EmulatorThread();
        EmulatorThread(const EmulatorThread &) = delete;
        EmulatorThread(EmulatorThread &&) = delete;
        EmulatorThread &operator=(const EmulatorThread &) = delete;
        EmulatorThread &operator=(EmulatorThread &&) = delete;

        void stop();
        void run() override;

        void update_input();
        Q_SIGNAL void on_update_fps_display(const QString &text);
        Q_SIGNAL void on_post_input(std::array<bool, 8> input);
        Q_SIGNAL void update_textures();

    private:
        std::atomic_bool running = true;

        QTimer input_timer;

        GBEmulatorController *gb_controller = nullptr;
        SwapChain<GB::LCD_WIDTH * GB::LCD_HEIGHT * 4> image_buffer;

        friend class EmulatorView;
    };

    class EmulatorView : public QOpenGLWidget {
        Q_OBJECT

    public:
        explicit EmulatorView(MainWindow *parent);
        ~EmulatorView() override;
        EmulatorView(const EmulatorView &) = delete;
        EmulatorView(EmulatorView &&) = delete;
        EmulatorView &operator=(const EmulatorView &) = delete;
        EmulatorView &operator=(EmulatorView &&) = delete;

        void showEvent(QShowEvent *ev) override;
        void hideEvent(QHideEvent *ev) override;
        void initializeGL() override;
        void resizeGL(int w, int h) override;
        void paintGL() override;

        void connect_slots();
        Q_SLOT void update_textures();

    private:
        float scaled_width = 0.0, scaled_height = 0.0;

        EmulatorThread *thread = nullptr;
        MainWindow *window = nullptr;
        GLFunctions *functions = nullptr;
        Renderer *renderer = nullptr;

        std::array<GLuint, 2> textures{};
        std::array<std::array<uint8_t, GB::LCD_WIDTH * GB::LCD_HEIGHT * 4>, 2> framebuffers{};
    };
}