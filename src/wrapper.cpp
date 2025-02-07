#include "wrapper.hpp"
#include "nes.hpp"

#include <cstdint>

#include <pybind11/cast.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>


cynes::wrapper::Wrapper::Wrapper(const char* rom)
: controller{0x00}
, _nes{rom}
, _frame{{240, 256, 3}}
, _saveStateSize{_nes.size()}
, _crashed{false} {}

cynes::wrapper::Wrapper::~Wrapper() {}

void cynes::wrapper::Wrapper::write(uint16_t address, uint8_t value) {
    _nes.write_cpu(address, value);
}

uint8_t cynes::wrapper::Wrapper::read(uint16_t address) {
    return _nes.read_cpu(address);
}

void cynes::wrapper::Wrapper::reset() {
    _nes.reset();
}

pybind11::array_t<uint8_t> cynes::wrapper::Wrapper::step(uint32_t frames) {
    _crashed |= _nes.step(
        _frame.mutable_data(),
        controller,
        frames
    );

    return _frame;
}

pybind11::array_t<uint8_t> cynes::wrapper::Wrapper::save() {
    pybind11::array_t<uint8_t> buffer{static_cast<int>(_saveStateSize)};
    _nes.save(buffer.mutable_data());

    return buffer;
}

void cynes::wrapper::Wrapper::load(pybind11::array_t<uint8_t> buffer) {
    _nes.load(buffer.mutable_data());
}

bool cynes::wrapper::Wrapper::hasCrashed() const {
    return _crashed;
}


PYBIND11_MODULE(emulator, mod) {
    mod.doc() = "C/C++ NES emulator with Python bindings";

    pybind11::class_<cynes::wrapper::Wrapper>(mod, "NES")
        .def(pybind11::init<const char*>(), pybind11::arg("rom"))
        .def_property_readonly("has_crashed", &cynes::wrapper::Wrapper::hasCrashed)
        .def_readwrite("controller", &cynes::wrapper::Wrapper::controller)
        .def("__setitem__", &cynes::wrapper::Wrapper::write, pybind11::arg("address"), pybind11::arg("value"))
        .def("__getitem__", &cynes::wrapper::Wrapper::read, pybind11::arg("address"))
        .def("reset", &cynes::wrapper::Wrapper::reset)
        .def("step", &cynes::wrapper::Wrapper::step, pybind11::arg("frames") = 1)
        .def("save", &cynes::wrapper::Wrapper::save)
        .def("load", &cynes::wrapper::Wrapper::load, pybind11::arg("buffer"))
        .doc() = "Headless NES emulator";
}
