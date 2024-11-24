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
#include "InputManager.hpp"
#include <QMainWindow>
#include <QTimer>
#include <filesystem>
#include <memory>

namespace Ui {
    class MainWindow;
}

class QAction;
class QLabel;

namespace Input {
    class InputDevice;
}

namespace QtFrontend {

    class EmulatorView;
    class SettingsWindow;
    class AboutWindow;

    class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow() override;
        MainWindow(const MainWindow &) = delete;
        MainWindow(MainWindow &&) = delete;
        MainWindow &operator=(const MainWindow &) = delete;
        MainWindow &operator=(MainWindow &&) = delete;

        auto showEvent(QShowEvent *) -> void override;
        auto closeEvent(QCloseEvent *event) -> void override;
        auto keyPressEvent(QKeyEvent *event) -> void override;
        auto keyReleaseEvent(QKeyEvent *event) -> void override;

        auto reset_action() const -> QAction *;
        auto pause_action() const -> QAction *;
        auto stop_action() const -> QAction *;
        auto fps_counter_label() const -> QLabel *;
    public slots:
        auto open_rom_file_browser() -> void;
        auto open_rom_from_recents(const QAction *action) -> void;
        auto open_gb_settings() -> void;
        auto open_about() -> void;
        auto clear_settings_ptr() -> void;
        auto clear_about_ptr() -> void;
        auto rom_load_success(const QString &message, int timeout = 0) -> void;
        auto rom_load_fail(const QString &message, int timeout = 0) const -> void;

    signals:
        auto rom_loaded(std::filesystem::path) -> void;
        auto reload_device_list() -> void;

    private:
        auto connect_slots() -> void;
        auto reload_recent_roms() const -> void;
        auto update_controllers() -> void;
        auto reload_controllers() -> void;

        QTimer input_timer;
        Input input_m;

        Ui::MainWindow *ui;
        SettingsWindow *settings = nullptr;
        AboutWindow *about = nullptr;
        QLabel *fps_counter = nullptr;
        EmulatorView *emulator_widget = nullptr;
    };
}