#pragma once
#include <QObject>
#include <QTimer>
#include <cinttypes>
#include <optional>
#include <set>
#include <tuple>

namespace QtFrontend {
    enum class QueryType {
        Keyboard,
        GamepadButton,
        GamepadAxis,
    };


    class Input final : public QObject {
        Q_OBJECT

    public:
        static constexpr int32_t GAMEPAD_BASE_IDX = 100;
        static constexpr int32_t KEYBOARD = 1;
        static constexpr int32_t GAMEPAD_AXIS_BIT = 1<< 16;
        static constexpr int32_t GAMEPAD_AXIS_THRESHOLD = 10000;

        Input();
        ~Input() override;
        auto is_key_down(int32_t device, int32_t key) const -> bool;
        auto is_any_key_down() const -> std::optional<std::tuple<int32_t, int32_t>>;

    public slots:
        auto key_pressed(int32_t key) -> void;
        auto key_released(int32_t key) -> void;

    private slots:
        static auto update_state() -> void;

    private:


        std::set<int32_t> pressed_keys;
        QTimer poll_timer{};
    };

}
