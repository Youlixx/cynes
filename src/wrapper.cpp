// cynes - C/C++ NES emulator with Python bindings
// Copyright (C) 2021  Combey Theo <https://www.gnu.org/licenses/>

#include "wrapper.h"
#include "emulator.h"

#include <unordered_map>

const unsigned int FRAME_BUFFER_SIZE = 256 * 240 * 3;

unsigned int currentIndex = 0;

std::unordered_map<unsigned int, nes::NES*> emulators;

unsigned int c_createEmulator(const char* rom) {
    nes::Mapper* mapper = nes::load(rom);

    if (mapper == nullptr) {
        return 0;
    }

    emulators[++currentIndex] = new nes::NES(mapper);

    return currentIndex;
}

unsigned int c_getSaveStateSize(unsigned int index) {
    if (index == 0) { return 0; }
    
    return emulators[index]->size();
}

uint8_t c_readMemory(unsigned int index, uint16_t address) {
    if (index == 0) { return 0x00; }

    emulators[index]->waitFrame();

    return emulators[index]->read(address);
}

bool c_runEmulator(unsigned int index, unsigned int frames, uint8_t state, uint8_t* frameBuffer) {
    if (index == 0) { return true; }

    emulators[index]->waitFrame();
    emulators[index]->setControllerState(state);

    memcpy(frameBuffer, emulators[index]->getFrameBuffer(), FRAME_BUFFER_SIZE);

    bool frozen = emulators[index]->isFrozen();

    emulators[index]->nextFrame(frames);

    return frozen;
}

void c_writeMemory(unsigned int index, uint16_t address, uint8_t value) {
    if (index == 0) { return; }

    emulators[index]->waitFrame();
    emulators[index]->write(address, value);
}

void c_saveState(unsigned int index, uint8_t* buffer) {
    if (index == 0) { return; }

    emulators[index]->waitFrame();
    emulators[index]->dump(buffer);
}

void c_loadState(unsigned int index, uint8_t* buffer) {
    if (index == 0) { return; }

    emulators[index]->waitFrame();
    emulators[index]->load(buffer);
}

void c_destroyEmulator(unsigned int index) {
    if (index == 0) { return; }

    delete emulators[index];

    emulators.erase(index);
}