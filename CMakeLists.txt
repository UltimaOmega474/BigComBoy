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

add_library(SunBoy-Core)
set_target_properties(SunBoy-Core PROPERTIES
	CXX_STANDARD 20
	RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_SOURCE_DIR}/bin>"
	OUTPUT_NAME "SunBoy-Core"
)

if(MSVC_USE_STATIC_CRT)
	set_target_properties(SunBoy-Core PROPERTIES
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
	)
else()
	set_target_properties(SunBoy-Core PROPERTIES
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
	)
endif()

target_sources(SunBoy-Core PRIVATE
	src/gb/core.cpp
	src/gb/sm83.cpp
	src/gb/cartridge.cpp
	src/gb/timer.cpp
	src/gb/ppu.cpp
	src/gb/pad.cpp
	src/gb/apu.cpp
)

find_package(imgui CONFIG)
find_package(fmt CONFIG)
find_package(SDL2 CONFIG)
find_package(SDL2_mixer CONFIG)
configure_file(LICENSE.txt ${CMAKE_SOURCE_DIR}/bin/LICENSE.txt COPYONLY)

if(imgui_FOUND AND fmt_FOUND AND SDL2_FOUND AND SDL2_mixer_FOUND AND BUILD_SUNBOY_FRONTEND)
	message(STATUS "Frontend for SunBoy is enabled.")
	add_subdirectory(extern/nativefiledialog-extended)
	add_subdirectory(extern/tomlplusplus)
	add_executable(SunBoy src/frontend/main.cpp)
	target_sources(SunBoy PRIVATE
		src/frontend/config.cpp
		src/frontend/gui_constants.cpp
		src/frontend/state.cpp
		src/frontend/input.cpp
		src/frontend/palette_edit.cpp
		src/frontend/audio.cpp
		src/frontend/menu/menu.cpp
		src/frontend/menu/menu_bar.cpp
		src/frontend/menu/settings_menu.cpp
		src/frontend/menu/input_menu.cpp
	)
	set_target_properties(SunBoy PROPERTIES
		CXX_STANDARD 20
		RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_SOURCE_DIR}/bin>"
		OUTPUT_NAME "SunBoy"
	)

	if(MSVC_USE_STATIC_CRT)
		set_target_properties(SunBoy PROPERTIES
			MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
		)

		set_target_properties(nfd PROPERTIES
			MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
		)
		set_target_properties(tomlplusplus_tomlplusplus PROPERTIES
			MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
		)
	else()
		set_target_properties(SunBoy PROPERTIES
			MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
		)
		set_target_properties(nfd PROPERTIES
			MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
		)
		set_target_properties(tomlplusplus_tomlplusplus PROPERTIES
			MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
		)
	endif()

	target_include_directories(SunBoy PRIVATE src/ extern/tomlplusplus)
	target_link_libraries(SunBoy PRIVATE SunBoy-Core SDL2::SDL2 SDL2::SDL2main imgui::imgui fmt::fmt nfd tomlplusplus::tomlplusplus)
	target_link_libraries(SunBoy PRIVATE $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>)
	configure_file(fonts/Open_Sans/OpenSans-SemiBold.ttf ${CMAKE_SOURCE_DIR}/bin/fonts/Open_Sans/OpenSans-SemiBold.ttf COPYONLY)
	configure_file(fonts/Open_Sans/OFL.txt ${CMAKE_SOURCE_DIR}/bin/fonts/Open_Sans/OFL.txt COPYONLY)
endif()