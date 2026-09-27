#!/usr/bin/env python3
"""
Self-contained regression + falsification tests for recover_keystream.

No firmware, no flash dump: a synthetic FAT12-shaped volume with a real ZIP is
encrypted with the same positional stream cipher and handed back to the
recovery. Run directly:

    python3 test_recover.py          # exit 0 = all pass, non-zero = a failure

An extra end-to-end check against a real dump runs only if RECOVER_DUMP points
at one; otherwise it is skipped (never silently passed).
"""
import io
import os
import random
import sys
import zipfile

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import recover_keystream as R  # noqa: E402  (local module, resolved from this dir)

MASK = R.MASK


def _scalar_keystream_byte(L, o, A, seed, B, W):
    # Independent scalar reference for the vectorised generator.
    j, t = o // R.BLK, o % R.BLK
    V = (L * A + seed + j * B + (t - 1) * W) & MASK
    return V >> 24


def test_generator_matches_scalar():
    A, seed, B, W = 0x38C9CDA0, 0x9E37A9EA, 0x41C64E6D, 0x9E3779B1
    args = R.position_args(20, A, B, W)
    ks = R.keystream_from_args(args, seed)
    for _ in range(2000):
        L = random.randrange(20)
        o = random.randrange(R.SEC)
        assert ks[L * R.SEC + o] == _scalar_keystream_byte(L, o, A, seed, B, W)
    # A == 32*B is the continuity relation the recovery relies on.
    assert (32 * B) & MASK == A


def test_solve_recovers_and_is_sound():
    # Top-byte-only intersection has a hard floor of about 2**24 / N survivors
    # (that is why the seed is recovered as an equivalence class, not a point).
    # With N large the set is small and enumerable; the truth always survives and
    # every survivor is consistent.
    for _ in range(5):
        x = random.randrange(1 << 32)
        offs = [random.randrange(1 << 32) for _ in range(4000)]
        cons = [(((x + off) & MASK) >> 24, off) for off in offs]
        sol = R._solve(cons, limit=200000)
        assert sol is not None and x in sol               # truth survives
        assert len(sol) < 20000                            # floor ~ 2**24/4000
        for cand in random.sample(sol, min(len(sol), 200)):
            assert all((((cand + off) & MASK) >> 24) == kb for kb, off in cons)
    # monotone shrink: more constraints never enlarge the survivor set.
    x = 0xDEADBEEF
    offs = [random.randrange(1 << 32) for _ in range(6000)]
    cons = [(((x + off) & MASK) >> 24, off) for off in offs]
    n_half = len(R._solve(cons[:3000], limit=1 << 30))
    n_full = len(R._solve(cons, limit=1 << 30))
    assert n_full <= n_half
    # an impossible pair of constraints on the same offset yields nothing.
    assert R._solve([(0x00, 12345), (0xFF, 12345)]) == []


def _build_encrypted_volume(seed, B, W, nsec=64):
    A = (32 * B) & MASK
    zbuf = io.BytesIO()
    with zipfile.ZipFile(zbuf, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("secret.txt", b"the treasure is behind the third star\n" * 30)
    zbytes = zbuf.getvalue()

    vol = bytearray(nsec * R.SEC)
    boot = bytearray(R.SEC)
    boot[0], boot[1], boot[2] = 0xEB, 0x3C, 0x90
    boot[3:11] = b"MSDOS5.0"
    boot[11], boot[12] = 0x00, 0x02          # bytes/sector = 512
    boot[13] = 1                              # sectors/cluster
    boot[14], boot[15] = 4, 0                 # reserved sectors = 4
    boot[16] = 2                              # number of FATs
    boot[17], boot[18] = 0x00, 0x02           # root entries = 512
    boot[21] = 0xF8                           # media descriptor
    boot[510], boot[511] = 0x55, 0xAA
    vol[0:R.SEC] = boot
    vol[4 * R.SEC:4 * R.SEC + 3] = bytes([0xF8, 0xFF, 0xFF])   # FAT1 signature
    zip_at = 8 * R.SEC
    vol[zip_at:zip_at + len(zbytes)] = zbytes                  # a real ZIP to find

    args = R.position_args(nsec, A, B, W)
    ct = (np.frombuffer(bytes(vol), dtype=np.uint8)
          ^ R.keystream_from_args(args, seed)).tobytes()
    return bytes(vol), ct, zip_at, len(zbytes)


def test_end_to_end_synthetic():
    seed, B, W = 0x12345678, 0x41C64E6D, 0x9E3779B1
    vol, ct, _, _ = _build_encrypted_volume(seed, B, W)
    res = R.recover(ct)
    assert res is not None, "recovery failed on a valid synthetic volume"
    A, s, rb, rw = res
    assert (rb, rw) == (B, W)
    assert (32 * rb) & MASK == A
    # a class-equivalent seed must decrypt the whole volume byte-for-byte.
    assert R.decrypt_all(ct, A, s, rb, rw) == vol
    assert R.volume_has_valid_zip(vol)


def test_falsification_corrupt_zip():
    seed, B, W = 0x0BADF00D, 0x41C64E6D, 0x9E3779B1
    _, ct, zip_at, zlen = _build_encrypted_volume(seed, B, W)
    ct = bytearray(ct)
    ct[zip_at + zlen // 2] ^= 0x01             # flip a byte inside the compressed data
    assert R.recover(bytes(ct)) is None, "recovery must reject a corrupted ZIP"


def test_falsification_wrong_dictionary():
    # If the true multipliers were absent from the dictionary, recovery fails.
    seed = 0x0FACADE0
    B, W = 0x41C64E6D, 0x9E3779B1
    _, ct, _, _ = _build_encrypted_volume(seed, B, W)
    saved = R.FAMOUS
    try:
        R.FAMOUS = [c for c in R.FAMOUS if c not in (B, W)]
        assert R.recover(ct) is None, "recovery must fail without the true constants"
    finally:
        R.FAMOUS = saved


def _run():
    tests = [v for k, v in sorted(globals().items()) if k.startswith("test_")]
    failed = 0
    for t in tests:
        try:
            t()
            print(f"[ok]   {t.__name__}")
        except AssertionError as e:
            failed += 1
            print(f"[FAIL] {t.__name__}: {e}")
        except Exception as e:  # noqa: BLE001 - surface unexpected errors as failures
            failed += 1
            print(f"[ERR]  {t.__name__}: {type(e).__name__}: {e}")

    dump = os.environ.get("RECOVER_DUMP")
    if dump and os.path.exists(dump):
        rc = R.main(["recover_keystream.py", dump, os.devnull])
        print(f"[{'ok' if rc == 0 else 'FAIL'}]   integration real dump (exit {rc})")
        failed += (rc != 0)
    else:
        print("[skip] integration real dump (set RECOVER_DUMP to enable)")

    print(f"\n{'PASS' if failed == 0 else 'FAIL'}: {len(tests)} unit tests, {failed} failure(s)")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(_run())
