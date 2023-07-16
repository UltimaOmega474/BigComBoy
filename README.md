# Angbe (Another Nameless GameBoy Emulator)

An emulator for the GameBoy written in C++. Only DMG compatible games work at this time. CGB support is planned but not in active development. The project is split into two modules, Angbe the emulator core and Angbe-Gui the frontend.

## Compatibility

Below you will find the current status of various feature implementations and test cases.

- [x] PPU - Scanline rendering
- [ ] PPU - Pixel FIFO
- [ ] APU
- [x] Keyboard Input Support
- [ ] Controller Input Support
- [x] MBC1
- [ ] MBC1M
- [x] MBC2
- [ ] MBC3
- [ ] MBC5
- [ ] MBC6
- [ ] MBC7
- [ ] MMM01
- [ ] M161
- [ ] HuC1
- [ ] HuC-3
- [ ] Serial Data Transfer for Link Cable

#### Blargg's Tests

- [x] cpu_instrs
- [x] instr_timing
- [x] mem_timing
- [x] mem_timing-2

## Building

You will need a C++ compiler that supports C++20 or later. The following insturctions are to install the dependencies for Angbe-Gui. The emulator core does not have any dependencies.

Angbe-Gui uses [Native File Dialog Extended](https://github.com/btzy/nativefiledialog-extended) as a submodule, please make sure the dependencies are installed before building Angbe-Gui.


1. Download and setup [vcpkg](https://github.com/microsoft/vcpkg) and [cmake](https://cmake.org/).

2. Install imgui
```bash
vcpkg install "imgui[core,sdl2-binding,sdl2-renderer-binding]" --triplet=x64-windows
```
1. Install {fmt}
```bash
vcpkg install fmt --triplet=x64-windows
```
1. Install SDL2
```bash
vcpkg install sdl2 --triplet=x64-windows
```

1. Configure and Build the project. Binaries are written to the `bin/` folder in the source directory.

Note: To use these dependencies as a static library change triplet to: `--triplet=x64-windows-static`

## License

Angbe is open source and distributed under the MIT License. See LICENSE.md for details.