#include "save_state.hpp"

using namespace cynes;

SaveState::SaveState(Mode mode, uint8_t* buffer)
    : _mode{mode}, _buffer{buffer}, _size{0} {}

size_t SaveState::size() const {
    return _size;
}
