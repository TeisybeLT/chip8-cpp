cmake_minimum_required(VERSION 3.15)
project(chip8-cpp CXX)

# Project options
option(BUILD_TESTS "Build unit tests" OFF)

# Dependencies
find_package(SDL2 CONFIG REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Set up outpud directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set up project options
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)

# Add subdirectories
add_subdirectory(src)

if(BUILD_TESTS)
	add_subdirectory(tests)
endif()
