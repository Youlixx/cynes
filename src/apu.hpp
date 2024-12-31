#ifndef __CYNES_APU__
#define __CYNES_APU__

#include <cstdint>

#include "utils.hpp"

namespace cynes {
// Forward declaration.
class NES;

/// Audio Processing Unit (see https://www.nesdev.org/wiki/APU).
/// This implementation does not produce any sound, it is only emulated for
/// timing and interrupt purposes.
class APU {
public:
    /// Initialize the APU.
    APU(NES& nes);
    ~APU() = default;

public:
    /// Set the APU in its power-up state.
    void power();

    /// Set the APU in its reset state.
    void reset();

    /// Tick the APU.
    /// @param reading Should be true if the APU is ticked on a reading cycle.
    /// @param preventLoad False by default, should be set to true only when
    /// called from `APU::loadDeltaChannelByte` to avoid recursion.
    void tick(bool reading, bool preventLoad = false);

    /// Write to the APU memory.
    /// @param address Memory address within the APU memory address space.
    /// @param value Value to write.
    void write(uint8_t address, uint8_t value);

    /// Read from the APU memory.
    /// @note This function has other side effects than simply reading from
    /// memory, it should not be used as a memory watch function.
    /// @param address Memory address within the APU memory address space.
    /// @return The value stored at the given address.
    uint8_t read(uint8_t address);

private:
    NES& _nes;

private:
    void updateCounters();
    void loadDeltaChannelByte(bool reading);

    void performDMA(uint8_t address);
    void performPendingDMA();

    void setFrameIRQ(bool irq);
    void setDeltaIRQ(bool irq);

private:
    bool _latchCycle;

    uint8_t _delayDMA;
    uint8_t _addressDMA;

    bool _pendingDMA;

    uint8_t _openBus;

private:
    uint32_t _frameCounterClock;
    uint32_t _delayFrameReset;

    uint8_t _channelCounters[0x4];

    bool _channelEnabled[0x4];
    bool _channelHalted[0x4];

    bool _stepMode;

    bool _inhibitFrameIRQ;
    bool _sendFrameIRQ;

    const uint8_t LENGTH_COUNTER_TABLE[0x20] = {
        0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06, 0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E,
        0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16, 0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E
    };

private:
    uint16_t _deltaChannelRemainingBytes;
    uint16_t _deltaChannelSampleLength;
    uint16_t _deltaChannelPeriodCounter;
    uint16_t _deltaChannelPeriodLoad;

    uint8_t _deltaChannelBitsInBuffer;

    bool _deltaChannelShouldLoop;
    bool _deltaChannelEnableIRQ;
    bool _deltaChannelSampleBufferEmpty;

    bool _enableDMC;
    bool _sendDeltaChannelIRQ;

    const uint16_t PERIOD_DMC_TABLE[0x10] = {
        0x1AC, 0x17C, 0x154, 0x140, 0x11E, 0x0FE, 0x0E2, 0x0D6, 0x0BE, 0x0A0, 0x08E, 0x080, 0x06A, 0x054, 0x048, 0x036
    };

private:
    enum class Register : uint8_t {
        PULSE_1_0 = 0x00,
        PULSE_1_3 = 0x03,
        PULSE_2_0 = 0x04,
        PULSE_2_3 = 0x07,
        TRIANGLE_0 = 0x08,
        TRIANGLE_3 = 0x0B,
        NOISE_0 = 0x0C,
        NOISE_3 = 0x0F,
        DELTA_0 = 0x10,
        DELTA_3 = 0x13,
        OAM_DMA = 0x14,
        CTRL_STATUS = 0x15,
        FRAME_COUNTER = 0x17
    };

public:
    template<DumpOperation operation, typename T>
    constexpr void dump(T& buffer) {
        cynes::dump<operation>(buffer, _latchCycle);
        cynes::dump<operation>(buffer, _delayDMA);
        cynes::dump<operation>(buffer, _addressDMA);
        cynes::dump<operation>(buffer, _pendingDMA);
        cynes::dump<operation>(buffer, _openBus);

        cynes::dump<operation>(buffer, _frameCounterClock);
        cynes::dump<operation>(buffer, _delayFrameReset);
        cynes::dump<operation>(buffer, _channelCounters);
        cynes::dump<operation>(buffer, _channelEnabled);
        cynes::dump<operation>(buffer, _channelHalted);
        cynes::dump<operation>(buffer, _stepMode);
        cynes::dump<operation>(buffer, _inhibitFrameIRQ);
        cynes::dump<operation>(buffer, _sendFrameIRQ);

        cynes::dump<operation>(buffer, _deltaChannelRemainingBytes);
        cynes::dump<operation>(buffer, _deltaChannelSampleLength);
        cynes::dump<operation>(buffer, _deltaChannelPeriodCounter);
        cynes::dump<operation>(buffer, _deltaChannelPeriodLoad);
        cynes::dump<operation>(buffer, _deltaChannelBitsInBuffer);
        cynes::dump<operation>(buffer, _deltaChannelShouldLoop);
        cynes::dump<operation>(buffer, _deltaChannelEnableIRQ);
        cynes::dump<operation>(buffer, _deltaChannelSampleBufferEmpty);
        cynes::dump<operation>(buffer, _enableDMC);
        cynes::dump<operation>(buffer, _sendDeltaChannelIRQ);
    }
};
}

#endif
