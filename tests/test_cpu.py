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
    "path_rom,frame_count", [
        ("blargg_nes_cpu_test5/cpu.nes", 978),
        ("blargg_nes_cpu_test5/official.nes", 640),
    ]
)
def test_blargg_nes_cpu_test5(path_rom: str, frame_count: int) -> None:
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
        expected_frame_count=frame_count,
        character_map=CHARACTER_MAP_DEFAULT
    )

@pytest.mark.parametrize(
    "path_rom,frame_count", [
        ("branch_timing_tests/1.Branch_Basics.nes", 13),
        ("branch_timing_tests/2.Backward_Branch.nes", 15),
        ("branch_timing_tests/3.Forward_Branch.nes", 15),
    ]
)
def test_branch_timing_tests(path_rom: str, frame_count: int) -> None:
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
        expected_frame_count=frame_count,
        character_map=CHARACTER_MAP_RESTRAINED
    )

@pytest.mark.parametrize(
    "path_rom,frame_count", [("cpu_dummy_reads/cpu_dummy_reads.nes", 43)]
)
def test_cpu_dummy_reads(path_rom: str, frame_count: int) -> None:
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
        expected_frame_count=frame_count,
        character_map=CHARACTER_MAP_DEFAULT
    )

@pytest.mark.parametrize(
    "path_rom,frame_count", [
        ("cpu_dummy_writes/cpu_dummy_writes_oam.nes", 313),
        ("cpu_dummy_writes/cpu_dummy_writes_ppumem.nes", 219),
    ]
)
def test_cpu_dummy_writes(path_rom: str, frame_count: int) -> None:
    """Run the cpu_dummy_writes test suite."""
    run_test_rom_ram(
        path_rom=path_rom,
        expected_frame_count=frame_count
    )

@pytest.mark.parametrize(
    "path_rom,frame_count", [
        ("cpu_exec_space/test_cpu_exec_space_apu.nes", 279),
        ("cpu_exec_space/test_cpu_exec_space_ppuio.nes", 40),
    ]
)
def test_cpu_exec_space(path_rom: str, frame_count: int) -> None:
    """Run the cpu_exec_space test suite."""
    run_test_rom_ram(
        path_rom=path_rom,
        expected_frame_count=frame_count
    )

@pytest.mark.parametrize(
    "path_rom,frame_count", [
        ("cpu_interrupts_v2/rom_singles/1-cli_latency.nes", 16),
        ("cpu_interrupts_v2/rom_singles/2-nmi_and_brk.nes", 112),
        ("cpu_interrupts_v2/rom_singles/3-nmi_and_irq.nes", 132),
        ("cpu_interrupts_v2/rom_singles/4-irq_and_dma.nes", 71),
        ("cpu_interrupts_v2/rom_singles/5-branch_delays_irq.nes", 379),
        ("cpu_interrupts_v2/cpu_interrupts.nes", 725),
    ]
)
def test_cpu_interrupts_v2(path_rom: str, frame_count: int) -> None:
    """Run the cpu_interrupts_v2 test suite."""
    run_test_rom_ram(
        path_rom=path_rom,
        expected_frame_count=frame_count
    )

@pytest.mark.parametrize(
    "path_rom,frame_count", [
        ("cpu_reset/ram_after_reset.nes", 156),
        ("cpu_reset/registers.nes", 160),
    ]
)
def test_cpu_reset(path_rom: str, frame_count: int) -> None:
    """Run the cpu_reset test suite."""
    run_test_rom_ram(
        path_rom=path_rom,
        expected_frame_count=frame_count
    )

@pytest.mark.parametrize(
    "path_rom,frame_count", [("cpu_timing_test6/cpu_timing_test.nes", 612)]
)
def test_cpu_timing_test6(path_rom: str, frame_count: int) -> None:
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
        expected_frame_count=frame_count,
        character_map=CHARACTER_MAP_RESTRAINED
    )

@pytest.mark.parametrize(
    "path_rom,frame_count", [("instr_misc/instr_misc.nes", 224)]
)
def test_instr_misc(path_rom: str, frame_count: int) -> None:
    """Run the instr_misc test suite."""
    run_test_rom_ram(
        path_rom=path_rom,
        expected_frame_count=frame_count
    )

@pytest.mark.parametrize(
    "path_rom,frame_count", [
        ("instr_test-v3/rom_singles/01-implied.nes", 98),
        ("instr_test-v3/rom_singles/02-immediate.nes", 87),
        ("instr_test-v3/rom_singles/03-zero_page.nes", 116),
        ("instr_test-v3/rom_singles/04-zp_xy.nes", 260),
        ("instr_test-v3/rom_singles/05-absolute.nes", 110),
        ("instr_test-v3/rom_singles/06-abs_xy.nes", 365),
        ("instr_test-v3/rom_singles/07-ind_x.nes", 146),
        ("instr_test-v3/rom_singles/08-ind_y.nes", 137),
        ("instr_test-v3/rom_singles/09-branches.nes", 41),
        ("instr_test-v3/rom_singles/10-stack.nes", 164),
        ("instr_test-v3/rom_singles/11-jmp_jsr.nes", 17),
        ("instr_test-v3/rom_singles/12-rts.nes", 14),
        ("instr_test-v3/rom_singles/13-rti.nes", 15),
        ("instr_test-v3/rom_singles/14-brk.nes", 27),
        ("instr_test-v3/rom_singles/15-special.nes", 13),
        ("instr_test-v3/official_only.nes", 1798),
        ("instr_test-v3/all_instrs.nes", 2336),
    ]
)
def test_instr_test_v3(path_rom: str, frame_count: int) -> None:
    """Run the instr_test-v3 test suite."""
    run_test_rom_ram(
        path_rom=path_rom,
        expected_frame_count=frame_count
    )

@pytest.mark.parametrize(
    "path_rom,frame_count", [("instr_timing/instr_timing.nes", 1299)]
)
def test_instr_timing(path_rom: str, frame_count: int) -> None:
    """Run the instr_timing test suite."""
    run_test_rom_ram(
        path_rom=path_rom,
        expected_frame_count=frame_count
    )

@pytest.mark.parametrize(
    "path_rom,frame_count", [("other/nestest.nes", 33)]
)
def test_nestest(path_rom: str, frame_count: int) -> None:
    """Run the nestest test suite."""
    nes = FrameCountedNES(path_rom, timeout=frame_count+1)
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

    assert nes.frame_count == frame_count, (
        f"Expected the test to succeed in exactly {frame_count} frames, it succeeded "
        f"within {nes.frame_count} frames instead."
    )
