# cynes - C/C++ NES emulator with Python bindings
# Copyright (C) 2021  Combey Theo <https://www.gnu.org/licenses/>

from cynes.emulator import NESHeadless

import numpy as np

import warnings

with warnings.catch_warnings():
    warnings.simplefilter("ignore")
    
    import sdl2


NES_INPUT_RIGHT = 0x01
NES_INPUT_LEFT = 0x02
NES_INPUT_DOWN = 0x04
NES_INPUT_UP = 0x08
NES_INPUT_START = 0x10
NES_INPUT_SELECT = 0x20
NES_INPUT_B = 0x40
NES_INPUT_A = 0x80


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
        Initializes the NES emulator.

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

        self.__closed = True

        super().__init__(rom)

        self.__closed = False

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
        self.controller |= NES_INPUT_A

    def __input_b(self) -> None:
        self.controller |= NES_INPUT_B

    def __input_select(self) -> None:
        self.controller |= NES_INPUT_SELECT

    def __input_start(self) -> None:
        self.controller |= NES_INPUT_START

    def __input_up(self) -> None:
        self.controller |= NES_INPUT_UP

    def __input_down(self) -> None:
        self.controller |= NES_INPUT_DOWN

    def __input_left(self) -> None:
        self.controller |= NES_INPUT_LEFT

    def __input_right(self) -> None:
        self.controller |= NES_INPUT_RIGHT

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

    def __del__(self):
        self.close()


__version__ = "0.0.2"
