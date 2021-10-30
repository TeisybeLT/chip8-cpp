# Teisybe's Chip8-cpp Interpreter

Apparently, in order to start writing any decent emulators, one has to first participate in a rite of passage and write a simple Chip 8 interpreter, that is then sacrificed to the emulator gods. You can consider this project my contribution to the aforementioned rite of passage. `<sarcasm>`Being an original dude, I chose C++ and SDL to implement this interpreter, surely nobody has done this before.`</sarcasm>`

## Features

* Full original Chip 8 implementation
* Uses completely software audio pipeline
* Simple and concise single threaded implementation (SDL2 may spawn an additional thread for audio)
* Adjustable execution speed
* Adjustable screen scaling, which preserves the original aspect ratio
* Unit tests for core components of the interpreter

## Usage

Chip8-cpp interpreter does not have a graphical user interface, so all interactions with the application happen via terminal (or command line, for you Windows peeps).

First of all, to get usage instructions, run `./chip8-cpp --help`. A helpful information screen will be presented to you with all the available options.

To load an run a Chip 8 rom, use `./chip8-cpp -r <path to .ch8 file>`.

To change execution speed, use `-f <speed>` option (default is 500 instructions per second).

To change scale, use `--upscale-mult <multiplier>` option (default is original Chip 8 resolution multiplied by 20). Extremely high multipliers may negatively impact performance.

## Building

This project utilizes CMake to generate build files for your desired compiler. Currently, only Linux builds with GCC have been tested.

During build process, CMake also downloads additional dependencies: [cxxopts](https://github.com/jarro2783/cxxopts) for command line option parsing and [doctest](https://github.com/onqtam/doctest) for unit tests (if testing is enabled).

If unit testing is desired, use an additional `-DBUILD_TESTS=On` flag for CMake.

### GNU/Linux

Requirements (can be aquired from the package manager of your selected distro):
* Cmake 3.15+
* gcc 11.1+
* SDL 2


Build instructions:
```
git clone git@github.com:TeisybeLT/chip8-cpp.git
cd chip8-cpp
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

The resulting binary will be placed in `build/bin` directory.
