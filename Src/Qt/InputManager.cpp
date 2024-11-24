
#include "InputManager.hpp"
#include <SDL.h>

namespace QtFrontend {

    Input::Input() {
        connect(&poll_timer, &QTimer::timeout, &Input::update_state);
        poll_timer.start(5);
    }

    Input::~Input() { poll_timer.stop(); }

    auto Input::is_key_down(const int32_t device, const int32_t key) const -> bool {
        if (device >= GAMEPAD_BASE_IDX) {
            const auto sdl_idx = device - GAMEPAD_BASE_IDX;
            if (SDL_IsGameController(sdl_idx)) {
                SDL_GameController *controller = SDL_GameControllerOpen(sdl_idx);

                if (key & GAMEPAD_AXIS_BIT) {
                    const auto pressure = SDL_GameControllerGetAxis(
                        controller, static_cast<SDL_GameControllerAxis>(key & 0xFFFF));
                    return (pressure > GAMEPAD_AXIS_THRESHOLD) ||
                           (pressure < -GAMEPAD_AXIS_THRESHOLD);
                }

                return static_cast<bool>(SDL_GameControllerGetButton(
                    controller, static_cast<SDL_GameControllerButton>(key)));
            }
        } else {
            return pressed_keys.contains(key);
        }

        return false;
    }

    auto Input::is_any_key_down() const -> std::optional<std::tuple<int32_t, int32_t>> {
        if (!pressed_keys.empty()) {
            return {{KEYBOARD, *pressed_keys.begin()}};
        }

        for (int32_t i = 0; i < SDL_NumJoysticks(); ++i) {
            if (SDL_IsGameController(i)) {
                SDL_GameController *controller = SDL_GameControllerOpen(i);
                for (int j = 0; j < SDL_CONTROLLER_BUTTON_MAX; ++j) {
                    const auto button = static_cast<SDL_GameControllerButton>(j);
                    const auto state =
                        static_cast<bool>(SDL_GameControllerGetButton(controller, button));
                    if (state) {
                        return {{GAMEPAD_BASE_IDX + j, button}};
                    }
                }

                for (int j = 0; j < SDL_CONTROLLER_AXIS_MAX; ++j) {
                    const auto axis = static_cast<SDL_GameControllerAxis>(j);
                    const auto pressure = SDL_GameControllerGetAxis(controller, axis);
                    const bool state =
                        (pressure > GAMEPAD_AXIS_THRESHOLD) || (pressure < -GAMEPAD_AXIS_THRESHOLD);

                    if (state) {
                        return {{GAMEPAD_BASE_IDX + j, GAMEPAD_AXIS_BIT | axis}};
                    }
                }
            }
        }

        return std::nullopt;
    }

    auto Input::key_pressed(const int32_t key) -> void { pressed_keys.insert(key); }

    auto Input::key_released(const int32_t key) -> void { pressed_keys.erase(key); }

    auto Input::update_state() -> void {
        SDL_GameControllerUpdate();
        SDL_PumpEvents();
    }
}