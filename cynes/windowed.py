# cynes - C/C++ NES emulator with Python bindings
# Copyright (C) 2021 - 2024  Combey Theo <https://www.gnu.org/licenses/>

"""Module containing a simple NES wrapper using SDL2 for the rendering."""

from typing import Any, Callable, Optional, Type

import numpy as np
import sdl2
from numpy.typing import NDArray

from cynes import (
    NES_INPUT_A,
    NES_INPUT_B,
    NES_INPUT_DOWN,
    NES_INPUT_LEFT,
    NES_INPUT_RIGHT,
    NES_INPUT_SELECT,
    NES_INPUT_START,
    NES_INPUT_UP,
)
from cynes.emulator import NES  # type: ignore


class SDLContext:
    """SDL helper context."""

    def __init__(self, window_name: str, scaling_factor: int = 3) -> None:
        """Initialize the SDL context.

        Parameters
        ----------
        window_name: str
            Name of the window.
        scaling_factor: int
            Scaling factor of the window size.
        """
        self._hidden = False

        self._window = sdl2.SDL_CreateWindow(
            bytes(window_name, "ascii"),
            sdl2.SDL_WINDOWPOS_UNDEFINED,
            sdl2.SDL_WINDOWPOS_UNDEFINED,
            scaling_factor * 256,
            scaling_factor * 240,
            sdl2.SDL_WINDOW_SHOWN
        )

        self._renderer = sdl2.SDL_CreateRenderer(
            self._window,
            -1,
            sdl2.SDL_RENDERER_ACCELERATED
        )

        self._texture = sdl2.SDL_CreateTexture(
            self._renderer,
            sdl2.SDL_PIXELFORMAT_RGB24,
            sdl2.SDL_TEXTUREACCESS_STREAMING,
            256,
            240
        )

        self.keyboard_state = sdl2.SDL_GetKeyboardState(None)

    def __del__(self) -> None:
        """Destroy SDL2 variables."""
        sdl2.SDL_DestroyRenderer(self._renderer)
        sdl2.SDL_DestroyTexture(self._texture)
        sdl2.SDL_DestroyWindow(self._window)

    def render_frame(self, frame_buffer: NDArray[np.uint8]) -> None:
        """Update the rendered frame.

        Parameters
        ----------
        frame_buffer: NDArray[np.uint8]
            The numpy array containing the frame buffer (shape 240x256x3).
        """
        sdl2.SDL_UpdateTexture(
            self._texture,
            None,
            frame_buffer.ctypes._as_parameter_,
            768
        )

        sdl2.SDL_RenderCopy(self._renderer, self._texture, None, None)
        sdl2.SDL_RenderPresent(self._renderer)

    def hide_window(self) -> None:
        """Hide the SDL2 window."""
        if not self._hidden:
            self._hidden = True
            sdl2.SDL_HideWindow(self._window)

    @property
    def has_focus(self) -> bool:
        """Indicate whether the emulator window has the focus."""
        return sdl2.SDL_GetWindowFlags(self._window) & sdl2.SDL_WINDOW_INPUT_FOCUS


class WindowedNES(NES):
    """The windowed emulator class."""

    def __init__(
        self,
        rom: str,
        scaling_factor: int = 3,
        default_handlers: bool = True
    ) -> None:
        """Initialize the NES emulator.

        The emulator initialisation can fail if the ROM file cannot be found or if the
        Mapper used by the game is currently unsupported.

        Parameters
        ----------
        rom: str
            The path to the NES file containing the game data.
        scaling_factor: int
            Scaling factor of the window size.
        default_handlers: bool
            If set to True, the default key handlers will be registered.
        """
        super().__init__(rom)

        self._should_close = False
        self._handlers = {sdl2.SDL_SCANCODE_ESCAPE: self.__input_escape}

        self._context = SDLContext(
            window_name=rom,
            scaling_factor=scaling_factor
        )

        if default_handlers:
            self.register_handler(sdl2.SDL_SCANCODE_X, self.__input_a)
            self.register_handler(sdl2.SDL_SCANCODE_Z, self.__input_b)
            self.register_handler(sdl2.SDL_SCANCODE_A, self.__input_select)
            self.register_handler(sdl2.SDL_SCANCODE_S, self.__input_start)
            self.register_handler(sdl2.SDL_SCANCODE_UP, self.__input_up)
            self.register_handler(sdl2.SDL_SCANCODE_DOWN, self.__input_down)
            self.register_handler(sdl2.SDL_SCANCODE_LEFT, self.__input_left)
            self.register_handler(sdl2.SDL_SCANCODE_RIGHT, self.__input_right)

    def __enter__(self) -> "WindowedNES":
        """Enter the runtime context related to the emulator.

        Returns
        -------
        emulator: WindowedNES
            Current NES emulator.
        """
        return self

    def __exit__(
        self,
        error: Optional[Type[BaseException]],
        value: Optional[BaseException],
        traceback: Optional[Any]
    ) -> bool:
        """Close the window when exiting the runtime context related to the emulator.

        Parameters
        ----------
        error: Type[BaseException], optional
            If an error occured, the type of the exception.
        value: BaseException, optional
            If an error occured, the exception itself.
        traceback: Any, optional
            If an error occured, the current traceback.

        Returns
        -------
        should_suppress_error: bool
            True if the exception should be suppressed, False otherwise.
        """
        self.close()

        return True

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
        self.close()

    def register_handler(self, key_code: int, handler: Callable[[], None]) -> None:
        """Register a new key handler.

        Parameters
        ----------
        key_code: int
            The code of the key that triggers the handler.
        handler: Callable[[], None]
            A function that takes not argmuent, called when the key is pressed.
        """
        self._handlers[key_code] = handler

    def step(self, frames: int = 1) -> Optional[NDArray[np.uint8]]:
        """Run the emulator for the specified amount of frame.

        Parameters
        ----------
        frames: int, default: 1
            Indicates the number of frames for which the emulator will be run.

        Returns
        -------
        frame_buffer: NDArray[np.uint8], optional
            The numpy array containing the frame buffer (shape 240x256x3).
        """
        if self.should_close:
            self._context.hide_window()
            return

        sdl2.SDL_PumpEvents()

        # TODO: necessary?
        previous_state = self.controller

        if self._context.has_focus:
            for handler in self._handlers:
                if self._context.keyboard_state[handler]:
                    self._handlers[handler]()

            event = sdl2.SDL_Event()

            while sdl2.SDL_PollEvent(event):
                if event.type == sdl2.SDL_QUIT:
                    self.close()
                    return

        frame_buffer = super().step(frames=frames)

        self._context.render_frame(frame_buffer)
        self.controller = previous_state

        return frame_buffer

    def close(self) -> None:
        """Close the window."""
        self._should_close = True
        self._context.hide_window()

    @property
    def should_close(self) -> bool:
        """Indicate if the emulator should close.

        This is set True when the emulator has either crash or if the user requested to
        close the window.
        """
        return self.has_crashed or self._should_close
