# cynes - C/C++ NES emulator with Python bindings
# Copyright (C) 2021  Combey Theo <https://www.gnu.org/licenses/>

import numpy as np


class NESHeadless:
    """
    The base emulator class.

    A NESHeadless implements the basic attributes and methods to interact with
    the C API to run a NES emulator. Using this class, there is no window
    rendering (see the NES class instead).

    Attributes
    ----------
    controller: int
        The controller state represented over a single byte.
    should_close: bool
        Set to True when the emulator should be closed.

    Methods
    -------
    reset()
        Sends a reset signal to the emulator.
    step(frames)
        Computes the next frame and returns the frame buffer.
    save()
        Returns a dump of the current console state.
    load(buffer)
        Loads back a save state.
    should_close()
        Returns whether or not the emulator should be closed.
    """

    def __init__(self, rom: str):
        """
        Initializes the NES emulator.

        Arguments
        ---------
        rom: str
            The path to the NES file containing the game data.

        Notes
        -----
        The emulator initialisation can fail if the ROM file cannot be found or 
        if the Mapper used by the game is currently not supported.
        """

        ...

    def __setitem__(self, address: int, value: int) -> None:
        """
        Writes a value in the emulator memory at the specified address. Note 
        that this write is "silent", meaning that it has no impact on the 
        emulation and does not tick the internal emulator components. Only the 
        RAM at addresses between 0x0000 - 0x1FFF and 0x6000 - 0x7FFF (Mapper 
        RAM) can be accessed.

        Arguments
        ---------
        address: int
            The RAM address to write to.
        value: int
            The value to write in the RAM.
        """

        ...

    def __getitem__(self, address: int) -> int:
        """
        Reads a value in the emulator memory at the specified address. Note that 
        this read is "silent", meaning that it has no impact on the emulation 
        and does not tick the internal emulator components. Only the RAM at 
        addresses between 0x0000 - 0x1FFF and 0x6000 - 0x7FFF (Mapper RAM) can 
        be accessed.

        Arguments
        ---------
        address: int
            The RAM address to read from.

        Return
        ------
        value: int
            The value read from the RAM.
        """

        ...
    
    def reset(self) -> None:
        """
        Sends a reset signal to the emulator. Note that reseting the NES is 
        different from re-created a new emulator as the RAM content is not 
        erased.

        Arguments
        ---------
        index: int
            The index of the emulator.
        """

        ...

    def step(self, frames: int = 1) -> np.ndarray:
        """
        Runs the emulator for the specified amount of frame. To save 
        computational time, the current frame buffer content is instantly 
        returned, and then the next frames are computed in a separated thread.

        Arguments
        ---------
        frames: int (optional)
            Indicates the number of frames for which the emulator is ran.

        Return
        ------
        frame_buffer: np.ndarray
            The numpy array containing the frame buffer (shape 240x256x3)
        """
        
        ...

    def save(self) -> np.ndarray:
        """
        Dumps the current emulator state into a save state. The size of the dump 
        is variable and depend on the mapper used by the loaded game. The save 
        state basically acts as a checkpoint that can be restored at any time 
        without corrupting the NES memory. Note that this operation can be quite 
        constly as it necessites a lot of IO operations, so it should be used 
        cautiously.

        Return
        ------
        buffer: np.ndarray
            The numpy array containing the dump.
        """

        ...
        
    def load(self, buffer: np.ndarray) -> None:
        """
        Restores the emulator state from a save state. The save state basically 
        acts as a checkpoint that can be restored at any time without corrupting 
        the NES memory. Note that this operation can be quite constly as it 
        necessites a lot of IO operations, so it should be used cautiously.

        Arguments
        ---------
        buffer: np.ndarray
            The numpy array containing the dump.
        """

        ...

    def should_close(self) -> bool:
        """
        Returns whether or not the emulator should be closed. When this function
        returns True, then any call to class method will do nothing.

        Return
        ------
        should_close: bool
            If set to True then the emulator should not be used anymore.
        """

        ...
