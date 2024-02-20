add_library(Input STATIC
	DeviceRegistry.cpp
	SDLControllerDevice.cpp
)

target_link_libraries(Input PRIVATE
	Common
	SDL2::SDL2
)