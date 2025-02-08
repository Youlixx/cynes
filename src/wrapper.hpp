#ifndef __CYNES_WRAPPER__
#define __CYNES_WRAPPER__

#include "nes.hpp"

#include <pybind11/numpy.h>
#include <cstdint>

namespace cynes {
namespace wrapper {
/// NES Wrapper for Python bindings.
class NesWrapper {
public:
    /// Initialize the emulator.
    /// @param path_rom Path to the ROM file.
    NesWrapper(const char* path_rom);

    // Default destructor.
    ~NesWrapper() = default;

    /// Step the emulation by the given amount of frame.
    /// @param frames Number of frame of the step.
    /// @return Read-only framebuffer.
    const pybind11::array_t<uint8_t>& step(uint32_t frames);

    /// Return a save state of the emulator.
    /// @return Save state buffer.
    pybind11::array_t<uint8_t> save();

    /// Load a previous emulator state from a buffer.
    /// @note This function also reset the crashed flag.
    /// @param buffer Save state buffer.
    void load(pybind11::array_t<uint8_t> buffer);

    /// Write to the console memory.
    /// @note This function has other side effects than simply writing to the memory, it
    /// should not be used as a memory set function.
    /// @param address Memory address within the console memory address space.
    /// @param value Value to write.
    inline void write(uint16_t address, uint8_t value) {
        _nes.write_cpu(address, value);
    }

    /// Read from the console memory while ticking its components.
    /// @note This function has other side effects than simply reading from memory, it
    /// should not be used as a memory watch function.
    /// @param address Memory address within the console memory address space.
    /// @return The value stored at the given address.
    inline uint8_t read(uint16_t address) { return _nes.read_cpu(address); }

    /// Reset the emulator (same effect as pressing the reset button).
    inline void reset() { _nes.reset(); }

    /// Check whether or not the emulator has hit a JAM instruction.
    /// @note When the emulator has crashed, subsequent calls to `NesWrapper::step` will
    /// not do anything. Resetting the emulator or loading a valid save-state will reset
    /// this flag.
    /// @return True if the emulator crashed, false otherwise.
    inline bool has_crashed() const { return _crashed; }

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
