// cynes - C/C++ NES emulator with Python bindings
// Copyright (C) 2021  Combey Theo <https://www.gnu.org/licenses/>

#ifndef __NES_WRAPPER_H__
#define __NES_WRAPPER_H__

#include <cstdint>

unsigned int c_createEmulator(const char*);
unsigned int c_getSaveStateSize(unsigned int);

uint8_t c_readMemory(unsigned int, uint16_t);

bool c_runEmulator(unsigned int, unsigned int, uint8_t, uint8_t*);

void c_writeMemory(unsigned int, uint16_t, uint8_t);
void c_saveState(unsigned int, uint8_t*);
void c_loadState(unsigned int, uint8_t*);
void c_destroyEmulator(unsigned int);

#endif