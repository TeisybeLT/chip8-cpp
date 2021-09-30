cmake_minimum_required(VERSION 3.15)
project(chip8-cpp CXX)

# Dependencies
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

include_directories(src/)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)

set(chip8-src
	src/sdl_environment.cpp
	src/sdl_window.cpp
	src/sdl_beeper.cpp
	src/timer.cpp
	src/input.cpp
	src/display.cpp
	src/interpreter.cpp
	src/main.cpp
)

add_executable(chip8-cpp ${chip8-src})
target_link_libraries(chip8-cpp PRIVATE project_options ${SDL2_LIBRARIES})
