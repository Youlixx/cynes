# cynes - C/C++ NES emulator with Python bindings
# Copyright (C) 2021  Combey Theo <https://www.gnu.org/licenses/>

# distutils: language = c++

from cynes.emulator cimport *

import numpy as np

from collections.abc import Iterable


cdef class NESHeadless:
    """
    The base emulator class.

    A NESHeadless implements the basic attributes and methods to interact with
    the C API to run a NES emulator. Using this class, there is no window
    rendering (see the NES class instead).

    Attributes
    ----------
    controller: int
        The controllers state represented over two bytes.

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
        if the Mapper used by the game is currently unsupported.
        """

        try:
            self.__emulator = new NES(bytes(rom, "ascii"))
        except RuntimeError as error:
            raise IOError(error) from None
        
        self.__frame_buffer = np.zeros(shape=(240, 256, 3), dtype=np.uint8)
        self.__state_size = self.__emulator.size()

        self._should_close = False

        self.controller = 0x00

    def __setitem__(self, u16 address, u8 value) -> None:
        """
        Writes a value in the emulator memory at the specified address. Note
        that writing to certains addresses may desynchronise the components of
        the console, leading to undefined behavior. Only the RAM at addresses 
        between 0x0000 - 0x1FFF and 0x6000 - 0x7FFF (Mapper RAM) can be accessed
        safely.

        Arguments
        ---------
        address: int
            The RAM address to write to.
        value: int
            The value to write in the RAM.
        """

        self.__emulator.writeCPU(address, value)

    def __getitem__(self, u16 address) -> u8:
        """
        Reads a value in the emulator memory at the specified address. Note that 
        reading to certains addresses may desynchronise the components of the 
        console, leading to undefined behavior. Only the RAM at addresses 
        between 0x0000 - 0x1FFF and 0x6000 - 0x7FFF (Mapper RAM) can be accessed
        safely.

        Arguments
        ---------
        address: int
            The RAM address to read from.

        Return
        ------
        value: int
            The value read from the RAM.
        """

        return self.__emulator.readCPU(address)

    def reset(self) -> None:
        """
        Sends a reset signal to the emulator. Note that reseting the NES is 
        different from re-creating a new emulator as the RAM content is not 
        erased.
        """

        should_close = False

        self.__emulator.reset()

    def step(self, u32 frames = 1) -> np.ndarray:
        """
        Runs the emulator for the specified amount of frame. 

        Arguments
        ---------
        frames: int (optional)
            Indicates the number of frames for which the emulator is ran.

        Return
        ------
        frame_buffer: np.ndarray
            The numpy array containing the frame buffer (shape 240x256x3)
        """

        self._should_close |= self.__emulator.step(
            &self.__frame_buffer[0, 0, 0], self.controller, frames
        )
        
        return np.asarray(self.__frame_buffer)

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

        cdef np.ndarray[np.uint8_t, ndim=1] buff = np.zeros(
            shape=(self.__state_size,), dtype=np.uint8
        )

        self.__emulator.save(&buff[0])

        return buff

    def load(self, np.ndarray[np.uint8_t, ndim=1] buff) -> None:
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

        self.__emulator.load(&buff[0])

    def should_close(self) -> bool:
        """
        Returns whether or not the emulator should be closed or reset. When this 
        function returns True, then any call to class method will do nothing.

        Return
        ------
        should_close: bool
            If set to True then the emulator should not be used anymore.
        """

        return self._should_close

    def __dealloc__(self):
        del self.__emulator

        self._should_close = True
