"""Module containing CPU accuracy related test."""

import pytest

from tests.utils.test_rom import (
    MatchCondition,
    Matcher,
    run_test_rom_ppu,
    run_test_rom_ram,
)
from tests.utils.text_parsing import CHARACTER_MAP_DEFAULT, CHARACTER_MAP_RESTRAINED


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
