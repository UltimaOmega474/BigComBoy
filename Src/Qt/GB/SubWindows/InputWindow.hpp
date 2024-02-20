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

class QTimer;
class QPushButton;
class QRadioButton;
class QAbstractButton;

namespace Ui
{
    class InputWindow;
}

namespace QtFrontend
{
    class InputWindow : public QWidget
    {
        Q_OBJECT

        Ui::InputWindow *ui;
        QTimer *timer;
        std::array<QPushButton *, 8> buttons{};
        std::array<QRadioButton *, 2> pages{};

        int32_t selected_page = 0, selected_button = -1;

        std::array<Common::GBGamepadConfig, 2> pending_input_mappings;

    public:
        explicit InputWindow(QWidget *parent = nullptr);
        ~InputWindow() override;
        void keyPressEvent(QKeyEvent *event) override;
        void keyReleaseEvent(QKeyEvent *event) override;

        Q_SLOT void apply_changes();
        void on_device_index_changed(int index);
        void on_button_clicked();
        void on_page_changed();

        int32_t get_device_index_by_name(std::string_view name) const;
        Q_SLOT void reload_device_list();
        void check_device_for_input();
        void load_mappings_for_page(int32_t page);
        void begin_input_recording();
        void end_input_recording();
        void refresh_buttons_text();

        Q_SIGNAL void on_set_tab_focus(bool allowed);
    };
}