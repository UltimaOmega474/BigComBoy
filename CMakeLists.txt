cmake_minimum_required(VERSION 3.20)

if(DEFINED ENV{VCPKG_ROOT})
	set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT VCPKG_TARGET_TRIPLET)
	set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
endif()

option(MSVC_USE_STATIC_CRT "Enable MultiThreaded Runtime Library for MSVC" ON)

project(Angbe LANGUAGES CXX C)

add_library(Angbe)
set_target_properties(Angbe PROPERTIES
	CXX_STANDARD 20
	RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_SOURCE_DIR}/bin>"
	OUTPUT_NAME "Angbe"
)

if(MSVC_USE_STATIC_CRT)
	set_target_properties(Angbe PROPERTIES
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
	)
else()
	set_target_properties(Angbe PROPERTIES
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
	)
endif()

target_sources(Angbe PRIVATE
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

if(imgui_FOUND AND fmt_FOUND AND SDL2_FOUND)
	add_subdirectory(extern/nativefiledialog-extended)

	add_executable(Angbe-Gui src/frontend/main.cpp)
	target_sources(Angbe-Gui PRIVATE
		src/frontend/config.cpp
		src/frontend/menu_bar.cpp
		src/frontend/gui_constants.cpp
		src/frontend/state.cpp
		src/frontend/input.cpp
		src/frontend/palette_edit.cpp
	)
	set_target_properties(Angbe-Gui PROPERTIES
		CXX_STANDARD 20
		RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_SOURCE_DIR}/bin>"
		OUTPUT_NAME "Angbe-Gui"
	)

	if(MSVC_USE_STATIC_CRT)
		set_target_properties(Angbe-Gui PROPERTIES
			MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
		)

		set_target_properties(nfd PROPERTIES
			MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
		)
	else()
		set_target_properties(Angbe-Gui PROPERTIES
			MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
		)
		set_target_properties(nfd PROPERTIES
			MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
		)
	endif()

	target_include_directories(Angbe-Gui PRIVATE src/)
	target_link_libraries(Angbe-Gui PRIVATE Angbe SDL2::SDL2 SDL2::SDL2main imgui::imgui fmt::fmt nfd)

	configure_file(fonts/Open_Sans/OpenSans-VariableFont_wdth,wght.ttf ${CMAKE_SOURCE_DIR}/bin/fonts/Open_Sans/OpenSans-VariableFont_wdth,wght.ttf COPYONLY)
	configure_file(fonts/Open_Sans/OpenSans-SemiBold.ttf ${CMAKE_SOURCE_DIR}/bin/fonts/Open_Sans/OpenSans-SemiBold.ttf COPYONLY)
	configure_file(fonts/Open_Sans/OFL.txt ${CMAKE_SOURCE_DIR}/bin/fonts/Open_Sans/OFL.txt COPYONLY)
endif()