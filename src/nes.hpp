#ifndef __CYNES_EMULATOR__
#define __CYNES_EMULATOR__

#include <cstdint>

#include "utils.hpp"

namespace cynes {
class CPU;
class PPU;
class APU;

class Mapper;

class NES {
public:
    NES(const char* path);
    ~NES();

public:
    CPU* getCPU();
    PPU* getPPU();
    APU* getAPU();

    Mapper* getMapper();

public:
    void reset();

    void dummyRead();

    void write(uint16_t address, uint8_t value);
    void writeCPU(uint16_t address, uint8_t value);
    void writePPU(uint16_t address, uint8_t value);
    void writeOAM(uint8_t address, uint8_t value);

    uint8_t read(uint16_t address);
    uint8_t readCPU(uint16_t address);
    uint8_t readPPU(uint16_t address);
    uint8_t readOAM(uint8_t address) const;

    uint8_t getOpenBus() const;

    bool step(uint8_t* buffer, uint16_t controllers, unsigned int frames);

    unsigned int size();

    void save(uint8_t* buffer);
    void load(uint8_t* buffer);

private:
    CPU* _cpu;
    PPU* _ppu;
    APU* _apu;

    Mapper* _mapper;

private:
    uint8_t _memoryCPU[0x800];

    uint8_t _memoryOAM[0x100];
    uint8_t _memoryPalette[0x20];

    uint8_t _openBus;

    uint8_t _controllerStates[0x2];
    uint8_t _controllerShifters[0x2];

private:
    void loadMapper(const char* rom);

    void loadControllerShifter(bool polling);

    uint8_t pollController(uint8_t player);

private:
    template<DumpOperation operation, class T> void dump(T& buffer);
};
}

#endif
