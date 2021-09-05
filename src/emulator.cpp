// cynes - C/C++ NES emulator with Python bindings
// Copyright (C) 2021  Combey Theo <https://www.gnu.org/licenses/>

#include "emulator.h"

#include <iostream>


nes::Mapper::Mapper(uint8_t* programMemory, uint8_t* characterMemory, MirroringMode mode) :
    programMemory(programMemory), characterMemory(characterMemory), mode(mode) { }

nes::Mapper::~Mapper() {
    delete[] programMemory;
    delete[] characterMemory;
}

unsigned int nes::Mapper::size() {
    return 0x1;
}

void nes::Mapper::dump(uint8_t*& buffer) {
    switch (mode) {
    case MirroringMode::ONE_SCREEN_LOW: nes::write(buffer, 0x00); break;
    case MirroringMode::ONE_SCREEN_HIGH: nes::write(buffer, 0x01); break;
    case MirroringMode::VERTICAL: nes::write(buffer, 0x02); break;
    case MirroringMode::HORIZONTAL: nes::write(buffer, 0x03); break;
    }
}

void nes::Mapper::load(uint8_t*& buffer) {
    uint8_t mirroring = 0x00;

    nes::read(buffer, mirroring);

    switch (mirroring) {
    case 0x00: mode = MirroringMode::ONE_SCREEN_LOW; break;
    case 0x01: mode = MirroringMode::ONE_SCREEN_HIGH; break;
    case 0x02: mode = MirroringMode::VERTICAL; break;
    case 0x03: mode = MirroringMode::HORIZONTAL; break;
    }
}

void nes::Mapper::notify(uint16_t address, uint32_t cycle) { }

bool nes::Mapper::shouldIRQ() {
    return false;
}

uint16_t nes::Mapper::getMirroredAddress(uint16_t address) {
    switch (mode) {
    case MirroringMode::ONE_SCREEN_LOW: return address & 0x3FF;
    case MirroringMode::ONE_SCREEN_HIGH: return (address & 0x3FF) + 0xC00;
    case MirroringMode::VERTICAL: return address & 0x7FF;
    case MirroringMode::HORIZONTAL: return address & 0xBFF;
    default: return address & 0xFFF;
    }
}


nes::Mapper000::Mapper000(uint8_t* programMemory, uint8_t* characterMemory, uint8_t programBanks, uint8_t characterBanks, MirroringMode mode) :
    Mapper(programMemory, characterMemory, mode), programBanks(programBanks), characterBanks(characterBanks) { }

nes::Mapper000::~Mapper000() { }

uint8_t nes::Mapper000::readCPU(uint16_t address) {
    if (address >= 0x6000 && address < 0x8000) {
        return ram[address & 0x1FFF];
    }

    return programMemory[address & (programBanks > 1 ? 0x7FFF : 0x3FFF)];
}

uint8_t nes::Mapper000::readPPU(uint16_t address) {
    return characterMemory[address & 0x1FFF];
}

void nes::Mapper000::writeCPU(uint16_t address, uint8_t value) {
    if (address >= 0x6000 && address < 0x8000) {
        ram[address & 0x1FFF] = value;
    }
}

void nes::Mapper000::writePPU(uint16_t address, uint8_t value) {
    if (characterBanks == 0) {
        characterMemory[address & 0x1FFF] = value;
    }
}

unsigned int nes::Mapper000::size() {
    if (programBanks == 0) {
        return 0x4000 + Mapper::size();
    }

    return 0x2000 + Mapper::size();
}

void nes::Mapper000::dump(uint8_t*& buffer) {
    Mapper::dump(buffer);

    nes::write(buffer, ram, 0x2000);

    if (programBanks == 0) {
        nes::write(buffer, characterMemory, 0x2000);
    }
}

void nes::Mapper000::load(uint8_t*& buffer) {
    Mapper::load(buffer);

    nes::read(buffer, ram, 0x2000);

    if (programBanks == 0) {
        nes::read(buffer, characterMemory, 0x2000);
    }
}


nes::Mapper001::Mapper001(uint8_t* programMemory, uint8_t* characterMemory, uint8_t programBanks, uint8_t characterBanks, MirroringMode mode) :
    Mapper(programMemory, characterMemory, mode), programBanks(programBanks), characterBanks(characterBanks) {
    registerControl = 0x1C;

    programBankSelected = (programBanks - 1) << 4;
}

nes::Mapper001::~Mapper001() { }

uint8_t nes::Mapper001::readCPU(uint16_t address) {
    if (address >= 0x6000 && address < 0x8000) {
        return ram[address & 0x1FFF];
    }

    address &= 0x7FFF;

    if (registerControl & 0x08) {
        if (address < 0x4000) {
            return programMemory[(programBankSelected & 0x0F) * 0x4000 + (address & 0x3FFF)];
        } else {
            return programMemory[(programBankSelected & 0xF0) * 0x400 + (address & 0x3FFF)];
        }
    }

    return programMemory[programBankSelected * 0x4000 + address];
}

uint8_t nes::Mapper001::readPPU(uint16_t address) {
    if (characterBanks == 0) {
        return characterMemory[address & 0x1FFF];
    }

    address &= 0x1FFF;

    if (registerControl & 0x10) {
        if (address < 0x1000) {
            return characterMemory[(characterBankSelected & 0x001F) * 0x1000 + (address & 0x0FFF)];
        } else {
            return characterMemory[(characterBankSelected & 0x03E0) * 0x80 + (address & 0x0FFF)];
        }
    }

    return characterMemory[characterBankSelected * 0x1000 + address];
}

void nes::Mapper001::writeCPU(uint16_t address, uint8_t value) {
    if (address >= 0x6000 && address < 0x8000) {
        ram[address & 0x1FFF] = value;
    } else {
        if (value & 0x80) {
            registerLoad = 0x00;

            counter = 0;
        } else {
            registerLoad >>= 1;
            registerLoad |= (value & 0x01) << 4;

            if (++counter == 5) {
                uint8_t registerTarget = (address >> 13) & 0x03;

                switch (registerTarget) {
                case 0: {
                    registerControl = registerLoad & 0x1F;

                    switch (registerControl & 0x03) {
                    case 0: mode = MirroringMode::ONE_SCREEN_LOW; break;
                    case 1: mode = MirroringMode::ONE_SCREEN_HIGH; break;
                    case 2: mode = MirroringMode::VERTICAL; break;
                    case 3: mode = MirroringMode::HORIZONTAL; break;
                    }

                    break;
                }

                case 1: {
                    if (registerControl & 0x10) {
                        characterBankSelected &= 0x3E0;
                        characterBankSelected |= registerLoad & 0x1F;
                    } else {
                        characterBankSelected = registerLoad & 0x1E;
                    }

                    break;
                }

                case 2: {
                    if (registerControl & 0x10) {
                        characterBankSelected &= 0x001F;
                        characterBankSelected |= (registerLoad & 0x1F) << 5;
                    }

                    break;
                }

                case 3: {
                    if (registerControl & 0x08) {
                        if (registerControl & 0x04) {
                            programBankSelected = (programBanks - 1) << 4;
                            programBankSelected |= registerLoad & 0xF;
                        } else {
                            programBankSelected = (registerLoad & 0xF) << 4;
                        }
                    } else {
                        programBankSelected = registerLoad & 0x0E;
                    }

                    break;
                }
                }

                registerLoad = 0x00;

                counter = 0;
            }
        }
    }
}

void nes::Mapper001::writePPU(uint16_t address, uint8_t value) {
    if (characterBanks == 0) {
        characterMemory[address & 0x1FFF] = value;
    }
}

unsigned int nes::Mapper001::size() {
    if (programBanks == 0) {
        return 0x4006 + Mapper::size();
    }

    return 0x2006 + Mapper::size();
}

void nes::Mapper001::dump(uint8_t*& buffer) {
    Mapper::dump(buffer);

    nes::write(buffer, counter);
    nes::write(buffer, registerControl);
    nes::write(buffer, registerLoad);
    nes::write(buffer, programBankSelected);
    nes::write(buffer, characterBankSelected);
    nes::write(buffer, ram, 0x2000);

    if (programBanks == 0) {
        nes::write(buffer, characterMemory, 0x2000);
    }
}

void nes::Mapper001::load(uint8_t*& buffer) {
    Mapper::load(buffer);

    nes::read(buffer, counter);
    nes::read(buffer, registerControl);
    nes::read(buffer, registerLoad);
    nes::read(buffer, programBankSelected);
    nes::read(buffer, characterBankSelected);
    nes::read(buffer, ram, 0x2000);

    if (programBanks == 0) {
        nes::read(buffer, characterMemory, 0x2000);
    }
}


nes::Mapper003::Mapper003(uint8_t* programMemory, uint8_t* characterMemory, uint8_t programBanks, uint8_t characterBanks, MirroringMode mode) :
    Mapper(programMemory, characterMemory, mode), programBanks(programBanks), characterBanks(characterBanks), selectedCharacterBank(0) { }

nes::Mapper003::~Mapper003() { }

uint8_t nes::Mapper003::readCPU(uint16_t address) {
    return programMemory[address & (programBanks > 1 ? 0x7FFF : 0x3FFF)];
}

uint8_t nes::Mapper003::readPPU(uint16_t address) {
    return characterMemory[address + selectedCharacterBank * 0x2000];
}

void nes::Mapper003::writeCPU(uint16_t address, uint8_t value) {
    selectedCharacterBank = value & 0x3;
}

void nes::Mapper003::writePPU(uint16_t address, uint8_t value) { }

unsigned int nes::Mapper003::size() {
    return 0x1 + Mapper::size();
}

void nes::Mapper003::dump(uint8_t*& buffer) {
    Mapper::dump(buffer);

    nes::write(buffer, selectedCharacterBank);
}

void nes::Mapper003::load(uint8_t*& buffer) {
    Mapper::load(buffer);
    
    nes::read(buffer, selectedCharacterBank);
}


nes::Mapper004::Mapper004(uint8_t* programMemory, uint8_t* characterMemory, uint8_t programBanks, uint8_t characterBanks, MirroringMode mode) :
    Mapper(programMemory, characterMemory, mode), programBanks(programBanks), characterBanks(characterBanks) {
    programBankPointers[0x0] = 0x0000;
    programBankPointers[0x1] = 0x2000;
    programBankPointers[0x2] = (programBanks * 2 - 2) * 0x2000;
    programBankPointers[0x3] = (programBanks * 2 - 1) * 0x2000;
}

nes::Mapper004::~Mapper004() { }

uint8_t nes::Mapper004::readCPU(uint16_t address) {
    if (address >= 0x6000 && address < 0x8000) {
        return ram[address & 0x1FFF];
    } else if (address < 0xA000) {
        return programMemory[programBankPointers[0x0] + (address & 0x1FFF)];
    } else if (address < 0xC000) {
        return programMemory[programBankPointers[0x1] + (address & 0x1FFF)];
    } else if (address < 0xE000) {
        return programMemory[programBankPointers[0x2] + (address & 0x1FFF)];
    } else {
        return programMemory[programBankPointers[0x3] + (address & 0x1FFF)];
    }
}

uint8_t nes::Mapper004::readPPU(uint16_t address) {
    if (address < 0x0400) {
        return characterMemory[characterBankPointers[0x0] + (address & 0x3FF)];
    } else if (address < 0x0800) {
        return characterMemory[characterBankPointers[0x1] + (address & 0x3FF)];
    } else if (address < 0x0C00) {
        return characterMemory[characterBankPointers[0x2] + (address & 0x3FF)];
    } else if (address < 0x1000) {
        return characterMemory[characterBankPointers[0x3] + (address & 0x3FF)];
    } else if (address < 0x1400) {
        return characterMemory[characterBankPointers[0x4] + (address & 0x3FF)];
    } else if (address < 0x1800) {
        return characterMemory[characterBankPointers[0x5] + (address & 0x3FF)];
    } else if (address < 0x1C00) {
        return characterMemory[characterBankPointers[0x6] + (address & 0x3FF)];
    } else {
        return characterMemory[characterBankPointers[0x7] + (address & 0x3FF)];
    }
}

void nes::Mapper004::writeCPU(uint16_t address, uint8_t value) {
    if (address >= 0x6000 && address < 0x8000) {
        ram[address & 0x1FFF] = value;
    } else if (address < 0xA000) {
        if (address & 0x1) {
            registers[targetRegister] = value;

            updateProgramMapping();
            updateCharacterMapping();
        } else {
            targetRegister = value & 0x07;
            programMode = value & 0x40;
            characterMode = value & 0x80;
        }
    } else if (address < 0xC000) {
        if (address & 0x1) {
            // TODO RAM protect!
        } else  if (value & 0x1) {
            mode = MirroringMode::HORIZONTAL;
        } else {
            mode = MirroringMode::VERTICAL;
        }
    } else if (address < 0xE000) {
        if (address & 0x1) {
            counter = 0x0000;
            shouldReloadIRQ = true;
        } else {
            reloadValue = value;
        }
    } else {
        if (address & 0x1) {
            enableIRQ = true;
        } else {
            enableIRQ = false;
            sendIRQ = false;
        }
    }
}

void nes::Mapper004::writePPU(uint16_t address, uint8_t value) { }

unsigned int nes::Mapper004::size() {
    return 0x205E + Mapper::size();
}

void nes::Mapper004::dump(uint8_t*& buffer) {
    Mapper::dump(buffer);

    uint8_t flags = programMode;
    flags |= characterMode << 1;
    flags |= sendIRQ << 2;
    flags |= enableIRQ << 3;
    flags |= shouldReloadIRQ << 4;

    nes::write(buffer, targetRegister);
    nes::write(buffer, flags);
    nes::write(buffer, counter);
    nes::write(buffer, reloadValue);
    nes::write(buffer, lastCycle);
    nes::write(buffer, cyclesDown);
    nes::write(buffer, registers, 0x08);
    nes::write(buffer, programBankPointers, 0x04);
    nes::write(buffer, characterBankPointers, 0x08);
    nes::write(buffer, ram, 0x2000);
}

void nes::Mapper004::load(uint8_t*& buffer) {
    Mapper::load(buffer);

    uint8_t flags = 0x00;

    nes::read(buffer, targetRegister);
    nes::read(buffer, flags);
    nes::read(buffer, counter);
    nes::read(buffer, reloadValue);
    nes::read(buffer, lastCycle);
    nes::read(buffer, cyclesDown);
    nes::read(buffer, registers, 0x08);
    nes::read(buffer, programBankPointers, 0x04);
    nes::read(buffer, characterBankPointers, 0x08);
    nes::read(buffer, ram, 0x2000);

    programMode = flags & 0x01;
    characterMode = flags & 0x02;
    sendIRQ = flags & 0x04;
    enableIRQ = flags & 0x08;
    shouldReloadIRQ = flags & 0x10;
}

void nes::Mapper004::notify(uint16_t address, uint32_t cycle) {
    if (cyclesDown > 0) {
        if (lastCycle > cycle) {
            cyclesDown += 89342 - lastCycle + cycle;
        } else {
            cyclesDown += cycle - lastCycle;
        }
    }

    if (address & 0x1000) {
        if (cyclesDown > 10) {
            clockIRQ();
        }

        cyclesDown = 0;
    } else {
        if (cyclesDown == 0) {
            cyclesDown = 1;
        }
    }

    lastCycle = cycle;
}

bool nes::Mapper004::shouldIRQ() {
    bool irq = sendIRQ;

    sendIRQ = false;

    return irq;
}

void nes::Mapper004::updateProgramMapping() {
    if (programMode) {
        programBankPointers[2] = (registers[6] & 0x3F) * 0x2000;
        programBankPointers[0] = (programBanks * 2 - 2) * 0x2000;
    } else {
        programBankPointers[0] = (registers[6] & 0x3F) * 0x2000;
        programBankPointers[2] = (programBanks * 2 - 2) * 0x2000;
    }

    programBankPointers[1] = (registers[7] & 0x3F) * 0x2000;
    programBankPointers[3] = (programBanks * 2 - 1) * 0x2000;
}

void nes::Mapper004::updateCharacterMapping() {
    if (characterMode) {
        characterBankPointers[0x0] = registers[0x2] * 0x0400;
        characterBankPointers[0x1] = registers[0x3] * 0x0400;
        characterBankPointers[0x2] = registers[0x4] * 0x0400;
        characterBankPointers[0x3] = registers[0x5] * 0x0400;
        characterBankPointers[0x4] = (registers[0x0] & 0xFE) * 0x0400;
        characterBankPointers[0x5] = registers[0x0] * 0x0400 + 0x0400;
        characterBankPointers[0x6] = (registers[0x1] & 0xFE) * 0x0400;
        characterBankPointers[0x7] = registers[0x1] * 0x0400 + 0x0400;
    } else {
        characterBankPointers[0x0] = (registers[0x0] & 0xFE) * 0x0400;
        characterBankPointers[0x1] = registers[0x0] * 0x0400 + 0x0400;
        characterBankPointers[0x2] = (registers[0x1] & 0xFE) * 0x0400;
        characterBankPointers[0x3] = registers[0x1] * 0x0400 + 0x0400;
        characterBankPointers[0x4] = registers[0x2] * 0x0400;
        characterBankPointers[0x5] = registers[0x3] * 0x0400;
        characterBankPointers[0x6] = registers[0x4] * 0x0400;
        characterBankPointers[0x7] = registers[0x5] * 0x0400;
    }
}

void nes::Mapper004::clockIRQ() {
    if (counter == 0 || shouldReloadIRQ) {
        counter = reloadValue;
    } else {
        counter--;
    }

    if (counter == 0 && enableIRQ) {
        sendIRQ = true;
    }

    shouldReloadIRQ = false;
}


nes::Mapper* nes::load(const char* path) {
    FILE* stream = fopen(path, "rb");

    if (!stream) {
        return nullptr;
    }

    uint32_t header = getc(stream) << 24 | getc(stream) << 16 | getc(stream) << 8 | getc(stream);

    if (header != 0x4E45531A) {
        return  nullptr;
    }

    uint8_t programBanks = getc(stream);
    uint8_t characterBanks = getc(stream);

    uint8_t flag6 = getc(stream);
    uint8_t flag7 = getc(stream);

    for (int k = 0; k < 0x8; k++) {
        getc(stream);
    }

    bool mirroring = flag6 & 0x01;
    bool programRAM = flag6 & 0x02;
    bool trainer = flag6 & 0x04;
    bool ignoreMirroring = flag6 & 0x08;

    uint8_t lowMapper = flag6 >> 4;

    bool vsUni = flag7 & 0x01;
    bool playChoice10 = flag7 & 0x02;
    bool nes2Format = false;

    uint8_t hiMapper = flag7 >> 4;

    uint8_t mapperId = hiMapper << 4 | lowMapper;

    uint8_t* programROM = new uint8_t[0x4000 * programBanks];
    uint8_t* characterROM;

    for (int k = 0; k < 0x4000 * programBanks; k++) {
        programROM[k] = getc(stream);
    }

    if (characterBanks == 0) {
        characterROM = new uint8_t[0x2000]{ 0 };
    } else {
        characterROM = new uint8_t[0x2000 * characterBanks];

        for (int k = 0; k < 0x2000 * characterBanks; k++) {
            characterROM[k] = getc(stream);
        }
    }

    fclose(stream);

    MirroringMode mode = mirroring ? MirroringMode::VERTICAL : MirroringMode::HORIZONTAL;

    if (mapperId == 0) {
        return new Mapper000(programROM, characterROM, programBanks, characterBanks, mode);
    } else if (mapperId == 1) {
        return new Mapper001(programROM, characterROM, programBanks, characterBanks, mode);
    } else if (mapperId == 3) {
        return new Mapper003(programROM, characterROM, programBanks, characterBanks, mode);
    } else if (mapperId == 4) {
        return new Mapper004(programROM, characterROM, programBanks, characterBanks, mode);
    }

    return nullptr;
}

nes::PPU::PPU(Mapper& mapper) : mapper(mapper), pixelX(0), pixelY(0) { }

nes::PPU::~PPU() { }

void nes::PPU::tick() {
    if (delayDataWrite > 0) {
        if (--delayDataWrite == 0) {
            registerV = registerVDelayed;
            registerT = registerV;

            if (pixelY >= 241 || (!maskRenderBackground && !maskRenderForeground)) {
                mapper.notify(registerV & 0x3FFF, pixelY * 341 + pixelX);
            }
        }
    }

    if (pixelY < 240) {
        if ((pixelX >= 2 && pixelX < 258) || (pixelX >= 321 && pixelX < 338)) {
            fetchBackgroundData();
        }

        if (pixelX == 256) {
            incrementScrollY();
        } else if (pixelX == 257) {
            resetScrollX();
        }

        if (pixelX >= 2 && pixelX < 257) {
            updateForegroundShifter();
        }

        if (pixelX < 65) {
            clearForegroundData();
        } else if (pixelX < 257) {
            fetchForegroundData();
        } else if (pixelX < 321) {
            loadForegroundShifter();
        }
    } else if (pixelY == 241) {
        if (pixelX == 0) {
            mapper.notify(registerV, pixelY * 341 + pixelX);
        } else if (pixelX == 1 && !preventNMI) {
            statusVerticalBlank = true;

            if (controlInteruptOnVBL) {
                sendNMI = true;
            }
        }
    } else if (pixelY == 261) {
        if (pixelX == 1) {
            statusSpriteOverflow = false;
            statusSprite0Hit = false;
            statusVerticalBlank = false;

            memset(foregroundShifter, 0x00, 0x10);
        }

        if ((pixelX >= 2 && pixelX < 258) || (pixelX >= 321 && pixelX < 338)) {
            fetchBackgroundData();
        }

        if (pixelX == 256) {
            incrementScrollY();
        } else if (pixelX == 257) {
            resetScrollX();
        } else  if (pixelX >= 280 && pixelX < 305) {
            resetScrollY();
        }

        if (pixelX > 1 && pixelX < 257) {
            updateForegroundShifter();
        } else if (pixelX < 321) {
            loadForegroundShifter();
        }
    }

    uint8_t palette = blend();

    if (pixelX > 0 && pixelX < 257 && pixelY < 240) {
        memcpy(frameBuffer + (pixelY * 256 + pixelX - 1) * 3, PALETTE_COLORS[maskColorEmphasize][internalRead(0x3F00 | palette)], 3);
    }

    pixelX++;

    if (pixelX > 340) {
        resetForegroundData();

        pixelX = 0;
        pixelY++;

        if (pixelY == 241) {
            frameReady = true;
        }

        if (pixelY > 261) {
            pixelY = 0;

            if (cycleLatch && (maskRenderBackground || maskRenderForeground)) {
                pixelX++;
            }

            preventNMI = false;
            cycleLatch = !cycleLatch;

            for (int k = 0; k < 3; k++) {
                if (clockDecays[k] > 0) {
                    if (--clockDecays[k] == 0) {
                        switch (k) {
                        case 0: registerDecay &= 0x3F; break;
                        case 1: registerDecay &= 0xDF; break;
                        case 2: registerDecay &= 0xE0; break;
                        }
                    }
                }
            }
        }
    }

    if (delayDataRead) {
        delayDataRead--;
    }
}

void nes::PPU::write(uint8_t address, uint8_t value) {
    registerDecay = value;

    updateDecay(0xFF);

    switch (address) {
    case Register::PPU_CTRL: {
        registerT &= 0xF3FF;
        registerT |= (value & 0x03) << 10;

        controlIncrementMode = value & 0x04;
        controlForegroundTable = value & 0x08;
        controlBackgroundTable = value & 0x10;
        controlForegroundLarge = value & 0x20;
        controlInteruptOnVBL = value & 0x80;

        //"By toggling NMI_output ($2000 bit 7) during vertical blank without reading $2002, 
        // a program can cause /NMI to be pulled low multiple times, causing multiple NMIs to be generated."

        if (pixelY == 241) {
            if (pixelX < 4 && !controlInteruptOnVBL) {
                sendNMI = false;
            }

            if (pixelX < 3 && controlInteruptOnVBL && statusVerticalBlank) {
                sendNMI = true;
            }
        }

        break;
    }

    case Register::PPU_MASK: {
        maskGreyscaleMode = value & 0x01;
        maskRenderBackgroundLeft = value & 0x02;
        maskRenderForegroundLeft = value & 0x04;
        maskRenderBackground = value & 0x08;
        maskRenderForeground = value & 0x10;

        maskColorEmphasize = value >> 5;

        break;
    }

    case Register::OAM_ADDR: {
        foregroundSpritePointer = value;

        break;
    }

    case Register::OAM_DATA: {
        if (pixelY >= 240 || !maskRenderBackground && !maskRenderForeground) {
            if ((foregroundSpritePointer & 0x03) == 0x02) {
                value &= 0xE3;
            }

            memorySprites[foregroundSpritePointer++] = value;
        } else {
            foregroundSpritePointer += 4;
        }

        break;
    }

    case Register::PPU_SCROLL: {
        if (!addressLatch) {
            offsetX = value & 0x07;

            registerT &= 0xFFE0;
            registerT |= value >> 3;
        } else {
            registerT &= 0x8C1F;

            registerT |= (value & 0xF8) << 2;
            registerT |= (value & 0x07) << 12;
        }

        addressLatch = !addressLatch;

        break;
    }

    case Register::PPU_ADDR: {
        if (!addressLatch) {
            registerT &= 0x00FF;
            registerT |= value << 8;
        } else {
            registerT &= 0xFF00;
            registerT |= value;

            delayDataWrite = 3;

            registerVDelayed = registerT;
        }

        addressLatch = !addressLatch;

        break;
    }

    case Register::PPU_DATA: {
        if ((registerV & 0x3FFF) >= 0x3F00) {
            internalWrite(registerV, value);
        } else {
            if (pixelY > 240 || !maskRenderBackground && !maskRenderForeground) {
                internalWrite(registerV, value);
            } else {
                internalWrite(registerV, registerV & 0xFF);
            }
        }

        if (pixelY > 240 || !maskRenderBackground && !maskRenderForeground) {
            registerV += controlIncrementMode ? 32 : 1;

            mapper.notify(registerV, pixelY * 341 + pixelX);
        } else {
            incrementScrollX();
            incrementScrollY();
        }

        break;
    }
    }
}

void nes::PPU::writeDMA(uint8_t value) {
    memorySprites[foregroundSpritePointer++] = value;
}

uint8_t nes::PPU::read(uint8_t address) {
    switch (address) {
    case Register::PPU_STATUS: {
        registerDecay &= 0x1F;
        registerDecay |= statusSpriteOverflow << 5;
        registerDecay |= statusSprite0Hit << 6;
        registerDecay |= statusVerticalBlank << 7;

        updateDecay(0x1F);

        /*
        Reading $2002 within a few PPU clocks of when VBL is set results in special-case behavior.
        Reading one PPU clock before reads it as clear and never sets the flag or generates NMI for that frame.
        Reading on the same PPU clock or one later reads it as set, clears it, and suppresses the NMI for that frame.
        Reading two or more PPU clocks before/after it's set behaves normally (reads flag's value, clears it, and doesn't affect NMI operation).
        This suppression behavior is due to the $2002 read pulling the NMI line back up too quickly after it drops (NMI is active low) for the CPU to see it.
        (CPU inputs like NMI are sampled each clock.)
        */

        statusVerticalBlank = false;

        addressLatch = false;

        if (pixelY == 241) {
            if (pixelX == 1) {
                preventNMI = true;
            } else if (pixelX < 4) {
                //return registerDecay & 0x7F;
            }
        }

        return registerDecay;
    }

    case Register::OAM_DATA: {
        registerDecay = memorySprites[foregroundSpritePointer];

        if ((foregroundSpritePointer & 0x3) == 0x2) {
            registerDecay &= 0xE3;
        }

        updateDecay(0xFF);

        return registerDecay;
    }

    case Register::PPU_DATA: {
        if (delayDataRead) {
            return registerDecay;
        }

        uint8_t value = internalRead(registerV);

        if (registerV >= 0x3F00) {
            registerDecay &= 0xC0;
            registerDecay |= value & 0x3F;

            updateDecay(0xC0);

            bufferData = internalRead(registerV - 0x1000);
        } else {
            registerDecay = bufferData;
            bufferData = value;

            updateDecay(0xFF);
        }

        delayDataRead = 6;

        if (pixelY > 240 || !maskRenderBackground && !maskRenderForeground) {
            registerV += controlIncrementMode ? 32 : 1;

            mapper.notify(registerV, pixelY * 341 + pixelX);
        } else {
            incrementScrollX();
            incrementScrollY();
        }

        return registerDecay;
    }

    default: return registerDecay;
    }
}

void nes::PPU::updateDecay(uint8_t mask) {
    switch (mask) {
    case 0x1F: memset(clockDecays, 30, 2); break;
    case 0xC0: clockDecays[0] = 30; clockDecays[2] = 30; break;
    case 0xFF: memset(clockDecays, 30, 3); break;
    }
}

bool nes::PPU::shouldNMI() {
    bool nmi = sendNMI;

    sendNMI = false;

    return nmi;
}

bool nes::PPU::shouldRender() {
    bool render = frameReady;

    frameReady = false;

    return render;
}

uint8_t* nes::PPU::getFrameBuffer() {
    return frameBuffer;
}

void nes::PPU::dump(uint8_t*& buffer) {
    uint8_t controlFlags = controlIncrementMode;
    controlFlags |= controlForegroundTable << 1;
    controlFlags |= controlForegroundTable << 1;
    controlFlags |= controlBackgroundTable << 2;
    controlFlags |= controlForegroundLarge << 3;
    controlFlags |= controlInteruptOnVBL << 4;
    controlFlags |= foregroundSprite0Line << 5;
    controlFlags |= foregroundSprite0Should << 6;
    controlFlags |= foregroundSprite0Hit << 7;

    uint8_t maskFlags = maskGreyscaleMode;
    maskFlags |= maskRenderBackgroundLeft << 1;
    maskFlags |= maskRenderForegroundLeft << 2;
    maskFlags |= maskRenderBackground << 3;
    maskFlags |= maskRenderForeground << 4;
    maskFlags |= maskColorEmphasize << 5;

    uint8_t statusFlags = sendNMI;
    statusFlags |= preventNMI << 1;
    statusFlags |= addressLatch << 2;
    statusFlags |= cycleLatch << 3;
    statusFlags |= frameReady << 4;
    statusFlags |= statusSpriteOverflow << 5;
    statusFlags |= statusSprite0Hit << 6;
    statusFlags |= statusVerticalBlank << 7;

    nes::write(buffer, pixelX);
    nes::write(buffer, pixelY);
    nes::write(buffer, bufferData);
    nes::write(buffer, controlFlags);
    nes::write(buffer, maskFlags);
    nes::write(buffer, statusFlags);
    nes::write(buffer, registerDecay);
    nes::write(buffer, registerVDelayed);
    nes::write(buffer, delayDataRead);
    nes::write(buffer, delayDataWrite);
    nes::write(buffer, registerT);
    nes::write(buffer, registerV);
    nes::write(buffer, offsetX);
    nes::write(buffer, foregroundDataPointer);
    nes::write(buffer, foregroundSpriteCount);
    nes::write(buffer, foregroundSpriteCountNext);
    nes::write(buffer, foregroundSpritePointer);
    nes::write(buffer, foregroundReadDelay);
    nes::write(buffer, foregroundOffset);
    nes::write(buffer, foregroundEvaluationStep);
    nes::write(buffer, backgroundShifter, 0x4);
    nes::write(buffer, clockDecays, 0x3);
    nes::write(buffer, backgroundData, 0x4);
    nes::write(buffer, foregroundData, 0x20);
    nes::write(buffer, foregroundShifter, 0x10);
    nes::write(buffer, foregroundAttributes, 0x8);
    nes::write(buffer, foregroundPositions, 0x8);
    nes::write(buffer, memoryVideo, 0x1000);
    nes::write(buffer, memoryPalette, 0x20);
    nes::write(buffer, memorySprites, 0x100);
}

void nes::PPU::load(uint8_t*& buffer) {
    uint8_t controlFlags = 0x00;
    uint8_t maskFlags = 0x00;
    uint8_t statusFlags = 0x00;

    nes::read(buffer, pixelX);
    nes::read(buffer, pixelY);
    nes::read(buffer, bufferData);
    nes::read(buffer, controlFlags);
    nes::read(buffer, maskFlags);
    nes::read(buffer, statusFlags);
    nes::read(buffer, registerDecay);
    nes::read(buffer, registerVDelayed);
    nes::read(buffer, delayDataRead);
    nes::read(buffer, delayDataWrite);
    nes::read(buffer, registerT);
    nes::read(buffer, registerV);
    nes::read(buffer, offsetX);
    nes::read(buffer, foregroundDataPointer);
    nes::read(buffer, foregroundSpriteCount);
    nes::read(buffer, foregroundSpriteCountNext);
    nes::read(buffer, foregroundSpritePointer);
    nes::read(buffer, foregroundReadDelay);
    nes::read(buffer, foregroundOffset);
    nes::read(buffer, foregroundEvaluationStep);
    nes::read(buffer, backgroundShifter, 0x4);
    nes::read(buffer, clockDecays, 0x3);
    nes::read(buffer, backgroundData, 0x4);
    nes::read(buffer, foregroundData, 0x20);
    nes::read(buffer, foregroundShifter, 0x10);
    nes::read(buffer, foregroundAttributes, 0x8);
    nes::read(buffer, foregroundPositions, 0x8);
    nes::read(buffer, memoryVideo, 0x1000);
    nes::read(buffer, memoryPalette, 0x20);
    nes::read(buffer, memorySprites, 0x100);

    controlIncrementMode = controlFlags & 0x01;
    controlForegroundTable = controlFlags & 0x02;
    controlBackgroundTable = controlFlags & 0x04;
    controlForegroundLarge = controlFlags & 0x08;
    controlInteruptOnVBL = controlFlags & 0x10;
    foregroundSprite0Line = controlFlags & 0x20;
    foregroundSprite0Should = controlFlags & 0x40;
    foregroundSprite0Hit = controlFlags & 0x80;

    maskGreyscaleMode = maskFlags & 0x01;
    maskRenderBackgroundLeft = maskFlags & 0x02;
    maskRenderForegroundLeft = maskFlags & 0x04;
    maskRenderBackground = maskFlags & 0x08;
    maskRenderForeground = maskFlags & 0x10;
    maskColorEmphasize = maskFlags >> 5;

    sendNMI = statusFlags & 0x01;
    preventNMI = statusFlags & 0x02;
    addressLatch = statusFlags & 0x04;
    cycleLatch = statusFlags & 0x08;
    frameReady = statusFlags & 0x10;
    statusSpriteOverflow = statusFlags & 0x20;
    statusSprite0Hit = statusFlags & 0x40;
    statusVerticalBlank = statusFlags & 0x80;
}

void nes::PPU::internalWrite(uint16_t address, uint8_t value) {
    address &= 0x3FFF;

    if (address < 0x2000) {
        mapper.notify(address, pixelY * 341 + pixelX);
        mapper.writePPU(address, value);
    } else if (address < 0x3F00) {
        memoryVideo[mapper.getMirroredAddress(address)] = value;
    } else {
        address &= 0x001F;

        if (address == 0x0010) {
            address = 0x0000;
        } else if (address == 0x0014) {
            address = 0x0004;
        } else if (address == 0x0018) {
            address = 0x0008;
        } else if (address == 0x001C) {
            address = 0x000C;
        }

        memoryPalette[address] = value;
    }
}

uint8_t nes::PPU::internalRead(uint16_t address) {
    address &= 0x3FFF;

    if (address < 0x2000) {
        mapper.notify(address, pixelY * 341 + pixelX);

        return mapper.readPPU(address);
    } else if (address < 0x3F00) {
        return memoryVideo[mapper.getMirroredAddress(address)];
    } else {
        address &= 0x001F;

        if (address == 0x0010) {
            address = 0x0000;
        } else if (address == 0x0014) {
            address = 0x0004;
        } else if (address == 0x0018) {
            address = 0x0008;
        } else if (address == 0x001C) {
            address = 0x000C;
        }

        return memoryPalette[address];
    }
}

void nes::PPU::incrementScrollX() {
    if (maskRenderBackground || maskRenderForeground) {
        if ((registerV & 0x001F) == 0x1F) {
            registerV &= 0xFFE0;
            registerV ^= 0x0400;
        } else {
            registerV++;
        }
    }
}

void nes::PPU::incrementScrollY() {
    if (maskRenderBackground || maskRenderForeground) {
        if ((registerV & 0x7000) != 0x7000) {
            registerV += 0x1000;
        } else {
            registerV &= 0x8FFF;

            uint8_t coarseY = (registerV & 0x03E0) >> 5;

            if (coarseY == 0x1D) {
                coarseY = 0;
                registerV ^= 0x0800;
            } else if (((registerV >> 5) & 0x1F) == 0x1F) {
                coarseY = 0;
            } else {
                coarseY++;
            }

            registerV = (registerV & 0xFC1F) | (coarseY << 5);
        }
    }
}

void nes::PPU::resetScrollX() {
    if (maskRenderBackground || maskRenderForeground) {
        registerV &= 0xFBE0;
        registerV |= registerT & 0x041F;
    }
}

void nes::PPU::resetScrollY() {
    if (maskRenderBackground || maskRenderForeground) {
        registerV &= 0x841F;
        registerV |= registerT & 0x7BE0;
    }
}

void nes::PPU::fetchBackgroundData() {
    updateBackgroundShifter();

    switch ((pixelX - 1) & 0x07) {
    case 0x0: {
        backgroundShifter[0] = (backgroundShifter[0] & 0xFF00) | backgroundData[2];
        backgroundShifter[1] = (backgroundShifter[1] & 0xFF00) | backgroundData[3];

        if (backgroundData[1] & 0x01) {
            backgroundShifter[2] = (backgroundShifter[2] & 0xFF00) | 0xFF;
        } else {
            backgroundShifter[2] = (backgroundShifter[2] & 0xFF00);
        }

        if (backgroundData[1] & 0x02) {
            backgroundShifter[3] = (backgroundShifter[3] & 0xFF00) | 0xFF;
        } else {
            backgroundShifter[3] = (backgroundShifter[3] & 0xFF00);
        }

        backgroundData[0] = internalRead(0x2000 | (registerV & 0x0FFF));

        break;
    }

    case 0x2: {
        backgroundData[1] = internalRead(0x23C0 | (registerV & 0x0C00) | ((registerV >> 4) & 0x38) | ((registerV >> 2) & 0x07));

        if (registerV & 0x0040) {
            backgroundData[1] >>= 4;
        }

        if (registerV & 0x0002) {
            backgroundData[1] >>= 2;
        }

        backgroundData[1] &= 0x03;

        break;
    }

    case 0x4: {
        uint16_t address = controlBackgroundTable << 12;

        address |= backgroundData[0] << 4;
        address |= (registerV >> 12) & 0x07;

        backgroundData[2] = internalRead(address);

        break;
    }

    case 0x6: {
        uint16_t address = controlBackgroundTable << 12;

        address |= backgroundData[0] << 4;
        address |= 8 + ((registerV >> 12) & 0x07);

        backgroundData[3] = internalRead(address);

        break;
    }

    case 0x7: incrementScrollX(); break;
    }
}

void nes::PPU::updateBackgroundShifter() {
    if (maskRenderBackground) {
        backgroundShifter[0] <<= 1;
        backgroundShifter[1] <<= 1;
        backgroundShifter[2] <<= 1;
        backgroundShifter[3] <<= 1;
    }
}

void nes::PPU::resetForegroundData() {
    foregroundSpriteCountNext = foregroundSpriteCount;

    foregroundDataPointer = 0;
    foregroundSpriteCount = 0;
    foregroundEvaluationStep = 0x0;
    foregroundSprite0Line = foregroundSprite0Should;
    foregroundSprite0Should = false;
    foregroundSprite0Hit = false;
}

void nes::PPU::clearForegroundData() {
    if (pixelX & 0x01) {
        foregroundData[foregroundDataPointer++] = 0xFF;

        foregroundDataPointer &= 0x1F;
    }
}

void nes::PPU::fetchForegroundData() {
    if (pixelX % 2 == 0 && (maskRenderBackground || maskRenderForeground)) {
        uint8_t spriteSize = controlForegroundLarge ? 16 : 8;

        switch (foregroundEvaluationStep) {
        case 0x0: {
            foregroundData[foregroundSpriteCount * 4 + (foregroundSpritePointer & 0x03)] = memorySprites[foregroundSpritePointer];

            if (!(foregroundSpritePointer & 0x3)) {
                int16_t offsetY = int16_t(pixelY) - int16_t(memorySprites[foregroundSpritePointer]);

                if (offsetY >= 0 && offsetY < spriteSize) {
                    if (!foregroundSpritePointer++) {
                        foregroundSprite0Should = true;
                    }
                } else {
                    foregroundSpritePointer += 4;

                    if (!foregroundSpritePointer) {
                        foregroundEvaluationStep = 0x2;
                    } else if (foregroundSpriteCount == 8) {
                        foregroundEvaluationStep = 0x1;
                    }
                }
            } else if (!(++foregroundSpritePointer & 0x03)) {
                foregroundSpriteCount++;

                if (!foregroundSpritePointer) {
                    foregroundEvaluationStep = 0x2;
                } else if (foregroundSpriteCount == 8) {
                    foregroundEvaluationStep = 0x1;
                }
            }

            break;
        }

        case 0x1: {
            if (foregroundReadDelay) {
                foregroundReadDelay--;
            } else {
                int16_t offsetY = int16_t(pixelY) - int16_t(memorySprites[foregroundSpritePointer]);

                if (offsetY >= 0 && offsetY < spriteSize) {
                    statusSpriteOverflow = true;

                    foregroundSpritePointer++;
                    foregroundReadDelay = 3;
                } else {
                    uint8_t low = (foregroundSpritePointer + 1) & 0x03;

                    foregroundSpritePointer += 0x04;
                    foregroundSpritePointer &= 0xFC;

                    if (!foregroundSpritePointer) {
                        foregroundEvaluationStep = 0x2;
                    }

                    foregroundSpritePointer |= low;
                }
            }

            break;
        }

        default: foregroundSpritePointer = 0;
        }
    }
}

void nes::PPU::loadForegroundShifter() {
    foregroundSpritePointer = 0;

    if (pixelX == 257) {
        foregroundDataPointer = 0;
    }

    if (foregroundDataPointer == foregroundSpriteCount) {
        // return;
    }

    switch ((pixelX - 1) & 0x7) {
    case 0x4: {
        foregroundOffset = pixelY - foregroundData[foregroundDataPointer * 4];

        break;
    }

    case 0x5: {
        uint16_t addressPattern = 0x0000;

        if (controlForegroundLarge) {
            addressPattern = (foregroundData[foregroundDataPointer * 4 + 1] & 0x01) << 12;

            if (foregroundData[foregroundDataPointer * 4 + 2] & 0x80) {
                if (foregroundOffset < 8) {
                    addressPattern |= ((foregroundData[foregroundDataPointer * 4 + 1] & 0xFE) + 1) << 4;
                } else {
                    addressPattern |= ((foregroundData[foregroundDataPointer * 4 + 1] & 0xFE)) << 4;
                }

                addressPattern |= (7 - foregroundOffset) & 0x07;
            } else {
                if (foregroundOffset < 8) {
                    addressPattern |= ((foregroundData[foregroundDataPointer * 4 + 1] & 0xFE)) << 4;
                } else {
                    addressPattern |= ((foregroundData[foregroundDataPointer * 4 + 1] & 0xFE) + 1) << 4;
                }

                addressPattern |= foregroundOffset & 0x07;
            }
        } else {
            addressPattern = controlForegroundTable << 12 | foregroundData[foregroundDataPointer * 4 + 1] << 4;

            if (foregroundData[foregroundDataPointer * 4 + 2] & 0x80) {
                addressPattern |= 7 - foregroundOffset;
            } else {
                addressPattern |= foregroundOffset;
            }
        }

        uint8_t spritePatternLSBPlane = internalRead(addressPattern);
        uint8_t spritePatternMSBPlane = internalRead(addressPattern + 8);

        if (foregroundData[foregroundDataPointer * 4 + 2] & 0x40) {
            spritePatternLSBPlane = (spritePatternLSBPlane & 0xF0) >> 4 | (spritePatternLSBPlane & 0x0F) << 4;
            spritePatternLSBPlane = (spritePatternLSBPlane & 0xCC) >> 2 | (spritePatternLSBPlane & 0x33) << 2;
            spritePatternLSBPlane = (spritePatternLSBPlane & 0xAA) >> 1 | (spritePatternLSBPlane & 0x55) << 1;

            spritePatternMSBPlane = (spritePatternMSBPlane & 0xF0) >> 4 | (spritePatternMSBPlane & 0x0F) << 4;
            spritePatternMSBPlane = (spritePatternMSBPlane & 0xCC) >> 2 | (spritePatternMSBPlane & 0x33) << 2;
            spritePatternMSBPlane = (spritePatternMSBPlane & 0xAA) >> 1 | (spritePatternMSBPlane & 0x55) << 1;
        }

        foregroundShifter[foregroundDataPointer * 2] = spritePatternLSBPlane;
        foregroundShifter[foregroundDataPointer * 2 + 1] = spritePatternMSBPlane;

        break;
    }

    case 0x6: {
        foregroundAttributes[foregroundDataPointer] = foregroundData[foregroundDataPointer * 4 + 2];

        break;
    }

    case 0x7: {
        foregroundPositions[foregroundDataPointer] = foregroundData[foregroundDataPointer * 4 + 3];

        foregroundDataPointer++;

        break;
    }
    }
}

void nes::PPU::updateForegroundShifter() {
    if (maskRenderForeground) {
        for (uint8_t sprite = 0; sprite < foregroundSpriteCountNext; sprite++) {
            if (foregroundPositions[sprite] > 0) {
                foregroundPositions[sprite] --;
            } else {
                foregroundShifter[sprite * 2] <<= 1;
                foregroundShifter[sprite * 2 + 1] <<= 1;
            }
        }
    }
}

uint8_t nes::PPU::blend() {
    uint8_t backgroundPixel = 0x00;
    uint8_t backgroundPalette = 0x00;

    if (maskRenderBackground && (pixelX > 8 || maskRenderBackgroundLeft)) {
        uint16_t bitMask = 0x8000 >> offsetX;

        backgroundPixel = ((backgroundShifter[0] & bitMask) > 0) | (((backgroundShifter[1] & bitMask) > 0) << 1);
        backgroundPalette = ((backgroundShifter[2] & bitMask) > 0) | (((backgroundShifter[3] & bitMask) > 0) << 1);
    }

    uint8_t foregroundPixel = 0x00;
    uint8_t foregroundPalette = 0x00;
    uint8_t foregroundPriority = 0x00;

    if (maskRenderForeground && (pixelX > 8 || maskRenderForegroundLeft)) {
        foregroundSprite0Hit = false;

        for (uint8_t sprite = 0; sprite < foregroundSpriteCountNext; sprite++) {
            if (foregroundPositions[sprite] == 0) {
                foregroundPixel = ((foregroundShifter[sprite * 2] & 0x80) > 0) | (((foregroundShifter[sprite * 2 + 1] & 0x80) > 0) << 1);
                foregroundPalette = (foregroundAttributes[sprite] & 0x03) + 0x04;
                foregroundPriority = (foregroundAttributes[sprite] & 0x20) == 0x00;

                if (foregroundPixel != 0) {
                    if (sprite == 0) {
                        foregroundSprite0Hit = true;
                    }

                    break;
                }
            }
        }
    }

    uint8_t finalPixel = 0x00;
    uint8_t finalPalette = 0x00;

    if (!maskRenderBackground && !maskRenderForeground && (registerV & 0x3FFF) >= 0x3F00) {
        return registerV & 0x1F;
    }

    if (pixelX > 0 && pixelX < 258) {
        if (backgroundPixel == 0 && foregroundPixel > 0) {
            finalPixel = foregroundPixel;
            finalPalette = foregroundPalette;
        } else if (backgroundPixel > 0 && foregroundPixel == 0) {
            finalPixel = backgroundPixel;
            finalPalette = backgroundPalette;
        } else if (backgroundPixel > 0 && foregroundPixel > 0) {
            if (foregroundPriority) {
                finalPixel = foregroundPixel;
                finalPalette = foregroundPalette;
            } else {
                finalPixel = backgroundPixel;
                finalPalette = backgroundPalette;
            }

            if (foregroundSprite0Hit && foregroundSprite0Line && (pixelX > 8 || maskRenderBackgroundLeft || maskRenderForegroundLeft)) {
                statusSprite0Hit = true;
            }
        }
    }

    finalPixel |= finalPalette << 2;

    if (maskGreyscaleMode) {
        finalPixel &= 0x30;
    }

    return finalPixel;
}

nes::CPU::CPU(Mapper& mapper, PPU& ppu) : mapper(mapper), ppu(ppu) {
    registerA = 0x00;
    registerX = 0x00;
    registerY = 0x00;

    status = 0x34;

    stackPointer = 0xFD;
    controllerShifter = 0x00;

    programCounter = read(0xFFFC) | (read(0xFFFD) << 8);

    frozen = false;

    for (int k = 0; k < 5; k++) {
        internalTick();
    }
}

nes::CPU::~CPU() { }

void nes::CPU::tick() {
    if (frozen) {
        return;
    }

    if (spriteTransferActive) {
        internalTick();

        if (!spriteTransferSynchronized) {
            if (cycleLatch) {
                spriteTransferSynchronized = true;
            }
        } else {
            if (!cycleLatch) {
                spriteTransferValue = silentRead(spriteTransferAddress);
            } else {
                ppu.writeDMA(spriteTransferValue);

                if ((++spriteTransferAddress & 0x00FF) == 0) {
                    spriteTransferActive = false;
                    spriteTransferSynchronized = false;
                }
            }
        }
    } else {
        uint8_t instruction = fetch();

        (this->*INSTRUCTION_SET[instruction][1])();
        (this->*INSTRUCTION_SET[instruction][0])();
    }

    if (ppu.shouldNMI()) {
        interrupt(true);
    }

    if (mapper.shouldIRQ()) {
        interrupt(false);
    }

    cycleLatch = !cycleLatch;
}

void nes::CPU::reset() {
    programCounter = read(0xFFFC) | (read(0xFFFD) << 8);

    status |= Flag::I;

    stackPointer -= 3;

    frozen = false;

    for (int k = 0; k < 5; k++) {
        internalTick();
    }
}

void nes::CPU::interrupt(bool nmi) {
    if (nmi || !getStatus(Flag::I)) {
        write(0x100 | stackPointer--, programCounter >> 8);
        write(0x100 | stackPointer--, programCounter & 0x00FF);

        setStatus(Flag::B, 0);
        setStatus(Flag::U, 1);

        write(0x100 | stackPointer--, status);

        setStatus(Flag::I, 1);

        targetAddress = nmi ? 0xFFFA : 0xFFFE;

        for (int k = 0; k < 2; k++) {
            internalTick();
        }

        programCounter = read(targetAddress);
        programCounter |= read(targetAddress + 1) << 8;
    }
}

void nes::CPU::silentWrite(uint16_t address, uint8_t value) {
    if (address < 0x2000) {
        memory[address & 0x7FF] = value;
    } else if (address >= 0x6000 && address < 0x8000) {
        mapper.writeCPU(address, value);
    }
}

uint8_t nes::CPU::silentRead(uint16_t address) {
    if (address < 0x2000) {
        return memory[address & 0x7FF];
    } else if (address >= 0x6000 && address < 0x8000) {
        return mapper.readCPU(address);
    }
    
    return 0x00;
}

void nes::CPU::setControllerState(uint8_t state) {
    controllerShifter = state;
}

bool nes::CPU::isFrozen() {
    return frozen;
}

bool nes::CPU::isPollingController() {
    return pollingController;
}

void nes::CPU::dump(uint8_t*& buffer) {
    uint8_t flags = frozen | (cycleLatch << 1) | (spriteTransferActive << 2) | (spriteTransferSynchronized << 3);

    nes::write(buffer, registerA);
    nes::write(buffer, registerX);
    nes::write(buffer, registerY);
    nes::write(buffer, status);
    nes::write(buffer, stackPointer);
    nes::write(buffer, programCounter);
    nes::write(buffer, controllerShifter);
    nes::write(buffer, spriteTransferValue);
    nes::write(buffer, spriteTransferAddress);
    nes::write(buffer, flags);
    nes::write(buffer, memory, 0x800);
}

void nes::CPU::load(uint8_t*& buffer) {
    uint8_t flags = 0x00;

    nes::read(buffer, registerA);
    nes::read(buffer, registerX);
    nes::read(buffer, registerY);
    nes::read(buffer, status);
    nes::read(buffer, stackPointer);
    nes::read(buffer, programCounter);
    nes::read(buffer, controllerShifter);
    nes::read(buffer, spriteTransferValue);
    nes::read(buffer, spriteTransferAddress);
    nes::read(buffer, flags);
    nes::read(buffer, memory, 0x800);

    frozen = flags & 0x1;
    cycleLatch = flags & 0x2;
    spriteTransferActive = flags & 0x4;
    spriteTransferSynchronized = flags & 0x8;
}

void nes::CPU::internalTick() {
    // TODO tick APU one time

    for (int k = 0; k < 3; k++) {
        ppu.tick();
    }
}

uint8_t nes::CPU::fetch() {
    return read(programCounter++);
}

void nes::CPU::write(uint16_t address, uint8_t value) {
    ppu.tick();
    ppu.tick();

    if (address < 0x2000) {
        memory[address & 0x7FF] = value;
    } else if (address < 0x4000) {
        ppu.write(address & 0x0007, value);
    } else if (address == 0x4014) {
        spriteTransferAddress = value << 8;
        spriteTransferActive = true;
    } else if (address == 0x4016) {
        pollingController = value & 0x01;
    } else if (address < 0x4018) {
        //TODO
    } else {
        mapper.writeCPU(address, value);
    }

    ppu.tick();
}

uint8_t nes::CPU::read(uint16_t address) {
    ppu.tick();

    uint8_t value = 0x00;

    if (address < 0x2000) {
        value = memory[address & 0x7FF];
    } else if (address < 0x4000) {
        value = ppu.read(address & 0x0007);
    } else if (address == 0x4016) {
        value = (controllerShifter & 0x80) > 0;

        controllerShifter <<= 1;
    } else if (address < 0x4018) {
        // TODO
    } else {
        value = mapper.readCPU(address);
    }

    ppu.tick();
    ppu.tick();

    return value;
}

void nes::CPU::setStatus(uint8_t flag, bool value) {
    if (value) {
        status |= flag;
    } else {
        status &= ~flag;
    }
}

bool nes::CPU::getStatus(uint8_t flag) {
    return status & flag;
}

void nes::CPU::ABR() {
    ABW();

    registerM = read(targetAddress);
}

void nes::CPU::ABW() {
    targetAddress = fetch();
    targetAddress |= fetch() << 8;
}

void nes::CPU::ACC() {
    registerM = read(programCounter);
}

void nes::CPU::AXM() {
    AXW();

    registerM = read(targetAddress);
}

void nes::CPU::AXR() {
    targetAddress = fetch();

    uint16_t translated = targetAddress + registerX;

    bool invalidAddress = (targetAddress & 0xFF00) != (translated & 0xFF00);

    targetAddress = translated & 0x00FF;
    targetAddress |= fetch() << 8;

    registerM = read(targetAddress);

    if (invalidAddress) {
        targetAddress += 0x100;

        registerM = read(targetAddress);
    }
}

void nes::CPU::AXW() {
    targetAddress = fetch();

    uint16_t translated = targetAddress + registerX;

    bool invalidAddress = (targetAddress & 0xFF00) != (translated & 0xFF00);

    targetAddress = translated & 0x00FF;
    targetAddress |= fetch() << 8;

    registerM = read(targetAddress);

    if (invalidAddress) {
        targetAddress += 0x100;
    }
}

void nes::CPU::AYM() {
    AYW();

    registerM = read(targetAddress);
}

void nes::CPU::AYR() {
    targetAddress = fetch();

    uint16_t translated = targetAddress + registerY;

    bool invalidAddress = (targetAddress & 0xFF00) != (translated & 0xFF00);

    targetAddress = translated & 0x00FF;
    targetAddress |= fetch() << 8;

    registerM = read(targetAddress);

    if (invalidAddress) {
        targetAddress += 0x100;

        registerM = read(targetAddress);
    }
}

void nes::CPU::AYW() {
    targetAddress = fetch();

    uint16_t translated = targetAddress + registerY;

    bool invalidAddress = (targetAddress & 0xFF00) != (translated & 0xFF00);

    targetAddress = translated & 0x00FF;
    targetAddress |= fetch() << 8;

    registerM = read(targetAddress);

    if (invalidAddress) {
        targetAddress += 0x100;
    }
}

void nes::CPU::IMM() {
    registerM = fetch();
}

void nes::CPU::IMP() {
    registerM = fetch();

    setStatus(Flag::C, false);
}

void nes::CPU::IND() {
    uint16_t pointer = fetch();

    pointer |= fetch() << 8;

    if ((pointer & 0x00FF) == 0xFF) {
        targetAddress = read(pointer);
        targetAddress |= read(pointer & 0xFF00) << 8;
    } else {
        targetAddress = read(pointer);
        targetAddress |= read(pointer + 1) << 8;
    }
}

void nes::CPU::IXR() {
    IXW();

    registerM = read(targetAddress);
}

void nes::CPU::IXW() {
    uint8_t pointer = fetch();

    registerM = read(pointer);

    pointer += registerX;

    targetAddress = read(pointer);
    targetAddress |= read(++pointer & 0xFF) << 8;
}

void nes::CPU::IYM() {
    IYW();

    registerM = read(targetAddress);
}

void nes::CPU::IYR() {
    uint8_t pointer = fetch();

    targetAddress = read(pointer);

    uint16_t translated = targetAddress + registerY;

    bool invalidAddress = translated & 0xFF00;

    targetAddress = translated & 0x00FF;
    targetAddress |= read(++pointer & 0xFF) << 8;

    registerM = read(targetAddress);

    if (invalidAddress) {
        targetAddress += 0x100;

        registerM = read(targetAddress);
    }
}

void nes::CPU::IYW() {
    uint8_t pointer = fetch();

    targetAddress = read(pointer);

    uint16_t translated = targetAddress + registerY;

    bool invalidAddress = (targetAddress & 0xFF00) != (translated & 0xFF00);

    targetAddress = translated & 0x00FF;
    targetAddress |= read(++pointer & 0xFF) << 8;

    registerM = read(targetAddress);

    if (invalidAddress) {
        targetAddress += 0x100;
    }
}

void nes::CPU::REL() {
    targetAddress = fetch();

    if (targetAddress & 0x80) {
        targetAddress |= 0xFF00;
    }
}

void nes::CPU::ZPR() {
    ZPW();

    registerM = read(targetAddress);
}

void nes::CPU::ZPW() {
    targetAddress = fetch();
}

void nes::CPU::ZXR() {
    ZXW();

    registerM = read(targetAddress);
}

void nes::CPU::ZXW() {
    targetAddress = fetch();

    registerM = read(targetAddress);

    targetAddress += registerX;
    targetAddress &= 0x00FF;
}

void nes::CPU::ZYR() {
    ZYW();

    registerM = read(targetAddress);
}

void nes::CPU::ZYW() {
    targetAddress = fetch();

    registerM = read(targetAddress);

    targetAddress += registerY;
    targetAddress &= 0x00FF;
}

void nes::CPU::AAL() {
    setStatus(Flag::C, registerA & 0x80);

    registerA <<= 1;

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);
}

void nes::CPU::ADC() {
    uint16_t result = registerA + registerM + getStatus(Flag::C);

    setStatus(Flag::C, result & 0x0100);
    setStatus(Flag::V, ~(registerA ^ registerM) & (registerA ^ result) & 0x80);

    registerA = result & 0x00FF;

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);
}

void nes::CPU::ALR() {
    registerA &= registerM;

    setStatus(Flag::C, registerA & 0x01);

    registerA >>= 1;

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);
}

void nes::CPU::ANC() {
    registerA &= registerM;

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);
    setStatus(Flag::C, registerA & 0x80);
}

void nes::CPU::AND() {
    registerA &= registerM;

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);
}

void nes::CPU::ANE() {
    registerA = (registerA | 0xEE) & registerX & registerM;
}

void nes::CPU::ARR() {
    registerA &= registerM;

    registerA = (getStatus(Flag::C) << 7) | (registerA >> 1);

    setStatus(Flag::C, registerA & 0x40);
    setStatus(Flag::V, bool(registerA & 0x40) ^ bool(registerA & 0x20));
    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);
}

void nes::CPU::ASL() {
    write(targetAddress, registerM);

    setStatus(Flag::C, registerM & 0x80);

    registerM <<= 1;

    setStatus(Flag::Z, !registerM);
    setStatus(Flag::N, registerM & 0x80);

    write(targetAddress, registerM);
}

void nes::CPU::BCC() {
    if (!getStatus(Flag::C)) {
        read(programCounter);

        uint16_t translated = targetAddress + programCounter;

        if ((translated & 0xFF00) != (programCounter & 0xFF00)) {
            read(programCounter);
        }

        programCounter = translated;
    }
}

void nes::CPU::BCS() {
    if (getStatus(Flag::C)) {
        read(programCounter);

        uint16_t translated = targetAddress + programCounter;

        if ((translated & 0xFF00) != (programCounter & 0xFF00)) {
            read(programCounter);
        }

        programCounter = translated;
    }
}

void nes::CPU::BEQ() {
    if (getStatus(Flag::Z)) {
        read(programCounter);

        uint16_t translated = targetAddress + programCounter;

        if ((translated & 0xFF00) != (programCounter & 0xFF00)) {
            read(programCounter);
        }

        programCounter = translated;
    }
}

void nes::CPU::BIT() {
    setStatus(Flag::Z, !(registerA & registerM));
    setStatus(Flag::V, registerM & 0x40);
    setStatus(Flag::N, registerM & 0x80);
}

void nes::CPU::BMI() {
    if (getStatus(Flag::N)) {
        read(programCounter);

        uint16_t translated = targetAddress + programCounter;

        if ((translated & 0xFF00) != (programCounter & 0xFF00)) {
            read(programCounter);
        }

        programCounter = translated;
    }
}

void nes::CPU::BNE() {
    if (!getStatus(Flag::Z)) {
        read(programCounter);

        uint16_t translated = targetAddress + programCounter;

        if ((translated & 0xFF00) != (programCounter & 0xFF00)) {
            read(programCounter);
        }

        programCounter = translated;
    }
}

void nes::CPU::BPL() {
    if (!getStatus(Flag::N)) {
        read(programCounter);

        uint16_t translated = targetAddress + programCounter;

        if ((translated & 0xFF00) != (programCounter & 0xFF00)) {
            read(programCounter);
        }

        programCounter = translated;
    }
}

void nes::CPU::BRK() {
    programCounter++;

    write(0x100 | stackPointer--, programCounter >> 8);
    write(0x100 | stackPointer--, programCounter & 0x00FF);

    setStatus(Flag::I, 1);

    write(0x100 | stackPointer--, status | Flag::B);

    programCounter = read(0xFFFE);
    programCounter |= read(0xFFFF) << 8;
}

void nes::CPU::BVC() {
    if (!getStatus(Flag::V)) {
        read(programCounter);

        uint16_t translated = targetAddress + programCounter;

        programCounter = programCounter & 0xFF00 | ((programCounter & 0x00FF) + (targetAddress & 0x00FF)) & 0xFF;

        if (translated != programCounter) {
            read(programCounter);

            programCounter = translated;
        }
    }
}

void nes::CPU::BVS() {
    if (getStatus(Flag::V)) {
        read(programCounter);

        uint16_t translated = targetAddress + programCounter;

        programCounter = programCounter & 0xFF00 | ((programCounter & 0x00FF) + (targetAddress & 0x00FF)) & 0xFF;

        if (translated != programCounter) {
            read(programCounter);

            programCounter = translated;
        }
    }
}

void nes::CPU::CLC() {
    setStatus(Flag::C, 0);
}

void nes::CPU::CLD() {
    setStatus(Flag::D, 0);
}

void nes::CPU::CLI() {
    setStatus(Flag::I, 0);
}

void nes::CPU::CLV() {
    setStatus(Flag::V, 0);
}

void nes::CPU::CMP() {
    setStatus(Flag::C, registerA >= registerM);
    setStatus(Flag::Z, registerA == registerM);
    setStatus(Flag::N, (registerA - registerM) & 0x80);
}

void nes::CPU::CPX() {
    setStatus(Flag::C, registerX >= registerM);
    setStatus(Flag::Z, registerX == registerM);
    setStatus(Flag::N, (registerX - registerM) & 0x80);
}

void nes::CPU::CPY() {
    setStatus(Flag::C, registerY >= registerM);
    setStatus(Flag::Z, registerY == registerM);
    setStatus(Flag::N, (registerY - registerM) & 0x80);
}

void nes::CPU::DCP() {
    write(targetAddress, registerM);

    registerM--;

    setStatus(Flag::C, registerA >= registerM);
    setStatus(Flag::Z, registerA == registerM);
    setStatus(Flag::N, (registerA - registerM) & 0x80);

    write(targetAddress, registerM);
}

void nes::CPU::DEC() {
    write(targetAddress, registerM);

    registerM--;

    setStatus(Flag::Z, !registerM);
    setStatus(Flag::N, registerM & 0x80);

    write(targetAddress, registerM);
}

void nes::CPU::DEX() {
    registerX--;

    setStatus(Flag::Z, !registerX);
    setStatus(Flag::N, registerX & 0x80);
}

void nes::CPU::DEY() {
    registerY--;

    setStatus(Flag::Z, !registerY);
    setStatus(Flag::N, registerY & 0x80);
}

void nes::CPU::EOR() {
    registerA ^= registerM;

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);
}

void nes::CPU::INC() {
    write(targetAddress, registerM);

    registerM++;

    setStatus(Flag::Z, !registerM);
    setStatus(Flag::N, registerM & 0x80);

    write(targetAddress, registerM);
}

void nes::CPU::INX() {
    registerX++;

    setStatus(Flag::Z, !registerX);
    setStatus(Flag::N, registerX & 0x80);
}

void nes::CPU::INY() {
    registerY++;

    setStatus(Flag::Z, !registerY);
    setStatus(Flag::N, registerY & 0x80);
}

void nes::CPU::ISC() {
    write(targetAddress, registerM);

    registerM++;

    uint8_t value = registerM;

    registerM ^= 0xFF;

    uint16_t result = registerA + registerM + getStatus(Flag::C);

    setStatus(Flag::C, result & 0x0100);
    setStatus(Flag::V, ~(registerA ^ registerM) & (registerA ^ result) & 0x80);

    registerA = result & 0x00FF;

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);

    write(targetAddress, value);
}

void nes::CPU::JAM() {
    frozen = true;
}

void nes::CPU::JMP() {
    programCounter = targetAddress;
}

void nes::CPU::JSR() {
    internalTick();

    write(0x100 | stackPointer--, programCounter >> 8);
    write(0x100 | stackPointer--, programCounter & 0x00FF);

    programCounter = fetch() << 8;
    programCounter |= targetAddress;
}

void nes::CPU::LAR() {
    setStatus(Flag::C, registerA & 0x01);

    registerA >>= 1;

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);
}

void nes::CPU::LAS() {
    uint8_t result = registerM & stackPointer;

    registerA = result;
    registerX = result;
    stackPointer = result;
}

void nes::CPU::LAX() {
    registerA = registerM;
    registerX = registerM;

    setStatus(Flag::Z, !registerM);
    setStatus(Flag::N, registerM & 0x80);
}

void nes::CPU::LDA() {
    registerA = registerM;

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);
}

void nes::CPU::LDX() {
    registerX = registerM;

    setStatus(Flag::Z, !registerX);
    setStatus(Flag::N, registerX & 0x80);
}

void nes::CPU::LDY() {
    registerY = registerM;

    setStatus(Flag::Z, !registerY);
    setStatus(Flag::N, registerY & 0x80);
}

void nes::CPU::LSR() {
    write(targetAddress, registerM);

    setStatus(Flag::C, registerM & 0x01);

    registerM >>= 1;

    setStatus(Flag::Z, !registerM);
    setStatus(Flag::N, registerM & 0x80);

    write(targetAddress, registerM);
}

void nes::CPU::LXA() {
    registerA = registerM;
    registerX = registerM;

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);
}

void nes::CPU::NOP() { }

void nes::CPU::ORA() {
    registerA |= registerM;

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);
}

void nes::CPU::PHA() {
    write(0x100 | stackPointer--, registerA);
}

void nes::CPU::PHP() {
    write(0x100 | stackPointer--, status | Flag::B | Flag::U);
}

void nes::CPU::PLA() {
    stackPointer++;
    internalTick();

    registerA = read(0x100 | stackPointer);

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);
}

void nes::CPU::PLP() {
    stackPointer++;
    internalTick();

    status = read(0x100 | stackPointer);

    setStatus(Flag::B, 0);
    setStatus(Flag::U, 1);
}

void nes::CPU::RAL() {
    bool status = registerA & 0x80;

    registerA = uint8_t(getStatus(Flag::C)) | (registerA << 1);

    setStatus(Flag::C, status);
    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);
}

void nes::CPU::RAR() {
    bool status = registerA & 0x01;

    registerA = (getStatus(Flag::C) << 7) | (registerA >> 1);

    setStatus(Flag::C, status);
    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);
}

void nes::CPU::RLA() {
    write(targetAddress, registerM);

    bool status = registerM & 0x80;

    registerM = uint8_t(getStatus(Flag::C)) | (registerM << 1);

    setStatus(Flag::C, status);

    registerA &= registerM;

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);

    write(targetAddress, registerM);
}

void nes::CPU::ROL() {
    write(targetAddress, registerM);

    bool status = registerM & 0x80;

    registerM = uint8_t(getStatus(Flag::C)) | (registerM << 1);

    setStatus(Flag::C, status);
    setStatus(Flag::Z, !registerM);
    setStatus(Flag::N, registerM & 0x80);

    write(targetAddress, registerM);
}

void nes::CPU::ROR() {
    write(targetAddress, registerM);

    bool status = registerM & 0x01;

    registerM = (getStatus(Flag::C) << 7) | (registerM >> 1);

    setStatus(Flag::C, status);
    setStatus(Flag::Z, !registerM);
    setStatus(Flag::N, registerM & 0x80);

    write(targetAddress, registerM);
}

void nes::CPU::RRA() {
    write(targetAddress, registerM);

    bool status = registerM & 0x01;

    registerM = (getStatus(Flag::C) << 7) | (registerM >> 1);

    uint16_t result = registerA + registerM + status;

    setStatus(Flag::C, result & 0x0100);
    setStatus(Flag::V, ~(registerA ^ registerM) & (registerA ^ result) & 0x80);

    registerA = result & 0x00FF;

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);

    write(targetAddress, registerM);
}

void nes::CPU::RTI() {
    stackPointer++;
    internalTick();

    status = read(0x100 | stackPointer);

    setStatus(Flag::B, 0);
    setStatus(Flag::U, 1);

    programCounter = read(0x100 | ++stackPointer);
    programCounter |= read(0x100 | ++stackPointer) << 8;
}

void nes::CPU::RTS() {
    stackPointer++;
    internalTick();

    programCounter = read(0x100 | stackPointer);
    programCounter |= read(0x100 | ++stackPointer) << 8;

    programCounter++;

    internalTick();
}

void nes::CPU::SAX() {
    write(targetAddress, registerA & registerX);
}

void nes::CPU::SBC() {
    registerM ^= 0xFF;

    uint16_t result = registerA + registerM + getStatus(Flag::C);

    setStatus(Flag::C, result & 0x0100);
    setStatus(Flag::V, ~(registerA ^ registerM) & (registerA ^ result) & 0x80);

    registerA = result & 0x00FF;

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);
}

void nes::CPU::SBX() {
    registerX &= registerA;

    setStatus(Flag::C, registerX >= registerM);
    setStatus(Flag::Z, registerX == registerM);

    registerX -= registerM;

    setStatus(Flag::N, registerX & 0x80);
}

void nes::CPU::SEC() {
    setStatus(Flag::C, 1);
}

void nes::CPU::SED() {
    setStatus(Flag::D, 1);
}

void nes::CPU::SEI() {
    setStatus(Flag::I, 1);
}

void nes::CPU::SHA() {
    write(targetAddress, registerA & registerX & (uint8_t(targetAddress >> 8) + 1));
}

void nes::CPU::SHX() {
    uint8_t addressHigh = 1 + (targetAddress >> 8);

    write(((registerX & addressHigh) << 8) | targetAddress & 0xFF, registerX & addressHigh);
}

void nes::CPU::SHY() {
    uint8_t addressHigh = 1 + (targetAddress >> 8);

    write(((registerY & addressHigh) << 8) | targetAddress & 0xFF, registerY & addressHigh);
}

void nes::CPU::SLO() {
    write(targetAddress, registerM);

    setStatus(Flag::C, registerM & 0x80);

    registerM <<= 1;

    registerA |= registerM;

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);

    write(targetAddress, registerM);
}

void nes::CPU::SRE() {
    write(targetAddress, registerM);

    setStatus(Flag::C, registerM & 0x01);

    registerM >>= 1;

    registerA ^= registerM;

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);

    write(targetAddress, registerM);
}

void nes::CPU::STA() {
    write(targetAddress, registerA);
}

void nes::CPU::STX() {
    write(targetAddress, registerX);
}

void nes::CPU::STY() {
    write(targetAddress, registerY);
}

void nes::CPU::TAS() {
    stackPointer = registerA & registerX;

    write(targetAddress, stackPointer & (uint8_t(targetAddress >> 8) + 1));
}

void nes::CPU::TAX() {
    registerX = registerA;

    setStatus(Flag::Z, !registerX);
    setStatus(Flag::N, registerX & 0x80);
}

void nes::CPU::TAY() {
    registerY = registerA;

    setStatus(Flag::Z, !registerY);
    setStatus(Flag::N, registerY & 0x80);
}

void nes::CPU::TSX() {
    registerX = stackPointer;

    setStatus(Flag::Z, !registerX);
    setStatus(Flag::N, registerX & 0x80);
}

void nes::CPU::TXA() {
    registerA = registerX;

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);
}

void nes::CPU::TXS() {
    stackPointer = registerX;
}

void nes::CPU::TYA() {
    registerA = registerY;

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);
}

void nes::CPU::USB() {
    registerM ^= 0xFF;

    uint16_t result = registerA + registerM + getStatus(Flag::C);

    setStatus(Flag::C, result & 0x0100);
    setStatus(Flag::V, ~(registerA ^ registerM) & (registerA ^ result) & 0x80);

    registerA = result & 0x00FF;

    setStatus(Flag::Z, !registerA);
    setStatus(Flag::N, registerA & 0x80);
}

nes::NES::NES(Mapper* mapper) : mapper(mapper) {
    ppu = new PPU(*mapper);
    cpu = new CPU(*mapper, *ppu);
}

nes::NES::~NES() {
    if (running) {
        runningFrame.join();
    }

    delete cpu;
    delete ppu;
    delete mapper;
}

void nes::NES::waitFrame() {
    if(running) {
        runningFrame.join();

        running = false;
    }
}

void nes::NES::nextFrame(unsigned int frames) {
    auto run = [this](unsigned int f) -> void {
        for(unsigned int k = 0; k < f; k++) {
            while (!ppu->shouldRender() && !cpu->isFrozen()) {
                cpu->tick();

                if (cpu->isPollingController()) {
                    cpu->setControllerState(controllerState);
                }
            }
        }
    };

    runningFrame = std::thread(run, frames);

    running = true;
}

void nes::NES::setControllerState(uint8_t state) {
    controllerState = state;
}

void nes::NES::write(uint16_t address, uint8_t value) {
    cpu->silentWrite(address, value);
}

uint8_t nes::NES::read(uint16_t address) {
    return cpu->silentRead(address);
}

uint8_t* nes::NES::getFrameBuffer() {
    return ppu->getFrameBuffer();
}

bool nes::NES::isFrozen() {
    return cpu->isFrozen();
}

unsigned int nes::NES::size() {
    return cpu->size() + ppu->size() + mapper->size();
}

void nes::NES::dump(uint8_t*& buffer) {
    cpu->dump(buffer);
    ppu->dump(buffer);
    mapper->dump(buffer);
}

void nes::NES::load(uint8_t*& buffer) {
    cpu->load(buffer);
    ppu->load(buffer);
    mapper->load(buffer);
}