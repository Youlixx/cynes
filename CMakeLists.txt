cmake_minimum_required(VERSION 3.14)

project(cynes C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11
    GIT_TAG        v3.0.1
)

FetchContent_MakeAvailable(pybind11)

add_library(cynes_core OBJECT
    src/apu.cpp
    src/cpu.cpp
    src/ppu.cpp
    src/nes.cpp
    src/mapper.cpp
)

set_property(TARGET cynes_core PROPERTY POSITION_INDEPENDENT_CODE ON)

target_compile_options(cynes_core PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror -Wno-missing-braces>
)

target_include_directories(cynes_core PRIVATE
    src/
)

pybind11_add_module(emulator
    src/wrapper.cpp
)

target_include_directories(emulator PRIVATE
    src/
)

target_compile_options(emulator PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

target_link_libraries(emulator PRIVATE
    cynes_core
)

target_compile_definitions(emulator PRIVATE
    PYTHON_MODULE_VERSION="${PYTHON_MODULE_VERSION}"
)

