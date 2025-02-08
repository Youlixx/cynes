#include "cpu.hpp"
#include "nes.hpp"

#include <cstring>

using _addr_ptr = void (cynes::CPU::*)();;
const _addr_ptr cynes::CPU::ADDRESSING_MODES[256] = {
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ixr, &cynes::CPU::addr_acc, &cynes::CPU::addr_ixr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_acc, &cynes::CPU::addr_imm, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr,
    &cynes::CPU::addr_rel, &cynes::CPU::addr_iyr, &cynes::CPU::addr_acc, &cynes::CPU::addr_iym, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ayr, &cynes::CPU::addr_imp, &cynes::CPU::addr_aym, &cynes::CPU::addr_axr, &cynes::CPU::addr_axr, &cynes::CPU::addr_axm, &cynes::CPU::addr_axm,
    &cynes::CPU::addr_abw, &cynes::CPU::addr_ixr, &cynes::CPU::addr_acc, &cynes::CPU::addr_ixr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_acc, &cynes::CPU::addr_imm, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr,
    &cynes::CPU::addr_rel, &cynes::CPU::addr_iyr, &cynes::CPU::addr_acc, &cynes::CPU::addr_iym, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ayr, &cynes::CPU::addr_imp, &cynes::CPU::addr_aym, &cynes::CPU::addr_axr, &cynes::CPU::addr_axr, &cynes::CPU::addr_axm, &cynes::CPU::addr_axm,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ixr, &cynes::CPU::addr_acc, &cynes::CPU::addr_ixr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_acc, &cynes::CPU::addr_imm, &cynes::CPU::addr_abw, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr,
    &cynes::CPU::addr_rel, &cynes::CPU::addr_iyr, &cynes::CPU::addr_acc, &cynes::CPU::addr_iym, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ayr, &cynes::CPU::addr_imp, &cynes::CPU::addr_aym, &cynes::CPU::addr_axr, &cynes::CPU::addr_axr, &cynes::CPU::addr_axm, &cynes::CPU::addr_axm,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ixr, &cynes::CPU::addr_acc, &cynes::CPU::addr_ixr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_acc, &cynes::CPU::addr_imm, &cynes::CPU::addr_ind, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr,
    &cynes::CPU::addr_rel, &cynes::CPU::addr_iyr, &cynes::CPU::addr_acc, &cynes::CPU::addr_iym, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ayr, &cynes::CPU::addr_imp, &cynes::CPU::addr_aym, &cynes::CPU::addr_axr, &cynes::CPU::addr_axr, &cynes::CPU::addr_axm, &cynes::CPU::addr_axm,
    &cynes::CPU::addr_imm, &cynes::CPU::addr_ixw, &cynes::CPU::addr_imm, &cynes::CPU::addr_ixw, &cynes::CPU::addr_zpw, &cynes::CPU::addr_zpw, &cynes::CPU::addr_zpw, &cynes::CPU::addr_zpw,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_abw, &cynes::CPU::addr_abw, &cynes::CPU::addr_abw, &cynes::CPU::addr_abw,
    &cynes::CPU::addr_rel, &cynes::CPU::addr_iyw, &cynes::CPU::addr_acc, &cynes::CPU::addr_iyw, &cynes::CPU::addr_zxw, &cynes::CPU::addr_zxw, &cynes::CPU::addr_zyw, &cynes::CPU::addr_zyw,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ayw, &cynes::CPU::addr_imp, &cynes::CPU::addr_ayw, &cynes::CPU::addr_axw, &cynes::CPU::addr_axw, &cynes::CPU::addr_ayw, &cynes::CPU::addr_ayw,
    &cynes::CPU::addr_imm, &cynes::CPU::addr_ixr, &cynes::CPU::addr_imm, &cynes::CPU::addr_ixr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr,
    &cynes::CPU::addr_rel, &cynes::CPU::addr_iyr, &cynes::CPU::addr_acc, &cynes::CPU::addr_iyr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zyr, &cynes::CPU::addr_zyr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ayr, &cynes::CPU::addr_imp, &cynes::CPU::addr_ayr, &cynes::CPU::addr_axr, &cynes::CPU::addr_axr, &cynes::CPU::addr_ayr, &cynes::CPU::addr_ayr,
    &cynes::CPU::addr_imm, &cynes::CPU::addr_ixr, &cynes::CPU::addr_imm, &cynes::CPU::addr_ixr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr,
    &cynes::CPU::addr_rel, &cynes::CPU::addr_iyr, &cynes::CPU::addr_acc, &cynes::CPU::addr_iym, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ayr, &cynes::CPU::addr_imp, &cynes::CPU::addr_aym, &cynes::CPU::addr_axr, &cynes::CPU::addr_axr, &cynes::CPU::addr_axm, &cynes::CPU::addr_axm,
    &cynes::CPU::addr_imm, &cynes::CPU::addr_ixr, &cynes::CPU::addr_imm, &cynes::CPU::addr_ixr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr,
    &cynes::CPU::addr_rel, &cynes::CPU::addr_iyr, &cynes::CPU::addr_acc, &cynes::CPU::addr_iym, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ayr, &cynes::CPU::addr_imp, &cynes::CPU::addr_aym, &cynes::CPU::addr_axr, &cynes::CPU::addr_axr, &cynes::CPU::addr_axm, &cynes::CPU::addr_axm
};

using _op_ptr = void (cynes::CPU::*)();;
const _op_ptr cynes::CPU::INSTRUCTIONS[256] = {
    &cynes::CPU::op_brk, &cynes::CPU::op_ora, &cynes::CPU::op_jam, &cynes::CPU::op_slo, &cynes::CPU::op_nop, &cynes::CPU::op_ora, &cynes:: CPU::op_asl, &cynes::CPU::op_slo,
    &cynes::CPU::op_php, &cynes::CPU::op_ora, &cynes::CPU::op_aal, &cynes::CPU::op_anc, &cynes::CPU::op_nop, &cynes::CPU::op_ora, &cynes:: CPU::op_asl, &cynes::CPU::op_slo,
    &cynes::CPU::op_bpl, &cynes::CPU::op_ora, &cynes::CPU::op_jam, &cynes::CPU::op_slo, &cynes::CPU::op_nop, &cynes::CPU::op_ora, &cynes:: CPU::op_asl, &cynes::CPU::op_slo,
    &cynes::CPU::op_clc, &cynes::CPU::op_ora, &cynes::CPU::op_nop, &cynes::CPU::op_slo, &cynes::CPU::op_nop, &cynes::CPU::op_ora, &cynes:: CPU::op_asl, &cynes::CPU::op_slo,
    &cynes::CPU::op_jsr, &cynes::CPU::op_and, &cynes::CPU::op_jam, &cynes::CPU::op_rla, &cynes::CPU::op_bit, &cynes::CPU::op_and, &cynes:: CPU::op_rol, &cynes::CPU::op_rla,
    &cynes::CPU::op_plp, &cynes::CPU::op_and, &cynes::CPU::op_ral, &cynes::CPU::op_anc, &cynes::CPU::op_bit, &cynes::CPU::op_and, &cynes:: CPU::op_rol, &cynes::CPU::op_rla,
    &cynes::CPU::op_bmi, &cynes::CPU::op_and, &cynes::CPU::op_jam, &cynes::CPU::op_rla, &cynes::CPU::op_nop, &cynes::CPU::op_and, &cynes:: CPU::op_rol, &cynes::CPU::op_rla,
    &cynes::CPU::op_sec, &cynes::CPU::op_and, &cynes::CPU::op_nop, &cynes::CPU::op_rla, &cynes::CPU::op_nop, &cynes::CPU::op_and, &cynes:: CPU::op_rol, &cynes::CPU::op_rla,
    &cynes::CPU::op_rti, &cynes::CPU::op_eor, &cynes::CPU::op_jam, &cynes::CPU::op_sre, &cynes::CPU::op_nop, &cynes::CPU::op_eor, &cynes:: CPU::op_lsr, &cynes::CPU::op_sre,
    &cynes::CPU::op_pha, &cynes::CPU::op_eor, &cynes::CPU::op_lar, &cynes::CPU::op_alr, &cynes::CPU::op_jmp, &cynes::CPU::op_eor, &cynes:: CPU::op_lsr, &cynes::CPU::op_sre,
    &cynes::CPU::op_bvc, &cynes::CPU::op_eor, &cynes::CPU::op_jam, &cynes::CPU::op_sre, &cynes::CPU::op_nop, &cynes::CPU::op_eor, &cynes:: CPU::op_lsr, &cynes::CPU::op_sre,
    &cynes::CPU::op_cli, &cynes::CPU::op_eor, &cynes::CPU::op_nop, &cynes::CPU::op_sre, &cynes::CPU::op_nop, &cynes::CPU::op_eor, &cynes:: CPU::op_lsr, &cynes::CPU::op_sre,
    &cynes::CPU::op_rts, &cynes::CPU::op_adc, &cynes::CPU::op_jam, &cynes::CPU::op_rra, &cynes::CPU::op_nop, &cynes::CPU::op_adc, &cynes:: CPU::op_ror, &cynes::CPU::op_rra,
    &cynes::CPU::op_pla, &cynes::CPU::op_adc, &cynes::CPU::op_rar, &cynes::CPU::op_arr, &cynes::CPU::op_jmp, &cynes::CPU::op_adc, &cynes:: CPU::op_ror, &cynes::CPU::op_rra,
    &cynes::CPU::op_bvs, &cynes::CPU::op_adc, &cynes::CPU::op_jam, &cynes::CPU::op_rra, &cynes::CPU::op_nop, &cynes::CPU::op_adc, &cynes:: CPU::op_ror, &cynes::CPU::op_rra,
    &cynes::CPU::op_sei, &cynes::CPU::op_adc, &cynes::CPU::op_nop, &cynes::CPU::op_rra, &cynes::CPU::op_nop, &cynes::CPU::op_adc, &cynes:: CPU::op_ror, &cynes::CPU::op_rra,
    &cynes::CPU::op_nop, &cynes::CPU::op_sta, &cynes::CPU::op_nop, &cynes::CPU::op_sax, &cynes::CPU::op_sty, &cynes::CPU::op_sta, &cynes:: CPU::op_stx, &cynes::CPU::op_sax,
    &cynes::CPU::op_dey, &cynes::CPU::op_nop, &cynes::CPU::op_txa, &cynes::CPU::op_ane, &cynes::CPU::op_sty, &cynes::CPU::op_sta, &cynes:: CPU::op_stx, &cynes::CPU::op_sax,
    &cynes::CPU::op_bcc, &cynes::CPU::op_sta, &cynes::CPU::op_jam, &cynes::CPU::op_sha, &cynes::CPU::op_sty, &cynes::CPU::op_sta, &cynes:: CPU::op_stx, &cynes::CPU::op_sax,
    &cynes::CPU::op_tya, &cynes::CPU::op_sta, &cynes::CPU::op_txs, &cynes::CPU::op_tas, &cynes::CPU::op_shy, &cynes::CPU::op_sta, &cynes:: CPU::op_shx, &cynes::CPU::op_sha,
    &cynes::CPU::op_ldy, &cynes::CPU::op_lda, &cynes::CPU::op_ldx, &cynes::CPU::op_lax, &cynes::CPU::op_ldy, &cynes::CPU::op_lda, &cynes:: CPU::op_ldx, &cynes::CPU::op_lax,
    &cynes::CPU::op_tay, &cynes::CPU::op_lda, &cynes::CPU::op_tax, &cynes::CPU::op_lxa, &cynes::CPU::op_ldy, &cynes::CPU::op_lda, &cynes:: CPU::op_ldx, &cynes::CPU::op_lax,
    &cynes::CPU::op_bcs, &cynes::CPU::op_lda, &cynes::CPU::op_jam, &cynes::CPU::op_lax, &cynes::CPU::op_ldy, &cynes::CPU::op_lda, &cynes:: CPU::op_ldx, &cynes::CPU::op_lax,
    &cynes::CPU::op_clv, &cynes::CPU::op_lda, &cynes::CPU::op_tsx, &cynes::CPU::op_las, &cynes::CPU::op_ldy, &cynes::CPU::op_lda, &cynes:: CPU::op_ldx, &cynes::CPU::op_lax,
    &cynes::CPU::op_cpy, &cynes::CPU::op_cmp, &cynes::CPU::op_nop, &cynes::CPU::op_dcp, &cynes::CPU::op_cpy, &cynes::CPU::op_cmp, &cynes:: CPU::op_dec, &cynes::CPU::op_dcp,
    &cynes::CPU::op_iny, &cynes::CPU::op_cmp, &cynes::CPU::op_dex, &cynes::CPU::op_sbx, &cynes::CPU::op_cpy, &cynes::CPU::op_cmp, &cynes:: CPU::op_dec, &cynes::CPU::op_dcp,
    &cynes::CPU::op_bne, &cynes::CPU::op_cmp, &cynes::CPU::op_jam, &cynes::CPU::op_dcp, &cynes::CPU::op_nop, &cynes::CPU::op_cmp, &cynes:: CPU::op_dec, &cynes::CPU::op_dcp,
    &cynes::CPU::op_cld, &cynes::CPU::op_cmp, &cynes::CPU::op_nop, &cynes::CPU::op_dcp, &cynes::CPU::op_nop, &cynes::CPU::op_cmp, &cynes:: CPU::op_dec, &cynes::CPU::op_dcp,
    &cynes::CPU::op_cpx, &cynes::CPU::op_sbc, &cynes::CPU::op_nop, &cynes::CPU::op_isc, &cynes::CPU::op_cpx, &cynes::CPU::op_sbc, &cynes:: CPU::op_inc, &cynes::CPU::op_isc,
    &cynes::CPU::op_inx, &cynes::CPU::op_sbc, &cynes::CPU::op_nop, &cynes::CPU::op_usb, &cynes::CPU::op_cpx, &cynes::CPU::op_sbc, &cynes:: CPU::op_inc, &cynes::CPU::op_isc,
    &cynes::CPU::op_beq, &cynes::CPU::op_sbc, &cynes::CPU::op_jam, &cynes::CPU::op_isc, &cynes::CPU::op_nop, &cynes::CPU::op_sbc, &cynes:: CPU::op_inc, &cynes::CPU::op_isc,
    &cynes::CPU::op_sed, &cynes::CPU::op_sbc, &cynes::CPU::op_nop, &cynes::CPU::op_isc, &cynes::CPU::op_nop, &cynes::CPU::op_sbc, &cynes:: CPU::op_inc, &cynes::CPU::op_isc
};


cynes::CPU::CPU(NES& nes)
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

void cynes::CPU::power() {
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

void cynes::CPU::reset() {
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

void cynes::CPU::tick() {
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

void cynes::CPU::poll() {
    _delay_non_maskable_interrupt = _should_issue_non_maskable_interrupt;

    if (!_edge_detector_non_maskable_interrupt && _line_non_maskable_interrupt) {
        _should_issue_non_maskable_interrupt = true;
    }

    _edge_detector_non_maskable_interrupt = _line_non_maskable_interrupt;
    _delay_interrupt = _should_issue_interrupt;

    _should_issue_interrupt = (_line_mapper_interrupt || _line_frame_interrupt || _line_delta_interrupt) && !get_status(Flag::I);
}

void cynes::CPU::set_non_maskable_interrupt(bool interrupt) {
    _line_non_maskable_interrupt = interrupt;
}

void cynes::CPU::set_mapper_interrupt(bool interrupt) {
    _line_mapper_interrupt = interrupt;
}

void cynes::CPU::set_frame_interrupt(bool interrupt) {
    _line_frame_interrupt = interrupt;
}

void cynes::CPU::set_delta_interrupt(bool interrupt) {
    _line_delta_interrupt = interrupt;
}

bool cynes::CPU::is_frozen() const {
    return _frozen;
}

uint8_t cynes::CPU::fetch_next() {
    return _nes.read(_program_counter++);
}

void cynes::CPU::set_status(uint8_t flag, bool value) {
    if (value) {
        _status |= flag;
    } else {
        _status &= ~flag;
    }
}

bool cynes::CPU::get_status(uint8_t flag) const {
    return _status & flag;
}

void cynes::CPU::addr_abr() {
    addr_abw();
    _register_m = _nes.read(_target_address);
}

void cynes::CPU::addr_abw() {
    _target_address = fetch_next();
    _target_address |= fetch_next() << 8;
}

void cynes::CPU::addr_acc() {
    _register_m = _nes.read(_program_counter);
}

void cynes::CPU::addr_axm() {
    addr_axw();
    _register_m = _nes.read(_target_address);
}

void cynes::CPU::addr_axr() {
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

void cynes::CPU::addr_axw() {
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

void cynes::CPU::addr_aym() {
    addr_ayw();
    _register_m = _nes.read(_target_address);
}

void cynes::CPU::addr_ayr() {
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

void cynes::CPU::addr_ayw() {
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

void cynes::CPU::addr_imm() {
    _register_m = fetch_next();
}

void cynes::CPU::addr_imp() {
    _register_m = _nes.read(_program_counter);
}

void cynes::CPU::addr_ind() {
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

void cynes::CPU::addr_ixr() {
    addr_ixw();
    _register_m = _nes.read(_target_address);
}

void cynes::CPU::addr_ixw() {
    uint8_t pointer = fetch_next();

    _register_m = _nes.read(pointer);

    pointer += _register_x;

    _target_address = _nes.read(pointer);
    _target_address |= _nes.read(++pointer & 0xFF) << 8;
}

void cynes::CPU::addr_iym() {
    addr_iyw();
    _register_m = _nes.read(_target_address);
}

void cynes::CPU::addr_iyr() {
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

void cynes::CPU::addr_iyw() {
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

void cynes::CPU::addr_rel() {
    _target_address = fetch_next();

    if (_target_address & 0x80) {
        _target_address |= 0xFF00;
    }
}

void cynes::CPU::addr_zpr() {
    addr_zpw();
    _register_m = _nes.read(_target_address);
}

void cynes::CPU::addr_zpw() {
    _target_address = fetch_next();
}

void cynes::CPU::addr_zxr() {
    addr_zxw();
    _register_m = _nes.read(_target_address);
}

void cynes::CPU::addr_zxw() {
    _target_address = fetch_next();
    _register_m = _nes.read(_target_address);
    _target_address += _register_x;
    _target_address &= 0x00FF;
}

void cynes::CPU::addr_zyr() {
    addr_zyw();
    _register_m = _nes.read(_target_address);
}

void cynes::CPU::addr_zyw() {
    _target_address = fetch_next();
    _register_m = _nes.read(_target_address);
    _target_address += _register_y;
    _target_address &= 0x00FF;
}

void cynes::CPU::op_aal() {
    set_status(Flag::C, _register_a & 0x80);

    _register_a <<= 1;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void cynes::CPU::op_adc() {
    uint16_t result = _register_a + _register_m + (get_status(Flag::C) ? 0x01 : 0x00);

    set_status(Flag::C, result & 0xFF00);
    set_status(Flag::V, ~(_register_a ^ _register_m) & (_register_a ^ result) & 0x80);

    _register_a = result & 0x00FF;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void cynes::CPU::op_alr() {
    _register_a &= _register_m;

    set_status(Flag::C, _register_a & 0x01);

    _register_a >>= 1;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void cynes::CPU::op_anc() {
    _register_a &= _register_m;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
    set_status(Flag::C, _register_a & 0x80);
}

void cynes::CPU::op_and() {
    _register_a &= _register_m;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void cynes::CPU::op_ane() {
    _register_a = (_register_a | 0xEE) & _register_x & _register_m;
}

void cynes::CPU::op_arr() {
    _register_a &= _register_m;
    _register_a = (get_status(Flag::C) ? 0x80 : 0x00) | (_register_a >> 1);

    set_status(Flag::C, _register_a & 0x40);
    set_status(Flag::V, bool(_register_a & 0x40) ^ bool(_register_a & 0x20));
    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void cynes::CPU::op_asl() {
    _nes.write(_target_address, _register_m);

    set_status(Flag::C, _register_m & 0x80);

    _register_m <<= 1;

    set_status(Flag::Z, !_register_m);
    set_status(Flag::N, _register_m & 0x80);

    _nes.write(_target_address, _register_m);
}

void cynes::CPU::op_bcc() {
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

void cynes::CPU::op_bcs() {
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

void cynes::CPU::op_beq() {
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

void cynes::CPU::op_bit() {
    set_status(Flag::Z, !(_register_a & _register_m));
    set_status(Flag::V, _register_m & 0x40);
    set_status(Flag::N, _register_m & 0x80);
}

void cynes::CPU::op_bmi() {
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

void cynes::CPU::op_bne() {
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

void cynes::CPU::op_bpl() {
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

void cynes::CPU::op_brk() {
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

void cynes::CPU::op_bvc() {
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

void cynes::CPU::op_bvs() {
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

void cynes::CPU::op_clc() {
    set_status(Flag::C, false);
}

void cynes::CPU::op_cld() {
    set_status(Flag::D, false);
}

void cynes::CPU::op_cli() {
    set_status(Flag::I, false);
}

void cynes::CPU::op_clv() {
    set_status(Flag::V, false);
}

void cynes::CPU::op_cmp() {
    set_status(Flag::C, _register_a >= _register_m);
    set_status(Flag::Z, _register_a == _register_m);
    set_status(Flag::N, (_register_a - _register_m) & 0x80);
}

void cynes::CPU::op_cpx() {
    set_status(Flag::C, _register_x >= _register_m);
    set_status(Flag::Z, _register_x == _register_m);
    set_status(Flag::N, (_register_x - _register_m) & 0x80);
}

void cynes::CPU::op_cpy() {
    set_status(Flag::C, _register_y >= _register_m);
    set_status(Flag::Z, _register_y == _register_m);
    set_status(Flag::N, (_register_y - _register_m) & 0x80);
}

void cynes::CPU::op_dcp() {
    _nes.write(_target_address, _register_m);

    _register_m--;

    set_status(Flag::C, _register_a >= _register_m);
    set_status(Flag::Z, _register_a == _register_m);
    set_status(Flag::N, (_register_a - _register_m) & 0x80);

    _nes.write(_target_address, _register_m);
}

void cynes::CPU::op_dec() {
    _nes.write(_target_address, _register_m);

    _register_m--;

    set_status(Flag::Z, !_register_m);
    set_status(Flag::N, _register_m & 0x80);

    _nes.write(_target_address, _register_m);
}

void cynes::CPU::op_dex() {
    _register_x--;

    set_status(Flag::Z, !_register_x);
    set_status(Flag::N, _register_x & 0x80);
}

void cynes::CPU::op_dey() {
    _register_y--;

    set_status(Flag::Z, !_register_y);
    set_status(Flag::N, _register_y & 0x80);
}

void cynes::CPU::op_eor() {
    _register_a ^= _register_m;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void cynes::CPU::op_inc() {
    _nes.write(_target_address, _register_m);

    _register_m++;

    set_status(Flag::Z, !_register_m);
    set_status(Flag::N, _register_m & 0x80);

    _nes.write(_target_address, _register_m);
}

void cynes::CPU::op_inx() {
    _register_x++;

    set_status(Flag::Z, !_register_x);
    set_status(Flag::N, _register_x & 0x80);
}

void cynes::CPU::op_iny() {
    _register_y++;

    set_status(Flag::Z, !_register_y);
    set_status(Flag::N, _register_y & 0x80);
}

void cynes::CPU::op_isc() {
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

void cynes::CPU::op_jam() {
    _frozen = true;
}

void cynes::CPU::op_jmp() {
    _program_counter = _target_address;
}

void cynes::CPU::op_jsr() {
    _nes.read(_program_counter);

    _program_counter--;

    _nes.write(0x100 | _stack_pointer--, _program_counter >> 8);
    _nes.write(0x100 | _stack_pointer--, _program_counter & 0x00FF);

    _program_counter = _target_address;
}

void cynes::CPU::op_lar() {
    set_status(Flag::C, _register_a & 0x01);

    _register_a >>= 1;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void cynes::CPU::op_las() {
    uint8_t result = _register_m & _stack_pointer;

    _register_a = result;
    _register_x = result;
    _stack_pointer = result;
}

void cynes::CPU::op_lax() {
    _register_a = _register_m;
    _register_x = _register_m;

    set_status(Flag::Z, !_register_m);
    set_status(Flag::N, _register_m & 0x80);
}

void cynes::CPU::op_lda() {
    _register_a = _register_m;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void cynes::CPU::op_ldx() {
    _register_x = _register_m;

    set_status(Flag::Z, !_register_x);
    set_status(Flag::N, _register_x & 0x80);
}

void cynes::CPU::op_ldy() {
    _register_y = _register_m;

    set_status(Flag::Z, !_register_y);
    set_status(Flag::N, _register_y & 0x80);
}

void cynes::CPU::op_lsr() {
    _nes.write(_target_address, _register_m);

    set_status(Flag::C, _register_m & 0x01);

    _register_m >>= 1;

    set_status(Flag::Z, !_register_m);
    set_status(Flag::N, _register_m & 0x80);

    _nes.write(_target_address, _register_m);
}

void cynes::CPU::op_lxa() {
    _register_a = _register_m;
    _register_x = _register_m;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void cynes::CPU::op_nop() {}

void cynes::CPU::op_ora() {
    _register_a |= _register_m;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void cynes::CPU::op_pha() {
    _nes.write(0x100 | _stack_pointer--, _register_a);
}

void cynes::CPU::op_php() {
    _nes.write(0x100 | _stack_pointer--, _status | Flag::B | Flag::U);
}

void cynes::CPU::op_pla() {
    _stack_pointer++;
    _nes.read(_program_counter);
    _register_a = _nes.read(0x100 | _stack_pointer);

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void cynes::CPU::op_plp() {
    _stack_pointer++;
    _nes.read(_program_counter);
    _status = _nes.read(0x100 | _stack_pointer) & 0xCF;
}

void cynes::CPU::op_ral() {
    bool carry = _register_a & 0x80;

    _register_a = (get_status(Flag::C) ? 0x01 : 0x00) | (_register_a << 1);

    set_status(Flag::C, carry);
    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void cynes::CPU::op_rar() {
    bool carry = _register_a & 0x01;

    _register_a = (get_status(Flag::C) ? 0x80 : 0x00) | (_register_a >> 1);

    set_status(Flag::C, carry);
    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void cynes::CPU::op_rla() {
    _nes.write(_target_address, _register_m);

    bool carry = _register_m & 0x80;

    _register_m = (get_status(Flag::C) ? 0x01 : 0x00) | (_register_m << 1);
    _register_a &= _register_m;

    set_status(Flag::C, carry);
    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);

    _nes.write(_target_address, _register_m);
}

void cynes::CPU::op_rol() {
    _nes.write(_target_address, _register_m);

    bool carry = _register_m & 0x80;

    _register_m = (get_status(Flag::C) ? 0x01 : 0x00) | (_register_m << 1);

    set_status(Flag::C, carry);
    set_status(Flag::Z, !_register_m);
    set_status(Flag::N, _register_m & 0x80);

    _nes.write(_target_address, _register_m);
}

void cynes::CPU::op_ror() {
    _nes.write(_target_address, _register_m);

    bool carry = _register_m & 0x01;

    _register_m = (get_status(Flag::C) ? 0x80 : 0x00) | (_register_m >> 1);

    set_status(Flag::C, carry);
    set_status(Flag::Z, !_register_m);
    set_status(Flag::N, _register_m & 0x80);

    _nes.write(_target_address, _register_m);
}

void cynes::CPU::op_rra() {
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

void cynes::CPU::op_rti() {
    _stack_pointer++;
    _nes.read(_program_counter);
    _status = _nes.read(0x100 | _stack_pointer) & 0xCF;
    _program_counter = _nes.read(0x100 | ++_stack_pointer);
    _program_counter |= _nes.read(0x100 | ++_stack_pointer) << 8;
}

void cynes::CPU::op_rts() {
    _stack_pointer++;

    _nes.read(_program_counter);
    _nes.read(_program_counter);

    _program_counter = _nes.read(0x100 | _stack_pointer);
    _program_counter |= _nes.read(0x100 | ++_stack_pointer) << 8;
    _program_counter++;
}

void cynes::CPU::op_sax() {
    _nes.write(_target_address, _register_a & _register_x);
}

void cynes::CPU::op_sbc() {
    _register_m ^= 0xFF;

    uint16_t result = _register_a + _register_m + (get_status(Flag::C) ? 0x01 : 0x00);

    set_status(Flag::C, result & 0xFF00);
    set_status(Flag::V, ~(_register_a ^ _register_m) & (_register_a ^ result) & 0x80);

    _register_a = result & 0x00FF;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void cynes::CPU::op_sbx() {
    _register_x &= _register_a;

    set_status(Flag::C, _register_x >= _register_m);
    set_status(Flag::Z, _register_x == _register_m);

    _register_x -= _register_m;

    set_status(Flag::N, _register_x & 0x80);
}

void cynes::CPU::op_sec() {
    set_status(Flag::C, true);
}

void cynes::CPU::op_sed() {
    set_status(Flag::D, true);
}

void cynes::CPU::op_sei() {
    set_status(Flag::I, true);
}

void cynes::CPU::op_sha() {
    _nes.write(_target_address, _register_a & _register_x & (uint8_t(_target_address >> 8) + 1));
}

void cynes::CPU::op_shx() {
    uint8_t address_high = 1 + (_target_address >> 8);

    _nes.write(((_register_x & address_high) << 8) | (_target_address & 0xFF), _register_x & address_high);
}

void cynes::CPU::op_shy() {
    uint8_t address_high = 1 + (_target_address >> 8);

    _nes.write(((_register_y & address_high) << 8) | (_target_address & 0xFF), _register_y & address_high);
}

void cynes::CPU::op_slo() {
    _nes.write(_target_address, _register_m);

    set_status(Flag::C, _register_m & 0x80);

    _register_m <<= 1;
    _register_a |= _register_m;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);

    _nes.write(_target_address, _register_m);
}

void cynes::CPU::op_sre() {
    _nes.write(_target_address, _register_m);

    set_status(Flag::C, _register_m & 0x01);

    _register_m >>= 1;
    _register_a ^= _register_m;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);

    _nes.write(_target_address, _register_m);
}

void cynes::CPU::op_sta() {
    _nes.write(_target_address, _register_a);
}

void cynes::CPU::op_stx() {
    _nes.write(_target_address, _register_x);
}

void cynes::CPU::op_sty() {
    _nes.write(_target_address, _register_y);
}

void cynes::CPU::op_tas() {
    _stack_pointer = _register_a & _register_x;

    _nes.write(_target_address, _stack_pointer & (uint8_t(_target_address >> 8) + 1));
}

void cynes::CPU::op_tax() {
    _register_x = _register_a;

    set_status(Flag::Z, !_register_x);
    set_status(Flag::N, _register_x & 0x80);
}

void cynes::CPU::op_tay() {
    _register_y = _register_a;

    set_status(Flag::Z, !_register_y);
    set_status(Flag::N, _register_y & 0x80);
}

void cynes::CPU::op_tsx() {
    _register_x = _stack_pointer;

    set_status(Flag::Z, !_register_x);
    set_status(Flag::N, _register_x & 0x80);
}

void cynes::CPU::op_txa() {
    _register_a = _register_x;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void cynes::CPU::op_txs() {
    _stack_pointer = _register_x;
}

void cynes::CPU::op_tya() {
    _register_a = _register_y;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}

void cynes::CPU::op_usb() {
    _register_m ^= 0xFF;

    uint16_t result = _register_a + _register_m + (get_status(Flag::C) ? 0x01 : 0x00);

    set_status(Flag::C, result & 0x0100);
    set_status(Flag::V, ~(_register_a ^ _register_m) & (_register_a ^ result) & 0x80);

    _register_a = result & 0x00FF;

    set_status(Flag::Z, !_register_a);
    set_status(Flag::N, _register_a & 0x80);
}
