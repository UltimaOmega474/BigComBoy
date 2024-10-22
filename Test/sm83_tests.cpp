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

#include "test_types.hpp"
#include <GB/CPU.hpp>
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <nlohmann/json.hpp>

#define FAIL_IF_DIFFERENT(a, b, name)                                                              \
    CHECKED_IF(a != b) { FAIL(fmt::format("{}: {} - expected {}: {}\n", name, a, name, b)); }

#define BATCH_RUN_TESTS(from, to)                                                                  \
    for (int i = from; i <= to; ++i) {                                                             \
        run_test(static_cast<uint8_t>(i));                                                         \
    }

enum class BusActivityType { Read = 0, Write = 1 };

struct BusActivity {
    uint16_t address = 0;
    uint8_t value = 0;

    BusActivityType type = BusActivityType::Read;
};

std::array<uint8_t, 65536> memory{};
BusActivity last_activity;
std::vector<BusActivity> activity_list{};

uint8_t test_read(uint16_t address) {
    last_activity = BusActivity{
        .address = address,
        .value = memory[address],
        .type = BusActivityType::Read,
    };
    activity_list.push_back(last_activity);
    return memory[address];
}

void test_write(uint16_t address, uint8_t value) {
    memory[address] = value;
    last_activity = BusActivity{
        .address = address,
        .value = value,
        .type = BusActivityType::Write,
    };

    activity_list.push_back(last_activity);
}

GB::CPU cpu;

void run_test(uint8_t opcode) {
    using json = nlohmann::json;
    auto path_str = std::filesystem::path(fmt::format("v2/{:02x}.json", opcode));

    if (std::ifstream file(path_str); file) {
        std::vector<TestCase> test_data = json::parse(file);

        cpu.bus_read_fn = test_read;
        cpu.bus_write_fn = test_write;

        for (auto &test : test_data) {
            INFO("TEST: " + test.name);
            const auto &init = test.initial;

            memory.fill(0);
            activity_list.clear();

            cpu.force_next_opcode(opcode);
            cpu.set_flags(init.f);
            cpu.a = init.a;
            cpu.b = init.b;
            cpu.c = init.c;
            cpu.d = init.d;
            cpu.e = init.e;
            cpu.h = init.h;
            cpu.l = init.l;
            cpu.program_counter = init.pc;
            cpu.stack_pointer = init.sp;

            for (const auto &[address, value] : test.initial.ram) {
                memory[address] = value;
            }

            for (int i = 0; i < test.cycles.size(); ++i) {
                cpu.clock();
            }
            std::erase_if(test.cycles, [](const TestCycle &x) { return x.access.empty(); });

            FAIL_IF_DIFFERENT(cpu.a, test.final.a, "a")
            FAIL_IF_DIFFERENT(cpu.b, test.final.b, "b")
            FAIL_IF_DIFFERENT(cpu.c, test.final.c, "c")
            FAIL_IF_DIFFERENT(cpu.d, test.final.d, "d")
            FAIL_IF_DIFFERENT(cpu.e, test.final.e, "e")
            FAIL_IF_DIFFERENT(cpu.flags(), test.final.f, "f")
            FAIL_IF_DIFFERENT(cpu.h, test.final.h, "h")
            FAIL_IF_DIFFERENT(cpu.l, test.final.l, "l")
            FAIL_IF_DIFFERENT(cpu.program_counter, test.final.pc, "pc")
            FAIL_IF_DIFFERENT(cpu.stack_pointer, test.final.sp, "sp")

            for (const auto &[address, value] : test.final.ram) {

                CHECKED_IF(memory[address] != value) {
                    FAIL(fmt::format("\nAddress:{:#x}: {:#x} - expected: {:#x}\n", address,
                                     memory[address], value));
                }
            }

            CHECKED_IF(activity_list.size() != test.cycles.size()) {
                FAIL(fmt::format("Incorrect number of cycles taken: {} - expected: {}\n",
                                 activity_list.size(), test.cycles.size()));
            }

            for (size_t i = 0; i < activity_list.size(); ++i) {
                const auto &[address, value, type] = activity_list.at(i);
                const auto &cycle = test.cycles.at(i);

                static constexpr std::array<std::string_view, 2> access{
                    "read",
                    "write",
                };

                CHECKED_IF(((address != cycle.address) || (value != cycle.value) ||
                            (access[static_cast<int32_t>(type)] != cycle.access))) {

                    INFO(fmt::format("\ncycle: {}\n", i + 1));
                    INFO(fmt::format("bus address: {} - expected: {}\n", address, cycle.address));
                    INFO(fmt::format("value: {} - expected: {}\n", value, cycle.value));
                    INFO(fmt::format("operation: {}, expected: {}\n",
                                     access[static_cast<int32_t>(type)], cycle.access));

                    FAIL("Cycles Mismatch");
                }
            }
        }
    }
}

TEST_CASE("ld indirect with a") {
    run_test(0x02);
    run_test(0x12);
    run_test(0x22);
    run_test(0x32);
    run_test(0x0A);
    run_test(0x1A);
    run_test(0x2A);
    run_test(0x3A);
}

TEST_CASE("ld rp, u16") {
    run_test(0x01);
    run_test(0x11);
    run_test(0x21);
    run_test(0x31);
}

TEST_CASE("miscellaneous ld commands") {
    run_test(0x08); // ld (direct), sp
    run_test(0xEA); // ld (direct), a
    run_test(0xFA); // ld a, (direct)
    run_test(0xF8); // ld hl, sp + i8
    run_test(0xE0); // ld (ffxx), a
    run_test(0xF0); // ld a, (ffxx)
    run_test(0xE2);
    run_test(0xF2);
}

TEST_CASE("ld r,r") {
    run_test(0x06);
    run_test(0x16);
    run_test(0x26);
    run_test(0x36);
    run_test(0x0E);
    run_test(0x1E);
    run_test(0x2E);
    run_test(0x3E);

    BATCH_RUN_TESTS(0x40, 0x7F)
}

TEST_CASE("add hl, rp") {
    run_test(0x09);
    run_test(0x19);
    run_test(0x29);
    run_test(0x39);
}

TEST_CASE("add sp, i8") {
    run_test(0xE8);
}

TEST_CASE("add/adc"){BATCH_RUN_TESTS(0x80, 0x8F)}

TEST_CASE("sub/sbc"){BATCH_RUN_TESTS(0x90, 0x9F)}

TEST_CASE("and/xor"){BATCH_RUN_TESTS(0xA0, 0xAF)}

TEST_CASE("or/cp") {
    BATCH_RUN_TESTS(0xB0, 0xBF)
}

TEST_CASE("add/adc/sub/sbc/and/xor/or/cp a, u8") {
    run_test(0xC6);
    run_test(0xCE);
    run_test(0xD6);
    run_test(0xDE);
    run_test(0xE6);
    run_test(0xEE);
    run_test(0xF6);
    run_test(0xFE);
}

TEST_CASE("inc rp/dec rp") {
    run_test(0x03);
    run_test(0x13);
    run_test(0x23);
    run_test(0x33);

    run_test(0x0B);
    run_test(0x1B);
    run_test(0x2B);
    run_test(0x3B);
}

TEST_CASE("inc r/dec r") {
    run_test(0x04);
    run_test(0x0C);
    run_test(0x14);
    run_test(0x1C);
    run_test(0x24);
    run_test(0x2C);
    run_test(0x34);
    run_test(0x3C);

    run_test(0x05);
    run_test(0x0D);
    run_test(0x15);
    run_test(0x1D);
    run_test(0x25);
    run_test(0x2D);
    run_test(0x35);
    run_test(0x3D);
}

TEST_CASE("1byte bit ops") {
    run_test(0x07);
    run_test(0x0F);
    run_test(0x17);
    run_test(0x1F);
    run_test(0x27);
    run_test(0x2F);
    run_test(0x37);
    run_test(0x3F);
}

TEST_CASE("jr i8/jr cc, i8") {
    run_test(0x18);
    run_test(0x20);
    run_test(0x28);
    run_test(0x30);
    run_test(0x38);
}

int main(const int argc, char *argv[]) {
    const int result = Catch::Session().run(argc, argv);
    return result;
}