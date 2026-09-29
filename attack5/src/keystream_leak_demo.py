#!/usr/bin/env python3
"""Model USB reads beyond storage and recover the generator from the tail alone

The input dump models flash bytes; attack1 models the device XOR response
The recovery function receives only decoded responses from erased tail sectors
The stored encrypted volume serves as a separate verification input
See attack5/README.md for the threat model and hardware limits
"""
import os
import sys
import math
import hashlib
from collections import Counter
from functools import lru_cache
from pathlib import Path

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "attack3", "src"))
import recover_keystream as R  # noqa: E402

STORAGE_OFF = R.STORAGE_OFF
STORAGE_LEN = R.STORAGE_LEN
SECTOR = R.SEC
DISK_SECTORS = 0x800
DISK_BYTES  = DISK_SECTORS * SECTOR


@lru_cache(maxsize=1)
def device_keystream_block():
    sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "attack1", "src"))
    from decrypt_storage import keystream_block
    return keystream_block


def ks_sector(lba):
    keystream_block = device_keystream_block()
    return b"".join(keystream_block(lba, j) for j in range(SECTOR // 16))


def entropy(buf):
    cnt = Counter(buf)
    n = len(buf)
    return max(0.0, -sum((v / n) * math.log2(v / n) for v in cnt.values()))


def decode_tail_response(tail):
    first = STORAGE_LEN // SECTOR
    if len(tail) != (DISK_SECTORS - first) * SECTOR:
        raise ValueError("USB tail must contain every sector from the storage boundary to the MSC limit")
    observed = {}
    for lba in range(first, DISK_SECTORS):
        offset = (lba - first) * SECTOR
        served = tail[offset:offset + SECTOR]
        if served != b"\x00" * SECTOR:
            observed[lba] = bytes(0xFF ^ byte for byte in served)
    return observed


def recover_generator_from_leak(ks_by_lba):
    """Recover a volume-equivalent generator from USB tail observations alone"""
    if len(ks_by_lba) < 4 or any(lba < STORAGE_LEN // SECTOR or len(ks) != SECTOR
                                 for lba, ks in ks_by_lba.items()):
        return None
    sample = sorted(ks_by_lba)[:4]
    survivors = []
    for B in R.FAMOUS:
        A = (32 * B) & R.MASK
        for W in R.FAMOUS:
            cons = []
            for lba in sample:
                ks = ks_by_lba[lba]
                for o in range(R.SEC):
                    off = (lba * A + R.lane_offset(o, B, W)) & R.MASK
                    cons.append((ks[o], off))
            if not R._solve(cons):
                continue
            constraints = [(ks[o], (lba * A + R.lane_offset(o, B, W)) & R.MASK)
                           for lba, ks in sorted(ks_by_lba.items()) for o in range(SECTOR)]
            seeds = R._solve(constraints)
            if not seeds:
                continue
            args = R.position_args(STORAGE_LEN // SECTOR, A, B, W)
            reference = R.keystream_from_args(args, seeds[0])
            if all((R.keystream_from_args(args, seed) == reference).all()
                   for seed in seeds[1:]):
                survivors.append((A, seeds[0], B, W))
    return survivors[0] if len(survivors) == 1 else None


def main(argv):
    if len(argv) == 4 and argv[1] == "--usb-tail-only":
        try:
            observations = decode_tail_response(Path(argv[2]).read_bytes())
            recovered = recover_generator_from_leak(observations)
            if recovered is None:
                raise ValueError("No unique volume-equivalent generator matches the USB observations")
            A, seed, B, W = recovered
            args = R.position_args(STORAGE_LEN // SECTOR, A, B, W)
            table = R.keystream_from_args(args, seed).tobytes()
            with open(argv[3], "xb") as output:
                output.write(table)
        except (OSError, ValueError) as error:
            print(f"USB-only recovery failed: {error}", file=sys.stderr)
            return 1
        print(f"PASS: recovered B=0x{B:08X} W=0x{W:08X} A=0x{A:08X} seed=0x{seed:08X}")
        print(f"Full-volume keystream SHA-256: {hashlib.sha256(table).hexdigest()}")
        print(f"Saved: {argv[3]} ({len(table)} bytes); no flash dump was read")
        return 0

    if len(argv) not in (2, 4) or (len(argv) == 4 and argv[2] != "--usb-tail"):
        print(__doc__)
        print("Usage: keystream_leak_demo.py <full_dump.bin> [--usb-tail <raw_usb_tail.bin>]")
        print("   or: keystream_leak_demo.py --usb-tail-only <raw_usb_tail.bin> <new_keystream.bin>")
        return 2
    data = Path(argv[1]).read_bytes()
    if len(data) != 0x200000:
        print("[FAIL] expected a complete 2 MiB flash dump")
        return 2

    outside_start = STORAGE_LEN // SECTOR
    tail_capture = Path(argv[3]).read_bytes() if len(argv) == 4 else None
    if tail_capture is not None:
        try:
            decode_tail_response(tail_capture)
        except ValueError as error:
            print(f"[FAIL] {error}")
            return 2
    print("=== attack5: modeled USB reads beyond storage ===")
    print(f"MSC disk        : {DISK_SECTORS} sectors = {DISK_BYTES // 1024} KiB "
          f"(param_2 < 0x800 at 0x10000354)")
    print(f"storage         : {STORAGE_LEN // 1024} KiB = {STORAGE_LEN // SECTOR} sectors (LBA 0..{STORAGE_LEN // SECTOR - 1})")

    # The first erased sector can precede the configured storage boundary
    first_ff = None
    for lba in range(DISK_SECTORS):
        off = STORAGE_OFF + lba * SECTOR
        if data[off:off + SECTOR] == b"\xff" * SECTOR:
            first_ff = lba
            break
    if first_ff is None:
        print("[!] no fully erased sectors; the dump does not match this image")
        return 2
    print(f"\n[1] first fully erased flash sector: LBA {first_ff} (0x{first_ff:x})")
    print(f"    storage boundary: LBA {outside_start}; "
          f"LBA {first_ff}..{outside_start - 1} are erased but still within storage")
    print(f"    beyond storage but within MSC: LBA {outside_start}..{DISK_SECTORS - 1}")

    # Show both the first erased sector and the configured boundary
    print("\n[2] comparison of served data (flash XOR keystream):")
    for lba in (first_ff - 1, first_ff, outside_start, outside_start + 100, DISK_SECTORS - 1):
        off = STORAGE_OFF + lba * SECTOR
        flash = data[off:off + SECTOR]
        served = bytes(c ^ k for c, k in zip(flash, ks_sector(lba)))
        erased = all(b == 0xFF for b in flash)
        if lba < outside_start:
            tag = "erased but within storage" if erased else "within storage"
        else:
            tag = "beyond storage -> keystream leak (~keystream)" if erased else "beyond storage, not erased"
        print(f"  LBA {lba:4d} (0x{lba:03x}): {tag}")
        print(f"    flash : {flash[:16].hex(' ')}")
        print(f"    served: {served[:16].hex(' ')}  entropy={entropy(served):.3f} bits/byte")

    print("\n[3] validate the model on every sector beyond storage:")
    ok = True
    leaked = 0
    exceptions = []
    for lba in range(outside_start, DISK_SECTORS):
        off = STORAGE_OFF + lba * SECTOR
        flash = data[off:off + SECTOR]
        served = bytes(c ^ k for c, k in zip(flash, ks_sector(lba)))
        if tail_capture is not None:
            offset = (lba - outside_start) * SECTOR
            measured = tail_capture[offset:offset + SECTOR]
            if measured != served:
                print(f"    FAIL: USB response at LBA {lba} differs from the stored image model")
                return 1
        complement = bytes(0xFF ^ k for k in ks_sector(lba))
        if flash == b"\xff" * SECTOR:
            if served != complement or all(b == 0xFF for b in served):
                print(f"    FAIL at LBA {lba}")
                ok = False
                break
            leaked += 1
        else:
            exceptions.append(lba)
    if ok:
        print(f"    OK: {leaked} erased sectors beyond storage return exactly ~keystream")
        if exceptions:
            print(f"    exceptions ({len(exceptions)}: {exceptions[:4]}...): not erased; they contain"
                  " encrypted zeros from the source image; served bytes are 0x00 and cannot feed the oracle")
    total = leaked * SECTOR
    print(f"    {total} keystream bytes available from modeled USB reads")
    if not ok:
        return 1

    print("\n[4] recover from USB observations only (attack3 public dictionary):")
    if tail_capture is not None:
        ks_by_lba = decode_tail_response(tail_capture)
    else:
        ks_by_lba = {}
        for lba in range(outside_start, DISK_SECTORS):
            off = STORAGE_OFF + lba * SECTOR
            flash = data[off:off + SECTOR]
            if flash == b"\xff" * SECTOR:
                served = bytes(c ^ k for c, k in zip(flash, ks_sector(lba)))
                ks_by_lba[lba] = bytes(0xFF ^ byte for byte in served)
    res = recover_generator_from_leak(ks_by_lba)
    if res is None:
        print("    FAIL: could not recover the generator from the leak")
        return 1
    A, seed, B, W = res
    print(f"    B=0x{B:08X} W=0x{W:08X} A=0x{A:08X} seed=0x{seed:08X} (equivalent representative)")

    print("\n[5] independent check against the encrypted volume in the dump:")
    storage_ct = data[STORAGE_OFF:STORAGE_OFF + STORAGE_LEN]
    pt = R.decrypt_all(storage_ct, A, seed, B, W)
    zf_ok = R.volume_has_valid_zip(pt)
    print(f"    sector 0 -> {pt[:11].hex(' ')} ...  OEM={bytes(pt[3:11])!r}  boot sig={pt[510:512].hex(' ')}")
    print(f"    valid FAT12 boot: {R.strict_bootsector(pt[:SECTOR])};  CRC-valid ZIP: {zf_ok}")
    if not (R.strict_bootsector(pt[:SECTOR]) and zf_ok):
        print("    FAIL: recovered keystream did not decrypt storage")
        return 1
    print("    OK: recovered parameters without ciphertext; the volume confirms extrapolation")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
