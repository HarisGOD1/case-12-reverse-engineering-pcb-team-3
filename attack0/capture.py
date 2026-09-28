#!/usr/bin/env python3
import argparse
import hashlib
import os
import subprocess
import sys
import tempfile
from pathlib import Path


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def capture(output):
    if output.exists() or output.is_symlink():
        raise FileExistsError(f"Output file already exists: {output}")

    with tempfile.TemporaryDirectory(dir=output.parent) as directory:
        first = Path(directory) / "first.bin"
        second = Path(directory) / "second.bin"
        subprocess.run(["picotool", "save", "-a", str(first)], check=True)
        subprocess.run(["picotool", "save", "-a", str(second)], check=True)

        first_hash = sha256(first)
        second_hash = sha256(second)
        if first_hash != second_hash or first.stat().st_size != second.stat().st_size:
            raise ValueError(f"Flash reads differ: {first_hash} != {second_hash}")

        os.link(first, output)
        print(f"SHA-256: {first_hash}\nSaved: {output}")


def main():
    parser = argparse.ArgumentParser(description="Read RP2040 flash twice and save it only if SHA-256 matches")
    parser.add_argument("output", type=Path, help="new binary output file")
    args = parser.parse_args()
    try:
        capture(args.output)
    except (OSError, subprocess.CalledProcessError, ValueError) as error:
        print(f"Capture failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
