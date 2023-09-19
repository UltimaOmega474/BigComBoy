add_executable(SunBoy main.cpp)
target_sources(SunBoy PRIVATE
	config.cpp
	gui_constants.cpp
	state.cpp
	input.cpp
	palette_edit.cpp
	audio.cpp
	menu/menu.cpp
	menu/menu_bar.cpp
	menu/settings_menu.cpp
	menu/input_menu.cpp
	menu/emulation_menu.cpp
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

target_include_directories(SunBoy PRIVATE ../ ../../extern/tomlplusplus)
target_link_libraries(SunBoy PRIVATE SunBoy-Core SDL2::SDL2 SDL2::SDL2main imgui::imgui fmt::fmt nfd tomlplusplus::tomlplusplus)
target_link_libraries(SunBoy PRIVATE $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>)