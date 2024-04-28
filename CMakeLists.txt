cmake_minimum_required(VERSION 3.19)

project(BigComBoy VERSION 0.9 LANGUAGES CXX C)

if(APPLE)
    enable_language(OBJCXX OBJC)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)

add_subdirectory(Src)
add_subdirectory(External/toml11)
add_subdirectory(External/discord-rpc)

configure_file(LICENSE.txt ${CMAKE_SOURCE_DIR}/bin/Release/LICENSE.txt COPYONLY)
configure_file(LICENSE.txt ${CMAKE_SOURCE_DIR}/bin/Debug/LICENSE.txt COPYONLY)
configure_file(THIRD-PARTY.txt ${CMAKE_SOURCE_DIR}/bin/Release/THIRD-PARTY.txt COPYONLY)
configure_file(THIRD-PARTY.txt ${CMAKE_SOURCE_DIR}/bin/Debug/THIRD-PARTY.txt COPYONLY)