"""Module containing function used to parse text from a framebuffer."""

import numpy as np
from numpy.typing import NDArray

from cynes.emulator import NES


def parse_zero_terminated_string(nes: NES, address: int) -> str:
    """Parse a zero terminated string from the emulator RAM.

    Args:
        nes (NES): Emulator instance.
        address (int): String starting address.

    Returns:
        parsed_string (str): Parsed string from RAM.
    """
    parsed_string = ""

    while address <= 0xFFFF and nes[address] != 0:
        parsed_string += chr(nes[address])
        address += 1

    return parsed_string.rstrip()


def parse_text_from_frame(
    frame: NDArray[np.uint8],
    character_map: dict[int, str],
    strip: bool = True
) -> str:
    """Parse text from a frame.

    Args:
        frame (NDArray[np.uint8]): Frame to parse text from.
        character_map (dict[int, str]): Font used by the ROM.
        strip (bool): If enabled, strip each parsed line and remove empty lines from the
            final string.

    Returns:
        text (str): Parsed text.
    """
    parsed_lines: list[str] = []

    for y in range(frame.shape[0] // 8):
        current_line = ""

        for x in range(frame.shape[1] // 8):
            character = frame[y * 8: y * 8 + 8, x * 8: x * 8 + 8, 0]
            character = int.from_bytes(
                np.packbits(np.reshape(character, -1)).tobytes(),
                byteorder="big"
            )

            if character in character_map:
                current_line += character_map[character]
            else:
                current_line += " "

        if strip:
            current_line = current_line.strip()
        parsed_lines.append(current_line)

    if strip:
        parsed_lines = [line for line in parsed_lines if len(line) > 0]

    return "\n".join(parsed_lines)


CHARACTER_MAP_DEFAULT = {
    0x1818181818001800: "!",
    0x6c6c6c0000000000: '"',
    0x006cfe6c6cfe6c00: "#",
    0x183e603c067c1800: "$",
    0x62660c1830664600: "%",
    0x1c361c386b663b00: "&",
    0x0c0c0c0000000000: "'",
    0x060c1818180c0600: "(",
    0x6030181818306000: ")",
    0x00663cff3c660000: "*",
    0x0018187e18180000: "+",
    0x0000000000303060: ",",
    0x0000007e00000000: "-",
    0x0000000000606000: ".",
    0x02060c1830604000: "/",
    0x384ccec6e6643800: "0",
    0x1838181818187e00: "1",
    0x7cc60e3c78e0fe00: "2",
    0x7e0c183c06c67c00: "3",
    0x1c3c6cccfe0c0c00: "4",
    0xfcc0fc0606c67c00: "5",
    0x3c60c0fcc6c67c00: "6",
    0xfec60c1830303000: "7",
    0x78c4e47886867c00: "8",
    0x7cc6c67e060c7800: "9",
    0x0018180000181800: ":",
    0x0018180000181860: ";",
    0x0018306030180000: "<",
    0x00007e00007e0000: "=",
    0x00180c060c180000: ">",
    0x7cc6061c30003000: "?",
    0x3c666e6a6e603e00: "@",
    0x386cc6c6fec6c600: "A",
    0xfcc6c6fcc6c6fc00: "B",
    0x3c66c0c0c0663c00: "C",
    0xf8ccc6c6c6ccf800: "D",
    0xfec0c0fcc0c0fe00: "E",
    0xfec0c0fcc0c0c000: "F",
    0x3e60c0cec6663e00: "G",
    0xc6c6c6fec6c6c600: "H",
    0x3c18181818183c00: "I",
    0x1e060606c6c67c00: "J",
    0xc6ccd8f0d8ccc600: "K",
    0x6060606060607e00: "L",
    0xc6eefefed6c6c600: "M",
    0xc6e6f6fedecec600: "N",
    0x7cc6c6c6c6c67c00: "O",
    0xfcc6c6fcc0c0c000: "P",
    0x7cc6c6c6dacc7600: "Q",
    0xfcc6c6fcd8ccc600: "R",
    0x7cc6c07c06c67c00: "S",
    0xfc30303030303000: "T",
    0xc6c6c6c6c6c67c00: "U",
    0xc6c6c6ee7c381000: "V",
    0xc6c6d6fefeeec600: "W",
    0xc6ee7c387ceec600: "X",
    0x6666663c18181800: "Y",
    0xfe0e1c3870e0fe00: "Z",
    0x1e18181818181e00: "[",
    0x406030180c060200: "\\",
    0x3c0c0c0c0c0c3c00: "]",
    0x10386cc600000000: "^",
    0x000000000000fe00: "_",
    0xc060300000000000: "`",
    0x0000780c7ccc7600: "a",
    0xc0c0f8ccccccf800: "b",
    0x000078ccc0cc7800: "c",
    0x0c0c7ccccccc7c00: "d",
    0x000078ccfcc07800: "e",
    0x1c30307830303000: "f",
    0x00007ccccc7c0c78: "g",
    0xc0c0f8cccccccc00: "h",
    0x0030003030303000: "i",
    0x0018001818181870: "j",
    0xc0c0ccd8f0d8cc00: "k",
    0x3030303030303000: "l",
    0x0000ecfed6d6d600: "m",
    0x0000d8eccccccc00: "n",
    0x00003c6666663c00: "o",
    0x0000f8ccccf8c0c0: "p",
    0x00007ccccc7c0c0c: "q",
    0x0000d8ecc0c0c000: "r",
    0x000078c0780cf800: "s",
    0x00307c3030301c00: "t",
    0x0000ccccccdc6c00: "u",
    0x0000cccccc783000: "v",
    0x0000d6d6d6fe6c00: "w",
    0x0000cc783078cc00: "x",
    0x0000cccccc7c0c78: "y",
    0x0000fc183060fc00: "z",
    0x0e18183018180e00: "{",
    0x1818181818181800: "|",
    0x7018180c18187000: "}",
    0x60f29e0c00000000: "~",
}

CHARACTER_MAP_RESTRAINED = {
    0x1818181818001800: "!",
    0x2828280000000000: '"',
    0x2828fe28fe282800: "#",
    0x107cd07c16fc1000: "$",
    0x0044081020440000: "%",
    0x3048502054483400: "&",
    0x1010000000000000: "'",
    0x1020202020201000: "(",
    0x1008080808081000: ")",
    0x1054383854100000: "*",
    0x0010107c10100000: "+",
    0x0000000018183000: ",",
    0x0000003c00000000: "-",
    0x0000000000000800: ".",
    0x384ccec6e6643800: "0",
    0x1838181818187e00: "1",
    0x7cc60e3c70c0fe00: "2",
    0x7e0c183c06c67c00: "3",
    0x1c3c6cccfe0c0c00: "4",
    0xfcc0fc0606c67c00: "5",
    0x3e60c0fcc6c67c00: "6",
    0xfec60c1830303000: "7",
    0x7cc6c67cc6c67c00: "8",
    0x7cc6c67e060c7800: "9",
    0x0018180018180000: ":",
    0x0018180018183000: ";",
    0x0010204020100000: "<",
    0x00003c003c000000: "=",
    0x0010080408100000: ">",
    0x0038440410001000: "?",
    0x38449aaa94403c00: "@",
    0x386cc6c6fec6c600: "A",
    0xfcc6c6fcc6c6fc00: "B",
    0x3c66c0c0c0663c00: "C",
    0xf8ccc6c6c6ccf800: "D",
    0xfec0c0fcc0c0fe00: "E",
    0xfec0c0f8c0c0c000: "F",
    0x3c66c0cec6663e00: "G",
    0xc6c6c6fec6c6c600: "H",
    0x7e18181818187e00: "I",
    0x0e060606c6c67c00: "J",
    0xc6ccd8f0f8dcce00: "K",
    0x6060606060607e00: "L",
    0xc6eefefed6c6c600: "M",
    0xc6e6f6fedecec600: "N",
    0x7cc6c6c6c6c67c00: "O",
    0xfcc6c6c6fcc0c000: "P",
    0x7cc6c6c6decc7a00: "Q",
    0xfcc6c6cef8dcce00: "R",
    0x78ccc07c06c67c00: "S",
    0x7e18181818181800: "T",
    0xc6c6c6c6c6c67c00: "U",
    0xc6c6c6ee7c381000: "V",
    0xc6c6d6fefeeec600: "W",
    0xc6ee7c387ceec600: "X",
    0x6666663c18181800: "Y",
    0xfe0e1c3870e0fe00: "Z",
}

CHARACTER_MAP_NESTEST = {
    0x1818181800181800: "!",
    0x3333660000000000: '"',
    0x6666ff66ff666600: "#",
    0x183e603c067c1800: "$",
    0x62660c1830664600: "%",
    0x3c663c3867663f00: "&",
    0x0c0c180000000000: "'",
    0x0c18303030180c00: "(",
    0x30180c0c0c183000: ")",
    0x00663cff3c660000: "*",
    0x0018187e18180000: "+",
    0x0000000000181830: ",",
    0x0000000000181800: ".",
    0x0003060c18306000: "/",
    0x3e63676b73633e00: "0",
    0x0c1c0c0c0c0c3f00: "1",
    0x3e63630e38637f00: "2",
    0x3e63630e63633e00: "3",
    0x060e1e267f060600: "4",
    0x7f63607e03633e00: "5",
    0x3e63607e63633e00: "6",
    0x7f63060c18183c00: "7",
    0x3e63633e63633e00: "8",
    0x3e63633f03633e00: "9",
    0x0000181800181800: ":",
    0x0000181800181830: ";",
    0x0e18306030180e00: "<",
    0x00007e007e000000: "=",
    0x70180c060c187000: ">",
    0x7e6303061c001818: "?",
    0x7cc6ceeee0e67c00: "@",
    0x1c36637f63636300: "A",
    0x6e73637e63637e00: "B",
    0x1e33606060331e00: "C",
    0x6c76636363667c00: "D",
    0x7f31303c30317f00: "E",
    0x7f31303c30307800: "F",
    0x1e33606763371d00: "G",
    0x6363637f63636300: "H",
    0x3c18181818183c00: "I",
    0x1f06060606663c00: "J",
    0x66666c786c676300: "K",
    0x7830606063637e00: "L",
    0x63777f6b63636300: "M",
    0x63737b6f67636300: "N",
    0x1c36636363361c00: "O",
    0x6e73637e60606000: "P",
    0x1c36636b67361d00: "Q",
    0x6e73637e6c676300: "R",
    0x3e63603e03633e00: "S",
    0x7e5a181818183c00: "T",
    0x7333636363763c00: "U",
    0x73336363663c1800: "V",
    0x7333636b7f776300: "W",
    0x6363361c36636300: "X",
    0x336363361c787000: "Y",
    0x7f63061c33637e00: "Z",
    0x3c30303030303c00: "[",
    0x406030180c060200: "\\",
    0x3c0c0c0c0c0c3c00: "]",
    0x000000000000ffff: "_",
    0x3030180000000000: "`",
    0x00003f6363673b00: "a",
    0x60606e7363633e00: "b",
    0x00003e6360633e00: "c",
    0x03033b6763633e00: "d",
    0x00003e617f603e00: "e",
    0x0e18183c18183c00: "f",
    0x00003e6063633d00: "g",
    0x60606e7363666700: "h",
    0x00001e0c0c0c1e00: "i",
    0x00003f060606663c: "j",
    0x6060666e7c676300: "k",
    0x1c0c0c0c0c0c1e00: "l",
    0x00006e7f6b626700: "m",
    0x00006e7363666700: "n",
    0x00003e6363633e00: "o",
    0x00003e63736e6060: "p",
    0x00003e63673b0303: "q",
    0x00006e73637e6300: "r",
    0x00003e711c473e00: "s",
    0x060c3f18181b0e00: "t",
    0x0000733363673b00: "u",
    0x0000733363663c00: "v",
    0x0000636b7f776300: "w",
    0x000063361c366300: "x",
    0x00003363633f033e: "y",
    0x00007f0e1c387f00: "z",
    0x0000006e3b000000: "~",
}
