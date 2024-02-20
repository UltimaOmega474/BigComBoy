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
#include <QDialog>

class QAbstractButton;

namespace Ui
{
    class SettingsWindow;
}

namespace QtFrontend
{
    class SettingsWindow : public QDialog
    {
        Q_OBJECT

        Ui::SettingsWindow *ui;
        bool allow_tab_focus = true;

    public:
        explicit SettingsWindow(QWidget *parent = nullptr, int32_t selected_index = 0);
        ~SettingsWindow() override;

        bool focusNextPrevChild(bool next) override;

        Q_SLOT void dialog_clicked(QAbstractButton *button);
        Q_SLOT void set_tab_focus(bool allowed);
        Q_SIGNAL void on_apply_changes_to_tabs();
    };
}