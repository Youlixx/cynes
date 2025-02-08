#ifndef __CYNES_PPU__
#define __CYNES_PPU__

#include <cstdint>
#include <memory>

#include "utils.hpp"

namespace cynes {
// Forward declaration.
class NES;

/// Picture Processing Unit (see https://www.nesdev.org/wiki/PPU).
class PPU {
public:
    /// Initialize the PPU.
    PPU(NES& nes);

    /// Default destructor.
    ~PPU() = default;

public:
    /// Set the PPU in its power-up state.
    void power();

    /// Set the PPU in its reset state.
    void reset();

    /// Tick the PPU.
    void tick();

    /// Write to the PPU memory.
    /// @note This function has other side effects than simply writing to the memory, it
    /// should not be used as a memory set function.
    /// @param address Memory address within the PPU memory address space.
    /// @param value Value to write.
    void write(uint8_t address, uint8_t value);

    /// Read from the APU memory.
    /// @note This function has other side effects than simply reading from memory, it
    /// should not be used as a memory watch function.
    /// @param address Memory address within the PPU memory address space.
    /// @return The value stored at the given address.
    uint8_t read(uint8_t address);

    /// Get a pointer to the internal frame buffer.
    const uint8_t* get_frame_buffer() const;

    /// Check whether or not the frame is ready.
    /// @note Calling this function will reset the flag.
    /// @return True if the frame is ready, false otherwise.
    bool is_frame_ready();

private:
    NES& _nes;

private:
    std::unique_ptr<uint8_t[]> _frame_buffer;

    uint16_t _current_x;
    uint16_t _current_y;

    bool _frame_ready;

    bool _rendering_enabled;
    bool _rendering_enabled_delayed;
    bool _prevent_vertical_blank;

private:
    bool _control_increment_mode;
    bool _control_foreground_table;
    bool _control_background_table;
    bool _control_foreground_large;
    bool _control_interrupt_on_vertical_blank;

private:
    bool _mask_grayscale_mode;
    bool _mask_render_background_left;
    bool _mask_render_foreground_left;
    bool _mask_render_background;
    bool _mask_render_foreground;

    uint8_t _mask_color_emphasize;

private:
    bool _status_sprite_overflow;
    bool _status_sprite_zero_hit;
    bool _status_vertical_blank;

private:
    const uint8_t DECAY_PERIOD = 30;
    uint8_t _clock_decays[3];
    uint8_t _register_decay;

private:
    bool _latch_cycle;
    bool _latch_address;

    uint16_t _register_t;
    uint16_t _register_v;
    uint16_t _delayed_register_v;

    uint8_t _scroll_x;
    uint8_t _delay_data_read_counter;
    uint8_t _delay_data_write_counter;
    uint8_t _buffer_data;

    void increment_scroll_x();
    void increment_scroll_y();

    void reset_scroll_x();
    void reset_scroll_y();

private:
    uint8_t _background_data[0x4];
    uint16_t _background_shifter[0x4];

    void load_background_shifters();
    void update_background_shifters();

private:
    uint8_t _foreground_data[0x20];
    uint8_t _foreground_shifter[0x10];
    uint8_t _foreground_attributes[0x8];
    uint8_t _foreground_positions[0x8];

    uint8_t _foreground_data_pointer;
    uint8_t _foreground_sprite_count;
    uint8_t _foreground_sprite_count_next;
    uint8_t _foreground_sprite_pointer;
    uint8_t _foreground_read_delay_counter;

    uint16_t _foreground_sprite_address;

    bool _foreground_sprite_zero_line;
    bool _foreground_sprite_zero_should;
    bool _foreground_sprite_zero_hit;

    enum class SpriteEvaluationStep {
        LOAD_SECONDARY_OAM, INCREMENT_POINTER, IDLE
    } _foreground_evaluation_step;

    void reset_foreground_data();
    void clear_foreground_data();
    void fetch_foreground_data();
    void load_foreground_shifter();
    void update_foreground_shifter();

    uint8_t blend_colors();

private:
    enum class Register : uint8_t {
        PPU_CTRL = 0x00,
        PPU_MASK = 0x01,
        PPU_STATUS = 0x02,
        OAM_ADDR = 0x03,
        OAM_DATA = 0x04,
        PPU_SCROLL = 0x05,
        PPU_ADDR = 0x06,
        PPU_DATA = 0x07
    };

public:
    template<DumpOperation operation, typename T>
    constexpr void dump(T& buffer) {
        cynes::dump<operation>(buffer, _current_x);
        cynes::dump<operation>(buffer, _current_y);
        cynes::dump<operation>(buffer, _frame_ready);
        cynes::dump<operation>(buffer, _rendering_enabled);
        cynes::dump<operation>(buffer, _rendering_enabled_delayed);
        cynes::dump<operation>(buffer, _prevent_vertical_blank);

        cynes::dump<operation>(buffer, _control_increment_mode);
        cynes::dump<operation>(buffer, _control_foreground_table);
        cynes::dump<operation>(buffer, _control_background_table);
        cynes::dump<operation>(buffer, _control_foreground_large);
        cynes::dump<operation>(buffer, _control_interrupt_on_vertical_blank);

        cynes::dump<operation>(buffer, _mask_grayscale_mode);
        cynes::dump<operation>(buffer, _mask_render_background_left);
        cynes::dump<operation>(buffer, _mask_render_foreground_left);
        cynes::dump<operation>(buffer, _mask_render_background);
        cynes::dump<operation>(buffer, _mask_render_foreground);
        cynes::dump<operation>(buffer, _mask_color_emphasize);

        cynes::dump<operation>(buffer, _status_sprite_overflow);
        cynes::dump<operation>(buffer, _status_sprite_zero_hit);
        cynes::dump<operation>(buffer, _status_vertical_blank);

        cynes::dump<operation>(buffer, _clock_decays);
        cynes::dump<operation>(buffer, _register_decay);

        cynes::dump<operation>(buffer, _latch_cycle);
        cynes::dump<operation>(buffer, _latch_address);
        cynes::dump<operation>(buffer, _register_t);
        cynes::dump<operation>(buffer, _register_v);
        cynes::dump<operation>(buffer, _delayed_register_v);
        cynes::dump<operation>(buffer, _scroll_x);
        cynes::dump<operation>(buffer, _delay_data_read_counter);
        cynes::dump<operation>(buffer, _delay_data_write_counter);
        cynes::dump<operation>(buffer, _buffer_data);

        cynes::dump<operation>(buffer, _background_data);
        cynes::dump<operation>(buffer, _background_shifter);

        cynes::dump<operation>(buffer, _foreground_data);
        cynes::dump<operation>(buffer, _foreground_shifter);
        cynes::dump<operation>(buffer, _foreground_attributes);
        cynes::dump<operation>(buffer, _foreground_positions);
        cynes::dump<operation>(buffer, _foreground_data_pointer);
        cynes::dump<operation>(buffer, _foreground_sprite_count);
        cynes::dump<operation>(buffer, _foreground_sprite_count_next);
        cynes::dump<operation>(buffer, _foreground_sprite_pointer);
        cynes::dump<operation>(buffer, _foreground_read_delay_counter);
        cynes::dump<operation>(buffer, _foreground_sprite_address);
        cynes::dump<operation>(buffer, _foreground_sprite_zero_line);
        cynes::dump<operation>(buffer, _foreground_sprite_zero_should);
        cynes::dump<operation>(buffer, _foreground_sprite_zero_hit);
        cynes::dump<operation>(buffer, _foreground_evaluation_step);
    }
};
}

#endif
