#include "mapper.hpp"
#include "cpu.hpp"
#include "nes.hpp"
#include <fstream>


cynes::Mapper::MemoryBank::MemoryBank()
    : offset{0}, read_only{true}, mapped{false} {}

cynes::Mapper::MemoryBank::MemoryBank(size_t offset, bool read_only)
    : offset{offset}, read_only{read_only}, mapped{true} {}


cynes::Mapper::Mapper(
    NES& nes,
    const NESMetadata& metadata,
    MirroringMode mode,
    uint8_t size_cpu_ram,
    uint8_t size_ppu_ram
) : _nes{nes}
  , _banks_prg{metadata.size_prg}
  , _banks_chr{metadata.size_chr}
  , _banks_cpu_ram{size_cpu_ram}
  , _banks_ppu_ram{size_ppu_ram}
  , _size_prg{static_cast<size_t>(_banks_prg) << 10}
  , _size_chr{static_cast<size_t>(_banks_chr) << 10}
  , _size_cpu_ram{static_cast<size_t>(_banks_cpu_ram) << 10}
  , _size_ppu_ram{static_cast<size_t>(_banks_ppu_ram) << 10}
  , _memory{new uint8_t[_size_prg + _size_chr + _size_cpu_ram + _size_ppu_ram]}
  , _banks_cpu{}
  , _banks_ppu{}
{
    if (metadata.memory_prg != nullptr) {
        std::memcpy(
            _memory.get(),
            metadata.memory_prg.get(),
            _size_prg
        );
    }

    if (metadata.memory_chr != nullptr) {
        std::memcpy(
            _memory.get() + _size_prg,
            metadata.memory_chr.get(),
            _size_chr
        );
    }

    if (metadata.trainer != nullptr) {
        std::memcpy(
            _memory.get() + _size_prg + _size_chr,
            metadata.trainer.get(),
            0x200
        );
    }

    set_mirroring_mode(mode);
}

std::unique_ptr<cynes::Mapper> cynes::Mapper::load_mapper(
    NES &nes,
    const std::filesystem::path& path_rom
) {
    std::ifstream stream{path_rom, std::ios::binary};

    if (!stream.is_open()) {
        throw std::runtime_error("The file cannot be read.");
    }

    uint32_t header;
    stream.read(reinterpret_cast<char*>(&header), sizeof(uint32_t));

    if (header != 0x1A53454E) {
        throw std::runtime_error("The specified file is not a NES ROM.");
    }

    uint8_t program_banks = stream.get();
    uint8_t character_banks = stream.get();
    uint8_t flag6 = stream.get();
    uint8_t flag7 = stream.get();

    stream.seekg(8, std::ios::cur);

    cynes::NESMetadata metadata{};
    metadata.size_prg = static_cast<uint16_t>(program_banks) << 4;
    metadata.size_chr = static_cast<uint16_t>(character_banks) << 3;

    if (flag6 & 0x04) {
        metadata.trainer.reset(new uint8_t[0x200]);
        stream.read(reinterpret_cast<char*>(metadata.trainer.get()), 0x200);
    }

    if (metadata.size_prg > 0) {
        size_t memory_size = static_cast<size_t>(metadata.size_prg) << 10;
        metadata.memory_prg.reset(new uint8_t[memory_size]);
        stream.read(reinterpret_cast<char*>(metadata.memory_prg.get()), memory_size);
    }

    if (metadata.size_chr > 0) {
        size_t memory_size = static_cast<size_t>(metadata.size_chr) << 10;
        metadata.memory_chr.reset(new uint8_t[memory_size]);
        stream.read(reinterpret_cast<char*>(metadata.memory_chr.get()), memory_size);
    } else {
        metadata.size_chr = 8;
        metadata.memory_chr.reset(new uint8_t[0x2000]);
    }

    stream.close();

    uint8_t mapper_index = (flag7 & 0xF0) | flag6 >> 4;

    cynes::MirroringMode mode = (flag6 & 0x01) == 1
        ? cynes::MirroringMode::VERTICAL
        : cynes::MirroringMode::HORIZONTAL;

    switch (mapper_index) {
    case   0: return std::make_unique<cynes::NROM> (nes, metadata, mode);
    case   1: return std::make_unique<cynes::MMC1> (nes, metadata, mode);
    case   2: return std::make_unique<cynes::UxROM>(nes, metadata, mode);
    case   3: return std::make_unique<cynes::CNROM>(nes, metadata, mode);
    case   4: return std::make_unique<cynes::MMC3> (nes, metadata, mode);
    case   7: return std::make_unique<cynes::AxROM>(nes, metadata);
    case   9: return std::make_unique<cynes::MMC2> (nes, metadata, mode);
    case  10: return std::make_unique<cynes::MMC4> (nes, metadata, mode);
    case  66: return std::make_unique<cynes::GxROM>(nes, metadata, mode);
    case  71: return std::make_unique<cynes::UxROM>(nes, metadata, mode);
    default: throw std::runtime_error("The ROM Mapper is not supported.");
    }
}

void cynes::Mapper::tick() { }

void cynes::Mapper::write_cpu(uint16_t address, uint8_t value) {
    const auto& bank = _banks_cpu[address >> 10];

    if (!bank.read_only && bank.mapped) {
        _memory[bank.offset + (address & 0x3FF)] = value;
    }
}

void cynes::Mapper::write_ppu(uint16_t address, uint8_t value) {
    const auto& bank = _banks_ppu[address >> 10];

    if (!bank.read_only && bank.mapped) {
        _memory[bank.offset + (address & 0x3FF)] = value;
    }
}

uint8_t cynes::Mapper::read_cpu(uint16_t address) {
    const auto& bank = _banks_cpu[address >> 10];

    if (!bank.mapped) {
        return _nes.get_open_bus();
    }

    return _memory[bank.offset + (address & 0x3FF)];
}

uint8_t cynes::Mapper::read_ppu(uint16_t address) {
    const auto& bank = _banks_ppu[address >> 10];

    if (!bank.mapped) {
        return 0x00;
    }

    return _memory[bank.offset + (address & 0x3FF)];
}

void cynes::Mapper::map_bank_prg(uint8_t page, uint16_t address) {
    _banks_cpu[page] = {
        static_cast<size_t>(address << 10),
        true
    };
}

void cynes::Mapper::map_bank_prg(uint8_t page, uint8_t size, uint16_t address) {
    for (uint8_t index = 0; index < size; index++) {
        map_bank_prg(page + index, address + index);
    }
}

void cynes::Mapper::map_bank_cpu_ram(uint8_t page, uint16_t address, bool read_only) {
    _banks_cpu[page] = {
        _size_prg + _size_chr + static_cast<size_t>(address << 10),
        read_only
    };
}

void cynes::Mapper::map_bank_cpu_ram(uint8_t page, uint8_t size, uint16_t address, bool read_only) {
    for (uint8_t index = 0; index < size; index++) {
        map_bank_cpu_ram(page + index, address + index, read_only);
    }
}

void cynes::Mapper::map_bank_chr(uint8_t page, uint16_t address) {
    _banks_ppu[page] = {
        _size_prg + static_cast<size_t>(address << 10),
        true
    };
}

void cynes::Mapper::map_bank_chr(uint8_t page, uint8_t size, uint16_t address) {
    for (uint8_t index = 0; index < size; index++) {
        map_bank_chr(page + index, address + index);
    }
}

void cynes::Mapper::map_bank_ppu_ram(uint8_t page, uint16_t address, bool read_only) {
    _banks_ppu[page] = {
        _size_prg + _size_chr + _size_cpu_ram + static_cast<size_t>(address << 10),
        read_only
    };
}

void cynes::Mapper::map_bank_ppu_ram(uint8_t page, uint8_t size, uint16_t address, bool read_only) {
    for (uint8_t index = 0; index < size; index++) {
        map_bank_ppu_ram(page + index, address + index, read_only);
    }
}

void cynes::Mapper::unmap_bank_cpu(uint8_t page) {
    _banks_cpu[page] = {};
}

void cynes::Mapper::unmap_bank_cpu(uint8_t page, uint8_t size) {
    for (uint8_t index = 0; index < size; index++) {
        unmap_bank_cpu(page + index);
    }
}

void cynes::Mapper::set_mirroring_mode(MirroringMode mode) {
    if (mode == MirroringMode::ONE_SCREEN_LOW) {
        map_bank_ppu_ram(0x8, 0x00, false);
        map_bank_ppu_ram(0x9, 0x00, false);
        map_bank_ppu_ram(0xA, 0x00, false);
        map_bank_ppu_ram(0xB, 0x00, false);
    } else if (mode == MirroringMode::ONE_SCREEN_HIGH) {
        map_bank_ppu_ram(0x8, 0x01, false);
        map_bank_ppu_ram(0x9, 0x01, false);
        map_bank_ppu_ram(0xA, 0x01, false);
        map_bank_ppu_ram(0xB, 0x01, false);
    } else if (mode == MirroringMode::VERTICAL) {
        map_bank_ppu_ram(0x8, 0x2, 0x00, false);
        map_bank_ppu_ram(0xA, 0x2, 0x00, false);
    } else if (mode == MirroringMode::HORIZONTAL) {
        map_bank_ppu_ram(0x8, 0x00, false);
        map_bank_ppu_ram(0x9, 0x00, false);
        map_bank_ppu_ram(0xA, 0x01, false);
        map_bank_ppu_ram(0xB, 0x01, false);
    }

    mirror_ppu_banks(0x8, 0x4, 0xC);
}

void cynes::Mapper::mirror_cpu_banks(uint8_t page, uint8_t size, uint8_t mirror) {
    for (uint8_t index = 0; index < size; index++) {
        _banks_cpu[mirror + index] = _banks_cpu[page + index];
    }
}

void cynes::Mapper::mirror_ppu_banks(uint8_t page, uint8_t size, uint8_t mirror) {
    for (uint8_t index = 0; index < size; index++) {
        _banks_ppu[mirror + index] = _banks_ppu[page + index];
    }
}


cynes::NROM::NROM(NES& nes, const NESMetadata& metadata, MirroringMode mode)
    : Mapper(nes, metadata, mode)
{
    map_bank_chr(0x0, 0x8, 0x0);

    if (_banks_prg == 0x20) {
        map_bank_prg(0x20, 0x20, 0x0);
    } else {
        map_bank_prg(0x20, 0x10, 0x0);
        map_bank_prg(0x30, 0x10, 0x0);
    }

    map_bank_cpu_ram(0x18, 0x8, 0x0, false);
}


cynes::MMC1::MMC1(
    NES& nes,
    const NESMetadata& metadata,
    MirroringMode mode
) : Mapper(nes, metadata, mode)
  , _tick{0x00}
  , _registers{}
  , _register{0x00}
  , _counter{0x00}
{
    memset(_registers, 0x00, 0x4);
    _registers[0x0] = 0xC;

    update_banks();
}

void cynes::MMC1::tick() {
    if (_tick < 6) {
        _tick++;
    }
}

void cynes::MMC1::write_cpu(uint16_t address, uint8_t value) {
    if (address < 0x8000) {
        cynes::Mapper::write_cpu(address, value);
    } else {
        write_registers((address >> 13) & 0x03, value);
    }
}

void cynes::MMC1::write_registers(uint8_t register_target, uint8_t value) {
    if (_tick == 6) {
        if (value & 0x80) {
            _registers[0x0] |= 0xC;

            update_banks();

            _register = 0x00;
            _counter = 0;
        } else {
            _register >>= 1;
            _register |= (value & 0x1) << 4;

            if (++_counter == 5) {
                _registers[register_target] = _register;

                update_banks();

                _register = 0x00;
                _counter = 0x00;
            }
        }
    }

    _tick = 0;
}

void cynes::MMC1::update_banks() {
    switch (_registers[0x0] & 0x03) {
    case 0: set_mirroring_mode(MirroringMode::ONE_SCREEN_LOW); break;
    case 1: set_mirroring_mode(MirroringMode::ONE_SCREEN_HIGH); break;
    case 2: set_mirroring_mode(MirroringMode::VERTICAL); break;
    case 3: set_mirroring_mode(MirroringMode::HORIZONTAL); break;
    }

    if (_registers[0x0] & 0x10) {
        map_bank_chr(0x0, 0x4, (_registers[0x1] & 0x1F) << 2);
        map_bank_chr(0x4, 0x4, (_registers[0x2] & 0x1F) << 2);
    } else {
        map_bank_chr(0x0, 0x8, (_registers[0x1] & 0x1E) << 2);
    }

    if (_registers[0x0] & 0x08) {
        if (_registers[0x0] & 0x04) {
            map_bank_prg(0x20, 0x10, (_registers[0x3] & 0x0F) << 4);
            map_bank_prg(0x30, 0x10, _banks_prg - 0x10);
        } else {
            map_bank_prg(0x20, 0x10, 0x0);
            map_bank_prg(0x30, 0x10, (_registers[0x3] & 0xF) << 4);
        }
    } else {
        map_bank_prg(0x20, 0x20, (_registers[0x3] & 0x0E) << 4);
    }

    bool read_only = _registers[0x3] & 0x10;
    map_bank_cpu_ram(0x18, 0x8, 0x0, read_only);
}


cynes::UxROM::UxROM(NES& nes, const NESMetadata& metadata, MirroringMode mode)
    : Mapper(nes, metadata, mode, 0x0, 0x10)
{
    map_bank_prg(0x20, 0x10, 0x00);
    map_bank_prg(0x30, 0x10, _banks_prg - 0x10);

    map_bank_ppu_ram(0x0, 0x8, 0x02, false);
}

void cynes::UxROM::write_cpu(uint16_t address, uint8_t value) {
    if (address < 0x8000) {
        cynes::Mapper::write_cpu(address, value);
    } else {
        map_bank_prg(0x20, 0x10, value << 4);
    }
}


cynes::CNROM::CNROM(NES& nes, const NESMetadata& metadata, MirroringMode mode)
    : Mapper(nes, metadata, mode, 0x0)
{
    map_bank_chr(0x0, 0x8, 0x0);

    if (_banks_prg == 0x20) {
        map_bank_prg(0x20, 0x20, 0x0);
    } else {
        map_bank_prg(0x20, 0x10, 0x0);
        map_bank_prg(0x30, 0x10, 0x0);
    }
}

void cynes::CNROM::write_cpu(uint16_t address, uint8_t value) {
    if (address < 0x8000) {
        cynes::Mapper::write_cpu(address, value);
    } else {
        map_bank_chr(0x0, 0x8, (value & 0x3) << 3);
    }
}


cynes::MMC3::MMC3(
    NES& nes,
    const NESMetadata& metadata,
    MirroringMode mode
) : Mapper(nes, metadata, mode)
  , _tick{0x0000}
  , _registers{}
  , _counter{0x0000}
  , _counter_reset_value{0x0000}
  , _register_target{0x00}
  , _mode_prg{false}
  , _mode_chr{false}
  , _enable_interrupt{false}
  , _should_reload_interrupt{false}
{
    map_bank_chr(0x0, 0x8, 0x0);
    map_bank_prg(0x20, 0x10, 0x0);
    map_bank_prg(0x30, 0x10, _banks_prg - 0x10);
    map_bank_cpu_ram(0x18, 0x8, 0x0, false);

    memset(_registers, 0x0000, 0x20);
}

void cynes::MMC3::tick() {
    if (_tick > 0 && _tick < 11) {
        _tick++;
    }
}

void cynes::MMC3::write_cpu(uint16_t address, uint8_t value) {
    if (address < 0x8000) {
        cynes::Mapper::write_cpu(address, value);
    } else if (address < 0xA000) {
        if (address & 0x1) {
            if (_register_target < 2) {
                value &= 0xFE;
            }

            _registers[_register_target] = value;

            if (_mode_prg) {
                map_bank_prg(0x20, 0x08, _banks_prg - 0x10);
                map_bank_prg(0x28, 0x08, (_registers[0x7] & 0x3F) << 3);
                map_bank_prg(0x30, 0x08, (_registers[0x6] & 0x3F) << 3);
                map_bank_prg(0x38, 0x08, _banks_prg - 0x8);
            } else {
                map_bank_prg(0x20, 0x08, (_registers[0x6] & 0x3F) << 3);
                map_bank_prg(0x28, 0x08, (_registers[0x7] & 0x3F) << 3);
                map_bank_prg(0x30, 0x10, _banks_prg - 0x10);
            }

            if (_mode_chr) {
                map_bank_chr(0x0, _registers[0x2]);
                map_bank_chr(0x1, _registers[0x3]);
                map_bank_chr(0x2, _registers[0x4]);
                map_bank_chr(0x3, _registers[0x5]);
                map_bank_chr(0x4, 0x2, _registers[0x0]);
                map_bank_chr(0x6, 0x2, _registers[0x1]);
            } else {
                map_bank_chr(0x0, 0x2, _registers[0x0]);
                map_bank_chr(0x2, 0x2, _registers[0x1]);
                map_bank_chr(0x4, _registers[0x2]);
                map_bank_chr(0x5, _registers[0x3]);
                map_bank_chr(0x6, _registers[0x4]);
                map_bank_chr(0x7, _registers[0x5]);
            }
        } else {
            _register_target = value & 0x07;
            _mode_prg = value & 0x40;
            _mode_chr = value & 0x80;
        }
    } else if (address < 0xC000) {
        if (address & 0x1) {
            bool read_only = value & 0x40;
            map_bank_cpu_ram(0x18, 0x8, 0x0, read_only);
        } else  if (value & 0x1) {
            set_mirroring_mode(MirroringMode::HORIZONTAL);
        } else {
            set_mirroring_mode(MirroringMode::VERTICAL);
        }
    } else if (address < 0xE000) {
        if (address & 0x1) {
            _counter = 0x0000;
            _should_reload_interrupt = true;
        } else {
            _counter_reset_value = value;
        }
    } else {
        if (address & 0x1) {
            _enable_interrupt = true;
        } else {
            _enable_interrupt = false;
            _nes.cpu.set_mapper_interrupt(false);
        }
    }
}

void cynes::MMC3::write_ppu(uint16_t address, uint8_t value) {
    update_state(address & 0x1000);
    cynes::Mapper::write_ppu(address, value);
}

uint8_t cynes::MMC3::read_ppu(uint16_t address) {
    update_state(address & 0x1000);
    return cynes::Mapper::read_ppu(address);
}

void cynes::MMC3::update_state(bool state) {
    if (state) {
        if (_tick > 10) {
            if (_counter == 0 || _should_reload_interrupt) {
                _counter = _counter_reset_value;
            } else {
                _counter--;
            }

            if (_counter == 0 && _enable_interrupt) {
                _nes.cpu.set_mapper_interrupt(true);
            }

            _should_reload_interrupt = false;
        }

        _tick = 0;
    } else if (_tick == 0) {
        _tick = 1;
    }
}


cynes::AxROM::AxROM(NES& nes, const NESMetadata& metadata)
    : Mapper(nes, metadata, MirroringMode::ONE_SCREEN_LOW, 0x8, 0x10)
{
    map_bank_ppu_ram(0x0, 0x8, 0x2, false);
    map_bank_prg(0x20, 0x20, 0x0);
}

void cynes::AxROM::write_cpu(uint16_t address, uint8_t value) {
    if (address < 0x8000) {
        cynes::Mapper::write_cpu(address, value);
    } else {
        map_bank_prg(0x20, 0x20, (value & 0x07) << 5);

        if (value & 0x10) {
            set_mirroring_mode(MirroringMode::ONE_SCREEN_HIGH);
        } else {
            set_mirroring_mode(MirroringMode::ONE_SCREEN_LOW);
        }
    }
}


cynes::GxROM::GxROM(NES& nes, const NESMetadata& metadata, MirroringMode mode)
    : Mapper(nes, metadata, mode, 0x0)
{
    map_bank_prg(0x20, 0x20, 0x0);
    map_bank_chr(0x00, 0x08, 0x0);
}

void cynes::GxROM::write_cpu(uint16_t address, uint8_t value) {
    if (address < 0x8000) {
        cynes::Mapper::write_cpu(address, value);
    } else {
        map_bank_prg(0x20, 0x20, (value & 0x30) << 1);
        map_bank_chr(0x00, 0x08, (value & 0x03) << 3);
    }
}
