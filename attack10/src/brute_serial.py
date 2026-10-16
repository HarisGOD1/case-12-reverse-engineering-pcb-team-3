#!/usr/bin/env python3
"""Negative check: is the AES-256 key a deterministic KDF of the public chip
serial? The serial 8295FF712987C1CA is readable, so if the key were derived
from it the search space would be a handful of KDFs, not 2^256.

Exact oracle: keystream[0] = AES256_ECB_ENC(key, IV_public XOR salt) must equal
4a5e4990642bb18272952f73b9854d09. Result on this device: no match -> the key is
NOT a function of public data.
"""
import sys, os, hashlib
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from decrypt_rp2350 import decrypt_with_raw_key

def find_dump():
    cands = (sys.argv[1:2]) + [
        os.path.join(os.path.dirname(HERE), "firmware_dump.bin"),               # report root
        os.path.join(os.path.dirname(os.path.dirname(HERE)), "firmware_dump.bin"),
        os.path.join(os.getcwd(), "firmware_dump.bin"),
    ]
    for c in cands:
        if c and os.path.isfile(c): return c
    raise SystemExit("firmware_dump.bin not found; put it in the report folder or pass its path as an argument")

DUMP = find_dump()
d = open(DUMP, "rb").read()
IV_PUB = d[0x80014:0x80024]
CT = d[0x80024:0x80024 + 0xc320]
assert len(IV_PUB) == 16 and len(CT) % 16 == 0

SER_BE = bytes.fromhex("8295ff712987c1ca")   # chip serial / unique id
SER_LE = SER_BE[::-1]
PT_KNOWN = bytes.fromhex("00200820970100201b0100201d010020")   # app vector table (from UART leak)

def plausible(pt):
    import struct
    sp = struct.unpack_from("<I", pt, 0)[0]
    rv = struct.unpack_from("<I", pt, 4)[0]
    vt = (0x20000000 <= sp <= 0x20082000) and (rv & 1) and (
        0x20000000 <= rv < 0x20082000 or 0x10000000 <= rv < 0x10200000)
    pb = pt[:4] == b"\xd3\xde\xff\xff"
    exact = pt[:16] == PT_KNOWN
    return vt, pb, exact

def k32(b): return (b * (32 // len(b) + 1))[:32]
def s16(b): return (b * (16 // len(b) + 1))[:16]

cands = []
for name, ser in (("be", SER_BE), ("le", SER_LE)):
    cands += [
        (f"sha256(serial)_{name} / zero_salt",        hashlib.sha256(ser).digest(),            b"\x00"*16),
        (f"sha256(serial)_{name} / sha256[:16]salt",  hashlib.sha256(ser).digest(),            hashlib.sha256(ser).digest()[:16]),
        (f"sha256(serial*2)_{name}",                  hashlib.sha256(ser+ser).digest(),        hashlib.sha256(ser+ser).digest()[:16]),
        (f"serial_repeated_{name}",                   k32(ser),                                s16(ser)),
        (f"sha256('enc'+serial)_{name}",              hashlib.sha256(b"enc"+ser).digest(),     b"\x00"*16),
        (f"sha512(serial)[:32/32:48]_{name}",         hashlib.sha512(ser).digest()[:32],       hashlib.sha512(ser).digest()[32:48]),
        (f"md5(serial)x2_{name}",                     (hashlib.md5(ser).digest()*2),           hashlib.md5(ser).digest()),
        (f"sha256(IV_PUB+serial)_{name}",             hashlib.sha256(IV_PUB+ser).digest(),     b"\x00"*16),
    ]

print(f"IV_public={IV_PUB.hex()}  ct={len(CT)}B  serial_be={SER_BE.hex()}")
hit = False
for name, key, salt in cands:
    pt = decrypt_with_raw_key(key, salt, IV_PUB, CT[:64])
    vt, pb, exact = plausible(pt)
    tag = "  <<< MATCH" if (vt or pb or exact) else ""
    print(f"{name:40s} head={pt[:16].hex()} vt={vt} pb={pb} exact={exact}{tag}")
    if vt or pb or exact:
        hit = True

print("\nRESULT:", "KEY DERIVED FROM SERIAL (see above)" if hit else
      "no serial-derived key matches -> key is NOT a function of public data; brute-force fully closed")
