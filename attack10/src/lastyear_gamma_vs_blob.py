#!/usr/bin/env python3
"""Negative check: is the blob @0x300000 / tail @0x3ffe00 encrypted with LAST
YEAR's self-made LCG gamma (fixed firmware constants, NO key, NO PIN)? If the
app this year is a ported usb_token, its internal storage might reuse that
cipher -> blob decrypts with the KNOWN gamma, bypassing OTP.
Oracle: per start_lba, decrypt sector 0, check FAT12 sig and whole-region
entropy drop. Result on this device: no hit -> not this cipher.
Offline, read-only on the dump.
"""
import os, sys, math
from collections import Counter

MUL_LBA = 0x38C9CDA0
SEED     = 0x9E37A9EA
INC_BLK  = 0x41C64E6D
LANES = [0x61C8864F, 0x00000000, 0x9E3779B1, 0x3C6EF362,
         0xDAA66D13, 0x78DDE6C4, 0x17156075, 0xB54CDA26,
         0x538453D7, 0xF1BBCD88, 0x8FF34739, 0x2E2AC0EA,
         0xCC623A9B, 0x6A99B44C, 0x08D12DFD, 0xA708A7AE]

def ks_block(lba, j):
    s = (lba * MUL_LBA + SEED + j * INC_BLK) & 0xFFFFFFFF
    return bytes(((s + c) & 0xFFFFFFFF) >> 24 for c in LANES)

HERE = os.path.dirname(os.path.abspath(__file__))
def find_dump():
    cands = (sys.argv[1:2]) + [
        os.path.join(os.path.dirname(HERE), "firmware_dump.bin"),
        os.path.join(os.path.dirname(os.path.dirname(HERE)), "firmware_dump.bin"),
        os.path.join(os.getcwd(), "firmware_dump.bin"),
    ]
    for c in cands:
        if c and os.path.isfile(c): return c
    raise SystemExit("firmware_dump.bin not found; put it in the report folder or pass its path as an argument")
DUMP = find_dump()
d = open(DUMP, "rb").read()
REGIONS = {"blob@0x300000": d[0x300000:0x306600], "tail@0x3ffe00": d[0x3ffe00:0x400000]}

def ent(b):
    if not b: return 0.0
    c = Counter(b); n = len(b)
    return -sum(v/n*math.log2(v/n) for v in c.values())

def dec_region(region, start_lba):
    out = bytearray()
    nsec = (len(region) + 511)//512
    for sec in range(nsec):
        for j in range(32):
            off = sec*512 + j*16
            if off >= len(region): break
            ks = ks_block(start_lba+sec, j)
            blk = region[off:off+16]
            out += bytes(a ^ b for a, b in zip(blk, ks))
    return bytes(out)

def sec0_sig(region, start_lba):
    out = bytearray()
    for j in range(32):
        ks = ks_block(start_lba, j)
        blk = region[j*16:j*16+16]
        out += bytes(a ^ b for a, b in zip(blk, ks))
    return (out[0]==0xEB and out[2]==0x90), (out[510:512]==b"\x55\xaa"), out[:16]

SWEEP = 300000
for name, region in REGIONS.items():
    base_ent = ent(region[:4096] if len(region) >= 4096 else region)
    print(f"=== {name}  ({len(region)}B, baseline entropy {base_ent:.3f}) ===")
    fat_hits = sig_hits = 0
    for start_lba in range(SWEEP):
        fj, bs, head = sec0_sig(region, start_lba)
        if bs:
            sig_hits += 1
            if fj: fat_hits += 1
    print(f"  sector-0 sweep 0..{SWEEP}: 55AA hits={sig_hits}  FAT(EB..90+55AA)={fat_hits}")
    for slba in (0, 0x300000//512, 6144, 2048):
        dec = dec_region(region, slba)
        e = ent(dec[:4096] if len(dec) >= 4096 else dec)
        flag = "  <- ENTROPY DROP" if e < base_ent - 0.5 else ""
        print(f"  start_lba={slba:>7}: decrypted entropy={e:.3f}  head={dec[:16].hex()}{flag}")
    print()
print("RESULT: FAT/entropy-drop hit => last-year gamma decrypts it (OTP bypass). None => not this cipher.")
