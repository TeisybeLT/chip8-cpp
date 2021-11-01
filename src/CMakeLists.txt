set(bin_name "chip8-cpp")

# Download testing framework
message(STATUS "Downloading cxxopts")
file(DOWNLOAD https://raw.githubusercontent.com/jarro2783/cxxopts/master/include/cxxopts.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/cxxopts.hpp
	SHOW_PROGRESS
)

set(chip8_cpp_src
	sdl/sdl_environment.cpp
	sdl/sdl_window.cpp
	sdl/sdl_beeper.cpp
	io/input.cpp
	io/display.cpp
	timer.cpp
	instructions.cpp
	interpreter.cpp
	main.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${bin_name} ${chip8_cpp_src})
target_link_libraries(${bin_name}
	PRIVATE project_options
	PRIVATE ${SDL2_LIBRARIES}
)
