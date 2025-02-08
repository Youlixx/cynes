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

    /// Default destructor.
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

    using _addr_ptr = void (CPU::*)();
    static const _addr_ptr ADDRESSING_MODES[256];

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

    using _op_ptr = void (CPU::*)();
    static const _op_ptr INSTRUCTIONS[256];

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
