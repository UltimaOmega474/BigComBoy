# Big ComBoy

Cross-platform Game Boy & Game Boy Color emulator written in C++. 

## Cartridge Compatibility

- [x] MBC1
- [ ] MBC1M
- [x] MBC2
- [x] MBC3/MBC30
- [x] MBC3 RTC
- [x] MBC5
- [ ] MBC6
- [ ] MBC7
- [ ] MMM01
- [ ] M161
- [ ] HuC1
- [ ] HuC-3
- [ ] Link Cable

## Building

1. Make sure your C++ compiler supports C++23.
2. Use git to clone the project with submodules.
3. Install the following dependencies: CMake, Qt Framework 6, {fmt}, SDL2.
4. Use CMake to build the BigComBoy target. 

## License

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

See LICENSE.txt for the full GPLv3 License. 

Big ComBoy also uses a number of third party libraries, see THIRD-PARTY.txt for more info.