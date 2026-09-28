#!/usr/bin/env python3
"""Verify a forged full flash dump with the firmware-based decryptor

The maker uses attack3 recovery. This checker uses attack1 constants instead
It reads the archive through mtools and opens its layers with artifacts/unpack_layers.py
"""
import argparse
import os
import subprocess
import sys
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "attack1"))
from decrypt_storage import STORAGE_LEN, STORAGE_OFF, decrypt  # noqa: E402
sys.path.insert(0, str(ROOT / "artifacts"))
from unpack_layers import unpack  # noqa: E402
from bomb_html import inject_bomb  # noqa: E402


def verify_forgery(original, forged):
    source, changed = original.read_bytes(), forged.read_bytes()
    if len(source) != 0x200000 or len(changed) != len(source):
        raise ValueError("Both inputs must be complete 2 MiB flash dumps")
    end = STORAGE_OFF + STORAGE_LEN
    if source[:STORAGE_OFF] != changed[:STORAGE_OFF] or source[end:] != changed[end:]:
        raise ValueError("The forgery changed bytes outside the storage region")
    if source[STORAGE_OFF:end] == changed[STORAGE_OFF:end]:
        raise ValueError("The storage ciphertext did not change")

    with tempfile.TemporaryDirectory() as directory:
        scratch = Path(directory)
        finals = []
        depths = []
        for label, dump in (("original", original), ("forged", forged)):
            image = scratch / f"{label}.img"
            image.write_bytes(decrypt(dump))
            archive = scratch / f"{label}.zip"
            subprocess.run(
                ["mcopy", "-i", str(image), "::/your_prize.zip", str(archive)],
                check=True, capture_output=True, env={**os.environ, "MTOOLS_SKIP_CHECK": "1"},
            )
            final = scratch / f"{label}.html"
            depths.append(unpack(archive, final))
            finals.append(final.read_bytes())

    if depths[0] != depths[1] or finals[1] != inject_bomb(finals[0]):
        raise ValueError("The forged layers or final page do not match the chosen payload")
    print(f"PASS: {depths[1]} encrypted layers open; the forged page keeps the source HTML and adds the selected script")
    print("PASS: firmware-based decryption reads the forged ZIP; flash outside storage is unchanged")


def main():
    parser = argparse.ArgumentParser(description="Check a full flash forgery without the recovery code that made it")
    parser.add_argument("original", type=Path, help="original full flash dump")
    parser.add_argument("forged", type=Path, help="forged full flash dump")
    args = parser.parse_args()
    try:
        verify_forgery(args.original, args.forged)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"Verification failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
