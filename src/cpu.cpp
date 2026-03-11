#include "cpu.hpp"
#include "nes.hpp"

#include <cstring>

using namespace cynes;

using _addr_ptr = void (CPU::*)();
const _addr_ptr CPU::ADDRESSING_MODES[256] = {
    &CPU::addr_imp, &CPU::addr_ixr, &CPU::addr_acc, &CPU::addr_ixr, &CPU::addr_zpr, &CPU::addr_zpr, &CPU::addr_zpr, &CPU::addr_zpr,
    &CPU::addr_imp, &CPU::addr_imm, &CPU::addr_acc, &CPU::addr_imm, &CPU::addr_abr, &CPU::addr_abr, &CPU::addr_abr, &CPU::addr_abr,
    &CPU::addr_rel, &CPU::addr_iyr, &CPU::addr_acc, &CPU::addr_iym, &CPU::addr_zxr, &CPU::addr_zxr, &CPU::addr_zxr, &CPU::addr_zxr,
    &CPU::addr_imp, &CPU::addr_ayr, &CPU::addr_imp, &CPU::addr_aym, &CPU::addr_axr, &CPU::addr_axr, &CPU::addr_axm, &CPU::addr_axm,
    &CPU::addr_abw, &CPU::addr_ixr, &CPU::addr_acc, &CPU::addr_ixr, &CPU::addr_zpr, &CPU::addr_zpr, &CPU::addr_zpr, &CPU::addr_zpr,
    &CPU::addr_imp, &CPU::addr_imm, &CPU::addr_acc, &CPU::addr_imm, &CPU::addr_abr, &CPU::addr_abr, &CPU::addr_abr, &CPU::addr_abr,
    &CPU::addr_rel, &CPU::addr_iyr, &CPU::addr_acc, &CPU::addr_iym, &CPU::addr_zxr, &CPU::addr_zxr, &CPU::addr_zxr, &CPU::addr_zxr,
    &CPU::addr_imp, &CPU::addr_ayr, &CPU::addr_imp, &CPU::addr_aym, &CPU::addr_axr, &CPU::addr_axr, &CPU::addr_axm, &CPU::addr_axm,
    &CPU::addr_imp, &CPU::addr_ixr, &CPU::addr_acc, &CPU::addr_ixr, &CPU::addr_zpr, &CPU::addr_zpr, &CPU::addr_zpr, &CPU::addr_zpr,
    &CPU::addr_imp, &CPU::addr_imm, &CPU::addr_acc, &CPU::addr_imm, &CPU::addr_abw, &CPU::addr_abr, &CPU::addr_abr, &CPU::addr_abr,
    &CPU::addr_rel, &CPU::addr_iyr, &CPU::addr_acc, &CPU::addr_iym, &CPU::addr_zxr, &CPU::addr_zxr, &CPU::addr_zxr, &CPU::addr_zxr,
    &CPU::addr_imp, &CPU::addr_ayr, &CPU::addr_imp, &CPU::addr_aym, &CPU::addr_axr, &CPU::addr_axr, &CPU::addr_axm, &CPU::addr_axm,
    &CPU::addr_imp, &CPU::addr_ixr, &CPU::addr_acc, &CPU::addr_ixr, &CPU::addr_zpr, &CPU::addr_zpr, &CPU::addr_zpr, &CPU::addr_zpr,
    &CPU::addr_imp, &CPU::addr_imm, &CPU::addr_acc, &CPU::addr_imm, &CPU::addr_ind, &CPU::addr_abr, &CPU::addr_abr, &CPU::addr_abr,
    &CPU::addr_rel, &CPU::addr_iyr, &CPU::addr_acc, &CPU::addr_iym, &CPU::addr_zxr, &CPU::addr_zxr, &CPU::addr_zxr, &CPU::addr_zxr,
    &CPU::addr_imp, &CPU::addr_ayr, &CPU::addr_imp, &CPU::addr_aym, &CPU::addr_axr, &CPU::addr_axr, &CPU::addr_axm, &CPU::addr_axm,
    &CPU::addr_imm, &CPU::addr_ixw, &CPU::addr_imm, &CPU::addr_ixw, &CPU::addr_zpw, &CPU::addr_zpw, &CPU::addr_zpw, &CPU::addr_zpw,
    &CPU::addr_imp, &CPU::addr_imm, &CPU::addr_imp, &CPU::addr_imm, &CPU::addr_abw, &CPU::addr_abw, &CPU::addr_abw, &CPU::addr_abw,
    &CPU::addr_rel, &CPU::addr_iyw, &CPU::addr_acc, &CPU::addr_iyw, &CPU::addr_zxw, &CPU::addr_zxw, &CPU::addr_zyw, &CPU::addr_zyw,
    &CPU::addr_imp, &CPU::addr_ayw, &CPU::addr_imp, &CPU::addr_ayw, &CPU::addr_axw, &CPU::addr_axw, &CPU::addr_ayw, &CPU::addr_ayw,
    &CPU::addr_imm, &CPU::addr_ixr, &CPU::addr_imm, &CPU::addr_ixr, &CPU::addr_zpr, &CPU::addr_zpr, &CPU::addr_zpr, &CPU::addr_zpr,
    &CPU::addr_imp, &CPU::addr_imm, &CPU::addr_imp, &CPU::addr_imm, &CPU::addr_abr, &CPU::addr_abr, &CPU::addr_abr, &CPU::addr_abr,
    &CPU::addr_rel, &CPU::addr_iyr, &CPU::addr_acc, &CPU::addr_iyr, &CPU::addr_zxr, &CPU::addr_zxr, &CPU::addr_zyr, &CPU::addr_zyr,
    &CPU::addr_imp, &CPU::addr_ayr, &CPU::addr_imp, &CPU::addr_ayr, &CPU::addr_axr, &CPU::addr_axr, &CPU::addr_ayr, &CPU::addr_ayr,
    &CPU::addr_imm, &CPU::addr_ixr, &CPU::addr_imm, &CPU::addr_ixr, &CPU::addr_zpr, &CPU::addr_zpr, &CPU::addr_zpr, &CPU::addr_zpr,
    &CPU::addr_imp, &CPU::addr_imm, &CPU::addr_imp, &CPU::addr_imm, &CPU::addr_abr, &CPU::addr_abr, &CPU::addr_abr, &CPU::addr_abr,
    &CPU::addr_rel, &CPU::addr_iyr, &CPU::addr_acc, &CPU::addr_iym, &CPU::addr_zxr, &CPU::addr_zxr, &CPU::addr_zxr, &CPU::addr_zxr,
    &CPU::addr_imp, &CPU::addr_ayr, &CPU::addr_imp, &CPU::addr_aym, &CPU::addr_axr, &CPU::addr_axr, &CPU::addr_axm, &CPU::addr_axm,
    &CPU::addr_imm, &CPU::addr_ixr, &CPU::addr_imm, &CPU::addr_ixr, &CPU::addr_zpr, &CPU::addr_zpr, &CPU::addr_zpr, &CPU::addr_zpr,
    &CPU::addr_imp, &CPU::addr_imm, &CPU::addr_imp, &CPU::addr_imm, &CPU::addr_abr, &CPU::addr_abr, &CPU::addr_abr, &CPU::addr_abr,
    &CPU::addr_rel, &CPU::addr_iyr, &CPU::addr_acc, &CPU::addr_iym, &CPU::addr_zxr, &CPU::addr_zxr, &CPU::addr_zxr, &CPU::addr_zxr,
    &CPU::addr_imp, &CPU::addr_ayr, &CPU::addr_imp, &CPU::addr_aym, &CPU::addr_axr, &CPU::addr_axr, &CPU::addr_axm, &CPU::addr_axm
};

using _op_ptr = void (CPU::*)();
const _op_ptr CPU::INSTRUCTIONS[256] = {
    &CPU::op_brk, &CPU::op_ora, &CPU::op_jam, &CPU::op_slo, &CPU::op_nop, &CPU::op_ora, &CPU::op_asl, &CPU::op_slo,
    &CPU::op_php, &CPU::op_ora, &CPU::op_aal, &CPU::op_anc, &CPU::op_nop, &CPU::op_ora, &CPU::op_asl, &CPU::op_slo,
    &CPU::op_bpl, &CPU::op_ora, &CPU::op_jam, &CPU::op_slo, &CPU::op_nop, &CPU::op_ora, &CPU::op_asl, &CPU::op_slo,
    &CPU::op_clc, &CPU::op_ora, &CPU::op_nop, &CPU::op_slo, &CPU::op_nop, &CPU::op_ora, &CPU::op_asl, &CPU::op_slo,
    &CPU::op_jsr, &CPU::op_and, &CPU::op_jam, &CPU::op_rla, &CPU::op_bit, &CPU::op_and, &CPU::op_rol, &CPU::op_rla,
    &CPU::op_plp, &CPU::op_and, &CPU::op_ral, &CPU::op_anc, &CPU::op_bit, &CPU::op_and, &CPU::op_rol, &CPU::op_rla,
    &CPU::op_bmi, &CPU::op_and, &CPU::op_jam, &CPU::op_rla, &CPU::op_nop, &CPU::op_and, &CPU::op_rol, &CPU::op_rla,
    &CPU::op_sec, &CPU::op_and, &CPU::op_nop, &CPU::op_rla, &CPU::op_nop, &CPU::op_and, &CPU::op_rol, &CPU::op_rla,
    &CPU::op_rti, &CPU::op_eor, &CPU::op_jam, &CPU::op_sre, &CPU::op_nop, &CPU::op_eor, &CPU::op_lsr, &CPU::op_sre,
    &CPU::op_pha, &CPU::op_eor, &CPU::op_lar, &CPU::op_alr, &CPU::op_jmp, &CPU::op_eor, &CPU::op_lsr, &CPU::op_sre,
    &CPU::op_bvc, &CPU::op_eor, &CPU::op_jam, &CPU::op_sre, &CPU::op_nop, &CPU::op_eor, &CPU::op_lsr, &CPU::op_sre,
    &CPU::op_cli, &CPU::op_eor, &CPU::op_nop, &CPU::op_sre, &CPU::op_nop, &CPU::op_eor, &CPU::op_lsr, &CPU::op_sre,
    &CPU::op_rts, &CPU::op_adc, &CPU::op_jam, &CPU::op_rra, &CPU::op_nop, &CPU::op_adc, &CPU::op_ror, &CPU::op_rra,
    &CPU::op_pla, &CPU::op_adc, &CPU::op_rar, &CPU::op_arr, &CPU::op_jmp, &CPU::op_adc, &CPU::op_ror, &CPU::op_rra,
    &CPU::op_bvs, &CPU::op_adc, &CPU::op_jam, &CPU::op_rra, &CPU::op_nop, &CPU::op_adc, &CPU::op_ror, &CPU::op_rra,
    &CPU::op_sei, &CPU::op_adc, &CPU::op_nop, &CPU::op_rra, &CPU::op_nop, &CPU::op_adc, &CPU::op_ror, &CPU::op_rra,
    &CPU::op_nop, &CPU::op_sta, &CPU::op_nop, &CPU::op_sax, &CPU::op_sty, &CPU::op_sta, &CPU::op_stx, &CPU::op_sax,
    &CPU::op_dey, &CPU::op_nop, &CPU::op_txa, &CPU::op_ane, &CPU::op_sty, &CPU::op_sta, &CPU::op_stx, &CPU::op_sax,
    &CPU::op_bcc, &CPU::op_sta, &CPU::op_jam, &CPU::op_sha, &CPU::op_sty, &CPU::op_sta, &CPU::op_stx, &CPU::op_sax,
    &CPU::op_tya, &CPU::op_sta, &CPU::op_txs, &CPU::op_tas, &CPU::op_shy, &CPU::op_sta, &CPU::op_shx, &CPU::op_sha,
    &CPU::op_ldy, &CPU::op_lda, &CPU::op_ldx, &CPU::op_lax, &CPU::op_ldy, &CPU::op_lda, &CPU::op_ldx, &CPU::op_lax,
    &CPU::op_tay, &CPU::op_lda, &CPU::op_tax, &CPU::op_lxa, &CPU::op_ldy, &CPU::op_lda, &CPU::op_ldx, &CPU::op_lax,
    &CPU::op_bcs, &CPU::op_lda, &CPU::op_jam, &CPU::op_lax, &CPU::op_ldy, &CPU::op_lda, &CPU::op_ldx, &CPU::op_lax,
    &CPU::op_clv, &CPU::op_lda, &CPU::op_tsx, &CPU::op_las, &CPU::op_ldy, &CPU::op_lda, &CPU::op_ldx, &CPU::op_lax,
    &CPU::op_cpy, &CPU::op_cmp, &CPU::op_nop, &CPU::op_dcp, &CPU::op_cpy, &CPU::op_cmp, &CPU::op_dec, &CPU::op_dcp,
    &CPU::op_iny, &CPU::op_cmp, &CPU::op_dex, &CPU::op_sbx, &CPU::op_cpy, &CPU::op_cmp, &CPU::op_dec, &CPU::op_dcp,
    &CPU::op_bne, &CPU::op_cmp, &CPU::op_jam, &CPU::op_dcp, &CPU::op_nop, &CPU::op_cmp, &CPU::op_dec, &CPU::op_dcp,
    &CPU::op_cld, &CPU::op_cmp, &CPU::op_nop, &CPU::op_dcp, &CPU::op_nop, &CPU::op_cmp, &CPU::op_dec, &CPU::op_dcp,
    &CPU::op_cpx, &CPU::op_sbc, &CPU::op_nop, &CPU::op_isc, &CPU::op_cpx, &CPU::op_sbc, &CPU::op_inc, &CPU::op_isc,
    &CPU::op_inx, &CPU::op_sbc, &CPU::op_nop, &CPU::op_usb, &CPU::op_cpx, &CPU::op_sbc, &CPU::op_inc, &CPU::op_isc,
    &CPU::op_beq, &CPU::op_sbc, &CPU::op_jam, &CPU::op_isc, &CPU::op_nop, &CPU::op_sbc, &CPU::op_inc, &CPU::op_isc,
    &CPU::op_sed, &CPU::op_sbc, &CPU::op_nop, &CPU::op_isc, &CPU::op_nop, &CPU::op_sbc, &CPU::op_inc, &CPU::op_isc
};


CPU::CPU(NES& nes)
: _nes{nes}
, _frozen{false}
, _register_a{0x00}
, _register_x{0x00}
, _register_y{0x00}
, _register_m{0x00}
, _stack_pointer{0x00}
, _program_counter{0x0000}
, _delay_interrupt{false}
, _should_issue_interrupt{false}
, _line_mapper_interrupt{false}
, _line_frame_interrupt{false}
, _line_delta_interrupt{false}
, _line_non_maskable_interrupt{false}
, _edge_detector_non_maskable_interrupt{false}
, _delay_non_maskable_interrupt{false}
, _should_issue_non_maskable_interrupt{false}
, _status{0x00}
, _target_address{0x0000} {}

void CPU::power() {
    _frozen = false;
    _line_non_maskable_interrupt = false;
    _line_mapper_interrupt = false;
    _line_frame_interrupt = false;
    _line_delta_interrupt = false;
    _should_issue_interrupt = false;
    _register_a = 0x00;
    _register_x = 0x00;
    _register_y = 0x00;
    _stack_pointer = 0xFD;
    _status = Flag::I;
    _program_counter = _nes.read_cpu(0xFFFC);
    _program_counter |= _nes.read_cpu(0xFFFD) << 8;
}

void CPU::reset() {
    _frozen = false;
    _line_non_maskable_interrupt = false;
    _line_mapper_interrupt = false;
    _line_frame_interrupt = false;
    _line_delta_interrupt = false;
    _stack_pointer -= 3;
    _status |= Flag::I;
    _program_counter = _nes.read_cpu(0xFFFC);
    _program_counter |= _nes.read_cpu(0xFFFD) << 8;
}

void CPU::tick() {
    if (_frozen) {
        return;
    }

    uint8_t instruction = fetch_next();

    (this->*ADDRESSING_MODES[instruction])();
    (this->*INSTRUCTIONS[instruction])();

    if (_delay_non_maskable_interrupt || _delay_interrupt) {
        _nes.read(_program_counter);
        _nes.read(_program_counter);

        _nes.write(0x100 | _stack_pointer--, _program_counter >> 8);
        _nes.write(0x100 | _stack_pointer--, _program_counter & 0x00FF);

        uint16_t address = _should_issue_non_maskable_interrupt ? 0xFFFA : 0xFFFE;

        _should_issue_non_maskable_interrupt = false;

        _nes.write(0x100 | _stack_pointer--, _status | Flag::U);

        set_status(Flag::I, true);

        _program_counter = _nes.read(address);
        _program_counter |= _nes.read(address + 1) << 8;
    }
}

void CPU::poll() {
    _delay_non_maskable_interrupt = _should_issue_non_maskable_interrupt;

    if (!_edge_detector_non_maskable_interrupt && _line_non_maskable_interrupt) {
        _should_issue_non_maskable_interrupt = true;
    }

    _edge_detector_non_maskable_interrupt = _line_non_maskable_interrupt;
    _delay_interrupt = _should_issue_interrupt;

    _should_issue_interrupt = (_line_mapper_interrupt || _line_frame_interrupt || _line_delta_interrupt) && !get_status(Flag::I);
}

void CPU::set_non_maskable_interrupt(bool interrupt) {
    _line_non_maskable_interrupt = interrupt;
}

void CPU::set_mapper_interrupt(bool interrupt) {
    _line_mapper_interrupt = interrupt;
}

void CPU::set_frame_interrupt(bool interrupt) {
    _line_frame_interrupt = interrupt;
}

void CPU::set_delta_interrupt(bool interrupt) {
    _line_delta_interrupt = interrupt;
}

bool CPU::is_frozen() const {
    return _frozen;
}

void CPU::stream_state(SaveState& save_state) {
    save_state.stream(_frozen);
    save_state.stream(_register_a);
    save_state.stream(_register_x);
    save_state.stream(_register_y);
    save_state.stream(_register_m);
    save_state.stream(_stack_pointer);
    save_state.stream(_program_counter);
    save_state.stream(_target_address);
    save_state.stream(_status);

    save_state.stream(_delay_interrupt);
    save_state.stream(_should_issue_interrupt);
    save_state.stream(_line_mapper_interrupt);
    save_state.stream(_line_frame_interrupt);
    save_state.stream(_line_delta_interrupt);
    save_state.stream(_line_non_maskable_interrupt);
    save_state.stream(_edge_detector_non_maskable_interrupt);
    save_state.stream(_delay_non_maskable_interrupt);
    save_state.stream(_should_issue_non_maskable_interrupt);
}

uint8_t CPU::fetch_next() {
    return _nes.read(_program_counter++);
}

void CPU::set_status(uint8_t flag, bool value) {
    if (value) {
        _status |= flag;
    } else {
        _status &= ~flag;
    }
}

bool CPU::get_status(uint8_t flag) const {
    return _status & flag;
}

void CPU::addr_abr() {
    addr_abw();
    _register_m = _nes.read(_target_address);
}

void CPU::addr_abw() {
    _target_address = fetch_next();
    _target_address |= fetch_next() << 8;
}

void CPU::addr_acc() {
    _register_m = _nes.read(_program_counter);
}

void CPU::addr_axm() {
    addr_axw();
    _register_m = _nes.read(_target_address);
}

void CPU::addr_axr() {
    _target_address = fetch_next();

    uint16_t translated = _target_address + _register_x;
    bool invalid_address = (_target_address & 0xFF00) != (translated & 0xFF00);

    _target_address = translated & 0x00FF;
    _target_address |= fetch_next() << 8;
    _register_m = _nes.read(_target_address);

    if (invalid_address) {
        _target_address += 0x100;
        _register_m = _nes.read(_target_address);
    }
}

void CPU::addr_axw() {
    _target_address = fetch_next();

    uint16_t translated = _target_address + _register_x;
    bool invalid_address = (_target_address & 0xFF00) != (translated & 0xFF00);

    _target_address = translated & 0x00FF;
    _target_address |= fetch_next() << 8;
    _register_m = _nes.read(_target_address);

    if (invalid_address) {
        _target_address += 0x100;
    }
}

void CPU::addr_aym() {
    addr_ayw();
    _register_m = _nes.read(_target_address);
}

void CPU::addr_ayr() {
    _target_address = fetch_next();

    uint16_t translated = _target_address + _register_y;
    bool invalid_address = (_target_address & 0xFF00) != (translated & 0xFF00);

    _target_address = translated & 0x00FF;
    _target_address |= fetch_next() << 8;
    _register_m = _nes.read(_target_address);

    if (invalid_address) {
        _target_address += 0x100;
        _register_m = _nes.read(_target_address);
    }
}

void CPU::addr_ayw() {
    _target_address = fetch_next();

    uint16_t translated = _target_address + _register_y;
    bool invalid_address = (_target_address & 0xFF00) != (translated & 0xFF00);

    _target_address = translated & 0x00FF;
    _target_address |= fetch_next() << 8;
    _register_m = _nes.read(_target_address);

    if (invalid_address) {
        _target_address += 0x100;
    }
}

void CPU::addr_imm() {
    _register_m = fetch_next();
}

void CPU::addr_imp() {
    _register_m = _nes.read(_program_counter);
}

void CPU::addr_ind() {
    uint16_t pointer = fetch_next();

    pointer |= fetch_next() << 8;

    if ((pointer & 0x00FF) == 0xFF) {
        _target_address = _nes.read(pointer);
        _target_address |= _nes.read(pointer & 0xFF00) << 8;
    } else {
        _target_address = _nes.read(pointer);
        _target_address |= _nes.read(pointer + 1) << 8;
    }
}

void CPU::addr_ixr() {
    addr_ixw();
    _register_m = _nes.read(_target_address);
}

void CPU::addr_ixw() {
    uint8_t pointer = fetch_next();

    _register_m = _nes.read(pointer);

    pointer += _register_x;

    _target_address = _nes.read(pointer);
    _target_address |= _nes.read(++pointer & 0xFF) << 8;
}

void CPU::addr_iym() {
    addr_iyw();
    _register_m = _nes.read(_target_address);
}

void CPU::addr_iyr() {
    uint8_t pointer = fetch_next();

    _target_address = _nes.read(pointer);

    uint16_t translated = _target_address + _register_y;
    bool invalid_address = translated & 0xFF00;

    _target_address = translated & 0x00FF;
    _target_address |= _nes.read(++pointer & 0xFF) << 8;
    _register_m = _nes.read(_target_address);

    if (invalid_address) {
        _target_address += 0x100;
        _register_m = _nes.read(_target_address);
    }
}

void CPU::addr_iyw() {
    uint8_t pointer = fetch_next();

    _target_address = _nes.read(pointer);

    uint16_t translated = _target_address + _register_y;
    bool invalid_address = (_target_address & 0xFF00) != (translated & 0xFF00);

    _target_address = translated & 0x00FF;
    _target_address |= _nes.read(++pointer & 0xFF) << 8;
    _register_m = _nes.read(_target_address);

    if (invalid_address) {
        _target_address += 0x100;
    }
}

void CPU::addr_rel() {
    _target_address = fetch_next();

    if (_target_address & 0x80) {
        _target_address |= 0xFF00;
    }
}

void CPU::addr_zpr() {
    addr_zpw();
    _register_m = _nes.read(_target_address);
}

void CPU::addr_zpw() {
    _target_address = fetch_next();
}

void CPU::addr_zxr() {
    addr_zxw();
    _register_m = _nes.read(_target_address);
}

void CPU::addr_zxw() {
    _target_address = fetch_next();
    _register_m = _nes.read(_target_address);
    _target_address += _register_x;
    _target_address &= 0x00FF;
}

void CPU::addr_zyr() {
    addr_zyw();
    _register_m = _nes.read(_target_address);
}

void CPU::addr_zyw() {
    _target_address = fetch_next();
    _register_m = _nes.read(_target_address);
    _target_address += _register_y;
    _target_address &= 0x00FF;
}

void CPU::op_aal() {
    set_status(Flag::C, _register_a & 0x80);

    _register_a <<= 1;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void CPU::op_adc() {
    uint16_t result = _register_a + _register_m + (get_status(Flag::C) ? 0x01 : 0x00);

    set_status(Flag::C, result & 0xFF00);
    set_status(Flag::V, ~(_register_a ^ _register_m) & (_register_a ^ result) & 0x80);

    _register_a = result & 0x00FF;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void CPU::op_alr() {
    _register_a &= _register_m;

    set_status(Flag::C, _register_a & 0x01);

    _register_a >>= 1;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void CPU::op_anc() {
    _register_a &= _register_m;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
    set_status(Flag::C, _register_a & 0x80);
}

void CPU::op_and() {
    _register_a &= _register_m;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void CPU::op_ane() {
    _register_a = (_register_a | 0xEE) & _register_x & _register_m;
}

void CPU::op_arr() {
    _register_a &= _register_m;
    _register_a = (get_status(Flag::C) ? 0x80 : 0x00) | (_register_a >> 1);

    set_status(Flag::C, _register_a & 0x40);
    set_status(Flag::V, bool(_register_a & 0x40) ^ bool(_register_a & 0x20));
    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void CPU::op_asl() {
    _nes.write(_target_address, _register_m);

    set_status(Flag::C, _register_m & 0x80);

    _register_m <<= 1;

    set_status(Flag::Z, !_register_m);
    set_status(Flag::N, _register_m & 0x80);

    _nes.write(_target_address, _register_m);
}

void CPU::op_bcc() {
    if (!get_status(Flag::C)) {
        if (_should_issue_interrupt && !_delay_interrupt) {
            _should_issue_interrupt = false;
        }

        _nes.read(_program_counter);

        uint16_t translated = _target_address + _program_counter;

        if ((translated & 0xFF00) != (_program_counter & 0xFF00)) {
            _nes.read(_program_counter);
        }

        _program_counter = translated;
    }
}

void CPU::op_bcs() {
    if (get_status(Flag::C)) {
        if (_should_issue_interrupt && !_delay_interrupt) {
            _should_issue_interrupt = false;
        }

        _nes.read(_program_counter);

        uint16_t translated = _target_address + _program_counter;

        if ((translated & 0xFF00) != (_program_counter & 0xFF00)) {
            _nes.read(_program_counter);
        }

        _program_counter = translated;
    }
}

void CPU::op_beq() {
    if (get_status(Flag::Z)) {
        if (_should_issue_interrupt && !_delay_interrupt) {
            _should_issue_interrupt = false;
        }

        _nes.read(_program_counter);

        uint16_t translated = _target_address + _program_counter;

        if ((translated & 0xFF00) != (_program_counter & 0xFF00)) {
            _nes.read(_program_counter);
        }

        _program_counter = translated;
    }
}

void CPU::op_bit() {
    set_status(Flag::Z, !(_register_a & _register_m));
    set_status(Flag::V, _register_m & 0x40);
    set_status(Flag::N, _register_m & 0x80);
}

void CPU::op_bmi() {
    if (get_status(Flag::N)) {
        if (_should_issue_interrupt && !_delay_interrupt) {
            _should_issue_interrupt = false;
        }

        _nes.read(_program_counter);

        uint16_t translated = _target_address + _program_counter;

        if ((translated & 0xFF00) != (_program_counter & 0xFF00)) {
            _nes.read(_program_counter);
        }

        _program_counter = translated;
    }
}

void CPU::op_bne() {
    if (!get_status(Flag::Z)) {
        if (_should_issue_interrupt && !_delay_interrupt) {
            _should_issue_interrupt = false;
        }

        _nes.read(_program_counter);

        uint16_t translated = _target_address + _program_counter;

        if ((translated & 0xFF00) != (_program_counter & 0xFF00)) {
            _nes.read(_program_counter);
        }

        _program_counter = translated;
    }
}

void CPU::op_bpl() {
    if (!get_status(Flag::N)) {
        if (_should_issue_interrupt && !_delay_interrupt) {
            _should_issue_interrupt = false;
        }

        _nes.read(_program_counter);

        uint16_t translated = _target_address + _program_counter;

        if ((translated & 0xFF00) != (_program_counter & 0xFF00)) {
            _nes.read(_program_counter);
        }

        _program_counter = translated;
    }
}

void CPU::op_brk() {
    _program_counter++;

    _nes.write(0x100 | _stack_pointer--, _program_counter >> 8);
    _nes.write(0x100 | _stack_pointer--, _program_counter & 0x00FF);

    uint16_t address = _should_issue_non_maskable_interrupt ? 0xFFFA : 0xFFFE;

    _should_issue_non_maskable_interrupt = false;

    _nes.write(0x100 | _stack_pointer--, _status | Flag::B | Flag::U);

    set_status(Flag::I, true);

    _program_counter = _nes.read(address);
    _program_counter |= _nes.read(address + 1) << 8;

    _delay_non_maskable_interrupt = false;
}

void CPU::op_bvc() {
    if (!get_status(Flag::V)) {
        if (_should_issue_interrupt && !_delay_interrupt) {
            _should_issue_interrupt = false;
        }

        _nes.read(_program_counter);

        uint16_t translated = _target_address + _program_counter;

        if ((translated & 0xFF00) != (_program_counter & 0xFF00)) {
            _nes.read(_program_counter);
        }

        _program_counter = translated;
    }
}

void CPU::op_bvs() {
    if (get_status(Flag::V)) {
        if (_should_issue_interrupt && !_delay_interrupt) {
            _should_issue_interrupt = false;
        }

        _nes.read(_program_counter);

        uint16_t translated = _target_address + _program_counter;

        if ((translated & 0xFF00) != (_program_counter & 0xFF00)) {
            _nes.read(_program_counter);
        }

        _program_counter = translated;
    }
}

void CPU::op_clc() {
    set_status(Flag::C, false);
}

void CPU::op_cld() {
    set_status(Flag::D, false);
}

void CPU::op_cli() {
    set_status(Flag::I, false);
}

void CPU::op_clv() {
    set_status(Flag::V, false);
}

void CPU::op_cmp() {
    set_status(Flag::C, _register_a >= _register_m);
    set_status(Flag::Z, _register_a == _register_m);
    set_status(Flag::N, (_register_a - _register_m) & 0x80);
}

void CPU::op_cpx() {
    set_status(Flag::C, _register_x >= _register_m);
    set_status(Flag::Z, _register_x == _register_m);
    set_status(Flag::N, (_register_x - _register_m) & 0x80);
}

void CPU::op_cpy() {
    set_status(Flag::C, _register_y >= _register_m);
    set_status(Flag::Z, _register_y == _register_m);
    set_status(Flag::N, (_register_y - _register_m) & 0x80);
}

void CPU::op_dcp() {
    _nes.write(_target_address, _register_m);

    _register_m--;

    set_status(Flag::C, _register_a >= _register_m);
    set_status(Flag::Z, _register_a == _register_m);
    set_status(Flag::N, (_register_a - _register_m) & 0x80);

    _nes.write(_target_address, _register_m);
}

void CPU::op_dec() {
    _nes.write(_target_address, _register_m);

    _register_m--;

    set_status(Flag::Z, !_register_m);
    set_status(Flag::N, _register_m & 0x80);

    _nes.write(_target_address, _register_m);
}

void CPU::op_dex() {
    _register_x--;

    set_status(Flag::Z, !_register_x);
    set_status(Flag::N, _register_x & 0x80);
}

void CPU::op_dey() {
    _register_y--;

    set_status(Flag::Z, !_register_y);
    set_status(Flag::N, _register_y & 0x80);
}

void CPU::op_eor() {
    _register_a ^= _register_m;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void CPU::op_inc() {
    _nes.write(_target_address, _register_m);

    _register_m++;

    set_status(Flag::Z, !_register_m);
    set_status(Flag::N, _register_m & 0x80);

    _nes.write(_target_address, _register_m);
}

void CPU::op_inx() {
    _register_x++;

    set_status(Flag::Z, !_register_x);
    set_status(Flag::N, _register_x & 0x80);
}

void CPU::op_iny() {
    _register_y++;

    set_status(Flag::Z, !_register_y);
    set_status(Flag::N, _register_y & 0x80);
}

void CPU::op_isc() {
    _nes.write(_target_address, _register_m);

    _register_m++;

    uint8_t value = _register_m;

    _register_m ^= 0xFF;

    uint16_t result = _register_a + _register_m + (get_status(Flag::C) ? 0x01 : 0x00);

    set_status(Flag::C, result & 0x0100);
    set_status(Flag::V, ~(_register_a ^ _register_m) & (_register_a ^ result) & 0x80);

    _register_a = result & 0x00FF;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);

    _nes.write(_target_address, value);
}

void CPU::op_jam() {
    _frozen = true;
}

void CPU::op_jmp() {
    _program_counter = _target_address;
}

void CPU::op_jsr() {
    _nes.read(_program_counter);

    _program_counter--;

    _nes.write(0x100 | _stack_pointer--, _program_counter >> 8);
    _nes.write(0x100 | _stack_pointer--, _program_counter & 0x00FF);

    _program_counter = _target_address;
}

void CPU::op_lar() {
    set_status(Flag::C, _register_a & 0x01);

    _register_a >>= 1;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void CPU::op_las() {
    uint8_t result = _register_m & _stack_pointer;

    _register_a = result;
    _register_x = result;
    _stack_pointer = result;
}

void CPU::op_lax() {
    _register_a = _register_m;
    _register_x = _register_m;

    set_status(Flag::Z, !_register_m);
    set_status(Flag::N, _register_m & 0x80);
}

void CPU::op_lda() {
    _register_a = _register_m;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void CPU::op_ldx() {
    _register_x = _register_m;

    set_status(Flag::Z, !_register_x);
    set_status(Flag::N, _register_x & 0x80);
}

void CPU::op_ldy() {
    _register_y = _register_m;

    set_status(Flag::Z, !_register_y);
    set_status(Flag::N, _register_y & 0x80);
}

void CPU::op_lsr() {
    _nes.write(_target_address, _register_m);

    set_status(Flag::C, _register_m & 0x01);

    _register_m >>= 1;

    set_status(Flag::Z, !_register_m);
    set_status(Flag::N, _register_m & 0x80);

    _nes.write(_target_address, _register_m);
}

void CPU::op_lxa() {
    _register_a = _register_m;
    _register_x = _register_m;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void CPU::op_nop() {}

void CPU::op_ora() {
    _register_a |= _register_m;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void CPU::op_pha() {
    _nes.write(0x100 | _stack_pointer--, _register_a);
}

void CPU::op_php() {
    _nes.write(0x100 | _stack_pointer--, _status | Flag::B | Flag::U);
}

void CPU::op_pla() {
    _stack_pointer++;
    _nes.read(_program_counter);
    _register_a = _nes.read(0x100 | _stack_pointer);

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void CPU::op_plp() {
    _stack_pointer++;
    _nes.read(_program_counter);
    _status = _nes.read(0x100 | _stack_pointer) & 0xCF;
}

void CPU::op_ral() {
    bool carry = _register_a & 0x80;

    _register_a = (get_status(Flag::C) ? 0x01 : 0x00) | (_register_a << 1);

    set_status(Flag::C, carry);
    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void CPU::op_rar() {
    bool carry = _register_a & 0x01;

    _register_a = (get_status(Flag::C) ? 0x80 : 0x00) | (_register_a >> 1);

    set_status(Flag::C, carry);
    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void CPU::op_rla() {
    _nes.write(_target_address, _register_m);

    bool carry = _register_m & 0x80;

    _register_m = (get_status(Flag::C) ? 0x01 : 0x00) | (_register_m << 1);
    _register_a &= _register_m;

    set_status(Flag::C, carry);
    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);

    _nes.write(_target_address, _register_m);
}

void CPU::op_rol() {
    _nes.write(_target_address, _register_m);

    bool carry = _register_m & 0x80;

    _register_m = (get_status(Flag::C) ? 0x01 : 0x00) | (_register_m << 1);

    set_status(Flag::C, carry);
    set_status(Flag::Z, !_register_m);
    set_status(Flag::N, _register_m & 0x80);

    _nes.write(_target_address, _register_m);
}

void CPU::op_ror() {
    _nes.write(_target_address, _register_m);

    bool carry = _register_m & 0x01;

    _register_m = (get_status(Flag::C) ? 0x80 : 0x00) | (_register_m >> 1);

    set_status(Flag::C, carry);
    set_status(Flag::Z, !_register_m);
    set_status(Flag::N, _register_m & 0x80);

    _nes.write(_target_address, _register_m);
}

void CPU::op_rra() {
    _nes.write(_target_address, _register_m);

    uint8_t carry = _register_m & 0x01;

    _register_m = (get_status(Flag::C) ? 0x80 : 0x00) | (_register_m >> 1);

    uint16_t result = _register_a + _register_m + carry;

    set_status(Flag::C, result & 0x0100);
    set_status(Flag::V, ~(_register_a ^ _register_m) & (_register_a ^ result) & 0x80);

    _register_a = result & 0x00FF;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);

    _nes.write(_target_address, _register_m);
}

void CPU::op_rti() {
    _stack_pointer++;
    _nes.read(_program_counter);
    _status = _nes.read(0x100 | _stack_pointer) & 0xCF;
    _program_counter = _nes.read(0x100 | ++_stack_pointer);
    _program_counter |= _nes.read(0x100 | ++_stack_pointer) << 8;
}

void CPU::op_rts() {
    _stack_pointer++;

    _nes.read(_program_counter);
    _nes.read(_program_counter);

    _program_counter = _nes.read(0x100 | _stack_pointer);
    _program_counter |= _nes.read(0x100 | ++_stack_pointer) << 8;
    _program_counter++;
}

void CPU::op_sax() {
    _nes.write(_target_address, _register_a & _register_x);
}

void CPU::op_sbc() {
    _register_m ^= 0xFF;

    uint16_t result = _register_a + _register_m + (get_status(Flag::C) ? 0x01 : 0x00);

    set_status(Flag::C, result & 0xFF00);
    set_status(Flag::V, ~(_register_a ^ _register_m) & (_register_a ^ result) & 0x80);

    _register_a = result & 0x00FF;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void CPU::op_sbx() {
    _register_x &= _register_a;

    set_status(Flag::C, _register_x >= _register_m);
    set_status(Flag::Z, _register_x == _register_m);

    _register_x -= _register_m;

    set_status(Flag::N, _register_x & 0x80);
}

void CPU::op_sec() {
    set_status(Flag::C, true);
}

void CPU::op_sed() {
    set_status(Flag::D, true);
}

void CPU::op_sei() {
    set_status(Flag::I, true);
}

void CPU::op_sha() {
    _nes.write(_target_address, _register_a & _register_x & (uint8_t(_target_address >> 8) + 1));
}

void CPU::op_shx() {
    uint8_t address_high = 1 + (_target_address >> 8);

    _nes.write(((_register_x & address_high) << 8) | (_target_address & 0xFF), _register_x & address_high);
}

void CPU::op_shy() {
    uint8_t address_high = 1 + (_target_address >> 8);

    _nes.write(((_register_y & address_high) << 8) | (_target_address & 0xFF), _register_y & address_high);
}

void CPU::op_slo() {
    _nes.write(_target_address, _register_m);

    set_status(Flag::C, _register_m & 0x80);

    _register_m <<= 1;
    _register_a |= _register_m;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);

    _nes.write(_target_address, _register_m);
}

void CPU::op_sre() {
    _nes.write(_target_address, _register_m);

    set_status(Flag::C, _register_m & 0x01);

    _register_m >>= 1;
    _register_a ^= _register_m;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);

    _nes.write(_target_address, _register_m);
}

void CPU::op_sta() {
    _nes.write(_target_address, _register_a);
}

void CPU::op_stx() {
    _nes.write(_target_address, _register_x);
}

void CPU::op_sty() {
    _nes.write(_target_address, _register_y);
}

void CPU::op_tas() {
    _stack_pointer = _register_a & _register_x;

    _nes.write(_target_address, _stack_pointer & (uint8_t(_target_address >> 8) + 1));
}

void CPU::op_tax() {
    _register_x = _register_a;

    set_status(Flag::Z, !_register_x);
    set_status(Flag::N, _register_x & 0x80);
}

void CPU::op_tay() {
    _register_y = _register_a;

    set_status(Flag::Z, !_register_y);
    set_status(Flag::N, _register_y & 0x80);
}

void CPU::op_tsx() {
    _register_x = _stack_pointer;

    set_status(Flag::Z, !_register_x);
    set_status(Flag::N, _register_x & 0x80);
}

void CPU::op_txa() {
    _register_a = _register_x;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void CPU::op_txs() {
    _stack_pointer = _register_x;
}

void CPU::op_tya() {
    _register_a = _register_y;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void CPU::op_usb() {
    _register_m ^= 0xFF;

    uint16_t result = _register_a + _register_m + (get_status(Flag::C) ? 0x01 : 0x00);

    set_status(Flag::C, result & 0x0100);
    set_status(Flag::V, ~(_register_a ^ _register_m) & (_register_a ^ result) & 0x80);

    _register_a = result & 0x00FF;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}
