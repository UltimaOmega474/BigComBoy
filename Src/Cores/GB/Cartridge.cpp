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

#include "Cartridge.hpp"
#include "Constants.hpp"
#include <fstream>

namespace GB {
    Cartridge::Cartridge(CartHeader &&header) : header_(std::move(header)) {}

    const CartHeader &Cartridge::header() const { return header_; }

    auto Cartridge::from_file(std::filesystem::path rom_path) -> std::unique_ptr<Cartridge> {
        return std::unique_ptr<Cartridge>(from_file_raw_ptr(std::move(rom_path)));
    }

    auto Cartridge::from_file_raw_ptr(std::filesystem::path rom_path) -> Cartridge * {
        if (!std::filesystem::exists(rom_path)) {
            return nullptr;
        }

        CartHeader header{};
        header.file_path = std::move(rom_path);
        std::ifstream rom(header.file_path, std::ios::binary | std::ios::ate);
        if (rom) {

            const size_t rom_len = rom.tellg();

            if (rom_len < 0x14F) {
                return nullptr;
            }

            rom.seekg(0x134);

            header.title.resize(16, '\0');

            rom.read(header.title.data(), static_cast<std::streamsize>(header.title.size()));

            rom.seekg(0x143);

            rom.read(reinterpret_cast<char *>(&header.cgb_support), 1);
            rom.read(reinterpret_cast<char *>(&header.license_code), 2);
            rom.read(reinterpret_cast<char *>(&header.sgb_flag), 1);
            rom.read(reinterpret_cast<char *>(&header.mbc_type), 1);
            rom.read(reinterpret_cast<char *>(&header.rom_size), 1);
            rom.read(reinterpret_cast<char *>(&header.ram_size), 1);
            rom.read(reinterpret_cast<char *>(&header.region_code), 1);
            rom.read(reinterpret_cast<char *>(&header.old_license_code), 1);
            rom.read(reinterpret_cast<char *>(&header.version), 1);
            rom.read(reinterpret_cast<char *>(&header.header_checksum), 1);
            rom.read(reinterpret_cast<char *>(&header.checksum), 2);

            Cartridge *mbc = nullptr;
            switch (header.mbc_type) {
            case 0x00: // ROM ONLY
            {
                mbc = new ROM(std::move(header));
                break;
            }
            case 0x01: // MBC1
            case 0x02: // MBC1+RAM
            case 0x03: // MBC1+RAM+BATTERY
            {
                mbc = new MBC1(std::move(header));
                break;
            }
            case 0x05: // MBC2
            case 0x06: // MBC2+BATTERY
            {
                mbc = new MBC2(std::move(header));
                break;
            }

            case 0x0F: // MBC3+TIMER+BATTERY
            case 0x10: // MBC3+TIMER+RAM+BATTERY
            case 0x11: // MBC3
            case 0x12: // MBC3+RAM
            case 0x13: // MBC3+RAM+BATTERY
            {
                mbc = new MBC3(std::move(header));
                break;
            }

            case 0x19: // MBC5
            case 0x1A: // MBC5+RAM
            case 0x1B: // MBC5+RAM+BATTERY
            case 0x1C: // MBC5+RUMBLE
            case 0x1D: // MBC5+RUMBLE+RAM
            case 0x1E: // MBC5+RUMBLE+RAM+BATTERY
            {
                mbc = new MBC5(std::move(header));
                break;
            }
            default:;
            }

            if (mbc) {
                mbc->init_banks(rom);
                mbc->load_sram_from_file();
                rom.close();
                return mbc;
            }

        }

        return nullptr;
    }

    ROM::ROM(CartHeader &&header) : Cartridge(std::move(header)) {}

    auto ROM::reset() -> void {}

    auto ROM::init_banks(std::ifstream &rom_stream) -> void {
        rom_stream.seekg(0, std::ios::end);

        rom.resize(rom_stream.tellg());

        rom_stream.seekg(0);
        rom_stream.read(reinterpret_cast<char *>(rom.data()),
                        static_cast<std::streamsize>(rom.size()));
    }

    auto ROM::read(const uint16_t address) -> uint8_t { return rom[address]; }

    auto ROM::write(uint16_t, uint8_t) -> void {}

    auto ROM::read_ram(uint16_t) -> uint8_t { return 0xFF; }

    auto ROM::write_ram(uint16_t, uint8_t) -> void {}

    auto ROM::save_sram_to_file() -> void {}

    auto ROM::load_sram_from_file() -> void {}

    auto ROM::clock(const int32_t cycles) -> void {}

    MBC1::MBC1(CartHeader &&header) : Cartridge(std::move(header)) { eram.fill(0); }

    auto MBC1::has_battery() const -> bool { return header_.mbc_type == 3; }

    auto MBC1::reset() -> void {
        mode = false;
        rom_bank_num = 1;
        bank_upper_bits = 0;
        ram_enabled = false;
        eram.fill(0);
    }

    auto MBC1::init_banks(std::ifstream &rom_stream) -> void {
        rom_stream.seekg(0, std::ios::end);
        const auto rom_len = rom_stream.tellg();
        rom_stream.seekg(0);

        rom.resize(rom_len);

        rom_stream.read(reinterpret_cast<char *>(rom.data()), rom_len);
    }

    auto MBC1::read(const uint16_t address) -> uint8_t {
        int32_t bank_num = (bank_upper_bits << 5);

        if (address < 0x4000) {
            bank_num = bank_num % static_cast<int32_t>(rom.size() / 0x4000);
            return rom[(mode ? (bank_num * 0x4000) : 0) + address];
        }

        bank_num |= rom_bank_num;
        bank_num = bank_num % static_cast<int32_t>(rom.size() / 0x4000);

        return rom[(bank_num * 0x4000) + (address & 0x3FFF)];
    }

    auto MBC1::write(const uint16_t address, const uint8_t value) -> void {
        switch (address >> 12) {
        case 0x0:
        case 0x1: {
            ram_enabled = (value & 0xF) == 0xA;
            break;
        }
        case 0x2:
        case 0x3: {
            rom_bank_num = value & 0x1F;
            if (rom_bank_num == 0) {
                rom_bank_num = 1;
            }
            break;
        }

        case 0x4:
        case 0x5: {
            if (mode) {
                if (header_.ram_size == RamSize::Ram32KB) {
                    bank_upper_bits = value & 0x3;
                }
            }

            if (header_.rom_size >= RomSize::Rom1MB) {
                bank_upper_bits = (value & 0x3);
            }
            break;
        }

        case 0x6:
        case 0x7: {
            mode = value & 0x1;
            break;
        }
        default:;
        }
    }

    auto MBC1::read_ram(const uint16_t address) -> uint8_t {
        if (ram_enabled) {
            return eram[(mode ? (bank_upper_bits * 0x2000) : 0) + address];
        }

        return 0xFF;
    }

    auto MBC1::write_ram(const uint16_t address, const uint8_t value) -> void {
        if (ram_enabled) {
            eram[(mode ? (bank_upper_bits * 0x2000) : 0) + address] = value;
        }
    }

    auto MBC1::save_sram_to_file() -> void {
        if (!has_battery()) {
            return;
        }

        std::filesystem::path path = header_.file_path;
        path += ".sram";

        std::ofstream sram(path, std::ios::binary);
        if (sram) {
            sram.write(reinterpret_cast<char *>(eram.data()),
                       static_cast<std::streamsize>(eram.size()));
            sram.close();
        }
    }

    auto MBC1::load_sram_from_file() -> void {
        if (!has_battery()) {
            return;
        }

        std::filesystem::path path = header_.file_path;
        path += ".sram";

        std::ifstream sram(path, std::ios::binary | std::ios::ate);
        if (sram) {
            const auto len = sram.tellg();

            sram.seekg(0);

            sram.read(reinterpret_cast<char *>(eram.data()), len);
            sram.close();
        }
    }

    auto MBC1::clock(int32_t) -> void {}

    MBC2::MBC2(CartHeader &&header) : Cartridge(std::move(header)) {}

    auto MBC2::has_battery() const -> bool { return header_.mbc_type == 6; }

    auto MBC2::reset() -> void {
        rom_bank_num = 1;
        ram_enabled = false;
        ram.fill(0);
    }

    auto MBC2::init_banks(std::ifstream &rom_stream) -> void {
        rom_stream.seekg(0, std::ios::end);
        const auto rom_len = rom_stream.tellg();
        rom_stream.seekg(0);

        rom.resize(rom_len);

        rom_stream.read(reinterpret_cast<char *>(rom.data()), rom_len);
    }

    auto MBC2::read(const uint16_t address) -> uint8_t {
        if (address < 0x4000) {
            return rom[address];
        }

        auto bank = rom_bank_num % (rom.size() / 0x4000);

        return rom[(bank * 0x4000) + (address & 0x3FFF)];
    }

    auto MBC2::write(const uint16_t address, const uint8_t value) -> void {
        if (address < 0x4000) {
            if (address & 0x100) {

                rom_bank_num = value & 0xF;

                if (rom_bank_num == 0) {
                    rom_bank_num = 1;
                }
            } else {
                // ram enable
                ram_enabled = (value & 0xF) == 0xA;
            }
        }
    }

    auto MBC2::read_ram(const uint16_t address) -> uint8_t {
        if (ram_enabled) {
            return (ram[address & 0x01FF] & 0xF) | 0xF0;
        }

        return 0xFF;
    }

    auto MBC2::write_ram(const uint16_t address, const uint8_t value) -> void {
        if (ram_enabled) {
            ram[address & 0x01FF] = value & 0xF;
        }
    }

    auto MBC2::save_sram_to_file() -> void {
        if (!has_battery()) {
            return;
        }

        std::filesystem::path path = header_.file_path;
        path += ".sram";

        std::ofstream sram(path, std::ios::binary);
        if (sram) {
            sram.write(reinterpret_cast<char *>(ram.data()),
                       static_cast<std::streamsize>(ram.size()));
            sram.close();
        }
    }

    auto MBC2::load_sram_from_file() -> void {
        if (!has_battery()) {
            return;
        }

        std::filesystem::path path = header_.file_path;
        path += ".sram";

        std::ifstream sram(path, std::ios::binary | std::ios::ate);
        if (sram) {
            const auto len = sram.tellg();

            sram.seekg(0);

            sram.read(reinterpret_cast<char *>(ram.data()), len);
            sram.close();
        }
    }

    auto MBC2::clock(int32_t) -> void {}

    RTCCounter::RTCCounter(const uint8_t bit_mask) : mask(bit_mask) {}

    auto RTCCounter::get() const -> uint8_t { return counter; }

    auto RTCCounter::set(const uint8_t value) -> void {
        counter = value;
        counter &= mask;
    }

    auto RTCCounter::increment() -> void {
        counter++;
        counter &= mask;
    }

    MBC3::MBC3(CartHeader &&header) : Cartridge(std::move(header)) {}

    auto MBC3::has_rtc() const -> bool {
        switch (header_.mbc_type) {
        case 0x0F:   // MBC3+TIMER+BATTERY
        case 0x10: { // MBC3+TIMER+RAM+BATTERY
            return true;
        }
        default:;
        }

        return false;
    }

    auto MBC3::has_battery() const -> bool {
        switch (header_.mbc_type) {
        case 0x0F:   // MBC3+TIMER+BATTERY
        case 0x10:   // MBC3+TIMER+RAM+BATTERY
        case 0x13: { // MBC3+RAM+BATTERY
            return true;
        }
        default:;
        }
        return false;
    }

    auto MBC3::reset() -> void {
        rom_bank_num = 1;
        ram_rtc_select = 0;
        ram_rtc_enabled = false;
        eram.fill(0);
        latch_byte = 0;
        rtc_cycles = 0;
        rtc = RTCTimePoint{};
        shadow_rtc = RTCTimePoint{};
        rtc_ctrl = 0;
    }

    auto MBC3::init_banks(std::ifstream &rom_stream) -> void {
        rom_stream.seekg(0, std::ios::end);
        const auto rom_len = rom_stream.tellg();
        rom_stream.seekg(0);

        rom.resize(rom_len);

        rom_stream.read(reinterpret_cast<char *>(rom.data()), rom_len);
    }

    auto MBC3::read(const uint16_t address) -> uint8_t {
        if (address < 0x4000) {
            return rom[address];
        }

        return rom[(rom_bank_num * 0x4000) + (address & 0x3FFF)];
    }

    auto MBC3::write(uint16_t address, uint8_t value) -> void {
        switch (address >> 12) {
        case 0x0:
        case 0x1: {
            ram_rtc_enabled = (value & 0xF) == 0xA;
            break;
        }
        case 0x2:
        case 0x3: {
            rom_bank_num = value; // MBC3 carts will access banks 1-7F, MBC30 1-FF
            if (rom_bank_num == 0) {
                rom_bank_num = 1;
            }
            break;
        }

        case 0x4:
        case 0x5: {
            ram_rtc_select = value;
            break;
        }

        case 0x6:
        case 0x7: {
            if ((latch_byte == 0) && (value == 1)) {
                shadow_rtc = rtc;
            }

            latch_byte = value;
            break;
        }
        default:;
        }
    }

    auto MBC3::read_ram(const uint16_t address) -> uint8_t {
        switch (ram_rtc_select) {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4: // MBC30 carts will access banks 0-7
        case 0x5:
        case 0x6:
        case 0x7: {
            if (ram_rtc_enabled) {
                return eram[(ram_rtc_select * 0x2000) + address];
            }

            return 0xFF;
        }
        case 0x8: {
            return shadow_rtc.seconds.get();
        }
        case 0x9: {
            return shadow_rtc.minutes.get();
        }
        case 0xA: {
            return shadow_rtc.hours.get();
        }
        case 0xB: {
            return static_cast<uint8_t>(shadow_rtc.days & 0xFF);
        }
        case 0xC: {
            return rtc_ctrl | ((shadow_rtc.days & 0x100) ? 1 : 0);
        }
        default: return 0xFF;
        }
    }

    auto MBC3::write_ram(const uint16_t address, const uint8_t value) -> void {
        switch (ram_rtc_select) {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4: // MBC30 carts will access banks 0-7
        case 0x5:
        case 0x6:
        case 0x7: {
            if (ram_rtc_enabled) {
                eram[(ram_rtc_select * 0x2000) + address] = value;
            }
            break;
        }
        case 0x8: {
            rtc_cycles = 0;
            rtc.seconds.set(value);
            break;
        }
        case 0x9: {
            rtc.minutes.set(value);
            break;
        }
        case 0xA: {
            rtc.hours.set(value);
            break;
        }
        case 0xB: {
            rtc.days &= ~0xFF;
            rtc.days |= value;
            break;
        }
        case 0xC: {
            rtc_ctrl = value & 0xC0;

            rtc.days &= ~0x100;
            rtc.days |= (value & 0x1) ? 0x100 : 0;
            break;
        }
        default:;
        }
    }

    auto MBC3::save_sram_to_file() -> void {
        if (!has_battery()) {
            return;
        }

        std::filesystem::path path = header_.file_path;
        path += ".sram";

        std::ofstream sram(path, std::ios::binary);
        if (sram) {
            sram.write(reinterpret_cast<char *>(eram.data()),
                       static_cast<std::streamsize>(eram.size()));
            sram.close();
        }
    }

    auto MBC3::load_sram_from_file() -> void {
        if (!has_battery()) {
            return;
        }

        std::filesystem::path path = header_.file_path;
        path += ".sram";

        std::ifstream sram(path, std::ios::binary | std::ios::ate);
        if (sram) {
            auto len = sram.tellg();

            sram.seekg(0);

            sram.read(reinterpret_cast<char *>(eram.data()), len);
            sram.close();
        }
    }

    auto MBC3::clock(const int32_t cycles) -> void {
        if (has_rtc() && !(rtc_ctrl & 64)) {
            rtc_cycles += cycles;

            if (rtc_cycles == CPU_CLOCK_RATE) {
                rtc_cycles = 0;
                rtc.seconds.increment();

                if (rtc.seconds.get() == 60) {
                    rtc.seconds.set(0);
                    rtc.minutes.increment();

                    if (rtc.minutes.get() == 60) {
                        rtc.minutes.set(0);
                        rtc.hours.increment();

                        if (rtc.hours.get() == 24) {
                            rtc.hours.set(0);
                            rtc.days++;

                            if (rtc.days == 512) {
                                rtc.days = 0;
                                rtc_ctrl |= 128;
                            }
                        }
                    }
                }
            }
        }
    }

    MBC5::MBC5(CartHeader &&header) : Cartridge(std::move(header)) {}

    auto MBC5::has_battery() const -> bool {
        switch (header_.mbc_type) {
        case 0x1B:   // MBC5+RAM+BATTERY
        case 0x1E: { // MBC5+RUMBLE+RAM+BATTERY
            return true;
        }
        default:;
        }

        return false;
    }

    auto MBC5::reset() -> void {
        rom_bank_num = 1;
        bank_upper_bits = 0;
        ram_bank_num = 0;
        ram_enabled = false;
        eram.fill(0);
    }

    auto MBC5::init_banks(std::ifstream &rom_stream) -> void {
        rom_stream.seekg(0, std::ios::end);
        const auto rom_len = rom_stream.tellg();
        rom_stream.seekg(0);

        rom.resize(rom_len);

        rom_stream.read(reinterpret_cast<char *>(rom.data()), rom_len);
    }

    auto MBC5::read(const uint16_t address) -> uint8_t {
        int32_t bank_num = rom_bank_num | bank_upper_bits;

        if (address < 0x4000) {
            return rom[address];
        }

        bank_num = bank_num % static_cast<int32_t>(rom.size() / 0x4000);

        return rom[(bank_num * 0x4000) + (address & 0x3FFF)];
    }

    auto MBC5::write(const uint16_t address, const uint8_t value) -> void {
        switch (address >> 12) {
        case 0x0:
        case 0x1: {
            ram_enabled = (value & 0xF) == 0xA;
            break;
        }
        case 0x2: {
            rom_bank_num = value;
            break;
        }
        case 0x3: {
            bank_upper_bits = (value & 0x1) << 8;
            break;
        }

        case 0x4:
        case 0x5: {
            ram_bank_num = value & 0xF;
            break;
        }
        default:;
        }
    }

    uint8_t MBC5::read_ram(uint16_t address) {
        if (ram_enabled) {
            return eram[(ram_bank_num * 0x2000) + address];
        }

        return 0xFF;
    }

    auto MBC5::write_ram(const uint16_t address,const uint8_t value) -> void {
        if (ram_enabled) {
            eram[(ram_bank_num * 0x2000) + address] = value;
        }
    }

    auto MBC5::save_sram_to_file() -> void {
        if (!has_battery()) {
            return;
        }

        std::filesystem::path path = header_.file_path;
        path += ".sram";

        std::ofstream sram(path, std::ios::binary);
        if (sram) {
            sram.write(reinterpret_cast<char *>(eram.data()),
                       static_cast<std::streamsize>(eram.size()));
            sram.close();
        }
    }

    auto MBC5::load_sram_from_file() -> void {
        if (!has_battery()) {
            return;
        }

        std::filesystem::path path = header_.file_path;
        path += ".sram";

        std::ifstream sram(path, std::ios::binary | std::ios::ate);
        if (sram) {
            const auto len = sram.tellg();

            sram.seekg(0);

            sram.read(reinterpret_cast<char *>(eram.data()), len);
            sram.close();
        }
    }

    auto MBC5::clock(int32_t) -> void {}
}
