#include "nes.hpp"

#include "apu.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "mapper.hpp"


constexpr uint8_t PALETTE_RAM_BOOT_VALUES[0x20] = {
    0x09, 0x01, 0x00, 0x01, 0x00, 0x02, 0x02, 0x0D,
    0x08, 0x10, 0x08, 0x24, 0x00, 0x00, 0x04, 0x2C,
    0x09, 0x01, 0x34, 0x03, 0x00, 0x04, 0x00, 0x14,
    0x08, 0x3A, 0x00, 0x02, 0x00, 0x20, 0x2C, 0x08
};


cynes::NES::NES(const char* path)
    : cpu{*this}
    , ppu{*this}
    , apu{*this}
    , _mapper{Mapper::load_mapper(static_cast<NES&>(*this), path)}
    , _memory_cpu{new uint8_t[0x800]}
    , _memory_oam{new uint8_t[0x100]}
    , _memory_palette{new uint8_t[0x20]}
{
    cpu.power();
    ppu.power();
    apu.power();

    std::memcpy(_memory_palette.get(), PALETTE_RAM_BOOT_VALUES, 0x20);
    std::memset(_memory_cpu.get(), 0x00, 0x800);
    std::memset(_memory_oam.get(), 0x00, 0x100);
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
    _open_bus = value;

    if (address < 0x2000) {
        _memory_cpu[address & 0x7FF] = value;
    } else if (address < 0x4000) {
        ppu.write(address & 0x7, value);
    } else if (address == 0x4016) {
        load_controller_shifter(~value & 0x01);
    } else if (address < 0x4018) {
        apu.write(address & 0xFF, value);
    } else {
        _mapper->write_cpu(address, value);
    }
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

    cynes::dump<operation>(buffer, _memory_cpu.get(), 0x800);
    cynes::dump<operation>(buffer, _memory_oam.get(), 0x100);
    cynes::dump<operation>(buffer, _memory_palette.get(), 0x20);

    cynes::dump<operation>(buffer, _controller_status);
    cynes::dump<operation>(buffer, _controller_shifters);
}

template void cynes::NES::dump<cynes::DumpOperation::SIZE>(unsigned int&);
template void cynes::NES::dump<cynes::DumpOperation::DUMP>(uint8_t*&);
template void cynes::NES::dump<cynes::DumpOperation::LOAD>(uint8_t*&);
