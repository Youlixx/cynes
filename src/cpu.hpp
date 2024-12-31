#ifndef __CYNES_CPU__
#define __CYNES_CPU__

#include <cstdint>

#include "utils.hpp"

namespace cynes {
class NES;

class CPU {
public:
    CPU(NES& nes);
    ~CPU();

public:
    void power();
    void reset();

    void tick();
    void poll();

    void setNMI(bool nmi);
    void setMapperIRQ(bool irq);
    void setFrameIRQ(bool irq);
    void setDeltaIRQ(bool irq);

    bool isFrozen() const;

private:
    NES& _nes;

private:
    bool _frozen;

    uint8_t _registerA;
    uint8_t _registerX;
    uint8_t _registerY;
    uint8_t _registerM;
    uint8_t _stackPointer;

    uint16_t _programCounter;

    uint8_t fetch();

private:
    bool _delayIRQ;
    bool _shouldIRQ;

    bool _lineMapperIRQ;
    bool _lineFrameIRQ;
    bool _lineDeltaIRQ;

    bool _lineNMI;
    bool _edgeDetectorNMI;

    bool _delayNMI;
    bool _shouldNMI;

private:
    uint8_t _status;

    void setStatus(uint8_t flag, bool value);
    bool getStatus(uint8_t flag) const;

    enum Flag : uint8_t {
        C = 0x01, Z = 0x02, I = 0x04, D = 0x08, B = 0x10, U = 0x20, V = 0x40, N = 0x80
    };

private:
    uint16_t _targetAddress;

    void ABR(); void ABW(); void ACC(); void AXM(); void AXR(); void AXW(); void AYM(); void AYR();
    void AYW(); void IMM(); void IMP(); void IND(); void IXR(); void IXW(); void IYM(); void IYR();
    void IYW(); void REL(); void ZPR(); void ZPW(); void ZXR(); void ZXW(); void ZYR(); void ZYW();

    void (CPU::* _addressingModes[256]) (void) = {
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

private:
    void AAL(); void ADC(); void ALR(); void ANC(); void AND(); void ANE(); void ARR(); void ASL();
    void BCC(); void BCS(); void BEQ(); void BIT(); void BMI(); void BNE(); void BPL(); void BRK();
    void BVC(); void BVS(); void CLC(); void CLD(); void CLI(); void CLV(); void CMP(); void CPX();
    void CPY(); void DCP(); void DEC(); void DEX(); void DEY(); void EOR(); void INC(); void INX();
    void INY(); void ISC(); void JAM(); void JMP(); void JSR(); void LAR(); void LAS(); void LAX();
    void LDA(); void LDX(); void LDY(); void LSR(); void LXA(); void NOP(); void ORA(); void PHA();
    void PHP(); void PLA(); void PLP(); void RAL(); void RAR(); void RLA(); void ROL(); void ROR();
    void RRA(); void RTI(); void RTS(); void SAX(); void SBC(); void SBX(); void SEC(); void SED();
    void SEI(); void SHA(); void SHX(); void SHY(); void SLO(); void SRE(); void STA(); void STX();
    void STY(); void TAS(); void TAX(); void TAY(); void TSX(); void TXA(); void TXS(); void TYA();
    void USB();

    void (CPU::* _instructions[256]) (void) = {
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

public:
    template<DumpOperation operation, typename T>
    constexpr void dump(T& buffer) {
        cynes::dump<operation>(buffer, _frozen);
        cynes::dump<operation>(buffer, _registerA);
        cynes::dump<operation>(buffer, _registerX);
        cynes::dump<operation>(buffer, _registerY);
        cynes::dump<operation>(buffer, _registerM);
        cynes::dump<operation>(buffer, _stackPointer);
        cynes::dump<operation>(buffer, _programCounter);
        cynes::dump<operation>(buffer, _targetAddress);
        cynes::dump<operation>(buffer, _status);

        cynes::dump<operation>(buffer, _delayIRQ);
        cynes::dump<operation>(buffer, _shouldIRQ);
        cynes::dump<operation>(buffer, _lineMapperIRQ);
        cynes::dump<operation>(buffer, _lineFrameIRQ);
        cynes::dump<operation>(buffer, _lineDeltaIRQ);
        cynes::dump<operation>(buffer, _lineNMI);
        cynes::dump<operation>(buffer, _edgeDetectorNMI);
        cynes::dump<operation>(buffer, _delayNMI);
        cynes::dump<operation>(buffer, _shouldNMI);
    }
};
}

#endif
