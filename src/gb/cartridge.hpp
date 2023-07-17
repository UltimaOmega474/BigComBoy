#pragma once
#include <cinttypes>
#include <array>
#include <vector>
#include <string>
#include <string_view>
#include <fstream>
#include <memory>
namespace SunBoy
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
		std::string file_path;
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

		virtual void init_banks(std::ifstream &rom_stream) = 0;
		virtual uint8_t read(uint16_t address) = 0;
		virtual void write(uint16_t address, uint8_t value) = 0;
		virtual uint8_t read_ram(uint16_t address) = 0;
		virtual void write_ram(uint16_t address, uint8_t value) = 0;

		virtual void save_sram_to_file() = 0;
		virtual void load_sram_from_file() = 0;

		static std::shared_ptr<Cartridge> from_file(std::string_view rom_path);
	};

	class NoMBC : public Cartridge
	{
		std::vector<uint8_t> rom;

	public:
		NoMBC(CartHeader &&header);
		virtual ~NoMBC() = default;
		void init_banks(std::ifstream &rom_stream) override;
		uint8_t read(uint16_t address) override;
		void write(uint16_t address, uint8_t value) override;
		uint8_t read_ram(uint16_t address) override;
		void write_ram(uint16_t address, uint8_t value) override;
		void save_sram_to_file() override{};
		void load_sram_from_file() override{};
	};

	class MBC1 : public Cartridge
	{
		uint8_t mode = 0, rom_bank_num = 1,
				ram_bank_num = 0, bank_upper_bits = 0;

		bool ram_enabled = false;
		std::array<uint8_t, 8192> eram{};
		std::vector<std::vector<uint8_t>> bank_list;

	public:
		MBC1(CartHeader &&header);
		virtual ~MBC1() = default;

		void init_banks(std::ifstream &rom_stream) override;
		uint8_t read(uint16_t addr) override;
		void write(uint16_t addr, uint8_t value) override;
		uint8_t read_ram(uint16_t addr) override;
		void write_ram(uint16_t addr, uint8_t value) override;

		void save_sram_to_file() override;
		void load_sram_from_file() override;
	};

	class MBC2 : public Cartridge
	{
		uint8_t rom_bank_num = 1;
		bool ram_enabled = false;
		std::array<uint8_t, 512> ram{};
		std::vector<std::vector<uint8_t>> bank_list;

	public:
		MBC2(CartHeader &&header);
		virtual ~MBC2() = default;

		void init_banks(std::ifstream &rom_stream) override;
		uint8_t read(uint16_t address) override;
		void write(uint16_t address, uint8_t value) override;
		uint8_t read_ram(uint16_t address) override;
		void write_ram(uint16_t address, uint8_t value) override;

		void save_sram_to_file() override;
		void load_sram_from_file() override;
	};
}
