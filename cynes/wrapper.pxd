# cynes - C/C++ NES emulator with Python bindings
# Copyright (C) 2021  Combey Theo <https://www.gnu.org/licenses/>

from libcpp cimport bool

ctypedef unsigned char u8
ctypedef unsigned short u16
ctypedef unsigned int u32

cdef extern from "../src/emulator.cpp":
    pass

cdef extern from "../src/wrapper.cpp":
    pass

cdef extern from "../src/wrapper.h":
    cdef u32 c_createEmulator(const char*)
    cdef u32 c_getSaveStateSize(u32)
    
    cdef u8 c_readMemory(u32, u16)

    cdef bool c_runEmulator(u32, u32, u8, u8*)

    cdef void c_writeMemory(u32, u16, u8)
    cdef void c_saveState(u32, u8*)
    cdef void c_loadState(u32, u8*)
    cdef void c_destroyEmulator(u32)
