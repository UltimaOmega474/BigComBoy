find_package(Catch2 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

include(Catch)

add_executable(SM83Test sm83_tests.cpp test_types.cpp)
target_link_libraries(SM83Test PRIVATE GB fmt::fmt nlohmann_json::nlohmann_json Catch2::Catch2)

catch_discover_tests(SM83Test WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Test/GameboyCPUTests/")
