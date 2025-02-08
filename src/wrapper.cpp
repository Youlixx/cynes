#include "wrapper.hpp"
#include "nes.hpp"

#include <cstdint>

#include <pybind11/cast.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>


cynes::wrapper::NesWrapper::NesWrapper(const char* path_rom)
    : controller{0x00}
    , _nes{path_rom}
    , _save_state_size{_nes.size()}
    , _frame{
        {240, 256, 3},
        {256 * 3, 3, 1},
        _nes.get_frame_buffer(),
        pybind11::capsule(_nes.get_frame_buffer(), [](void *) {})
    }
    , _crashed{false}
{
    pybind11::detail::array_proxy(_frame.ptr())->flags &= ~pybind11::detail::npy_api::NPY_ARRAY_WRITEABLE_;
}

const pybind11::array_t<uint8_t>& cynes::wrapper::NesWrapper::step(uint32_t frames) {
    _crashed |= _nes.step(controller, frames);
    return _frame;
}

pybind11::array_t<uint8_t> cynes::wrapper::NesWrapper::save() {
    pybind11::array_t<uint8_t> buffer{static_cast<int>(_save_state_size)};
    _nes.save(buffer.mutable_data());
    return buffer;
}

void cynes::wrapper::NesWrapper::load(pybind11::array_t<uint8_t> buffer) {
    _nes.load(buffer.mutable_data());
    _crashed = false;
}


PYBIND11_MODULE(emulator, mod) {
    mod.doc() = "C/C++ NES emulator with Python bindings";

#ifdef PYTHON_MODULE_VERSION
    mod.attr("__version__") = PYTHON_MODULE_VERSION;
#else
    mod.attr("__version__") = "0.0.0";
#endif

    pybind11::class_<cynes::wrapper::NesWrapper>(mod, "NES")
        .def(
            pybind11::init<const char*>(),
            pybind11::arg("path_rom"),
            "Initialize the emulator."
        )
        .def(
            "__setitem__",
            &cynes::wrapper::NesWrapper::write,
            pybind11::arg("address"),
            pybind11::arg("value"),
            "Write a value in the emulator memory at the specified address."
        )
        .def(
            "__getitem__",
            &cynes::wrapper::NesWrapper::read,
            pybind11::arg("address"),
            "Read a value in the emulator memory at the specified address."
        )
        .def(
            "reset",
            &cynes::wrapper::NesWrapper::reset,
            "Send a reset signal to the emulator."
        )
        .def(
            "step",
            &cynes::wrapper::NesWrapper::step,
            pybind11::arg("frames") = 1,
            "Run the emulator for the specified amount of frame."
        )
        .def(
            "save",
            &cynes::wrapper::NesWrapper::save,
            "Dump the current emulator state into a save state."
        )
        .def(
            "load",
            &cynes::wrapper::NesWrapper::load,
            pybind11::arg("buffer"),
            "Restore the emulator state from a save state."
        )
        .def_readwrite(
            "controller",
            &cynes::wrapper::NesWrapper::controller,
            "Emulator controller state."
        )
        .def_property_readonly(
            "has_crashed",
            &cynes::wrapper::NesWrapper::has_crashed,
            "Indicate whether the CPU crashed after hitting an invalid op-code."
        )
        .doc() = "Headless NES emulator";
}
