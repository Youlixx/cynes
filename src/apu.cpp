#include "apu.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "nes.hpp"

#include <cstring>


cynes::APU::APU(NES& nes)
: _nes{nes}
, _latchCycle{false}
, _delayDMA{0x00}
, _addressDMA{0x00}
, _pendingDMA{false}
, _openBus{0x00}
, _frameCounterClock{0x0000}
, _delayFrameReset{0x0000}
, _channelCounters{}
, _channelEnabled{}
, _channelHalted{}
, _stepMode{false}
, _inhibitFrameIRQ{false}
, _sendFrameIRQ{false}
, _deltaChannelRemainingBytes{0x0000}
, _deltaChannelSampleLength{0x0000}
, _deltaChannelPeriodCounter{0x0000}
, _deltaChannelPeriodLoad{0x0000}
, _deltaChannelBitsInBuffer{0x00}
, _deltaChannelShouldLoop{false}
, _deltaChannelEnableIRQ{false}
, _deltaChannelSampleBufferEmpty{false}
, _enableDMC{false}
, _sendDeltaChannelIRQ{false}
{
    memset(_channelCounters, 0x00, 4);
    memset(_channelEnabled, false, 4);
    memset(_channelHalted, false, 4);
}

void cynes::APU::power() {
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

void cynes::APU::reset() {
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

void cynes::APU::tick(bool reading, bool preventLoad) {
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

void cynes::APU::write(uint8_t address, uint8_t value) {
    _openBus = value;

    switch (static_cast<Register>(address)) {
    case Register::PULSE_1_0:
        _channelHalted[0x0] = value & 0x20;
        break;
    case Register::PULSE_1_3:
        if (_channelEnabled[0x0]) _channelCounters[0x0] = LENGTH_COUNTER_TABLE[value >> 3];
        break;
    case Register::PULSE_2_0:
        _channelHalted[0x1] = value & 0x20;
        break;
    case Register::PULSE_2_3:
        if (_channelEnabled[0x1]) _channelCounters[0x1] = LENGTH_COUNTER_TABLE[value >> 3];
        break;
    case Register::TRIANGLE_0:
        _channelHalted[0x2] = value & 0x80;
        break;
    case Register::TRIANGLE_3:
        if (_channelEnabled[0x2]) _channelCounters[0x2] = LENGTH_COUNTER_TABLE[value >> 3];
        break;
    case Register::NOISE_0:
        _channelHalted[0x3] = value & 0x20;
        break;
    case Register::NOISE_3:
        if (_channelEnabled[0x3]) _channelCounters[0x3] = LENGTH_COUNTER_TABLE[value >> 3];
        break;
    case Register::OAM_DMA:
        performDMA(value);
        break;
    case Register::DELTA_3:
        _deltaChannelSampleLength = (value << 4) + 1;
        break;

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

uint8_t cynes::APU::read(uint8_t address) {
    if (static_cast<Register>(address) == Register::CTRL_STATUS) {
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

void cynes::APU::updateCounters() {
    for (uint8_t channel = 0; channel < 0x4; channel++) {
        if (!_channelHalted[channel] && _channelCounters[channel] > 0) {
            _channelCounters[channel]--;
        }
    }
}

void cynes::APU::loadDeltaChannelByte(bool reading) {
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

        _nes.ppu.tick();
        _nes.ppu.tick();
        _nes.ppu.tick();
        _nes.cpu.poll();
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

void cynes::APU::performDMA(uint8_t address) {
    _addressDMA = address;
    _pendingDMA = true;
}

void cynes::APU::performPendingDMA() {
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

void cynes::APU::setFrameIRQ(bool irq) {
    _sendFrameIRQ = irq;

    _nes.cpu.setFrameIRQ(irq);
}

void cynes::APU::setDeltaIRQ(bool irq) {
    _sendDeltaChannelIRQ = irq;

    _nes.cpu.setDeltaIRQ(irq);
}
