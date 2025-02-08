cmake_minimum_required(VERSION 3.14)

project(cynes C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(cynes_core STATIC
    src/apu.cpp
    src/cpu.cpp
    src/ppu.cpp
    src/nes.cpp
    src/mapper.cpp
)

set_property(TARGET cynes_core PROPERTY POSITION_INDEPENDENT_CODE ON)

target_compile_options(cynes_core PRIVATE
    -Wall -Wextra -Wpedantic -Werror -Wno-missing-braces
)

target_include_directories(cynes_core PRIVATE
    src/
)

include(FetchContent)

FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11
    GIT_TAG        v2.13.6
)

FetchContent_MakeAvailable(pybind11)

pybind11_add_module(emulator
    src/wrapper.cpp
)

target_include_directories(emulator PRIVATE
    src/
)

target_compile_options(emulator PRIVATE
    -Wall -Wextra -Wpedantic -Werror
)

target_link_libraries(emulator PRIVATE
    cynes_core
)
