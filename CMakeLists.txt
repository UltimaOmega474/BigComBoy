cmake_minimum_required(VERSION 3.19)

project(BigComBoy VERSION 0.9 LANGUAGES CXX C OBJCXX OBJC)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)

add_subdirectory(Src)
add_subdirectory(External/toml11)