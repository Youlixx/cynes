"""Module containing CPU accuracy related test."""

import pytest

from tests.utils.test_rom import (
    MatchCondition,
    Matcher,
    run_test_rom_ppu,
)
from tests.utils.text_parsing import CHARACTER_MAP_DEFAULT


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
