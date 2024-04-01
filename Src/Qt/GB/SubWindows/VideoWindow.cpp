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

#include "VideoWindow.hpp"
#include "ui_VideoWindow.h"
#include <QtWidgets/qcheckbox.h>

namespace QtFrontend {
    VideoWindow::VideoWindow(QWidget *parent)
        : QWidget(parent), ui(new Ui::VideoWindow), video(Common::Config::Current().gameboy.video) {
        ui->setupUi(this);

        connect(ui->buttonGroup, &QButtonGroup::buttonClicked, this,
                &VideoWindow::select_scaling_mode);
        connect(ui->blending_box, &QCheckBox::clicked, this, &VideoWindow::set_blending_enabled);

        ui->blending_box->setChecked(video.frame_blending);

        if (video.smooth_scaling) {
            ui->smooth_radio->setChecked(true);
        } else {
            ui->pixelated_radio->setChecked(true);
        }
    }

    VideoWindow::~VideoWindow() {
        delete ui;
        ui = nullptr;
    }

    void VideoWindow::apply_changes() { Common::Config::Current().gameboy.video = video; }

    void VideoWindow::set_blending_enabled(bool checked) { video.frame_blending = checked; }

    void VideoWindow::select_scaling_mode(QAbstractButton *btn) {
        video.smooth_scaling = (btn == ui->smooth_radio);
    }

}