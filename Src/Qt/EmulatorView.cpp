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

#include "EmulatorView.hpp"
#include "Common/Config.hpp"
#include "GB/GBEmulatorController.hpp"
#include "MainWindow.hpp"
#include "OGL/GLFunctions.hpp"
#include "OGL/Renderer.hpp"
#include <QCoreApplication>
#include <QLabel>
#include <QScreen>
#include <QWindow>
#include <fmt/format.h>

namespace QtFrontend {
    EmulatorThread::EmulatorThread(QObject *parent)
        : QThread(parent), input_timer(), gb_controller(new GBEmulatorController) {
        connect(gb_controller, &GBEmulatorController::on_show,
                dynamic_cast<QOpenGLWidget *>(parent), &QWidget::show);
        connect(gb_controller, &GBEmulatorController::on_hide,
                dynamic_cast<QOpenGLWidget *>(parent), &QWidget::hide);
        connect(this, &EmulatorThread::on_post_input, gb_controller,
                &GBEmulatorController::copy_input);

        connect(&input_timer, &QTimer::timeout, this, &EmulatorThread::update_input);

        gb_controller->moveToThread(this);
        input_timer.start(1);
    }

    EmulatorThread::~EmulatorThread() {
        input_timer.stop();
        stop();

        if (!wait(500))
            terminate();

        if (gb_controller) {
            delete gb_controller;
            gb_controller = nullptr;
        }
    }

    void EmulatorThread::stop() { running = false; }

    void EmulatorThread::run() {
        auto accumulator = std::chrono::nanoseconds::zero();
        auto last_timer_time = std::chrono::steady_clock::now();
        auto last_callback_time = std::chrono::steady_clock::now();
        auto interval = Common::Math::FrequencyToNano(60);

        std::array<double, 100> samples{};
        size_t next = 0;
        double current_average = 0;

        while (running) {
            using namespace std::chrono_literals;
            QCoreApplication::processEvents();

            if (gb_controller->get_state() != EmulationState::Stopped) {
                using namespace std::chrono_literals;
                auto time_now = std::chrono::steady_clock::now();
                auto delta = time_now - last_timer_time;

                if (delta >= interval)
                    delta = interval;

                last_timer_time = time_now;
                accumulator += delta;

                if (accumulator >= interval) {
                    auto time_now = std::chrono::steady_clock::now();
                    auto delta = time_now - last_callback_time;
                    last_callback_time = time_now;

                    samples[next++] = static_cast<double>(
                        std::chrono::duration_cast<std::chrono::microseconds>(delta).count());

                    if (next == samples.size()) {
                        next = 0;

                        current_average = std::accumulate(samples.begin(), samples.end(), 0) /
                                          static_cast<double>(samples.size());
                        current_average /= 1000.0;
                    }

                    double fps = std::trunc(1000.0 / current_average);

                    emit on_update_fps_display(QString::fromStdString(
                        fmt::format("FPS:{} Avg:{:05.2f}ms", fps, current_average)));

                    if (gb_controller->try_run_frame()) {
                        auto &image = image_buffer.next_rendering_image();
                        auto ppu_image = gb_controller->get_core().ppu.framebuffer();

                        std::copy(ppu_image.begin(), ppu_image.end(), image.begin());

                        emit update_textures();
                    }

                    accumulator -= interval;
                }
            } else {
                accumulator = 0ns;
            }
        }
    }

    void EmulatorThread::update_input() {
        if (gb_controller) {
            std::array<bool, 8> buttons{};
            gb_controller->process_input(buttons);
            emit on_post_input(buttons);
        }
    }

    EmulatorView::EmulatorView(MainWindow *parent)
        : QOpenGLWidget(parent), thread(new EmulatorThread(this)), window(parent),
          functions(new GLFunctions) {
        float ratio = static_cast<float>(screen()->devicePixelRatio());
        scaled_width = static_cast<float>(width()) * ratio;
        scaled_height = static_cast<float>(height()) * ratio;

        connect_slots();
        thread->start();
    }

    EmulatorView::~EmulatorView() {
        thread->stop();

        if (functions) {
            delete functions;
            functions = nullptr;
        }
    }

    void EmulatorView::showEvent(QShowEvent *ev) {
        window->get_reset_action()->setDisabled(false);
        window->get_pause_action()->setDisabled(false);
        window->get_stop_action()->setDisabled(false);
    }

    void EmulatorView::hideEvent(QHideEvent *ev) {
        window->get_pause_action()->setChecked(false);

        window->get_reset_action()->setDisabled(true);
        window->get_pause_action()->setDisabled(true);
        window->get_stop_action()->setDisabled(true);
    }

    void EmulatorView::initializeGL() {
        functions->initializeOpenGLFunctions();

        renderer = new Renderer(functions);

        for (auto &texture : textures)
            texture = functions->create_texture(GB::LCD_WIDTH, GB::LCD_HEIGHT);
    }

    void EmulatorView::resizeGL(int w, int h) {
        float ratio = static_cast<float>(screen()->devicePixelRatio());
        scaled_width = static_cast<float>(w) * ratio;
        scaled_height = static_cast<float>(h) * ratio;
    }

    void EmulatorView::paintGL() {
        renderer->reset_state(scaled_width, scaled_height);

        const auto use_frame_blending = Common::Config::Current().gameboy.video.frame_blending;

        auto [final_width, final_height] = Common::Math::FitToAspectRatio(
            scaled_width, scaled_height, GB::LCD_WIDTH, GB::LCD_HEIGHT);

        float final_x = scaled_width / 2.0f - (final_width / 2);

        renderer->draw_image(textures[0], final_x, 0, final_width, final_height);

        if (use_frame_blending) {
            float alpha = 0.5f;
            for (size_t i = 1; i < textures.size(); ++i) {
                auto fbtexture = textures[i];
                auto color = Color{1.0f, 1.0f, 1.0f, alpha};
                renderer->draw_image(fbtexture, final_x, 0, final_width, final_height, color);
            }
        }
    }

    void EmulatorView::connect_slots() {
        connect(thread, &EmulatorThread::on_update_fps_display, window->get_fps_counter(),
                &QLabel::setText);

        connect(thread->gb_controller, &GBEmulatorController::on_load_success, window,
                &MainWindow::rom_load_success);

        connect(thread->gb_controller, &GBEmulatorController::on_load_fail, window,
                &MainWindow::rom_load_fail);

        connect(thread, &EmulatorThread::update_textures, this, &EmulatorView::update_textures);

        connect(window->get_reset_action(), &QAction::triggered, thread->gb_controller,
                &GBEmulatorController::reset_emulation);

        connect(window->get_pause_action(), &QAction::triggered, thread->gb_controller,
                &GBEmulatorController::set_pause);

        connect(window->get_stop_action(), &QAction::triggered, thread->gb_controller,
                &GBEmulatorController::stop_emulation);

        connect(window, &MainWindow::on_rom_loaded, thread->gb_controller,
                &GBEmulatorController::start_rom);
    }

    void EmulatorView::update_textures() {
        const auto &config = Common::Config::Current().gameboy.video;

        if (config.frame_blending) {
            for (size_t i = (framebuffers.size() - 1); i > 0; --i) {
                framebuffers[i] = framebuffers[i - 1];

                functions->update_texture_data(textures[i], GB::LCD_WIDTH, GB::LCD_HEIGHT,
                                               framebuffers[i]);

                functions->set_texture_filter(textures[i],
                                              config.smooth_scaling ? GL_LINEAR : GL_NEAREST);
            }
        }

        framebuffers[0] = thread->image_buffer.next_drawing_image();
        functions->update_texture_data(textures[0], GB::LCD_WIDTH, GB::LCD_HEIGHT, framebuffers[0]);
        functions->set_texture_filter(textures[0], config.smooth_scaling ? GL_LINEAR : GL_NEAREST);
        update();
    }
}