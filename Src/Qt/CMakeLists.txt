set(QT_SOURCES
	main.cpp
	Paths.cpp
	MainWindow.cpp
	MainWindow.ui
	KeyboardDevice.cpp
	EmulatorView.cpp
	GB/GBEmulatorController.cpp
	GB/AudioSystem.cpp
	GB/SubWindows/SettingsWindow.cpp
	GB/SubWindows/SettingsWindow.ui
	GB/SubWindows/EmulationWindow.cpp
	GB/SubWindows/EmulationWindow.ui
	GB/SubWindows/InputWindow.cpp
	GB/SubWindows/InputWindow.ui
	GB/SubWindows/VideoWindow.cpp
	GB/SubWindows/VideoWindow.ui
	GB/SubWindows/AudioWindow.cpp
	GB/SubWindows/AudioWindow.ui
	GB/Debuggers/Disassembler.cpp
	GB/Debuggers/Disassembler.ui
)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

if(APPLE)
	set(MACOSX_BUNDLE_ICON_FILE bcb)
	set(APP_ICON_MACOSX ${CMAKE_SOURCE_DIR}/Icons/MacOS/bcb.icns)
	set_source_files_properties(${APP_ICON_MACOSX} PROPERTIES
		MACOSX_PACKAGE_LOCATION "Resources")

	qt_add_executable(BigComBoy MACOSX_BUNDLE
		${QT_SOURCES}
		${APP_ICON_MACOSX}
	)

elseif(WIN32)
	set(APP_ICON_RESOURCE_WINDOWS ${CMAKE_SOURCE_DIR}/Icons/Windows/bcb.rc)
	qt_add_executable(BigComBoy
		${QT_SOURCES}

		${APP_ICON_RESOURCE_WINDOWS}
	)
else()
	qt_add_executable(BigComBoy
		${QT_SOURCES}
	)
endif()

target_include_directories(BigComBoy PRIVATE ${MAIN_INCLUDE_DIR})

target_link_libraries(BigComBoy PRIVATE
	Qt6::Widgets
	toml11
	fmt::fmt
	GB
	Common
	SDL2::SDL2
	GL
	Input
)

set_target_properties(BigComBoy PROPERTIES
	OUTPUT_NAME "BigComBoy"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
	MACOSX_BUNDLE_GUI_IDENTIFIER com.ultimaomega.bigcomboy
	MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
	MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
	MACOSX_BUNDLE TRUE
	WIN32_EXECUTABLE TRUE
)

install(TARGETS BigComBoy
	BUNDLE DESTINATION .
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
