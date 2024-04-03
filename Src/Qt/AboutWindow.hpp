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

namespace Ui {
    class AboutWindow;
}

namespace QtFrontend {
    class AboutWindow : public QDialog {
        Q_OBJECT

    public:
        explicit AboutWindow(QWidget *parent = nullptr);
        ~AboutWindow();
        AboutWindow(const AboutWindow &) = delete;
        AboutWindow(AboutWindow &&) = delete;
        AboutWindow &operator=(const AboutWindow &) = delete;
        AboutWindow &operator=(AboutWindow &&) = delete;

    private:
        Ui::AboutWindow *ui = nullptr;
    };
}