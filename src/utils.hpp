#ifndef __CYNES_UTILS__
#define __CYNES_UTILS__

#include <cstdint>
#include <cstring>

namespace cynes {
enum class DumpOperation {
    SIZE, DUMP, LOAD
};

template<DumpOperation operation, typename T>
constexpr void dump(uint8_t*& buffer, T& value) {
    if constexpr (operation == DumpOperation::DUMP) {
        memcpy(buffer, &value, sizeof(T));
    } else if constexpr (operation == DumpOperation::LOAD) {
        memcpy(&value, buffer, sizeof(T));
    }

    buffer += sizeof(T);
}

template<DumpOperation operation, typename T>
constexpr void dump(unsigned int& bufferSize, T&) {
    if constexpr (operation == DumpOperation::SIZE) {
        bufferSize += sizeof(T);
    }
}

template<DumpOperation operation, typename T>
constexpr void dump(uint8_t*& buffer, T* values, unsigned int size) {
    if constexpr (operation == DumpOperation::DUMP) {
        memcpy(buffer, values, sizeof(T) * size);
    } else if constexpr (operation == DumpOperation::LOAD) {
        memcpy(values, buffer, sizeof(T) * size);
    }

    buffer += sizeof(T) * size;
}

template<DumpOperation operation, typename T>
constexpr void dump(unsigned int& bufferSize, T*, unsigned int size) {
    if constexpr (operation == DumpOperation::SIZE) {
        bufferSize += sizeof(T) * size;
    }
}
}

#endif
