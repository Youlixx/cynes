# cynes - C/C++ NES emulator with Python bindings
# Copyright (C) 2021  Combey Theo <https://www.gnu.org/licenses/>

from cynes import wrapper

import numpy as np
import sdl2


class NESHeadless:
    """
    The base emulator class.

    A NESHeadless implements the basic attributes and methods to interact with
    the C API to run a NES emulator. Using this class, there is no window
    rendering (see the NES class instead).

    Attributes
    ----------
    controller: int
        The controller state represented over a single byte.
    should_close: bool
        Set to True when the emulator should be closed.

    Methods
    -------
    step(frames)
        Computes the next frame and returns the frame buffer.
    save()
        Returns a dump of the current console state.
    load(buffer)
        Loads back a save state.
    frozen()
        Returns whether or not the CPU is frozen.
    close()
        Closes the emulator.
    """

    INPUT_RIGHT = 0x01
    INPUT_LEFT = 0x02
    INPUT_DOWN = 0x04
    INPUT_UP = 0x08
    INPUT_START = 0x10
    INPUT_SELECT = 0x20
    INPUT_B = 0x40
    INPUT_A = 0x80

    def __init__(self, rom: str):
        """
        Instanciate the NES emulator.

        Arguments
        ---------
        rom: str
            The path to the NES file containing the game data.

        Notes
        -----
        The emulator initialisation can fail if the ROM file cannot be found or 
        if the Mapper used by the game is currently not supported.
        """

        self.__emulator = wrapper.NES_CreateEmulator(rom)

        if self.__emulator == 0:
            raise Exception(
                f"Unable de load the ROM {rom}, the mapper used might not be" \
                f"supported yet."
            )

        self.__frame_buffer = np.zeros(shape=(240, 256, 3), dtype=np.uint8)
        self.__state_size = wrapper.NES_GetSaveStateSize(self.__emulator)
        self._should_close = False

        self.controller = 0x00

    def __setitem__(self, address: int, value: int) -> None:
        """
        Writes a value in the emulator memory at the specified address. Note 
        that this write is "silent", meaning that it has no impact on the 
        emulation and does not tick the internal emulator components. Only the 
        RAM at addresses between 0x0000 - 0x1FFF and 0x6000 - 0x7FFF (Mapper 
        RAM) can be accessed.

        Arguments
        ---------
        address: int
            The RAM address to write to.
        value: int
            The value to write in the RAM.
        """

        if not isinstance(address, int):
            raise TypeError("Only integer indexes are supported.")

        wrapper.NES_WriteMemory(self.__emulator, address, value)

    def __getitem__(self, address: int) -> int:
        """
        Reads a value in the emulator memory at the specified address. Note that 
        this read is "silent", meaning that it has no impact on the emulation 
        and does not tick the internal emulator components. Only the RAM at 
        addresses between 0x0000 - 0x1FFF and 0x6000 - 0x7FFF (Mapper RAM) can 
        be accessed.

        Arguments
        ---------
        address: int
            The RAM address to read from.

        Return
        ------
        value: int
            The value read from the RAM.
        """

        if not isinstance(address, int):
            raise TypeError("Only integer indexes are supported.")

        return wrapper.NES_ReadMemory(self.__emulator, address)

    def step(self, frames: int = 1) -> np.ndarray:
        """
        Runs the emulator for the specified amount of frame. To save 
        computational time, the current frame buffer content is instantly 
        returned, and then the next frames are computed in a separated thread.

        Arguments
        ---------
        frames: int (optional)
            Indicates the number of frames for which the emulator is ran.

        Return
        ------
        frame_buffer: np.ndarray
            The numpy array containing the frame buffer (shape 240x256x3)
        """
        
        self._should_close |= wrapper.NES_RunEmulator(
            self.__emulator, frames, self.controller, self.__frame_buffer
        )

        return self.__frame_buffer

    def save(self) -> np.ndarray:
        """
        Dumps the current emulator state into a save state. The size of the dump 
        is variable and depend on the mapper used by the loaded game. The save 
        state basically acts as a checkpoint that can be restored at any time 
        without corrupting the NES memory. Note that this operation can be quite 
        constly as it necessites a lot of IO operations, so it should be used 
        cautiously.

        Return
        ------
        buffer: np.ndarray
            The numpy array containing the dump.
        """

        buffer = np.zeros(shape=(self.__state_size,), dtype=np.uint8)

        wrapper.NES_SaveState(self.__emulator, buffer)

        return buffer
        
    def load(self, buffer: np.ndarray) -> None:
        """
        Restores the emulator state from a save state. The save state basically 
        acts as a checkpoint that can be restored at any time without corrupting 
        the NES memory. Note that this operation can be quite constly as it 
        necessites a lot of IO operations, so it should be used cautiously.

        Arguments
        ---------
        buffer: np.ndarray
            The numpy array containing the dump.
        """

        wrapper.NES_LoadState(self.__emulator, buffer)

    def should_close(self) -> bool:
        """
        Returns whether or not the emulator should be closed. When this function
        returns True, then any call to class method will do nothing.

        Return
        ------
        should_close: bool
            If set to True then the emulator should not be used anymore.
        """

        return self._should_close

    def close(self) -> None:
        """
        This function should be called when the emulator is not used anymore. 
        It is automatically called when the object is deleted, so if you use the 
        emulator until the end of the program, you do not need to call it.
        """

        wrapper.NES_DestroyEmulator(self.__emulator)

        self._should_close = True
        self.__emulator = 0

    def __del__(self):
        self.close()


class NES(NESHeadless):
    """
    The windowed emulator class.

    A NES implements the basic attributes and methods to interact with the C API 
    to run a NES emulator in windowed mode.

    Attributes
    ----------
    controller: int
        The controller state represented over a single byte.
    should_close: bool
        Set to True when the window is closed (or when ESC is pressed).

    Methods
    -------
    register(key_code, handler)
        Registers a new key handler
    step(frames)
        Computes the next frame and returns the frame buffer.
    save()
        Returns a dump of the current console state.
    load(buffer)
        Loads back a save state.
    close()
        Closes the emulator.
    """

    def __init__(self, rom: str, default_handlers: bool = True, scale: int = 3):
        """
        Instanciate the NES emulator.

        Arguments
        ---------
        rom: str
            The path to the NES file containing the game data.
        default_handlers: bool, opt
            If set to True, the default key handlers will be registered.
        scale: int, opt
            Scaling factor of the window size.

        Notes
        -----
        The emulator initialisation can fail if the ROM file cannot be found or 
        if the Mapper used by the game is currently not supported.
        """

        super().__init__(rom)

        self.__window = sdl2.SDL_CreateWindow(
            bytes(rom, "ascii"), 
            sdl2.SDL_WINDOWPOS_UNDEFINED, 
            sdl2.SDL_WINDOWPOS_UNDEFINED, 
            scale * 256, 
            scale * 240, 
            sdl2.SDL_WINDOW_SHOWN
        )

        self.__keys = sdl2.SDL_GetKeyboardState(None)

        self.__renderer = sdl2.SDL_CreateRenderer(
            self.__window, -1, sdl2.SDL_RENDERER_ACCELERATED
        )

        self.__texture = sdl2.SDL_CreateTexture(
            self.__renderer, 
            sdl2.SDL_PIXELFORMAT_RGB24, 
            sdl2.SDL_TEXTUREACCESS_STREAMING, 
            256, 240
        )

        self.__handlers = { sdl2.SDL_SCANCODE_ESCAPE: self.__input_escape }

        if default_handlers:
            self.register(sdl2.SDL_SCANCODE_X, self.__input_a)
            self.register(sdl2.SDL_SCANCODE_Z, self.__input_b)
            self.register(sdl2.SDL_SCANCODE_A, self.__input_select)
            self.register(sdl2.SDL_SCANCODE_S, self.__input_start)
            self.register(sdl2.SDL_SCANCODE_UP, self.__input_up)
            self.register(sdl2.SDL_SCANCODE_DOWN, self.__input_down)
            self.register(sdl2.SDL_SCANCODE_LEFT, self.__input_left)
            self.register(sdl2.SDL_SCANCODE_RIGHT, self.__input_right)

        self.__closed = False

    def register(self, key_code: int, handler) -> None:
        """
        Registers a new key handler. The handler function will be called when 
        the specified key is pressed down.

        Arguments
        ---------
        key_code: int
            The code of the key that triggers the handler.
        handler: function
            A function that takes not argmuent, called when the key is pressed.
        """

        self.__handlers[key_code] = handler

    def __input_a(self) -> None:
        self.controller |= NES.INPUT_A

    def __input_b(self) -> None:
        self.controller |= NES.INPUT_B

    def __input_select(self) -> None:
        self.controller |= NES.INPUT_SELECT

    def __input_start(self) -> None:
        self.controller |= NES.INPUT_START

    def __input_up(self) -> None:
        self.controller |= NES.INPUT_UP

    def __input_down(self) -> None:
        self.controller |= NES.INPUT_DOWN

    def __input_left(self) -> None:
        self.controller |= NES.INPUT_LEFT

    def __input_right(self) -> None:
        self.controller |= NES.INPUT_RIGHT

    def __input_escape(self) -> None:
        self._should_close = True

    def step(self, frames: int = 1) -> np.ndarray:
        """
        Runs the emulator for the specified amount of frame. To save 
        computational time, the current frame buffer content is instantly 
        returned, and then the next frames are computed in a separated thread.

        Arguments
        ---------
        frames: int (optional)
            Indicates the number of frames for which the emulator is ran.

        Return
        ------
        frame_buffer: np.ndarray
            The numpy array containing the frame buffer (shape 240x256x3)
        """

        if self.__closed:
            return

        sdl2.SDL_PumpEvents()

        previous_state = self.controller

        if sdl2.SDL_GetWindowFlags(self.__window) & sdl2.SDL_WINDOW_INPUT_FOCUS:
            for handler in self.__handlers:
                if self.__keys[handler]:
                    self.__handlers[handler]()

            event = sdl2.SDL_Event()

            while sdl2.SDL_PollEvent(event):
                if event.type == sdl2.SDL_QUIT:
                    self._should_close = True
        
        frame_buffer = super().step(frames=frames)

        sdl2.SDL_UpdateTexture(
            self.__texture, None, frame_buffer.ctypes._as_parameter_, 768
        )
        
        sdl2.SDL_RenderCopy(self.__renderer, self.__texture, None, None)
        sdl2.SDL_RenderPresent(self.__renderer)

        self.controller = previous_state

        return frame_buffer

    def close(self) -> None:
        """
        This function closes the window and should be called when the emulator 
        is not used anymore. It is automatically called when the object is 
        deleted, so if you use the emulator until the end of the program, you 
        do not need to call it.
        """

        if self.__closed:
            return
        
        sdl2.SDL_DestroyRenderer(self.__renderer)
        sdl2.SDL_DestroyTexture(self.__texture)
        sdl2.SDL_DestroyWindow(self.__window)

        self.__closed = True

        super().close()

    def __del__(self):
        self.close()


__version__ = "0.0.1"
__all__ = ["NESHeadless", "NES"]