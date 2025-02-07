#include "nes.hpp"

#include "apu.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "mapper.hpp"

#include <fstream>
#include <memory>
#include <stdexcept>


// TODO: maybe move elsewhere?
std::unique_ptr<cynes::Mapper> load_mapper(cynes::NES& nes, const char* path) {
    std::ifstream stream{path, std::ios::binary};

    if (!stream.is_open()) {
        throw std::runtime_error("The file cannot be read.");
    }

    uint32_t header;
    stream.read(reinterpret_cast<char*>(&header), 4);

    if (header != 0x1A53454E) {
        throw std::runtime_error("The specified file is not a NES ROM.");
    }

    uint8_t program_banks = stream.get();
    uint8_t character_banks = stream.get();
    uint8_t flag6 = stream.get();
    uint8_t flag7 = stream.get();

    stream.seekg(8, std::ios::cur);

    cynes::NESMetadata metadata;

    metadata.size_prg = program_banks << 4;
    metadata.size_chr = character_banks << 3;

    if (flag6 & 0x04) {
        metadata.trainer = new uint8_t[0x200];
        stream.read(reinterpret_cast<char*>(metadata.trainer), 0x200);
    }

    if (metadata.size_prg > 0) {
        size_t memory_size = static_cast<size_t>(metadata.size_prg) << 10;
        metadata.memory_prg = new uint8_t[memory_size]{ 0 };
        stream.read(reinterpret_cast<char*>(metadata.memory_prg), memory_size);
    }

    if (metadata.size_chr > 0) {
        size_t memory_size = static_cast<size_t>(metadata.size_chr) << 10;
        metadata.memory_chr = new uint8_t[memory_size]{ 0 };
        stream.read(reinterpret_cast<char*>(metadata.memory_chr), memory_size);
    }

    if (metadata.size_chr == 0) {
        metadata.size_chr = 8;
        metadata.memory_chr = new uint8_t[0x2000]{ 0 };
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


cynes::NES::NES(const char* path)
    : cpu{*this}
    , ppu{*this}
    , apu{*this}
    , _mapper{load_mapper(static_cast<NES&>(*this), path)}
{
    cpu.power();
    ppu.power();
    apu.power();

    // TODO: move elsewhere
    uint8_t palette_ram_boot_values[0x20] = {
        0x09, 0x01, 0x00, 0x01, 0x00, 0x02, 0x02, 0x0D, 0x08, 0x10, 0x08, 0x24, 0x00, 0x00, 0x04, 0x2C,
        0x09, 0x01, 0x34, 0x03, 0x00, 0x04, 0x00, 0x14, 0x08, 0x3A, 0x00, 0x02, 0x00, 0x20, 0x2C, 0x08
    };

    std::memcpy(_memory_palette, palette_ram_boot_values, 0x20);

    std::memset(_memory_cpu, 0x00, 0x800);
    std::memset(_memory_oam, 0x00, 0x100);
    std::memset(_controller_status, 0x00, 0x2);
    std::memset(_controller_shifters, 0x00, 0x2);

    for (int i = 0; i < 8; i++) {
        dummy_read();
    }
}

void cynes::NES::reset() {
    cpu.reset();
    ppu.reset();
    apu.reset();

    for (int i = 0; i < 8; i++) {
        dummy_read();
    }
}

void cynes::NES::dummy_read() {
    apu.tick(true);
    ppu.tick();
    ppu.tick();
    ppu.tick();
    cpu.poll();
}

void cynes::NES::write(uint16_t address, uint8_t value) {
    apu.tick(false);
    ppu.tick();
    ppu.tick();

    write_cpu(address, value);

    ppu.tick();
    cpu.poll();
}

void cynes::NES::write_cpu(uint16_t address, uint8_t value) {
    if (address < 0x2000) {
        _memory_cpu[address & 0x7FF] = value;
    } else if (address < 0x4000) {
        ppu.write(address & 0x7, value);
    } else if (address == 0x4016) {
        load_controller_shifter(~value & 0x01);
    } else if (address < 0x4018) {
        apu.write(address & 0xFF, value);
    }

    _mapper->write_cpu(address, value);
}

void cynes::NES::write_ppu(uint16_t address, uint8_t value) {
    address &= 0x3FFF;

    if (address < 0x3F00) {
        _mapper->write_ppu(address, value);
    } else {
        address &= 0x1F;

        if (address == 0x10) {
            address = 0x00;
        } else if (address == 0x14) {
            address = 0x04;
        } else if (address == 0x18) {
            address = 0x08;
        } else if (address == 0x1C) {
            address = 0x0C;
        }

        _memory_palette[address] = value & 0x3F;
    }
}

void cynes::NES::write_oam(uint8_t address, uint8_t value) {
    _memory_oam[address] = value;
}

uint8_t cynes::NES::read(uint16_t address) {
    apu.tick(true);
    ppu.tick();
    ppu.tick();

    _open_bus = read_cpu(address);

    ppu.tick();
    cpu.poll();

    return _open_bus;
}

uint8_t cynes::NES::read_cpu(uint16_t address) {
    if (address < 0x2000) {
        return _memory_cpu[address & 0x7FF];
    } else if (address < 0x4000) {
        return ppu.read(address & 0x7);
    } else if (address == 0x4016) {
        return poll_controller(0x0);
    } else if (address == 0x4017) {
        return poll_controller(0x1);
    } else if (address < 0x4018) {
        return apu.read(address & 0xFF);
    } else {
        return _mapper->read_cpu(address);
    }
}

uint8_t cynes::NES::read_ppu(uint16_t address) {
    address &= 0x3FFF;

    if (address < 0x3F00) {
        return _mapper->read_ppu(address);
    } else {
        address &= 0x1F;

        if (address == 0x10) {
            address = 0x00;
        } else if (address == 0x14) {
            address = 0x04;
        } else if (address == 0x18) {
            address = 0x08;
        } else if (address == 0x1C) {
            address = 0x0C;
        }

        return _memory_palette[address];
    }
}

uint8_t cynes::NES::read_oam(uint8_t address) const {
    return _memory_oam[address];
}

uint8_t cynes::NES::get_open_bus() const {
    return _open_bus;
}

bool cynes::NES::step(uint16_t controllers, unsigned int frames) {
    _controller_status[0x0] = controllers & 0xFF;
    _controller_status[0x1] = controllers >> 8;

    for (unsigned int k = 0; k < frames; k++) {
        while (!ppu.is_frame_ready()) {
            cpu.tick();

            if (cpu.is_frozen()) {
                return true;
            }
        }
    }

    return false;
}

unsigned int cynes::NES::size() {
    unsigned int buffer_size = 0;
    dump<DumpOperation::SIZE>(buffer_size);

    return buffer_size;
}

void cynes::NES::save(uint8_t* buffer) {
    dump<DumpOperation::DUMP>(buffer);
}

void cynes::NES::load(uint8_t* buffer) {
    dump<DumpOperation::LOAD>(buffer);
}

cynes::Mapper& cynes::NES::get_mapper() {
    return static_cast<Mapper&>(*_mapper.get());
}

void cynes::NES::load_controller_shifter(bool polling) {
    if (polling) {
        memcpy(_controller_shifters, _controller_status, 0x2);
    }
}

uint8_t cynes::NES::poll_controller(uint8_t player) {
    uint8_t value = _controller_shifters[player] >> 7;

    _controller_shifters[player] <<= 1;

    return (_open_bus & 0xE0) | value;
}

template<cynes::DumpOperation operation, typename T>
void cynes::NES::dump(T& buffer) {
    cpu.dump<operation>(buffer);
    ppu.dump<operation>(buffer);
    apu.dump<operation>(buffer);

    _mapper->dump<operation>(buffer);

    cynes::dump<operation>(buffer, _memory_cpu);
    cynes::dump<operation>(buffer, _memory_oam);
    cynes::dump<operation>(buffer, _memory_palette);

    cynes::dump<operation>(buffer, _controller_status);
    cynes::dump<operation>(buffer, _controller_shifters);
}

template void cynes::NES::dump<cynes::DumpOperation::SIZE>(unsigned int&);
template void cynes::NES::dump<cynes::DumpOperation::DUMP>(uint8_t*&);
template void cynes::NES::dump<cynes::DumpOperation::LOAD>(uint8_t*&);
