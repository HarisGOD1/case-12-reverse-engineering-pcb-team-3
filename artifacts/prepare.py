#!/usr/bin/env python3
import argparse
import hashlib
import os
import shutil
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path


ARTIFACTS = Path(__file__).resolve().parent
DECRYPTOR = ARTIFACTS.parent / "attack1" / "decrypt_storage.py"


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def extract_zip(dump, scratch):
    image = scratch / "storage.img"
    candidate_zip = scratch / "your_prize.zip"
    subprocess.run([sys.executable, str(DECRYPTOR), str(dump), str(image)], check=True)
    subprocess.run(
        ["mcopy", "-i", str(image), "::/your_prize.zip", str(candidate_zip)],
        check=True,
        env={**os.environ, "MTOOLS_SKIP_CHECK": "1"},
    )

    with zipfile.ZipFile(candidate_zip) as archive:
        if sorted(archive.namelist()) != ["layers.zip", "readme.txt"] or archive.testzip():
            raise ValueError("Extracted ZIP failed the content or CRC check")
    return candidate_zip


def prepare(dump, reference_zip=None, verify=False):
    if dump.stat().st_size != 0x200000:
        raise ValueError("Expected a full 2 MiB flash dump")

    stored_dump = ARTIFACTS / "backup_full.bin"
    stored_zip = ARTIFACTS / "your_prize.zip"
    if verify:
        if sha256(dump) != sha256(stored_dump):
            raise ValueError("Flash dump differs from the stored artifact")
    elif any(path.exists() or path.is_symlink() for path in (stored_dump, stored_zip)):
        raise FileExistsError("Artifact already exists; remove or move it before regeneration")

    with tempfile.TemporaryDirectory(dir=ARTIFACTS) as directory:
        scratch = Path(directory)
        candidate_dump = scratch / "backup_full.bin"
        shutil.copyfile(dump, candidate_dump)
        candidate_zip = extract_zip(candidate_dump, scratch)
        if reference_zip is not None and sha256(candidate_zip) != sha256(reference_zip):
            raise ValueError("ZIP from flash differs from the supplied archive")
        if verify:
            if sha256(candidate_zip) != sha256(stored_zip):
                raise ValueError("ZIP from flash differs from the stored artifact")
            print("PASS: flash dump decrypts to the stored ZIP with a valid CRC")
            return

        os.link(candidate_dump, stored_dump)
        os.link(candidate_zip, stored_zip)
    print(f"Flash SHA-256: {sha256(stored_dump)}")
    print(f"ZIP SHA-256:   {sha256(stored_zip)}")


def main():
    parser = argparse.ArgumentParser(description="Store a flash dump and extract its prize ZIP")
    parser.add_argument("dump", type=Path, help="raw 2 MiB flash dump")
    parser.add_argument("--reference-zip", type=Path, help="optional archive for a byte-for-byte comparison")
    parser.add_argument("--verify", action="store_true", help="check the stored flash dump and ZIP without replacing them")
    args = parser.parse_args()
    try:
        prepare(args.dump, args.reference_zip, args.verify)
    except (OSError, ValueError, subprocess.CalledProcessError, zipfile.BadZipFile) as error:
        print(f"Artifact preparation failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
