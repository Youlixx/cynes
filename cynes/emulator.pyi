# cynes - C/C++ NES emulator with Python bindings
# Copyright (C) 2021 - 2025  Combey Theo <https://www.gnu.org/licenses/>

import numpy as np
from numpy.typing import NDArray

__version__ = ...


class NES:
    """The base emulator class."""

    controller: int
    """Emulator controller state.

    This variable is a 16-bit register representing the states of both P1 (lower 8 bits)
    and P2 (higher 8 bits) controllers. Each bit in the 8-bit registers corresponds to a
    specific button on the controller:
    - Bit 0: D-pad right
    - Bit 1: D-pad left
    - Bit 2: D-pad down
    - Bit 3: D-pad up
    - Bit 4: Start
    - Bit 5: Select
    - Bit 6: B button
    - Bit 7: A button

    Setting a bit to 1 indicates that the corresponding button is pressed, while
    resetting a bit to 0 indicates that the button is released. The controller state is
    persistent across frames, meaning that buttons must be explicitly marked as released
    by resetting the corresponding bit in the register.
    """

    def __init__(self, path_rom: str) -> None:
        """Initialize the NES emulator.

        This function sets up the NES emulator by loading the specified ROM file. The
        initialization process can fail for several reasons, including:
        - The ROM file cannot be found at the specified path.
        - The given file is not a valid ROM file.
        - The mapper used by the ROM is currently unsupported by the emulator.

        Args:
            path_rom (str): The path to the NES ROM file containing the game data.
        """
        ...

    def __setitem__(self, address: int, value: int) -> None:
        """Write a value to the emulator's memory at the specified address.

        Certain memory addresses are reserved for specific hardware components, and
        writing to these addresses may lead to undefined behavior or de-synchronization
        of console components.

        Safe memory regions for writing include:
        - RAM: Addresses between $0000 and $1FFF
        - Mapper RAM: Addresses between $6000 and $7FFF

        Args:
            address (int): The memory address where the value will be written.
            value (int): The value to be written at the specified memory address.

        Raises:
            TypeError: Error raised if the given address is out of bound (must be
                between $0000 and $FFFF), or when the given value is not a 8-bit
                positive integer.
        """
        ...

    def __getitem__(self, address: int) -> int:
        """Read a value from the emulator's memory at the specified address.

        Certain memory addresses are reserved for specific hardware components, and
        reading from these addresses may lead to undefined behavior or
        de-synchronization of console components.

        Safe memory regions for reading include:
        - RAM: Addresses between $0000 and $1FFF
        - Mapper RAM: Addresses between $6000 and $7FFF

        Args:
            address (int): The RAM address to read from.

        Returns:
            value (int): The value read from the RAM.

        Raises:
            TypeError: Error raised if the given address is out of bound (must be
                between $0000 and $FFFF).
        """
        ...

    def reset(self) -> None:
        """Send a reset signal to the emulator.

        This method simulates a reset of the NES console. Unlike creating a new emulator
        instance, resetting the emulator does not clear the RAM content. This means that
        any data stored in RAM will persist across the reset.
        """
        ...

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
        ...

    def save(self) -> NDArray[np.uint8]:
        """Dump the current emulator state into a save state.

        This method creates a snapshot of the emulator's current state, which can be
        used as a checkpoint. The size of the save state may vary depending on the
        mapper used by the currently running game. This save state can be restored at
        any time without corrupting the NES memory by using the `load` method.

        Returns:
            buffer (NDArray[np.uint8]): A NumPy array containing the dump of the
                emulator's state.
        """
        ...

    def load(self, buffer: NDArray[np.uint8]) -> None:
        """Restore the emulator state from a save state.

        This method restores a save state generated using the `save` method. When
        restoring a save state, you must ensure that the emulator was instantiated using
        the exact same ROM as the one used to generate the state.

        Args:
            buffer (NDArray[np.uint8]): A NumPy array containing the dump of the
                emulator's state to be restored.
        """
        ...

    @property
    def has_crashed(self) -> int:
        """Indicate whether the CPU has crashed due to encountering an invalid op-code.

        This method returns a flag that signals if the emulator has crashed. When the
        emulator is in a crashed state, subsequent calls to the `step` method will have
        no effect. To resume normal operation, the emulator must be reset or a valid
        save state must be loaded, which will clear this crash flag.
        """
        ...
