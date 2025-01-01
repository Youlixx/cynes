#include "nes.hpp"

#include "apu.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "mapper.hpp"

#include <fstream>
#include <memory>
#include <stdexcept>


std::unique_ptr<cynes::Mapper> loadMapper(cynes::NES& nes, const char* path) {
    std::ifstream stream{path, std::ios::binary};

    if (!stream.is_open()) {
        throw std::runtime_error("The file cannot be read.");
    }

    uint32_t header;
    stream.read(reinterpret_cast<char*>(&header), 4);

    if (header != 0x1A53454E) {
        throw std::runtime_error("The specified file is not a NES ROM.");
    }

    uint8_t programBanks = stream.get();
    uint8_t characterBanks = stream.get();
    uint8_t flag6 = stream.get();
    uint8_t flag7 = stream.get();

    stream.seekg(8, std::ios::cur);

    cynes::NESMetadata metadata;

    metadata.sizePRG = programBanks << 4;
    metadata.sizeCHR = characterBanks << 3;

    if (flag6 & 0x04) {
        metadata.trainer = new uint8_t[0x200];
        stream.read(reinterpret_cast<char*>(metadata.trainer), 0x200);
    }

    if (metadata.sizePRG > 0) {
        size_t memoryPRGSize = static_cast<size_t>(metadata.sizePRG) << 10;
        metadata.memoryPRG = new uint8_t[memoryPRGSize]{ 0 };
        stream.read(reinterpret_cast<char*>(metadata.memoryPRG), memoryPRGSize);
    }

    if (metadata.sizeCHR > 0) {
        size_t memoryCHRSize = static_cast<size_t>(metadata.sizeCHR) << 10;
        metadata.memoryCHR = new uint8_t[memoryCHRSize]{ 0 };
        stream.read(reinterpret_cast<char*>(metadata.memoryCHR), memoryCHRSize);
    }

    if (metadata.sizeCHR == 0) {
        metadata.sizeCHR = 8;
        metadata.memoryCHR = new uint8_t[0x2000]{ 0 };
    }

    stream.close();

    uint8_t mapperId = (flag7 & 0xF0) | flag6 >> 4;

    cynes::MirroringMode mode = (flag6 & 0x01) == 1
        ? cynes::MirroringMode::VERTICAL
        : cynes::MirroringMode::HORIZONTAL;

    switch (mapperId) {
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
, _mapper{loadMapper(static_cast<NES&>(*this), path)}
{
    cpu.power();
    ppu.power();
    apu.power();

    uint8_t paletteRamBootValues[0x20] = {
        0x09, 0x01, 0x00, 0x01, 0x00, 0x02, 0x02, 0x0D, 0x08, 0x10, 0x08, 0x24, 0x00, 0x00, 0x04, 0x2C,
        0x09, 0x01, 0x34, 0x03, 0x00, 0x04, 0x00, 0x14, 0x08, 0x3A, 0x00, 0x02, 0x00, 0x20, 0x2C, 0x08
    };

    memcpy(_memoryPalette, paletteRamBootValues, 0x20);

    memset(_memoryCPU, 0x00, 0x800);
    memset(_memoryOAM, 0x00, 0x100);

    memset(_controllerStates, 0x00, 0x2);
    memset(_controllerShifters, 0x00, 0x2);

    for (int i = 0; i < 8; i++) {
        dummyRead();
    }
}

void cynes::NES::reset() {
    cpu.reset();
    ppu.reset();
    apu.reset();

    for (int i = 0; i < 8; i++) {
        dummyRead();
    }
}

void cynes::NES::dummyRead() {
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

    writeCPU(address, value);

    ppu.tick();
    cpu.poll();
}

void cynes::NES::writeCPU(uint16_t address, uint8_t value) {
    if (address < 0x2000) {
        _memoryCPU[address & 0x7FF] = value;
    } else if (address < 0x4000) {
        ppu.write(address & 0x7, value);
    } else if (address == 0x4016) {
        loadControllerShifter(~value & 0x01);
    } else if (address < 0x4018) {
        apu.write(address & 0xFF, value);
    }

    _mapper->writeCPU(address, value);
}

void cynes::NES::writePPU(uint16_t address, uint8_t value) {
    address &= 0x3FFF;

    if (address < 0x3F00) {
        _mapper->writePPU(address, value);
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

        _memoryPalette[address] = value & 0x3F;
    }
}

void cynes::NES::writeOAM(uint8_t address, uint8_t value) {
    _memoryOAM[address] = value;
}

uint8_t cynes::NES::read(uint16_t address) {
    apu.tick(true);
    ppu.tick();
    ppu.tick();

    _openBus = readCPU(address);

    ppu.tick();
    cpu.poll();

    return _openBus;
}

uint8_t cynes::NES::readCPU(uint16_t address) {
    if (address < 0x2000) {
        return _memoryCPU[address & 0x7FF];
    } else if (address < 0x4000) {
        return ppu.read(address & 0x7);
    } else if (address == 0x4016) {
        return pollController(0x0);
    } else if (address == 0x4017) {
        return pollController(0x1);
    } else if (address < 0x4018) {
        return apu.read(address & 0xFF);
    } else {
        return _mapper->readCPU(address);
    }
}

uint8_t cynes::NES::readPPU(uint16_t address) {
    address &= 0x3FFF;

    if (address < 0x3F00) {
        return _mapper->readPPU(address);
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

        return _memoryPalette[address];
    }
}

uint8_t cynes::NES::readOAM(uint8_t address) const {
    return _memoryOAM[address];
}

uint8_t cynes::NES::getOpenBus() const {
    return _openBus;
}

bool cynes::NES::step(uint8_t* buffer, uint16_t controllers, unsigned int frames) {
    _controllerStates[0x0] = controllers & 0xFF;
    _controllerStates[0x1] = controllers >> 8;

    for (unsigned int k = 0; k < frames; k++) {
        while (!ppu.isFrameReady()) {
            cpu.tick();

            if (cpu.isFrozen()) {
                return true;
            }
        }
    }

    // TODO should return the framebuffer ptr to avoid the memcpy
    memcpy(buffer, ppu.getFrameBuffer(), 0x2D000);

    return false;
}

unsigned int cynes::NES::size() {
    unsigned int bufferSize = 0;

    dump<DumpOperation::SIZE>(bufferSize);

    return bufferSize;
}

void cynes::NES::save(uint8_t* buffer) {
    dump<DumpOperation::DUMP>(buffer);
}

void cynes::NES::load(uint8_t* buffer) {
    dump<DumpOperation::LOAD>(buffer);
}

cynes::Mapper& cynes::NES::getMapper() {
    return static_cast<Mapper&>(*_mapper.get());
}

void cynes::NES::loadControllerShifter(bool polling) {
    if (polling) {
        memcpy(_controllerShifters, _controllerStates, 0x2);
    }
}

uint8_t cynes::NES::pollController(uint8_t player) {
    uint8_t value = _controllerShifters[player] >> 7;

    _controllerShifters[player] <<= 1;

    return (_openBus & 0xE0) | value;
}

template<cynes::DumpOperation operation, typename T>
void cynes::NES::dump(T& buffer) {
    cpu.dump<operation>(buffer);
    ppu.dump<operation>(buffer);
    apu.dump<operation>(buffer);

    _mapper->dump<operation>(buffer);

    cynes::dump<operation>(buffer, _memoryCPU);
    cynes::dump<operation>(buffer, _memoryOAM);
    cynes::dump<operation>(buffer, _memoryPalette);

    cynes::dump<operation>(buffer, _controllerStates);
    cynes::dump<operation>(buffer, _controllerShifters);
}

template void cynes::NES::dump<cynes::DumpOperation::SIZE>(unsigned int&);
template void cynes::NES::dump<cynes::DumpOperation::DUMP>(uint8_t*&);
template void cynes::NES::dump<cynes::DumpOperation::LOAD>(uint8_t*&);
