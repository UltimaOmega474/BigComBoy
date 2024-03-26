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
#include "Cores/GB/Trace.hpp"
#include <QDialog>
#include <cinttypes>
#include <deque>

namespace Ui
{
    class TraceLogWindow;
}

namespace QtFrontend
{
    class GBEmulatorController;

    class TraceLogWindow : public QDialog
    {
        Ui::TraceLogWindow *ui = nullptr;
        GBEmulatorController *gb_emulator = nullptr;
        QString label_text;

    public:
        explicit TraceLogWindow(QWidget *parent, GBEmulatorController *gb_emulator);
        ~TraceLogWindow() override;

        Q_SLOT void update_trace_log(std::deque<GB::Instruction> &logger);

    private:
        void connect_slots();
    };
}