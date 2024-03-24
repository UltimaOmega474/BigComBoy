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
#include "Common/Timer.hpp"
#include "GB/GBEmulatorController.hpp"
#include "GL/Renderer.hpp"
#include "MainWindow.hpp"
#include <QCoreApplication>
#include <QLabel>
#include <QWindow>
#include <fmt/format.h>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include "../GL/CocoaContext.h"
#pragma clang diagnostic pop
#elif WIN32
#include "GL/WGLContext.hpp"
#else
#include "GL/NullContext.hpp"
#include <GL/glew.h>
#endif

namespace QtFrontend
{
    EmulatorThread::EmulatorThread(QObject *parent) : QThread(parent), input_timer() {}

    EmulatorThread::~EmulatorThread()
    {
        input_timer.stop();
        stop();

        if (!wait(500))
            terminate();

        if (context)
        {
            delete context;
            context = nullptr;
        }

        if (gb_controller)
        {
            delete gb_controller;
            gb_controller = nullptr;
        }
    }

    void EmulatorThread::stop() { running = false; }

    void EmulatorThread::create_resources(void *window_handle)
    {
        if (initialized)
            return;

#ifdef __APPLE__
        context = new GL::CocoaContext();
#elif WIN32
        context = new GL::WGLContext();
#else
        context = new GL::NullContext();
#endif

        if (context->create(window_handle))
        {
            running = true;
            renderer = new GL::Renderer();
            gb_controller = new GBEmulatorController();
            gb_controller->moveToThread(this);

            connect(gb_controller, &GBEmulatorController::on_show,
                    dynamic_cast<QWidget *>(parent()), &QWidget::show);
            connect(gb_controller, &GBEmulatorController::on_hide,
                    dynamic_cast<QWidget *>(parent()), &QWidget::hide);
            connect(this, &EmulatorThread::on_post_input, gb_controller,
                    &GBEmulatorController::copy_input);
            connect(&input_timer, &QTimer::timeout, this, &EmulatorThread::update_input);

            context->set_swap_interval(1);
            context->done_current();

            input_timer.start(1);
            initialized = true;
        }
        else
        {
            delete context;
            context = nullptr;
        }
    }

    void EmulatorThread::resize(int w, int h)
    {
        std::lock_guard<std::mutex> lg(rendering);
        if (context)
        {
            context->done_current();
            context->make_current();
            context->update(w, h);
            context->done_current();
        }
    }

    void EmulatorThread::run()
    {
        auto main_loop = [&](Common::Timer &timer)
        {
            gb_controller->update();

            auto current_average = timer.average_ft();
            double fps = std::trunc(timer.average_fps());

            emit on_update_fps_display(
                QString::fromStdString(fmt::format("FPS:{} Avg:{:05.2f}ms", fps, current_average)));
        };

        Common::Timer timer{};
        timer.set_interval(Common::Math::FrequencyToNano(60));

        while (running)
        {
            using namespace std::chrono_literals;
            QCoreApplication::processEvents();

            if (context)
            {
                std::lock_guard<std::mutex> lg(rendering);
                context->make_current();

                if (gb_controller->get_state() != EmulationState::Stopped)
                    timer.run(main_loop);
                else
                    timer.reset();

                QWidget *widget = dynamic_cast<QWidget *>(parent());
                auto size = widget->size();
                auto screen_width = size.width(), screen_height = size.height();

                GL::ClearColorBuffer(0, 0, 0);

                renderer->reset_state(static_cast<float>(screen_width),
                                      static_cast<float>(screen_height));

                gb_controller->draw_scene(renderer, static_cast<float>(screen_width),
                                          static_cast<float>(screen_height));

                context->swap_buffers();
                context->done_current();
            }
        }
    }

    void EmulatorThread::update_input()
    {
        if (gb_controller)
        {
            std::array<bool, 8> buttons{};
            gb_controller->process_input(buttons);
            emit on_post_input(buttons);
        }
    }

    EmulatorView::EmulatorView(QWidget *parent) : QWidget(parent), thread(new EmulatorThread(this))
    {
        setAttribute(Qt::WA_OpaquePaintEvent);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_PaintOnScreen);
        setAttribute(Qt::WA_NativeWindow);
        windowHandle()->setSurfaceType(QWindow::OpenGLSurface);

        thread->create_resources(reinterpret_cast<void *>(windowHandle()->winId()));
        thread->start();
    }

    EmulatorView::~EmulatorView() { thread->stop(); }

    GBEmulatorController *EmulatorView::gb_controller() const { return thread->gb_controller; }

    void EmulatorView::set_window(MainWindow *main_window)
    {
        connect(main_window, &MainWindow::on_rom_loaded, thread->gb_controller,
                &GBEmulatorController::start_rom);
        connect(thread->gb_controller, &GBEmulatorController::on_load_success, main_window,
                &MainWindow::rom_load_success);
        connect(thread->gb_controller, &GBEmulatorController::on_load_fail, main_window,
                &MainWindow::rom_load_fail);
        connect(thread, &EmulatorThread::on_update_fps_display, main_window->get_fps_counter(),
                &QLabel::setText);

        window = main_window;
    }

    void EmulatorView::showEvent(QShowEvent *ev)
    {
        connect(window->get_reset_action(), &QAction::triggered, thread->gb_controller,
                &GBEmulatorController::reset_emulation);

        connect(window->get_pause_action(), &QAction::triggered, thread->gb_controller,
                &GBEmulatorController::set_pause);

        connect(window->get_stop_action(), &QAction::triggered, thread->gb_controller,
                &GBEmulatorController::stop_emulation);

        window->get_reset_action()->setDisabled(false);
        window->get_pause_action()->setDisabled(false);
        window->get_stop_action()->setDisabled(false);
    }

    void EmulatorView::hideEvent(QHideEvent *ev)
    {
        window->get_reset_action()->disconnect();
        window->get_pause_action()->disconnect();
        window->get_stop_action()->disconnect();
        window->get_pause_action()->setChecked(false);

        window->get_reset_action()->setDisabled(true);
        window->get_pause_action()->setDisabled(true);
        window->get_stop_action()->setDisabled(true);
    }

    void EmulatorView::resizeEvent(QResizeEvent *ev)
    {
        thread->resize(size().width(), size().height());
    }
}