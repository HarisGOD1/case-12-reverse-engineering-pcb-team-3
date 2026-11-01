#!/usr/bin/env python3
"""Recover the RP2350 OTP AES key from the attack11 dumper log and decrypt partition B.

The dumper prints one ECC line per page. Pages 29-30 hold the 4-way XOR key share and
page 31 holds the IV salt, so the log alone is enough. The AES/CTR scheme is owned by
attack10/src/decrypt_rp2350.py and is imported rather than copied.
"""

import argparse
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "attack10" / "src"))
import decrypt_rp2350

PAGE_RE = re.compile(r"^page (\d+): ECC=(.*?) all_ones")
IV_PUBLIC = bytes.fromhex("921e94597201e138c2747ca1e1bd1b0b")
APP_VECTORS = bytes.fromhex("00200820970100201b0100201d010020")
CT_OFFSET = 0x80024
CT_LENGTH = 0xC320


def parse_pages(text):
    pages = {}
    for line in text.splitlines():
        match = PAGE_RE.match(line.strip())
        if match:
            pages[int(match.group(1))] = [int(word, 16) for word in match.group(2).split()]
    if len(pages) != 64:
        raise SystemExit(f"expected 64 page lines in the dumper log, found {len(pages)}")
    return pages


def key_material(pages):
    # Key share words 16..31 sit in the FIB hole and are never indexed by decrypt().
    share = pages[29][:32] + pages[29][32:64] + pages[30][:32]
    return struct.pack("<96H", *share), struct.pack("<8H", *pages[31][:8])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dumper_output", help="attack11 dumper log (text)")
    parser.add_argument("flash_dump", help="full 4 MiB flash dump")
    parser.add_argument("-o", "--out", help="write the decrypted partition B here")
    args = parser.parse_args()

    pages = parse_pages(Path(args.dumper_output).read_text(errors="replace"))
    key4way, salt = key_material(pages)
    aes_key = decrypt_rp2350.reconstruct_key(key4way)

    flash = Path(args.flash_dump).read_bytes()
    ciphertext = flash[CT_OFFSET:CT_OFFSET + CT_LENGTH]
    plaintext = decrypt_rp2350.decrypt_with_raw_key(aes_key, salt, IV_PUBLIC, ciphertext)

    vectors_ok = plaintext[:16] == APP_VECTORS
    print(f"AES-256 key : {aes_key.hex()}")
    print(f"IV salt     : {salt.hex()}")
    print(f"IV public   : {IV_PUBLIC.hex()}")
    print(f"plaintext   : {plaintext[:16].hex()}")
    print(f"vector table: {'matches the UART leak' if vectors_ok else 'MISMATCH'}")

    if args.out:
        Path(args.out).write_bytes(plaintext)
        print(f"written     : {args.out} ({len(plaintext)} bytes)")

    return 0 if vectors_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
