#ifndef __CYNES_UTILS__
#define __CYNES_UTILS__

#include <cstdint>
#include <cstring>

namespace cynes {
enum class DumpOperation {
    SIZE, DUMP, LOAD
};

template<DumpOperation operation, class T>
void dump(uint8_t*& buffer, T& value) {
    switch (operation) {
        case DumpOperation::DUMP:
            memcpy(buffer, &value, sizeof(T));
            break;
        case DumpOperation::LOAD:
            memcpy(&value, buffer, sizeof(T));
            break;
    }

    buffer += sizeof(T);
}

template<DumpOperation operation, class T>
void dump(unsigned int& bufferSize, T& value) {
    if (operation == DumpOperation::SIZE) {
        bufferSize += sizeof(T);
    }
}

template<DumpOperation operation, class T>
void dump(uint8_t*& buffer, T* values, unsigned int size) {
    switch (operation) {
        case DumpOperation::DUMP:
            memcpy(buffer, values, sizeof(T) * size);
            break;
        case DumpOperation::LOAD:
            memcpy(values, buffer, sizeof(T) * size);
            break;
    }

    buffer += sizeof(T) * size;
}

template<DumpOperation operation, class T>
void dump(unsigned int& bufferSize, T* value, unsigned int size) {
    if (operation == DumpOperation::SIZE) {
        bufferSize += sizeof(T) * size;
    }
}
}

#endif
