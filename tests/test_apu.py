"""Module containing APU accuracy related test."""

import pytest

from tests.utils.test_rom import (
    MatchCondition,
    Matcher,
    run_test_rom_ppu,
    run_test_rom_ram,
)
from tests.utils.text_parsing import CHARACTER_MAP_RESTRAINED


@pytest.mark.parametrize(
    "path_rom", [
        "apu_reset/4015_cleared.nes",
        "apu_reset/4017_timing.nes",
        "apu_reset/4017_written.nes",
        "apu_reset/irq_flag_cleared.nes",
        "apu_reset/len_ctrs_enabled.nes",
        "apu_reset/works_immediately.nes",
    ]
)
def test_apu_reset(path_rom: str) -> None:
    """Run the apu_reset test suite."""
    run_test_rom_ram(path_rom=path_rom)

@pytest.mark.parametrize(
    "path_rom", [
        "apu_test/rom_singles/1-len_ctr.nes",
        "apu_test/rom_singles/2-len_table.nes",
        "apu_test/rom_singles/3-irq_flag.nes",
        "apu_test/rom_singles/4-jitter.nes",
        "apu_test/rom_singles/5-len_timing.nes",
        "apu_test/rom_singles/6-irq_flag_timing.nes",
        "apu_test/rom_singles/7-dmc_basics.nes",
        "apu_test/rom_singles/8-dmc_rates.nes",
    ]
)
def test_apu_test(path_rom: str) -> None:
    """Run the apu_test test suite."""
    run_test_rom_ram(path_rom=path_rom)

@pytest.mark.parametrize(
    "path_rom", [
        "blargg_apu_2005.07.30/01.len_ctr.nes",
        "blargg_apu_2005.07.30/02.len_table.nes",
        "blargg_apu_2005.07.30/03.irq_flag.nes",
        "blargg_apu_2005.07.30/04.clock_jitter.nes",
        "blargg_apu_2005.07.30/05.len_timing_mode0.nes",
        "blargg_apu_2005.07.30/06.len_timing_mode1.nes",
        "blargg_apu_2005.07.30/07.irq_flag_timing.nes",
        "blargg_apu_2005.07.30/08.irq_timing.nes",
        "blargg_apu_2005.07.30/09.reset_timing.nes",
        "blargg_apu_2005.07.30/10.len_halt_timing.nes",
        "blargg_apu_2005.07.30/11.len_reload_timing.nes",
    ]
)
def test_blargg_apu_2005_07_30(path_rom: str) -> None:
    """Run the blargg_apu_2005.07.30 test suite."""
    run_test_rom_ppu(
        path_rom=path_rom,
        success_matcher=Matcher(
            string="$01",
            condition=MatchCondition.LAST_LINE_STRICT
        ),
        failure_matcher=Matcher(
            string=r"\$(?!01\b)\d{2}",
            condition=MatchCondition.REGEX
        ),
        character_map=CHARACTER_MAP_RESTRAINED
    )
