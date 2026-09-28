#!/usr/bin/env python3
import argparse
import hashlib
import io
import os
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path


def only_member(data, expected):
    with zipfile.ZipFile(io.BytesIO(data)) as archive:
        names = archive.namelist()
        if names != [expected]:
            raise ValueError(f"Expected {expected} alone, got {names}")
        return archive.read(expected)


def unpack(source, output):
    if output.exists() or output.is_symlink():
        raise FileExistsError(f"Output already exists: {output}")

    with zipfile.ZipFile(source) as outer:
        if sorted(outer.namelist()) != ["layers.zip", "readme.txt"]:
            raise ValueError("Unexpected outer archive contents")
        current = only_member(outer.read("layers.zip"), "layer_1.zip")

    with tempfile.TemporaryDirectory(dir=output.parent) as directory:
        layer_file = Path(directory) / "layer.zip"
        index = 1
        while True:
            with zipfile.ZipFile(io.BytesIO(current)) as layer:
                names = layer.namelist()
                expected = f"layer_{index + 1}.zip"
                if names not in ([expected], ["prize.html"]):
                    raise ValueError(f"Unexpected member at layer {index}: {names}")
                final = names == ["prize.html"]

            layer_file.write_bytes(current)
            result = subprocess.run(
                ["7z", "x", "-so", f"-p{index}", str(layer_file)],
                capture_output=True,
            )
            if result.returncode != 0 or not result.stdout:
                raise ValueError(f"Failed to open layer {index}: {result.stderr.decode(errors='replace').strip()}")

            current = result.stdout
            if final:
                break
            index += 1

        completed = Path(directory) / "prize.html"
        completed.write_bytes(current)
        os.link(completed, output)
    print(f"Unpacked {index} layers; SHA-256: {hashlib.sha256(current).hexdigest()}")
    print(f"Saved: {output}")
    return index


def main():
    parser = argparse.ArgumentParser(description="Unpack numbered password-protected ZIP layers")
    parser.add_argument("archive", type=Path, help="your_prize.zip")
    parser.add_argument("output", type=Path, help="new prize.html file")
    args = parser.parse_args()
    try:
        unpack(args.archive, args.output)
    except (OSError, ValueError, zipfile.BadZipFile) as error:
        print(f"Extraction failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
