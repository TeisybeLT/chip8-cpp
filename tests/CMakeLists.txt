set(test_bin "chip8-cpp-tests")

# Download testing framework
message(STATUS "Downloading doctest")
file(DOWNLOAD https://raw.githubusercontent.com/onqtam/doctest/master/doctest/doctest.h
	${CMAKE_CURRENT_SOURCE_DIR}/doctest.h
	SHOW_PROGRESS
)

include_directories(${CMAKE_SOURCE_DIR}/src)

set(chip8_test_src
	${CMAKE_SOURCE_DIR}/src/timer.cpp
	${CMAKE_SOURCE_DIR}/src/instructions.cpp
	${CMAKE_SOURCE_DIR}/src/io/input.cpp
	timer_tests.cpp
	instruction_internals.cpp
	comparison_instructions.cpp
	flow_instructions.cpp
	ld_instructions.cpp
	math_instructions.cpp
	misc_instructions.cpp
	main.cpp
)

add_executable(${test_bin} ${chip8_test_src})
target_link_libraries(${test_bin}
	PRIVATE project_options
	PRIVATE ${SDL2_LIBRARIES})
