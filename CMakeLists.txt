cmake_minimum_required(VERSION 3.5.1)

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

target_include_directories(cynes_core PRIVATE
    src/
)

include(FetchContent)

FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11
    GIT_TAG        v2.12.0
)

FetchContent_GetProperties(pybind11)

if(NOT pybind11_POPULATED)
    FetchContent_Populate(pybind11)
    add_subdirectory(${pybind11_SOURCE_DIR} ${pybind11_BINARY_DIR})
endif()

add_definitions(-DPYTHON_MODULE_VERSION="${PYTHON_MODULE_VERSION}")

pybind11_add_module(emulator
    src/wrapper.cpp
)

target_include_directories(emulator PRIVATE
    src/
)

target_link_libraries(emulator PRIVATE
    cynes_core
)
