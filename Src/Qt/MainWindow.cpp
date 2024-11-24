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

#include "MainWindow.hpp"
#include "AboutWindow.hpp"
#include "Common/Config.hpp"
#include "DiscordRPC.hpp"
#include "EmulatorView.hpp"
#include "GB/SubWindows/SettingsWindow.hpp"
#include "Input/DeviceRegistry.hpp"
#include "Input/SDLControllerDevice.hpp"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <SDL.h>
#include <fmt/format.h>
#include <string>

namespace QtFrontend {
    MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), input_timer(),
          ui(new Ui::MainWindow), fps_counter(new QLabel(tr("--"))) {
        ui->setupUi(this);
        ui->menuLoad_Recent->setEnabled(false);

        delete ui->actionDummy_Item;
        ui->actionDummy_Item = nullptr;

        setStatusBar(new QStatusBar(this));
        statusBar()->addPermanentWidget(fps_counter, 0);


        reload_controllers();
        reload_recent_roms();

        const auto &config = Common::Config::current();
        resize(config.wsize_x, config.wsize_y);
        menuBar()->setNativeMenuBar(true);

        emulator_widget = new EmulatorView(this);
        setCentralWidget(emulator_widget);
        centralWidget()->hide();

        connect_slots();

        setWindowTitle(QString("Big ComBoy " BCB_VER));
        DiscordRPC::initialize();
    }

    MainWindow::~MainWindow() {
        delete ui;
        ui = nullptr;
    }

    auto MainWindow::showEvent(QShowEvent *event) -> void { input_timer.start(1); }

    auto MainWindow::closeEvent(QCloseEvent *event) -> void {
        auto &config = Common::Config::current();
        config.wsize_x = width();
        config.wsize_y = height();
        input_timer.stop();
        DiscordRPC::close();
    }

    auto MainWindow::keyPressEvent(QKeyEvent *event) -> void {
      //  keyboard->key_down(event->key());
        input_m.key_pressed(event->key());
        event->accept();
    }

    auto MainWindow::keyReleaseEvent(QKeyEvent *event) -> void {
       // keyboard->key_up(event->key());
        input_m.key_released(event->key());
        event->accept();
    }

    auto MainWindow::reset_action() const -> QAction * { return ui->actionReset; }

    auto MainWindow::pause_action() const -> QAction * { return ui->actionPause; }

    auto MainWindow::stop_action() const -> QAction * { return ui->actionStop; }

    auto MainWindow::fps_counter_label() const -> QLabel * { return fps_counter; }

    auto MainWindow::open_rom_file_browser() -> void {
        QFileDialog dialog;
        dialog.setFileMode(QFileDialog::FileMode::ExistingFile);
        dialog.setViewMode(QFileDialog::Detail);

        if (dialog.exec()) {
            std::string filePath = dialog.selectedFiles().first().toStdString();
            emit rom_loaded(filePath);
        }
    }

    auto MainWindow::open_rom_from_recents(const QAction *action) -> void {
        const auto filePath = action->text().toStdString();

        emit rom_loaded(filePath);
    }

    auto MainWindow::open_gb_settings() -> void {
        if (!settings) {
            int32_t menu = 0;

            if (sender() == ui->actionVideo) {
                menu = 1;
            } else if (sender() == ui->actionAudio) {
                menu = 2;
            } else if (sender() == ui->actionInput) {
                menu = 3;
            }

            settings = new SettingsWindow(this, menu);
            settings->show();
            settings->raise();
            settings->activateWindow();
            connect(settings, &QDialog::finished, this, &MainWindow::clear_settings_ptr);
        }
    }

    auto MainWindow::open_about() -> void {
        if (!about) {
            about = new AboutWindow(this);
            connect(about, &QDialog::finished, this, &MainWindow::clear_about_ptr);
        }

        about->show();
        about->raise();
        about->activateWindow();
    }

    auto MainWindow::clear_settings_ptr() -> void { settings = nullptr; }

    auto MainWindow::clear_about_ptr() -> void { about = nullptr; }

    auto MainWindow::rom_load_success(const QString &message, int timeout) -> void {
        Common::Config::current().add_rom_to_history(message.toStdString());
        statusBar()->showMessage(
            QString::fromStdString(fmt::format("'{}' Loaded successfully.", message.toStdString())),
            5000);
        reload_recent_roms();

        const QFileInfo info(message);
        DiscordRPC::new_activity(info.fileName().toStdString());
    }

    auto MainWindow::rom_load_fail(const QString &message, int timeout) const -> void {
        statusBar()->showMessage(
            QString::fromStdString(fmt::format("Unable to load '{}'", message.toStdString())),
            5000);
    }

    auto MainWindow::connect_slots() -> void {
        connect(ui->menuLoad_Recent, &QMenu::triggered, this, &MainWindow::open_rom_from_recents);
        connect(&input_timer, &QTimer::timeout, this, &MainWindow::update_controllers);
        connect(ui->actionLoad, &QAction::triggered, this, &MainWindow::open_rom_file_browser);
        connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);
        connect(ui->actionEmulation, &QAction::triggered, this, &MainWindow::open_gb_settings);
        connect(ui->actionVideo, &QAction::triggered, this, &MainWindow::open_gb_settings);
        connect(ui->actionAudio, &QAction::triggered, this, &MainWindow::open_gb_settings);
        connect(ui->actionInput, &QAction::triggered, this, &MainWindow::open_gb_settings);
        connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::open_about);
    }

    auto MainWindow::reload_recent_roms() const -> void {
        for (QAction *act : ui->menuLoad_Recent->actions()) {
            delete act;
        }
        ui->menuLoad_Recent->actions().clear();

        for (const auto &path : Common::Config::current().recent_roms) {
            auto *act = new QAction(QString::fromStdString(path), ui->menuLoad_Recent);
            ui->menuLoad_Recent->addAction(act);
        }
        ui->menuLoad_Recent->setEnabled(!ui->menuLoad_Recent->actions().empty());
    }

    auto MainWindow::update_controllers() -> void {
        SDL_Event event;

    }

    auto MainWindow::reload_controllers() -> void {

    }
}