
# cynes - C/C++ NES emulator with Python bindings
cynes is a lightweight multiplatform NES emulator providing a simple Python interface. The core of the emulation is based on the very complete documentation provided by the [Nesdev Wiki](https://wiki.nesdev.com/w/index.php?title=NES_reference_guide). The current implementation consists of
 - A cycle-accurate CPU emulation
 - A cycle-accurate PPU emulation
 - A cycle-accurate APU emulation (even though it does not produce any sound)
 - Few basic NES mappers (more to come)

The Python bindings allow to interact easily with one or several NES emulators at the same time, ideal for machine learning application.

## Installation
cynes can be installed using pip :
```
pip install cynes
```

It can also be built from source using [cython](https://github.com/cython/cython).
```
python setup.py build
```

## How to use
A cynes NES emulator can be created by instanticiating a new NES object. The following code is the minimal code to run a ROM file.
```python
from cynes import NES

# We initialize a new emulator by specifying the ROM file used
nes = NES("smb.nes")

# While the emulator should not be close, we can continue the emulation
while not nes.should_close():
    # The step method run the emulation for a single frame
    # It also returns the content of the frame buffer as a numpy array
    frame = nes.step()
```
Multiple emulators can be created at once by instantiating several NES objects.

### Windowed / Headless modes
A cynes NES emulator can either be run in windowed or headless mode.
```python
from cynes import NESHeadless, NES

# We can create a NES emulator without a rendering window
nes_headless = NESHeadless("smb.nes")

# And with the rendering window
nes = NES("smb.nes")
```
While the rendering overhead is quite small, running in headless mode can improve the performances when the window is not needed. The content of the frame buffer can still be accessed in the same way as previously, using the `step` method.

### Controller
The state of the controller can be directly modified using the following syntax :
```python
from cynes import *

# Simple input
nes.controller = NES_INPUT_RIGHT

# Multiple button presses at once
nes.controller = NES_INPUT_RIGHT | NES_INPUT_A

# Chaining multiple button presses at once
nes.controller = NES_INPUT_START 
nes.controller |= NES_INPUT_B 
nes.controller |= NES_INPUT_SELECT

# Undefined behavior
nes.controller = NES_INPUT_RIGHT | NES_INPUT_LEFT
nes.controller = NES_INPUT_DOWN | NES_INPUT_UP

# Run the emulator with the specified controller state for 5 frames
nes.step(frames=5)
```
Note that the state of the controller is maintain even after the `step` method is called. This means that it has to be reset to 0 to release the buttons. 

Two controllers can be used at the same time. The state of the second controller can be modified by updating the 8 most significant bits of the same variable.

```python
# P1 will press left and P2 will press the right button
nes.controller = NES_INPUT_LEFT | NES_INPUT_RIGHT << 8
```

### Key handlers
Key handlers are a simple way of associating custom actions to shortcuts. This feature is only present with the windowed mode. The key events (and their associated handlers) are fired when calling the `step` method.
```python
# Disable the default window controls
nes = NES("smb.nes", default_handlers=False)

# Custom key handlers can be defined using the register method
import sdl2

def kill():
    nes.close()

nes.register(sdl2.SDL_SCANCODE_O, kill)
```
By default, the emulator comes with key handlers that map window keys to the controller buttons. The mapping is the following :
 - the arrow keys for the D-pad
 - the keys X and Z for the A and B buttons respectively
 - the keys A and S for the SELECT and START buttons respectively

### Save states
The state of the emulator can be saved as a numpy array and later be restored.
```python
# The state of the emulator can be dump using the save method
save_state = nes.save()

# And restored using the load method
nes.load(save_state)
```
Memory modification should never be performed directly on a save state, as it is prone to memory corruption. Theses two methods can be quite slow, therefore, they should be called sparsely.

### Memory access
The memory of the emulator can be read from and written to using the following syntax :
```python
# The memory content can be accessed as if the emulator was an array
player_state = nes[0x000E]

# And can be written in a similar fashion
nes[0x075A] = 0x8
```
Note that only the CPU RAM `$0000 - $1FFFF` and the mapper RAM `$6000 - $7FFF` should be accessed. Trying to read / write a value to other addresses may desynchronize the components of the emulator, resulting in a undefined behavior.

### Closing
An emulator is automatically closed when the object is released by Python. In windowed mode, the `close` method can be use to close the window without having to wait for Python to release the object.
It can also be closed manualy using the `close` method.
```python
# In windowed mode, this can be use to close the window
nes.close()

# Deleting the emulator in windowed mode also closes the window
del nes

# The method should_close indicates whether or not the emulator function should be called
nes.close()
nes.should_close() # True
```
When the emulator is closed, but the object is not deleted yet, the `should_close` method will return True, indicating that calling any NES function will not work properly. This method can also return True in two other cases :
 - When the CPU of the emulator is frozen. When the CPU hits a JAM instruction (illegal opcode), it is frozen until the emulator is reset. This should never happen, but memory corruptions can cause them, so be careful when accessing the NES memory.
 - In windowed mode, when the window is closed or when the ESC key is pressed.

## License
This project is licensed under GPL-3.0

```plain
cynes - C/C++ NES emulator with Python bindings
Copyright (C) 2021  Combey Theo

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
```
