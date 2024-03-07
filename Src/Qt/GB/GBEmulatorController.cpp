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
#include "GL/Renderer.hpp"
#include "Input/DeviceRegistry.hpp"

namespace QtFrontend
{
    GBEmulatorController::GBEmulatorController() : QObject(nullptr), sram_timer(new QTimer(this))
    {
        for (auto &texture : textures)
        {
            texture = GL::CreateTexture(GB::LCD_WIDTH, GB::LCD_HEIGHT);
        }

        connect(sram_timer, &QTimer::timeout, this, &GBEmulatorController::save_sram);
    }

    GBEmulatorController::~GBEmulatorController()
    {
        for (auto texture : textures)
        {
            GL::DestroyTexture(texture);
        }
        sram_timer->stop();
    }

    EmulationState GBEmulatorController::get_state() const { return state; }

    void GBEmulatorController::update()
    {
        using namespace std::chrono_literals;

        if (state == EmulationState::Running && audio_system.should_continue())
        {
            core.run_for_frames(1);
            update_textures();
        }
    }

    void GBEmulatorController::draw_scene(GL::Renderer *renderer, float screen_width,
                                          float screen_height)
    {
        if (state == EmulationState::Stopped)
            return;

        const auto use_frame_blending = Common::Config::Current().gameboy.video.frame_blending;

        auto [final_width, final_height] = Common::Math::FitToAspectRatio(
            screen_width, screen_height, GB::LCD_WIDTH, GB::LCD_HEIGHT);

        float final_x = screen_width / 2.0f - (final_width / 2);

        GL::SetAlphaBlend();

        renderer->draw_image(textures[0], final_x, 0, final_width, final_height);

        if (use_frame_blending)
        {
            float alpha = 0.5f;
            for (size_t i = 1; i < textures.size(); ++i)
            {
                auto fbtexture = textures[i];
                auto color = GL::Color{1.0f, 1.0f, 1.0f, alpha};
                renderer->draw_image(fbtexture, final_x, 0, final_width, final_height, color);
            }
        }
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

        if (new_cart)
        {
            const auto &emulation = Common::Config::Current().gameboy.emulation;
            cart.reset();
            cart = std::move(new_cart);

            core.load_boot_rom_from_file(emulation.boot_rom_path);
            core.initialize(cart.get(), emulation.skip_boot_rom);
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
        core.initialize(nullptr, true);
        cart->save_sram_to_file();
        cart.reset();
        state = EmulationState::Stopped;
        emit on_hide();
    }

    void GBEmulatorController::reset_emulation()
    {
        const auto &emulation = Common::Config::Current().gameboy.emulation;

        core.load_boot_rom_from_file(emulation.boot_rom_path);
        core.initialize(cart.get(), emulation.skip_boot_rom);
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

    void GBEmulatorController::update_textures()
    {
        const auto &config = Common::Config::Current().gameboy.video;

        if (config.frame_blending)
        {
            for (size_t i = (framebuffers.size() - 1); i > 0; --i)
            {
                framebuffers[i] = framebuffers[i - 1];

                GL::UpdateTextureData(textures[i], GB::LCD_WIDTH, GB::LCD_HEIGHT, framebuffers[i]);

                GL::SetTextureFilter(textures[i], config.smooth_scaling ? GL_LINEAR : GL_NEAREST);
            }
        }

        framebuffers[0] = core.ppu.framebuffer_complete;
        GL::UpdateTextureData(textures[0], GB::LCD_WIDTH, GB::LCD_HEIGHT, framebuffers[0]);
        GL::SetTextureFilter(textures[0], config.smooth_scaling ? GL_LINEAR : GL_NEAREST);
    }
}