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

namespace GB
{
    enum RomSizes
    {
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

    enum RamSizes
    {
        NoRam = 0,
        Ram2KB = 1,
        Ram8KB = 2,
        Ram32KB = 3,
        Ram128KB = 4,
        Ram64KB = 5
    };

    struct CartHeader
    {
        std::filesystem::path file_path;
        std::string title;

        uint8_t cgb_support = 0;
        uint8_t sgb_flag = 0, mbc_type = 0;
        uint8_t rom_size = 0, ram_size = 0;
        uint8_t region_code = 0, old_license_code = 0;
        uint8_t version = 0, header_checksum = 0;
        uint16_t entry_point = 0, license_code = 0, checksum = 0;
    };

    class Cartridge
    {
    public:
        CartHeader header;

        Cartridge(CartHeader &&header);
        virtual ~Cartridge() = default;

        virtual bool has_battery() const = 0;

        virtual void init_banks(std::ifstream &rom_stream) = 0;
        virtual uint8_t read(uint16_t address) = 0;
        virtual void write(uint16_t address, uint8_t value) = 0;
        virtual uint8_t read_ram(uint16_t address) = 0;
        virtual void write_ram(uint16_t address, uint8_t value) = 0;
        virtual void save_sram_to_file() = 0;
        virtual void load_sram_from_file() = 0;
        virtual void tick(uint32_t cycles) = 0;

        static std::unique_ptr<Cartridge> from_file(std::filesystem::path rom_path);
        static Cartridge *from_file_raw_ptr(std::filesystem::path rom_path);
    };

    class NoMBC : public Cartridge
    {
        std::vector<uint8_t> rom;

    public:
        NoMBC(CartHeader &&header);
        virtual ~NoMBC() override = default;

        bool has_battery() const override { return false; }

        void init_banks(std::ifstream &rom_stream) override;
        uint8_t read(uint16_t address) override;
        void write(uint16_t address, uint8_t value) override;
        uint8_t read_ram(uint16_t address) override;
        void write_ram(uint16_t address, uint8_t value) override;
        void save_sram_to_file() override{};
        void load_sram_from_file() override{};

        void tick(uint32_t cycles) override {}
    };

    class MBC1 : public Cartridge
    {
        uint32_t mode = 0, rom_bank_num = 1, bank_upper_bits = 0;

        bool ram_enabled = false;
        std::array<uint8_t, 32768> eram{};
        std::vector<uint8_t> rom{};

    public:
        MBC1(CartHeader &&header);
        virtual ~MBC1() override = default;

        bool has_battery() const override;

        void init_banks(std::ifstream &rom_stream) override;
        uint8_t read(uint16_t addr) override;
        void write(uint16_t addr, uint8_t value) override;
        uint8_t read_ram(uint16_t addr) override;
        void write_ram(uint16_t addr, uint8_t value) override;
        void save_sram_to_file() override;
        void load_sram_from_file() override;
        void tick(uint32_t cycles) override {}
    };

    class MBC2 : public Cartridge
    {
        uint16_t rom_bank_num = 1;
        bool ram_enabled = false;
        std::array<uint8_t, 512> ram{};
        std::vector<uint8_t> rom{};

    public:
        MBC2(CartHeader &&header);
        virtual ~MBC2() override = default;

        bool has_battery() const override;

        void init_banks(std::ifstream &rom_stream) override;
        uint8_t read(uint16_t address) override;
        void write(uint16_t address, uint8_t value) override;
        uint8_t read_ram(uint16_t address) override;
        void write_ram(uint16_t address, uint8_t value) override;
        void save_sram_to_file() override;
        void load_sram_from_file() override;
        void tick(uint32_t cycles) override {}
    };

    class RTCCounter
    {
        uint8_t counter = 0;
        uint8_t mask;

    public:
        RTCCounter(uint8_t bit_mask);
        uint8_t get() const;

        void set(uint8_t value);
        void increment();
    };

    struct RTCTimePoint
    {
        RTCCounter seconds = {0x3F}, minutes = {0x3F}, hours = {0x1F};
        uint16_t days = 0;
    };

    class MBC3 : public Cartridge
    {
        uint32_t rom_bank_num = 1, ram_rtc_select = 0;

        bool ram_rtc_enabled = false;
        std::array<uint8_t, 65536> eram{};
        std::vector<uint8_t> rom{};

        uint8_t latch_byte = 0;

        uint32_t rtc_cycles = 0;
        RTCTimePoint rtc{}, shadow_rtc{};
        uint16_t rtc_ctrl = 0;

    public:
        MBC3(CartHeader &&header);
        virtual ~MBC3() override = default;

        bool has_rtc() const;
        bool has_battery() const override;

        void init_banks(std::ifstream &rom_stream) override;
        uint8_t read(uint16_t addr) override;
        void write(uint16_t addr, uint8_t value) override;
        uint8_t read_ram(uint16_t addr) override;
        void write_ram(uint16_t addr, uint8_t value) override;
        void save_sram_to_file() override;
        void load_sram_from_file() override;
        void tick(uint32_t cycles) override;
    };
}
