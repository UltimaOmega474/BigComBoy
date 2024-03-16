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

#include "Common/Config.hpp"
#include "GB/Debuggers/Disassembler.hpp"
#include "MainWindow.hpp"
#include "Paths.hpp"
#include <QApplication>
#include <cstdlib>
#include <filesystem>
#define SDL_MAIN_HANDLED
#include <SDL.h>

#ifdef WIN32
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

void save_config() { Common::Config::Current().write_to_file(QtFrontend::Paths::ConfigLocation()); }

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO);
    Common::Config::Current().read_from_file(QtFrontend::Paths::ConfigLocation());

    atexit(SDL_Quit);
    atexit(save_config);

    // QtFrontend::MainWindow w;
    //  w.show();

    QtFrontend::Disassembler disasm;
    disasm.show();
    return a.exec();
}
