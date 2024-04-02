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

#include "SettingsWindow.hpp"
#include "Common/Config.hpp"
#include "InputWindow.hpp"
#include "Qt/GB/SubWindows/AudioWindow.hpp"
#include "Qt/GB/SubWindows/EmulationWindow.hpp"
#include "Qt/GB/SubWindows/VideoWindow.hpp"
#include "Qt/Paths.hpp"
#include "ui_SettingsWindow.h"
#include <QCoreApplication>
#include <QDialogButtonBox>

namespace QtFrontend {
    SettingsWindow::SettingsWindow(QWidget *parent, int32_t selected_index)
        : QDialog(parent), ui(new Ui::SettingsWindow) {
        ui->setupUi(this);

        connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

        connect(ui->input_tab, &InputWindow::set_tab_focus, this, &SettingsWindow::set_tab_focus);

        connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &SettingsWindow::dialog_clicked);

        connect(this, &SettingsWindow::apply_changes_to_tabs, ui->emulation_tab,
                &EmulationWindow::apply_changes);
        connect(this, &SettingsWindow::apply_changes_to_tabs, ui->video_tab,
                &VideoWindow::apply_changes);
        connect(this, &SettingsWindow::apply_changes_to_tabs, ui->audio_tab,
                &AudioWindow::apply_changes);
        connect(this, &SettingsWindow::apply_changes_to_tabs, ui->input_tab,
                &InputWindow::apply_changes);

        setAttribute(Qt::WA_DeleteOnClose);
        setFixedSize(sizeHint());
        ui->tabWidget->setCurrentIndex(selected_index);
    }

    SettingsWindow::~SettingsWindow() {
        delete ui;
        ui = nullptr;
    }

    bool SettingsWindow::focusNextPrevChild(bool next) {
        if (allow_tab_focus) {
            return QWidget::focusNextPrevChild(next);
        } else {
            return false;
        }
    }

    void SettingsWindow::dialog_clicked(QAbstractButton *button) {
        QString name = button->text();

        if (name == "Apply" || name == "OK") {
            emit apply_changes_to_tabs();

            Common::Config::current().write_to_file(QtFrontend::Paths::ConfigLocation());
        }
    }

    void SettingsWindow::set_tab_focus(bool allowed) { allow_tab_focus = allowed; }

}