#include "ppu.hpp"
#include "cpu.hpp"
#include "nes.hpp"
#include "mapper.hpp"

#include <cstring>

cynes::PPU::PPU(NES& nes)
: _nes{nes}
, _pixelX{0x0000}
, _pixelY{0x0000}
, _renderingEnabled{false}
, _renderingEnabledDelayed{false}
, _preventVerticalBlank{false}
, _controlIncrementMode{false}
, _controlForegroundTable{false}
, _controlBackgroundTable{false}
, _controlForegroundLarge{false}
, _controlInterruptOnVertivalBlank{false}
, _maskGreyscaleMode{false}
, _maskRenderBackgroundLeft{false}
, _maskRenderForegroundLeft{false}
, _maskRenderBackground{false}
, _maskRenderForeground{false}
, _maskColorEmphasize{0x00}
, _statusSpriteOverflow{false}
, _statusSpriteZeroHit{false}
, _statusVerticalBlank{false}
, _clockDecays{}
, _registerDecay{0x00}
, _latchCycle{false}
, _latchAddress{false}
, _registerT{0x0000}
, _registerV{0x0000}
, _delayedRegisterV{0x0000}
, _scrollX{0x00}
, _delayDataRead{0x00}
, _delayDataWrite{0x00}
, _bufferData{0x00}
, _backgroundData{}
, _backgroundShifter{}
, _foregroundData{}
, _foregroundShifter{}
, _foregroundAttributes{}
, _foregroundPositions{}
, _foregroundDataPointer{0x00}
, _foregroundSpriteCount{0x00}
, _foregroundSpriteCountNext{0x00}
, _foregroundSpritePointer{0x00}
, _foregroundReadDelay{0x00}
, _foregroundSpriteAddress{0x0000}
, _foregroundSpriteZeroLine{false}
, _foregroundSpriteZeroShould{false}
, _foregroundSpriteZeroHit{false}
, _foregroundEvaluationStep{SpriteEvaluationStep::LOAD_SECONDARY_OAM}
{
    memset(_clockDecays, 0x00, 0x3);
    memset(_backgroundData, 0x00, 0x4);
    memset(_backgroundShifter, 0x0000, 0x8);
    memset(_foregroundData, 0x00, 0x20);
    memset(_foregroundShifter, 0x00, 0x10);
    memset(_foregroundAttributes, 0x00, 0x8);
    memset(_foregroundPositions, 0x00, 0x8);
}

void cynes::PPU::power() {
    _pixelY = 0xFF00;
    _pixelX = 0xFF00;

    _renderingEnabled = false;
    _renderingEnabledDelayed = false;
    _preventVerticalBlank = false;

    _controlIncrementMode = false;
    _controlForegroundTable = false;
    _controlBackgroundTable = false;
    _controlForegroundLarge = false;
    _controlInterruptOnVertivalBlank = false;

    _maskGreyscaleMode = false;
    _maskRenderBackgroundLeft = false;
    _maskRenderForegroundLeft = false;
    _maskRenderBackground = false;
    _maskRenderForeground = false;

    _maskColorEmphasize = 0x00;

    _statusSpriteOverflow = true;
    _statusSpriteZeroHit = false;
    _statusVerticalBlank = true;

    _foregroundSpritePointer = 0x00;

    _latchAddress = false;
    _latchCycle = false;

    _registerT = 0x0000;
    _registerV = 0x0000;
    _scrollX = 0x00;

    _delayDataWrite = 0x00;
    _delayDataRead = 0x00;
    _bufferData = 0x00;
}

void cynes::PPU::reset() {
    _pixelY = 0xFF00;
    _pixelX = 0xFF00;

    _renderingEnabled = false;
    _renderingEnabledDelayed = false;
    _preventVerticalBlank = false;

    _controlIncrementMode = false;
    _controlForegroundTable = false;
    _controlBackgroundTable = false;
    _controlForegroundLarge = false;
    _controlInterruptOnVertivalBlank = false;

    _maskGreyscaleMode = false;
    _maskRenderBackgroundLeft = false;
    _maskRenderForegroundLeft = false;
    _maskRenderBackground = false;
    _maskRenderForeground = false;

    _maskColorEmphasize = 0x00;

    _latchAddress = false;
    _latchCycle = false;

    _registerT = 0x0000;
    _registerV = 0x0000;
    _scrollX = 0x00;

    _delayDataWrite = 0x00;
    _delayDataRead = 0x00;
    _bufferData = 0x00;
}

void cynes::PPU::tick() {
    if (_pixelX > 339) {
        _pixelX = 0;

        if (++_pixelY > 261) {
            _pixelY = 0;
            _foregroundSpriteCount = 0;

            _latchCycle = !_latchCycle;

            for (int k = 0; k < 3; k++) {
                if (_clockDecays[k] > 0 && --_clockDecays[k] == 0) {
                    switch (k) {
                    case 0: _registerDecay &= 0x3F; break;
                    case 1: _registerDecay &= 0xDF; break;
                    case 2: _registerDecay &= 0xE0; break;
                    }
                }
            }
        }

        resetForegroundData();

        if (_pixelY == 261) {
            _statusSpriteOverflow = false;
            _statusSpriteZeroHit = false;

            memset(_foregroundShifter, 0x00, 0x10);
        }
    } else {
        _pixelX++;

        if (_pixelY < 240) {
            if (_pixelX < 257 || (_pixelX >= 321 && _pixelX < 337)) {
                loadBackgroundShifters();
            }

            if (_pixelX == 256) {
                incrementScrollY();
            } else if (_pixelX == 257) {
                resetScrollX();
            }

            if (_pixelX >= 2 && _pixelX < 257) {
                updateForegroundShifter();
            }

            if (_pixelX < 65) {
                clearForegroundData();
            } else if (_pixelX < 257) {
                fetchForegroundData();
            } else if (_pixelX < 321) {
                loadForegroundShifter();
            }

            if (_pixelX > 0 && _pixelX < 257 && _pixelY < 240) {
                memcpy(_frameBuffer + ((_pixelY << 8) + _pixelX - 1) * 3, PALETTE_COLORS[_maskColorEmphasize][_nes.readPPU(0x3F00 | blend())], 3);
            }
        } else if (_pixelY == 240 && _pixelX == 1) {
            _nes.readPPU(_registerV);
        } else if (_pixelY == 261) {
            if (_pixelX == 1) {
                _statusVerticalBlank = false;

                _nes.cpu.setNMI(false);
            }

            if (_pixelX < 257 || (_pixelX >= 321 && _pixelX < 337)) {
                loadBackgroundShifters();
            }

            if (_pixelX == 256) {
                incrementScrollY();
            } else if (_pixelX == 257) {
                resetScrollX();
            } else  if (_pixelX >= 280 && _pixelX < 305) {
                resetScrollY();
            }

            if (_pixelX > 1) {
                if (_pixelX < 257) {
                    updateForegroundShifter();
                } else if (_pixelX < 321) {
                    loadForegroundShifter();
                }
            }

            if (_renderingEnabled && (_pixelX == 337 || _pixelX == 339)) {
                _nes.readPPU(0x2000 | (_registerV & 0x0FFF));

                if (_pixelX == 339 && _latchCycle) {
                    _pixelX = 340;
                }
            }
        } else if (_pixelX == 1 && _pixelY == 241) {
            if (!_preventVerticalBlank) {
                _statusVerticalBlank = true;

                if (_controlInterruptOnVertivalBlank) {
                    _nes.cpu.setNMI(true);
                }
            }

            _preventVerticalBlank = false;
            _frameReady = true;
        }
    }

    if (_renderingEnabledDelayed != _renderingEnabled) {
        _renderingEnabledDelayed = _renderingEnabled;

        if (_pixelY < 240 || _pixelY == 261) {
            if (!_renderingEnabledDelayed) {
                _nes.readPPU(_registerV);

                if (_pixelX >= 65 && _pixelX <= 256) {
                    _foregroundSpritePointer++;
                }
            }
        }
    }

    if (_renderingEnabled != (_maskRenderBackground || _maskRenderForeground)) {
        _renderingEnabled = _maskRenderBackground || _maskRenderForeground;
    }


    if (_delayDataWrite > 0 && --_delayDataWrite == 0) {
        _registerV = _delayedRegisterV;
        _registerT = _registerV;

        if ((_pixelY >= 240 && _pixelY != 261) || !_renderingEnabled) {
            _nes.readPPU(_registerV);
        }
    }

    if (_delayDataRead > 0) {
        _delayDataRead--;
    }

    _nes.mapper->tick();
}

void cynes::PPU::write(uint8_t address, uint8_t value) {
    memset(_clockDecays, DECAY_PERIOD, 3);

    _registerDecay = value;

    switch (static_cast<Register>(address)) {
    case Register::PPU_CTRL: {
        _registerT &= 0xF3FF;
        _registerT |= (value & 0x03) << 10;

        _controlIncrementMode = value & 0x04;
        _controlForegroundTable = value & 0x08;
        _controlBackgroundTable = value & 0x10;
        _controlForegroundLarge = value & 0x20;
        _controlInterruptOnVertivalBlank = value & 0x80;

        if (!_controlInterruptOnVertivalBlank) {
            _nes.cpu.setNMI(false);
        } else if (_statusVerticalBlank) {
            _nes.cpu.setNMI(true);
        }

        break;
    }

    case Register::PPU_MASK: {
        _maskGreyscaleMode = value & 0x01;
        _maskRenderBackgroundLeft = value & 0x02;
        _maskRenderForegroundLeft = value & 0x04;
        _maskRenderBackground = value & 0x08;
        _maskRenderForeground = value & 0x10;
        _maskColorEmphasize = value >> 5;

        break;
    }

    case Register::OAM_ADDR: {
        _foregroundSpritePointer = value;

        break;
    }

    case Register::OAM_DATA: {
        if ((_pixelY >= 240 && _pixelY != 261) || !_renderingEnabled) {
            if ((_foregroundSpritePointer & 0x03) == 0x02) {
                value &= 0xE3;
            }

            _nes.writeOAM(_foregroundSpritePointer++, value);
        } else {
            _foregroundSpritePointer += 4;
        }

        break;
    }

    case Register::PPU_SCROLL: {
        if (!_latchAddress) {
            _scrollX = value & 0x07;

            _registerT &= 0xFFE0;
            _registerT |= value >> 3;
        } else {
            _registerT &= 0x8C1F;

            _registerT |= (value & 0xF8) << 2;
            _registerT |= (value & 0x07) << 12;
        }

        _latchAddress = !_latchAddress;

        break;
    }

    case Register::PPU_ADDR: {
        if (!_latchAddress) {
            _registerT &= 0x00FF;
            _registerT |= value << 8;
        } else {
            _registerT &= 0xFF00;
            _registerT |= value;

            _delayDataWrite = 3;
            _delayedRegisterV = _registerT;
        }

        _latchAddress = !_latchAddress;

        break;
    }

    case Register::PPU_DATA: {
        if ((_registerV & 0x3FFF) >= 0x3F00) {
            _nes.writePPU(_registerV, value);
        } else {
            if ((_pixelY >= 240 && _pixelY != 261) || !_renderingEnabled) {
                _nes.writePPU(_registerV, value);
            } else {
                _nes.writePPU(_registerV, _registerV & 0xFF);
            }
        }

        if ((_pixelY >= 240 && _pixelY != 261) || !_renderingEnabled) {
            _registerV += _controlIncrementMode ? 32 : 1;
            _registerV &= 0x7FFF;

            _nes.readPPU(_registerV);
        } else {
            incrementScrollX();
            incrementScrollY();
        }

        break;
    }

    default: break;
    }
}

uint8_t cynes::PPU::read(uint8_t address) {
    switch (static_cast<Register>(address)) {
    case Register::PPU_STATUS: {
        memset(_clockDecays, DECAY_PERIOD, 2);

        _latchAddress = false;

        _registerDecay &= 0x1F;
        _registerDecay |= _statusSpriteOverflow << 5;
        _registerDecay |= _statusSpriteZeroHit << 6;
        _registerDecay |= _statusVerticalBlank << 7;

        _statusVerticalBlank = false;
        _nes.cpu.setNMI(false);

        if (_pixelY == 241 && _pixelX == 0) {
            _preventVerticalBlank = true;
        }

        break;
    }

    case Register::OAM_DATA: {
        memset(_clockDecays, DECAY_PERIOD, 3);

        _registerDecay = _nes.readOAM(_foregroundSpritePointer);

        break;
    }

    case Register::PPU_DATA: {
        if (_delayDataRead == 0) {
            uint8_t value = _nes.readPPU(_registerV);

            if ((_registerV & 0x3FFF) >= 0x3F00) {
                _registerDecay &= 0xC0;
                _registerDecay |= value & 0x3F;

                _clockDecays[0] = _clockDecays[2] = DECAY_PERIOD;

                _bufferData = _nes.readPPU(_registerV - 0x1000);
            } else {
                _registerDecay = _bufferData;
                _bufferData = value;

                memset(_clockDecays, DECAY_PERIOD, 3);
            }

            if ((_pixelY >= 240 && _pixelY != 261) || !_renderingEnabled) {
                _registerV += _controlIncrementMode ? 32 : 1;
                _registerV &= 0x7FFF;

                _nes.readPPU(_registerV);
            } else {
                incrementScrollX();
                incrementScrollY();
            }

            _delayDataRead = 6;
        }

        break;
    }

    default: break;
    }

    return _registerDecay;
}

uint8_t* cynes::PPU::getFrameBuffer() {
    return _frameBuffer;
}

bool cynes::PPU::isFrameReady() {
    bool frameReady = _frameReady;

    _frameReady = false;

    return frameReady;
}

void cynes::PPU::incrementScrollX() {
    if (_maskRenderBackground || _maskRenderForeground) {
        if ((_registerV & 0x001F) == 0x1F) {
            _registerV &= 0xFFE0;
            _registerV ^= 0x0400;
        } else {
            _registerV++;
        }
    }
}

void cynes::PPU::incrementScrollY() {
    if (_maskRenderBackground || _maskRenderForeground) {
        if ((_registerV & 0x7000) != 0x7000) {
            _registerV += 0x1000;
        } else {
            _registerV &= 0x8FFF;

            uint8_t coarseY = (_registerV & 0x03E0) >> 5;

            if (coarseY == 0x1D) {
                coarseY = 0;
                _registerV ^= 0x0800;
            } else if (((_registerV >> 5) & 0x1F) == 0x1F) {
                coarseY = 0;
            } else {
                coarseY++;
            }

            _registerV &= 0xFC1F;
            _registerV |= coarseY << 5;
        }
    }
}

void cynes::PPU::resetScrollX() {
    if (_maskRenderBackground || _maskRenderForeground) {
        _registerV &= 0xFBE0;
        _registerV |= _registerT & 0x041F;
    }
}

void cynes::PPU::resetScrollY() {
    if (_maskRenderBackground || _maskRenderForeground) {
        _registerV &= 0x841F;
        _registerV |= _registerT & 0x7BE0;
    }
}


void cynes::PPU::loadBackgroundShifters() {
    updateBackgroundShifters();

    if (_renderingEnabled) {
        switch (_pixelX & 0x07) {
        case 0x1: {
            _backgroundShifter[0] = (_backgroundShifter[0] & 0xFF00) | _backgroundData[2];
            _backgroundShifter[1] = (_backgroundShifter[1] & 0xFF00) | _backgroundData[3];

            if (_backgroundData[1] & 0x01) {
                _backgroundShifter[2] = (_backgroundShifter[2] & 0xFF00) | 0xFF;
            } else {
                _backgroundShifter[2] = (_backgroundShifter[2] & 0xFF00);
            }

            if (_backgroundData[1] & 0x02) {
                _backgroundShifter[3] = (_backgroundShifter[3] & 0xFF00) | 0xFF;
            } else {
                _backgroundShifter[3] = (_backgroundShifter[3] & 0xFF00);
            }

            uint16_t address = 0x2000;
            address |= _registerV & 0x0FFF;

            _backgroundData[0] = _nes.readPPU(address);

            break;
        }

        case 0x3: {
            uint16_t address = 0x23C0;
            address |= _registerV & 0x0C00;
            address |= (_registerV >> 4) & 0x38;
            address |= (_registerV >> 2) & 0x07;

            _backgroundData[1] = _nes.readPPU(address);

            if (_registerV & 0x0040) {
                _backgroundData[1] >>= 4;
            }

            if (_registerV & 0x0002) {
                _backgroundData[1] >>= 2;
            }

            _backgroundData[1] &= 0x03;

            break;
        }

        case 0x5: {
            uint16_t address = _controlBackgroundTable << 12;
            address |= _backgroundData[0] << 4;
            address |= _registerV >> 12;

            _backgroundData[2] = _nes.readPPU(address);

            break;
        }

        case 0x7: {
            uint16_t address = _controlBackgroundTable << 12;
            address |= _backgroundData[0] << 4;
            address |= _registerV >> 12;
            address += 0x8;

            _backgroundData[3] = _nes.readPPU(address);

            break;

        }

        case 0x0: incrementScrollX(); break;
        }
    }
}

void cynes::PPU::updateBackgroundShifters() {
    if (_maskRenderBackground || _maskRenderForeground) {
        _backgroundShifter[0] <<= 1;
        _backgroundShifter[1] <<= 1;
        _backgroundShifter[2] <<= 1;
        _backgroundShifter[3] <<= 1;
    }
}

void cynes::PPU::resetForegroundData() {
    _foregroundSpriteCountNext = _foregroundSpriteCount;

    _foregroundDataPointer = 0;
    _foregroundSpriteCount = 0;
    _foregroundEvaluationStep = SpriteEvaluationStep::LOAD_SECONDARY_OAM;
    _foregroundSpriteZeroLine = _foregroundSpriteZeroShould;
    _foregroundSpriteZeroShould = false;
    _foregroundSpriteZeroHit = false;
}

void cynes::PPU::clearForegroundData() {
    if (_pixelX & 0x01) {
        _foregroundData[_foregroundDataPointer++] = 0xFF;

        _foregroundDataPointer &= 0x1F;
    }
}

void cynes::PPU::fetchForegroundData() {
    if (_pixelX % 2 == 0 && _renderingEnabled) {
        uint8_t spriteSize = _controlForegroundLarge ? 16 : 8;

        switch (_foregroundEvaluationStep) {
        case SpriteEvaluationStep::LOAD_SECONDARY_OAM: {
            uint8_t spriteData = _nes.readOAM(_foregroundSpritePointer);

            _foregroundData[_foregroundSpriteCount * 4 + (_foregroundSpritePointer & 0x03)] = spriteData;

            if (!(_foregroundSpritePointer & 0x3)) {
                int16_t offsetY = int16_t(_pixelY) - int16_t(spriteData);

                if (offsetY >= 0 && offsetY < spriteSize) {
                    if (!_foregroundSpritePointer++) {
                        _foregroundSpriteZeroShould = true;
                    }
                } else {
                    _foregroundSpritePointer += 4;

                    if (!_foregroundSpritePointer) {
                        _foregroundEvaluationStep = SpriteEvaluationStep::IDLE;
                    } else if (_foregroundSpriteCount == 8) {
                        _foregroundEvaluationStep = SpriteEvaluationStep::INCREMENT_POINTER;
                    }
                }
            } else if (!(++_foregroundSpritePointer & 0x03)) {
                _foregroundSpriteCount++;

                if (!_foregroundSpritePointer) {
                    _foregroundEvaluationStep = SpriteEvaluationStep::IDLE;
                } else if (_foregroundSpriteCount == 8) {
                    _foregroundEvaluationStep = SpriteEvaluationStep::INCREMENT_POINTER;
                }
            }

            break;
        }

        case SpriteEvaluationStep::INCREMENT_POINTER: {
            if (_foregroundReadDelay) {
                _foregroundReadDelay--;
            } else {
                int16_t offsetY = int16_t(_pixelY) - int16_t(_nes.readOAM(_foregroundSpritePointer));

                if (offsetY >= 0 && offsetY < spriteSize) {
                    _statusSpriteOverflow = true;

                    _foregroundSpritePointer++;
                    _foregroundReadDelay = 3;
                } else {
                    uint8_t low = (_foregroundSpritePointer + 1) & 0x03;

                    _foregroundSpritePointer += 0x04;
                    _foregroundSpritePointer &= 0xFC;

                    if (!_foregroundSpritePointer) {
                        _foregroundEvaluationStep = SpriteEvaluationStep::IDLE;
                    }

                    _foregroundSpritePointer |= low;
                }
            }

            break;
        }

        default: _foregroundSpritePointer = 0;
        }
    }
}

void cynes::PPU::loadForegroundShifter() {
    if (_renderingEnabled) {
        _foregroundSpritePointer = 0;

        if (_pixelX == 257) {
            _foregroundDataPointer = 0;
        }

        switch (_pixelX & 0x7) {
        case 0x1: {
            uint16_t address = 0x2000;
            address |= _registerV & 0x0FFF;

            _nes.readPPU(address);

            break;
        }

        case 0x3: {
            uint16_t address = 0x23C0;
            address |= _registerV & 0x0C00;
            address |= (_registerV >> 4) & 0x38;
            address |= (_registerV >> 2) & 0x07;

            _nes.readPPU(address);

            break;
        }

        case 0x5: {
            uint8_t spriteIndex = _foregroundData[_foregroundDataPointer * 4 + 1];
            uint8_t spriteAttribute = _foregroundData[_foregroundDataPointer * 4 + 2];

            uint8_t offset = 0x00;

            if (_foregroundDataPointer < _foregroundSpriteCount) {
                offset = _pixelY - _foregroundData[_foregroundDataPointer * 4];
            }

            _foregroundSpriteAddress = 0x0000;

            if (_controlForegroundLarge) {
                _foregroundSpriteAddress = (spriteIndex & 0x01) << 12;

                if (spriteAttribute & 0x80) {
                    if (offset < 8) {
                        _foregroundSpriteAddress |= ((spriteIndex & 0xFE) + 1) << 4;
                    } else {
                        _foregroundSpriteAddress |= ((spriteIndex & 0xFE)) << 4;
                    }
                } else {
                    if (offset < 8) {
                        _foregroundSpriteAddress |= ((spriteIndex & 0xFE)) << 4;
                    } else {
                        _foregroundSpriteAddress |= ((spriteIndex & 0xFE) + 1) << 4;
                    }
                }
            } else {
                _foregroundSpriteAddress = _controlForegroundTable << 12 | spriteIndex << 4;
            }

            if (spriteAttribute & 0x80) {
                _foregroundSpriteAddress |= (7 - offset) & 0x07;
            } else {
                _foregroundSpriteAddress |= offset & 0x07;
            }

            uint8_t spritePatternLSBPlane = _nes.readPPU(_foregroundSpriteAddress);


            if (spriteAttribute & 0x40) {
                spritePatternLSBPlane = (spritePatternLSBPlane & 0xF0) >> 4 | (spritePatternLSBPlane & 0x0F) << 4;
                spritePatternLSBPlane = (spritePatternLSBPlane & 0xCC) >> 2 | (spritePatternLSBPlane & 0x33) << 2;
                spritePatternLSBPlane = (spritePatternLSBPlane & 0xAA) >> 1 | (spritePatternLSBPlane & 0x55) << 1;
            }

            _foregroundShifter[_foregroundDataPointer * 2] = spritePatternLSBPlane;

            break;
        }

        case 0x7: {
            uint8_t spritePatternMSBPlane = _nes.readPPU(_foregroundSpriteAddress + 8);

            if (_foregroundData[_foregroundDataPointer * 4 + 2] & 0x40) {
                spritePatternMSBPlane = (spritePatternMSBPlane & 0xF0) >> 4 | (spritePatternMSBPlane & 0x0F) << 4;
                spritePatternMSBPlane = (spritePatternMSBPlane & 0xCC) >> 2 | (spritePatternMSBPlane & 0x33) << 2;
                spritePatternMSBPlane = (spritePatternMSBPlane & 0xAA) >> 1 | (spritePatternMSBPlane & 0x55) << 1;
            }

            _foregroundShifter[_foregroundDataPointer * 2 + 1] = spritePatternMSBPlane;
            _foregroundPositions[_foregroundDataPointer] = _foregroundData[_foregroundDataPointer * 4 + 3];
            _foregroundAttributes[_foregroundDataPointer] = _foregroundData[_foregroundDataPointer * 4 + 2];

            _foregroundDataPointer++;

            break;
        }
        }
    }
}

void cynes::PPU::updateForegroundShifter() {
    if (_maskRenderForeground) {
        for (uint8_t sprite = 0; sprite < _foregroundSpriteCountNext; sprite++) {
            if (_foregroundPositions[sprite] > 0) {
                _foregroundPositions[sprite] --;
            } else {
                _foregroundShifter[sprite * 2] <<= 1;
                _foregroundShifter[sprite * 2 + 1] <<= 1;
            }
        }
    }
}

uint8_t cynes::PPU::blend() {
    if (!_renderingEnabled && (_registerV & 0x3FFF) >= 0x3F00) {
        return _registerV & 0x1F;
    }

    uint8_t backgroundPixel = 0x00;
    uint8_t backgroundPalette = 0x00;

    if (_maskRenderBackground && (_pixelX > 8 || _maskRenderBackgroundLeft)) {
        uint16_t bitMask = 0x8000 >> _scrollX;

        backgroundPixel = ((_backgroundShifter[0] & bitMask) > 0) | (((_backgroundShifter[1] & bitMask) > 0) << 1);
        backgroundPalette = ((_backgroundShifter[2] & bitMask) > 0) | (((_backgroundShifter[3] & bitMask) > 0) << 1);
    }

    uint8_t foregroundPixel = 0x00;
    uint8_t foregroundPalette = 0x00;
    uint8_t foregroundPriority = 0x00;

    if (_maskRenderForeground && (_pixelX > 8 || _maskRenderForegroundLeft)) {
        _foregroundSpriteZeroHit = false;

        for (uint8_t sprite = 0; sprite < _foregroundSpriteCountNext; sprite++) {
            if (_foregroundPositions[sprite] == 0) {
                foregroundPixel = ((_foregroundShifter[sprite * 2] & 0x80) > 0) | (((_foregroundShifter[sprite * 2 + 1] & 0x80) > 0) << 1);
                foregroundPalette = (_foregroundAttributes[sprite] & 0x03) + 0x04;
                foregroundPriority = (_foregroundAttributes[sprite] & 0x20) == 0x00;

                if (foregroundPixel != 0) {
                    if (sprite == 0 && _pixelX != 256) {
                        _foregroundSpriteZeroHit = true;
                    }

                    break;
                }
            }
        }
    }

    uint8_t finalPixel = 0x00;
    uint8_t finalPalette = 0x00;

    if (backgroundPixel == 0 && foregroundPixel > 0) {
        finalPixel = foregroundPixel;
        finalPalette = foregroundPalette;
    } else if (backgroundPixel > 0 && foregroundPixel == 0) {
        finalPixel = backgroundPixel;
        finalPalette = backgroundPalette;
    } else if (backgroundPixel > 0 && foregroundPixel > 0) {
        if (foregroundPriority) {
            finalPixel = foregroundPixel;
            finalPalette = foregroundPalette;
        } else {
            finalPixel = backgroundPixel;
            finalPalette = backgroundPalette;
        }

        if (_foregroundSpriteZeroHit && _foregroundSpriteZeroLine && (_pixelX > 8 || _maskRenderBackgroundLeft || _maskRenderForegroundLeft)) {
            _statusSpriteZeroHit = true;
        }
    }

    finalPixel |= finalPalette << 2;

    if (_maskGreyscaleMode) {
        finalPixel &= 0x30;
    }

    return finalPixel;
}
