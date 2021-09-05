# cynes - C/C++ NES emulator with Python bindings
# Copyright (C) 2021  Combey Theo <https://www.gnu.org/licenses/>

# distutils: language = c++

from cynes.wrapper cimport *

cimport numpy as np

def NES_CreateEmulator(path):
    return c_createEmulator(bytes(path, "ascii"))

def NES_GetSaveStateSize(index):
    return c_getSaveStateSize(index)

def NES_ReadMemory(index, address):
    return c_readMemory(index, address)

def NES_RunEmulator(index, frames, state, np.ndarray[np.uint8_t, ndim=3] buff):
    return c_runEmulator(index, frames, state, &buff[0, 0, 0])

def NES_WriteMemory(index, address, value):
    c_writeMemory(index, address, value)

def NES_SaveState(index, np.ndarray[np.uint8_t, ndim=1] buff):
    c_saveState(index, &buff[0])

def NES_LoadState(index, np.ndarray[np.uint8_t, ndim=1] buff):
    c_loadState(index, &buff[0])

def NES_DestroyEmulator(index):
    c_destroyEmulator(index)
