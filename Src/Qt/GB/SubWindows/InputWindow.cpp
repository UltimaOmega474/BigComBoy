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

#include "InputWindow.hpp"
#include "Cores/GB/Pad.hpp"
#include "Input/DeviceRegistry.hpp"
#include "ui_InputWindow.h"
#include <QKeyEvent>
#include <QMessageBox>
#include <QTimer>

namespace QtFrontend {
    InputWindow::InputWindow(QWidget *parent)
        : QWidget(parent), ui(new Ui::InputWindow), timer(new QTimer(this)),
          pending_input_mappings(Common::Config::current().gameboy.input_mappings) {
        ui->setupUi(this);
        reload_device_list();
        buttons[static_cast<size_t>(GB::PadButton::Left)] = ui->button_left;
        buttons[static_cast<size_t>(GB::PadButton::Right)] = ui->button_right;
        buttons[static_cast<size_t>(GB::PadButton::Up)] = ui->button_up;
        buttons[static_cast<size_t>(GB::PadButton::Down)] = ui->button_down;
        buttons[static_cast<size_t>(GB::PadButton::A)] = ui->button_a;
        buttons[static_cast<size_t>(GB::PadButton::B)] = ui->button_b;
        buttons[static_cast<size_t>(GB::PadButton::Select)] = ui->button_select;
        buttons[static_cast<size_t>(GB::PadButton::Start)] = ui->button_start;

        pages[0] = ui->page_1_radio;
        pages[1] = ui->page_2_radio;

        load_mappings_for_page(selected_page);

        connect(ui->device_select, &QComboBox::currentIndexChanged, this,
                &InputWindow::change_device_index);

        for (auto page : pages) {
            connect(page, &QRadioButton::clicked, this, &InputWindow::page_changed);
        }

        for (auto button : buttons) {
            connect(button, &QPushButton::clicked, this, &InputWindow::button_click);
        }

        connect(timer, &QTimer::timeout, this, &InputWindow::check_device_for_input);
    }

    InputWindow::~InputWindow() {
        delete ui;
        ui = nullptr;
    }

    void InputWindow::keyPressEvent(QKeyEvent *event) {
        for (auto device : Input::devices()) {
            device->key_down(event->key());
        }

        event->accept();
    }

    void InputWindow::keyReleaseEvent(QKeyEvent *event) {
        for (auto device : Input::devices()) {
            device->key_up(event->key());
        }

        event->accept();
    }

    int32_t InputWindow::get_device_index_by_name(std::string_view name) const {
        return static_cast<int32_t>(
            ui->device_select->findText(QString::fromStdString(std::string(name))));
    }

    void InputWindow::change_device_index(int index) {
        end_input_recording();

        if (index == -1) {
            for (auto button : buttons) {
                button->setEnabled(false);
            }
            return;
        }

        refresh_buttons_text();
    }

    void InputWindow::button_click() {
        auto clicked_button = dynamic_cast<QPushButton *>(sender());
        clicked_button->setText("...");

        auto resulting_iterator = std::find(buttons.begin(), buttons.end(), clicked_button);
        selected_button = static_cast<int32_t>(std::distance(buttons.begin(), resulting_iterator));

        buttons[selected_button]->setFocus();
        begin_input_recording();
    }

    void InputWindow::page_changed() {
        auto clicked_page = dynamic_cast<QRadioButton *>(sender());
        auto resulting_iterator = std::find(pages.begin(), pages.end(), clicked_page);
        selected_page = static_cast<int32_t>(std::distance(pages.begin(), resulting_iterator));
        load_mappings_for_page(selected_page);
    }

    void InputWindow::apply_changes() {
        Common::Config::current().gameboy.input_mappings = pending_input_mappings;
    }

    void InputWindow::reload_device_list() {
        ui->device_select->clear();

        for (const auto &device : Input::devices()) {
            std::string str{device->name()};
            ui->device_select->addItem(QString::fromStdString(str));
        }
    }

    void InputWindow::check_device_for_input() {
        int32_t selected_device = ui->device_select->currentIndex();

        auto device =
            Input::try_find_by_name(ui->device_select->itemText(selected_device).toStdString());

        if (device) {
            std::optional<Input::InputSource> result = device.value()->get_input_for_any_key();

            if (result) {

                auto &mapping = pending_input_mappings[selected_page];

                Input::InputSource &button_source = mapping.buttons[selected_button];
                button_source = result.value();

                mapping.device_name = device.value()->name();

                selected_button = -1;
                end_input_recording();
            }
        }
    }

    void InputWindow::load_mappings_for_page(int32_t page) {
        reload_device_list();
        end_input_recording();

        const auto &mapping = pending_input_mappings[page];

        int32_t index = get_device_index_by_name(mapping.device_name);

        if (index == -1) {
            QMessageBox msgBox{};
            auto msg = QString("The device: '%1' is not connected.\nIf you change any mappings "
                               "for this entry, "
                               "the original device configuration will be overwritten.")
                           .arg(QString::fromStdString(mapping.device_name));
            msgBox.setText(msg);
            msgBox.exec();
        } else {
            ui->device_select->setCurrentIndex(index);
        }

        refresh_buttons_text();
    }

    void InputWindow::begin_input_recording() {
        emit set_tab_focus(false);

        for (auto button : buttons) {
            button->setFocusPolicy(Qt::NoFocus);
            button->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        }

        grabKeyboard();
        timer->start(1);
    }

    void InputWindow::end_input_recording() {
        emit set_tab_focus(true);

        for (auto button : buttons) {
            button->setFocusPolicy(Qt::StrongFocus);
            button->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        }

        releaseKeyboard();
        timer->stop();
        selected_button = -1;
        refresh_buttons_text();
    }

    void InputWindow::refresh_buttons_text() {
        const auto &devices = Input::devices();
        int32_t selected_device = ui->device_select->currentIndex();

        if (devices.empty() || selected_device == -1) {
            for (auto button : buttons) {
                button->setEnabled(false);
            }
            return;
        }

        const auto &mapping = pending_input_mappings[selected_page];
        const auto device = Input::try_find_by_name(mapping.device_name);

        for (int i = 0; i < buttons.size(); ++i) {
            if (device && mapping.device_name == device.value()->name()) {
                std::string name = device.value()->key_to_str(mapping.buttons[i]);

                buttons[i]->setText(QString::fromStdString(name));

            } else {
                buttons[i]->setText("???");
            }

            buttons[i]->setEnabled(true);
        }
    }
}