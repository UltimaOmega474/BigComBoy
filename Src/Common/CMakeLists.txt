add_library(Common STATIC
	IO.cpp
	Math.cpp
	Config.cpp
	Timer.cpp
)
target_include_directories(Common PRIVATE ${MAIN_INCLUDE_DIR})
target_link_libraries(Common PRIVATE toml11)