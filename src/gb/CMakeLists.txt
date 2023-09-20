add_library(SunBoy-Core)
set_target_properties(SunBoy-Core PROPERTIES
	CXX_STANDARD 20
	RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_SOURCE_DIR}/bin>"
	OUTPUT_NAME "SunBoy-Core"
)
target_link_libraries(SunBoy-Core PRIVATE fmt::fmt)

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
	core.cpp
	sm83.cpp
	cartridge.cpp
	timer.cpp
	ppu.cpp
	pad.cpp
	apu.cpp
	bus.cpp
)