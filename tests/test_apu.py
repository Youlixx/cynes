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
