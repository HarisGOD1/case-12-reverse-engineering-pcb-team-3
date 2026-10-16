#!/usr/bin/env python3
"""Negative check: is the blob @0x300000 encrypted under a key DERIVED FROM THE
PIN? If so the PIN is only 10^4 values -> brute-force offline, bypassing OTP.
Oracle: decrypt head of the blob with each candidate, look for FAT12 BPB /
picobin / structure. Result on this device: 0 structured hits -> PIN is NOT the
storage key (it is an access gate, not a crypto factor).
Offline, read-only on the dump.
"""
import sys, os, struct, hashlib
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

try:
    from Crypto.Cipher import AES
    def ecb(key, b16): return AES.new(key, AES.MODE_ECB).encrypt(b16)
except Exception:
    from decrypt_rp2350 import aes256_encrypt_block as _enc
    def ecb(key, b16): return bytes(_enc(key, b16))

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
blob = d[0x300000:0x306600]          # 26112 B storage candidate
print(f"blob={len(blob)}B  head={blob[:16].hex()}")

def ks_block_fw(key, iv0, k):        # firmware CTR: nonce[12..15] ^= BE(k)
    n = bytearray(iv0); kb = struct.pack(">I", k)
    for i in range(4): n[12+i] ^= kb[i]
    return ecb(key, bytes(n))

def ks_block_std(key, iv0, k):       # standard CTR: 128-bit BE counter add
    ctr = (int.from_bytes(iv0, "big") + k) & ((1 << 128) - 1)
    return ecb(key, ctr.to_bytes(16, "big"))

def kdf(pin):
    s = "%04d" % pin
    return [
        ("sha256(ascii4)",       hashlib.sha256(s.encode()).digest()),
        ("sha256(str)",          hashlib.sha256(str(pin).encode()).digest()),
        ("md5x2(ascii4)",        hashlib.md5(s.encode()).digest() * 2),
        ("ascii4_repeat",        (s.encode() * 8)[:32]),
        ("sha256(ascii4+NUL)",   hashlib.sha256(s.encode() + b"\x00"*12).digest()),
        ("sha256('pin'+ascii4)", hashlib.sha256(b"pin" + s.encode()).digest()),
    ]

IVS = [("zero", b"\x00"*16), ("blobhead", blob[:16])]
SCHEMES = [("fw_ctr", ks_block_fw), ("std_ctr", ks_block_std)]

def head_decrypt(key, iv0, schemefn, nbytes=16):
    out = bytearray()
    for bi in range((nbytes + 15)//16):
        ks = schemefn(key, iv0, bi)
        blk = blob[bi*16:bi*16+16]
        out += bytes(a ^ b for a, b in zip(blk, ks))
    return bytes(out)

def looks_structured(dec):
    tags = []
    if dec[0:1] == b"\xeb" and dec[2:3] == b"\x90": tags.append("FAT_jump")
    if dec[0:4] == b"\xd3\xde\xff\xff": tags.append("picobin")
    if dec[3:11] in (b"MSDOS5.0", b"MSWIN4.1", b"mkfs.fat"): tags.append("OEM")
    printable = sum(1 for c in dec[:16] if 0x20 <= c < 0x7f)
    if printable >= 13: tags.append(f"ascii{printable}")
    return tags

hits = []
tested = 0
for pin in range(10000):
    for kn, key in kdf(pin):
        for ivn, iv in IVS:
            for sn, fn in SCHEMES:
                dec0 = head_decrypt(key, iv, fn, 16)
                tested += 1
                t = looks_structured(dec0)
                if t:
                    full = head_decrypt(key, iv, fn, 512)
                    sig = full[0x1fe:0x200] == b"\x55\xaa"
                    hits.append((pin, kn, ivn, sn, t, sig))

print(f"tested {tested} (pin x kdf x iv x scheme)")
confirmed = [h for h in hits if h[5]]
structured = [h for h in hits if "FAT_jump" in h[4] or "picobin" in h[4] or "OEM" in h[4]]
print(f"55AA-confirmed hits: {len(confirmed)}   real-structure hits: {len(structured)}   ascii-only (noise): {len(hits)-len(structured)}")
if confirmed:
    print("RESULT: CONFIRMED FAT at 0x300000 under a PIN-derived key:")
    for h in confirmed: print("   ", h)
else:
    print("RESULT: no PIN-derived key yields structured plaintext at 0x300000")
    print("-> model B (PIN as storage key) NOT supported; favors model A (PIN = access gate)")
