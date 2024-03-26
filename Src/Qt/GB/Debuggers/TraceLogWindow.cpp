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

#include "TraceLogWindow.hpp"
#include "../GBEmulatorController.hpp"
#include "Cores/GB/Disassembler.hpp"
#include "ui_TraceLogWindow.h"
#include <QFontDatabase>
#include <QMutexLocker>
#include <QTextEdit>
#include <format>

namespace QtFrontend
{
    TraceLogWindow::TraceLogWindow(QWidget *parent, GBEmulatorController *gb_emulator)
        : QDialog(parent), ui(new Ui::TraceLogWindow), gb_emulator(gb_emulator)
    {
        ui->setupUi(this);
        setAttribute(Qt::WA_DeleteOnClose);
        connect_slots();

        label_text.reserve(gb_emulator->get_core().logger.get_line_limit() * 32);
    }

    TraceLogWindow::~TraceLogWindow()
    {
        delete ui;
        ui = nullptr;
    }

    void TraceLogWindow::update_trace_log(std::deque<GB::Instruction> &history)
    {
        label_text.clear();
        for (auto &ins : history)
        {
            std::string output_text = std::format("${:04X}: ", ins.pc);

            std::string bytes_str;
            for (size_t j = 0; j < ins.bytes.size(); ++j)
            {
                if (j < ins.len)
                    bytes_str += std::format("{:02X} ", ins.bytes[j]);
                else
                    bytes_str += "   ";
            }
            output_text += bytes_str;

            output_text += std::format("{}\n", GB::DecodeInstruction(ins.bytes));

            label_text += QString::fromStdString(output_text);
        }

        ui->log_view->setPlainText(label_text);
    }

    void TraceLogWindow::connect_slots()
    {
        connect(ui->begin_btn, &QPushButton::clicked, gb_emulator,
                &GBEmulatorController::begin_trace);
        connect(ui->end_btn, &QPushButton::clicked, gb_emulator, &GBEmulatorController::end_trace);

        qRegisterMetaType<std::deque<GB::Instruction>>("std::deque<GB::Instruction>&");
        connect(gb_emulator, &GBEmulatorController::upload_trace_log, this,
                &TraceLogWindow::update_trace_log);
    }
}