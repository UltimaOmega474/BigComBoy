#pragma once
#include <QObject>
#include <cinttypes>
#include <span>
#include <filesystem>

namespace QtFrontend {

    class EmulatorContext {
    public:
       virtual ~EmulatorContext() = default;

        virtual auto framebuffer_pixels() -> std::span<uint8_t> = 0;
        virtual auto screen_size() -> std::pair<int32_t, int32_t> = 0;
        virtual auto run() -> void = 0;
        virtual auto process_input(std::array<bool, 8> &buttons) -> void = 0;

    public slots:
        virtual auto load_rom_file(const std::filesystem::path& path) -> void = 0;
        virtual auto load_state_file(const std::filesystem::path& path) -> void = 0;
        virtual auto save_state_file(const std::filesystem::path& path) -> void = 0;
        virtual auto reset() -> void = 0;
        virtual auto pause(bool checked) -> void = 0;
        virtual auto stop() -> void = 0;
        virtual auto step_into_cycle() -> void = 0;
        virtual auto step_into_instruction() -> void = 0;
        virtual auto step_over_instruction() -> void = 0;


    signals:
        auto load_result(bool is_successful, const QString &message) -> void;
        auto load_successful(const QString &message, int timeout = 0) -> void;
        auto load_failure(const QString &message, int timeout = 0) -> void;
        auto hide_central_widget() -> void;
    };

}
