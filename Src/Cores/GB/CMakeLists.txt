add_library(GB STATIC
	Core.cpp
	Cartridge.cpp
	Timer.cpp
	PPU.cpp
	Pad.cpp
	APU.cpp
	Bus.cpp
	DMA.cpp
	CPU.cpp
)
target_include_directories(GB INTERFACE ../)