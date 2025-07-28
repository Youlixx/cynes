#ifndef __CYNES_EMULATOR__
#define __CYNES_EMULATOR__

#include <cstdint>
#include <memory>

#include "apu.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "mapper.hpp"

#include "utils.hpp"

namespace cynes {
/// Main NES class, contains the RAM, CPU, PPU, APU, Mapper, etc...
class NES {
public:
    // TODO maybe allow to use a constructor with a raw byte ptr.
    /// Initialize the NES.
    /// @param path Path to the ROM.
    NES(const char* path);

    /// Default destructor.
    ~NES() = default;

public:
    /// Reset the emulator (same effect as pressing the reset button).
    void reset();

    /// Perform a dummy read cycle.
    void dummy_read();

    /// Write to the console memory while ticking its components.
    /// @note This function has other side effects than simply writing to the memory, it
    /// should not be used as a memory set function.
    /// @param address Memory address within the console memory address space.
    /// @param value Value to write.
    void write(uint16_t address, uint8_t value);

    /// Write to the console memory.
    /// @note This function has other side effects than simply writing to the memory, it
    /// should not be used as a memory set function.
    /// @param address Memory address within the console memory address space.
    /// @param value Value to write.
    void write_cpu(uint16_t address, uint8_t value);

    /// Write to the PPU memory.
    /// @note This function has other side effects than simply writing to the memory, it
    /// should not be used as a memory set function.
    /// @param address Memory address within the console memory address space.
    /// @param value Value to write.
    void write_ppu(uint16_t address, uint8_t value);

    /// Write to the OAM memory.
    /// @note This function has other side effects than simply writing to the memory, it
    /// should not be used as a memory set function.
    /// @param address Memory address within the console memory address space.
    /// @param value Value to write.
    void write_oam(uint8_t address, uint8_t value);

    /// Read from the console memory while ticking its components.
    /// @note This function has other side effects than simply reading from  memory, it
    /// should not be used as a memory watch function.
    /// @param address Memory address within the console memory address space.
    /// @return The value stored at the given address.
    uint8_t read(uint16_t address);

    /// Read from the console memory.
    /// @note This function has other side effects than simply reading from memory, it
    /// should not be used as a memory watch function.
    /// @param address Memory address within the console memory address space.
    /// @return The value stored at the given address.
    uint8_t read_cpu(uint16_t address);

    /// Returns the pointer to the console's memory.
    const uint8_t* get_ram_pointer() const;

    /// Read from the PPU memory.
    /// @note This function has other side effects than simply reading from memory, it
    /// should not be used as a memory watch function.
    /// @param address Memory address within the console memory address space.
    /// @return The value stored at the given address.
    uint8_t read_ppu(uint16_t address);

    /// Read from the OAM memory.
    /// @note This function has other side effects than simply reading from memory, it
    /// should not be used as a memory watch function.
    /// @param address Memory address within the console memory address space.
    /// @return The value stored at the given address.
    uint8_t read_oam(uint8_t address) const;

    /// Get the current open bus state.
    /// @return The open bus value.
    uint8_t get_open_bus() const;

    /// Step the emulation by the given amount of frame.
    /// @param controllers Controllers states (first 8-bits for controller 1, the
    /// remaining 8-bits fro controller 2).
    /// @param frames Number of frame of the step.
    /// @return True if the CPU is frozen, false otherwise.
    bool step(uint16_t controllers, unsigned int frames);

    /// Get the size of the save state.
    /// @return The size of the save state buffer.
    unsigned int size();

    /// Save the state of the emulator to the buffer.
    /// @param buffer Save state buffer.
    void save(uint8_t* buffer);

    /// Load a previous emulator state from the buffer.
    /// @param buffer Save state buffer.
    void load(uint8_t* buffer);

    /// Get a pointer to the internal frame buffer.
    inline const uint8_t* get_frame_buffer() const {
        return ppu.get_frame_buffer();
    }

public:
    CPU cpu;
    PPU ppu;
    APU apu;

    Mapper& get_mapper();

private:
    std::unique_ptr<Mapper> _mapper;

private:
    std::unique_ptr<uint8_t[]> _memory_cpu;
    std::unique_ptr<uint8_t[]> _memory_oam;
    std::unique_ptr<uint8_t[]> _memory_palette;

    uint8_t _open_bus;

    uint8_t _controller_status[0x2];
    uint8_t _controller_shifters[0x2];

private:
    void load_controller_shifter(bool polling);

    uint8_t poll_controller(uint8_t player);

private:
    template<DumpOperation operation, class T> void dump(T& buffer);
};
}

#endif
