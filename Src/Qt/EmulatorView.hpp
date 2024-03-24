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
#include <QThread>
#include <QTimer>
#include <QWidget>
#include <atomic>
#include <mutex>

namespace GL
{
    class Renderer;
    class Context;
}

namespace QtFrontend
{
    class MainWindow;
    class GBEmulatorController;

    class EmulatorThread : public QThread
    {
        Q_OBJECT

        std::atomic_bool running = false;
        std::mutex rendering;

        GL::Context *context = nullptr;
        GL::Renderer *renderer = nullptr;
        bool initialized = false;

    public:
        QTimer input_timer;
        GBEmulatorController *gb_controller = nullptr;

        EmulatorThread(QObject *parent);
        ~EmulatorThread() override;

        void stop();
        void create_resources(void *window_handle);

        void resize(int w, int h);
        void run() override;

        void update_input();
        Q_SIGNAL void on_update_fps_display(const QString &text);
        Q_SIGNAL void on_post_input(std::array<bool, 8> input);
    };

    class EmulatorView : public QWidget
    {
        Q_OBJECT
        EmulatorThread *thread = nullptr;
        MainWindow *window = nullptr;

    public:
        EmulatorView(QWidget *parent);
        ~EmulatorView() override;

        GBEmulatorController *gb_controller() const;

        void set_window(MainWindow *main_window);
        void showEvent(QShowEvent *ev) override;
        void hideEvent(QHideEvent *ev) override;
        void resizeEvent(QResizeEvent *ev) override;

        QPaintEngine *paintEngine() const override { return nullptr; }
    };
}