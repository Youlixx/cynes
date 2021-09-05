# cynes - C/C++ NES emulator with Python bindings
# Copyright (C) 2021  Combey Theo <https://www.gnu.org/licenses/>

import numpy as np


def NES_CreateEmulator(path: str) -> int:
    """
    C API function that instanciate a new NES emulator and returns its 
    associated unique index. If the specified ROM can't be loaded, the returned
    index will be 0.

    Arguments
    ---------
    path: str
        The path to the NES ROM file.

    Return
    ------
    index: int
        The unique index of the created emulator.
    """

    ...

def NES_GetSaveStateSize(index: int) -> int:
    """
    C API function that returns the size of the save state array in bytes.

    Arguments
    ---------
    index: int
        The index of the emulator.

    Return
    ------
    size: int
        The size of the save state array.
    """

    ...

def NES_ReadMemory(index: int, address: int) -> int:
    """
    C API function that reads a value in the emulator memory at the specified
    address. Note that this read is "silent", meaning that it has no impact on 
    the emulation and does not tick the internal emulator components. Only the 
    RAM at addresses between 0x0000 - 0x1FFF and 0x6000 - 0x7FFF (Mapper RAM) 
    can be accessed.

    Arguments
    ---------
    index: int
        The index of the emulator.
    address: int
        The RAM address to read from.

    Return
    ------
    value: int
        The value read from the RAM.
    """
    
    ...

def NES_RunEmulator(index: int, frames: int, state: int, buffer: np.ndarray):
    """
    C API function that runs the emulator for the specified amount of frame. To
    save computational time, the current frame buffer content is instantly
    returned, and then the next frames are computed in a separated thread. It
    also returns the current state of the CPU. When the CPU hits a JAM 
    instruction (illegal opcode), it is frozen until the emulator is reset. 
    Most of the games will never hit a JAM instruction, but memory corruptions 
    can cause them.

    Arguments
    ---------
    index: int
        The index of the emulator.
    frames: int
        Indicates the number of frames for which the emulator is ran.
    state: int
        The controller state that will be held for the emulated frames.
    buffer: np.ndarray
        The buffer to fill with the frame data.
    """

    ...

def NES_WriteMemory(index: int, address: int, value: int):
    """
    C API function that writes a value in the emulator memory at the specified
    address. Note that this write is "silent", meaning that it has no impact on 
    the emulation and does not tick the internal emulator components. Only the 
    RAM at addresses between 0x0000 - 0x1FFF and 0x6000 - 0x7FFF (Mapper RAM) 
    can be accessed.

    Arguments
    ---------
    index: int
        The index of the emulator.
    address: int
        The RAM address to write to.
    value: int
        The value written in the RAM.
    """

    ...

def NES_SaveState(index: int, buffer: np.ndarray):
    """
    C API fuction that dumps the current emulator state into a save state. The
    buffer used to hold the data must at least as big as the size given by the
    NES_GetSaveStateSize function. The save state basically acts as a checkpoint
    that can be restored at any time without corrupting the NES memory. Note 
    that this operation can be quite constly as it necessites a lot of IO 
    operations, so it should be used cautiously.

    Arguments
    ---------
    index: int
        The index of the emulator.
    buffer: np.ndarray
        The buffer to fill with the emulator data.
    """

    ...

def NES_LoadState(index: int, buffer: np.ndarray):
    """
    C API function that restores the emulator state from a save state. The 
    buffer used to hold the data must at least as big as the size given by the
    NES_GetSaveStateSize function. The save state basically acts as a checkpoint
    that can be restored at any time without corrupting the NES memory. Note 
    that this operation can be quite constly as it necessites a lot of IO 
    operations, so it should be used cautiously.

    Arguments
    ---------
    index: int
        The index of the emulator.
    buffer: np.ndarray
        The buffer holding the emulator data.
    """

    ...

def NES_DestroyEmulator(index: int):
    """
    C API function that destroys a running emulator. This function should be
    called to ensure proper memory management.

    Arguments
    ---------
    index: int
        The index of the emulator.
    """

    ...
