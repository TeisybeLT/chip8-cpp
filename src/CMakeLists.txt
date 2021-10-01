set(bin_name "chip8-cpp")

# Download testing framework
message(STATUS "Downloading cxxopts")
file(DOWNLOAD https://raw.githubusercontent.com/jarro2783/cxxopts/master/include/cxxopts.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/cxxopts.hpp
	SHOW_PROGRESS
)

set(chip8_cpp_src
	sdl_environment.cpp
	sdl_window.cpp
	sdl_beeper.cpp
	timer.cpp
	input.cpp
	display.cpp
	interpreter.cpp
	main.cpp
)

add_executable(${bin_name} ${chip8_cpp_src})
target_link_libraries(${bin_name}
	PRIVATE project_options
	PRIVATE ${SDL2_LIBRARIES}
)
