"""Module containing PPU accuracy related test."""

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

@pytest.mark.xfail
@pytest.mark.parametrize(
    "path_rom", ["nmi_sync/demo_ntsc.nes"]
)
def test_nmi_sync(path_rom: str) -> None:
    """Run the nmi_sync test suite."""
    # TODO: this test ROM actually fails, needs to be fixed.
    raise NotImplementedError("not implemented!")

@pytest.mark.parametrize(
    "path_rom", [
        "oam_read/oam_read.nes",
        "oam_stress/oam_stress.nes"
    ]
)
def test_oam_read_stress(path_rom: str) -> None:
    """Run the oam_read and oam_stress test suites."""
    run_test_rom_ram(path_rom=path_rom)

@pytest.mark.parametrize(
    "path_rom", ["ppu_open_bus/ppu_open_bus.nes"]
)
def test_ppu_open_bus(path_rom: str) -> None:
    """Run the ppu_open_bus test suite."""
    run_test_rom_ram(path_rom=path_rom)

@pytest.mark.parametrize(
    "path_rom", [
        "ppu_vbl_nmi/rom_singles/01-vbl_basics.nes",
        "ppu_vbl_nmi/rom_singles/02-vbl_set_time.nes",
        "ppu_vbl_nmi/rom_singles/03-vbl_clear_time.nes",
        "ppu_vbl_nmi/rom_singles/04-nmi_control.nes",
        "ppu_vbl_nmi/rom_singles/05-nmi_timing.nes",
        "ppu_vbl_nmi/rom_singles/06-suppression.nes",
        "ppu_vbl_nmi/rom_singles/07-nmi_on_timing.nes",
        "ppu_vbl_nmi/rom_singles/08-nmi_off_timing.nes",
        "ppu_vbl_nmi/rom_singles/09-even_odd_frames.nes",
        "ppu_vbl_nmi/rom_singles/10-even_odd_timing.nes",
        "ppu_vbl_nmi/ppu_vbl_nmi.nes",
    ]
)
def test_ppu_vbl_nmi(path_rom: str) -> None:
    """Run the ppu_vbl_nmi test suite."""
    run_test_rom_ram(path_rom=path_rom)

@pytest.mark.parametrize(
    "path_rom", [
        "sprdma_and_dmc_dma/sprdma_and_dmc_dma_512.nes",
        "sprdma_and_dmc_dma/sprdma_and_dmc_dma.nes",
    ]
)
def test_sprdma_and_dmc_dma(path_rom: str) -> None:
    """Run the sprdma_and_dmc_dma test suite."""
    run_test_rom_ram(path_rom=path_rom)

@pytest.mark.parametrize(
    "path_rom", [
        "sprite_hit_tests_2005.10.05/01.basics.nes",
        "sprite_hit_tests_2005.10.05/02.alignment.nes",
        "sprite_hit_tests_2005.10.05/03.corners.nes",
        "sprite_hit_tests_2005.10.05/04.flip.nes",
        "sprite_hit_tests_2005.10.05/05.left_clip.nes",
        "sprite_hit_tests_2005.10.05/06.right_edge.nes",
        "sprite_hit_tests_2005.10.05/07.screen_bottom.nes",
        "sprite_hit_tests_2005.10.05/08.double_height.nes",
        "sprite_hit_tests_2005.10.05/09.timing_basics.nes",
        "sprite_hit_tests_2005.10.05/10.timing_order.nes",
        "sprite_hit_tests_2005.10.05/11.edge_timing.nes",
    ]
)
def test_sprite_hit_tests_2005_10_05(path_rom: str) -> None:
    """Run the sprite_hit_tests_2005.10.05 test suite."""
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
    "path_rom", [
        "sprite_overflow_tests/1.Basics.nes",
        "sprite_overflow_tests/2.Details.nes",
        "sprite_overflow_tests/3.Timing.nes",
        "sprite_overflow_tests/4.Obscure.nes",
        "sprite_overflow_tests/5.Emulator.nes",
    ]
)
def test_sprite_overflow_tests(path_rom: str) -> None:
    """Run the sprite_overflow_tests test suite."""
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

@pytest.mark.xfail
@pytest.mark.parametrize(
    "path_rom", ["tvpassfail/tv.nes"]
)
def test_tvpassfail_tv(path_rom: str) -> None:
    """Run the tvpassfail test suite."""
    raise NotImplementedError("not implemented!")

@pytest.mark.parametrize(
    "path_rom", [
        "vbl_nmi_timing/1.frame_basics.nes",
        "vbl_nmi_timing/2.vbl_timing.nes",
        "vbl_nmi_timing/3.even_odd_frames.nes",
        "vbl_nmi_timing/4.vbl_clear_timing.nes",
        "vbl_nmi_timing/5.nmi_suppression.nes",
        "vbl_nmi_timing/6.nmi_disable.nes",
        "vbl_nmi_timing/7.nmi_timing.nes",
    ]
)
def test_vbl_nmi_timing(path_rom: str) -> None:
    """Run the vbl_nmi_timing/1.frame_basics test suite."""
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
