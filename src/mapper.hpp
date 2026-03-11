#ifndef __CYNES_MAPPER__
#define __CYNES_MAPPER__

#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>

#include "save_state.hpp"

namespace cynes {
// Forward declaration.
class NES;

enum class MirroringMode : uint8_t {
    NONE, ONE_SCREEN_LOW, ONE_SCREEN_HIGH, HORIZONTAL, VERTICAL
};

/// Simple wrapper storing memory parsed from a ROM file.
struct ParsedMemory {
public:
    bool read_only_chr = true;
    uint16_t size_prg = 0x00;
    uint16_t size_chr = 0x00;

    std::unique_ptr<uint8_t[]> trainer;
    std::unique_ptr<uint8_t[]> memory_prg;
    std::unique_ptr<uint8_t[]> memory_chr;
};

/// Generic NES Mapper (see https://www.nesdev.org/wiki/Mapper).
class Mapper {
public:
    /// Initialize the mapper.
    /// @param nes Emulator.
    /// @param metadata ROM metadata.
    /// @param mode Mapper mirroring mode.
    /// @param size_cpu_ram Size of the CPU RAM.
    /// @param size_ppu_ram Size of the PPU RAM.
    Mapper(
        NES& nes,
        const ParsedMemory& metadata,
        MirroringMode mode,
        uint8_t size_cpu_ram = 0x8,
        uint8_t size_ppu_ram = 0x2
    );

    /// Default destructor.
    virtual ~Mapper() = default;

    /// Load and deserialize a ROM into a mapper.
    /// @param nes Emulator.
    /// @param path_rom Path to the NES ROM file.
    /// @return A pointer to the instantiated mapper.
    static std::unique_ptr<Mapper> load_mapper(
        NES& nes,
        const std::filesystem::path& path_rom
    );

public:
    /// Tick the mapper.
    virtual void tick();

    /// Write to a CPU mapped memory bank.
    /// @note This function has other side effects than simply writing to the memory, it
    /// should not be used as a memory set function.
    /// @param address Memory address within the console memory address space.
    /// @param value Value to write.
    virtual void write_cpu(uint16_t address, uint8_t value);

    /// Write to a PPU mapped memory bank.
    /// @note This function has other side effects than simply writing to the memory, it
    /// should not be used as a memory set function.
    /// @param address Memory address within the console memory address space.
    /// @param value Value to write.
    virtual void write_ppu(uint16_t address, uint8_t value);

    /// Read from the CPU memory mapped banks.
    /// @note This function has other side effects than simply reading from memory, it
    /// should not be used as a memory watch function.
    /// @param address Memory address within the console memory address space.
    /// @return The value stored at the given address.
    virtual uint8_t read_cpu(uint16_t address);

    /// Read from the PPU memory mapped banks.
    /// @note This function has other side effects than simply reading from memory, it
    /// should not be used as a memory watch function.
    /// @param address Memory address within the console memory address space.
    /// @return The value stored at the given address.
    virtual uint8_t read_ppu(uint16_t address);

    /// Stream the mapper state into / out of a save state.
    /// @param save_state Current save state.
    virtual void stream_state(SaveState& save_state);

protected:
    /// A memory bank provides a view within the mapper memory.
    // Each bank is exactly 0x400 bytes large.
    struct MemoryBank {
    public:
        /// Initialize an unmapped bank.
        MemoryBank();

        /// Initialize a mapped bank using the given offset.
        /// @param offset Mapper memory offset.
        /// @param read_only Bank read only flag.
        MemoryBank(size_t offset, bool read_only);

        /// Default destructor.
        ~MemoryBank() = default;

        /// Stream the memory bank state into / out of a save state.
        /// @param save_state Current save state.
        void stream_state(SaveState& save_state);

    public:
        size_t offset;
        bool read_only;
        bool mapped;
    };

protected:
    NES& _nes;

protected:
    const uint16_t _banks_prg;
    const uint16_t _banks_chr;
    const uint8_t _banks_cpu_ram;
    const uint8_t _banks_ppu_ram;

private:
    const size_t _size_prg;
    const size_t _size_chr;
    const size_t _size_cpu_ram;
    const size_t _size_ppu_ram;
    const bool _read_only_chr;

    std::unique_ptr<uint8_t[]> _memory;

    std::array<MemoryBank, 0x40> _banks_cpu;
    std::array<MemoryBank, 0x10> _banks_ppu;

protected:
    void map_bank_prg(uint8_t page, uint16_t address);
    void map_bank_prg(uint8_t page, uint8_t size, uint16_t address);

    void map_bank_cpu_ram(uint8_t page, uint16_t address, bool read_only);
    void map_bank_cpu_ram(uint8_t page, uint8_t size, uint16_t address, bool read_only);

    void map_bank_chr(uint8_t page, uint16_t address);
    void map_bank_chr(uint8_t page, uint8_t size, uint16_t address);

    void map_bank_ppu_ram(uint8_t page, uint16_t address, bool read_only);
    void map_bank_ppu_ram(uint8_t page, uint8_t size, uint16_t address, bool read_only);

    void unmap_bank_cpu(uint8_t page);
    void unmap_bank_cpu(uint8_t page, uint8_t size);

    void set_mirroring_mode(MirroringMode mode);

    void mirror_cpu_banks(uint8_t page, uint8_t size, uint8_t mirror);
    void mirror_ppu_banks(uint8_t page, uint8_t size, uint8_t mirror);
};


/// NROM mapper (see https://www.nesdev.org/wiki/NROM).
class NROM : public Mapper {
public:
    /// Initialize the mapper.
    /// @param nes Emulator.
    /// @param metadata ROM metadata.
    /// @param mode Mapper mirroring mode.
    NROM(NES& nes, const ParsedMemory& metadata, MirroringMode mode);

    /// Default destructor.
    ~NROM() = default;
};


/// MMC1 mapper (see https://www.nesdev.org/wiki/MMC1).
class MMC1 : public Mapper {
public:
    MMC1(NES& nes, const ParsedMemory& metadata, MirroringMode mode);
    ~MMC1() = default;

public:
    /// Tick the mapper.
    void tick() override;

    /// Write to a CPU mapped memory bank.
    /// @note This function has other side effects than simply writing to the memory, it
    /// should not be used as a memory set function.
    /// @param address Memory address within the console memory address space.
    /// @param value Value to write.
    void write_cpu(uint16_t address, uint8_t value) override;

    /// Stream the mapper state into / out of a save state.
    /// @param save_state Current save state.
    void stream_state(SaveState& save_state) override;

private:
    void write_registers(uint8_t register_target, uint8_t value);
    void update_banks();

private:
    uint8_t _tick;
    uint8_t _registers[0x4];
    uint8_t _register;
    uint8_t _counter;
};


/// UxROM mapper (see https://www.nesdev.org/wiki/UxROM).
class UxROM : public Mapper {
public:
    UxROM(NES& nes, const ParsedMemory& metadata, MirroringMode mode);
    ~UxROM() = default;

public:
    /// Write to a CPU mapped memory bank.
    /// @note This function has other side effects than simply writing to the memory, it
    /// should not be used as a memory set function.
    /// @param address Memory address within the console memory address space.
    /// @param value Value to write.
    virtual void write_cpu(uint16_t address, uint8_t value);
};


/// CNROM mapper (see https://www.nesdev.org/wiki/CNROM).
class CNROM : public Mapper {
public:
    CNROM(NES& nes, const ParsedMemory& metadata, MirroringMode mode);
    ~CNROM() = default;

public:
    /// Write to a CPU mapped memory bank.
    /// @note This function has other side effects than simply writing to the memory, it
    /// should not be used as a memory set function.
    /// @param address Memory address within the console memory address space.
    /// @param value Value to write.
    virtual void write_cpu(uint16_t address, uint8_t value);
};


/// MMC3 mapper (see https://www.nesdev.org/wiki/MMC3).
class MMC3 : public Mapper {
public:
    MMC3(NES& nes, const ParsedMemory& metadata, MirroringMode mode);
    ~MMC3() = default;

public:
    /// Tick the mapper.
    void tick() override;

    /// Write to a CPU mapped memory bank.
    /// @note This function has other side effects than simply writing to the memory, it
    /// should not be used as a memory set function.
    /// @param address Memory address within the console memory address space.
    /// @param value Value to write.
    void write_cpu(uint16_t address, uint8_t value) override;

    /// Write to a PPU mapped memory bank.
    /// @note This function has other side effects than simply writing to the memory, it
    /// should not be used as a memory set function.
    /// @param address Memory address within the console memory address space.
    /// @param value Value to write.
    void write_ppu(uint16_t address, uint8_t value) override;

    /// Read from the PPU memory mapped banks.
    /// @note This function has other side effects than simply reading from memory, it
    /// should not be used as a memory watch function.
    /// @param address Memory address within the console memory address space.
    /// @return The value stored at the given address.
    uint8_t read_ppu(uint16_t address) override;

    /// Stream the mapper state into / out of a save state.
    /// @param save_state Current save state.
    void stream_state(SaveState& save_state) override;

private:
    void update_state(bool state);

private:
    uint32_t _tick;
    uint32_t _registers[0x8];
    uint16_t _counter;
    uint16_t _counter_reset_value;

    uint8_t _register_target;

    bool _mode_prg;
    bool _mode_chr;
    bool _enable_interrupt;
    bool _should_reload_interrupt;
};


/// AxROM mapper (see https://www.nesdev.org/wiki/AxROM).
class AxROM : public Mapper {
public:
    AxROM(NES& nes, const ParsedMemory& metadata);
    ~AxROM() = default;

public:
    /// Write to a CPU mapped memory bank.
    /// @note This function has other side effects than simply writing to the memory, it
    /// should not be used as a memory set function.
    /// @param address Memory address within the console memory address space.
    /// @param value Value to write.
    virtual void write_cpu(uint16_t address, uint8_t value);
};

/// Generic MMC mapper (see https://www.nesdev.org/wiki/MMC2).
template<uint8_t BANK_SIZE>
class MMC : public Mapper {
public:
    MMC(NES& nes, const ParsedMemory& metadata, MirroringMode mode) :
        Mapper(nes, metadata, mode) {
        map_bank_chr(0x0, 0x8, 0x0);

        map_bank_prg(0x20, BANK_SIZE, 0x0);
        map_bank_prg(0x20 + BANK_SIZE, 0x20 - BANK_SIZE, _banks_prg - 0x20 + BANK_SIZE);

        map_bank_cpu_ram(0x18, 0x8, 0x0, true);

        memset(_latches, false, 0x2);
        memset(_selected_banks, 0x0, 0x4);
    }

    ~MMC() = default;

public:
    /// Write to a CPU mapped memory bank.
    /// @note This function has other side effects than simply writing to the memory, it
    /// should not be used as a memory set function.
    /// @param address Memory address within the console memory address space.
    /// @param value Value to write.
    void write_cpu(uint16_t address, uint8_t value) override {
        if (address < 0xA000) {
            Mapper::write_cpu(address, value);
        } else if (address < 0xB000) {
            map_bank_prg(0x20, BANK_SIZE, (value & 0xF) * BANK_SIZE);
        } else if (address < 0xC000) {
            _selected_banks[0x0] = value & 0x1F; update_banks();
        } else if (address < 0xD000) {
            _selected_banks[0x1] = value & 0x1F; update_banks();
        } else if (address < 0xE000) {
            _selected_banks[0x2] = value & 0x1F; update_banks();
        } else if (address < 0xF000) {
            _selected_banks[0x3] = value & 0x1F; update_banks();
        } else {
            if (value & 0x01) {
                set_mirroring_mode(MirroringMode::HORIZONTAL);
            } else {
                set_mirroring_mode(MirroringMode::VERTICAL);
            }
        }
    }

    /// Read from the PPU memory mapped banks.
    /// @note This function has other side effects than simply reading from memory, it
    /// should not be used as a memory watch function.
    /// @param address Memory address within the console memory address space.
    /// @return The value stored at the given address.
    uint8_t read_ppu(uint16_t address) override {
        uint8_t value = Mapper::read_ppu(address);

        if (address == 0x0FD8) {
            _latches[0] = true; update_banks();
        } else if (address == 0x0FE8) {
            _latches[0] = false; update_banks();
        } else if (address >= 0x1FD8 && address < 0x1FE0) {
            _latches[1] = true; update_banks();
        } else if (address >= 0x1FE8 && address < 0x1FF0) {
            _latches[1] = false; update_banks();
        }

        return value;
    }

    /// Stream the mapper state into / out of a save state.
    /// @param save_state Current save state.
    void stream_state(SaveState& save_state) override {
        Mapper::stream_state(save_state);
        save_state.stream(_latches);
        save_state.stream(_selected_banks);
    }

private:
    void update_banks() {
        if (_latches[0]) {
            map_bank_chr(0x0, 0x4, _selected_banks[0x0] << 2);
        } else {
            map_bank_chr(0x0, 0x4, _selected_banks[0x1] << 2);
        }

        if (_latches[1]) {
            map_bank_chr(0x4, 0x4, _selected_banks[0x2] << 2);
        } else {
            map_bank_chr(0x4, 0x4, _selected_banks[0x3] << 2);
        }
    }

private:
    bool _latches[0x2];

    uint8_t _selected_banks[0x4];
};

using MMC2 = MMC<0x08>;
using MMC4 = MMC<0x10>;


/// GxROM mapper (see https://www.nesdev.org/wiki/GxROM).
class GxROM : public Mapper {
public:
    GxROM(NES& nes, const ParsedMemory& metadata, MirroringMode mode);
    ~GxROM() = default;

public:
    /// Write to a CPU mapped memory bank.
    /// @note This function has other side effects than simply writing to the memory, it
    /// should not be used as a memory set function.
    /// @param address Memory address within the console memory address space.
    /// @param value Value to write.
    virtual void write_cpu(uint16_t address, uint8_t value);
};
}

#endif
