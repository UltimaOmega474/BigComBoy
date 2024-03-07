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
#include "Common/Config.hpp"
#include <QWidget>
#include <string_view>

namespace Ui
{
    class EmulationWindow;
}

class QAbstractButton;

namespace QtFrontend
{
    class EmulationWindow : public QWidget
    {
        Q_OBJECT

        Ui::EmulationWindow *ui = nullptr;
        Common::GBConfig::EmulationData emulation;

    public:
        explicit EmulationWindow(QWidget *parent = nullptr);
        ~EmulationWindow() override;

        Q_SLOT void apply_changes();
        Q_SLOT void select_bootrom();
        Q_SLOT void set_allow_sram(bool checked);
        Q_SLOT void change_interval(int32_t value);
        Q_SLOT void set_console(QAbstractButton *btn);
        Q_SLOT void boot_path_changed(const QString &path);

        Q_SIGNAL void set_boot_path_text(const QString &text);
    };
}