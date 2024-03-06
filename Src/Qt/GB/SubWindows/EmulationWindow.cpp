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
#include "ui_EmulationWindow.h"
#include <QFileDialog>
#include <QPushButton>

namespace QtFrontend
{
    EmulationWindow::EmulationWindow(QWidget *parent)
        : QWidget(parent), ui(new Ui::EmulationWindow),
          emulation(Common::Config::Current().gameboy.emulation)
    {
        ui->setupUi(this);
        connect(ui->skip_boot_btn, &QCheckBox::clicked, this, &EmulationWindow::set_skip_boot);
        connect(ui->allow_sram, &QCheckBox::clicked, this, &EmulationWindow::set_allow_sram);
        connect(ui->browse_btn, &QPushButton::clicked, this, &EmulationWindow::select_bootrom);
        connect(this, &EmulationWindow::set_boot_path_text, ui->boot_path, &QLineEdit::setText);
        connect(ui->sram_interval, &QSpinBox::valueChanged, this,
                &EmulationWindow::change_interval);
        connect(ui->boot_path, &QLineEdit::textChanged, this, &EmulationWindow::boot_path_changed);

        ui->skip_boot_btn->setChecked(emulation.skip_boot_rom);
        ui->allow_sram->setChecked(emulation.allow_sram_saving);
        ui->sram_interval->setValue(emulation.sram_save_interval);
        ui->boot_path->setText(QString::fromStdString(emulation.boot_rom_path));
    }

    EmulationWindow::~EmulationWindow()
    {
        delete ui;
        ui = nullptr;
    }

    void EmulationWindow::apply_changes()
    {
        Common::Config::Current().gameboy.emulation = emulation;
    }

    void EmulationWindow::select_bootrom()
    {
        QFileDialog dialog;
        dialog.setFileMode(QFileDialog::FileMode::ExistingFile);
        dialog.setViewMode(QFileDialog::Detail);

        if (dialog.exec())
        {
            QString path = dialog.selectedFiles().first();

            emulation.boot_rom_path = path.toStdString();
            emit set_boot_path_text(path);
        }
    }

    void EmulationWindow::set_skip_boot(bool checked) { emulation.skip_boot_rom = checked; }

    void EmulationWindow::set_allow_sram(bool checked) { emulation.allow_sram_saving = checked; }

    void EmulationWindow::change_interval(int32_t value) { emulation.sram_save_interval = value; }

    void EmulationWindow::boot_path_changed(const QString &path)
    {
        emulation.boot_rom_path = path.toStdString();
    }

}