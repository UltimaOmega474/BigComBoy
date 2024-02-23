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

namespace GB
{
    Cartridge::Cartridge(CartHeader &&header) : header(std::move(header)) {}

    std::unique_ptr<Cartridge> Cartridge::from_file(std::filesystem::path rom_path)
    {
        return std::unique_ptr<Cartridge>(from_file_raw_ptr(std::move(rom_path)));
    }

    Cartridge *Cartridge::from_file_raw_ptr(std::filesystem::path rom_path)
    {
        if (!std::filesystem::exists(rom_path))
            return nullptr;

        CartHeader header{};
        header.file_path = std::move(rom_path);
        std::ifstream rom(header.file_path, std::ios::binary | std::ios::ate);
        if (rom)
        {

            size_t rom_len = rom.tellg();

            if (rom_len < 0x14F)
                return {};

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
            switch (header.mbc_type)
            {
            case 0x00: // ROM ONLY
            {
                mbc = new NoMBC(std::move(header));
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
            }

            if (mbc)
            {
                mbc->init_banks(rom);
                mbc->load_sram_from_file();
                rom.close();
                return mbc;
            }
        }

        return nullptr;
    }

    NoMBC::NoMBC(CartHeader &&header) : Cartridge(std::move(header)), rom() {}

    void NoMBC::init_banks(std::ifstream &rom_stream)
    {
        rom_stream.seekg(0, std::ios::end);

        rom.resize(rom_stream.tellg());

        rom_stream.seekg(0);
        rom_stream.read(reinterpret_cast<char *>(rom.data()), rom.size());
    }

    uint8_t NoMBC::read(uint16_t address) { return rom[address]; }

    void NoMBC::write(uint16_t address, uint8_t value) {}

    uint8_t NoMBC::read_ram(uint16_t address) { return 0xFF; }

    void NoMBC::write_ram(uint16_t address, uint8_t value) {}

    MBC1::MBC1(CartHeader &&header) : Cartridge(std::move(header)), eram() { eram.fill(0); }

    bool MBC1::has_battery() const { return header.mbc_type == 3; }

    void MBC1::init_banks(std::ifstream &rom_stream)
    {
        rom_stream.seekg(0, std::ios::end);
        auto rom_len = rom_stream.tellg();
        rom_stream.seekg(0);

        rom.resize(rom_len);

        rom_stream.read(reinterpret_cast<char *>(rom.data()), rom_len);
    }

    uint8_t MBC1::read(uint16_t address)
    {
        uint32_t bank_num = (bank_upper_bits << 5);

        if (address < 0x4000)
        {
            bank_num = bank_num % (rom.size() / 0x4000);
            return rom[(mode ? (bank_num * 0x4000) : 0) + address];
        }

        bank_num |= rom_bank_num;
        bank_num = bank_num % (rom.size() / 0x4000);

        return rom[(bank_num * 0x4000) + (address & 0x3FFF)];
    }

    void MBC1::write(uint16_t address, uint8_t value)
    {
        switch (address >> 12)
        {
        case 0x0:
        case 0x1:
        {
            ram_enabled = (value & 0xF) == 0xA;
            break;
        }
        case 0x2:
        case 0x3:
        {
            rom_bank_num = value & 0x1F;
            if (rom_bank_num == 0)
                rom_bank_num = 1;
            break;
        }

        case 0x4:
        case 0x5:
        {
            if (mode)
            {
                if (header.ram_size == Ram32KB)
                {
                    bank_upper_bits = value & 0x3;
                }
            }

            if (header.rom_size >= Rom1MB)
            {
                bank_upper_bits = (value & 0x3);
            }
            break;
        }

        case 0x6:
        case 0x7:
        {
            mode = value & 0x1;
            break;
        }
        }
    }

    uint8_t MBC1::read_ram(uint16_t address)
    {
        if (ram_enabled)
            return eram[(mode ? (bank_upper_bits * 0x2000) : 0) + address];

        return 0xFF;
    }

    void MBC1::write_ram(uint16_t address, uint8_t value)
    {
        if (ram_enabled)
            eram[(mode ? (bank_upper_bits * 0x2000) : 0) + address] = value;
    }

    void MBC1::save_sram_to_file()
    {
        if (!has_battery())
            return;

        std::filesystem::path path = header.file_path;
        path += ".sram";

        std::ofstream sram(path, std::ios::binary);
        if (sram)
        {
            sram.write(reinterpret_cast<char *>(eram.data()),
                       static_cast<std::streamsize>(eram.size()));
            sram.close();
        }
    }

    void MBC1::load_sram_from_file()
    {
        if (!has_battery())
            return;

        std::filesystem::path path = header.file_path;
        path += ".sram";

        std::ifstream sram(path, std::ios::binary | std::ios::ate);
        if (sram)
        {
            auto len = sram.tellg();

            sram.seekg(0);

            sram.read(reinterpret_cast<char *>(eram.data()), len);
            sram.close();
        }
    }

    MBC2::MBC2(CartHeader &&header) : Cartridge(std::move(header)) {}

    bool MBC2::has_battery() const { return header.mbc_type == 6; }

    void MBC2::init_banks(std::ifstream &rom_stream)
    {
        rom_stream.seekg(0, std::ios::end);
        auto rom_len = rom_stream.tellg();
        rom_stream.seekg(0);

        rom.resize(rom_len);

        rom_stream.read(reinterpret_cast<char *>(rom.data()), rom_len);
    }

    uint8_t MBC2::read(uint16_t address)
    {
        if (address < 0x4000)
        {
            return rom[address];
        }

        auto bank = rom_bank_num % (rom.size() / 0x4000);

        return rom[(bank * 0x4000) + (address & 0x3FFF)];
    }

    void MBC2::write(uint16_t address, uint8_t value)
    {
        if (address < 0x4000)
        {
            if (address & 0x100)
            {
                rom_bank_num = value & 0xF;
                if (rom_bank_num == 0)
                    rom_bank_num = 1;
            }
            else
            {
                // ram enable
                ram_enabled = (value & 0xF) == 0xA;
            }
        }
    }

    uint8_t MBC2::read_ram(uint16_t address)
    {
        if (ram_enabled)
            return (ram[address & 0x01FF] & 0xF) | 0xF0;

        return 0xFF;
    }

    void MBC2::write_ram(uint16_t address, uint8_t value)
    {
        if (ram_enabled)
            ram[address & 0x01FF] = value & 0xF;
    }

    void MBC2::save_sram_to_file()
    {
        if (!has_battery())
            return;

        std::filesystem::path path = header.file_path;
        path += ".sram";

        std::ofstream sram(path, std::ios::binary);
        if (sram)
        {
            sram.write(reinterpret_cast<char *>(ram.data()),
                       static_cast<std::streamsize>(ram.size()));
            sram.close();
        }
    }

    void MBC2::load_sram_from_file()
    {
        if (!has_battery())
            return;

        std::filesystem::path path = header.file_path;
        path += ".sram";

        std::ifstream sram(path, std::ios::binary | std::ios::ate);
        if (sram)
        {
            auto len = sram.tellg();

            sram.seekg(0);

            sram.read(reinterpret_cast<char *>(ram.data()), len);
            sram.close();
        }
    }

    RTCCounter::RTCCounter(uint8_t bit_mask) : mask(bit_mask) {}

    uint8_t RTCCounter::get() const { return counter; }

    void RTCCounter::set(uint8_t value)
    {
        counter = value;
        counter &= mask;
    }

    void RTCCounter::increment()
    {
        counter++;
        counter &= mask;
    }

    MBC3::MBC3(CartHeader &&header) : Cartridge(std::move(header)) {}

    bool MBC3::has_rtc() const
    {
        switch (header.mbc_type)
        {
        case 0x0F: // MBC3+TIMER+BATTERY
        case 0x10: // MBC3+TIMER+RAM+BATTERY
        {
            return true;
        }
        }

        return false;
    }

    bool MBC3::has_battery() const
    {
        switch (header.mbc_type)
        {
        case 0x0F: // MBC3+TIMER+BATTERY
        case 0x10: // MBC3+TIMER+RAM+BATTERY
        case 0x13: // MBC3+RAM+BATTERY
        {
            return true;
        }
        }
        return false;
    }

    void MBC3::init_banks(std::ifstream &rom_stream)
    {
        rom_stream.seekg(0, std::ios::end);
        auto rom_len = rom_stream.tellg();
        rom_stream.seekg(0);

        rom.resize(rom_len);

        rom_stream.read(reinterpret_cast<char *>(rom.data()), rom_len);
    }

    uint8_t MBC3::read(uint16_t address)
    {
        if (address < 0x4000)
            return rom[address];

        return rom[(rom_bank_num * 0x4000) + (address & 0x3FFF)];
    }

    void MBC3::write(uint16_t address, uint8_t value)
    {
        switch (address >> 12)
        {
        case 0x0:
        case 0x1:
        {
            ram_rtc_enabled = (value & 0xF) == 0xA;
            break;
        }
        case 0x2:
        case 0x3:
        {
            rom_bank_num = value; // MBC3 carts will access banks 1-7F, MBC30 1-FF
            if (rom_bank_num == 0)
                rom_bank_num = 1;
            break;
        }

        case 0x4:
        case 0x5:
        {
            ram_rtc_select = value;
            break;
        }

        case 0x6:
        case 0x7:
        {
            if ((latch_byte == 0) && (value == 1))
                shadow_rtc = rtc;

            latch_byte = value;
            break;
        }
        }
    }

    uint8_t MBC3::read_ram(uint16_t address)
    {

        switch (ram_rtc_select)
        {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4: // MBC30 carts will access banks 0-7
        case 0x5:
        case 0x6:
        case 0x7:
        {
            if (ram_rtc_enabled)
                return eram[(ram_rtc_select * 0x2000) + address];

            break;
        }
        case 0x8:
        {
            return shadow_rtc.seconds.get();
        }
        case 0x9:
        {
            return shadow_rtc.minutes.get();
        }
        case 0xA:
        {
            return shadow_rtc.hours.get();
        }
        case 0xB:
        {
            return static_cast<uint8_t>(shadow_rtc.days & 0xFF);
        }
        case 0xC:
        {
            uint32_t a = rtc_ctrl | ((shadow_rtc.days & 0x100) ? 1 : 0);

            return a;
        }
        }
        return 0xFF;
    }

    void MBC3::write_ram(uint16_t address, uint8_t value)
    {
        switch (ram_rtc_select)
        {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4: // MBC30 carts will access banks 0-7
        case 0x5:
        case 0x6:
        case 0x7:
        {
            if (ram_rtc_enabled)
                eram[(ram_rtc_select * 0x2000) + address] = value;
            break;
        }
        case 0x8:
        {
            rtc_cycles = 0;
            rtc.seconds.set(value);
            break;
        }
        case 0x9:
        {
            rtc.minutes.set(value);
            break;
        }
        case 0xA:
        {
            rtc.hours.set(value);
            break;
        }
        case 0xB:
        {
            rtc.days &= ~0xFF;
            rtc.days |= value;
            break;
        }
        case 0xC:
        {
            rtc_ctrl = value & 0xC0;

            rtc.days &= ~0x100;
            rtc.days |= (value & 0x1) ? 0x100 : 0;
            break;
        }
        }
    }

    void MBC3::save_sram_to_file()
    {
        if (!has_battery())
            return;

        std::filesystem::path path = header.file_path;
        path += ".sram";

        std::ofstream sram(path, std::ios::binary);
        if (sram)
        {
            sram.write(reinterpret_cast<char *>(eram.data()),
                       static_cast<std::streamsize>(eram.size()));
            sram.close();
        }
    }

    void MBC3::load_sram_from_file()
    {
        if (!has_battery())
            return;

        std::filesystem::path path = header.file_path;
        path += ".sram";

        std::ifstream sram(path, std::ios::binary | std::ios::ate);
        if (sram)
        {
            auto len = sram.tellg();

            sram.seekg(0);

            sram.read(reinterpret_cast<char *>(eram.data()), len);
            sram.close();
        }
    }

    void MBC3::tick(uint32_t cycles)
    {
        if (has_rtc() && !(rtc_ctrl & 64))
        {
            rtc_cycles += cycles;

            if (rtc_cycles == CPU_CLOCK_RATE)
            {
                rtc_cycles = 0;
                rtc.seconds.increment();

                if (rtc.seconds.get() == 60)
                {
                    rtc.seconds.set(0);
                    rtc.minutes.increment();

                    if (rtc.minutes.get() == 60)
                    {
                        rtc.minutes.set(0);
                        rtc.hours.increment();

                        if (rtc.hours.get() == 24)
                        {
                            rtc.hours.set(0);
                            rtc.days++;

                            if (rtc.days == 512)
                            {
                                rtc.days = 0;
                                rtc_ctrl |= 128;
                            }
                        }
                    }
                }
            }
        }
    }
}
