#include "nes.hpp"

#include "apu.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "mapper.hpp"

#include <stdexcept>


cynes::NES::NES(const char* path) : _cpu{*this}, _ppu{*this}, _apu{*this} {
    loadMapper(path);

    _cpu.power();
    _ppu.power();
    _apu.power();

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

cynes::NES::~NES() {
    delete _mapper;
}

cynes::CPU& cynes::NES::getCPU() {
    return _cpu;
}

const cynes::CPU& cynes::NES::getCPU() const {
    return _cpu;
}

cynes::PPU& cynes::NES::getPPU() {
    return _ppu;
}

const cynes::PPU& cynes::NES::getPPU() const {
    return _ppu;
}

cynes::APU& cynes::NES::getAPU() {
    return _apu;
}

const cynes::APU& cynes::NES::getAPU() const {
    return _apu;
}

cynes::Mapper* cynes::NES::getMapper() {
    return _mapper;
}

void cynes::NES::reset() {
    _cpu.reset();
    _ppu.reset();
    _apu.reset();

    for (int i = 0; i < 8; i++) {
        dummyRead();
    }
}

void cynes::NES::dummyRead() {
    _apu.tick(true);
    _ppu.tick();
    _ppu.tick();
    _ppu.tick();
    _cpu.poll();
}

void cynes::NES::write(uint16_t address, uint8_t value) {
    _apu.tick(false);
    _ppu.tick();
    _ppu.tick();

    writeCPU(address, value);

    _ppu.tick();
    _cpu.poll();
}

void cynes::NES::writeCPU(uint16_t address, uint8_t value) {
    if (address < 0x2000) {
        _memoryCPU[address & 0x7FF] = value;
    } else if (address < 0x4000) {
        _ppu.write(address & 0x7, value);
    } else if (address == 0x4016) {
        loadControllerShifter(~value & 0x01);
    } else if (address < 0x4018) {
        _apu.write(address & 0xFF, value);
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
    _apu.tick(true);
    _ppu.tick();
    _ppu.tick();

    _openBus = readCPU(address);

    _ppu.tick();
    _cpu.poll();

    return _openBus;
}

uint8_t cynes::NES::readCPU(uint16_t address) {
    if (address < 0x2000) {
        return _memoryCPU[address & 0x7FF];
    } else if (address < 0x4000) {
        return _ppu.read(address & 0x7);
    } else if (address == 0x4016) {
        return pollController(0x0);
    } else if (address == 0x4017) {
        return pollController(0x1);
    } else if (address < 0x4018) {
        return _apu.read(address & 0xFF);
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

void cynes::NES::loadMapper(const char* path) {
    FILE* stream = fopen(path, "rb");

    if (!stream) {
        throw std::runtime_error("The file cannot be read.");
    }

    uint32_t header = getc(stream) << 24 | getc(stream) << 16 | getc(stream) << 8 | getc(stream);

    if (header != 0x4E45531A) {
        throw std::runtime_error("The specified file is not a NES ROM.");
    }

    uint8_t programBanks = getc(stream);
    uint8_t characterBanks = getc(stream);

    uint8_t flag6 = getc(stream);
    uint8_t flag7 = getc(stream);

    for (int k = 0; k < 0x8; k++) {
        (void)getc(stream);
    }

    NESMetadata metadata;

    metadata.sizePRG = programBanks << 4;
    metadata.sizeCHR = characterBanks << 3;

    if (flag6 & 0x04) {
        metadata.trainer = new uint8_t[0x200];

        for (int k = 0x00; k < 0x200; k++) {
            metadata.trainer[k] = getc(stream);
        }
    }

    if (metadata.sizePRG > 0) {
        metadata.memoryPRG = new uint8_t[uint64_t(metadata.sizePRG) << 10]{ 0 };

        for (int k = 0x00; k < metadata.sizePRG << 10; k++) {
            metadata.memoryPRG[k] = getc(stream);
        }
    }

    if (metadata.sizeCHR > 0) {
        metadata.memoryCHR = new uint8_t[uint64_t(metadata.sizeCHR) << 10]{ 0 };

        for (int k = 0x00; k < metadata.sizeCHR << 10; k++) {
            metadata.memoryCHR[k] = getc(stream);
        }
    }

    if (metadata.sizeCHR == 0) {
        metadata.sizeCHR = 8;

        metadata.memoryCHR = new uint8_t[0x2000]{ 0 };
    }

    fclose(stream);

    uint8_t mapperId = (flag7 & 0xF0) | flag6 >> 4;

    MirroringMode mode = (flag6 & 0x01) == 1 ? MirroringMode::VERTICAL : MirroringMode::HORIZONTAL;

    switch (mapperId) {
    case   0: _mapper = new NROM (*this, metadata, mode); return;
    case   1: _mapper = new MMC1 (*this, metadata, mode); return;
    case   2: _mapper = new UxROM(*this, metadata, mode); return;
    case   3: _mapper = new CNROM(*this, metadata, mode); return;
    case   4: _mapper = new MMC3 (*this, metadata, mode); return;
    case   7: _mapper = new AxROM(*this, metadata); return;
    case   9: _mapper = new MMC2 (*this, metadata, mode); return;
    case  10: _mapper = new MMC4 (*this, metadata, mode); return;
    case  66: _mapper = new GxROM(*this, metadata, mode); return;
    case  71: _mapper = new UxROM(*this, metadata, mode); return;
    default: throw std::runtime_error("The ROM Mapper is not supported.");
    }
}

bool cynes::NES::step(uint8_t* buffer, uint16_t controllers, unsigned int frames) {
    _controllerStates[0x0] = controllers & 0xFF;
    _controllerStates[0x1] = controllers >> 8;

    for (unsigned int k = 0; k < frames; k++) {
        while (!_ppu.isFrameReady()) {
            _cpu.tick();

            if (_cpu.isFrozen()) {
                return true;
            }
        }
    }

    memcpy(buffer, _ppu.getFrameBuffer(), 0x2D000);

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
    _cpu.dump<operation>(buffer);
    _ppu.dump<operation>(buffer);
    _apu.dump<operation>(buffer);

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
