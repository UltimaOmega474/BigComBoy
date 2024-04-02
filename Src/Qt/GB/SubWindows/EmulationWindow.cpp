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

#include "EmulationWindow.hpp"
#include "Cores/GB/Constants.hpp"
#include "ui_EmulationWindow.h"
#include <QFileDialog>
#include <QPushButton>
#include <QRadioButton>
#include <array>

namespace QtFrontend {
    EmulationWindow::EmulationWindow(QWidget *parent)
        : QWidget(parent), ui(new Ui::EmulationWindow),
          emulation(Common::Config::current().gameboy.emulation) {
        ui->setupUi(this);

        connect(ui->console_btn_group, &QButtonGroup::buttonClicked, this,
                &EmulationWindow::set_console);

        connect(ui->gb_browse_btn, &QPushButton::clicked, this, &EmulationWindow::select_bootrom);
        connect(ui->gb_boot_path, &QLineEdit::textChanged, this,
                &EmulationWindow::boot_path_changed);
        connect(this, &EmulationWindow::set_boot_path_text, ui->gb_boot_path, &QLineEdit::setText);

        connect(ui->gbc_browse_btn, &QPushButton::clicked, this, &EmulationWindow::select_bootrom);
        connect(ui->gbc_boot_path, &QLineEdit::textChanged, this,
                &EmulationWindow::boot_path_changed);
        connect(this, &EmulationWindow::set_boot_path_text, ui->gbc_boot_path, &QLineEdit::setText);

        connect(ui->allow_sram, &QCheckBox::clicked, this, &EmulationWindow::set_allow_sram);
        connect(ui->sram_interval, &QSpinBox::valueChanged, this,
                &EmulationWindow::change_interval);

        ui->allow_sram->setChecked(emulation.allow_sram_saving);
        ui->sram_interval->setValue(emulation.sram_save_interval);

        std::array<QRadioButton *, 3> btns{

            ui->gb_btn,
            ui->gbc_btn,
            ui->auto_btn,
        };

        btns[static_cast<size_t>(emulation.console)]->setChecked(true);

        ui->gb_boot_path->setText(QString::fromStdString(emulation.dmg_bootstrap.string()));
        ui->gbc_boot_path->setText(QString::fromStdString(emulation.cgb_bootstrap.string()));
    }

    EmulationWindow::~EmulationWindow() {
        delete ui;
        ui = nullptr;
    }

    void EmulationWindow::apply_changes() {
        Common::Config::current().gameboy.emulation = emulation;
    }

    void EmulationWindow::select_bootrom() {
        QFileDialog dialog;
        dialog.setFileMode(QFileDialog::FileMode::ExistingFile);
        dialog.setViewMode(QFileDialog::Detail);

        if (dialog.exec()) {
            QString path = dialog.selectedFiles().first();

            if (sender() == ui->gb_browse_btn) {
                emulation.dmg_bootstrap = path.toStdString();
                ui->gb_boot_path->setText(path);
            } else {
                emulation.cgb_bootstrap = path.toStdString();
                ui->gbc_boot_path->setText(path);
            }
        }
    }

    void EmulationWindow::set_allow_sram(bool checked) { emulation.allow_sram_saving = checked; }

    void EmulationWindow::change_interval(int32_t value) { emulation.sram_save_interval = value; }

    void EmulationWindow::set_console(QAbstractButton *btn) {
        if (btn == ui->auto_btn) {
            emulation.console = GB::ConsoleType::AutoSelect;
        } else if (btn == ui->gb_btn) {
            emulation.console = GB::ConsoleType::DMG;
        } else if (btn == ui->gbc_btn) {
            emulation.console = GB::ConsoleType::CGB;
        }
    }

    void EmulationWindow::boot_path_changed(const QString &path) {
        if (sender() == ui->gb_boot_path) {
            emulation.dmg_bootstrap = path.toStdString();
        } else {
            emulation.cgb_bootstrap = path.toStdString();
        }
    }

}