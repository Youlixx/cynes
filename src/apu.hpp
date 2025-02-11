#ifndef __CYNES_APU__
#define __CYNES_APU__

#include <cstdint>

#include "utils.hpp"

namespace cynes {
// Forward declaration.
class NES;

/// Audio Processing Unit (see https://www.nesdev.org/wiki/APU).
/// This implementation does not produce any sound, it is only emulated for timing and
/// interrupt purposes.
class APU {
public:
    /// Initialize the APU.
    APU(NES& nes);

    /// Default destructor.
    ~APU() = default;

public:
    /// Set the APU in its power-up state.
    void power();

    /// Set the APU in its reset state.
    void reset();

    /// Tick the APU.
    /// @param reading Should be true if the APU is ticked on a reading cycle.
    /// @param prevent_load False by default, should be set to true only when called
    /// from `APU::load_delta_channel_byte` to avoid recursion.
    void tick(bool reading, bool prevent_load = false);

    /// Write to the APU memory.
    /// @param address Memory address within the APU memory address space.
    /// @param value Value to write.
    void write(uint8_t address, uint8_t value);

    /// Read from the APU memory.
    /// @note This function has other side effects than simply reading from memory, it
    /// should not be used as a memory watch function.
    /// @param address Memory address within the APU memory address space.
    /// @return The value stored at the given address.
    uint8_t read(uint8_t address);

private:
    NES& _nes;

private:
    void update_counters();
    void load_delta_channel_byte(bool reading);

    void perform_dma(uint8_t address);
    void perform_pending_dma();

    void set_frame_interrupt(bool interrupt);
    void set_delta_interrupt(bool interrupt);

private:
    bool _latch_cycle;

    uint8_t _delay_dma;
    uint8_t _address_dma;

    bool _pending_dma;

    uint8_t _internal_open_bus;

private:
    uint32_t _frame_counter_clock;
    uint32_t _delay_frame_reset;

    uint8_t _channels_counters[0x4];

    bool _channel_enabled[0x4];
    bool _channel_halted[0x4];

    bool _step_mode;

    bool _inhibit_frame_interrupt;
    bool _send_frame_interrupt;

private:
    uint16_t _delta_channel_remaining_bytes;
    uint16_t _delta_channel_sample_length;
    uint16_t _delta_channel_period_counter;
    uint16_t _delta_channel_period_load;

    uint8_t _delta_channel_bits_in_buffer;

    bool _delta_channel_should_loop;
    bool _delta_channel_enable_interrupt;
    bool _delta_channel_sample_buffer_empty;

    bool _enable_dmc;
    bool _send_delta_channel_interrupt;

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
        cynes::dump<operation>(buffer, _latch_cycle);
        cynes::dump<operation>(buffer, _delay_dma);
        cynes::dump<operation>(buffer, _address_dma);
        cynes::dump<operation>(buffer, _pending_dma);

        cynes::dump<operation>(buffer, _frame_counter_clock);
        cynes::dump<operation>(buffer, _delay_frame_reset);
        cynes::dump<operation>(buffer, _channels_counters);
        cynes::dump<operation>(buffer, _channel_enabled);
        cynes::dump<operation>(buffer, _channel_halted);
        cynes::dump<operation>(buffer, _step_mode);
        cynes::dump<operation>(buffer, _inhibit_frame_interrupt);
        cynes::dump<operation>(buffer, _send_frame_interrupt);

        cynes::dump<operation>(buffer, _delta_channel_remaining_bytes);
        cynes::dump<operation>(buffer, _delta_channel_sample_length);
        cynes::dump<operation>(buffer, _delta_channel_period_counter);
        cynes::dump<operation>(buffer, _delta_channel_period_load);
        cynes::dump<operation>(buffer, _delta_channel_bits_in_buffer);
        cynes::dump<operation>(buffer, _delta_channel_should_loop);
        cynes::dump<operation>(buffer, _delta_channel_enable_interrupt);
        cynes::dump<operation>(buffer, _delta_channel_sample_buffer_empty);
        cynes::dump<operation>(buffer, _enable_dmc);
        cynes::dump<operation>(buffer, _send_delta_channel_interrupt);
    }
};
}

#endif
