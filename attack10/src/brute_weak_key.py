#!/usr/bin/env python3
"""
Exact-oracle brute force for the RP2350 enc_bootloader AES-256 key, assuming a
weak/human-chosen key was flashed instead of a random OTP secret.

Oracle (from live UART leak of block 0):
  nonce0 = IV_public XOR salt
  ks0    = AES256_ECB_ENC(key, nonce0)
  match if ks0 == KS0_TARGET

Exact single-block check: one AES-256 encryption per (key, salt) candidate,
no false positives possible (128-bit match). Result on this device: NOT FOUND
among structured candidates -> the key is a real random OTP secret.
"""
import hashlib
import itertools
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from decrypt_rp2350 import aes256_encrypt_block

IV_PUBLIC = bytes.fromhex("921e94597201e138c2747ca1e1bd1b0b")
KS0_TARGET = bytes.fromhex("4a5e4990642bb18272952f73b9854d09")

# upstream pico-examples demo key/salt (public, from the example repo)
DEMO_KEY = bytes.fromhex("ca065526618e6e8f58a4c9093a18d29b92cd1613b3759db8b18797a272abe8f0")
DEMO_SALT = bytes.fromhex("8cb6c678ecdb25dc5ef9c23d5492c48c")

assert len(IV_PUBLIC) == 16 and len(KS0_TARGET) == 16


def check(key32: bytes, salt16: bytes):
    assert len(key32) == 32 and len(salt16) == 16
    nonce0 = bytes(a ^ b for a, b in zip(IV_PUBLIC, salt16))
    ks = aes256_encrypt_block(key32, nonce0)
    return ks == KS0_TARGET


# ---------------------------------------------------------------- salts
SALTS = {
    "zero": b"\x00" * 16,
    "ff": b"\xff" * 16,
    "demo_salt": DEMO_SALT,
    "serial_x2": bytes.fromhex("8295ff712987c1ca") * 2,
    "serial2_x2": bytes.fromhex("179764f532d29850") * 2,
    "iv_public": IV_PUBLIC,
}

# ---------------------------------------------------------------- key strings
STRINGS = [
    "dsokolov", "d.sokolov", "D.Sokolov", "Sokolov",
    "Positive Technologies", "PositiveTechnologies", "PositiveLabs",
    "POSILABS", "posilabs", "positive", "Positive",
    "LCT2026", "lct2026", "LCT-2026", "ЛЦТ2026", "лцт2026",
    "rp2350", "RP2350", "rp2040", "RP2040",
    "enc_bootloader", "encbootloader", "enc-bootloader",
    "raspberrypi", "RaspberryPi", "raspberry", "Raspberry Pi",
    "password", "Password", "secret", "Secret", "admin", "Admin",
    "0000", "1234", "123456", "12345678", "00000000",
    "aeskey", "AESkey", "aes256", "AES256", "aes-256-ctr",
    "pico", "Pico", "pico2", "Pico2", "PICO2", "pico-sdk", "picoSDK",
    "usb_token", "usbtoken", "CyberSafe", "cybersafe", "CYBERSAFE",
    "safe", "Safe", "token", "Token",
    "qwerty", "QWERTY", "letmein", "pin", "PIN",
    "2026", "20261009", "2026-10-09", "09102026",
    "default", "Default", "test", "Test", "demo", "Demo", "DEMO",
    "C:/Users/dsokolov", "C:\\Users\\dsokolov",
    "unique_id", "pico_unique_id", "_retrieve_unique_id_on_boot",
    "8295FF712987C1CA", "8295ff712987c1ca",
    "CA2987712971FF95228", "179764F532D29850",
    "8B2316D5", "61826D31",
    "Positive Technologies CTF", "ЛЦТ 2026", "Positive Hack Days",
    "encrypted_partition", "partition_b", "PartitionB",
    "mbedtls", "mbedtls_aes", "picotool", "sdk-2.3.1", "2.3.1",
    "flash_id", "boot2", "stage2",
]

# build 32-byte keys from strings: pad/truncate, repeat-pad
STRING_KEYS = []
for s in STRINGS:
    b = s.encode()
    STRING_KEYS.append(("raw-null-pad:" + s, (b + b"\x00" * 32)[:32]))
    STRING_KEYS.append(("raw-space-pad:" + s, (b + b" " * 32)[:32]))
    rep = (b * (32 // len(b) + 1))[:32]
    STRING_KEYS.append(("raw-repeat:" + s, rep))

# hash-derived keys
HASH_KEYS = []
for s in STRINGS:
    b = s.encode()
    HASH_KEYS.append(("sha256:" + s, hashlib.sha256(b).digest()))
    HASH_KEYS.append(("sha512[:32]:" + s, hashlib.sha512(b).digest()[:32]))
    HASH_KEYS.append(("md5x2:" + s, hashlib.md5(b).digest() * 2))
    for saltname, salt in SALTS.items():
        HASH_KEYS.append((f"sha256({s}+{saltname}):", hashlib.sha256(b + salt).digest()))
        HASH_KEYS.append((f"sha256({saltname}+{s}):", hashlib.sha256(salt + b).digest()))

# serial / pubkey-derived
for hx, name in [
    ("8295ff712987c1ca", "serial"),
    ("ca1c8729717fff95", "serial_rev_bytes"),
    ("179764f532d29850", "serial2"),
    ("8b2316d5", "pubkey_prefix"),
    ("61826d31", "demo_pubkey"),
]:
    raw = bytes.fromhex(hx)
    HASH_KEYS.append((f"sha256({name}):", hashlib.sha256(raw).digest()))
    HASH_KEYS.append((f"repeat:{name}", (raw * (32 // len(raw) + 1))[:32]))

# byte-pattern keys
PATTERN_KEYS = []
PATTERN_KEYS.append(("incrementing-00..1f", bytes(range(32))))
PATTERN_KEYS.append(("incrementing-01..20", bytes((i + 1) & 0xFF for i in range(32))))
PATTERN_KEYS.append(("decrementing-1f..00", bytes(range(31, -1, -1))))
PATTERN_KEYS.append(("repeat-00010203", (bytes(range(4)) * 8)))
for b in range(256):
    PATTERN_KEYS.append((f"const-0x{b:02x}", bytes([b]) * 32))
PATTERN_KEYS.append(("demo_key_asis", DEMO_KEY))
PATTERN_KEYS.append(("demo_key_reversed", DEMO_KEY[::-1]))
PATTERN_KEYS.append(("demo_key_xor_serial",
                      bytes(a ^ b for a, b in zip(DEMO_KEY, (bytes.fromhex("8295ff712987c1ca") * 4)[:32]))))

# compound combinations of base tokens (join with various separators, hash)
COMPOUND_KEYS = []
BASE_TOKENS = [
    "dsokolov", "Positive", "PositiveTechnologies", "PositiveLabs",
    "LCT2026", "lct2026", "RP2350", "rp2350", "RP2350A0A4", "A4",
    "enc_bootloader", "pico2", "2026", "20261009", "sdk2.3.1",
    "CyberSafe", "token", "safe", "bootloader", "mbedtls",
]
for a, b in itertools.permutations(BASE_TOKENS, 2):
    for sep in ("", "_", "-", ".", " "):
        s = f"{a}{sep}{b}"
        COMPOUND_KEYS.append((f"sha256({s}):", hashlib.sha256(s.encode()).digest()))

# build-path / date derived
EXTRA = [
    "C:/Users/dsokolov/.pico-sdk/sdk/2.3.1",
    "C:\\Users\\dsokolov\\.pico-sdk\\sdk\\2.3.1",
    "dsokolov@2026-10-09", "RP2350A0A4-8295FF712987C1CA",
    "8295FF712987C1CA-RP2350A0A4", "enc_bootloader-2026-10-09",
    "RP2350 Positive Technologies LCT 2026",
    "Positive Technologies LCT 2026 RP2350",
]
for s in EXTRA:
    COMPOUND_KEYS.append((f"sha256({s}):", hashlib.sha256(s.encode()).digest()))
    COMPOUND_KEYS.append((f"sha512[:32]({s}):", hashlib.sha512(s.encode()).digest()[:32]))

ALL_KEYS = STRING_KEYS + HASH_KEYS + PATTERN_KEYS + COMPOUND_KEYS

print(f"Key candidates: {len(ALL_KEYS)}  Salt candidates: {len(SALTS)}  "
      f"Total checks: {len(ALL_KEYS) * len(SALTS)}")

found = None
checked = 0
for kname, key in ALL_KEYS:
    for sname, salt in SALTS.items():
        checked += 1
        if check(key, salt):
            found = (kname, key, sname, salt)
            break
    if found:
        break

print(f"Checked: {checked}")
if found:
    kname, key, sname, salt = found
    print("FOUND!")
    print("  key name :", kname)
    print("  key hex  :", key.hex())
    print("  salt name:", sname)
    print("  salt hex :", salt.hex())
else:
    print("NOT FOUND among structured candidates -> key is a real random OTP secret.")
    sys.exit(2)
