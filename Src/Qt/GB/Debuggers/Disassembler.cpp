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

#include "Disassembler.hpp"
#include "../GBEmulatorController.hpp"
#include "Cores/GB/Core.hpp"
#include "Cores/GB/Disassembler.hpp"
#include "ui_Disassembler.h"
#include <QFontDatabase>
#include <QHeaderView>
#include <QMutexLocker>
#include <QTableWidgetItem>
#include <QtCore/qnamespace.h>

namespace QtFrontend
{

    Disassembler::Disassembler(QWidget *parent, GBEmulatorController *gb_emulator)
        : QDialog(parent), ui(new Ui::Disassembler), gb_emulator(gb_emulator)
    {
        ui->setupUi(this);

        auto p = ui->label->palette();
        ui->label->setBackgroundRole(QPalette::Base);
        ui->label->setPalette(p);
        ui->label->setAutoFillBackground(true);
        auto f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        f.setPointSize(14);
        ui->label->setFont(f);

        connect(gb_emulator, &GBEmulatorController::on_update_debuggers, this,
                &Disassembler::update_data);

        QMutexLocker<QMutex> lock(gb_emulator->get_mutex());
        auto &core = gb_emulator->get_core();

        core.enable_debug_tools = true;
    }

    Disassembler::~Disassembler()
    {
        delete ui;
        ui = nullptr;

        QMutexLocker<QMutex> lock(gb_emulator->get_mutex());

        auto &core = gb_emulator->get_core();
        core.enable_debug_tools = false;
    }

    void Disassembler::update_data()
    {
        QMutexLocker<QMutex> lock(gb_emulator->get_mutex());

        auto &core = gb_emulator->get_core();
        const auto &history = core.disassembler.get_history();
        const auto &upcoming = core.disassembler.get_upcoming();

        QString label_text;

        for (const auto &ins : history)
        {
            std::string output_text = std::format("${:04X}: ", ins.program_counter);

            std::string bytes_str;
            for (size_t j = 0; j < ins.bytes.size(); ++j)
            {
                if (j < ins.len)
                    bytes_str += std::format("{:02X} ", ins.bytes[j]);
                else
                    bytes_str += "   ";
            }

            output_text += bytes_str;

            output_text += std::format("{}<br>", GB::Disassembler::DecodeInstruction(ins.bytes));
            label_text += QString::fromStdString(output_text);
        }

        for (const auto &ins : upcoming)
        {
            std::string output_text = std::format("${:04X}: ", ins.program_counter);

            std::string bytes_str;
            for (size_t j = 0; j < ins.bytes.size(); ++j)
            {
                if (j < ins.len)
                    bytes_str += std::format("{:02X} ", ins.bytes[j]);
                else
                    bytes_str += "   ";
            }

            output_text += bytes_str;

            output_text += std::format("{}", GB::Disassembler::DecodeInstruction(ins.bytes));

            label_text += QString("<span style='background-color: blue;'>%1</span><br>")
                              .arg(QString::fromStdString(output_text));
        }

        ui->label->setTextFormat(Qt::RichText);
        ui->label->setText(label_text);
    }

}