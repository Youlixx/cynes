#ifndef __CYNES_CPU__
#define __CYNES_CPU__

#include <cstdint>

#include "utils.hpp"

namespace cynes {
// Forward declaration.
class NES;

/// NES 6502 CPU implementation (see https://www.nesdev.org/wiki/CPU).
class CPU {
public:
    /// Initialize the CPU.
    CPU(NES& nes);
    ~CPU() = default;

public:
    /// Set the CPU in its power-up state.
    void power();

    /// Set the CPU in its reset state.
    void reset();

    /// Tick the CPU.
    void tick();

    /// Poll the CPU for the non-maskable interrupt.
    void poll();

    /// Set the non-maskable interrupt flag value.
    /// @param interrupt Non-maskable interrupt value.
    void set_non_maskable_interrupt(bool interrupt);

    /// Set the state of the mapper interrupt.
    /// @param interrupt Interrupt state.
    void set_mapper_interrupt(bool interrupt);

    /// Set the state of the frame interrupt.
    /// @param interrupt Interrupt state.
    void set_frame_interrupt(bool interrupt);

    /// Set the state of the delta interrupt.
    /// @param interrupt Interrupt state.
    void set_delta_interrupt(bool interrupt);

    /// Check whether or not the CPU has hit an invalid opcode.
    bool is_frozen() const;

private:
    NES& _nes;

private:
    bool _frozen;

    uint8_t _register_a;
    uint8_t _register_x;
    uint8_t _register_y;
    uint8_t _register_m;
    uint8_t _stack_pointer;

    uint16_t _program_counter;

    uint8_t fetch_next();

private:
    bool _delay_interrupt;
    bool _should_issue_interrupt;

    bool _line_mapper_interrupt;
    bool _line_frame_interrupt;
    bool _line_delta_interrupt;

    bool _line_non_maskable_interrupt;
    bool _edge_detector_non_maskable_interrupt;

    bool _delay_non_maskable_interrupt;
    bool _should_issue_non_maskable_interrupt;

private:
    uint8_t _status;

    void set_status(uint8_t flag, bool value);
    bool get_status(uint8_t flag) const;

    enum Flag : uint8_t {
        C = 0x01, Z = 0x02, I = 0x04, D = 0x08, B = 0x10, U = 0x20, V = 0x40, N = 0x80
    };

private:
    uint16_t _target_address;

    void addr_abr();
    void addr_abw();
    void addr_acc();
    void addr_axm();
    void addr_axr();
    void addr_axw();
    void addr_aym();
    void addr_ayr();
    void addr_ayw();
    void addr_imm();
    void addr_imp();
    void addr_ind();
    void addr_ixr();
    void addr_ixw();
    void addr_iym();
    void addr_iyr();
    void addr_iyw();
    void addr_rel();
    void addr_zpr();
    void addr_zpw();
    void addr_zxr();
    void addr_zxw();
    void addr_zyr();
    void addr_zyw();

    void (CPU::* _addressing_modes[256]) (void) = {
        &CPU::addr_imp,&CPU::addr_ixr,&CPU::addr_acc,&CPU::addr_ixr,&CPU::addr_zpr,&CPU::addr_zpr,&CPU::addr_zpr,&CPU::addr_zpr,
        &CPU::addr_imp,&CPU::addr_imm,&CPU::addr_acc,&CPU::addr_imm,&CPU::addr_abr,&CPU::addr_abr,&CPU::addr_abr,&CPU::addr_abr,
        &CPU::addr_rel,&CPU::addr_iyr,&CPU::addr_acc,&CPU::addr_iym,&CPU::addr_zxr,&CPU::addr_zxr,&CPU::addr_zxr,&CPU::addr_zxr,
        &CPU::addr_imp,&CPU::addr_ayr,&CPU::addr_imp,&CPU::addr_aym,&CPU::addr_axr,&CPU::addr_axr,&CPU::addr_axm,&CPU::addr_axm,
        &CPU::addr_abw,&CPU::addr_ixr,&CPU::addr_acc,&CPU::addr_ixr,&CPU::addr_zpr,&CPU::addr_zpr,&CPU::addr_zpr,&CPU::addr_zpr,
        &CPU::addr_imp,&CPU::addr_imm,&CPU::addr_acc,&CPU::addr_imm,&CPU::addr_abr,&CPU::addr_abr,&CPU::addr_abr,&CPU::addr_abr,
        &CPU::addr_rel,&CPU::addr_iyr,&CPU::addr_acc,&CPU::addr_iym,&CPU::addr_zxr,&CPU::addr_zxr,&CPU::addr_zxr,&CPU::addr_zxr,
        &CPU::addr_imp,&CPU::addr_ayr,&CPU::addr_imp,&CPU::addr_aym,&CPU::addr_axr,&CPU::addr_axr,&CPU::addr_axm,&CPU::addr_axm,
        &CPU::addr_imp,&CPU::addr_ixr,&CPU::addr_acc,&CPU::addr_ixr,&CPU::addr_zpr,&CPU::addr_zpr,&CPU::addr_zpr,&CPU::addr_zpr,
        &CPU::addr_imp,&CPU::addr_imm,&CPU::addr_acc,&CPU::addr_imm,&CPU::addr_abw,&CPU::addr_abr,&CPU::addr_abr,&CPU::addr_abr,
        &CPU::addr_rel,&CPU::addr_iyr,&CPU::addr_acc,&CPU::addr_iym,&CPU::addr_zxr,&CPU::addr_zxr,&CPU::addr_zxr,&CPU::addr_zxr,
        &CPU::addr_imp,&CPU::addr_ayr,&CPU::addr_imp,&CPU::addr_aym,&CPU::addr_axr,&CPU::addr_axr,&CPU::addr_axm,&CPU::addr_axm,
        &CPU::addr_imp,&CPU::addr_ixr,&CPU::addr_acc,&CPU::addr_ixr,&CPU::addr_zpr,&CPU::addr_zpr,&CPU::addr_zpr,&CPU::addr_zpr,
        &CPU::addr_imp,&CPU::addr_imm,&CPU::addr_acc,&CPU::addr_imm,&CPU::addr_ind,&CPU::addr_abr,&CPU::addr_abr,&CPU::addr_abr,
        &CPU::addr_rel,&CPU::addr_iyr,&CPU::addr_acc,&CPU::addr_iym,&CPU::addr_zxr,&CPU::addr_zxr,&CPU::addr_zxr,&CPU::addr_zxr,
        &CPU::addr_imp,&CPU::addr_ayr,&CPU::addr_imp,&CPU::addr_aym,&CPU::addr_axr,&CPU::addr_axr,&CPU::addr_axm,&CPU::addr_axm,
        &CPU::addr_imm,&CPU::addr_ixw,&CPU::addr_imm,&CPU::addr_ixw,&CPU::addr_zpw,&CPU::addr_zpw,&CPU::addr_zpw,&CPU::addr_zpw,
        &CPU::addr_imp,&CPU::addr_imm,&CPU::addr_imp,&CPU::addr_imm,&CPU::addr_abw,&CPU::addr_abw,&CPU::addr_abw,&CPU::addr_abw,
        &CPU::addr_rel,&CPU::addr_iyw,&CPU::addr_acc,&CPU::addr_iyw,&CPU::addr_zxw,&CPU::addr_zxw,&CPU::addr_zyw,&CPU::addr_zyw,
        &CPU::addr_imp,&CPU::addr_ayw,&CPU::addr_imp,&CPU::addr_ayw,&CPU::addr_axw,&CPU::addr_axw,&CPU::addr_ayw,&CPU::addr_ayw,
        &CPU::addr_imm,&CPU::addr_ixr,&CPU::addr_imm,&CPU::addr_ixr,&CPU::addr_zpr,&CPU::addr_zpr,&CPU::addr_zpr,&CPU::addr_zpr,
        &CPU::addr_imp,&CPU::addr_imm,&CPU::addr_imp,&CPU::addr_imm,&CPU::addr_abr,&CPU::addr_abr,&CPU::addr_abr,&CPU::addr_abr,
        &CPU::addr_rel,&CPU::addr_iyr,&CPU::addr_acc,&CPU::addr_iyr,&CPU::addr_zxr,&CPU::addr_zxr,&CPU::addr_zyr,&CPU::addr_zyr,
        &CPU::addr_imp,&CPU::addr_ayr,&CPU::addr_imp,&CPU::addr_ayr,&CPU::addr_axr,&CPU::addr_axr,&CPU::addr_ayr,&CPU::addr_ayr,
        &CPU::addr_imm,&CPU::addr_ixr,&CPU::addr_imm,&CPU::addr_ixr,&CPU::addr_zpr,&CPU::addr_zpr,&CPU::addr_zpr,&CPU::addr_zpr,
        &CPU::addr_imp,&CPU::addr_imm,&CPU::addr_imp,&CPU::addr_imm,&CPU::addr_abr,&CPU::addr_abr,&CPU::addr_abr,&CPU::addr_abr,
        &CPU::addr_rel,&CPU::addr_iyr,&CPU::addr_acc,&CPU::addr_iym,&CPU::addr_zxr,&CPU::addr_zxr,&CPU::addr_zxr,&CPU::addr_zxr,
        &CPU::addr_imp,&CPU::addr_ayr,&CPU::addr_imp,&CPU::addr_aym,&CPU::addr_axr,&CPU::addr_axr,&CPU::addr_axm,&CPU::addr_axm,
        &CPU::addr_imm,&CPU::addr_ixr,&CPU::addr_imm,&CPU::addr_ixr,&CPU::addr_zpr,&CPU::addr_zpr,&CPU::addr_zpr,&CPU::addr_zpr,
        &CPU::addr_imp,&CPU::addr_imm,&CPU::addr_imp,&CPU::addr_imm,&CPU::addr_abr,&CPU::addr_abr,&CPU::addr_abr,&CPU::addr_abr,
        &CPU::addr_rel,&CPU::addr_iyr,&CPU::addr_acc,&CPU::addr_iym,&CPU::addr_zxr,&CPU::addr_zxr,&CPU::addr_zxr,&CPU::addr_zxr,
        &CPU::addr_imp,&CPU::addr_ayr,&CPU::addr_imp,&CPU::addr_aym,&CPU::addr_axr,&CPU::addr_axr,&CPU::addr_axm,&CPU::addr_axm
    };

private:
    void op_aal();
    void op_adc();
    void op_alr();
    void op_anc();
    void op_and();
    void op_ane();
    void op_arr();
    void op_asl();
    void op_bcc();
    void op_bcs();
    void op_beq();
    void op_bit();
    void op_bmi();
    void op_bne();
    void op_bpl();
    void op_brk();
    void op_bvc();
    void op_bvs();
    void op_clc();
    void op_cld();
    void op_cli();
    void op_clv();
    void op_cmp();
    void op_cpx();
    void op_cpy();
    void op_dcp();
    void op_dec();
    void op_dex();
    void op_dey();
    void op_eor();
    void op_inc();
    void op_inx();
    void op_iny();
    void op_isc();
    void op_jam();
    void op_jmp();
    void op_jsr();
    void op_lar();
    void op_las();
    void op_lax();
    void op_lda();
    void op_ldx();
    void op_ldy();
    void op_lsr();
    void op_lxa();
    void op_nop();
    void op_ora();
    void op_pha();
    void op_php();
    void op_pla();
    void op_plp();
    void op_ral();
    void op_rar();
    void op_rla();
    void op_rol();
    void op_ror();
    void op_rra();
    void op_rti();
    void op_rts();
    void op_sax();
    void op_sbc();
    void op_sbx();
    void op_sec();
    void op_sed();
    void op_sei();
    void op_sha();
    void op_shx();
    void op_shy();
    void op_slo();
    void op_sre();
    void op_sta();
    void op_stx();
    void op_sty();
    void op_tas();
    void op_tax();
    void op_tay();
    void op_tsx();
    void op_txa();
    void op_txs();
    void op_tya();
    void op_usb();

    void (CPU::* _instructions[256]) (void) = {
        &CPU::op_brk,&CPU::op_ora,&CPU::op_jam,&CPU::op_slo,&CPU::op_nop,&CPU::op_ora,&CPU::op_asl,&CPU::op_slo,
        &CPU::op_php,&CPU::op_ora,&CPU::op_aal,&CPU::op_anc,&CPU::op_nop,&CPU::op_ora,&CPU::op_asl,&CPU::op_slo,
        &CPU::op_bpl,&CPU::op_ora,&CPU::op_jam,&CPU::op_slo,&CPU::op_nop,&CPU::op_ora,&CPU::op_asl,&CPU::op_slo,
        &CPU::op_clc,&CPU::op_ora,&CPU::op_nop,&CPU::op_slo,&CPU::op_nop,&CPU::op_ora,&CPU::op_asl,&CPU::op_slo,
        &CPU::op_jsr,&CPU::op_and,&CPU::op_jam,&CPU::op_rla,&CPU::op_bit,&CPU::op_and,&CPU::op_rol,&CPU::op_rla,
        &CPU::op_plp,&CPU::op_and,&CPU::op_ral,&CPU::op_anc,&CPU::op_bit,&CPU::op_and,&CPU::op_rol,&CPU::op_rla,
        &CPU::op_bmi,&CPU::op_and,&CPU::op_jam,&CPU::op_rla,&CPU::op_nop,&CPU::op_and,&CPU::op_rol,&CPU::op_rla,
        &CPU::op_sec,&CPU::op_and,&CPU::op_nop,&CPU::op_rla,&CPU::op_nop,&CPU::op_and,&CPU::op_rol,&CPU::op_rla,
        &CPU::op_rti,&CPU::op_eor,&CPU::op_jam,&CPU::op_sre,&CPU::op_nop,&CPU::op_eor,&CPU::op_lsr,&CPU::op_sre,
        &CPU::op_pha,&CPU::op_eor,&CPU::op_lar,&CPU::op_alr,&CPU::op_jmp,&CPU::op_eor,&CPU::op_lsr,&CPU::op_sre,
        &CPU::op_bvc,&CPU::op_eor,&CPU::op_jam,&CPU::op_sre,&CPU::op_nop,&CPU::op_eor,&CPU::op_lsr,&CPU::op_sre,
        &CPU::op_cli,&CPU::op_eor,&CPU::op_nop,&CPU::op_sre,&CPU::op_nop,&CPU::op_eor,&CPU::op_lsr,&CPU::op_sre,
        &CPU::op_rts,&CPU::op_adc,&CPU::op_jam,&CPU::op_rra,&CPU::op_nop,&CPU::op_adc,&CPU::op_ror,&CPU::op_rra,
        &CPU::op_pla,&CPU::op_adc,&CPU::op_rar,&CPU::op_arr,&CPU::op_jmp,&CPU::op_adc,&CPU::op_ror,&CPU::op_rra,
        &CPU::op_bvs,&CPU::op_adc,&CPU::op_jam,&CPU::op_rra,&CPU::op_nop,&CPU::op_adc,&CPU::op_ror,&CPU::op_rra,
        &CPU::op_sei,&CPU::op_adc,&CPU::op_nop,&CPU::op_rra,&CPU::op_nop,&CPU::op_adc,&CPU::op_ror,&CPU::op_rra,
        &CPU::op_nop,&CPU::op_sta,&CPU::op_nop,&CPU::op_sax,&CPU::op_sty,&CPU::op_sta,&CPU::op_stx,&CPU::op_sax,
        &CPU::op_dey,&CPU::op_nop,&CPU::op_txa,&CPU::op_ane,&CPU::op_sty,&CPU::op_sta,&CPU::op_stx,&CPU::op_sax,
        &CPU::op_bcc,&CPU::op_sta,&CPU::op_jam,&CPU::op_sha,&CPU::op_sty,&CPU::op_sta,&CPU::op_stx,&CPU::op_sax,
        &CPU::op_tya,&CPU::op_sta,&CPU::op_txs,&CPU::op_tas,&CPU::op_shy,&CPU::op_sta,&CPU::op_shx,&CPU::op_sha,
        &CPU::op_ldy,&CPU::op_lda,&CPU::op_ldx,&CPU::op_lax,&CPU::op_ldy,&CPU::op_lda,&CPU::op_ldx,&CPU::op_lax,
        &CPU::op_tay,&CPU::op_lda,&CPU::op_tax,&CPU::op_lxa,&CPU::op_ldy,&CPU::op_lda,&CPU::op_ldx,&CPU::op_lax,
        &CPU::op_bcs,&CPU::op_lda,&CPU::op_jam,&CPU::op_lax,&CPU::op_ldy,&CPU::op_lda,&CPU::op_ldx,&CPU::op_lax,
        &CPU::op_clv,&CPU::op_lda,&CPU::op_tsx,&CPU::op_las,&CPU::op_ldy,&CPU::op_lda,&CPU::op_ldx,&CPU::op_lax,
        &CPU::op_cpy,&CPU::op_cmp,&CPU::op_nop,&CPU::op_dcp,&CPU::op_cpy,&CPU::op_cmp,&CPU::op_dec,&CPU::op_dcp,
        &CPU::op_iny,&CPU::op_cmp,&CPU::op_dex,&CPU::op_sbx,&CPU::op_cpy,&CPU::op_cmp,&CPU::op_dec,&CPU::op_dcp,
        &CPU::op_bne,&CPU::op_cmp,&CPU::op_jam,&CPU::op_dcp,&CPU::op_nop,&CPU::op_cmp,&CPU::op_dec,&CPU::op_dcp,
        &CPU::op_cld,&CPU::op_cmp,&CPU::op_nop,&CPU::op_dcp,&CPU::op_nop,&CPU::op_cmp,&CPU::op_dec,&CPU::op_dcp,
        &CPU::op_cpx,&CPU::op_sbc,&CPU::op_nop,&CPU::op_isc,&CPU::op_cpx,&CPU::op_sbc,&CPU::op_inc,&CPU::op_isc,
        &CPU::op_inx,&CPU::op_sbc,&CPU::op_nop,&CPU::op_usb,&CPU::op_cpx,&CPU::op_sbc,&CPU::op_inc,&CPU::op_isc,
        &CPU::op_beq,&CPU::op_sbc,&CPU::op_jam,&CPU::op_isc,&CPU::op_nop,&CPU::op_sbc,&CPU::op_inc,&CPU::op_isc,
        &CPU::op_sed,&CPU::op_sbc,&CPU::op_nop,&CPU::op_isc,&CPU::op_nop,&CPU::op_sbc,&CPU::op_inc,&CPU::op_isc
    };

public:
    template<DumpOperation operation, typename T>
    constexpr void dump(T& buffer) {
        cynes::dump<operation>(buffer, _frozen);
        cynes::dump<operation>(buffer, _register_a);
        cynes::dump<operation>(buffer, _register_x);
        cynes::dump<operation>(buffer, _register_y);
        cynes::dump<operation>(buffer, _register_m);
        cynes::dump<operation>(buffer, _stack_pointer);
        cynes::dump<operation>(buffer, _program_counter);
        cynes::dump<operation>(buffer, _target_address);
        cynes::dump<operation>(buffer, _status);

        cynes::dump<operation>(buffer, _delay_interrupt);
        cynes::dump<operation>(buffer, _should_issue_interrupt);
        cynes::dump<operation>(buffer, _line_mapper_interrupt);
        cynes::dump<operation>(buffer, _line_frame_interrupt);
        cynes::dump<operation>(buffer, _line_delta_interrupt);
        cynes::dump<operation>(buffer, _line_non_maskable_interrupt);
        cynes::dump<operation>(buffer, _edge_detector_non_maskable_interrupt);
        cynes::dump<operation>(buffer, _delay_non_maskable_interrupt);
        cynes::dump<operation>(buffer, _should_issue_non_maskable_interrupt);
    }
};
}

#endif
