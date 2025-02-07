#ifndef __CYNES_WRAPPER__
#define __CYNES_WRAPPER__

#include <cstdint>

#include <pybind11/numpy.h>

#include "nes.hpp"


namespace cynes {
namespace wrapper {
class Wrapper {
public:
    Wrapper(const char* rom);
    ~Wrapper();

    void write(uint16_t address, uint8_t value);
    uint8_t read(uint16_t address);

    void reset();

    const pybind11::array_t<uint8_t>& step(uint32_t frames);

    pybind11::array_t<uint8_t> save();
    void load(pybind11::array_t<uint8_t> dump);

    bool has_crashed() const;

public:
    uint16_t controller;

private:
    NES _nes;
    const size_t _save_state_size;

    pybind11::array_t<uint8_t> _frame;
    bool _crashed;
};
}
}

#endif
