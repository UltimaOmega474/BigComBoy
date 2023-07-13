# Angbe (Another Nameless GameBoy Emulator)

Angbe is an emulator that runs games for the GameBoy handheld console written in C++. The current version of this project only supports the original Dot Matrix Game(DMG) model. Angbe is split into two modules, Angbe-Core as a library and Angbe-Gui as the frontend.

## Compatibility

- [x] Video (Scanline rendering)
- [ ] Audio Support
- [ ] Input Support
- [x] MBC1
- [ ] MBC2
- [ ] MBC3
- [ ] MBC5
- [ ] Serial Port

## Building

You will need a C++ compiler that supports C++20 or later. The following insturctions are to install the dependencies for Angbe-Gui. Angbe-Core does not have any external dependencies.

Angbe-Gui uses [Native File Dialog Extended](https://github.com/btzy/nativefiledialog-extended) as a submodule, please make sure the dependencies are installed before building Angbe-Gui.


1. Download and setup [vcpkg](https://github.com/microsoft/vcpkg) and [cmake](https://cmake.org/).

2. Install imgui
```bash
vcpkg install "imgui[core,sdl2-binding,sdl2-renderer-binding]" --triplet=x64-windows
```
3. Install {fmt}
```bash
vcpkg install fmt --triplet=x64-windows
```
4. Install SDL2
```bash
vcpkg install sdl2 --triplet=x64-windows
```

5. Configure and Build the project. Binaries are written to the `bin/` folder in the source directory.

Note: To use these dependencies as a static library change triplet to: `--triplet=x64-windows-static`

## License

Angbe is open source and distributed under the MIT License. See LICENSE.md for details. Release packages for Angbe contain code from SDL2, {fmt} and imgui which contain their own licenses included with the release. Angbe also uses the OpenSans font. See `fonts/Open_Sans/OFL.txt` in both the source and release package for the license.