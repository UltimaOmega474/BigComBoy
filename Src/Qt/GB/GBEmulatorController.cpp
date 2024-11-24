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

namespace QtFrontend {
    GBEmulatorController::GBEmulatorController() : QObject(nullptr), sram_timer(new QTimer(this)) {
       // connect(sram_timer, &QTimer::timeout, this, &GBEmulatorController::save_sram);
    }

    GBEmulatorController::~GBEmulatorController() { sram_timer->stop(); }

    EmulationState GBEmulatorController::get_state() const { return state; }

    GB::Core &GBEmulatorController::get_core() { return core; }

    auto GBEmulatorController::run() -> void {
        using namespace std::chrono_literals;

        if (state == EmulationState::Running && audio_system.should_continue()) {
            core.run_for_frames(1);
        }
    }

    auto GBEmulatorController::process_input(std::array<bool, 8> &buttons) -> void {
        const auto &mappings = Common::Config::current().gameboy.input_mappings;

        for (const auto &mapping : mappings) {
            auto device_option = Input::try_find_by_name(mapping.device_name);

            if (device_option) {
                const auto device = device_option.value();

                for (int i = 0; i < mapping.buttons.size(); ++i) {
                    if (device->is_key_down(mapping.buttons[i])) {
                        buttons[i] = true;
                    }
                }
            }
        }
    }

    void GBEmulatorController::init_by_console_type() {
        const auto &emulation = Common::Config::current().gameboy.emulation;

        switch (emulation.console) {
        case GB::ConsoleType::AutoSelect: {
            core.initialize(cart.get());
            break;
        }
        case GB::ConsoleType::DMG: {
            core.initialize_with_bootstrap(cart.get(), emulation.console, emulation.dmg_bootstrap);
            break;
        }
        case GB::ConsoleType::CGB: {
            core.initialize_with_bootstrap(cart.get(), emulation.console, emulation.cgb_bootstrap);
            break;
        }
        }
    }
}