#ifndef __CYNES_SAVE_STATE__
#define __CYNES_SAVE_STATE__

#include <cstdint>
#include <cstring>

namespace cynes {
/// Save state utility class.
class SaveState {
public:
    /// Save state mode.
    enum class Mode { Save, Load, Size };

    /// Initialize the save state.
    /// @param mode Serialization mode.
    SaveState(Mode mode, uint8_t* buffer = nullptr);

    /// Default destructor.
    ~SaveState() = default;

    /// Stream a single value into / out of the save state.
    /// @tparam T Value type.
    /// @param value Value to stream.
    template<typename T>
    void stream(T& value) {
        if (_mode == Mode::Save) {
            std::memcpy(_buffer, &value, sizeof(T));
            _buffer += sizeof(T);
        } else if (_mode == Mode::Load) {
            std::memcpy(&value, _buffer, sizeof(T));
            _buffer += sizeof(T);
        }
        _size += sizeof(T);
    }

    /// Stream a multiple values into / out of the save state.
    /// @tparam T Value type.
    /// @param values Values to stream.
    /// @param count Number of values to stream.
    template<typename T>
    void stream(T* values, size_t count) {
        size_t bytes = sizeof(T) * count;
        if (_mode == Mode::Save) {
            std::memcpy(_buffer, values, bytes);
            _buffer += bytes;
        } else if (_mode == Mode::Load) {
            std::memcpy(values, _buffer, bytes);
            _buffer += bytes;
        }
        _size += bytes;
    }

    /// Get the save state size in bytes.
    /// @return Save state size in bytes.
    size_t size() const;

private:
    Mode _mode;
    uint8_t* _buffer;
    size_t _size;
};
}

#endif
