#!/usr/bin/env python3
"""
PoC decryptor for the LCT-2026 "cyber-safe" USB token (RP2040 / usb_token).

The hidden storage lives at flash 0x10100000 (file offset 0x100000 in a full
2 MiB dump, 704 KiB long). Firmware exposes it over USB-MSC and XOR-decrypts
every sector on the fly in the read10 callback (Ghidra address 0x10000354).

The keystream is a self-made CTR-style stream whose key material is a set of
constants baked into the firmware image (pool at 0x100004ec..0x10000538).
The 4-digit PIN does NOT take part in the keystream, so the volume can be
decrypted straight from a flash dump, without the device, the PIN or the encoder.

For a 16-byte block at absolute LBA L and block index j (0..31):
    s = (L*0x38C9CDA0 + 0x9E37A9EA + j*0x41C64E6D) & 0xFFFFFFFF
    keystream_byte_i = top byte of (s + C_i)   for i in 0..15
    plaintext = ciphertext XOR keystream

Usage:
    python3 decrypt_storage.py backup_full.bin storage_decrypted.img
"""
import sys

STORAGE_OFF = 0x100000          # 0x10100000 - 0x10000000 (XIP base)
STORAGE_LEN = 0xB0000           # 704 KiB
SECTOR = 512

MUL_LBA = 0x38C9CDA0            # per-LBA multiplier   (DAT_100004f0)
SEED    = 0x9E37A9EA            # base nonce           (DAT_100004f4)
INC_BLK = 0x41C64E6D            # per-block increment  (DAT_10000530)

# One additive constant per output byte, in emitted-byte order.
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
    pt = decrypt(dump)
    with open(out, "wb") as f:
        f.write(pt)
    print("sector 0 :", pt[:16].hex(" "))
    print("OEM name :", bytes(pt[3:11]).decode("latin1"))
    print("boot sig :", pt[510:512].hex(" "), "(expect 55 aa)")
    print("written  :", out, len(pt), "bytes")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
