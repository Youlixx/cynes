#ifndef __CYNES_UTILS__
#define __CYNES_UTILS__

#include <cstdint>
#include <cstring>

namespace cynes {
class SaveState {
public:
    enum class Mode { Save, Load, Size };

    /// Initialize the save state.
    /// @param mode Serialization mode.
    SaveState(Mode mode, uint8_t* buffer = nullptr)
        : _mode(mode), _buffer(buffer), _size(0) {}

    /// Default destructor.
    ~SaveState() = default;

    template<typename T>
    void stream(T& value) {
        if (_mode == Mode::Save) {
            memcpy(_buffer, &value, sizeof(T));
            _buffer += sizeof(T);
        } else if (_mode == Mode::Load) {
            memcpy(&value, _buffer, sizeof(T));
            _buffer += sizeof(T);
        }
        _size += sizeof(T);
    }

    template<typename T>
    void stream(T* values, size_t count) {
        size_t bytes = sizeof(T) * count;
        if (_mode == Mode::Save) {
            memcpy(_buffer, values, bytes);
            _buffer += bytes;
        } else if (_mode == Mode::Load) {
            memcpy(values, _buffer, bytes);
            _buffer += bytes;
        }
        _size += bytes;
    }

    size_t size() const { return _size; }

private:
    Mode _mode;
    uint8_t* _buffer;
    size_t _size;
};
}

#endif
