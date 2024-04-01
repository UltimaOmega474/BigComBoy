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

class QAbstractButton;

namespace Ui {
    class VideoWindow;
}

namespace QtFrontend {
    class VideoWindow : public QWidget {
        Q_OBJECT

    public:
        explicit VideoWindow(QWidget *parent = nullptr);
        ~VideoWindow();
        VideoWindow(const VideoWindow &) = delete;
        VideoWindow(VideoWindow &&) = delete;
        VideoWindow &operator=(const VideoWindow &) = delete;
        VideoWindow &operator=(VideoWindow &&) = delete;

        Q_SLOT void apply_changes();
        Q_SLOT void set_blending_enabled(bool checked);
        Q_SLOT void select_scaling_mode(QAbstractButton *btn);

    private:
        Ui::VideoWindow *ui = nullptr;
        Common::GBConfig::VideoData video;
    };
}