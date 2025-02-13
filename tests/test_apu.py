"""Module containing APU accuracy related test."""

import pytest

from tests.utils.test_rom import run_test_rom_ram


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
