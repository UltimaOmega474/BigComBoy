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
#include <array>
#include <cinttypes>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace GB {
    enum class RomSize {
        Rom32KB = 0,
        Rom64KB = 1,
        Rom128KB = 2,
        Rom256KB = 3,
        Rom512KB = 4,
        Rom1MB = 5,
        Rom2MB = 6,
        Rom4MB = 7,
        Rom8MB = 8,
        Rom1_1MB = 52,
        Rom1_2MB = 53,
        ROM1_5MB = 54
    };

    enum class RamSize {
        NoRam = 0,
        Ram2KB = 1,
        Ram8KB = 2,
        Ram32KB = 3,
        Ram128KB = 4,
        Ram64KB = 5
    };

    struct CartHeader {
        std::filesystem::path file_path;
        std::string title;

        uint8_t cgb_support = 0;
        uint8_t sgb_flag = 0;
        uint8_t mbc_type = 0;
        uint8_t region_code = 0;
        uint8_t old_license_code = 0;
        uint8_t version = 0;
        uint8_t header_checksum = 0;
        uint16_t entry_point = 0;
        uint16_t license_code = 0;
        uint16_t checksum = 0;
        RomSize rom_size = RomSize::Rom32KB;
        RamSize ram_size = RamSize::NoRam;
    };

    class Cartridge {
    public:
        explicit Cartridge(CartHeader &&header);
        virtual ~Cartridge() = default;
        Cartridge(const Cartridge &) = delete;
        Cartridge(Cartridge &&) = delete;
        auto operator=(const Cartridge &) -> Cartridge & = delete;
        auto operator=(Cartridge &&) -> Cartridge & = delete;

        auto header() const -> const CartHeader &;
        virtual auto has_battery() const -> bool = 0;

        virtual auto reset() -> void = 0;

        virtual auto init_banks(std::ifstream &rom_stream) -> void = 0;

        virtual auto read(uint16_t address) -> uint8_t = 0;
        virtual auto write(uint16_t address, uint8_t value) -> void = 0;
        virtual auto read_ram(uint16_t address) -> uint8_t = 0;
        virtual auto write_ram(uint16_t address, uint8_t value) -> void = 0;

        virtual auto save_sram_to_file() -> void = 0;
        virtual auto load_sram_from_file() -> void = 0;
        virtual auto clock(int32_t cycles) -> void = 0;

        static auto from_file(std::filesystem::path rom_path) -> std::unique_ptr<Cartridge>;
        static auto from_file_raw_ptr(std::filesystem::path rom_path) -> Cartridge *;

    protected:
        CartHeader header_;
    };

    class ROM final : public Cartridge {
    public:
        explicit ROM(CartHeader &&header);
        ~ROM() = default;
        ROM(const ROM &) = delete;
        ROM(ROM &&) = delete;
        auto operator=(const ROM &) -> ROM & = delete;
        auto operator=(ROM &&) -> ROM & = delete;

        auto has_battery() const -> bool override { return false; }

        auto reset() -> void override;
        auto init_banks(std::ifstream &rom_stream) -> void override;

        auto read(uint16_t address) -> uint8_t override;
        auto write(uint16_t address, uint8_t value) -> void override;
        auto read_ram(uint16_t address) -> uint8_t override;
        auto write_ram(uint16_t address, uint8_t value) -> void override;

        auto save_sram_to_file() -> void override;
        auto load_sram_from_file() -> void override;
        auto clock(int32_t cycles) -> void override;

    private:
        std::vector<uint8_t> rom;
    };

    class MBC1 final : public Cartridge {
    public:
        explicit MBC1(CartHeader &&header);
        ~MBC1() = default;
        MBC1(const MBC1 &) = delete;
        MBC1(MBC1 &&) = delete;
        auto operator=(const MBC1 &) -> MBC1 & = delete;
        auto operator=(MBC1 &&) -> MBC1 & = delete;

        auto has_battery() const -> bool override;

        auto reset() -> void override;
        auto init_banks(std::ifstream &rom_stream) -> void override;

        auto read(uint16_t address) -> uint8_t override;
        auto write(uint16_t address, uint8_t value) -> void override;
        auto read_ram(uint16_t address) -> uint8_t override;
        auto write_ram(uint16_t address, uint8_t value) -> void override;

        auto save_sram_to_file() -> void override;
        auto load_sram_from_file() -> void override;
        auto clock(int32_t cycles) -> void override;

    private:
        bool mode = false;
        int32_t rom_bank_num = 1;
        int32_t bank_upper_bits = 0;

        bool ram_enabled = false;
        std::array<uint8_t, 32768> eram{};
        std::vector<uint8_t> rom{};
    };

    class MBC2 final : public Cartridge {
    public:
        explicit MBC2(CartHeader &&header);
        ~MBC2() = default;
        MBC2(const MBC2 &) = delete;
        MBC2(MBC2 &&) = delete;
        auto operator=(const MBC2 &) -> MBC2 & = delete;
        auto operator=(MBC2 &&) -> MBC2 & = delete;

        auto has_battery() const -> bool override;

        auto reset() -> void override;
        auto init_banks(std::ifstream &rom_stream) -> void override;

        auto read(uint16_t address) -> uint8_t override;
        auto write(uint16_t address, uint8_t value) -> void override;
        auto read_ram(uint16_t address) -> uint8_t override;
        auto write_ram(uint16_t address, uint8_t value) -> void override;

        auto save_sram_to_file() -> void override;
        auto load_sram_from_file() -> void override;
        auto clock(int32_t cycles) -> void override;

    private:
        bool ram_enabled = false;
        uint16_t rom_bank_num = 1;

        std::array<uint8_t, 512> ram{};
        std::vector<uint8_t> rom{};
    };

    class RTCCounter {
    public:
        explicit RTCCounter(uint8_t bit_mask);
        auto get() const -> uint8_t;

        auto set(uint8_t value) -> void;
        auto increment() -> void;

    private:
        uint8_t counter = 0;
        uint8_t mask;
    };

    struct RTCTimePoint {
        uint16_t days = 0;

        RTCCounter seconds{0x3F};
        RTCCounter minutes{0x3F};
        RTCCounter hours{0x1F};
    };

    class MBC3 final : public Cartridge {
    public:
        explicit MBC3(CartHeader &&header);
        ~MBC3() = default;
        MBC3(const MBC3 &) = delete;
        MBC3(MBC3 &&) = delete;
        auto operator=(const MBC3 &) -> MBC3 & = delete;
        auto operator=(MBC3 &&) -> MBC3 & = delete;

        auto has_rtc() const -> bool;
        auto has_battery() const -> bool override;

        auto reset() -> void override;
        auto init_banks(std::ifstream &rom_stream) -> void override;

        auto read(uint16_t address) -> uint8_t override;
        auto write(uint16_t address, uint8_t value) -> void override;
        auto read_ram(uint16_t address) -> uint8_t override;
        auto write_ram(uint16_t address, uint8_t value) -> void override;

        auto save_sram_to_file() -> void override;
        auto load_sram_from_file() -> void override;
        auto clock(int32_t cycles) -> void override;

    private:
        bool ram_rtc_enabled = false;
        uint8_t latch_byte = 0;
        uint16_t rtc_ctrl = 0;
        int32_t rom_bank_num = 1;
        int32_t ram_rtc_select = 0;
        int32_t rtc_cycles = 0;

        RTCTimePoint rtc{}, shadow_rtc{};
        std::array<uint8_t, 65536> eram{};
        std::vector<uint8_t> rom{};
    };

    class MBC5 final : public Cartridge {
    public:
        explicit MBC5(CartHeader &&header);
        ~MBC5() = default;
        MBC5(const MBC5 &) = delete;
        MBC5(MBC5 &&) = delete;
        auto operator=(const MBC5 &) -> MBC5 & = delete;
        auto operator=(MBC5 &&) -> MBC5 & = delete;

        auto has_battery() const -> bool override;

        auto reset() -> void override;
        auto init_banks(std::ifstream &rom_stream) -> void override;

        auto read(uint16_t addr) -> uint8_t override;
        auto write(uint16_t addr, uint8_t value) -> void override;
        auto read_ram(uint16_t addr) -> uint8_t override;
        auto write_ram(uint16_t addr, uint8_t value) -> void override;

        auto save_sram_to_file() -> void override;
        auto load_sram_from_file() -> void override;
        auto clock(int32_t cycles) -> void override;

    private:
        bool ram_enabled = false;
        int32_t rom_bank_num = 1;
        int32_t bank_upper_bits = 0;
        int32_t ram_bank_num = 0;

        std::array<uint8_t, 131072> eram{};
        std::vector<uint8_t> rom{};
    };
}
