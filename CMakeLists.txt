cmake_minimum_required(VERSION 3.20)

if(DEFINED ENV{VCPKG_ROOT})
	set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT VCPKG_TARGET_TRIPLET)
	set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
endif()

option(MSVC_USE_STATIC_CRT "Enable MultiThreaded Runtime Library for MSVC" ON)
option(BUILD_SUNBOY_FRONTEND "Build Frontend" ON)
project(SunBoy LANGUAGES CXX C)

configure_file(LICENSE.txt ${CMAKE_SOURCE_DIR}/bin/LICENSE.txt COPYONLY)

find_package(imgui CONFIG)
find_package(fmt CONFIG REQUIRED)
find_package(SDL2 CONFIG)
find_package(SDL2_mixer CONFIG)

add_subdirectory(src/gb)

if(imgui_FOUND AND fmt_FOUND AND SDL2_FOUND AND SDL2_mixer_FOUND AND BUILD_SUNBOY_FRONTEND)
	message(STATUS "Frontend for SunBoy is enabled.")
	add_subdirectory(extern/nativefiledialog-extended)
	add_subdirectory(extern/tomlplusplus)
	add_subdirectory(src/frontend)
	configure_file(fonts/Open_Sans/OpenSans-SemiBold.ttf ${CMAKE_SOURCE_DIR}/bin/fonts/Open_Sans/OpenSans-SemiBold.ttf COPYONLY)
	configure_file(fonts/Open_Sans/OFL.txt ${CMAKE_SOURCE_DIR}/bin/fonts/Open_Sans/OFL.txt COPYONLY)
endif()