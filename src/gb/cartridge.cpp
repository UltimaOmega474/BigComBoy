#include "cartridge.hpp"

#include <fstream>
namespace GameBoy
{
	Cartridge::Cartridge(CartHeader &&header)
		: header(std::move(header))
	{
	}

	std::unique_ptr<Cartridge> Cartridge::from_file(std::string_view rom_path)
	{
		CartHeader header{};
		header.file_path = rom_path;
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

			switch (header.mbc_type)
			{
			case 0:
			{
				auto mbc = std::make_unique<NoMBC>(std::move(header));
				rom.close();
				return std::move(mbc);
			}
			case 1:
			{
				auto mbc = std::make_unique<MBC1>(std::move(header));
				mbc->init_banks(rom);
				mbc->load_sram_from_file();
				rom.close();
				return std::move(mbc);
			}
			}
		}

		return {};
	}

	NoMBC::NoMBC(CartHeader &&header)
		: Cartridge(std::move(header)), rom()
	{
	}

	void NoMBC::init_banks(std::ifstream &rom_stream)
	{
		rom_stream.seekg(0, std::ios::end);

		rom.resize(rom_stream.tellg());

		rom_stream.seekg(0);
		rom_stream.read(reinterpret_cast<char *>(rom.data()), rom.size());
	}

	uint8_t NoMBC::read(uint16_t addr)
	{
		return rom[addr];
	}

	void NoMBC::write(uint16_t addr, uint8_t value)
	{
	}

	uint8_t NoMBC::read_ram(uint16_t addr)
	{
		return 0xFF;
	}

	void NoMBC::write_ram(uint16_t addr, uint8_t value)
	{
	}

	MBC1::MBC1(CartHeader &&header)
		: Cartridge(std::move(header)), mode(0), rom_bank_num(1), ram_bank_num(0), bank_upper_bits(0),
		  ram_enabled(0), eram()
	{
		eram.fill(0);
	}

	void MBC1::init_banks(std::ifstream &rom_stream)
	{

		rom_stream.seekg(0, std::ios::end);
		size_t rom_len = rom_stream.tellg();
		rom_stream.seekg(0);

		auto itc = rom_len / 0x4000;
		uint32_t offset = 0;
		for (auto i = 0; i < itc; ++i)
		{
			std::vector<uint8_t> bank;
			bank.resize(0x4000);

			rom_stream.read(reinterpret_cast<char *>(bank.data()), static_cast<std::streamsize>(bank.size()));

			bank_list.push_back(std::move(bank));
			offset += 0x4000;
		}
	}

	uint8_t MBC1::read(uint16_t address)
	{
		if (address < 0x4000)
		{
			return bank_list[0][address];
		}

		return bank_list[rom_bank_num % bank_list.size()][address & 0x3FFF];
	}

	void MBC1::write(uint16_t address, uint8_t value)
	{
		if (((address >= 0x6000) && (address <= 0x7FFF)))
		{
			// mode
			if (header.ram_size > Ram8KB && header.rom_size > Rom512KB)
			{
				mode = value & 0x1;
			}
		}
		else if (((address >= 0x4000) && (address <= 0x5FFF)))
		{
			// ram bank or rom bank
			ram_bank_num = value & 0x3;
			bank_upper_bits = value & 0x3;
		}
		else if (((address >= 0x2000) && (address <= 0x3FFF)))
		{
			// rom bank num
			rom_bank_num = value & 0x1F;
			if (rom_bank_num == 0)
				rom_bank_num = 1;

			if (mode)
			{
				rom_bank_num |= (bank_upper_bits << 5);
			}
		}
		else if (((address >= 0x0000) && (address <= 0x1FFF)))
		{
			// ram enable
			ram_enabled = (value & 0xA) == 0xA;
		}
	}

	uint8_t MBC1::read_ram(uint16_t address)
	{
		return eram[ram_bank_num * 0x2000 + address];
	}

	void MBC1::write_ram(uint16_t address, uint8_t value)
	{
		eram[ram_bank_num * 0x2000 + address] = value;
	}

	void MBC1::save_sram_to_file()
	{
		std::string path = header.file_path;
		path += ".sram";

		std::ofstream sram(path, std::ios::binary);
		if (sram)
		{
			sram.write(reinterpret_cast<char *>(eram.data()), static_cast<std::streamsize>(eram.size()));
			sram.close();
		}
	}

	void MBC1::load_sram_from_file()
	{
		std::string path = header.file_path;
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
}
