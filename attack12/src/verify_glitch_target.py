#!/usr/bin/env python3
"""Verify the glitch target inside the decrypted usb_token image.

The campaign aims at one instruction in the application's PIN check. This script
derives that instruction from the image bytes, so the report cites a check and
not a hand-copied address. A wrong address here would send the whole sweep to a
place that cannot produce a hit, and nothing on the bench would say so.

Usage:
    python3 verify_glitch_target.py [IMAGE]

IMAGE defaults to attack11/out/partition_b_decrypted.bin. Exit status is 0 when
every check passes and 1 on the first mismatch.
"""

import struct
import sys
from pathlib import Path

IMAGE_BASE = 0x20000000
DEFAULT_IMAGE = "attack11/out/partition_b_decrypted.bin"


def halfword(image: bytes, address: int) -> int:
    return struct.unpack_from("<H", image, address - IMAGE_BASE)[0]


def word(image: bytes, address: int) -> int:
    return struct.unpack_from("<I", image, address - IMAGE_BASE)[0]


def branch_offset(instruction: int) -> int:
    """Return the byte offset of a 16-bit b.n, which is imm11 doubled."""
    return (instruction & 0x7FF) * 2


def check_cbz_target(image: bytes) -> int:
    """The comparison result decides success at 0x20000bfc."""
    address = 0x20000BFC
    instruction = halfword(image, address)
    if instruction & 0xFF00 != 0xB100:
        raise SystemExit(f"0x{address:08x}: expected cbz, found 0x{instruction:04x}")
    offset = ((instruction >> 9 & 1) << 6) | ((instruction >> 3 & 0x1F) << 1)
    return address + 4 + offset


def check_fail_branch(image: bytes) -> int:
    """The branch into the failure path is the instruction a glitch must skip."""
    address = 0x20000C00
    instruction = halfword(image, address)
    if instruction & 0xF800 != 0xE000:
        raise SystemExit(f"0x{address:08x}: expected b.n, found 0x{instruction:04x}")
    return address + 4 + branch_offset(instruction)


def check_encoder_pins(image: bytes) -> list:
    """The encoder pins come from the three compares in the GPIO dispatcher."""
    pins = []
    for address, expected in ((0x20000758, 29), (0x2000075C, 27), (0x20000760, 28)):
        instruction = halfword(image, address)
        if instruction & 0xF800 != 0x2800:
            raise SystemExit(f"0x{address:08x}: expected cmp, found 0x{instruction:04x}")
        if (instruction & 0xFF) != expected:
            raise SystemExit(f"0x{address:08x}: expected cmp #{expected}")
        pins.append(expected)
    return pins


def main() -> int:
    image_path = Path(sys.argv[1] if len(sys.argv) > 1 else DEFAULT_IMAGE)
    image = image_path.read_bytes()
    print(f"image: {image_path} ({len(image)} bytes)")

    success = check_cbz_target(image)
    if success != 0x20000C02:
        raise SystemExit(f"cbz at 0x20000bfc targets 0x{success:08x}, expected 0x20000c02")
    print("0x20000bfc  cbz r0 -> 0x20000c02   equal hashes reach the success path")

    fail = check_fail_branch(image)
    if fail != 0x20000C34:
        raise SystemExit(f"b.n at 0x20000c00 targets 0x{fail:08x}, expected 0x20000c34")
    print("0x20000c00  b.n    -> 0x20000c34   skipping it falls through to 0x20000c02")

    pins = check_encoder_pins(image)
    print(f"encoder pins {pins[0]}/{pins[1]}/{pins[2]}, the button is {pins[2]}")

    unlock_flag = word(image, 0x20000D0C)
    print(f"unlock flag pointer 0x{unlock_flag:08x}, written at 0x20000c20")

    print("all checks passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
