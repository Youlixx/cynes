"""Module containing CPU accuracy related test."""

import pytest

from cynes import NES_INPUT_SELECT, NES_INPUT_START
from tests.utils.test_rom import (
    FrameCountedNES,
    MatchCondition,
    Matcher,
    run_test_rom_ppu,
    run_test_rom_ram,
)
from tests.utils.text_parsing import (
    CHARACTER_MAP_DEFAULT,
    CHARACTER_MAP_NESTEST,
    CHARACTER_MAP_RESTRAINED,
    parse_text_from_frame,
)


@pytest.mark.parametrize(
    "path_rom", [
        "blargg_nes_cpu_test5/cpu.nes",
        "blargg_nes_cpu_test5/official.nes",
    ]
)
def test_blargg_nes_cpu_test5(path_rom: str) -> None:
    """Run the blargg_nes_cpu_test5 test suite."""
    run_test_rom_ppu(
        path_rom=path_rom,
        success_matcher=Matcher(
            string="All tests complete",
            condition=MatchCondition.LAST_LINE_STRICT
        ),
        failure_matcher=Matcher(
            string="Failed",
            condition=MatchCondition.LAST_LINE_STRICT
        ),
        character_map=CHARACTER_MAP_DEFAULT
    )

@pytest.mark.parametrize(
    "path_rom", [
        "branch_timing_tests/1.Branch_Basics.nes",
        "branch_timing_tests/2.Backward_Branch.nes",
        "branch_timing_tests/3.Forward_Branch.nes",
    ]
)
def test_branch_timing_tests(path_rom: str) -> None:
    """Run the branch_timing_tests test suite."""
    run_test_rom_ppu(
        path_rom=path_rom,
        success_matcher=Matcher(
            string="PASSED",
            condition=MatchCondition.LAST_LINE_STRICT
        ),
        failure_matcher=Matcher(
            string="FAILED",
            condition=MatchCondition.LAST_LINE_STRICT
        ),
        character_map=CHARACTER_MAP_RESTRAINED
    )

@pytest.mark.parametrize(
    "path_rom", ["cpu_dummy_reads/cpu_dummy_reads.nes"]
)
def test_cpu_dummy_reads(path_rom: str) -> None:
    """Run the cpu_dummy_reads test suite."""
    run_test_rom_ppu(
        path_rom=path_rom,
        success_matcher=Matcher(
            string="Passed",
            condition=MatchCondition.LAST_LINE_STRICT
        ),
        failure_matcher=Matcher(
            string="Failed",
            condition=MatchCondition.CONTAINS
        ),
        character_map=CHARACTER_MAP_DEFAULT
    )

@pytest.mark.parametrize(
    "path_rom", [
        "cpu_dummy_writes/cpu_dummy_writes_oam.nes",
        "cpu_dummy_writes/cpu_dummy_writes_ppumem.nes",
    ]
)
def test_cpu_dummy_writes(path_rom: str) -> None:
    """Run the cpu_dummy_writes test suite."""
    run_test_rom_ram(path_rom=path_rom)

@pytest.mark.parametrize(
    "path_rom", [
        "cpu_exec_space/test_cpu_exec_space_apu.nes",
        "cpu_exec_space/test_cpu_exec_space_ppuio.nes",
    ]
)
def test_cpu_exec_space(path_rom: str) -> None:
    """Run the cpu_exec_space test suite."""
    run_test_rom_ram(path_rom=path_rom)

@pytest.mark.parametrize(
    "path_rom", [
        "cpu_interrupts_v2/rom_singles/1-cli_latency.nes",
        "cpu_interrupts_v2/rom_singles/2-nmi_and_brk.nes",
        "cpu_interrupts_v2/rom_singles/3-nmi_and_irq.nes",
        "cpu_interrupts_v2/rom_singles/4-irq_and_dma.nes",
        "cpu_interrupts_v2/rom_singles/5-branch_delays_irq.nes",
        "cpu_interrupts_v2/cpu_interrupts.nes",
    ]
)
def test_cpu_interrupts_v2(path_rom: str) -> None:
    """Run the cpu_interrupts_v2 test suite."""
    run_test_rom_ram(path_rom=path_rom)

@pytest.mark.parametrize(
    "path_rom", [
        "cpu_reset/ram_after_reset.nes",
        "cpu_reset/registers.nes",
    ]
)
def test_cpu_reset(path_rom: str) -> None:
    """Run the cpu_reset test suite."""
    run_test_rom_ram(path_rom=path_rom)

@pytest.mark.parametrize(
    "path_rom", ["cpu_timing_test6/cpu_timing_test.nes"]
)
def test_cpu_timing_test6(path_rom: str) -> None:
    """Run the cpu_timing_test6 test suite."""
    run_test_rom_ppu(
        path_rom=path_rom,
        success_matcher=Matcher(
            string="PASSED",
            condition=MatchCondition.LAST_LINE_STRICT
        ),
        failure_matcher=Matcher(
            string="FAILED",
            condition=MatchCondition.CONTAINS
        ),
        character_map=CHARACTER_MAP_RESTRAINED
    )

@pytest.mark.parametrize(
    "path_rom", ["instr_misc/instr_misc.nes"]
)
def test_instr_misc(path_rom: str) -> None:
    """Run the instr_misc test suite."""
    run_test_rom_ram(path_rom=path_rom)

@pytest.mark.parametrize(
    "path_rom", [
        "instr_test-v3/rom_singles/01-implied.nes",
        "instr_test-v3/rom_singles/02-immediate.nes",
        "instr_test-v3/rom_singles/03-zero_page.nes",
        "instr_test-v3/rom_singles/04-zp_xy.nes",
        "instr_test-v3/rom_singles/05-absolute.nes",
        "instr_test-v3/rom_singles/06-abs_xy.nes",
        "instr_test-v3/rom_singles/07-ind_x.nes",
        "instr_test-v3/rom_singles/08-ind_y.nes",
        "instr_test-v3/rom_singles/09-branches.nes",
        "instr_test-v3/rom_singles/10-stack.nes",
        "instr_test-v3/rom_singles/11-jmp_jsr.nes",
        "instr_test-v3/rom_singles/12-rts.nes",
        "instr_test-v3/rom_singles/13-rti.nes",
        "instr_test-v3/rom_singles/14-brk.nes",
        "instr_test-v3/rom_singles/15-special.nes",
        "instr_test-v3/official_only.nes",
        "instr_test-v3/all_instrs.nes",
    ]
)
def test_instr_test_v3(path_rom: str) -> None:
    """Run the instr_test-v3 test suite."""
    run_test_rom_ram(path_rom=path_rom)

@pytest.mark.parametrize(
    "path_rom", ["instr_timing/instr_timing.nes"]
)
def test_instr_timing(path_rom: str) -> None:
    """Run the instr_timing test suite."""
    run_test_rom_ram(path_rom=path_rom)

@pytest.mark.parametrize(
    "path_rom", ["other/nestest.nes"]
)
def test_nestest(path_rom: str) -> None:
    """Run the nestest test suite."""
    nes = FrameCountedNES(path_rom, timeout=50)
    output = parse_text_from_frame(nes.step(), CHARACTER_MAP_NESTEST)

    while "Select: Invalid ops!" not in output:
        output = parse_text_from_frame(nes.step(), CHARACTER_MAP_NESTEST)

    # Press start to run the tests.
    nes.controller = NES_INPUT_START

    while "~~ Run all tests" in output:
        output = parse_text_from_frame(nes.step(), CHARACTER_MAP_NESTEST)

    assert "OK Run all tests" in output, (
        "Some tests failed with the following errors:\n" +
        "\n".join(output.split("\n")[1:-3])
    )

    # Press select to switch to invalid opcode tests.
    nes.controller = NES_INPUT_SELECT

    while "Select: Normal ops" not in output:
        output = parse_text_from_frame(nes.step(), CHARACTER_MAP_NESTEST)

    # Press start to run the tests.
    nes.controller = NES_INPUT_START

    while "~~ Run all tests" in output:
        output = parse_text_from_frame(nes.step(), CHARACTER_MAP_NESTEST)

    assert "OK Run all tests" in output, (
        "Some tests failed with the following errors:\n" +
        "\n".join(output.split("\n")[1:-3])
    )

@pytest.mark.parametrize(
    "path_rom", [
        "nes_instr_test/rom_singles/01-implied.nes",
        "nes_instr_test/rom_singles/02-immediate.nes",
        "nes_instr_test/rom_singles/03-zero_page.nes",
        "nes_instr_test/rom_singles/04-zp_xy.nes",
        "nes_instr_test/rom_singles/05-absolute.nes",
        "nes_instr_test/rom_singles/06-abs_xy.nes",
        "nes_instr_test/rom_singles/07-ind_x.nes",
        "nes_instr_test/rom_singles/08-ind_y.nes",
        "nes_instr_test/rom_singles/09-branches.nes",
        "nes_instr_test/rom_singles/10-stack.nes",
        "nes_instr_test/rom_singles/11-special.nes",
    ]
)
def test_nes_instr_test(path_rom: str) -> None:
    """Run the nes_instr_test test suite."""
    run_test_rom_ram(path_rom=path_rom)
