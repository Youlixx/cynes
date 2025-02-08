#ifndef __CYNES_MAPPER__
#define __CYNES_MAPPER__

#include <cstdint>
#include <cstring>

#include "utils.hpp"

namespace cynes {
// Forward declaration.
class NES;

enum class MirroringMode : uint8_t {
    NONE, ONE_SCREEN_LOW, ONE_SCREEN_HIGH, HORIZONTAL, VERTICAL
};

// TODO: use smart pointers
struct NESMetadata {
public:
    uint16_t size_prg = 0x00;
    uint16_t size_chr = 0x00;

    uint8_t* trainer = nullptr;
    uint8_t* memory_prg = nullptr;
    uint8_t* memory_chr = nullptr;
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
        NESMetadata metadata,
        MirroringMode mode,
        uint8_t size_cpu_ram = 0x8,
        uint8_t size_ppu_ram = 0x2
    );

    virtual ~Mapper();

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

protected:
    struct MemoryBank {
    public:
        uint8_t* memory = nullptr;
        bool read_only = true;

        template<DumpOperation operation, typename T>
        constexpr void dump(T& buffer) {
            cynes::dump<operation>(buffer, memory);
            cynes::dump<operation>(buffer, read_only);
        }
    };

protected:
    NES& _nes;

protected:
    const uint16_t _size_prg;
    const uint16_t _sire_chr;

    const uint8_t _size_cpu_ram;
    const uint8_t _size_ppu_ram;

    uint8_t* _memory_prg;
    uint8_t* _memory_chr;

    uint8_t* _memory_cpu_ram;
    uint8_t* _memory_ppu_ram;

    MemoryBank _banks_cpu[0x40];
    MemoryBank _banks_ppu[0x10];

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

public:
    template<DumpOperation operation, typename T>
    constexpr void dump(T& buffer) {
        for (uint8_t k = 0x00; k < 0x40; k++) {
            _banks_cpu[k].dump<operation>(buffer);
        }

        for (uint8_t k = 0x00; k < 0x10; k++) {
            _banks_ppu[k].dump<operation>(buffer);
        }

        if (_size_cpu_ram) {
            cynes::dump<operation>(buffer, _memory_cpu_ram, _size_cpu_ram << 10);
        }

        if (_size_ppu_ram) {
            cynes::dump<operation>(buffer, _memory_ppu_ram, _size_ppu_ram << 10);
        }
    }
};


/// NROM mapper (see https://www.nesdev.org/wiki/NROM).
class NROM : public Mapper {
public:
    NROM(NES& nes, NESMetadata metadata, MirroringMode mode);
    ~NROM() = default;
};


/// MMC1 mapper (see https://www.nesdev.org/wiki/MMC1).
class MMC1 : public Mapper {
public:
    MMC1(NES& nes, NESMetadata metadata, MirroringMode mode);
    ~MMC1() = default;

public:
    /// Tick the mapper.
    virtual void tick();

    /// Write to a CPU mapped memory bank.
    /// @note This function has other side effects than simply writing to the memory, it
    /// should not be used as a memory set function.
    /// @param address Memory address within the console memory address space.
    /// @param value Value to write.
    virtual void write_cpu(uint16_t address, uint8_t value);

private:
    void write_registers(uint8_t register_target, uint8_t value);
    void update_banks();

private:
    uint8_t _tick;
    uint8_t _registers[0x4];
    uint8_t _register;
    uint8_t _counter;

public:
    template<DumpOperation operation, typename T>
    constexpr void dump(T& buffer) {
        Mapper::dump<operation>(buffer);

        cynes::dump<operation>(buffer, _tick);
        cynes::dump<operation>(buffer, _registers);
        cynes::dump<operation>(buffer, _register);
        cynes::dump<operation>(buffer, _counter);
    }
};


/// UxROM mapper (see https://www.nesdev.org/wiki/UxROM).
class UxROM : public Mapper {
public:
    UxROM(NES& nes, NESMetadata metadata, MirroringMode mode);
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
    CNROM(NES& nes, NESMetadata metadata, MirroringMode mode);
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
    MMC3(NES& nes, NESMetadata metadata, MirroringMode mode);
    ~MMC3() = default;

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

    /// Read from the PPU memory mapped banks.
    /// @note This function has other side effects than simply reading from memory, it
    /// should not be used as a memory watch function.
    /// @param address Memory address within the console memory address space.
    /// @return The value stored at the given address.
    virtual uint8_t read_ppu(uint16_t address);

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

public:
    template<DumpOperation operation, typename T>
    constexpr void dump(T& buffer) {
        Mapper::dump<operation>(buffer);

        cynes::dump<operation>(buffer, _tick);
        cynes::dump<operation>(buffer, _registers);
        cynes::dump<operation>(buffer, _counter);
        cynes::dump<operation>(buffer, _counter_reset_value);
        cynes::dump<operation>(buffer, _register_target);
        cynes::dump<operation>(buffer, _mode_prg);
        cynes::dump<operation>(buffer, _mode_chr);
        cynes::dump<operation>(buffer, _enable_interrupt);
        cynes::dump<operation>(buffer, _should_reload_interrupt);
    }
};


/// AxROM mapper (see https://www.nesdev.org/wiki/AxROM).
class AxROM : public Mapper {
public:
    AxROM(NES& nes, NESMetadata metadata);
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
    MMC(NES& nes, NESMetadata metadata, MirroringMode mode) :
        Mapper(nes, metadata, mode) {
        map_bank_chr(0x0, 0x8, 0x0);

        map_bank_prg(0x20, BANK_SIZE, 0x0);
        map_bank_prg(0x20 + BANK_SIZE, 0x20 - BANK_SIZE, _size_prg - 0x20 + BANK_SIZE);

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
    virtual void write_cpu(uint16_t address, uint8_t value) {
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
    virtual uint8_t read_ppu(uint16_t address) {
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

public:
    template<DumpOperation operation, typename T>
    constexpr void dump(T& buffer) {
        Mapper::dump<operation>(buffer);

        cynes::dump<operation>(buffer, _latches);
        cynes::dump<operation>(buffer, _selected_banks);
    }
};

using MMC2 = MMC<0x08>;
using MMC4 = MMC<0x10>;


/// GxROM mapper (see https://www.nesdev.org/wiki/GxROM).
class GxROM : public Mapper {
public:
    GxROM(NES& nes, NESMetadata metadata, MirroringMode mode);
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
