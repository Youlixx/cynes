# cynes - C/C++ NES emulator with Python bindings
# Copyright (C) 2021  Combey Theo <https://www.gnu.org/licenses/>

from libcpp cimport bool

cimport numpy as np

ctypedef unsigned char u8
ctypedef unsigned short u16
ctypedef unsigned int u32

cdef extern from "../src/emulator.cpp":
    pass

cdef extern from "../src/emulator.h" namespace "cynes":
    cdef cppclass NES:
        NES(const char*) except +

        void reset()

        bool step(u8*, u16, u32)

        void writeCPU(u16, u8)
        u8 readCPU(u16)

        u32 size()

        void save(u8*)
        void load(u8*)


cdef class NESHeadless:
    cdef NES * __emulator

    cdef u8[:, :, :] __frame_buffer

    cdef u32 __state_size

    cdef public u16 controller
    
    cdef public bool _should_close

