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

#include "GBEmulatorController.hpp"
#include "Common/Config.hpp"
#include "Input/DeviceRegistry.hpp"

namespace QtFrontend
{
    GBEmulatorController::GBEmulatorController() : QObject(nullptr), sram_timer(new QTimer(this))
    {
        connect(sram_timer, &QTimer::timeout, this, &GBEmulatorController::save_sram);
    }

    GBEmulatorController::~GBEmulatorController() { sram_timer->stop(); }

    EmulationState GBEmulatorController::get_state() const { return state; }

    GB::Core &GBEmulatorController::get_core() { return core; }

    bool GBEmulatorController::try_run_frame()
    {
        using namespace std::chrono_literals;

        if (state == EmulationState::Running && audio_system.should_continue())
        {
            core.run_for_frames(1);
            return true;
        }

        return false;
    }

    void GBEmulatorController::process_input(std::array<bool, 8> &buttons)
    {
        const auto &mappings = Common::Config::Current().gameboy.input_mappings;

        for (const auto &mapping : mappings)
        {
            auto device_option = Input::DeviceRegistry::TryFindDeviceByName(mapping.device_name);

            if (device_option)
            {
                auto device = device_option.value();

                for (int i = 0; i < mapping.buttons.size(); ++i)
                {
                    if (device->is_key_down(mapping.buttons[i]))
                        buttons[i] = true;
                }
            }
        }
    }

    void GBEmulatorController::start_rom(std::filesystem::path path)
    {
        auto new_cart = GB::Cartridge::from_file(path);

        if (cart)
        {
            cart->save_sram_to_file();
            cart.reset();
        }

        if (new_cart)
        {
            const auto &emulation = Common::Config::Current().gameboy.emulation;

            cart = std::move(new_cart);

            init_by_console_type();

            audio_system.prep_for_playback(core.apu);

            state = EmulationState::Running;

            int32_t interval_seconds = emulation.sram_save_interval * 1000;

            sram_timer->stop();
            sram_timer->start(interval_seconds);

            emit on_load_success(QString::fromStdString(path.string()));
            emit on_show();
        }
        else
        {
            emit on_load_fail(QString::fromStdString(path.string()));
        }
    }

    void GBEmulatorController::copy_input(std::array<bool, 8> buttons)
    {
        using namespace GB;
        core.pad.clear_buttons();

        if (buttons[static_cast<size_t>(PadButton::Left)])
            core.pad.set_pad_state(PadButton::Left, true);
        if (buttons[static_cast<size_t>(PadButton::Right)])
            core.pad.set_pad_state(PadButton::Right, true);
        if (buttons[static_cast<size_t>(PadButton::Up)])
            core.pad.set_pad_state(PadButton::Up, true);
        if (buttons[static_cast<size_t>(PadButton::Down)])
            core.pad.set_pad_state(PadButton::Down, true);

        if (buttons[static_cast<size_t>(PadButton::B)])
            core.pad.set_pad_state(PadButton::B, true);
        if (buttons[static_cast<size_t>(PadButton::A)])
            core.pad.set_pad_state(PadButton::A, true);
        if (buttons[static_cast<size_t>(PadButton::Select)])
            core.pad.set_pad_state(PadButton::Select, true);
        if (buttons[static_cast<size_t>(PadButton::Start)])
            core.pad.set_pad_state(PadButton::Start, true);
    }

    void GBEmulatorController::set_pause(bool checked)
    {
        switch (state)
        {
        case EmulationState::Paused:
        {
            state = EmulationState::Running;
            break;
        }
        case EmulationState::Running:
        {
            state = EmulationState::Paused;
            break;
        }
        default:
        {
            break;
        }
        }
    }

    void GBEmulatorController::stop_emulation()
    {
        sram_timer->stop();
        core.initialize(nullptr);
        cart->save_sram_to_file();
        cart.reset();
        state = EmulationState::Stopped;
        emit on_hide();
    }

    void GBEmulatorController::reset_emulation()
    {
        init_by_console_type();
        audio_system.prep_for_playback(core.apu);
    }

    void GBEmulatorController::save_sram()
    {
        int32_t interval_seconds =
            Common::Config::Current().gameboy.emulation.sram_save_interval * 1000;
        cart->save_sram_to_file();

        if (sram_timer->interval() != interval_seconds)
            sram_timer->setInterval(interval_seconds);
    }

    void GBEmulatorController::init_by_console_type()
    {
        const auto &emulation = Common::Config::Current().gameboy.emulation;

        switch (emulation.console)
        {
        case GB::ConsoleType::AutoSelect:
        {
            core.initialize(cart.get());
            break;
        }
        case GB::ConsoleType::DMG:
        {
            core.initialize_with_bootstrap(cart.get(), emulation.console, emulation.dmg_bootstrap);
            break;
        }
        case GB::ConsoleType::CGB:
        {
            core.initialize_with_bootstrap(cart.get(), emulation.console, emulation.cgb_bootstrap);
            break;
        }
        }
    }
}