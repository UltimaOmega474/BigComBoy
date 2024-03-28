# Big ComBoy

Game Boy & Game Boy Color emulator written in C++. 

## Hardware Compatibility

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

Windows and MacOS are currently supported. Linux port is planned but not a priority.

A C++ compiler that supports C++20 along with CMake is required.

The following external dependencies are required. 

- Qt6 
- fmtlib
- SDL2
- GLEW
- toml11 (Included as a submodule)

Install them using your favorite package manager on your platform of choice. Windows users should consider using vcpkg.

Use the provided CMakeLists to configure. The frontend can be built via the BigComBoy target. The GB target contains the emulator core and has no external dependencies required for use.

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