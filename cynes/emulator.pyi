# cynes - C/C++ NES emulator with Python bindings
# Copyright (C) 2021 - 2024  Combey Theo <https://www.gnu.org/licenses/>

import numpy as np
from numpy.typing import NDArray

class NES:
    """The base emulator class."""

    controller: int
    """Emulator controller state.

    This variable is a 16-bits register representing both P1 (lower 8-bits) and P2
    (higher 8-bits) controllers. Each bit from the 8-bit registers represent a button:
    - 0. D-pad right.
    - 1. D-pad left.
    - 2. D-pad down.
    - 3. D-pard up.
    - 4. Start.
    - 5. Select.
    - 6. B-button.
    - 7. A-button.

    Setting a bit to 1 will mark the button as pressed, and resetting a bit to 0 will
    mark the button as released. The controller state is persistent across frames,
    meaning that released button have to be explicitly removed from the bit mask
    """

    def __init__(self, rom: str) -> None:
        """Initialize the NES emulator.

        The emulator initialisation can fail if the ROM file cannot be found or if the
        Mapper used by the game is currently unsupported.

        Parameters
        ----------
        rom: str
            The path to the NES file containing the game data.
        """
        ...

    def __setitem__(self, address: int, value: int) -> None:
        """Write a value in the emulator memory at the specified address.

        Writing to certains addresses may desynchronise the components of the console,
        leading to undefined behavior. Only the RAM at addresses between 0x0000 - 0x1FFF
        and 0x6000 - 0x7FFF (Mapper RAM) can be accessed safely.

        Parameters
        ----------
        address: int
            The RAM address to write to.
        value: int
            The value to write in the RAM.
        """
        ...

    def __getitem__(self, address: int) -> int:
        """Read a value in the emulator memory at the specified address.

        Reading certains addresses may desynchronise the components of the console,
        leading to undefined behavior. Only the RAM at addresses between 0x0000 - 0x1FFF
        and 0x6000 - 0x7FFF (Mapper RAM) can be accessed safely.

        Parameters
        ----------
        address: int
            The RAM address to read from.

        Returns
        -------
        value: int
            The value read from the RAM.
        """
        ...

    def reset(self) -> None:
        """Send a reset signal to the emulator.

        Reseting the NES is different from re-creating a new emulator as the RAM content
        is not cleared.
        """
        ...

    def step(self, frames: int = 1) -> NDArray[np.uint8]:
        """Run the emulator for the specified amount of frame.

        Parameters
        ----------
        frames: int, default: 1
            Indicates the number of frames for which the emulator will be run.

        Returns
        -------
        frame_buffer: NDArray[np.uint8]
            The numpy array containing the frame buffer (shape 240x256x3).
        """
        ...

    def save(self) -> NDArray[np.uint8]:
        """Dump the current emulator state into a save state.

        The size of the dump is variable and depend on the mapper used by the running
        game. The save state basically acts as a checkpoint that can be restored at any
        time without corrupting the NES memory.

        Returns
        -------
        buffer: NDArray[np.uint8]
            The numpy array containing the dump.
        """
        ...

    def load(self, buffer: NDArray[np.uint8]) -> None:
        """Restore the emulator state from a save state.

        The save state basically acts as a checkpoint that can be restored at any time
        without corrupting the NES memory.

        Parameters
        ----------
        buffer: NDArray[np.uint8]
            The numpy array containing the dump.
        """
        ...

    @property
    def has_crashed(self) -> int:
        """Indicate whether the CPU crashed after hitting an invalid op-code.

        When the emulator has crashed, subsequent calls to `step` will not do anything.
        Resetting the emulator / loading a valid save-state will reset this flag.
        """
        ...
