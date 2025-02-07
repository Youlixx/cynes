#include "ppu.hpp"
#include "cpu.hpp"
#include "nes.hpp"
#include "mapper.hpp"

#include <cstring>

cynes::PPU::PPU(NES& nes)
: _nes{nes}
, _current_x{0x0000}
, _current_y{0x0000}
, _rendering_enabled{false}
, _rendering_enabled_delayed{false}
, _prevent_vertical_blank{false}
, _control_increment_mode{false}
, _control_foreground_table{false}
, _control_background_table{false}
, _control_foreground_large{false}
, _control_interrupt_on_vertical_blank{false}
, _mask_grayscale_mode{false}
, _mask_render_background_left{false}
, _mask_render_foreground_left{false}
, _mask_render_background{false}
, _mask_render_foreground{false}
, _mask_color_emphasize{0x00}
, _status_sprite_overflow{false}
, _status_sprite_zero_hit{false}
, _status_vertical_blank{false}
, _clock_decays{}
, _register_decay{0x00}
, _latch_cycle{false}
, _latch_address{false}
, _register_t{0x0000}
, _register_v{0x0000}
, _delayed_register_v{0x0000}
, _scroll_x{0x00}
, _delay_data_read_counter{0x00}
, _delay_data_write_counter{0x00}
, _buffer_data{0x00}
, _background_data{}
, _background_shifter{}
, _foreground_data{}
, _foreground_shifter{}
, _foreground_attributes{}
, _foreground_positions{}
, _foreground_data_pointer{0x00}
, _foreground_sprite_count{0x00}
, _foreground_sprite_count_next{0x00}
, _foreground_sprite_pointer{0x00}
, _foreground_read_delay_counter{0x00}
, _foreground_sprite_address{0x0000}
, _foreground_sprite_zero_line{false}
, _foreground_sprite_zero_should{false}
, _foreground_sprite_zero_hit{false}
, _foreground_evaluation_step{SpriteEvaluationStep::LOAD_SECONDARY_OAM}
{
    std::memset(_clock_decays, 0x00, 0x3);
    std::memset(_background_data, 0x00, 0x4);
    std::memset(_background_shifter, 0x0000, 0x8);
    std::memset(_foreground_data, 0x00, 0x20);
    std::memset(_foreground_shifter, 0x00, 0x10);
    std::memset(_foreground_attributes, 0x00, 0x8);
    std::memset(_foreground_positions, 0x00, 0x8);
}

void cynes::PPU::power() {
    _current_y = 0xFF00;
    _current_x = 0xFF00;

    _rendering_enabled = false;
    _rendering_enabled_delayed = false;
    _prevent_vertical_blank = false;

    _control_increment_mode = false;
    _control_foreground_table = false;
    _control_background_table = false;
    _control_foreground_large = false;
    _control_interrupt_on_vertical_blank = false;

    _mask_grayscale_mode = false;
    _mask_render_background_left = false;
    _mask_render_foreground_left = false;
    _mask_render_background = false;
    _mask_render_foreground = false;

    _mask_color_emphasize = 0x00;

    _status_sprite_overflow = true;
    _status_sprite_zero_hit = false;
    _status_vertical_blank = true;

    _foreground_sprite_pointer = 0x00;

    _latch_address = false;
    _latch_cycle = false;

    _register_t = 0x0000;
    _register_v = 0x0000;
    _scroll_x = 0x00;

    _delay_data_write_counter = 0x00;
    _delay_data_read_counter = 0x00;
    _buffer_data = 0x00;
}

void cynes::PPU::reset() {
    _current_y = 0xFF00;
    _current_x = 0xFF00;

    _rendering_enabled = false;
    _rendering_enabled_delayed = false;
    _prevent_vertical_blank = false;

    _control_increment_mode = false;
    _control_foreground_table = false;
    _control_background_table = false;
    _control_foreground_large = false;
    _control_interrupt_on_vertical_blank = false;

    _mask_grayscale_mode = false;
    _mask_render_background_left = false;
    _mask_render_foreground_left = false;
    _mask_render_background = false;
    _mask_render_foreground = false;

    _mask_color_emphasize = 0x00;

    _latch_address = false;
    _latch_cycle = false;

    _register_t = 0x0000;
    _register_v = 0x0000;
    _scroll_x = 0x00;

    _delay_data_write_counter = 0x00;
    _delay_data_read_counter = 0x00;
    _buffer_data = 0x00;
}

void cynes::PPU::tick() {
    if (_current_x > 339) {
        _current_x = 0;

        if (++_current_y > 261) {
            _current_y = 0;
            _foreground_sprite_count = 0;

            _latch_cycle = !_latch_cycle;

            for (int k = 0; k < 3; k++) {
                if (_clock_decays[k] > 0 && --_clock_decays[k] == 0) {
                    switch (k) {
                    case 0: _register_decay &= 0x3F; break;
                    case 1: _register_decay &= 0xDF; break;
                    case 2: _register_decay &= 0xE0; break;
                    }
                }
            }
        }

        reset_foreground_data();

        if (_current_y == 261) {
            _status_sprite_overflow = false;
            _status_sprite_zero_hit = false;

            memset(_foreground_shifter, 0x00, 0x10);
        }
    } else {
        _current_x++;

        if (_current_y < 240) {
            if (_current_x < 257 || (_current_x >= 321 && _current_x < 337)) {
                load_background_shifters();
            }

            if (_current_x == 256) {
                increment_scroll_y();
            } else if (_current_x == 257) {
                reset_scroll_x();
            }

            if (_current_x >= 2 && _current_x < 257) {
                update_foreground_shifter();
            }

            if (_current_x < 65) {
                clear_foreground_data();
            } else if (_current_x < 257) {
                fetch_foreground_data();
            } else if (_current_x < 321) {
                load_foreground_shifter();
            }

            if (_current_x > 0 && _current_x < 257 && _current_y < 240) {
                memcpy(_frame_buffer + ((_current_y << 8) + _current_x - 1) * 3, PALETTE_COLORS[_mask_color_emphasize][_nes.read_ppu(0x3F00 | blend_colors())], 3);
            }
        } else if (_current_y == 240 && _current_x == 1) {
            _nes.read_ppu(_register_v);
        } else if (_current_y == 261) {
            if (_current_x == 1) {
                _status_vertical_blank = false;

                _nes.cpu.set_non_maskable_interrupt(false);
            }

            if (_current_x < 257 || (_current_x >= 321 && _current_x < 337)) {
                load_background_shifters();
            }

            if (_current_x == 256) {
                increment_scroll_y();
            } else if (_current_x == 257) {
                reset_scroll_x();
            } else  if (_current_x >= 280 && _current_x < 305) {
                reset_scroll_y();
            }

            if (_current_x > 1) {
                if (_current_x < 257) {
                    update_foreground_shifter();
                } else if (_current_x < 321) {
                    load_foreground_shifter();
                }
            }

            if (_rendering_enabled && (_current_x == 337 || _current_x == 339)) {
                _nes.read_ppu(0x2000 | (_register_v & 0x0FFF));

                if (_current_x == 339 && _latch_cycle) {
                    _current_x = 340;
                }
            }
        } else if (_current_x == 1 && _current_y == 241) {
            if (!_prevent_vertical_blank) {
                _status_vertical_blank = true;

                if (_control_interrupt_on_vertical_blank) {
                    _nes.cpu.set_non_maskable_interrupt(true);
                }
            }

            _prevent_vertical_blank = false;
            _frame_ready = true;
        }
    }

    if (_rendering_enabled_delayed != _rendering_enabled) {
        _rendering_enabled_delayed = _rendering_enabled;

        if (_current_y < 240 || _current_y == 261) {
            if (!_rendering_enabled_delayed) {
                _nes.read_ppu(_register_v);

                if (_current_x >= 65 && _current_x <= 256) {
                    _foreground_sprite_pointer++;
                }
            }
        }
    }

    if (_rendering_enabled != (_mask_render_background || _mask_render_foreground)) {
        _rendering_enabled = _mask_render_background || _mask_render_foreground;
    }


    if (_delay_data_write_counter > 0 && --_delay_data_write_counter == 0) {
        _register_v = _delayed_register_v;
        _register_t = _register_v;

        if ((_current_y >= 240 && _current_y != 261) || !_rendering_enabled) {
            _nes.read_ppu(_register_v);
        }
    }

    if (_delay_data_read_counter > 0) {
        _delay_data_read_counter--;
    }

    _nes.get_mapper().tick();
}

void cynes::PPU::write(uint8_t address, uint8_t value) {
    memset(_clock_decays, DECAY_PERIOD, 3);

    _register_decay = value;

    switch (static_cast<Register>(address)) {
    case Register::PPU_CTRL: {
        _register_t &= 0xF3FF;
        _register_t |= (value & 0x03) << 10;

        _control_increment_mode = value & 0x04;
        _control_foreground_table = value & 0x08;
        _control_background_table = value & 0x10;
        _control_foreground_large = value & 0x20;
        _control_interrupt_on_vertical_blank = value & 0x80;

        if (!_control_interrupt_on_vertical_blank) {
            _nes.cpu.set_non_maskable_interrupt(false);
        } else if (_status_vertical_blank) {
            _nes.cpu.set_non_maskable_interrupt(true);
        }

        break;
    }

    case Register::PPU_MASK: {
        _mask_grayscale_mode = value & 0x01;
        _mask_render_background_left = value & 0x02;
        _mask_render_foreground_left = value & 0x04;
        _mask_render_background = value & 0x08;
        _mask_render_foreground = value & 0x10;
        _mask_color_emphasize = value >> 5;

        break;
    }

    case Register::OAM_ADDR: {
        _foreground_sprite_pointer = value;

        break;
    }

    case Register::OAM_DATA: {
        if ((_current_y >= 240 && _current_y != 261) || !_rendering_enabled) {
            if ((_foreground_sprite_pointer & 0x03) == 0x02) {
                value &= 0xE3;
            }

            _nes.write_oam(_foreground_sprite_pointer++, value);
        } else {
            _foreground_sprite_pointer += 4;
        }

        break;
    }

    case Register::PPU_SCROLL: {
        if (!_latch_address) {
            _scroll_x = value & 0x07;

            _register_t &= 0xFFE0;
            _register_t |= value >> 3;
        } else {
            _register_t &= 0x8C1F;

            _register_t |= (value & 0xF8) << 2;
            _register_t |= (value & 0x07) << 12;
        }

        _latch_address = !_latch_address;

        break;
    }

    case Register::PPU_ADDR: {
        if (!_latch_address) {
            _register_t &= 0x00FF;
            _register_t |= value << 8;
        } else {
            _register_t &= 0xFF00;
            _register_t |= value;

            _delay_data_write_counter = 3;
            _delayed_register_v = _register_t;
        }

        _latch_address = !_latch_address;

        break;
    }

    case Register::PPU_DATA: {
        if ((_register_v & 0x3FFF) >= 0x3F00) {
            _nes.write_ppu(_register_v, value);
        } else {
            if ((_current_y >= 240 && _current_y != 261) || !_rendering_enabled) {
                _nes.write_ppu(_register_v, value);
            } else {
                _nes.write_ppu(_register_v, _register_v & 0xFF);
            }
        }

        if ((_current_y >= 240 && _current_y != 261) || !_rendering_enabled) {
            _register_v += _control_increment_mode ? 32 : 1;
            _register_v &= 0x7FFF;

            _nes.read_ppu(_register_v);
        } else {
            increment_scroll_x();
            increment_scroll_y();
        }

        break;
    }

    default: break;
    }
}

uint8_t cynes::PPU::read(uint8_t address) {
    switch (static_cast<Register>(address)) {
    case Register::PPU_STATUS: {
        memset(_clock_decays, DECAY_PERIOD, 2);

        _latch_address = false;

        _register_decay &= 0x1F;
        _register_decay |= _status_sprite_overflow << 5;
        _register_decay |= _status_sprite_zero_hit << 6;
        _register_decay |= _status_vertical_blank << 7;

        _status_vertical_blank = false;
        _nes.cpu.set_non_maskable_interrupt(false);

        if (_current_y == 241 && _current_x == 0) {
            _prevent_vertical_blank = true;
        }

        break;
    }

    case Register::OAM_DATA: {
        memset(_clock_decays, DECAY_PERIOD, 3);

        _register_decay = _nes.read_oam(_foreground_sprite_pointer);

        break;
    }

    case Register::PPU_DATA: {
        if (_delay_data_read_counter == 0) {
            uint8_t value = _nes.read_ppu(_register_v);

            if ((_register_v & 0x3FFF) >= 0x3F00) {
                _register_decay &= 0xC0;
                _register_decay |= value & 0x3F;

                _clock_decays[0] = _clock_decays[2] = DECAY_PERIOD;

                _buffer_data = _nes.read_ppu(_register_v - 0x1000);
            } else {
                _register_decay = _buffer_data;
                _buffer_data = value;

                memset(_clock_decays, DECAY_PERIOD, 3);
            }

            if ((_current_y >= 240 && _current_y != 261) || !_rendering_enabled) {
                _register_v += _control_increment_mode ? 32 : 1;
                _register_v &= 0x7FFF;

                _nes.read_ppu(_register_v);
            } else {
                increment_scroll_x();
                increment_scroll_y();
            }

            _delay_data_read_counter = 6;
        }

        break;
    }

    default: break;
    }

    return _register_decay;
}

uint8_t* cynes::PPU::get_frame_buffer() {
    return _frame_buffer;
}

bool cynes::PPU::is_frame_ready() {
    bool frame_ready = _frame_ready;
    _frame_ready = false;

    return frame_ready;
}

void cynes::PPU::increment_scroll_x() {
    if (_mask_render_background || _mask_render_foreground) {
        if ((_register_v & 0x001F) == 0x1F) {
            _register_v &= 0xFFE0;
            _register_v ^= 0x0400;
        } else {
            _register_v++;
        }
    }
}

void cynes::PPU::increment_scroll_y() {
    if (_mask_render_background || _mask_render_foreground) {
        if ((_register_v & 0x7000) != 0x7000) {
            _register_v += 0x1000;
        } else {
            _register_v &= 0x8FFF;

            uint8_t coarse_y = (_register_v & 0x03E0) >> 5;

            if (coarse_y == 0x1D) {
                coarse_y = 0;
                _register_v ^= 0x0800;
            } else if (((_register_v >> 5) & 0x1F) == 0x1F) {
                coarse_y = 0;
            } else {
                coarse_y++;
            }

            _register_v &= 0xFC1F;
            _register_v |= coarse_y << 5;
        }
    }
}

void cynes::PPU::reset_scroll_x() {
    if (_mask_render_background || _mask_render_foreground) {
        _register_v &= 0xFBE0;
        _register_v |= _register_t & 0x041F;
    }
}

void cynes::PPU::reset_scroll_y() {
    if (_mask_render_background || _mask_render_foreground) {
        _register_v &= 0x841F;
        _register_v |= _register_t & 0x7BE0;
    }
}


void cynes::PPU::load_background_shifters() {
    update_background_shifters();

    if (_rendering_enabled) {
        switch (_current_x & 0x07) {
        case 0x1: {
            _background_shifter[0] = (_background_shifter[0] & 0xFF00) | _background_data[2];
            _background_shifter[1] = (_background_shifter[1] & 0xFF00) | _background_data[3];

            if (_background_data[1] & 0x01) {
                _background_shifter[2] = (_background_shifter[2] & 0xFF00) | 0xFF;
            } else {
                _background_shifter[2] = (_background_shifter[2] & 0xFF00);
            }

            if (_background_data[1] & 0x02) {
                _background_shifter[3] = (_background_shifter[3] & 0xFF00) | 0xFF;
            } else {
                _background_shifter[3] = (_background_shifter[3] & 0xFF00);
            }

            uint16_t address = 0x2000;
            address |= _register_v & 0x0FFF;

            _background_data[0] = _nes.read_ppu(address);

            break;
        }

        case 0x3: {
            uint16_t address = 0x23C0;
            address |= _register_v & 0x0C00;
            address |= (_register_v >> 4) & 0x38;
            address |= (_register_v >> 2) & 0x07;

            _background_data[1] = _nes.read_ppu(address);

            if (_register_v & 0x0040) {
                _background_data[1] >>= 4;
            }

            if (_register_v & 0x0002) {
                _background_data[1] >>= 2;
            }

            _background_data[1] &= 0x03;

            break;
        }

        case 0x5: {
            uint16_t address = _control_background_table << 12;
            address |= _background_data[0] << 4;
            address |= _register_v >> 12;

            _background_data[2] = _nes.read_ppu(address);

            break;
        }

        case 0x7: {
            uint16_t address = _control_background_table << 12;
            address |= _background_data[0] << 4;
            address |= _register_v >> 12;
            address += 0x8;

            _background_data[3] = _nes.read_ppu(address);

            break;

        }

        case 0x0: increment_scroll_x(); break;
        }
    }
}

void cynes::PPU::update_background_shifters() {
    if (_mask_render_background || _mask_render_foreground) {
        _background_shifter[0] <<= 1;
        _background_shifter[1] <<= 1;
        _background_shifter[2] <<= 1;
        _background_shifter[3] <<= 1;
    }
}

void cynes::PPU::reset_foreground_data() {
    _foreground_sprite_count_next = _foreground_sprite_count;

    _foreground_data_pointer = 0;
    _foreground_sprite_count = 0;
    _foreground_evaluation_step = SpriteEvaluationStep::LOAD_SECONDARY_OAM;
    _foreground_sprite_zero_line = _foreground_sprite_zero_should;
    _foreground_sprite_zero_should = false;
    _foreground_sprite_zero_hit = false;
}

void cynes::PPU::clear_foreground_data() {
    if (_current_x & 0x01) {
        _foreground_data[_foreground_data_pointer++] = 0xFF;

        _foreground_data_pointer &= 0x1F;
    }
}

void cynes::PPU::fetch_foreground_data() {
    if (_current_x % 2 == 0 && _rendering_enabled) {
        uint8_t sprite_size = _control_foreground_large ? 16 : 8;

        switch (_foreground_evaluation_step) {
        case SpriteEvaluationStep::LOAD_SECONDARY_OAM: {
            uint8_t sprite_data = _nes.read_oam(_foreground_sprite_pointer);

            _foreground_data[_foreground_sprite_count * 4 + (_foreground_sprite_pointer & 0x03)] = sprite_data;

            if (!(_foreground_sprite_pointer & 0x3)) {
                int16_t offset_y = int16_t(_current_y) - int16_t(sprite_data);

                if (offset_y >= 0 && offset_y < sprite_size) {
                    if (!_foreground_sprite_pointer++) {
                        _foreground_sprite_zero_should = true;
                    }
                } else {
                    _foreground_sprite_pointer += 4;

                    if (!_foreground_sprite_pointer) {
                        _foreground_evaluation_step = SpriteEvaluationStep::IDLE;
                    } else if (_foreground_sprite_count == 8) {
                        _foreground_evaluation_step = SpriteEvaluationStep::INCREMENT_POINTER;
                    }
                }
            } else if (!(++_foreground_sprite_pointer & 0x03)) {
                _foreground_sprite_count++;

                if (!_foreground_sprite_pointer) {
                    _foreground_evaluation_step = SpriteEvaluationStep::IDLE;
                } else if (_foreground_sprite_count == 8) {
                    _foreground_evaluation_step = SpriteEvaluationStep::INCREMENT_POINTER;
                }
            }

            break;
        }

        case SpriteEvaluationStep::INCREMENT_POINTER: {
            if (_foreground_read_delay_counter) {
                _foreground_read_delay_counter--;
            } else {
                int16_t offset_y = int16_t(_current_y) - int16_t(_nes.read_oam(_foreground_sprite_pointer));

                if (offset_y >= 0 && offset_y < sprite_size) {
                    _status_sprite_overflow = true;

                    _foreground_sprite_pointer++;
                    _foreground_read_delay_counter = 3;
                } else {
                    uint8_t low = (_foreground_sprite_pointer + 1) & 0x03;

                    _foreground_sprite_pointer += 0x04;
                    _foreground_sprite_pointer &= 0xFC;

                    if (!_foreground_sprite_pointer) {
                        _foreground_evaluation_step = SpriteEvaluationStep::IDLE;
                    }

                    _foreground_sprite_pointer |= low;
                }
            }

            break;
        }

        default: _foreground_sprite_pointer = 0;
        }
    }
}

void cynes::PPU::load_foreground_shifter() {
    if (_rendering_enabled) {
        _foreground_sprite_pointer = 0;

        if (_current_x == 257) {
            _foreground_data_pointer = 0;
        }

        switch (_current_x & 0x7) {
        case 0x1: {
            uint16_t address = 0x2000;
            address |= _register_v & 0x0FFF;

            _nes.read_ppu(address);

            break;
        }

        case 0x3: {
            uint16_t address = 0x23C0;
            address |= _register_v & 0x0C00;
            address |= (_register_v >> 4) & 0x38;
            address |= (_register_v >> 2) & 0x07;

            _nes.read_ppu(address);

            break;
        }

        case 0x5: {
            uint8_t sprite_index = _foreground_data[_foreground_data_pointer * 4 + 1];
            uint8_t sprite_attribute = _foreground_data[_foreground_data_pointer * 4 + 2];

            uint8_t offset = 0x00;

            if (_foreground_data_pointer < _foreground_sprite_count) {
                offset = _current_y - _foreground_data[_foreground_data_pointer * 4];
            }

            _foreground_sprite_address = 0x0000;

            if (_control_foreground_large) {
                _foreground_sprite_address = (sprite_index & 0x01) << 12;

                if (sprite_attribute & 0x80) {
                    if (offset < 8) {
                        _foreground_sprite_address |= ((sprite_index & 0xFE) + 1) << 4;
                    } else {
                        _foreground_sprite_address |= ((sprite_index & 0xFE)) << 4;
                    }
                } else {
                    if (offset < 8) {
                        _foreground_sprite_address |= ((sprite_index & 0xFE)) << 4;
                    } else {
                        _foreground_sprite_address |= ((sprite_index & 0xFE) + 1) << 4;
                    }
                }
            } else {
                _foreground_sprite_address = _control_foreground_table << 12 | sprite_index << 4;
            }

            if (sprite_attribute & 0x80) {
                _foreground_sprite_address |= (7 - offset) & 0x07;
            } else {
                _foreground_sprite_address |= offset & 0x07;
            }

            uint8_t sprite_pattern_lsb_plane = _nes.read_ppu(_foreground_sprite_address);


            if (sprite_attribute & 0x40) {
                sprite_pattern_lsb_plane = (sprite_pattern_lsb_plane & 0xF0) >> 4 | (sprite_pattern_lsb_plane & 0x0F) << 4;
                sprite_pattern_lsb_plane = (sprite_pattern_lsb_plane & 0xCC) >> 2 | (sprite_pattern_lsb_plane & 0x33) << 2;
                sprite_pattern_lsb_plane = (sprite_pattern_lsb_plane & 0xAA) >> 1 | (sprite_pattern_lsb_plane & 0x55) << 1;
            }

            _foreground_shifter[_foreground_data_pointer * 2] = sprite_pattern_lsb_plane;

            break;
        }

        case 0x7: {
            uint8_t sprite_pattern_msb_plane = _nes.read_ppu(_foreground_sprite_address + 8);

            if (_foreground_data[_foreground_data_pointer * 4 + 2] & 0x40) {
                sprite_pattern_msb_plane = (sprite_pattern_msb_plane & 0xF0) >> 4 | (sprite_pattern_msb_plane & 0x0F) << 4;
                sprite_pattern_msb_plane = (sprite_pattern_msb_plane & 0xCC) >> 2 | (sprite_pattern_msb_plane & 0x33) << 2;
                sprite_pattern_msb_plane = (sprite_pattern_msb_plane & 0xAA) >> 1 | (sprite_pattern_msb_plane & 0x55) << 1;
            }

            _foreground_shifter[_foreground_data_pointer * 2 + 1] = sprite_pattern_msb_plane;
            _foreground_positions[_foreground_data_pointer] = _foreground_data[_foreground_data_pointer * 4 + 3];
            _foreground_attributes[_foreground_data_pointer] = _foreground_data[_foreground_data_pointer * 4 + 2];

            _foreground_data_pointer++;

            break;
        }
        }
    }
}

void cynes::PPU::update_foreground_shifter() {
    if (_mask_render_foreground) {
        for (uint8_t sprite = 0; sprite < _foreground_sprite_count_next; sprite++) {
            if (_foreground_positions[sprite] > 0) {
                _foreground_positions[sprite] --;
            } else {
                _foreground_shifter[sprite * 2] <<= 1;
                _foreground_shifter[sprite * 2 + 1] <<= 1;
            }
        }
    }
}

uint8_t cynes::PPU::blend_colors() {
    if (!_rendering_enabled && (_register_v & 0x3FFF) >= 0x3F00) {
        return _register_v & 0x1F;
    }

    uint8_t background_pixel = 0x00;
    uint8_t background_palette = 0x00;

    if (_mask_render_background && (_current_x > 8 || _mask_render_background_left)) {
        uint16_t bit_mask = 0x8000 >> _scroll_x;

        background_pixel = ((_background_shifter[0] & bit_mask) > 0) | (((_background_shifter[1] & bit_mask) > 0) << 1);
        background_palette = ((_background_shifter[2] & bit_mask) > 0) | (((_background_shifter[3] & bit_mask) > 0) << 1);
    }

    uint8_t foreground_pixel = 0x00;
    uint8_t foreground_palette = 0x00;
    uint8_t foreground_priority = 0x00;

    if (_mask_render_foreground && (_current_x > 8 || _mask_render_foreground_left)) {
        _foreground_sprite_zero_hit = false;

        for (uint8_t sprite = 0; sprite < _foreground_sprite_count_next; sprite++) {
            if (_foreground_positions[sprite] == 0) {
                foreground_pixel = ((_foreground_shifter[sprite * 2] & 0x80) > 0) | (((_foreground_shifter[sprite * 2 + 1] & 0x80) > 0) << 1);
                foreground_palette = (_foreground_attributes[sprite] & 0x03) + 0x04;
                foreground_priority = (_foreground_attributes[sprite] & 0x20) == 0x00;

                if (foreground_pixel != 0) {
                    if (sprite == 0 && _current_x != 256) {
                        _foreground_sprite_zero_hit = true;
                    }

                    break;
                }
            }
        }
    }

    uint8_t final_pixel = 0x00;
    uint8_t final_palette = 0x00;

    if (background_pixel == 0 && foreground_pixel > 0) {
        final_pixel = foreground_pixel;
        final_palette = foreground_palette;
    } else if (background_pixel > 0 && foreground_pixel == 0) {
        final_pixel = background_pixel;
        final_palette = background_palette;
    } else if (background_pixel > 0 && foreground_pixel > 0) {
        if (foreground_priority) {
            final_pixel = foreground_pixel;
            final_palette = foreground_palette;
        } else {
            final_pixel = background_pixel;
            final_palette = background_palette;
        }

        if (_foreground_sprite_zero_hit && _foreground_sprite_zero_line && (_current_x > 8 || _mask_render_background_left || _mask_render_foreground_left)) {
            _status_sprite_zero_hit = true;
        }
    }

    final_pixel |= final_palette << 2;

    if (_mask_grayscale_mode) {
        final_pixel &= 0x30;
    }

    return final_pixel;
}
