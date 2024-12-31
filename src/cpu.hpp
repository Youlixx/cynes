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

    void (CPU::* _addressingModes[256]) (void);

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

    void (CPU::* _instructions[256]) (void);

public:
    template<DumpOperation operation, class T>
    void dump(T& buffer) {
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
