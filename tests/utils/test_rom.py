"""Module containing function to run test ROM in a automated way."""

import os
from enum import Enum, auto

import numpy as np
from numpy.typing import NDArray

from cynes.emulator import NES
from tests.utils.text_parsing import parse_text_from_frame, parse_zero_terminated_string

PATH_ROMS = os.path.abspath(os.environ.get("PATH_ROMS", "roms"))


class FrameCountedNES(NES):
    """A simple wrapper with a frame timeout assert."""

    def __init__(
        self,
        path_rom: str,
        timeout: int | None = None,
        warmup: int = 1
    ) -> None:
        """Initialize an emulator with a frame counter timeout.

        Args:
            path_rom (str): The path to the NES ROM file containing the game data.
            timeout (int, optional): If specified, automatically raises an assertion
                error if the number of `step` call exceed the timeout value.
            warmup (int): Number of step to run when initializing the emulator (note
                that they are ignored by the frame counter). Useful to let the ROM set
                the initial state of the RAM before running the test.
        """
        super().__init__(os.path.join(PATH_ROMS, path_rom))

        self._timeout = timeout
        self._frame_counter = 0

        super().step(frames=warmup)

    def step(self, frames: int = 1) -> NDArray[np.uint8]:
        """Run the emulator for the specified number of frames.

        This method advances the emulator's state by the specified number of frames. By
        default, it runs the emulator for a single frame.

        Args:
            frames (int): The number of frames to run the emulator for. Default is 1.

        Returns:
            framebuffer (NDArray[np.uint8]): A NumPy array containing the frame buffer
                in RGB format. The array has a shape of (240, 256, 3) and provides a
                read-only view of the framebuffer. If modifications are needed, a copy
                of the array should be made.
        """
        if self._timeout is not None:
            assert self._frame_counter <= self._timeout, "Emulation timed out."
        self._frame_counter += frames

        return super().step(frames)

    @property
    def frame_count(self) -> int:
        """Number of step executed."""
        return self._frame_counter


def run_test_rom_ram(path_rom: str, timeout: int = 10000) -> int:
    """Run a test ROM while parsing the RAM results.

    All text output is written starting at $6004, with a zero-byte terminator at the
    end. As more text is written, the terminator is moved forward, so an emulator can
    print the current text at any time.

    The test status is written to $6000. $80 means the test is running, $81 means the
    test needs the reset button pressed, but delayed by at least 100 msec from now.
    $00-$7F means the test has completed and given that result code.

    To allow an emulator to know when one of these tests is running and the data at
    $6000+ is valid, as opposed to some other NES program, $DE $B0 $G1 is written to
    $6001-$6003.

    Args:
        path_rom (str): Path to the test ROM.
        timeout (int, optional): Automatically raises an assertion error if the number
            of `step` call exceed the timeout value.
    """
    nes = FrameCountedNES(path_rom, timeout)

    while nes[0x6001] != 0xDE or nes[0x6002] != 0xB0 or nes[0x6003] != 0x61:
        nes.step()

    while nes[0x6000] == 0x80:
        nes.step()

        if nes[0x6000] == 0x81:
            nes.step(10)
            nes.reset()

            while nes[0x6000] != 0x80:
                nes.step()

    assert nes[0x6000] == 0, (
        f"Test failed with error code {nes[0x6000]}: "
        f"{parse_zero_terminated_string(nes, 0x6004)}"
    )


class MatchCondition(Enum):
    """Match condition enumeration."""

    STARTS_WITH = auto()
    ENDS_WITH = auto()
    CONTAINS = auto()
    FULL_TEXT_STRICT = auto()
    LINE_STRICT = auto()
    LINE_CONTAINS = auto()
    LAST_LINE_STRICT = auto()


class Matcher:
    """Simple string matcher class."""

    def __init__(self, string: str, condition: MatchCondition) -> None:
        """Initialize the matcher.

        Args:
            string (str): String to match.
            condition (MatchCondition): String match condition.
        """
        self._string = string
        self._condition = condition

    def __call__(self, output: str) -> bool:
        """Check whether the output contains the string.

        Args:
            output (str): Output string.

        Returns:
            match (bool): True if the target string was found according to the match
                condition, False otherwise.
        """
        output = output.strip()

        if len(output) == 0:
            return False

        if self._condition is MatchCondition.STARTS_WITH:
            return output.startswith(self._string)
        elif self._condition is MatchCondition.ENDS_WITH:
            return output.endswith(self._string)
        elif self._condition is MatchCondition.CONTAINS:
            return self._string in output
        elif self._condition is MatchCondition.FULL_TEXT_STRICT:
            return output == self._string
        elif self._condition is MatchCondition.LINE_STRICT:
            return any(line.strip() == self._string for line in output.splitlines())
        elif self._condition is MatchCondition.LINE_CONTAINS:
            return any(self._string in line.strip() for line in output.splitlines())
        elif self._condition is MatchCondition.LAST_LINE_STRICT:
            return output.splitlines()[-1].strip() == self._string
        else:
            raise ValueError(f"Unknown match condition {self._condition}")


def run_test_rom_ppu(
    path_rom: str,
    success_matcher: Matcher,
    failure_matcher: Matcher,
    character_map: dict[int, str],
    timeout: int = 10000
) -> None:
    """Run a test ROM while parsing the screen output.

    Args:
        path_rom (str): Path to the test ROM.
        success_matcher (Matcher): Expected string to be printed on the screen in case
            of success.
        failure_matcher (Matcher): Expected string to be printed on the screen in case
            of failure.
        path_rom (str): Path to the test ROM.
        character_map (dict[int, str]): Font used by the ROM.
        timeout (int, optional): Automatically raises an assertion error if the number
            of `step` call exceed the timeout value.
    """
    nes = FrameCountedNES(path_rom, timeout)
    output = parse_text_from_frame(nes.step(), character_map=character_map)

    while not success_matcher(output):
        output = parse_text_from_frame(nes.step(), character_map=character_map)
        assert not failure_matcher(output), f"Test failed with error: {output}"
