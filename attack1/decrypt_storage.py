#!/usr/bin/env python3
"""Decrypt the storage area of a full RP2040 flash dump

The USB READ(10) callback at 0x10000354 uses constants from flash, not the PIN
See attack1/README.md for the cipher and artifacts/README.md for the source dump
"""
import sys

STORAGE_OFF = 0x100000
STORAGE_LEN = 0xB0000
SECTOR = 512

MUL_LBA = 0x38C9CDA0
SEED = 0x9E37A9EA
INC_BLK = 0x41C64E6D

LANES = [0x61C8864F, 0x00000000, 0x9E3779B1, 0x3C6EF362,
         0xDAA66D13, 0x78DDE6C4, 0x17156075, 0xB54CDA26,
         0x538453D7, 0xF1BBCD88, 0x8FF34739, 0x2E2AC0EA,
         0xCC623A9B, 0x6A99B44C, 0x08D12DFD, 0xA708A7AE]


def keystream_block(lba, j):
    s = (lba * MUL_LBA + SEED + j * INC_BLK) & 0xFFFFFFFF
    return bytes(((s + c) & 0xFFFFFFFF) >> 24 for c in LANES)


def decrypt(dump_path):
    with open(dump_path, "rb") as f:
        f.seek(STORAGE_OFF)
        buf = bytearray(f.read(STORAGE_LEN))
    if len(buf) != STORAGE_LEN:
        raise ValueError(f"Incomplete encrypted storage: expected {STORAGE_LEN} bytes, got {len(buf)}")
    for lba in range(len(buf) // SECTOR):
        for j in range(SECTOR // 16):
            off = lba * SECTOR + j * 16
            ks = keystream_block(lba, j)
            for k in range(16):
                buf[off + k] ^= ks[k]
    return buf


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 1
    dump = argv[1]
    out = argv[2] if len(argv) > 2 else "storage_decrypted.img"
    try:
        pt = decrypt(dump)
    except (OSError, ValueError) as error:
        print(f"Decryption failed: {error}", file=sys.stderr)
        return 1
    with open(out, "wb") as f:
        f.write(pt)
    print("sector 0 :", pt[:16].hex(" "))
    print("OEM name :", bytes(pt[3:11]).decode("latin1"))
    print("boot sig :", pt[510:512].hex(" "), "(expect 55 aa)")
    print("written  :", out, len(pt), "bytes")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
