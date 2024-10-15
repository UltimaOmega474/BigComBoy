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
#include <nlohmann/json.hpp>

// Unused, tests are only loaded.
void to_json(nlohmann::json &j, const RamState &value) {}
void to_json(nlohmann::json &j, const TestState &value) {}
void to_json(nlohmann::json &j, const TestCycle &value) {}
void to_json(nlohmann::json &j, const TestCase &value) {}

void from_json(const nlohmann::json &j, RamState &value) {
    value.address = j[0].get_to(value.address);
    value.value = j[1].get_to(value.value);
}

void from_json(const nlohmann::json &j, TestState &value) {
    j.at("a").get_to(value.a);
    j.at("b").get_to(value.b);
    j.at("c").get_to(value.c);
    j.at("d").get_to(value.d);
    j.at("e").get_to(value.e);
    j.at("f").get_to(value.f);
    j.at("h").get_to(value.h);
    j.at("l").get_to(value.l);
    j.at("pc").get_to(value.pc);
    j.at("sp").get_to(value.sp);

    for (const auto &ram : j["ram"]) {
        value.ram.push_back(ram);
    }
}

void from_json(const nlohmann::json &j, TestCycle &value) {
    if (j.is_null()) {
        return;
    }
    value.address = j[0].get_to(value.address);
    value.value = j[1].get_to(value.value);
    value.access = j[2].get_to(value.access);
}

void from_json(const nlohmann::json &j, TestCase &value) {

    value.name = j.at("name").get_to(value.name);
    value.initial = j.at("initial").get_to(value.initial);
    value.final = j.at("final").get_to(value.final);

    for (const auto &cycle : j["cycles"]) {
        value.cycles.push_back(cycle);
    }
}
