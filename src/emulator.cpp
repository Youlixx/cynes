// cynes - C/C++ NES emulator with Python bindings
// Copyright (C) 2021  Combey Theo <https://www.gnu.org/licenses/>

#include "emulator.h"

#include <iostream>


using namespace cynes;


NES::NES(const char* path) {
    _cpu = new CPU(*this);
    _ppu = new PPU(*this);
    _apu = new APU(*this);

    loadMapper(path);

    _cpu->power();
    _ppu->power();
    _apu->power();

    uint8_t paletteRamBootValues[0x20] = {
        0x09, 0x01, 0x00, 0x01, 0x00, 0x02, 0x02, 0x0D, 0x08, 0x10, 0x08, 0x24, 0x00, 0x00, 0x04, 0x2C,
        0x09, 0x01, 0x34, 0x03, 0x00, 0x04, 0x00, 0x14, 0x08, 0x3A, 0x00, 0x02, 0x00, 0x20, 0x2C, 0x08
    };

    memcpy(_memoryPalette, paletteRamBootValues, 0x20);

    memset(_memoryCPU, 0x00, 0x800);
    memset(_memoryOAM, 0x00, 0x100);
    memset(_memoryVideo, 0x00, 0x1000);
    memset(_memoryExtraRAM, 0x00, 0x2000);

    _controllerState = 0x00;
    _controllerShifter = 0x00;

    for (int i = 0; i < 8; i++) {
        dummyRead();
    }
}

NES::~NES() {
    delete _cpu;
    delete _ppu;
    delete _apu;

    delete _mapper;

    delete[] _memoryPRG;
    delete[] _memoryCHR;
}

CPU* NES::getCPU() {
    return _cpu;
}

PPU* NES::getPPU() {
    return _ppu;
}

APU* NES::getAPU() {
    return _apu;
}

Mapper* NES::getMapper() {
    return _mapper;
}

void NES::reset() {
    _cpu->reset();
    _ppu->reset();
    _apu->reset();

    for (int i = 0; i < 8; i++) {
        dummyRead();
    }
}

void NES::dummyRead() {
    _apu->tick(true);
    _ppu->tick();
    _ppu->tick();
    _ppu->tick();
    _cpu->poll();
}

void NES::write(uint16_t address, uint8_t value) {
    _apu->tick(false);
    _ppu->tick();
    _ppu->tick();

    writeCPU(address, value);

    _ppu->tick();
    _cpu->poll();
}

void NES::writeCPU(uint16_t address, uint8_t value) {
    if (address < 0x2000) {
        _memoryCPU[address & 0x7FF] = value;
    } else if (address < 0x4000) {
        _ppu->write(address & 0x7, value);
    } else if (address == 0x4016) {
        loadControllerShifter(value & 0x01);
    } else if (address < 0x4018) {
        _apu->write(address & 0xFF, value);
    } else if (address >= 0x6000 && address < 0x8000) {
        if (_mapper->isRAMWritable()) _memoryExtraRAM[address & 0x1FFF] = value;
    } else if (address >= 0x8000) {
        _mapper->writeCPU(address & 0x7FFF, value);
    }
}

void NES::writePPU(uint16_t address, uint8_t value) {
    address &= 0x3FFF;

    if (address < 0x2000) {
        _memoryCHR[_mapper->getAddressPPU(address & 0x1FFF)] = value;
    } else if (address < 0x3F00) {
        _memoryVideo[_mapper->getMirroredAddress(address)] = value;
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

void NES::writeOAM(uint8_t address, uint8_t value) {
    _memoryOAM[address] = value;
}

uint8_t NES::read(uint16_t address) {
    _apu->tick(true);
    _ppu->tick();
    _ppu->tick();

    uint8_t value = readCPU(address);

    _ppu->tick();
    _cpu->poll();

    return value;
}

uint8_t NES::readCPU(uint16_t address) {
    if (address < 0x2000) {
        return _memoryCPU[address & 0x7FF];
    } else if (address < 0x4000) {
        return _ppu->read(address & 0x7);
    } else if (address == 0x4016) {
        return pollController();
    } else if (address < 0x4018) {
        return _apu->read(address & 0xFF);
    } else if (address >= 0x6000 && address < 0x8000) {
        return _memoryExtraRAM[address & 0x1FFF];
    } else if (address >= 0x8000) {
        return _memoryPRG[_mapper->getAddressCPU(address & 0x7FFF)];
    }

    return 0x00;
}

uint8_t NES::readPPU(uint16_t address) {
    address &= 0x3FFF;

    if (address < 0x2000) {
        return _memoryCHR[_mapper->getAddressPPU(address & 0x1FFF)];
    } else if (address < 0x3F00) {
        return _memoryVideo[_mapper->getMirroredAddress(address)];
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

uint8_t NES::readOAM(uint8_t address) {
    return _memoryOAM[address];
}

void NES::loadMapper(const char* path) {
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

    _memoryPRG = new uint8_t[0x4000 * programBanks];

    for (int k = 0; k < 0x4000 * programBanks; k++) {
        _memoryPRG[k] = getc(stream);
    }

    if (characterBanks == 0) {
        _memoryCHR = new uint8_t[0x2000]{ 0 };
    } else {
        _memoryCHR = new uint8_t[0x2000 * characterBanks];

        for (int k = 0; k < 0x2000 * characterBanks; k++) {
            _memoryCHR[k] = getc(stream);
        }
    }

    fclose(stream);

    MirroringMode mode = mirroring ? MirroringMode::VERTICAL : MirroringMode::HORIZONTAL;

    switch (mapperId) {
    case 0: _mapper = new Mapper000(*this, programBanks, characterBanks, mode); return;
    case 1: _mapper = new Mapper001(*this, programBanks, characterBanks, mode); return;
    case 2: _mapper = new Mapper002(*this, programBanks, characterBanks, mode); return;
    case 3: _mapper = new Mapper003(*this, programBanks, characterBanks, mode); return;
    case 4: _mapper = new Mapper004(*this, programBanks, characterBanks, mode); return;
    default: throw std::runtime_error("The ROM Mapper is not supported.");
    }
}

bool NES::step(uint8_t* buffer, unsigned int frames) {
    for (unsigned int k = 0; k < frames; k++) {
        while (!_ppu->isFrameReady()) {
            _cpu->tick();

            if (_cpu->isFrozen()) {
                return true;
            }
        }
    }

    memcpy(buffer, _ppu->getFrameBuffer(), 0x2D000);

    return false;
}

void NES::setControllerState(uint8_t state) {
    _controllerState = state;
}

unsigned int NES::size() {
    unsigned int bufferSize = 0;

    dump<DumpOperation::SIZE>(bufferSize);

    return bufferSize;
}

void NES::save(uint8_t* buffer) {
    dump<DumpOperation::DUMP>(buffer);
}

void NES::load(uint8_t* buffer) {
    dump<DumpOperation::LOAD>(buffer);
}

void NES::loadControllerShifter(bool polling) {
    if (polling) {
        _controllerShifter = _controllerState;
    }
}

uint8_t NES::pollController() {
    uint8_t value = _controllerShifter >> 7;

    _controllerShifter <<= 1;

    return value;
}

template<DumpOperation operation, class T>
void NES::dump(T& buffer) {
    _cpu->dump<operation>(buffer);
    _ppu->dump<operation>(buffer);
    _apu->dump<operation>(buffer);

    _mapper->dump<operation>(buffer);

    cynes::dump<operation>(buffer, _memoryCPU);
    cynes::dump<operation>(buffer, _memoryOAM);
    cynes::dump<operation>(buffer, _memoryVideo);
    cynes::dump<operation>(buffer, _memoryPalette);
    cynes::dump<operation>(buffer, _memoryExtraRAM);

    cynes::dump<operation>(buffer, _controllerState);
    cynes::dump<operation>(buffer, _controllerShifter);
}

template void NES::dump<DumpOperation::SIZE>(unsigned int&);
template void NES::dump<DumpOperation::DUMP>(uint8_t*&);
template void NES::dump<DumpOperation::LOAD>(uint8_t*&);



CPU::CPU(NES& nes) : _nes(nes), _instructions(), _addressingModes() {
    _frozen = false;

    _targetAddress = 0x0000;
    _programCounter = 0x0000;

    _registerA = 0x00;
    _registerX = 0x00;
    _registerY = 0x00;
    _registerM = 0x00;
    _stackPointer = 0x00;

    _status = 0x00;

    _delayIRQ = false;
    _shouldIRQ = false;
    _lineMapperIRQ = false;
    _lineFrameIRQ = false;
    _lineDeltaIRQ = false;

    _edgeDetectorNMI = false;
    _delayNMI = false;
    _shouldNMI = false;
    _lineNMI = false;

    void (CPU:: * instructions[256]) (void) = {
        &CPU::BRK,&CPU::ORA,&CPU::JAM,&CPU::SLO,&CPU::NOP,&CPU::ORA,&CPU::ASL,&CPU::SLO,&CPU::PHP,&CPU::ORA,&CPU::AAL,&CPU::ANC,&CPU::NOP,&CPU::ORA,&CPU::ASL,&CPU::SLO,
        &CPU::BPL,&CPU::ORA,&CPU::JAM,&CPU::SLO,&CPU::NOP,&CPU::ORA,&CPU::ASL,&CPU::SLO,&CPU::CLC,&CPU::ORA,&CPU::NOP,&CPU::SLO,&CPU::NOP,&CPU::ORA,&CPU::ASL,&CPU::SLO,
        &CPU::JSR,&CPU::AND,&CPU::JAM,&CPU::RLA,&CPU::BIT,&CPU::AND,&CPU::ROL,&CPU::RLA,&CPU::PLP,&CPU::AND,&CPU::RAL,&CPU::ANC,&CPU::BIT,&CPU::AND,&CPU::ROL,&CPU::RLA,
        &CPU::BMI,&CPU::AND,&CPU::JAM,&CPU::RLA,&CPU::NOP,&CPU::AND,&CPU::ROL,&CPU::RLA,&CPU::SEC,&CPU::AND,&CPU::NOP,&CPU::RLA,&CPU::NOP,&CPU::AND,&CPU::ROL,&CPU::RLA,
        &CPU::RTI,&CPU::EOR,&CPU::JAM,&CPU::SRE,&CPU::NOP,&CPU::EOR,&CPU::LSR,&CPU::SRE,&CPU::PHA,&CPU::EOR,&CPU::LAR,&CPU::ALR,&CPU::JMP,&CPU::EOR,&CPU::LSR,&CPU::SRE,
        &CPU::BVC,&CPU::EOR,&CPU::JAM,&CPU::SRE,&CPU::NOP,&CPU::EOR,&CPU::LSR,&CPU::SRE,&CPU::CLI,&CPU::EOR,&CPU::NOP,&CPU::SRE,&CPU::NOP,&CPU::EOR,&CPU::LSR,&CPU::SRE,
        &CPU::RTS,&CPU::ADC,&CPU::JAM,&CPU::RRA,&CPU::NOP,&CPU::ADC,&CPU::ROR,&CPU::RRA,&CPU::PLA,&CPU::ADC,&CPU::RAR,&CPU::ARR,&CPU::JMP,&CPU::ADC,&CPU::ROR,&CPU::RRA,
        &CPU::BVS,&CPU::ADC,&CPU::JAM,&CPU::RRA,&CPU::NOP,&CPU::ADC,&CPU::ROR,&CPU::RRA,&CPU::SEI,&CPU::ADC,&CPU::NOP,&CPU::RRA,&CPU::NOP,&CPU::ADC,&CPU::ROR,&CPU::RRA,
        &CPU::NOP,&CPU::STA,&CPU::NOP,&CPU::SAX,&CPU::STY,&CPU::STA,&CPU::STX,&CPU::SAX,&CPU::DEY,&CPU::NOP,&CPU::TXA,&CPU::ANE,&CPU::STY,&CPU::STA,&CPU::STX,&CPU::SAX,
        &CPU::BCC,&CPU::STA,&CPU::JAM,&CPU::SHA,&CPU::STY,&CPU::STA,&CPU::STX,&CPU::SAX,&CPU::TYA,&CPU::STA,&CPU::TXS,&CPU::TAS,&CPU::SHY,&CPU::STA,&CPU::SHX,&CPU::SHA,
        &CPU::LDY,&CPU::LDA,&CPU::LDX,&CPU::LAX,&CPU::LDY,&CPU::LDA,&CPU::LDX,&CPU::LAX,&CPU::TAY,&CPU::LDA,&CPU::TAX,&CPU::LXA,&CPU::LDY,&CPU::LDA,&CPU::LDX,&CPU::LAX,
        &CPU::BCS,&CPU::LDA,&CPU::JAM,&CPU::LAX,&CPU::LDY,&CPU::LDA,&CPU::LDX,&CPU::LAX,&CPU::CLV,&CPU::LDA,&CPU::TSX,&CPU::LAS,&CPU::LDY,&CPU::LDA,&CPU::LDX,&CPU::LAX,
        &CPU::CPY,&CPU::CMP,&CPU::NOP,&CPU::DCP,&CPU::CPY,&CPU::CMP,&CPU::DEC,&CPU::DCP,&CPU::INY,&CPU::CMP,&CPU::DEX,&CPU::SBX,&CPU::CPY,&CPU::CMP,&CPU::DEC,&CPU::DCP,
        &CPU::BNE,&CPU::CMP,&CPU::JAM,&CPU::DCP,&CPU::NOP,&CPU::CMP,&CPU::DEC,&CPU::DCP,&CPU::CLD,&CPU::CMP,&CPU::NOP,&CPU::DCP,&CPU::NOP,&CPU::CMP,&CPU::DEC,&CPU::DCP,
        &CPU::CPX,&CPU::SBC,&CPU::NOP,&CPU::ISC,&CPU::CPX,&CPU::SBC,&CPU::INC,&CPU::ISC,&CPU::INX,&CPU::SBC,&CPU::NOP,&CPU::USB,&CPU::CPX,&CPU::SBC,&CPU::INC,&CPU::ISC,
        &CPU::BEQ,&CPU::SBC,&CPU::JAM,&CPU::ISC,&CPU::NOP,&CPU::SBC,&CPU::INC,&CPU::ISC,&CPU::SED,&CPU::SBC,&CPU::NOP,&CPU::ISC,&CPU::NOP,&CPU::SBC,&CPU::INC,&CPU::ISC
    };

    void (CPU:: * addressingModes[256]) (void) = {
        &CPU::IMP,&CPU::IXR,&CPU::ACC,&CPU::IXR,&CPU::ZPR,&CPU::ZPR,&CPU::ZPR,&CPU::ZPR,&CPU::IMP,&CPU::IMM,&CPU::ACC,&CPU::IMM,&CPU::ABR,&CPU::ABR,&CPU::ABR,&CPU::ABR,
        &CPU::REL,&CPU::IYR,&CPU::ACC,&CPU::IYM,&CPU::ZXR,&CPU::ZXR,&CPU::ZXR,&CPU::ZXR,&CPU::IMP,&CPU::AYR,&CPU::IMP,&CPU::AYM,&CPU::AXR,&CPU::AXR,&CPU::AXM,&CPU::AXM,
        &CPU::ABW,&CPU::IXR,&CPU::ACC,&CPU::IXR,&CPU::ZPR,&CPU::ZPR,&CPU::ZPR,&CPU::ZPR,&CPU::IMP,&CPU::IMM,&CPU::ACC,&CPU::IMM,&CPU::ABR,&CPU::ABR,&CPU::ABR,&CPU::ABR,
        &CPU::REL,&CPU::IYR,&CPU::ACC,&CPU::IYM,&CPU::ZXR,&CPU::ZXR,&CPU::ZXR,&CPU::ZXR,&CPU::IMP,&CPU::AYR,&CPU::IMP,&CPU::AYM,&CPU::AXR,&CPU::AXR,&CPU::AXM,&CPU::AXM,
        &CPU::IMP,&CPU::IXR,&CPU::ACC,&CPU::IXR,&CPU::ZPR,&CPU::ZPR,&CPU::ZPR,&CPU::ZPR,&CPU::IMP,&CPU::IMM,&CPU::ACC,&CPU::IMM,&CPU::ABW,&CPU::ABR,&CPU::ABR,&CPU::ABR,
        &CPU::REL,&CPU::IYR,&CPU::ACC,&CPU::IYM,&CPU::ZXR,&CPU::ZXR,&CPU::ZXR,&CPU::ZXR,&CPU::IMP,&CPU::AYR,&CPU::IMP,&CPU::AYM,&CPU::AXR,&CPU::AXR,&CPU::AXM,&CPU::AXM,
        &CPU::IMP,&CPU::IXR,&CPU::ACC,&CPU::IXR,&CPU::ZPR,&CPU::ZPR,&CPU::ZPR,&CPU::ZPR,&CPU::IMP,&CPU::IMM,&CPU::ACC,&CPU::IMM,&CPU::IND,&CPU::ABR,&CPU::ABR,&CPU::ABR,
        &CPU::REL,&CPU::IYR,&CPU::ACC,&CPU::IYM,&CPU::ZXR,&CPU::ZXR,&CPU::ZXR,&CPU::ZXR,&CPU::IMP,&CPU::AYR,&CPU::IMP,&CPU::AYM,&CPU::AXR,&CPU::AXR,&CPU::AXM,&CPU::AXM,
        &CPU::IMM,&CPU::IXW,&CPU::IMM,&CPU::IXW,&CPU::ZPW,&CPU::ZPW,&CPU::ZPW,&CPU::ZPW,&CPU::IMP,&CPU::IMM,&CPU::IMP,&CPU::IMM,&CPU::ABW,&CPU::ABW,&CPU::ABW,&CPU::ABW,
        &CPU::REL,&CPU::IYW,&CPU::ACC,&CPU::IYW,&CPU::ZXW,&CPU::ZXW,&CPU::ZYW,&CPU::ZYW,&CPU::IMP,&CPU::AYW,&CPU::IMP,&CPU::AYW,&CPU::AXW,&CPU::AXW,&CPU::AYW,&CPU::AYW,
        &CPU::IMM,&CPU::IXR,&CPU::IMM,&CPU::IXR,&CPU::ZPR,&CPU::ZPR,&CPU::ZPR,&CPU::ZPR,&CPU::IMP,&CPU::IMM,&CPU::IMP,&CPU::IMM,&CPU::ABR,&CPU::ABR,&CPU::ABR,&CPU::ABR,
        &CPU::REL,&CPU::IYR,&CPU::ACC,&CPU::IYR,&CPU::ZXR,&CPU::ZXR,&CPU::ZYR,&CPU::ZYR,&CPU::IMP,&CPU::AYR,&CPU::IMP,&CPU::AYR,&CPU::AXR,&CPU::AXR,&CPU::AYR,&CPU::AYR,
        &CPU::IMM,&CPU::IXR,&CPU::IMM,&CPU::IXR,&CPU::ZPR,&CPU::ZPR,&CPU::ZPR,&CPU::ZPR,&CPU::IMP,&CPU::IMM,&CPU::IMP,&CPU::IMM,&CPU::ABR,&CPU::ABR,&CPU::ABR,&CPU::ABR,
        &CPU::REL,&CPU::IYR,&CPU::ACC,&CPU::IYM,&CPU::ZXR,&CPU::ZXR,&CPU::ZXR,&CPU::ZXR,&CPU::IMP,&CPU::AYR,&CPU::IMP,&CPU::AYM,&CPU::AXR,&CPU::AXR,&CPU::AXM,&CPU::AXM,
        &CPU::IMM,&CPU::IXR,&CPU::IMM,&CPU::IXR,&CPU::ZPR,&CPU::ZPR,&CPU::ZPR,&CPU::ZPR,&CPU::IMP,&CPU::IMM,&CPU::IMP,&CPU::IMM,&CPU::ABR,&CPU::ABR,&CPU::ABR,&CPU::ABR,
        &CPU::REL,&CPU::IYR,&CPU::ACC,&CPU::IYM,&CPU::ZXR,&CPU::ZXR,&CPU::ZXR,&CPU::ZXR,&CPU::IMP,&CPU::AYR,&CPU::IMP,&CPU::AYM,&CPU::AXR,&CPU::AXR,&CPU::AXM,&CPU::AXM
    };

    memcpy(_instructions, instructions, sizeof(instructions));
    memcpy(_addressingModes, addressingModes, sizeof(addressingModes));
}

CPU::~CPU() { }

void CPU::power() {
    _frozen = false;

    _lineNMI = false;
    _lineMapperIRQ = false;
    _lineFrameIRQ = false;
    _lineDeltaIRQ = false;

    _shouldIRQ = false;

    _registerA = 0x00;
    _registerX = 0x00;
    _registerY = 0x00;
    _stackPointer = 0xFD;

    _status = Flag::I;

    _programCounter = _nes.readCPU(0xFFFC);
    _programCounter |= _nes.readCPU(0xFFFD) << 8;
}

void CPU::reset() {
    _frozen = false;

    _lineNMI = false;
    _lineMapperIRQ = false;
    _lineFrameIRQ = false;
    _lineDeltaIRQ = false;

    _stackPointer -= 3;

    _status |= Flag::I;

    _programCounter = _nes.readCPU(0xFFFC);
    _programCounter |= _nes.readCPU(0xFFFD) << 8;
}

void CPU::tick() {
    if (_frozen) {
        return;
    }

    uint8_t instruction = fetch();

    (this->*_addressingModes[instruction])();
    (this->*_instructions[instruction])();

    if (_delayNMI || _delayIRQ) {
        _nes.read(_programCounter);
        _nes.read(_programCounter);

        _nes.write(0x100 | _stackPointer--, _programCounter >> 8);
        _nes.write(0x100 | _stackPointer--, _programCounter & 0x00FF);

        uint16_t address = _shouldNMI ? 0xFFFA : 0xFFFE;

        _shouldNMI = false;

        _nes.write(0x100 | _stackPointer--, _status | Flag::U);

        setStatus(Flag::I, true);

        _programCounter = _nes.read(address);
        _programCounter |= _nes.read(address + 1) << 8;
    }
}

void CPU::poll() {
    _delayNMI = _shouldNMI;

    if (!_edgeDetectorNMI && _lineNMI) {
        _shouldNMI = true;
    }

    _edgeDetectorNMI = _lineNMI;
    _delayIRQ = _shouldIRQ;

    _shouldIRQ = (_lineMapperIRQ || _lineFrameIRQ || _lineDeltaIRQ) && !getStatus(Flag::I);
}

void CPU::setNMI(bool nmi) {
    _lineNMI = nmi;
}

void CPU::setMapperIRQ(bool irq) {
    _lineMapperIRQ = irq;
}

void CPU::setFrameIRQ(bool irq) {
    _lineFrameIRQ = irq;
}

void CPU::setDeltaIRQ(bool irq) {
    _lineDeltaIRQ = irq;
}

bool CPU::isFrozen() const {
    return _frozen;
}

uint8_t CPU::fetch() {
    return _nes.read(_programCounter++);
}

void CPU::setStatus(uint8_t flag, bool value) {
    if (value) {
        _status |= flag;
    } else {
        _status &= ~flag;
    }
}

bool CPU::getStatus(uint8_t flag) const {
    return _status & flag;
}

void CPU::ABR() {
    ABW();

    _registerM = _nes.read(_targetAddress);
}

void CPU::ABW() {
    _targetAddress = fetch();
    _targetAddress |= fetch() << 8;
}

void CPU::ACC() {
    _registerM = _nes.read(_programCounter);
}

void CPU::AXM() {
    AXW();

    _registerM = _nes.read(_targetAddress);
}

void CPU::AXR() {
    _targetAddress = fetch();

    uint16_t translated = _targetAddress + _registerX;

    bool invalidAddress = (_targetAddress & 0xFF00) != (translated & 0xFF00);

    _targetAddress = translated & 0x00FF;
    _targetAddress |= fetch() << 8;

    _registerM = _nes.read(_targetAddress);

    if (invalidAddress) {
        _targetAddress += 0x100;

        _registerM = _nes.read(_targetAddress);
    }
}

void CPU::AXW() {
    _targetAddress = fetch();

    uint16_t translated = _targetAddress + _registerX;

    bool invalidAddress = (_targetAddress & 0xFF00) != (translated & 0xFF00);

    _targetAddress = translated & 0x00FF;
    _targetAddress |= fetch() << 8;

    _registerM = _nes.read(_targetAddress);

    if (invalidAddress) {
        _targetAddress += 0x100;
    }
}

void CPU::AYM() {
    AYW();

    _registerM = _nes.read(_targetAddress);
}

void CPU::AYR() {
    _targetAddress = fetch();

    uint16_t translated = _targetAddress + _registerY;

    bool invalidAddress = (_targetAddress & 0xFF00) != (translated & 0xFF00);

    _targetAddress = translated & 0x00FF;
    _targetAddress |= fetch() << 8;

    _registerM = _nes.read(_targetAddress);

    if (invalidAddress) {
        _targetAddress += 0x100;

        _registerM = _nes.read(_targetAddress);
    }
}

void CPU::AYW() {
    _targetAddress = fetch();

    uint16_t translated = _targetAddress + _registerY;

    bool invalidAddress = (_targetAddress & 0xFF00) != (translated & 0xFF00);

    _targetAddress = translated & 0x00FF;
    _targetAddress |= fetch() << 8;

    _registerM = _nes.read(_targetAddress);

    if (invalidAddress) {
        _targetAddress += 0x100;
    }
}

void CPU::IMM() {
    _registerM = fetch();
}

void CPU::IMP() {
    _registerM = _nes.read(_programCounter);
}

void CPU::IND() {
    uint16_t pointer = fetch();

    pointer |= fetch() << 8;

    if ((pointer & 0x00FF) == 0xFF) {
        _targetAddress = _nes.read(pointer);
        _targetAddress |= _nes.read(pointer & 0xFF00) << 8;
    } else {
        _targetAddress = _nes.read(pointer);
        _targetAddress |= _nes.read(pointer + 1) << 8;
    }
}

void CPU::IXR() {
    IXW();

    _registerM = _nes.read(_targetAddress);
}

void CPU::IXW() {
    uint8_t pointer = fetch();

    _registerM = _nes.read(pointer);

    pointer += _registerX;

    _targetAddress = _nes.read(pointer);
    _targetAddress |= _nes.read(++pointer & 0xFF) << 8;
}

void CPU::IYM() {
    IYW();

    _registerM = _nes.read(_targetAddress);
}

void CPU::IYR() {
    uint8_t pointer = fetch();

    _targetAddress = _nes.read(pointer);

    uint16_t translated = _targetAddress + _registerY;

    bool invalidAddress = translated & 0xFF00;

    _targetAddress = translated & 0x00FF;
    _targetAddress |= _nes.read(++pointer & 0xFF) << 8;

    _registerM = _nes.read(_targetAddress);

    if (invalidAddress) {
        _targetAddress += 0x100;

        _registerM = _nes.read(_targetAddress);
    }
}

void CPU::IYW() {
    uint8_t pointer = fetch();

    _targetAddress = _nes.read(pointer);

    uint16_t translated = _targetAddress + _registerY;

    bool invalidAddress = (_targetAddress & 0xFF00) != (translated & 0xFF00);

    _targetAddress = translated & 0x00FF;
    _targetAddress |= _nes.read(++pointer & 0xFF) << 8;

    _registerM = _nes.read(_targetAddress);

    if (invalidAddress) {
        _targetAddress += 0x100;
    }
}

void CPU::REL() {
    _targetAddress = fetch();

    if (_targetAddress & 0x80) {
        _targetAddress |= 0xFF00;
    }
}

void CPU::ZPR() {
    ZPW();

    _registerM = _nes.read(_targetAddress);
}

void CPU::ZPW() {
    _targetAddress = fetch();
}

void CPU::ZXR() {
    ZXW();

    _registerM = _nes.read(_targetAddress);
}

void CPU::ZXW() {
    _targetAddress = fetch();

    _registerM = _nes.read(_targetAddress);

    _targetAddress += _registerX;
    _targetAddress &= 0x00FF;
}

void CPU::ZYR() {
    ZYW();

    _registerM = _nes.read(_targetAddress);
}

void CPU::ZYW() {
    _targetAddress = fetch();

    _registerM = _nes.read(_targetAddress);

    _targetAddress += _registerY;
    _targetAddress &= 0x00FF;
}

void CPU::AAL() {
    setStatus(Flag::C, _registerA & 0x80);

    _registerA <<= 1;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void CPU::ADC() {
    uint16_t result = _registerA + _registerM + (getStatus(Flag::C) ? 0x01 : 0x00);

    setStatus(Flag::C, result & 0xFF00);
    setStatus(Flag::V, ~(_registerA ^ _registerM) & (_registerA ^ result) & 0x80);

    _registerA = result & 0x00FF;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void CPU::ALR() {
    _registerA &= _registerM;

    setStatus(Flag::C, _registerA & 0x01);

    _registerA >>= 1;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void CPU::ANC() {
    _registerA &= _registerM;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
    setStatus(Flag::C, _registerA & 0x80);
}

void CPU::AND() {
    _registerA &= _registerM;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void CPU::ANE() {
    _registerA = (_registerA | 0xEE) & _registerX & _registerM;
}

void CPU::ARR() {
    _registerA &= _registerM;

    _registerA = (getStatus(Flag::C) ? 0x80 : 0x00) | (_registerA >> 1);

    setStatus(Flag::C, _registerA & 0x40);
    setStatus(Flag::V, bool(_registerA & 0x40) ^ bool(_registerA & 0x20));
    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void CPU::ASL() {
    _nes.write(_targetAddress, _registerM);

    setStatus(Flag::C, _registerM & 0x80);

    _registerM <<= 1;

    setStatus(Flag::Z, !_registerM);
    setStatus(Flag::N, _registerM & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void CPU::BCC() {
    if (!getStatus(Flag::C)) {
        if (_shouldIRQ && !_delayIRQ) {
            _shouldIRQ = false;
        }

        _nes.read(_programCounter);

        uint16_t translated = _targetAddress + _programCounter;

        if ((translated & 0xFF00) != (_programCounter & 0xFF00)) {
            _nes.read(_programCounter);
        }

        _programCounter = translated;
    }
}

void CPU::BCS() {
    if (getStatus(Flag::C)) {
        if (_shouldIRQ && !_delayIRQ) {
            _shouldIRQ = false;
        }

        _nes.read(_programCounter);

        uint16_t translated = _targetAddress + _programCounter;

        if ((translated & 0xFF00) != (_programCounter & 0xFF00)) {
            _nes.read(_programCounter);
        }

        _programCounter = translated;
    }
}

void CPU::BEQ() {
    if (getStatus(Flag::Z)) {
        if (_shouldIRQ && !_delayIRQ) {
            _shouldIRQ = false;
        }

        _nes.read(_programCounter);

        uint16_t translated = _targetAddress + _programCounter;

        if ((translated & 0xFF00) != (_programCounter & 0xFF00)) {
            _nes.read(_programCounter);
        }

        _programCounter = translated;
    }
}

void CPU::BIT() {
    setStatus(Flag::Z, !(_registerA & _registerM));
    setStatus(Flag::V, _registerM & 0x40);
    setStatus(Flag::N, _registerM & 0x80);
}

void CPU::BMI() {
    if (getStatus(Flag::N)) {
        if (_shouldIRQ && !_delayIRQ) {
            _shouldIRQ = false;
        }

        _nes.read(_programCounter);

        uint16_t translated = _targetAddress + _programCounter;

        if ((translated & 0xFF00) != (_programCounter & 0xFF00)) {
            _nes.read(_programCounter);
        }

        _programCounter = translated;
    }
}

void CPU::BNE() {
    if (!getStatus(Flag::Z)) {
        if (_shouldIRQ && !_delayIRQ) {
            _shouldIRQ = false;
        }

        _nes.read(_programCounter);

        uint16_t translated = _targetAddress + _programCounter;

        if ((translated & 0xFF00) != (_programCounter & 0xFF00)) {
            _nes.read(_programCounter);
        }

        _programCounter = translated;
    }
}

void CPU::BPL() {
    if (!getStatus(Flag::N)) {
        if (_shouldIRQ && !_delayIRQ) {
            _shouldIRQ = false;
        }

        _nes.read(_programCounter);

        uint16_t translated = _targetAddress + _programCounter;

        if ((translated & 0xFF00) != (_programCounter & 0xFF00)) {
            _nes.read(_programCounter);
        }

        _programCounter = translated;
    }
}

void CPU::BRK() {
    _programCounter++;

    _nes.write(0x100 | _stackPointer--, _programCounter >> 8);
    _nes.write(0x100 | _stackPointer--, _programCounter & 0x00FF);

    uint16_t address = _shouldNMI ? 0xFFFA : 0xFFFE;

    _shouldNMI = false;

    _nes.write(0x100 | _stackPointer--, _status | Flag::B | Flag::U);

    setStatus(Flag::I, true);

    _programCounter = _nes.read(address);
    _programCounter |= _nes.read(address + 1) << 8;

    _delayNMI = false;
}

void CPU::BVC() {
    if (!getStatus(Flag::V)) {
        if (_shouldIRQ && !_delayIRQ) {
            _shouldIRQ = false;
        }

        _nes.read(_programCounter);

        uint16_t translated = _targetAddress + _programCounter;

        if ((translated & 0xFF00) != (_programCounter & 0xFF00)) {
            _nes.read(_programCounter);
        }

        _programCounter = translated;
    }
}

void CPU::BVS() {
    if (getStatus(Flag::V)) {
        if (_shouldIRQ && !_delayIRQ) {
            _shouldIRQ = false;
        }

        _nes.read(_programCounter);

        uint16_t translated = _targetAddress + _programCounter;

        if ((translated & 0xFF00) != (_programCounter & 0xFF00)) {
            _nes.read(_programCounter);
        }

        _programCounter = translated;
    }
}

void CPU::CLC() {
    setStatus(Flag::C, false);
}

void CPU::CLD() {
    setStatus(Flag::D, false);
}

void CPU::CLI() {
    setStatus(Flag::I, false);
}

void CPU::CLV() {
    setStatus(Flag::V, false);
}

void CPU::CMP() {
    setStatus(Flag::C, _registerA >= _registerM);
    setStatus(Flag::Z, _registerA == _registerM);
    setStatus(Flag::N, (_registerA - _registerM) & 0x80);
}

void CPU::CPX() {
    setStatus(Flag::C, _registerX >= _registerM);
    setStatus(Flag::Z, _registerX == _registerM);
    setStatus(Flag::N, (_registerX - _registerM) & 0x80);
}

void CPU::CPY() {
    setStatus(Flag::C, _registerY >= _registerM);
    setStatus(Flag::Z, _registerY == _registerM);
    setStatus(Flag::N, (_registerY - _registerM) & 0x80);
}

void CPU::DCP() {
    _nes.write(_targetAddress, _registerM);

    _registerM--;

    setStatus(Flag::C, _registerA >= _registerM);
    setStatus(Flag::Z, _registerA == _registerM);
    setStatus(Flag::N, (_registerA - _registerM) & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void CPU::DEC() {
    _nes.write(_targetAddress, _registerM);

    _registerM--;

    setStatus(Flag::Z, !_registerM);
    setStatus(Flag::N, _registerM & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void CPU::DEX() {
    _registerX--;

    setStatus(Flag::Z, !_registerX);
    setStatus(Flag::N, _registerX & 0x80);
}

void CPU::DEY() {
    _registerY--;

    setStatus(Flag::Z, !_registerY);
    setStatus(Flag::N, _registerY & 0x80);
}

void CPU::EOR() {
    _registerA ^= _registerM;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void CPU::INC() {
    _nes.write(_targetAddress, _registerM);

    _registerM++;

    setStatus(Flag::Z, !_registerM);
    setStatus(Flag::N, _registerM & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void CPU::INX() {
    _registerX++;

    setStatus(Flag::Z, !_registerX);
    setStatus(Flag::N, _registerX & 0x80);
}

void CPU::INY() {
    _registerY++;

    setStatus(Flag::Z, !_registerY);
    setStatus(Flag::N, _registerY & 0x80);
}

void CPU::ISC() {
    _nes.write(_targetAddress, _registerM);

    _registerM++;

    uint8_t value = _registerM;

    _registerM ^= 0xFF;

    uint16_t result = _registerA + _registerM + (getStatus(Flag::C) ? 0x01 : 0x00);

    setStatus(Flag::C, result & 0x0100);
    setStatus(Flag::V, ~(_registerA ^ _registerM) & (_registerA ^ result) & 0x80);

    _registerA = result & 0x00FF;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);

    _nes.write(_targetAddress, value);
}

void CPU::JAM() {
    _frozen = true;
}

void CPU::JMP() {
    _programCounter = _targetAddress;
}

void CPU::JSR() {
    _nes.read(_programCounter);

    _programCounter--;

    _nes.write(0x100 | _stackPointer--, _programCounter >> 8);
    _nes.write(0x100 | _stackPointer--, _programCounter & 0x00FF);

    _programCounter = _targetAddress;
}

void CPU::LAR() {
    setStatus(Flag::C, _registerA & 0x01);

    _registerA >>= 1;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void CPU::LAS() {
    uint8_t result = _registerM & _stackPointer;

    _registerA = result;
    _registerX = result;
    _stackPointer = result;
}

void CPU::LAX() {
    _registerA = _registerM;
    _registerX = _registerM;

    setStatus(Flag::Z, !_registerM);
    setStatus(Flag::N, _registerM & 0x80);
}

void CPU::LDA() {
    _registerA = _registerM;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void CPU::LDX() {
    _registerX = _registerM;

    setStatus(Flag::Z, !_registerX);
    setStatus(Flag::N, _registerX & 0x80);
}

void CPU::LDY() {
    _registerY = _registerM;

    setStatus(Flag::Z, !_registerY);
    setStatus(Flag::N, _registerY & 0x80);
}

void CPU::LSR() {
    _nes.write(_targetAddress, _registerM);

    setStatus(Flag::C, _registerM & 0x01);

    _registerM >>= 1;

    setStatus(Flag::Z, !_registerM);
    setStatus(Flag::N, _registerM & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void CPU::LXA() {
    _registerA = _registerM;
    _registerX = _registerM;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void CPU::NOP() {

}

void CPU::ORA() {
    _registerA |= _registerM;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void CPU::PHA() {
    _nes.write(0x100 | _stackPointer--, _registerA);
}

void CPU::PHP() {
    _nes.write(0x100 | _stackPointer--, _status | Flag::B | Flag::U);
}

void CPU::PLA() {
    _stackPointer++;

    _nes.read(_programCounter);

    _registerA = _nes.read(0x100 | _stackPointer);

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void CPU::PLP() {
    _stackPointer++;

    _nes.read(_programCounter);

    _status = _nes.read(0x100 | _stackPointer) & 0xCF;
}

void CPU::RAL() {
    bool carry = _registerA & 0x80;

    _registerA = (getStatus(Flag::C) ? 0x01 : 0x00) | (_registerA << 1);

    setStatus(Flag::C, carry);
    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void CPU::RAR() {
    bool carry = _registerA & 0x01;

    _registerA = (getStatus(Flag::C) ? 0x80 : 0x00) | (_registerA >> 1);

    setStatus(Flag::C, carry);
    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void CPU::RLA() {
    _nes.write(_targetAddress, _registerM);

    bool carry = _registerM & 0x80;

    _registerM = (getStatus(Flag::C) ? 0x01 : 0x00) | (_registerM << 1);
    _registerA &= _registerM;

    setStatus(Flag::C, carry);
    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void CPU::ROL() {
    _nes.write(_targetAddress, _registerM);

    bool carry = _registerM & 0x80;

    _registerM = (getStatus(Flag::C) ? 0x01 : 0x00) | (_registerM << 1);

    setStatus(Flag::C, carry);
    setStatus(Flag::Z, !_registerM);
    setStatus(Flag::N, _registerM & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void CPU::ROR() {
    _nes.write(_targetAddress, _registerM);

    bool carry = _registerM & 0x01;

    _registerM = (getStatus(Flag::C) ? 0x80 : 0x00) | (_registerM >> 1);

    setStatus(Flag::C, carry);
    setStatus(Flag::Z, !_registerM);
    setStatus(Flag::N, _registerM & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void CPU::RRA() {
    _nes.write(_targetAddress, _registerM);

    uint8_t carry = _registerM & 0x01;

    _registerM = (getStatus(Flag::C) ? 0x80 : 0x00) | (_registerM >> 1);

    uint16_t result = _registerA + _registerM + carry;

    setStatus(Flag::C, result & 0x0100);
    setStatus(Flag::V, ~(_registerA ^ _registerM) & (_registerA ^ result) & 0x80);

    _registerA = result & 0x00FF;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void CPU::RTI() {
    _stackPointer++;

    _nes.read(_programCounter);

    _status = _nes.read(0x100 | _stackPointer) & 0xCF;
    _programCounter = _nes.read(0x100 | ++_stackPointer);
    _programCounter |= _nes.read(0x100 | ++_stackPointer) << 8;
}

void CPU::RTS() {
    _stackPointer++;

    _nes.read(_programCounter);
    _nes.read(_programCounter);

    _programCounter = _nes.read(0x100 | _stackPointer);
    _programCounter |= _nes.read(0x100 | ++_stackPointer) << 8;

    _programCounter++;
}

void CPU::SAX() {
    _nes.write(_targetAddress, _registerA & _registerX);
}

void CPU::SBC() {
    _registerM ^= 0xFF;

    uint16_t result = _registerA + _registerM + (getStatus(Flag::C) ? 0x01 : 0x00);

    setStatus(Flag::C, result & 0xFF00);
    setStatus(Flag::V, ~(_registerA ^ _registerM) & (_registerA ^ result) & 0x80);

    _registerA = result & 0x00FF;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void CPU::SBX() {
    _registerX &= _registerA;

    setStatus(Flag::C, _registerX >= _registerM);
    setStatus(Flag::Z, _registerX == _registerM);

    _registerX -= _registerM;

    setStatus(Flag::N, _registerX & 0x80);
}

void CPU::SEC() {
    setStatus(Flag::C, true);
}

void CPU::SED() {
    setStatus(Flag::D, true);
}

void CPU::SEI() {
    setStatus(Flag::I, true);
}

void CPU::SHA() {
    _nes.write(_targetAddress, _registerA & _registerX & (uint8_t(_targetAddress >> 8) + 1));
}

void CPU::SHX() {
    uint8_t addressHigh = 1 + (_targetAddress >> 8);

    _nes.write(((_registerX & addressHigh) << 8) | _targetAddress & 0xFF, _registerX & addressHigh);
}

void CPU::SHY() {
    uint8_t addressHigh = 1 + (_targetAddress >> 8);

    _nes.write(((_registerY & addressHigh) << 8) | _targetAddress & 0xFF, _registerY & addressHigh);
}

void CPU::SLO() {
    _nes.write(_targetAddress, _registerM);

    setStatus(Flag::C, _registerM & 0x80);

    _registerM <<= 1;
    _registerA |= _registerM;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void CPU::SRE() {
    _nes.write(_targetAddress, _registerM);

    setStatus(Flag::C, _registerM & 0x01);

    _registerM >>= 1;
    _registerA ^= _registerM;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void CPU::STA() {
    _nes.write(_targetAddress, _registerA);
}

void CPU::STX() {
    _nes.write(_targetAddress, _registerX);
}

void CPU::STY() {
    _nes.write(_targetAddress, _registerY);
}

void CPU::TAS() {
    _stackPointer = _registerA & _registerX;

    _nes.write(_targetAddress, _stackPointer & (uint8_t(_targetAddress >> 8) + 1));
}

void CPU::TAX() {
    _registerX = _registerA;

    setStatus(Flag::Z, !_registerX);
    setStatus(Flag::N, _registerX & 0x80);
}

void CPU::TAY() {
    _registerY = _registerA;

    setStatus(Flag::Z, !_registerY);
    setStatus(Flag::N, _registerY & 0x80);
}

void CPU::TSX() {
    _registerX = _stackPointer;

    setStatus(Flag::Z, !_registerX);
    setStatus(Flag::N, _registerX & 0x80);
}

void CPU::TXA() {
    _registerA = _registerX;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void CPU::TXS() {
    _stackPointer = _registerX;
}

void CPU::TYA() {
    _registerA = _registerY;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void CPU::USB() {
    _registerM ^= 0xFF;

    uint16_t result = _registerA + _registerM + (getStatus(Flag::C) ? 0x01 : 0x00);

    setStatus(Flag::C, result & 0x0100);
    setStatus(Flag::V, ~(_registerA ^ _registerM) & (_registerA ^ result) & 0x80);

    _registerA = result & 0x00FF;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}



PPU::PPU(NES& nes) : _nes(nes) {
    _pixelX = 0x0000;
    _pixelY = 0x0000;

    _renderingEnabled = false;
    _renderingEnabledDelayed = false;
    _preventVerticalBlank = false;

    _controlIncrementMode = false;
    _controlForegroundTable = false;
    _controlBackgroundTable = false;
    _controlForegroundLarge = false;
    _controlInterruptOnVertivalBlank = false;

    _maskGreyscaleMode = false;
    _maskRenderBackgroundLeft = false;
    _maskRenderForegroundLeft = false;
    _maskRenderBackground = false;
    _maskRenderForeground = false;

    _maskColorEmphasize = 0x00;

    _statusSpriteOverflow = false;
    _statusSpriteZeroHit = false;
    _statusVerticalBlank = false;

    memset(_clockDecays, 0x00, 0x3);

    _registerDecay = 0x00;

    _latchCycle = false;
    _latchAddress = false;

    _registerT = 0x0000;
    _registerV = 0x0000;
    _delayedRegisterV = 0x0000;

    _scrollX = 0x00;

    _delayDataRead = 0x00;
    _delayDataWrite = 0x00;
    _bufferData = 0x00;

    memset(_backgroundData, 0x00, 0x4);
    memset(_backgroundShifter, 0x0000, 0x8);

    memset(_foregroundData, 0x00, 0x20);
    memset(_foregroundShifter, 0x00, 0x10);
    memset(_foregroundAttributes, 0x00, 0x8);
    memset(_foregroundPositions, 0x00, 0x8);

    _foregroundDataPointer = 0x00;
    _foregroundSpriteCount = 0x00;
    _foregroundSpriteCountNext = 0x00;
    _foregroundSpritePointer = 0x00;
    _foregroundReadDelay = 0x00;

    _foregroundSpriteAddress = 0x0000;

    _foregroundSpriteZeroLine = false;
    _foregroundSpriteZeroShould = false;
    _foregroundSpriteZeroHit = false;

    _foregroundEvaluationStep = SpriteEvaluationStep::LOAD_SECONDARY_OAM;
}

PPU::~PPU() {}

void PPU::power() {
    _pixelY = 0xFF00;
    _pixelX = 0xFF00;

    _renderingEnabled = false;
    _renderingEnabledDelayed = false;
    _preventVerticalBlank = false;

    _controlIncrementMode = false;
    _controlForegroundTable = false;
    _controlBackgroundTable = false;
    _controlForegroundLarge = false;
    _controlInterruptOnVertivalBlank = false;

    _maskGreyscaleMode = false;
    _maskRenderBackgroundLeft = false;
    _maskRenderForegroundLeft = false;
    _maskRenderBackground = false;
    _maskRenderForeground = false;

    _maskColorEmphasize = 0x00;

    _statusSpriteOverflow = true;
    _statusSpriteZeroHit = false;
    _statusVerticalBlank = true;

    _foregroundSpritePointer = 0x00;

    _latchAddress = false;
    _latchCycle = false;

    _registerT = 0x0000;
    _registerV = 0x0000;
    _scrollX = 0x00;

    _delayDataWrite = 0x00;
    _delayDataRead = 0x00;
    _bufferData = 0x00;
}

void PPU::reset() {
    _pixelY = 0xFF00;
    _pixelX = 0xFF00;

    _renderingEnabled = false;
    _renderingEnabledDelayed = false;
    _preventVerticalBlank = false;

    _controlIncrementMode = false;
    _controlForegroundTable = false;
    _controlBackgroundTable = false;
    _controlForegroundLarge = false;
    _controlInterruptOnVertivalBlank = false;

    _maskGreyscaleMode = false;
    _maskRenderBackgroundLeft = false;
    _maskRenderForegroundLeft = false;
    _maskRenderBackground = false;
    _maskRenderForeground = false;

    _maskColorEmphasize = 0x00;

    _latchAddress = false;
    _latchCycle = false;

    _registerT = 0x0000;
    _registerV = 0x0000;
    _scrollX = 0x00;

    _delayDataWrite = 0x00;
    _delayDataRead = 0x00;
    _bufferData = 0x00;
}

void PPU::tick() {
    if (_pixelX > 339) {
        _pixelX = 0;

        if (++_pixelY > 261) {
            _pixelY = 0;
            _foregroundSpriteCount = 0;

            _latchCycle = !_latchCycle;

            for (int k = 0; k < 3; k++) {
                if (_clockDecays[k] > 0 && --_clockDecays[k] == 0) {
                    switch (k) {
                    case 0: _registerDecay &= 0x3F; break;
                    case 1: _registerDecay &= 0xDF; break;
                    case 2: _registerDecay &= 0xE0; break;
                    }
                }
            }
        }

        resetForegroundData();

        if (_pixelY == 261) {
            _statusSpriteOverflow = false;
            _statusSpriteZeroHit = false;

            memset(_foregroundShifter, 0x00, 0x10);
        }
    } else {
        _pixelX++;

        if (_pixelY < 240) {
            if (_pixelX >= 1 && _pixelX < 257 || _pixelX >= 321 && _pixelX < 337) {
                loadBackgroundShifters();
            }

            if (_pixelX == 256) {
                incrementScrollY();
            } else if (_pixelX == 257) {
                resetScrollX();
            }

            if (_pixelX >= 2 && _pixelX < 257) {
                updateForegroundShifter();
            }

            if (_pixelX < 65) {
                clearForegroundData();
            } else if (_pixelX < 257) {
                fetchForegroundData();
            } else if (_pixelX < 321) {
                loadForegroundShifter();
            }

            if (_pixelX > 0 && _pixelX < 257 && _pixelY < 240) {
                memcpy(_frameBuffer + ((_pixelY << 8) + _pixelX - 1) * 3, PALETTE_COLORS[_maskColorEmphasize][_nes.readPPU(0x3F00 | blend())], 3);
            }
        } else if (_pixelY == 240 && _pixelX == 1) {
            _nes.getMapper()->notifyStateA12(_registerV & 0x1000);
        } else if (_pixelY == 261) {
            if (_pixelX == 1) {
                _statusVerticalBlank = false;

                _nes.getCPU()->setNMI(false);
            }

            if (_pixelX >= 1 && _pixelX < 257 || _pixelX >= 321 && _pixelX < 337) {
                loadBackgroundShifters();
            }

            if (_pixelX == 256) {
                incrementScrollY();
            } else if (_pixelX == 257) {
                resetScrollX();
            } else  if (_pixelX >= 280 && _pixelX < 305) {
                resetScrollY();
            }

            if (_pixelX > 1) {
                if (_pixelX < 257) {
                    updateForegroundShifter();
                } else if (_pixelX < 321) {
                    loadForegroundShifter();
                }
            }

            if (_renderingEnabled && (_pixelX == 337 || _pixelX == 339)) {
                readAndNotifyA12(0x2000 | (_registerV & 0x0FFF));

                if (_pixelX == 339 && _latchCycle) {
                    _pixelX = 340;
                }
            }
        } else if (_pixelX == 1 && _pixelY == 241) {
            if (!_preventVerticalBlank) {
                _statusVerticalBlank = true;

                if (_controlInterruptOnVertivalBlank) {
                    _nes.getCPU()->setNMI(true);
                }
            }

            _preventVerticalBlank = false;
            _frameReady = true;
        }
    }

    if (_renderingEnabledDelayed != _renderingEnabled) {
        _renderingEnabledDelayed = _renderingEnabled;

        if (_pixelY < 240 || _pixelY == 261) {
            if (!_renderingEnabledDelayed) {
                _nes.getMapper()->notifyStateA12(_registerV & 0x1000);

                if (_pixelX >= 65 && _pixelX <= 256) {
                    _foregroundSpritePointer++;
                }
            }
        }
    }

    if (_renderingEnabled != (_maskRenderBackground || _maskRenderForeground)) {
        _renderingEnabled = _maskRenderBackground || _maskRenderForeground;
    }


    if (_delayDataWrite > 0 && --_delayDataWrite == 0) {
        _registerV = _delayedRegisterV;
        _registerT = _registerV;

        if (_pixelY >= 240 && _pixelY != 261 || !_renderingEnabled) {
            _nes.getMapper()->notifyStateA12(_registerV & 0x1000);
        }
    }

    if (_delayDataRead > 0) {
        _delayDataRead--;
    }

    _nes.getMapper()->tick();
}

void PPU::write(uint8_t addr, uint8_t value) {
    memset(_clockDecays, DECAY_PERIOD, 3);

    _registerDecay = value;

    switch (addr) {
    case Register::PPU_CTRL: {
        _registerT &= 0xF3FF;
        _registerT |= (value & 0x03) << 10;

        _controlIncrementMode = value & 0x04;
        _controlForegroundTable = value & 0x08;
        _controlBackgroundTable = value & 0x10;
        _controlForegroundLarge = value & 0x20;
        _controlInterruptOnVertivalBlank = value & 0x80;

        if (!_controlInterruptOnVertivalBlank) {
            _nes.getCPU()->setNMI(false);
        } else if (_statusVerticalBlank) {
            _nes.getCPU()->setNMI(true);
        }

        break;
    }

    case Register::PPU_MASK: {
        _maskGreyscaleMode = value & 0x01;
        _maskRenderBackgroundLeft = value & 0x02;
        _maskRenderForegroundLeft = value & 0x04;
        _maskRenderBackground = value & 0x08;
        _maskRenderForeground = value & 0x10;
        _maskColorEmphasize = value >> 5;

        break;
    }

    case Register::OAM_ADDR: {
        _foregroundSpritePointer = value;

        break;
    }

    case Register::OAM_DATA: {
        if (_pixelY >= 240 && _pixelY != 261 || !_renderingEnabled) {
            if ((_foregroundSpritePointer & 0x03) == 0x02) {
                value &= 0xE3;
            }

            _nes.writeOAM(_foregroundSpritePointer++, value);
        } else {
            _foregroundSpritePointer += 4;
        }

        break;
    }

    case Register::PPU_SCROLL: {
        if (!_latchAddress) {
            _scrollX = value & 0x07;

            _registerT &= 0xFFE0;
            _registerT |= value >> 3;
        } else {
            _registerT &= 0x8C1F;

            _registerT |= (value & 0xF8) << 2;
            _registerT |= (value & 0x07) << 12;
        }

        _latchAddress = !_latchAddress;

        break;
    }

    case Register::PPU_ADDR: {
        if (!_latchAddress) {
            _registerT &= 0x00FF;
            _registerT |= value << 8;
        } else {
            _registerT &= 0xFF00;
            _registerT |= value;

            _delayDataWrite = 3;
            _delayedRegisterV = _registerT;
        }

        _latchAddress = !_latchAddress;

        break;
    }

    case Register::PPU_DATA: {
        if ((_registerV & 0x3FFF) >= 0x3F00) {
            _nes.writePPU(_registerV, value);
        } else {
            if (_pixelY >= 240 && _pixelY != 261 || !_renderingEnabled) {
                writeAndNotifyA12(_registerV, value);
            } else {
                writeAndNotifyA12(_registerV, _registerV & 0xFF);
            }
        }

        if (_pixelY >= 240 && _pixelY != 261 || !_renderingEnabled) {
            _registerV += _controlIncrementMode ? 32 : 1;
            _registerV &= 0x7FFF;

            _nes.getMapper()->notifyStateA12(_registerV & 0x1000);
        } else {
            incrementScrollX();
            incrementScrollY();
        }

        break;
    }

    default: break;
    }
}

uint8_t PPU::read(uint8_t addr) {
    switch (addr) {
    case Register::PPU_STATUS: {
        memset(_clockDecays, DECAY_PERIOD, 2);

        _latchAddress = false;

        _registerDecay &= 0x1F;
        _registerDecay |= _statusSpriteOverflow << 5;
        _registerDecay |= _statusSpriteZeroHit << 6;
        _registerDecay |= _statusVerticalBlank << 7;

        _statusVerticalBlank = false;
        _nes.getCPU()->setNMI(false);

        if (_pixelY == 241 && _pixelX == 0) {
            _preventVerticalBlank = true;
        }

        break;
    }

    case Register::OAM_DATA: {
        memset(_clockDecays, DECAY_PERIOD, 3);

        _registerDecay = _nes.readOAM(_foregroundSpritePointer);

        break;
    }

    case Register::PPU_DATA: {
        if (_delayDataRead == 0) {
            uint8_t value = readAndNotifyA12(_registerV);

            if ((_registerV & 0x3FFF) >= 0x3F00) {
                _registerDecay &= 0xC0;
                _registerDecay |= value & 0x3F;

                _clockDecays[0] = _clockDecays[2] = DECAY_PERIOD;

                _bufferData = _nes.readPPU(_registerV - 0x1000);
            } else {
                _registerDecay = _bufferData;
                _bufferData = value;

                memset(_clockDecays, DECAY_PERIOD, 3);
            }

            if (_pixelY >= 240 && _pixelY != 261 || !_renderingEnabled) {
                _registerV += _controlIncrementMode ? 32 : 1;
                _registerV &= 0x7FFF;

                _nes.getMapper()->notifyStateA12(_registerV & 0x1000);
            } else {
                incrementScrollX();
                incrementScrollY();
            }

            _delayDataRead = 6;
        }

        break;
    }

    default: break;
    }

    return _registerDecay;
}

uint8_t* PPU::getFrameBuffer() {
    return _frameBuffer;
}

bool PPU::isFrameReady() {
    bool frameReady = _frameReady;

    _frameReady = false;

    return frameReady;
}

void PPU::writeAndNotifyA12(uint16_t addr, uint8_t value) {
    _nes.getMapper()->notifyStateA12(addr & 0x1000);
    _nes.writePPU(addr, value);
}

uint8_t PPU::readAndNotifyA12(uint16_t addr) {
    _nes.getMapper()->notifyStateA12(addr & 0x1000);

    return _nes.readPPU(addr);
}

void PPU::incrementScrollX() {
    if (_maskRenderBackground || _maskRenderForeground) {
        if ((_registerV & 0x001F) == 0x1F) {
            _registerV &= 0xFFE0;
            _registerV ^= 0x0400;
        } else {
            _registerV++;
        }
    }
}

void PPU::incrementScrollY() {
    if (_maskRenderBackground || _maskRenderForeground) {
        if ((_registerV & 0x7000) != 0x7000) {
            _registerV += 0x1000;
        } else {
            _registerV &= 0x8FFF;

            uint8_t coarseY = (_registerV & 0x03E0) >> 5;

            if (coarseY == 0x1D) {
                coarseY = 0;
                _registerV ^= 0x0800;
            } else if (((_registerV >> 5) & 0x1F) == 0x1F) {
                coarseY = 0;
            } else {
                coarseY++;
            }

            _registerV &= 0xFC1F;
            _registerV |= coarseY << 5;
        }
    }
}

void PPU::resetScrollX() {
    if (_maskRenderBackground || _maskRenderForeground) {
        _registerV &= 0xFBE0;
        _registerV |= _registerT & 0x041F;
    }
}

void PPU::resetScrollY() {
    if (_maskRenderBackground || _maskRenderForeground) {
        _registerV &= 0x841F;
        _registerV |= _registerT & 0x7BE0;
    }
}


void PPU::loadBackgroundShifters() {
    updateBackgroundShifters();

    if (_renderingEnabled) {
        switch (_pixelX & 0x07) {
        case 0x1: {
            _backgroundShifter[0] = (_backgroundShifter[0] & 0xFF00) | _backgroundData[2];
            _backgroundShifter[1] = (_backgroundShifter[1] & 0xFF00) | _backgroundData[3];

            if (_backgroundData[1] & 0x01) {
                _backgroundShifter[2] = (_backgroundShifter[2] & 0xFF00) | 0xFF;
            } else {
                _backgroundShifter[2] = (_backgroundShifter[2] & 0xFF00);
            }

            if (_backgroundData[1] & 0x02) {
                _backgroundShifter[3] = (_backgroundShifter[3] & 0xFF00) | 0xFF;
            } else {
                _backgroundShifter[3] = (_backgroundShifter[3] & 0xFF00);
            }

            uint16_t address = 0x2000;
            address |= _registerV & 0x0FFF;

            _backgroundData[0] = readAndNotifyA12(address);

            break;
        }

        case 0x3: {
            uint16_t address = 0x23C0;
            address |= _registerV & 0x0C00;
            address |= (_registerV >> 4) & 0x38;
            address |= (_registerV >> 2) & 0x07;

            _backgroundData[1] = readAndNotifyA12(address);

            if (_registerV & 0x0040) {
                _backgroundData[1] >>= 4;
            }

            if (_registerV & 0x0002) {
                _backgroundData[1] >>= 2;
            }

            _backgroundData[1] &= 0x03;

            break;
        }

        case 0x5: {
            uint16_t address = _controlBackgroundTable << 12;
            address |= _backgroundData[0] << 4;
            address |= _registerV >> 12;

            _backgroundData[2] = readAndNotifyA12(address);

            break;
        } case 0x7: {
            uint16_t address = _controlBackgroundTable << 12;
            address |= _backgroundData[0] << 4;
            address |= _registerV >> 12;
            address += 0x8;

            _backgroundData[3] = readAndNotifyA12(address);

            break;

        }

        case 0x0: incrementScrollX(); break;
        }
    }
}

void PPU::updateBackgroundShifters() {
    if (_maskRenderBackground || _maskRenderForeground) {
        _backgroundShifter[0] <<= 1;
        _backgroundShifter[1] <<= 1;
        _backgroundShifter[2] <<= 1;
        _backgroundShifter[3] <<= 1;
    }
}

void PPU::resetForegroundData() {
    _foregroundSpriteCountNext = _foregroundSpriteCount;

    _foregroundDataPointer = 0;
    _foregroundSpriteCount = 0;
    _foregroundEvaluationStep = SpriteEvaluationStep::LOAD_SECONDARY_OAM;
    _foregroundSpriteZeroLine = _foregroundSpriteZeroShould;
    _foregroundSpriteZeroShould = false;
    _foregroundSpriteZeroHit = false;
}

void PPU::clearForegroundData() {
    if (_pixelX & 0x01) {
        _foregroundData[_foregroundDataPointer++] = 0xFF;

        _foregroundDataPointer &= 0x1F;
    }
}

void PPU::fetchForegroundData() {
    if (_pixelX % 2 == 0 && _renderingEnabled) {
        uint8_t spriteSize = _controlForegroundLarge ? 16 : 8;

        switch (_foregroundEvaluationStep) {
        case SpriteEvaluationStep::LOAD_SECONDARY_OAM: {
            uint8_t spriteData = _nes.readOAM(_foregroundSpritePointer);

            _foregroundData[_foregroundSpriteCount * 4 + (_foregroundSpritePointer & 0x03)] = spriteData;

            if (!(_foregroundSpritePointer & 0x3)) {
                int16_t offsetY = int16_t(_pixelY) - int16_t(spriteData);

                if (offsetY >= 0 && offsetY < spriteSize) {
                    if (!_foregroundSpritePointer++) {
                        _foregroundSpriteZeroShould = true;
                    }
                } else {
                    _foregroundSpritePointer += 4;

                    if (!_foregroundSpritePointer) {
                        _foregroundEvaluationStep = SpriteEvaluationStep::IDLE;
                    } else if (_foregroundSpriteCount == 8) {
                        _foregroundEvaluationStep = SpriteEvaluationStep::INCREMENT_POINTER;
                    }
                }
            } else if (!(++_foregroundSpritePointer & 0x03)) {
                _foregroundSpriteCount++;

                if (!_foregroundSpritePointer) {
                    _foregroundEvaluationStep = SpriteEvaluationStep::IDLE;
                } else if (_foregroundSpriteCount == 8) {
                    _foregroundEvaluationStep = SpriteEvaluationStep::INCREMENT_POINTER;
                }
            }

            break;
        }

        case SpriteEvaluationStep::INCREMENT_POINTER: {
            if (_foregroundReadDelay) {
                _foregroundReadDelay--;
            } else {
                int16_t offsetY = int16_t(_pixelY) - int16_t(_nes.readOAM(_foregroundSpritePointer));

                if (offsetY >= 0 && offsetY < spriteSize) {
                    _statusSpriteOverflow = true;

                    _foregroundSpritePointer++;
                    _foregroundReadDelay = 3;
                } else {
                    uint8_t low = (_foregroundSpritePointer + 1) & 0x03;

                    _foregroundSpritePointer += 0x04;
                    _foregroundSpritePointer &= 0xFC;

                    if (!_foregroundSpritePointer) {
                        _foregroundEvaluationStep = SpriteEvaluationStep::IDLE;
                    }

                    _foregroundSpritePointer |= low;
                }
            }

            break;
        }

        default: _foregroundSpritePointer = 0;
        }
    }
}

void PPU::loadForegroundShifter() {
    if (_renderingEnabled) {
        _foregroundSpritePointer = 0;

        if (_pixelX == 257) {
            _foregroundDataPointer = 0;
        }

        switch (_pixelX & 0x7) {
        case 0x1: {
            _nes.getMapper()->notifyStateA12(false);

            break;
        }

        case 0x5: {
            uint8_t spriteIndex = _foregroundData[_foregroundDataPointer * 4 + 1];
            uint8_t spriteAttribute = _foregroundData[_foregroundDataPointer * 4 + 2];

            uint8_t offset = 0x00;

            if (_foregroundDataPointer < _foregroundSpriteCount) {
                offset = _pixelY - _foregroundData[_foregroundDataPointer * 4];
            }

            _foregroundSpriteAddress = 0x0000;

            if (_controlForegroundLarge) {
                _foregroundSpriteAddress = (spriteIndex & 0x01) << 12;

                if (spriteAttribute & 0x80) {
                    if (offset < 8) {
                        _foregroundSpriteAddress |= ((spriteIndex & 0xFE) + 1) << 4;
                    } else {
                        _foregroundSpriteAddress |= ((spriteIndex & 0xFE)) << 4;
                    }
                } else {
                    if (offset < 8) {
                        _foregroundSpriteAddress |= ((spriteIndex & 0xFE)) << 4;
                    } else {
                        _foregroundSpriteAddress |= ((spriteIndex & 0xFE) + 1) << 4;
                    }
                }
            } else {
                _foregroundSpriteAddress = _controlForegroundTable << 12 | spriteIndex << 4;
            }

            if (spriteAttribute & 0x80) {
                _foregroundSpriteAddress |= (7 - offset) & 0x07;
            } else {
                _foregroundSpriteAddress |= offset & 0x07;
            }

            uint8_t spritePatternLSBPlane = readAndNotifyA12(_foregroundSpriteAddress);


            if (spriteAttribute & 0x40) {
                spritePatternLSBPlane = (spritePatternLSBPlane & 0xF0) >> 4 | (spritePatternLSBPlane & 0x0F) << 4;
                spritePatternLSBPlane = (spritePatternLSBPlane & 0xCC) >> 2 | (spritePatternLSBPlane & 0x33) << 2;
                spritePatternLSBPlane = (spritePatternLSBPlane & 0xAA) >> 1 | (spritePatternLSBPlane & 0x55) << 1;
            }

            _foregroundShifter[_foregroundDataPointer * 2] = spritePatternLSBPlane;

            break;
        }

        case 0x7: {
            uint8_t spritePatternMSBPlane = readAndNotifyA12(_foregroundSpriteAddress + 8);

            if (_foregroundData[_foregroundDataPointer * 4 + 2] & 0x40) {
                spritePatternMSBPlane = (spritePatternMSBPlane & 0xF0) >> 4 | (spritePatternMSBPlane & 0x0F) << 4;
                spritePatternMSBPlane = (spritePatternMSBPlane & 0xCC) >> 2 | (spritePatternMSBPlane & 0x33) << 2;
                spritePatternMSBPlane = (spritePatternMSBPlane & 0xAA) >> 1 | (spritePatternMSBPlane & 0x55) << 1;
            }

            _foregroundShifter[_foregroundDataPointer * 2 + 1] = spritePatternMSBPlane;
            _foregroundPositions[_foregroundDataPointer] = _foregroundData[_foregroundDataPointer * 4 + 3];
            _foregroundAttributes[_foregroundDataPointer] = _foregroundData[_foregroundDataPointer * 4 + 2];

            _foregroundDataPointer++;

            break;
        }
        }
    }
}

void PPU::updateForegroundShifter() {
    if (_maskRenderForeground) {
        for (uint8_t sprite = 0; sprite < _foregroundSpriteCountNext; sprite++) {
            if (_foregroundPositions[sprite] > 0) {
                _foregroundPositions[sprite] --;
            } else {
                _foregroundShifter[sprite * 2] <<= 1;
                _foregroundShifter[sprite * 2 + 1] <<= 1;
            }
        }
    }
}

uint8_t PPU::blend() {
    if (!_renderingEnabled && (_registerV & 0x3FFF) >= 0x3F00) {
        return _registerV & 0x1F;
    }

    uint8_t backgroundPixel = 0x00;
    uint8_t backgroundPalette = 0x00;

    if (_maskRenderBackground && (_pixelX > 8 || _maskRenderBackgroundLeft)) {
        uint16_t bitMask = 0x8000 >> _scrollX;

        backgroundPixel = ((_backgroundShifter[0] & bitMask) > 0) | (((_backgroundShifter[1] & bitMask) > 0) << 1);
        backgroundPalette = ((_backgroundShifter[2] & bitMask) > 0) | (((_backgroundShifter[3] & bitMask) > 0) << 1);
    }

    uint8_t foregroundPixel = 0x00;
    uint8_t foregroundPalette = 0x00;
    uint8_t foregroundPriority = 0x00;

    if (_maskRenderForeground && (_pixelX > 8 || _maskRenderForegroundLeft)) {
        _foregroundSpriteZeroHit = false;

        for (uint8_t sprite = 0; sprite < _foregroundSpriteCountNext; sprite++) {
            if (_foregroundPositions[sprite] == 0) {
                foregroundPixel = ((_foregroundShifter[sprite * 2] & 0x80) > 0) | (((_foregroundShifter[sprite * 2 + 1] & 0x80) > 0) << 1);
                foregroundPalette = (_foregroundAttributes[sprite] & 0x03) + 0x04;
                foregroundPriority = (_foregroundAttributes[sprite] & 0x20) == 0x00;

                if (foregroundPixel != 0) {
                    if (sprite == 0 && _pixelX != 256) {
                        _foregroundSpriteZeroHit = true;
                    }

                    break;
                }
            }
        }
    }

    uint8_t finalPixel = 0x00;
    uint8_t finalPalette = 0x00;

    if (_pixelX > 0 && _pixelY < 258) {
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

            if (_foregroundSpriteZeroHit && _foregroundSpriteZeroLine && (_pixelX > 8 || _maskRenderBackgroundLeft || _maskRenderForegroundLeft)) {
                _statusSpriteZeroHit = true;
            }
        }
    }

    finalPixel |= finalPalette << 2;

    if (_maskGreyscaleMode) {
        finalPixel &= 0x30;
    }

    return finalPixel;
}



APU::APU(NES& nes) : _nes(nes) {
    _latchCycle = false;

    _delayDMA = 0x00;
    _addressDMA = 0x00;

    _pendingDMA = false;

    _openBus = 0x00;

    _frameCounterClock = 0x0000;
    _delayFrameReset = 0x0000;

    memset(_channelCounters, 0x00, 4);
    memset(_channelEnabled, false, 4);
    memset(_channelHalted, false, 4);

    _stepMode = false;

    _inhibitFrameIRQ = false;
    _sendFrameIRQ = false;

    _deltaChannelRemainingBytes = 0x0000;
    _deltaChannelSampleLength = 0x0000;
    _deltaChannelPeriodCounter = 0x0000;
    _deltaChannelPeriodLoad = 0x0000;

    _deltaChannelBitsInBuffer = 0x00;

    _deltaChannelShouldLoop = false;
    _deltaChannelEnableIRQ = false;
    _deltaChannelSampleBufferEmpty = false;

    _enableDMC = false;
    _sendDeltaChannelIRQ = false;
}

APU::~APU() { }

void APU::power() {
    _latchCycle = false;

    _delayDMA = 0x00;
    _addressDMA = 0x00;

    _pendingDMA = false;

    _openBus = 0x00;

    _frameCounterClock = 0x0000;
    _delayFrameReset = 0x0000;

    memset(_channelCounters, 0x00, 4);
    memset(_channelEnabled, false, 4);
    memset(_channelHalted, false, 4);

    _stepMode = false;

    _inhibitFrameIRQ = false;
    _sendFrameIRQ = false;

    _deltaChannelRemainingBytes = 0x0000;
    _deltaChannelSampleLength = 0x0000;
    _deltaChannelPeriodCounter = PERIOD_DMC_TABLE[0];
    _deltaChannelPeriodLoad = PERIOD_DMC_TABLE[0];

    _deltaChannelBitsInBuffer = 0x08;

    _deltaChannelShouldLoop = false;
    _deltaChannelEnableIRQ = false;
    _deltaChannelSampleBufferEmpty = true;

    _enableDMC = false;
    _sendDeltaChannelIRQ = false;
}

void APU::reset() {
    _enableDMC = false;

    memset(_channelCounters, 0x00, 4);
    memset(_channelEnabled, false, 4);

    _sendDeltaChannelIRQ = false;
    _deltaChannelRemainingBytes = 0;

    _latchCycle = false;

    _delayDMA = 0x00;
    _sendFrameIRQ = false;
    _sendDeltaChannelIRQ = false;
    _deltaChannelPeriodCounter = PERIOD_DMC_TABLE[0];
    _deltaChannelPeriodLoad = PERIOD_DMC_TABLE[0];
    _deltaChannelRemainingBytes = 0;
    _deltaChannelSampleBufferEmpty = true;
    _deltaChannelBitsInBuffer = 8;

    _nes.write(0x4015, 0x00);
    _nes.write(0x4017, _stepMode << 7 | _inhibitFrameIRQ << 6);
}

void APU::tick(bool reading, bool preventLoad) {
    if (reading) {
        performPendingDMA();
    }

    _latchCycle = !_latchCycle;

    if (_stepMode) {
        if (_delayFrameReset > 0 && --_delayFrameReset == 0) {
            _frameCounterClock = 0;
        } else if (++_frameCounterClock == 37282) {
            _frameCounterClock = 0;
        } if (_frameCounterClock == 14913 || _frameCounterClock == 37281) {
            updateCounters();
        }
    } else {
        if (_delayFrameReset > 0 && --_delayFrameReset == 0) {
            _frameCounterClock = 0;
        } else if (++_frameCounterClock == 29830) {
            _frameCounterClock = 0;

            if (!_inhibitFrameIRQ) {
                setFrameIRQ(true);
            }
        }

        if (_frameCounterClock == 14913 || _frameCounterClock == 29829) {
            updateCounters();
        }

        if (_frameCounterClock >= 29828 && !_inhibitFrameIRQ) {
            setFrameIRQ(true);
        }
    }

    if (--_deltaChannelPeriodCounter == 0) {
        _deltaChannelPeriodCounter = _deltaChannelPeriodLoad;

        if (--_deltaChannelBitsInBuffer == 0) {
            _deltaChannelBitsInBuffer = 8;

            if (!_deltaChannelSampleBufferEmpty) {
                _deltaChannelSampleBufferEmpty = true;
            }

            if (_deltaChannelRemainingBytes > 0 && !preventLoad) {
                loadDeltaChannelByte(reading);
            }
        }
    }
}

void APU::write(uint8_t address, uint8_t value) {
    _openBus = value;

    switch (address) {
    case Register::PULSE_1_0: _channelHalted[0x0] = value & 0x20; break;
    case Register::PULSE_1_3: if (_channelEnabled[0x0]) _channelCounters[0x0] = LENGTH_COUNTER_TABLE[value >> 3]; break;
    case Register::PULSE_2_0: _channelHalted[0x1] = value & 0x20; break;
    case Register::PULSE_2_3: if (_channelEnabled[0x1]) _channelCounters[0x1] = LENGTH_COUNTER_TABLE[value >> 3]; break;
    case Register::TRIANGLE_0: _channelHalted[0x2] = value & 0x80; break;
    case Register::TRIANGLE_3: if (_channelEnabled[0x2]) _channelCounters[0x2] = LENGTH_COUNTER_TABLE[value >> 3]; break;
    case Register::NOISE_0: _channelHalted[0x3] = value & 0x20; break;
    case Register::NOISE_3: if (_channelEnabled[0x3]) _channelCounters[0x3] = LENGTH_COUNTER_TABLE[value >> 3]; break;
    case Register::OAM_DMA: performDMA(value); break;
    case Register::DELTA_3: _deltaChannelSampleLength = (value << 4) + 1; break;

    case Register::DELTA_0: {
        _deltaChannelEnableIRQ = value & 0x80;
        _deltaChannelShouldLoop = value & 0x40;
        _deltaChannelPeriodLoad = PERIOD_DMC_TABLE[value & 0x0F];

        if (!_deltaChannelEnableIRQ) {
            setDeltaIRQ(false);
        }

        break;
    }

    case Register::CTRL_STATUS: {
        _enableDMC = value & 0x10;

        for (uint8_t channel = 0; channel < 0x4; channel++) {
            _channelEnabled[channel] = value & (1 << channel);

            if (!_channelEnabled[channel]) {
                _channelCounters[channel] = 0;
            }
        }

        setDeltaIRQ(false);

        if (!_enableDMC) {
            _deltaChannelRemainingBytes = 0;
        } else {
            if (_deltaChannelRemainingBytes == 0) {
                _deltaChannelRemainingBytes = _deltaChannelSampleLength;
                if (_deltaChannelSampleBufferEmpty) {
                    loadDeltaChannelByte(false);
                }
            }
        }

        break;
    }

    case Register::FRAME_COUNTER: {
        _stepMode = value & 0x80;
        _inhibitFrameIRQ = value & 0x40;

        if (_inhibitFrameIRQ) {
            setFrameIRQ(false);
        }

        _delayFrameReset = _latchCycle ? 4 : 3;

        if (_stepMode) {
            updateCounters();
        }

        break;
    }
    }
}

uint8_t APU::read(uint8_t address) {
    if (address == Register::CTRL_STATUS) {
        _openBus = _sendDeltaChannelIRQ << 7;
        _openBus |= _sendFrameIRQ << 6;
        _openBus |= (_deltaChannelRemainingBytes > 0) << 4;

        for (uint8_t channel = 0; channel < 0x4; channel++) {
            _openBus |= (_channelCounters[channel] > 0) << channel;
        }

        setFrameIRQ(false);
    }

    return _openBus;
}

void APU::updateCounters() {
    for (uint8_t channel = 0; channel < 0x4; channel++) {
        if (!_channelHalted[channel] && _channelCounters[channel] > 0) {
            _channelCounters[channel]--;
        }
    }
}

void APU::loadDeltaChannelByte(bool reading) {
    uint8_t delay = _delayDMA;

    if (delay == 0) {
        if (reading) {
            delay = 0x4;
        } else {
            delay = 0x3;
        }
    }

    for (uint8_t i = 0; i < delay; i++) {
        tick(false, true);

        _nes.getPPU()->tick();
        _nes.getPPU()->tick();
        _nes.getPPU()->tick();
        _nes.getCPU()->poll();
    }

    _deltaChannelSampleBufferEmpty = false;

    if (--_deltaChannelRemainingBytes == 0) {
        if (_deltaChannelShouldLoop) {
            _deltaChannelRemainingBytes = _deltaChannelSampleLength;
        } else if (_deltaChannelEnableIRQ) {
            setDeltaIRQ(true);
        }
    }
}

void APU::performDMA(uint8_t address) {
    _addressDMA = address;
    _pendingDMA = true;
}

void APU::performPendingDMA() {
    if (!_pendingDMA) {
        return;
    }

    _pendingDMA = false;
    _delayDMA = 0x2;

    if (!_latchCycle) {
        _nes.dummyRead();
    }

    _nes.dummyRead();

    uint16_t currentAddress = _addressDMA << 8;
    uint8_t lowByte = 0x00;

    _nes.write(0x2004, _nes.read(currentAddress++));

    while ((lowByte = currentAddress & 0xFF) != 0) {
        uint8_t value = _nes.read(currentAddress++);

        if (lowByte == 254) {
            _delayDMA = 0x1;

            _nes.write(0x2004, value);

            _delayDMA = 0x2;
        } else if (lowByte == 255) {
            _delayDMA = 0x3;

            _nes.write(0x2004, value);

            _delayDMA = 0x0;
        } else {
            _nes.write(0x2004, value);
        }
    }
}

void APU::setFrameIRQ(bool irq) {
    _sendFrameIRQ = irq;

    _nes.getCPU()->setFrameIRQ(irq);
}

void APU::setDeltaIRQ(bool irq) {
    _sendDeltaChannelIRQ = irq;

    _nes.getCPU()->setDeltaIRQ(irq);
}



Mapper::Mapper(NES& nes, uint8_t bankSizePRG, uint8_t bankSizeCHR, uint8_t bankCountPRG, uint8_t bankCountCHR, MirroringMode mode) :
    _nes(nes), BANK_SIZE_PRG(bankSizePRG), BANK_SIZE_CHR(bankSizeCHR), BANK_COUNT_PRG(bankCountPRG), BANK_COUNT_CHR(bankCountCHR), _mode(mode),
    BANK_MASK_PRG(0xFFFF >> (16 - bankSizePRG)), BANK_MASK_CHR(0xFFFF >> (16 - bankSizeCHR)) {
    if (BANK_SIZE_PRG > 15 || BANK_SIZE_CHR > 13) {
        throw "Invalid bank size";
    }

    _bankIndexPRG = new uint32_t[0x8000 >> BANK_SIZE_PRG]{ 0x00 };
    _bankIndexCHR = new uint32_t[0x2000 >> BANK_SIZE_CHR]{ 0x00 };

    _writableRAM = true;
}

Mapper::~Mapper() {
    delete[] _bankIndexPRG;
    delete[] _bankIndexCHR;
}

uint16_t Mapper::getMirroredAddress(uint16_t address) const {
    switch (_mode) {
    case MirroringMode::ONE_SCREEN_LOW: return address & 0x3FF;
    case MirroringMode::ONE_SCREEN_HIGH: return (address & 0x3FF) + 0xC00;
    case MirroringMode::VERTICAL: return address & 0x7FF;
    case MirroringMode::HORIZONTAL: return address & 0xBFF;
    default: return address & 0xFFF;
    }
}

uint32_t Mapper::getAddressCPU(uint16_t address) const {
    return _bankIndexPRG[address >> BANK_SIZE_PRG] + (address & BANK_MASK_PRG);
}

uint32_t Mapper::getAddressPPU(uint16_t address) const {
    return _bankIndexCHR[address >> BANK_SIZE_CHR] + (address & BANK_MASK_CHR);
}

bool Mapper::isRAMWritable() const {
    return _writableRAM;
}


Mapper000::Mapper000(NES& nes, uint8_t bankCountPRG, uint8_t bankCountCHR, MirroringMode mode) :
    Mapper(nes, 14, 13, bankCountPRG, bankCountCHR, mode) {
    _bankIndexPRG[0x0] = 0x0000;

    if (bankCountPRG > 1) {
        _bankIndexPRG[0x1] = 0x4000;
    } else {
        _bankIndexPRG[0x1] = 0x0000;
    }
}

Mapper000::~Mapper000() { }


Mapper001::Mapper001(NES& nes, uint8_t bankCountPRG, uint8_t bankCountCHR, MirroringMode mode) :
    Mapper(nes, 14, 12, bankCountPRG, bankCountCHR, mode) {
    _bankIndexPRG[0x0] = 0x0000;
    _bankIndexPRG[0x1] = (bankCountPRG - 1) * 0x4000;

    _counter = 0x00;
    _registerControl = 0x00;
    _registerLoad = 0x00;
}

Mapper001::~Mapper001() {}

void Mapper001::writeCPU(uint16_t address, uint8_t value) {
    if (value & 0x80) {
        _registerLoad = 0x00;

        _counter = 0;
    } else {
        _registerLoad >>= 1;
        _registerLoad |= (value & 0x01) << 4;

        if (++_counter == 5) {
            uint8_t registerTarget = (address >> 13) & 0x03;

            switch (registerTarget) {
            case 0: {
                _registerControl = _registerLoad & 0x1F;

                switch (_registerControl & 0x03) {
                case 0: _mode = MirroringMode::ONE_SCREEN_LOW; break;
                case 1: _mode = MirroringMode::ONE_SCREEN_HIGH; break;
                case 2: _mode = MirroringMode::VERTICAL; break;
                case 3: _mode = MirroringMode::HORIZONTAL; break;
                }

                break;
            }

            case 1: {
                if (_registerControl & 0x10) {
                    _bankIndexCHR[0x0] = (_registerLoad & 0x1F) * 0x1000;
                } else {
                    _bankIndexCHR[0x0] = (_registerLoad & 0x1E) * 0x1000;
                    _bankIndexCHR[0x1] = (_registerLoad & 0x1E) * 0x1000 + 0x1000;
                }

                break;
            }

            case 2: {
                if (_registerControl & 0x10) {
                    _bankIndexCHR[0x1] = (_registerLoad & 0x1F) * 0x1000;
                }

                break;
            }

            case 3: {
                if (_registerControl & 0x08) {
                    if (_registerControl & 0x04) {
                        _bankIndexPRG[0x0] = (_registerLoad & 0x0F) * 0x4000;
                        _bankIndexPRG[0x1] = (BANK_COUNT_PRG - 1) * 0x4000;
                    } else {
                        _bankIndexPRG[0x1] = (_registerLoad & 0xF) * 0x4000;
                    }
                } else {
                    _bankIndexPRG[0x0] = (_registerLoad & 0x0E) * 0x4000;
                    _bankIndexPRG[0x1] = (_registerLoad & 0x0E) * 0x4000 + 0x4000;
                }

                _writableRAM = ~_registerLoad & 0x10;

                break;
            }
            }

            _registerLoad = 0x00;
            _counter = 0x00;
        }
    }
}

Mapper002::Mapper002(NES& nes, uint8_t bankCountPRG, uint8_t bankCountCHR, MirroringMode mode) :
    Mapper(nes, 14, 13, bankCountPRG, bankCountCHR, mode) {
    _bankIndexPRG[0x0] = 0x0000;
    _bankIndexPRG[0x1] = (BANK_COUNT_PRG - 1) * 0x4000;
}

Mapper002::~Mapper002() { }

void Mapper002::writeCPU(uint16_t address, uint8_t value) {
    _bankIndexPRG[0x0] = value * 0x4000;
}


Mapper003::Mapper003(NES& nes, uint8_t bankCountPRG, uint8_t bankCountCHR, MirroringMode mode) :
    Mapper(nes, 14, 13, bankCountPRG, bankCountCHR, mode) {
    _bankIndexPRG[0x0] = 0x0000;

    if (bankCountPRG > 1) {
        _bankIndexPRG[0x1] = 0x4000;
    } else {
        _bankIndexPRG[0x1] = 0x0000;
    }
}

Mapper003::~Mapper003() { }

void Mapper003::writeCPU(uint16_t address, uint8_t value) {
    _bankIndexCHR[0x0] = value * 0x2000;
}


Mapper004::Mapper004(NES& nes, uint8_t bankCountPRG, uint8_t bankCountCHR, MirroringMode mode) :
    Mapper(nes, 13, 10, bankCountPRG, bankCountCHR, mode) {
    _bankIndexPRG[0x0] = 0x0000;
    _bankIndexPRG[0x1] = 0x2000;
    _bankIndexPRG[0x2] = (bankCountPRG * 2 - 2) * 0x2000;
    _bankIndexPRG[0x3] = (bankCountPRG * 2 - 1) * 0x2000;

    memset(_registers, 0x0000, 0x20);

    _tick = 0x0000;
    _counter = 0x0000;
    _counterReload = 0x0000;

    _targetRegister = 0x00;

    _modePRG = false;
    _modeCHR = false;

    _enableIRQ = false;
    _shouldReloadIRQ = false;
}

Mapper004::~Mapper004() { }

void Mapper004::tick() {
    if (_tick > 0 && _tick < 11) {
        _tick++;
    }
}

void Mapper004::notifyStateA12(bool state) {
    if (state) {
        if (_tick > 10) {
            if (_counter == 0 || _shouldReloadIRQ) {
                _counter = _counterReload;
            } else {
                _counter--;
            }

            if (_counter == 0 && _enableIRQ) {
                _nes.getCPU()->setMapperIRQ(true);
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

void Mapper004::writeCPU(uint16_t address, uint8_t value) {
    if (address < 0x2000) {
        if (address & 0x1) {
            if (_targetRegister < 2) {
                value &= 0xFE;
            }

            _registers[_targetRegister] = value;

            if (_modePRG) {
                _bankIndexPRG[0x2] = (_registers[0x6] & 0x3F) * 0x2000;
                _bankIndexPRG[0x0] = (BANK_COUNT_PRG * 2 - 2) * 0x2000;
            } else {
                _bankIndexPRG[0x0] = (_registers[0x6] & 0x3F) * 0x2000;
                _bankIndexPRG[0x2] = (BANK_COUNT_PRG * 2 - 2) * 0x2000;
            }

            _bankIndexPRG[0x1] = (_registers[0x7] & 0x3F) * 0x2000;
            _bankIndexPRG[0x3] = (BANK_COUNT_PRG * 2 - 1) * 0x2000;

            if (_modeCHR) {
                _bankIndexCHR[0x0] = _registers[0x2] * 0x0400;
                _bankIndexCHR[0x1] = _registers[0x3] * 0x0400;
                _bankIndexCHR[0x2] = _registers[0x4] * 0x0400;
                _bankIndexCHR[0x3] = _registers[0x5] * 0x0400;
                _bankIndexCHR[0x4] = (_registers[0x0] & 0xFE) * 0x0400;
                _bankIndexCHR[0x5] = _registers[0x0] * 0x0400 + 0x0400;
                _bankIndexCHR[0x6] = (_registers[0x1] & 0xFE) * 0x0400;
                _bankIndexCHR[0x7] = _registers[0x1] * 0x0400 + 0x0400;
            } else {
                _bankIndexCHR[0x0] = (_registers[0x0] & 0xFE) * 0x0400;
                _bankIndexCHR[0x1] = _registers[0x0] * 0x0400 + 0x0400;
                _bankIndexCHR[0x2] = (_registers[0x1] & 0xFE) * 0x0400;
                _bankIndexCHR[0x3] = _registers[0x1] * 0x0400 + 0x0400;
                _bankIndexCHR[0x4] = _registers[0x2] * 0x0400;
                _bankIndexCHR[0x5] = _registers[0x3] * 0x0400;
                _bankIndexCHR[0x6] = _registers[0x4] * 0x0400;
                _bankIndexCHR[0x7] = _registers[0x5] * 0x0400;
            }
        } else {
            _targetRegister = value & 0x07;
            _modePRG = value & 0x40;
            _modeCHR = value & 0x80;
        }
    } else if (address < 0x4000) {
        if (address & 0x1) {
            _writableRAM = ~value & 0x40;
        } else  if (value & 0x1) {
            _mode = MirroringMode::HORIZONTAL;
        } else {
            _mode = MirroringMode::VERTICAL;
        }
    } else if (address < 0x6000) {
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
            _nes.getCPU()->setMapperIRQ(false);
        }
    }
}