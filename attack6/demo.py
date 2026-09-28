#!/usr/bin/env python3
"""Run modified firmware images against the real PIN checker in rp2040js"""
import argparse
import subprocess
import sys
import tempfile
from pathlib import Path

from make_evil_firmware import patch_image


ROOT = Path(__file__).resolve().parents[1]
PROGRAM = ROOT / "reversing" / "program.bin"
STAND = ROOT / "attack4" / "emulator-stand"


def build_flash(flash, variant):
    program = PROGRAM.read_bytes()
    if flash[:len(program)] != program:
        raise ValueError("Flash code differs from reversing/program.bin")
    return patch_image(program, variant) + flash[len(program):]


def run_case(path, pin, expected_open):
    result = subprocess.run(
        ["node", "stand.mjs", str(path), pin], cwd=STAND, capture_output=True, text=True,
    )
    print(result.stdout, end="")
    if result.stderr:
        print(result.stderr, file=sys.stderr, end="")
    expected_status = 0 if expected_open else 3
    expected_flag = "= 1 -> UNLOCKED" if expected_open else "= 0 -> locked"
    if result.returncode != expected_status or expected_flag not in result.stdout:
        raise ValueError(f"PIN {pin}: expected {expected_flag} and exit {expected_status}, got {result.returncode}")


def demonstrate(dump):
    original = dump.read_bytes()
    if len(original) != 0x200000:
        raise ValueError("Expected a full 2 MiB flash dump")

    with tempfile.TemporaryDirectory(dir=Path(__file__).resolve().parent) as directory:
        scratch = Path(directory)
        for variant, cases in (
            ("original", (("3952", True), ("1234", False))),
            ("anypin", (("0000", True),)),
            ("pin1234", (("1234", True), ("3952", False))),
        ):
            image = original if variant == "original" else build_flash(original, variant)
            path = scratch / f"{variant}.bin"
            path.write_bytes(image)
            for pin, expected_open in cases:
                print(f"[{variant}] enter PIN {pin}: expected {'open' if expected_open else 'closed'}", flush=True)
                run_case(path, pin, expected_open)
    print("PASS: both firmware changes alter PIN behavior on the real emulated binary")


def main():
    parser = argparse.ArgumentParser(description="Demonstrate both firmware changes in the RP2040 emulator")
    parser.add_argument("dump", type=Path, help="full raw flash image")
    args = parser.parse_args()
    try:
        demonstrate(args.dump)
    except (OSError, ValueError) as error:
        print(f"Demonstration failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
