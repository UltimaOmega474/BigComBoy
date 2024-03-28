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
#include <QMainWindow>
#include <QTimer>
#include <filesystem>
#include <memory>

namespace Ui
{
    class MainWindow;
}

class QAction;
class QLabel;

namespace Input
{
    class InputDevice;
}

namespace QtFrontend
{
    class EmulatorView;
    class SettingsWindow;

    class MainWindow : public QMainWindow
    {
        Q_OBJECT

        QTimer input_timer;
        std::vector<std::unique_ptr<Input::InputDevice>> controllers;
        std::unique_ptr<Input::InputDevice> keyboard;

        Ui::MainWindow *ui;
        SettingsWindow *settings = nullptr;

        QLabel *fps_counter = nullptr;
        EmulatorView *emulator_widget;

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow() override;
        MainWindow(const MainWindow &) = delete;
        MainWindow(MainWindow &&) = delete;
        MainWindow &operator=(const MainWindow &) = delete;
        MainWindow &operator=(MainWindow &&) = delete;

        void showEvent(QShowEvent *) override;
        void closeEvent(QCloseEvent *event) override;
        void keyPressEvent(QKeyEvent *event) override;
        void keyReleaseEvent(QKeyEvent *event) override;

        QAction *get_reset_action();
        QAction *get_pause_action();
        QAction *get_stop_action();
        QLabel *get_fps_counter();

        Q_SLOT void open_rom_file_browser();
        Q_SLOT void open_rom_from_recents(QAction *action);
        Q_SLOT void open_gb_settings();
        Q_SLOT void clear_settings_ptr();
        Q_SLOT void rom_load_success(const QString &message, int timeout = 0);
        Q_SLOT void rom_load_fail(const QString &message, int timeout = 0);

        Q_SIGNAL void on_rom_loaded(std::filesystem::path);
        Q_SIGNAL void on_reload_device_list();

    private:
        void connect_slots();
        void reload_recent_roms();
        void update_controllers();
        void reload_controllers();
    };
}