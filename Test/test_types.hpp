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
#include <cinttypes>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

struct RamState {
    int32_t address = 0;
    int32_t value = 0;
};

struct TestState {
    int32_t a = 0;
    int32_t b = 0;
    int32_t c = 0;
    int32_t d = 0;
    int32_t e = 0;
    int32_t f = 0;
    int32_t h = 0;
    int32_t l = 0;
    int32_t pc = 0;
    int32_t sp = 0;

    std::vector<RamState> ram;
};

struct TestCycle {
    int32_t address = 0;
    int32_t value = 0;
    std::string access;
};

struct TestCase {
    std::string name;
    TestState initial;
    TestState final;
    std::vector<TestCycle> cycles;
};

void to_json(nlohmann::json &j, const RamState &value);
void to_json(nlohmann::json &j, const TestState &value);
void to_json(nlohmann::json &j, const TestCycle &value);
void to_json(nlohmann::json &j, const TestCase &value);

void from_json(const nlohmann::json &j, RamState &value);
void from_json(const nlohmann::json &j, TestState &value);
void from_json(const nlohmann::json &j, TestCycle &value);
void from_json(const nlohmann::json &j, TestCase &value);