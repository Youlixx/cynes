# cynes - C/C++ NES emulator with Python bindings
# Copyright (C) 2021 - 2025  Combey Theo <https://www.gnu.org/licenses/>

"""Main cynes package.

This package contains two sub-module:
- `cynes.emulator` with the main `NES` class, which is a direct wrapper around the C/C++
  API. This class can be used to run an emulator in 'headless' mode, which means that
  nothing will be rendered to the screen. The content of the frame buffer can be
  accessed nonetheless.
- `cynes.windowed` with the `WindowedNES` class, derived from `NES`. This class is a
  simple wrapper around the base emulator providing a basic renderer and input handling
  using SDL2. The python wrapper `pysdl2` must be installed to use this class.

Below is the simplest way of running a ROM with a rendering window.
```
from cynes.windowed import WindowedNES
nes = WindowedNES("rom.nes")
while not nes.should_close:
    nes.step()
```
"""

from cynes.emulator import NES, __version__

NES_INPUT_RIGHT = 0x01
NES_INPUT_LEFT = 0x02
NES_INPUT_DOWN = 0x04
NES_INPUT_UP = 0x08
NES_INPUT_START = 0x10
NES_INPUT_SELECT = 0x20
NES_INPUT_B = 0x40
NES_INPUT_A = 0x80

__all__ = [
    "__version__",
    "NES",
    "NES_INPUT_RIGHT",
    "NES_INPUT_LEFT",
    "NES_INPUT_DOWN",
    "NES_INPUT_UP",
    "NES_INPUT_START",
    "NES_INPUT_SELECT",
    "NES_INPUT_B",
    "NES_INPUT_A"
]

