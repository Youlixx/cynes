"""Module containing PPU accuracy related test."""

import pytest

from tests.utils.test_rom import (
    MatchCondition,
    Matcher,
    run_test_rom_ppu,
)
from tests.utils.text_parsing import CHARACTER_MAP_RESTRAINED


@pytest.mark.parametrize(
    "path_rom", [
        "blargg_ppu_tests_2005.09.15b/palette_ram.nes",
        "blargg_ppu_tests_2005.09.15b/power_up_palette.nes",
        "blargg_ppu_tests_2005.09.15b/sprite_ram.nes",
        "blargg_ppu_tests_2005.09.15b/vbl_clear_time.nes",
        "blargg_ppu_tests_2005.09.15b/vram_access.nes",
    ]
)
def test_blargg_ppu_tests_2005_09_15b(path_rom: str) -> None:
    """Run the blargg_ppu_tests_2005.09.15b test suite."""
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
