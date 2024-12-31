#ifndef __CYNES_MAPPER__
#define __CYNES_MAPPER__

#include <cstdint>
#include <cstring>

#include "utils.hpp"

namespace cynes {
class NES;

enum class MirroringMode : uint8_t {
    NONE, ONE_SCREEN_LOW, ONE_SCREEN_HIGH, HORIZONTAL, VERTICAL
};

struct NESMetadata {
public:
    uint16_t sizePRG = 0x00;
    uint16_t sizeCHR = 0x00;

    uint8_t* trainer = nullptr;
    uint8_t* memoryPRG = nullptr;
    uint8_t* memoryCHR = nullptr;
};

class Mapper {
public:
    Mapper(NES& nes, NESMetadata metadata, MirroringMode mode, uint8_t sizeWRAM = 0x8, uint8_t sizeVRAM = 0x2, uint8_t sizeERAM = 0x0);
    virtual ~Mapper();

public:
    virtual void tick();

    virtual void writeCPU(uint16_t address, uint8_t value);
    virtual void writePPU(uint16_t address, uint8_t value);

    virtual uint8_t readCPU(uint16_t address);
    virtual uint8_t readPPU(uint16_t address);

protected:
    struct MemoryBank {
    public:
        uint8_t* memory = nullptr;

        bool access = false;

        template<DumpOperation operation, class T> void dump(T& buffer) {
            cynes::dump<operation>(buffer, memory);
            cynes::dump<operation>(buffer, access);
        }
    };

protected:
    NES& _nes;

protected:
    const uint16_t SIZE_PRG;
    const uint16_t SIZE_CHR;

    const uint8_t SIZE_WRAM;
    const uint8_t SIZE_VRAM;
    const uint8_t SIZE_ERAM;

    uint8_t* _memoryPRG;
    uint8_t* _memoryCHR;

    uint8_t* _memoryWRAM;
    uint8_t* _memoryVRAM;
    uint8_t* _memoryERAM;

    MemoryBank _banksCPU[0x40];
    MemoryBank _banksPPU[0x10];

protected:
    void setBankPRG(uint8_t page, uint16_t address);
    void setBankPRG(uint8_t page, uint8_t size, uint16_t address);

    void setBankWRAM(uint8_t page, uint16_t address, bool access);
    void setBankWRAM(uint8_t page, uint8_t size, uint16_t address, bool access);

    void setBankCHR(uint8_t page, uint16_t address);
    void setBankCHR(uint8_t page, uint8_t size, uint16_t address);

    void setBankVRAM(uint8_t page, uint16_t address, bool access);
    void setBankVRAM(uint8_t page, uint8_t size, uint16_t address, bool access);

    void setBankERAMCPU(uint8_t page, uint16_t address, bool access);
    void setBankERAMCPU(uint8_t page, uint8_t size, uint16_t address, bool access);

    void setBankERAMPPU(uint8_t page, uint16_t address, bool access);
    void setBankERAMPPU(uint8_t page, uint8_t size, uint16_t address, bool access);

    void removeBankCPU(uint8_t page);
    void removeBankCPU(uint8_t page, uint8_t size);

    void setMirroringMode(MirroringMode mode);

    void mirrorBankCPU(uint8_t page, uint8_t size, uint8_t mirror);
    void mirrorBankPPU(uint8_t page, uint8_t size, uint8_t mirror);

public:
    template<DumpOperation operation, class T> void dump(T& buffer) {
        for (uint8_t k = 0x00; k < 0x40; k++) {
            _banksCPU[k].dump<operation>(buffer);
        }

        for (uint8_t k = 0x00; k < 0x10; k++) {
            _banksPPU[k].dump<operation>(buffer);
        }

        if (SIZE_WRAM) {
            cynes::dump<operation>(buffer, _memoryWRAM, SIZE_WRAM << 10);
        }

        if (SIZE_VRAM) {
            cynes::dump<operation>(buffer, _memoryVRAM, SIZE_VRAM << 10);
        }

        if (SIZE_ERAM) {
            cynes::dump<operation>(buffer, _memoryERAM, SIZE_ERAM << 10);
        }
    }
};


class NROM : public Mapper {
public:
    NROM(NES& nes, NESMetadata metadata, MirroringMode mode);
    ~NROM();
};

class MMC1 : public Mapper {
public:
    MMC1(NES& nes, NESMetadata metadata, MirroringMode mode);
    ~MMC1();

public:
    virtual void tick();

    virtual void writeCPU(uint16_t address, uint8_t value);

private:
    void writeRegister(uint8_t registerTarget, uint8_t value);
    void updateBanks();

private:
    uint8_t _tick;

    uint8_t _registers[0x4];

    uint8_t _register;
    uint8_t _counter;

public:
    template<DumpOperation operation, class T> void dump(T& buffer) {
        Mapper::dump<operation>(buffer);

        cynes::dump<operation>(buffer, _tick);
        cynes::dump<operation>(buffer, _registers);
        cynes::dump<operation>(buffer, _register);
        cynes::dump<operation>(buffer, _counter);
    }
};

class UxROM : public Mapper {
public:
    UxROM(NES& nes, NESMetadata metadata, MirroringMode mode);
    ~UxROM();

public:
    virtual void writeCPU(uint16_t address, uint8_t value);
};

class CNROM : public Mapper {
public:
    CNROM(NES& nes, NESMetadata metadata, MirroringMode mode);
    ~CNROM();

public:
    virtual void writeCPU(uint16_t address, uint8_t value);
};

class MMC3 : public Mapper {
public:
    MMC3(NES& nes, NESMetadata metadata, MirroringMode mode);
    ~MMC3();

public:
    virtual void tick();

    virtual void writeCPU(uint16_t address, uint8_t value);
    virtual void writePPU(uint16_t address, uint8_t value);

    virtual uint8_t readPPU(uint16_t address);

private:
    void updateState(bool state);

private:
    uint32_t _tick;

    uint32_t _registers[0x8];

    uint16_t _counter;
    uint16_t _counterReload;

    uint8_t _registerTarget;

    bool _modePRG;
    bool _modeCHR;

    bool _enableIRQ;
    bool _shouldReloadIRQ;

public:
    template<DumpOperation operation, class T> void dump(T& buffer) {
        Mapper::dump<operation>(buffer);

        cynes::dump<operation>(buffer, _tick);
        cynes::dump<operation>(buffer, _registers);
        cynes::dump<operation>(buffer, _counter);
        cynes::dump<operation>(buffer, _counterReload);
        cynes::dump<operation>(buffer, _registerTarget);
        cynes::dump<operation>(buffer, _modePRG);
        cynes::dump<operation>(buffer, _modeCHR);
        cynes::dump<operation>(buffer, _enableIRQ);
        cynes::dump<operation>(buffer, _shouldReloadIRQ);
    }
};

class AxROM : public Mapper {
public:
    AxROM(NES& nes, NESMetadata metadata);
    ~AxROM();

public:
    virtual void writeCPU(uint16_t address, uint8_t value);
};

template<uint8_t bankSize>
class MMC : public Mapper {
public:
    MMC(NES& nes, NESMetadata metadata, MirroringMode mode) :
        Mapper(nes, metadata, mode) {
        setBankCHR(0x0, 0x8, 0x0);

        setBankPRG(0x20, bankSize, 0x0);
        setBankPRG(0x20 + bankSize, 0x20 - bankSize, SIZE_PRG - 0x20 + bankSize);

        setBankWRAM(0x18, 0x8, 0x0, true);

        memset(_latches, false, 0x2);
        memset(_selectedBanks, 0x0, 0x4);
    }

    ~MMC() { }

public:
    virtual void writeCPU(uint16_t address, uint8_t value) {
        if (address < 0xA000) {
            Mapper::writeCPU(address, value);
        } else if (address < 0xB000) {
            setBankPRG(0x20, bankSize, (value & 0xF) * bankSize);
        } else if (address < 0xC000) {
            _selectedBanks[0x0] = value & 0x1F; updateBanks();
        } else if (address < 0xD000) {
            _selectedBanks[0x1] = value & 0x1F; updateBanks();
        } else if (address < 0xE000) {
            _selectedBanks[0x2] = value & 0x1F; updateBanks();
        } else if (address < 0xF000) {
            _selectedBanks[0x3] = value & 0x1F; updateBanks();
        } else {
            if (value & 0x01) {
                setMirroringMode(MirroringMode::HORIZONTAL);
            } else {
                setMirroringMode(MirroringMode::VERTICAL);
            }
        }
    }

    virtual uint8_t readPPU(uint16_t address) {
        uint8_t value = Mapper::readPPU(address);

        if (address == 0x0FD8) {
            _latches[0] = true; updateBanks();
        } else if (address == 0x0FE8) {
            _latches[0] = false; updateBanks();
        } else if (address >= 0x1FD8 && address < 0x1FE0) {
            _latches[1] = true; updateBanks();
        } else if (address >= 0x1FE8 && address < 0x1FF0) {
            _latches[1] = false; updateBanks();
        }

        return value;
    }

private:
    void updateBanks() {
        if (_latches[0]) {
            setBankCHR(0x0, 0x4, _selectedBanks[0x0] << 2);
        } else {
            setBankCHR(0x0, 0x4, _selectedBanks[0x1] << 2);
        }

        if (_latches[1]) {
            setBankCHR(0x4, 0x4, _selectedBanks[0x2] << 2);
        } else {
            setBankCHR(0x4, 0x4, _selectedBanks[0x3] << 2);
        }
    }

private:
    bool _latches[0x2];

    uint8_t _selectedBanks[0x4];

public:
    template<DumpOperation operation, class T> void dump(T& buffer) {
        Mapper::dump<operation>(buffer);

        cynes::dump<operation>(buffer, _latches);
        cynes::dump<operation>(buffer, _selectedBanks);
    }
};

using MMC2 = MMC<0x08>;
using MMC4 = MMC<0x10>;

class GxROM : public Mapper {
public:
    GxROM(NES& nes, NESMetadata metadata, MirroringMode mode);
    ~GxROM();

public:
    virtual void writeCPU(uint16_t address, uint8_t value);
};
}

#endif
