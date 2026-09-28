#!/usr/bin/env python3
import argparse
import struct
import sys
from pathlib import Path


XIP_BASE = 0x10000000
REFERENCE_POINTER_OFFSET = 0xE10


def extract_pin(path):
    image = path.read_bytes()
    if len(image) != 0x200000:
        raise ValueError("Expected a full 2 MiB flash dump")

    address = struct.unpack_from("<I", image, REFERENCE_POINTER_OFFSET)[0]
    offset = address - XIP_BASE
    if not 0 <= offset < 0x100000 or offset + 5 > len(image):
        raise ValueError(f"PIN reference points outside firmware: 0x{address:08x}")

    reference = image[offset:offset + 5]
    if reference[0] != 0 or any(digit > 9 for digit in reference[1:]):
        raise ValueError(f"Invalid PIN reference at 0x{address:08x}")
    return "".join(str(digit) for digit in reference[1:])


def main():
    parser = argparse.ArgumentParser(description="Extract the PIN reference from a full RP2040 flash dump")
    parser.add_argument("dump", type=Path, help="raw 2 MiB flash image")
    args = parser.parse_args()
    try:
        pin = extract_pin(args.dump)
    except (OSError, ValueError) as error:
        print(f"PIN extraction failed: {error}", file=sys.stderr)
        return 1
    print(f"PIN: {pin}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
