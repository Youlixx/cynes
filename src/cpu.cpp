#include "cpu.hpp"
#include "nes.hpp"

#include <cstring>


cynes::CPU::CPU(NES& nes) : _nes(nes), _addressingModes(), _instructions() {
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

    // TODO could be moved in the header?
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

cynes::CPU::~CPU() { }

void cynes::CPU::power() {
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

void cynes::CPU::reset() {
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

void cynes::CPU::tick() {
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

void cynes::CPU::poll() {
    _delayNMI = _shouldNMI;

    if (!_edgeDetectorNMI && _lineNMI) {
        _shouldNMI = true;
    }

    _edgeDetectorNMI = _lineNMI;
    _delayIRQ = _shouldIRQ;

    _shouldIRQ = (_lineMapperIRQ || _lineFrameIRQ || _lineDeltaIRQ) && !getStatus(Flag::I);
}

void cynes::CPU::setNMI(bool nmi) {
    _lineNMI = nmi;
}

void cynes::CPU::setMapperIRQ(bool irq) {
    _lineMapperIRQ = irq;
}

void cynes::CPU::setFrameIRQ(bool irq) {
    _lineFrameIRQ = irq;
}

void cynes::CPU::setDeltaIRQ(bool irq) {
    _lineDeltaIRQ = irq;
}

bool cynes::CPU::isFrozen() const {
    return _frozen;
}

uint8_t cynes::CPU::fetch() {
    return _nes.read(_programCounter++);
}

void cynes::CPU::setStatus(uint8_t flag, bool value) {
    if (value) {
        _status |= flag;
    } else {
        _status &= ~flag;
    }
}

bool cynes::CPU::getStatus(uint8_t flag) const {
    return _status & flag;
}

void cynes::CPU::ABR() {
    ABW();

    _registerM = _nes.read(_targetAddress);
}

void cynes::CPU::ABW() {
    _targetAddress = fetch();
    _targetAddress |= fetch() << 8;
}

void cynes::CPU::ACC() {
    _registerM = _nes.read(_programCounter);
}

void cynes::CPU::AXM() {
    AXW();

    _registerM = _nes.read(_targetAddress);
}

void cynes::CPU::AXR() {
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

void cynes::CPU::AXW() {
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

void cynes::CPU::AYM() {
    AYW();

    _registerM = _nes.read(_targetAddress);
}

void cynes::CPU::AYR() {
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

void cynes::CPU::AYW() {
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

void cynes::CPU::IMM() {
    _registerM = fetch();
}

void cynes::CPU::IMP() {
    _registerM = _nes.read(_programCounter);
}

void cynes::CPU::IND() {
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

void cynes::CPU::IXR() {
    IXW();

    _registerM = _nes.read(_targetAddress);
}

void cynes::CPU::IXW() {
    uint8_t pointer = fetch();

    _registerM = _nes.read(pointer);

    pointer += _registerX;

    _targetAddress = _nes.read(pointer);
    _targetAddress |= _nes.read(++pointer & 0xFF) << 8;
}

void cynes::CPU::IYM() {
    IYW();

    _registerM = _nes.read(_targetAddress);
}

void cynes::CPU::IYR() {
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

void cynes::CPU::IYW() {
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

void cynes::CPU::REL() {
    _targetAddress = fetch();

    if (_targetAddress & 0x80) {
        _targetAddress |= 0xFF00;
    }
}

void cynes::CPU::ZPR() {
    ZPW();

    _registerM = _nes.read(_targetAddress);
}

void cynes::CPU::ZPW() {
    _targetAddress = fetch();
}

void cynes::CPU::ZXR() {
    ZXW();

    _registerM = _nes.read(_targetAddress);
}

void cynes::CPU::ZXW() {
    _targetAddress = fetch();

    _registerM = _nes.read(_targetAddress);

    _targetAddress += _registerX;
    _targetAddress &= 0x00FF;
}

void cynes::CPU::ZYR() {
    ZYW();

    _registerM = _nes.read(_targetAddress);
}

void cynes::CPU::ZYW() {
    _targetAddress = fetch();

    _registerM = _nes.read(_targetAddress);

    _targetAddress += _registerY;
    _targetAddress &= 0x00FF;
}

void cynes::CPU::AAL() {
    setStatus(Flag::C, _registerA & 0x80);

    _registerA <<= 1;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void cynes::CPU::ADC() {
    uint16_t result = _registerA + _registerM + (getStatus(Flag::C) ? 0x01 : 0x00);

    setStatus(Flag::C, result & 0xFF00);
    setStatus(Flag::V, ~(_registerA ^ _registerM) & (_registerA ^ result) & 0x80);

    _registerA = result & 0x00FF;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void cynes::CPU::ALR() {
    _registerA &= _registerM;

    setStatus(Flag::C, _registerA & 0x01);

    _registerA >>= 1;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void cynes::CPU::ANC() {
    _registerA &= _registerM;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
    setStatus(Flag::C, _registerA & 0x80);
}

void cynes::CPU::AND() {
    _registerA &= _registerM;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void cynes::CPU::ANE() {
    _registerA = (_registerA | 0xEE) & _registerX & _registerM;
}

void cynes::CPU::ARR() {
    _registerA &= _registerM;

    _registerA = (getStatus(Flag::C) ? 0x80 : 0x00) | (_registerA >> 1);

    setStatus(Flag::C, _registerA & 0x40);
    setStatus(Flag::V, bool(_registerA & 0x40) ^ bool(_registerA & 0x20));
    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void cynes::CPU::ASL() {
    _nes.write(_targetAddress, _registerM);

    setStatus(Flag::C, _registerM & 0x80);

    _registerM <<= 1;

    setStatus(Flag::Z, !_registerM);
    setStatus(Flag::N, _registerM & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void cynes::CPU::BCC() {
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

void cynes::CPU::BCS() {
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

void cynes::CPU::BEQ() {
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

void cynes::CPU::BIT() {
    setStatus(Flag::Z, !(_registerA & _registerM));
    setStatus(Flag::V, _registerM & 0x40);
    setStatus(Flag::N, _registerM & 0x80);
}

void cynes::CPU::BMI() {
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

void cynes::CPU::BNE() {
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

void cynes::CPU::BPL() {
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

void cynes::CPU::BRK() {
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

void cynes::CPU::BVC() {
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

void cynes::CPU::BVS() {
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

void cynes::CPU::CLC() {
    setStatus(Flag::C, false);
}

void cynes::CPU::CLD() {
    setStatus(Flag::D, false);
}

void cynes::CPU::CLI() {
    setStatus(Flag::I, false);
}

void cynes::CPU::CLV() {
    setStatus(Flag::V, false);
}

void cynes::CPU::CMP() {
    setStatus(Flag::C, _registerA >= _registerM);
    setStatus(Flag::Z, _registerA == _registerM);
    setStatus(Flag::N, (_registerA - _registerM) & 0x80);
}

void cynes::CPU::CPX() {
    setStatus(Flag::C, _registerX >= _registerM);
    setStatus(Flag::Z, _registerX == _registerM);
    setStatus(Flag::N, (_registerX - _registerM) & 0x80);
}

void cynes::CPU::CPY() {
    setStatus(Flag::C, _registerY >= _registerM);
    setStatus(Flag::Z, _registerY == _registerM);
    setStatus(Flag::N, (_registerY - _registerM) & 0x80);
}

void cynes::CPU::DCP() {
    _nes.write(_targetAddress, _registerM);

    _registerM--;

    setStatus(Flag::C, _registerA >= _registerM);
    setStatus(Flag::Z, _registerA == _registerM);
    setStatus(Flag::N, (_registerA - _registerM) & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void cynes::CPU::DEC() {
    _nes.write(_targetAddress, _registerM);

    _registerM--;

    setStatus(Flag::Z, !_registerM);
    setStatus(Flag::N, _registerM & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void cynes::CPU::DEX() {
    _registerX--;

    setStatus(Flag::Z, !_registerX);
    setStatus(Flag::N, _registerX & 0x80);
}

void cynes::CPU::DEY() {
    _registerY--;

    setStatus(Flag::Z, !_registerY);
    setStatus(Flag::N, _registerY & 0x80);
}

void cynes::CPU::EOR() {
    _registerA ^= _registerM;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void cynes::CPU::INC() {
    _nes.write(_targetAddress, _registerM);

    _registerM++;

    setStatus(Flag::Z, !_registerM);
    setStatus(Flag::N, _registerM & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void cynes::CPU::INX() {
    _registerX++;

    setStatus(Flag::Z, !_registerX);
    setStatus(Flag::N, _registerX & 0x80);
}

void cynes::CPU::INY() {
    _registerY++;

    setStatus(Flag::Z, !_registerY);
    setStatus(Flag::N, _registerY & 0x80);
}

void cynes::CPU::ISC() {
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

void cynes::CPU::JAM() {
    _frozen = true;
}

void cynes::CPU::JMP() {
    _programCounter = _targetAddress;
}

void cynes::CPU::JSR() {
    _nes.read(_programCounter);

    _programCounter--;

    _nes.write(0x100 | _stackPointer--, _programCounter >> 8);
    _nes.write(0x100 | _stackPointer--, _programCounter & 0x00FF);

    _programCounter = _targetAddress;
}

void cynes::CPU::LAR() {
    setStatus(Flag::C, _registerA & 0x01);

    _registerA >>= 1;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void cynes::CPU::LAS() {
    uint8_t result = _registerM & _stackPointer;

    _registerA = result;
    _registerX = result;
    _stackPointer = result;
}

void cynes::CPU::LAX() {
    _registerA = _registerM;
    _registerX = _registerM;

    setStatus(Flag::Z, !_registerM);
    setStatus(Flag::N, _registerM & 0x80);
}

void cynes::CPU::LDA() {
    _registerA = _registerM;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void cynes::CPU::LDX() {
    _registerX = _registerM;

    setStatus(Flag::Z, !_registerX);
    setStatus(Flag::N, _registerX & 0x80);
}

void cynes::CPU::LDY() {
    _registerY = _registerM;

    setStatus(Flag::Z, !_registerY);
    setStatus(Flag::N, _registerY & 0x80);
}

void cynes::CPU::LSR() {
    _nes.write(_targetAddress, _registerM);

    setStatus(Flag::C, _registerM & 0x01);

    _registerM >>= 1;

    setStatus(Flag::Z, !_registerM);
    setStatus(Flag::N, _registerM & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void cynes::CPU::LXA() {
    _registerA = _registerM;
    _registerX = _registerM;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void cynes::CPU::NOP() {

}

void cynes::CPU::ORA() {
    _registerA |= _registerM;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void cynes::CPU::PHA() {
    _nes.write(0x100 | _stackPointer--, _registerA);
}

void cynes::CPU::PHP() {
    _nes.write(0x100 | _stackPointer--, _status | Flag::B | Flag::U);
}

void cynes::CPU::PLA() {
    _stackPointer++;

    _nes.read(_programCounter);

    _registerA = _nes.read(0x100 | _stackPointer);

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void cynes::CPU::PLP() {
    _stackPointer++;

    _nes.read(_programCounter);

    _status = _nes.read(0x100 | _stackPointer) & 0xCF;
}

void cynes::CPU::RAL() {
    bool carry = _registerA & 0x80;

    _registerA = (getStatus(Flag::C) ? 0x01 : 0x00) | (_registerA << 1);

    setStatus(Flag::C, carry);
    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void cynes::CPU::RAR() {
    bool carry = _registerA & 0x01;

    _registerA = (getStatus(Flag::C) ? 0x80 : 0x00) | (_registerA >> 1);

    setStatus(Flag::C, carry);
    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void cynes::CPU::RLA() {
    _nes.write(_targetAddress, _registerM);

    bool carry = _registerM & 0x80;

    _registerM = (getStatus(Flag::C) ? 0x01 : 0x00) | (_registerM << 1);
    _registerA &= _registerM;

    setStatus(Flag::C, carry);
    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void cynes::CPU::ROL() {
    _nes.write(_targetAddress, _registerM);

    bool carry = _registerM & 0x80;

    _registerM = (getStatus(Flag::C) ? 0x01 : 0x00) | (_registerM << 1);

    setStatus(Flag::C, carry);
    setStatus(Flag::Z, !_registerM);
    setStatus(Flag::N, _registerM & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void cynes::CPU::ROR() {
    _nes.write(_targetAddress, _registerM);

    bool carry = _registerM & 0x01;

    _registerM = (getStatus(Flag::C) ? 0x80 : 0x00) | (_registerM >> 1);

    setStatus(Flag::C, carry);
    setStatus(Flag::Z, !_registerM);
    setStatus(Flag::N, _registerM & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void cynes::CPU::RRA() {
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

void cynes::CPU::RTI() {
    _stackPointer++;

    _nes.read(_programCounter);

    _status = _nes.read(0x100 | _stackPointer) & 0xCF;
    _programCounter = _nes.read(0x100 | ++_stackPointer);
    _programCounter |= _nes.read(0x100 | ++_stackPointer) << 8;
}

void cynes::CPU::RTS() {
    _stackPointer++;

    _nes.read(_programCounter);
    _nes.read(_programCounter);

    _programCounter = _nes.read(0x100 | _stackPointer);
    _programCounter |= _nes.read(0x100 | ++_stackPointer) << 8;

    _programCounter++;
}

void cynes::CPU::SAX() {
    _nes.write(_targetAddress, _registerA & _registerX);
}

void cynes::CPU::SBC() {
    _registerM ^= 0xFF;

    uint16_t result = _registerA + _registerM + (getStatus(Flag::C) ? 0x01 : 0x00);

    setStatus(Flag::C, result & 0xFF00);
    setStatus(Flag::V, ~(_registerA ^ _registerM) & (_registerA ^ result) & 0x80);

    _registerA = result & 0x00FF;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void cynes::CPU::SBX() {
    _registerX &= _registerA;

    setStatus(Flag::C, _registerX >= _registerM);
    setStatus(Flag::Z, _registerX == _registerM);

    _registerX -= _registerM;

    setStatus(Flag::N, _registerX & 0x80);
}

void cynes::CPU::SEC() {
    setStatus(Flag::C, true);
}

void cynes::CPU::SED() {
    setStatus(Flag::D, true);
}

void cynes::CPU::SEI() {
    setStatus(Flag::I, true);
}

void cynes::CPU::SHA() {
    _nes.write(_targetAddress, _registerA & _registerX & (uint8_t(_targetAddress >> 8) + 1));
}

void cynes::CPU::SHX() {
    uint8_t addressHigh = 1 + (_targetAddress >> 8);

    _nes.write(((_registerX & addressHigh) << 8) | (_targetAddress & 0xFF), _registerX & addressHigh);
}

void cynes::CPU::SHY() {
    uint8_t addressHigh = 1 + (_targetAddress >> 8);

    _nes.write(((_registerY & addressHigh) << 8) | (_targetAddress & 0xFF), _registerY & addressHigh);
}

void cynes::CPU::SLO() {
    _nes.write(_targetAddress, _registerM);

    setStatus(Flag::C, _registerM & 0x80);

    _registerM <<= 1;
    _registerA |= _registerM;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void cynes::CPU::SRE() {
    _nes.write(_targetAddress, _registerM);

    setStatus(Flag::C, _registerM & 0x01);

    _registerM >>= 1;
    _registerA ^= _registerM;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);

    _nes.write(_targetAddress, _registerM);
}

void cynes::CPU::STA() {
    _nes.write(_targetAddress, _registerA);
}

void cynes::CPU::STX() {
    _nes.write(_targetAddress, _registerX);
}

void cynes::CPU::STY() {
    _nes.write(_targetAddress, _registerY);
}

void cynes::CPU::TAS() {
    _stackPointer = _registerA & _registerX;

    _nes.write(_targetAddress, _stackPointer & (uint8_t(_targetAddress >> 8) + 1));
}

void cynes::CPU::TAX() {
    _registerX = _registerA;

    setStatus(Flag::Z, !_registerX);
    setStatus(Flag::N, _registerX & 0x80);
}

void cynes::CPU::TAY() {
    _registerY = _registerA;

    setStatus(Flag::Z, !_registerY);
    setStatus(Flag::N, _registerY & 0x80);
}

void cynes::CPU::TSX() {
    _registerX = _stackPointer;

    setStatus(Flag::Z, !_registerX);
    setStatus(Flag::N, _registerX & 0x80);
}

void cynes::CPU::TXA() {
    _registerA = _registerX;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void cynes::CPU::TXS() {
    _stackPointer = _registerX;
}

void cynes::CPU::TYA() {
    _registerA = _registerY;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}

void cynes::CPU::USB() {
    _registerM ^= 0xFF;

    uint16_t result = _registerA + _registerM + (getStatus(Flag::C) ? 0x01 : 0x00);

    setStatus(Flag::C, result & 0x0100);
    setStatus(Flag::V, ~(_registerA ^ _registerM) & (_registerA ^ result) & 0x80);

    _registerA = result & 0x00FF;

    setStatus(Flag::Z, !_registerA);
    setStatus(Flag::N, _registerA & 0x80);
}
