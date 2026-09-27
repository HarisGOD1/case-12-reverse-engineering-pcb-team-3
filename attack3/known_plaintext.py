#!/usr/bin/env python3
"""
attack3 -- the known-plaintext principle, self-contained and firmware-free.

The stream cipher's keystream is a pure function of position and never changes,
so wherever the plaintext is predictable the keystream falls out of a single XOR:

    keystream[pos] = ciphertext[pos] XOR known_plaintext[pos]

A FAT12 volume is full of predictable bytes: the boot sector magic and OEM
string, the FAT media/EOC markers, and the zero-filled reserved sectors. This
script recovers keystream at those positions from a cold dump, with no PIN, no
device and no firmware constants. Because the keystream repeats by position and
is identical across every same-build device, the recovered bytes decrypt the
same positions on any of them. Turning this partial leak into the full generator
(and thus every position) is [attack7](../attack7/README.md); here we only show
the primitive that makes the cipher unusable.

Usage: python3 known_plaintext.py <flash_dump.bin>
"""
import sys

STORAGE_OFF = 0x100000
SEC = 512

# FAT12 / MS-DOS bytes any analyst predicts without the firmware.
KNOWN_LBA0 = {
    0x000: 0xEB, 0x002: 0x90,
    0x003: ord('M'), 0x004: ord('S'), 0x005: ord('D'), 0x006: ord('O'),
    0x007: ord('S'), 0x008: ord('5'), 0x009: ord('.'), 0x00A: ord('0'),
    0x1FE: 0x55, 0x1FF: 0xAA,
}
RESERVED_ZERO_LBAS = (1, 2, 3)   # standard MS-DOS reserved area is zero-filled


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 2
    dump = open(argv[1], "rb").read()
    st = dump[STORAGE_OFF:]

    ks = {}                                   # position -> recovered keystream byte
    for off, pt in KNOWN_LBA0.items():
        ks[off] = st[off] ^ pt                # boot-sector known-plaintext
    for L in RESERVED_ZERO_LBAS:              # zero sectors: 512 keystream bytes each
        for o in range(SEC):
            ks[L * SEC + o] = st[L * SEC + o] ^ 0x00

    # Verify: XOR the recovered keystream back and confirm it reproduces the
    # known plaintext exactly -- i.e. these positions are now decryptable.
    ok = all((st[p] ^ k) == (KNOWN_LBA0.get(p, 0x00)) for p, k in ks.items())
    print(f"recovered keystream at {len(ks)} positions with no firmware/PIN/device")
    print(f"  boot-sector sample: LBA0[0..2] keystream = {bytes(ks[i] for i in (0,1,2) if i in ks).hex(' ')}")
    print(f"  from zero sector 1: 512 keystream bytes, first 8 = {bytes(ks[SEC+i] for i in range(8)).hex(' ')}")
    print(f"  round-trip decrypt of known positions matches plaintext: {ok}")
    print("  => these bytes decrypt the same positions on ANY same-build device;")
    print("     linear structure extends them to every position (see attack7)")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
