add_library(Common STATIC
	Math.cpp
	Config.cpp
)
target_include_directories(Common PRIVATE ${MAIN_INCLUDE_DIR})
target_link_libraries(Common PRIVATE toml11)