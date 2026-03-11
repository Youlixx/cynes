#include "wrapper.hpp"
#include "nes.hpp"
#include "save_state.hpp"

#include <cstdint>

#include <pybind11/cast.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl/filesystem.h>

using namespace cynes;

namespace {
/// Get the save state size.
/// @param nes Emulator with the loaded ROM.
/// @return Save state size.
size_t get_save_state_size(NES& nes) {
    SaveState save_state{SaveState::Mode::Size};
    nes.stream_state(save_state);
    return save_state.size();
}
}

wrapper::NesWrapper::NesWrapper(const std::filesystem::path& path_rom)
    : controller{0x00}
    , _nes{path_rom}
    , _save_state_size{get_save_state_size(_nes)}
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

const pybind11::array_t<uint8_t>& wrapper::NesWrapper::step(uint32_t frames) {
    _crashed |= _nes.step(controller, frames);
    return _frame;
}

pybind11::array_t<uint8_t> wrapper::NesWrapper::save() {
    pybind11::array_t<uint8_t> buffer{static_cast<int>(_save_state_size)};
    SaveState save_state{SaveState::Mode::Save, buffer.mutable_data()};
    _nes.stream_state(save_state);
    return buffer;
}

void wrapper::NesWrapper::load(pybind11::array_t<uint8_t> buffer) {
    SaveState save_state{SaveState::Mode::Load, buffer.mutable_data()};
    _nes.stream_state(save_state);
    _crashed = false;
}


PYBIND11_MODULE(emulator, mod) {
    mod.doc() = "C/C++ NES emulator with Python bindings";

#ifdef PYTHON_MODULE_VERSION
    mod.attr("__version__") = PYTHON_MODULE_VERSION;
#else
    mod.attr("__version__") = "0.0.0";
#endif

    pybind11::class_<wrapper::NesWrapper>(mod, "NES")
        .def(
            pybind11::init<const std::filesystem::path&>(),
            pybind11::arg("path_rom"),
            "Initialize the emulator."
        )
        .def(
            "__setitem__",
            &wrapper::NesWrapper::write,
            pybind11::arg("address"),
            pybind11::arg("value"),
            "Write a value in the emulator memory at the specified address."
        )
        .def(
            "__getitem__",
            &wrapper::NesWrapper::read,
            pybind11::arg("address"),
            "Read a value in the emulator memory at the specified address."
        )
        .def(
            "reset",
            &wrapper::NesWrapper::reset,
            "Send a reset signal to the emulator."
        )
        .def(
            "step",
            &wrapper::NesWrapper::step,
            pybind11::arg("frames") = 1,
            "Run the emulator for the specified amount of frame."
        )
        .def(
            "save",
            &wrapper::NesWrapper::save,
            "Dump the current emulator state into a save state."
        )
        .def(
            "load",
            &wrapper::NesWrapper::load,
            pybind11::arg("buffer"),
            "Restore the emulator state from a save state."
        )
        .def_readwrite(
            "controller",
            &wrapper::NesWrapper::controller,
            "Emulator controller state."
        )
        .def_property_readonly(
            "has_crashed",
            &wrapper::NesWrapper::has_crashed,
            "Indicate whether the CPU crashed after hitting an invalid op-code."
        )
        .doc() = "Headless NES emulator";
}
