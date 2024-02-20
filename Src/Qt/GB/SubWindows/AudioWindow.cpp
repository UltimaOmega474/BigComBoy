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

#include "AudioWindow.hpp"
#include "ui_AudioWindow.h"

namespace QtFrontend
{

    AudioWindow::AudioWindow(QWidget *parent)
        : QWidget(parent), ui(new Ui::AudioWindow), audio(Common::Config::Current().gameboy.audio)
    {
        ui->setupUi(this);

        connect_vol_controls(ui->master_slider, ui->master_spin);
        connect_vol_controls(ui->square1_slider, ui->square1_spin);
        connect_vol_controls(ui->square2_slider, ui->square2_spin);
        connect_vol_controls(ui->wave_slider, ui->wave_spin);
        connect_vol_controls(ui->noise_slider, ui->noise_spin);

        ui->master_slider->setValue(audio.volume);
        ui->square1_slider->setValue(audio.square1);
        ui->square2_slider->setValue(audio.square2);
        ui->wave_slider->setValue(audio.wave);
        ui->noise_slider->setValue(audio.noise);
    }

    AudioWindow::~AudioWindow()
    {
        delete ui;
        ui = nullptr;
    }
    void AudioWindow::connect_vol_controls(QSlider *slider, QSpinBox *spin)
    {
        connect(slider, &QSlider::valueChanged, spin, &QSpinBox::setValue);
        connect(spin, &QSpinBox::valueChanged, slider, &QSlider::setValue);
        connect(slider, &QSlider::valueChanged, this, &AudioWindow::change_volume);
        connect(spin, &QSpinBox::valueChanged, this, &AudioWindow::change_volume);
    }

    void AudioWindow::apply_changes() { Common::Config::Current().gameboy.audio = audio; }

    void AudioWindow::change_volume(int32_t volume)
    {
        QObject *obj = sender();

        if (obj == ui->master_slider || obj == ui->master_spin)
        {
            audio.volume = volume;
        }
        else if (obj == ui->square1_slider || obj == ui->square1_spin)
        {
            audio.square1 = volume;
        }
        else if (obj == ui->square2_slider || obj == ui->square2_spin)
        {
            audio.square2 = volume;
        }
        else if (obj == ui->wave_slider || obj == ui->wave_spin)
        {
            audio.wave = volume;
        }
        else if (obj == ui->noise_slider || obj == ui->noise_spin)
        {
            audio.noise = volume;
        }
    }
}