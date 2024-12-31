#include "mapper.hpp"
#include "cpu.hpp"
#include "nes.hpp"


cynes::Mapper::Mapper(
    NES& nes,
    NESMetadata metadata,
    MirroringMode mode,
    uint8_t sizeWRAM,
    uint8_t sizeVRAM,
    uint8_t sizeERAM
) : _nes{nes}
  , SIZE_PRG{metadata.sizePRG}
  , SIZE_CHR{metadata.sizeCHR}
  , SIZE_WRAM{sizeWRAM}
  , SIZE_VRAM{sizeVRAM}
  , SIZE_ERAM{sizeERAM}
  , _memoryPRG{metadata.memoryPRG}
  , _memoryCHR{metadata.memoryCHR}
  , _memoryWRAM{}
  , _memoryVRAM{}
  , _memoryERAM{}
  , _banksCPU{}
  , _banksPPU{}
{
    if (SIZE_WRAM) {
        _memoryWRAM = new uint8_t[uint64_t(SIZE_WRAM) << 10];

        if (metadata.trainer != nullptr) {
            memcpy(_memoryWRAM, metadata.trainer, 0x200);

            delete[] metadata.trainer;
        }
    }

    if (SIZE_VRAM) {
        _memoryVRAM = new uint8_t[uint64_t(SIZE_VRAM) << 10];
    }

    if (SIZE_ERAM) {
        _memoryERAM = new uint8_t[uint64_t(SIZE_ERAM) >> 10];
    }

    setMirroringMode(mode);
}

cynes::Mapper::~Mapper() {
    if (SIZE_PRG) {
        delete[] _memoryPRG;
    }

    if (SIZE_CHR) {
        delete[] _memoryCHR;
    }

    if (SIZE_WRAM) {
        delete[] _memoryWRAM;
    }

    if (SIZE_VRAM) {
        delete[] _memoryVRAM;
    }

    if (SIZE_ERAM) {
        delete[] _memoryERAM;
    }
}

void cynes::Mapper::tick() { }

void cynes::Mapper::writeCPU(uint16_t address, uint8_t value) {
    if (_banksCPU[address >> 10].access) {
        _banksCPU[address >> 10].memory[address & 0x3FF] = value;
    }
}

void cynes::Mapper::writePPU(uint16_t address, uint8_t value) {
    if (_banksPPU[address >> 10].access) {
        _banksPPU[address >> 10].memory[address & 0x3FF] = value;
    }
}

uint8_t cynes::Mapper::readCPU(uint16_t address) {
    if (_banksCPU[address >> 10].memory == nullptr) {
        return _nes.getOpenBus();
    }

    return _banksCPU[address >> 10].memory[address & 0x3FF];
}

uint8_t cynes::Mapper::readPPU(uint16_t address) {
    if (_banksPPU[address >> 10].memory == nullptr) {
        return 0x00;
    }

    return _banksPPU[address >> 10].memory[address & 0x3FF];
}

void cynes::Mapper::setBankPRG(uint8_t page, uint16_t address) {
    _banksCPU[page].memory = &_memoryPRG[address << 10];
    _banksCPU[page].access = false;
}

void cynes::Mapper::setBankPRG(uint8_t page, uint8_t size, uint16_t address) {
    for (uint8_t index = 0; index < size; index++) {
        setBankPRG(page + index, address + index);
    }
}

void cynes::Mapper::setBankWRAM(uint8_t page, uint16_t address, bool access) {
    _banksCPU[page].memory = &_memoryWRAM[address << 10];
    _banksCPU[page].access = access;
}

void cynes::Mapper::setBankWRAM(uint8_t page, uint8_t size, uint16_t address, bool access) {
    for (uint8_t index = 0; index < size; index++) {
        setBankWRAM(page + index, address + index, access);
    }
}

void cynes::Mapper::setBankCHR(uint8_t page, uint16_t address) {
    _banksPPU[page].memory = &_memoryCHR[address << 10];
    _banksPPU[page].access = false;
}

void cynes::Mapper::setBankCHR(uint8_t page, uint8_t size, uint16_t address) {
    for (uint8_t index = 0; index < size; index++) {
        setBankCHR(page + index, address + index);
    }
}

void cynes::Mapper::setBankVRAM(uint8_t page, uint16_t address, bool access) {
    _banksPPU[page].memory = &_memoryVRAM[address << 10];
    _banksPPU[page].access = access;
}

void cynes::Mapper::setBankVRAM(uint8_t page, uint8_t size, uint16_t address, bool access) {
    for (uint8_t index = 0; index < size; index++) {
        setBankVRAM(page + index, address + index, access);
    }
}

void cynes::Mapper::setBankERAMCPU(uint8_t page, uint16_t address, bool access) {
    _banksCPU[page].memory = &_memoryERAM[address << 10];
    _banksCPU[page].access = access;
}

void cynes::Mapper::setBankERAMCPU(uint8_t page, uint8_t size, uint16_t address, bool access) {
    for (uint8_t index = 0; index < size; index++) {
        setBankERAMCPU(page + index, address + index, access);
    }
}

void cynes::Mapper::setBankERAMPPU(uint8_t page, uint16_t address, bool access) {
    _banksPPU[page].memory = &_memoryERAM[address << 10];
    _banksPPU[page].access = access;
}

void cynes::Mapper::setBankERAMPPU(uint8_t page, uint8_t size, uint16_t address, bool access) {
    for (uint8_t index = 0; index < size; index++) {
        setBankERAMPPU(page + index, address + index, access);
    }
}

void cynes::Mapper::removeBankCPU(uint8_t page) {
    _banksPPU[page].memory = nullptr;
    _banksPPU[page].access = false;
}

void cynes::Mapper::removeBankCPU(uint8_t page, uint8_t size) {
    for (uint8_t index = 0; index < size; index++) {
        removeBankCPU(page + index);
    }
}

void cynes::Mapper::setMirroringMode(MirroringMode mode) {
    if (mode == MirroringMode::ONE_SCREEN_LOW) {
        setBankVRAM(0x8, 0x00, true);
        setBankVRAM(0x9, 0x00, true);
        setBankVRAM(0xA, 0x00, true);
        setBankVRAM(0xB, 0x00, true);
    } else if (mode == MirroringMode::ONE_SCREEN_HIGH) {
        setBankVRAM(0x8, 0x01, true);
        setBankVRAM(0x9, 0x01, true);
        setBankVRAM(0xA, 0x01, true);
        setBankVRAM(0xB, 0x01, true);
    } else if (mode == MirroringMode::VERTICAL) {
        setBankVRAM(0x8, 0x2, 0x00, true);
        setBankVRAM(0xA, 0x2, 0x00, true);
    } else if (mode == MirroringMode::HORIZONTAL) {
        setBankVRAM(0x8, 0x00, true);
        setBankVRAM(0x9, 0x00, true);
        setBankVRAM(0xA, 0x01, true);
        setBankVRAM(0xB, 0x01, true);
    }

    mirrorBankPPU(0x8, 0x4, 0xC);
}

void cynes::Mapper::mirrorBankCPU(uint8_t page, uint8_t size, uint8_t mirror) {
    for (uint8_t index = 0; index < size; index++) {
        _banksCPU[mirror + index].memory = _banksCPU[page + index].memory;
        _banksCPU[mirror + index].access = _banksCPU[page + index].access;
    }
}

void cynes::Mapper::mirrorBankPPU(uint8_t page, uint8_t size, uint8_t mirror) {
    for (uint8_t index = 0; index < size; index++) {
        _banksPPU[mirror + index].memory = _banksPPU[page + index].memory;
        _banksPPU[mirror + index].access = _banksPPU[page + index].access;
    }
}


cynes::NROM::NROM(NES& nes, NESMetadata metadata, MirroringMode mode) :
    Mapper(nes, metadata, mode) {
    setBankCHR(0x0, 0x8, 0x0);

    if (SIZE_PRG == 0x20) {
        setBankPRG(0x20, 0x20, 0x0);
    } else {
        setBankPRG(0x20, 0x10, 0x0);
        setBankPRG(0x30, 0x10, 0x0);
    }

    setBankWRAM(0x18, 0x8, 0x0, true);
}

cynes::NROM::~NROM() { }


cynes::MMC1::MMC1(
    NES& nes,
    NESMetadata metadata,
    MirroringMode mode
) : Mapper(nes, metadata, mode)
  , _tick{0x00}
  , _registers{}
  , _register{0x00}
  , _counter{0x00}
{
    memset(_registers, 0x00, 0x4);
    _registers[0x0] = 0xC;

    updateBanks();
}

cynes::MMC1::~MMC1() {}

void cynes::MMC1::tick() {
    if (_tick < 6) {
        _tick++;
    }
}

void cynes::MMC1::writeCPU(uint16_t address, uint8_t value) {
    if (address < 0x8000) {
        cynes::Mapper::writeCPU(address, value);
    } else {
        writeRegister((address >> 13) & 0x03, value);
    }
}

void cynes::MMC1::writeRegister(uint8_t registerTarget, uint8_t value) {
    if (_tick == 6) {
        if (value & 0x80) {
            _registers[0x0] |= 0xC;

            updateBanks();

            _register = 0x00;
            _counter = 0;
        } else {
            _register >>= 1;
            _register |= (value & 0x1) << 4;

            if (++_counter == 5) {
                _registers[registerTarget] = _register;

                updateBanks();

                _register = 0x00;
                _counter = 0x00;
            }
        }
    }

    _tick = 0;
}

void cynes::MMC1::updateBanks() {
    switch (_registers[0x0] & 0x03) {
    case 0: setMirroringMode(MirroringMode::ONE_SCREEN_LOW); break;
    case 1: setMirroringMode(MirroringMode::ONE_SCREEN_HIGH); break;
    case 2: setMirroringMode(MirroringMode::VERTICAL); break;
    case 3: setMirroringMode(MirroringMode::HORIZONTAL); break;
    }

    if (_registers[0x0] & 0x10) {
        setBankCHR(0x0, 0x4, (_registers[0x1] & 0x1F) << 2);
        setBankCHR(0x4, 0x4, (_registers[0x2] & 0x1F) << 2);
    } else {
        setBankCHR(0x0, 0x8, (_registers[0x1] & 0x1E) << 2);
    }

    if (_registers[0x0] & 0x08) {
        if (_registers[0x0] & 0x04) {
            setBankPRG(0x20, 0x10, (_registers[0x3] & 0x0F) << 4);
            setBankPRG(0x30, 0x10, SIZE_PRG - 0x10);
        } else {
            setBankPRG(0x20, 0x10, 0x0);
            setBankPRG(0x30, 0x10, (_registers[0x3] & 0xF) << 4);
        }
    } else {
        setBankPRG(0x20, 0x20, (_registers[0x3] & 0x0E) << 4);
    }

    if (_registers[0x3] & 0x10) {
        setBankWRAM(0x18, 0x8, 0x0, false);
    } else {
        setBankWRAM(0x18, 0x8, 0x0, true);
    }
}


cynes::UxROM::UxROM(NES& nes, NESMetadata metadata, MirroringMode mode) :
    Mapper(nes, metadata, mode, 0x0, 0x10) {
    setBankPRG(0x20, 0x10, 0x00);
    setBankPRG(0x30, 0x10, SIZE_PRG - 0x10);

    setBankVRAM(0x0, 0x8, 0x02, true);
}

cynes::UxROM::~UxROM() { }

void cynes::UxROM::writeCPU(uint16_t address, uint8_t value) {
    if (address < 0x8000) {
        cynes::Mapper::writeCPU(address, value);
    } else {
        setBankPRG(0x20, 0x10, value << 4);
    }
}


cynes::CNROM::CNROM(NES& nes, NESMetadata metadata, MirroringMode mode) :
    Mapper(nes, metadata, mode, 0x0) {
    setBankCHR(0x0, 0x8, 0x0);

    if (SIZE_PRG == 0x20) {
        setBankPRG(0x20, 0x20, 0x0);
    } else {
        setBankPRG(0x20, 0x10, 0x0);
        setBankPRG(0x30, 0x10, 0x0);
    }
}

cynes::CNROM::~CNROM() { }

void cynes::CNROM::writeCPU(uint16_t address, uint8_t value) {
    if (address < 0x8000) {
        cynes::Mapper::writeCPU(address, value);
    } else {
        setBankCHR(0x0, 0x8, (value & 0x3) << 3);
    }
}


cynes::MMC3::MMC3(
    NES& nes,
    NESMetadata metadata,
    MirroringMode mode
) : Mapper(nes, metadata, mode)
  , _tick{0x0000}
  , _registers{}
  , _counter{0x0000}
  , _counterReload{0x0000}
  , _registerTarget{0x00}
  , _modePRG{false}
  , _modeCHR{false}
  , _enableIRQ{false}
  , _shouldReloadIRQ{false}
{
    setBankCHR(0x0, 0x8, 0x0);
    setBankPRG(0x20, 0x10, 0x0);
    setBankPRG(0x30, 0x10, SIZE_PRG - 0x10);
    setBankWRAM(0x18, 0x8, 0x0, true);

    memset(_registers, 0x0000, 0x20);
}

cynes::MMC3::~MMC3() { }

void cynes::MMC3::tick() {
    if (_tick > 0 && _tick < 11) {
        _tick++;
    }
}

void cynes::MMC3::writeCPU(uint16_t address, uint8_t value) {
    if (address < 0x8000) {
        cynes::Mapper::writeCPU(address, value);
    } else if (address < 0xA000) {
        if (address & 0x1) {
            if (_registerTarget < 2) {
                value &= 0xFE;
            }

            _registers[_registerTarget] = value;

            if (_modePRG) {
                setBankPRG(0x20, 0x08, SIZE_PRG - 0x10);
                setBankPRG(0x28, 0x08, (_registers[0x7] & 0x3F) << 3);
                setBankPRG(0x30, 0x08, (_registers[0x6] & 0x3F) << 3);
                setBankPRG(0x38, 0x08, SIZE_PRG - 0x8);
            } else {
                setBankPRG(0x20, 0x08, (_registers[0x6] & 0x3F) << 3);
                setBankPRG(0x28, 0x08, (_registers[0x7] & 0x3F) << 3);
                setBankPRG(0x30, 0x10, SIZE_PRG - 0x10);
            }

            if (_modeCHR) {
                setBankCHR(0x0, _registers[0x2]);
                setBankCHR(0x1, _registers[0x3]);
                setBankCHR(0x2, _registers[0x4]);
                setBankCHR(0x3, _registers[0x5]);
                setBankCHR(0x4, 0x2, _registers[0x0]);
                setBankCHR(0x6, 0x2, _registers[0x1]);
            } else {
                setBankCHR(0x0, 0x2, _registers[0x0]);
                setBankCHR(0x2, 0x2, _registers[0x1]);
                setBankCHR(0x4, _registers[0x2]);
                setBankCHR(0x5, _registers[0x3]);
                setBankCHR(0x6, _registers[0x4]);
                setBankCHR(0x7, _registers[0x5]);
            }
        } else {
            _registerTarget = value & 0x07;
            _modePRG = value & 0x40;
            _modeCHR = value & 0x80;
        }
    } else if (address < 0xC000) {
        if (address & 0x1) {
            if (value & 0x40) {
                setBankWRAM(0x18, 0x8, 0x0, false);
            } else {
                setBankWRAM(0x18, 0x8, 0x0, true);
            }
        } else  if (value & 0x1) {
            setMirroringMode(MirroringMode::HORIZONTAL);
        } else {
            setMirroringMode(MirroringMode::VERTICAL);
        }
    } else if (address < 0xE000) {
        if (address & 0x1) {
            _counter = 0x0000;
            _shouldReloadIRQ = true;
        } else {
            _counterReload = value;
        }
    } else {
        if (address & 0x1) {
            _enableIRQ = true;
        } else {
            _enableIRQ = false;
            _nes.getCPU().setMapperIRQ(false);
        }
    }
}

void cynes::MMC3::writePPU(uint16_t address, uint8_t value) {
    updateState(address & 0x1000);

    cynes::Mapper::writePPU(address, value);
}

uint8_t cynes::MMC3::readPPU(uint16_t address) {
    updateState(address & 0x1000);

    return cynes::Mapper::readPPU(address);
}

void cynes::MMC3::updateState(bool state) {
    if (state) {
        if (_tick > 10) {
            if (_counter == 0 || _shouldReloadIRQ) {
                _counter = _counterReload;
            } else {
                _counter--;
            }

            if (_counter == 0 && _enableIRQ) {
                _nes.getCPU().setMapperIRQ(true);
            }

            _shouldReloadIRQ = false;
        }

        _tick = 0;
    } else {
        if (_tick == 0) {
            _tick = 1;
        }
    }
}


cynes::AxROM::AxROM(NES& nes, NESMetadata metadata) : Mapper(nes, metadata, MirroringMode::ONE_SCREEN_LOW, 0x8, 0x10) {
    setBankVRAM(0x0, 0x8, 0x2, true);
    setBankPRG(0x20, 0x20, 0x0);
}

cynes::AxROM::~AxROM() { }

void cynes::AxROM::writeCPU(uint16_t address, uint8_t value) {
    if (address < 0x8000) {
        cynes::Mapper::writeCPU(address, value);
    } else {
        setBankPRG(0x20, 0x20, (value & 0x07) << 5);

        if (value & 0x10) {
            setMirroringMode(MirroringMode::ONE_SCREEN_HIGH);
        } else {
            setMirroringMode(MirroringMode::ONE_SCREEN_LOW);
        }
    }
}


cynes::GxROM::GxROM(NES& nes, NESMetadata metadata, MirroringMode mode) : Mapper(nes, metadata, mode, 0x0) {
    setBankPRG(0x20, 0x20, 0x0);
    setBankCHR(0x00, 0x08, 0x0);
}

cynes::GxROM::~GxROM() { }

void cynes::GxROM::writeCPU(uint16_t address, uint8_t value) {
    if (address < 0x8000) {
        cynes::Mapper::writeCPU(address, value);
    } else {
        setBankPRG(0x20, 0x20, (value & 0x30) << 1);
        setBankCHR(0x00, 0x08, (value & 0x03) << 3);
    }
}
