#!/usr/bin/env python3
"""
Offline decryptor for the RP2350 pico-examples "encrypted bootloader" scheme
(raspberrypi/pico-examples bootloaders/encrypted + picotool encrypt).

Scheme (verified against upstream source, master branch):
  - AES-256-CTR (14 rounds), keystream = AES256-ECB-ENC(counter_block).
  - Key is a 4-way XOR share, stored in OTP pages 29..30.
  - Initial counter block iv0 = IV_public (from flash) XOR IV_OTPsalt (OTP page 31).
  - Per 16-byte block k: nonce = iv0 with bytes[12..15] ^= big-endian uint32(k).
    (NB: the 32-bit counter is XORed into the last 4 bytes, NOT added; top 12 bytes stay fixed.)
  - plaintext = ciphertext XOR keystream (encryption and decryption are identical).

Self-contained: includes a minimal pure-python AES-256 so no external lib is needed.
"""

# ---------------------------------------------------------------- AES-256 core
_SBOX = []
def _init_sbox():
    p = q = 1
    sbox = [0]*256
    while True:
        p = p ^ ((p << 1) & 0xFF) ^ (0x1B if p & 0x80 else 0)
        q ^= q << 1; q ^= q << 2; q ^= q << 4; q &= 0xFF
        if q & 0x80: q ^= 0x09
        xf = q ^ ((q << 1)|(q >> 7)) ^ ((q << 2)|(q >> 6)) ^ ((q << 3)|(q >> 5)) ^ ((q << 4)|(q >> 4))
        sbox[p] = (xf ^ 0x63) & 0xFF
        if p == 1: break
    sbox[0] = 0x63
    return sbox
_SBOX = _init_sbox()
_RCON = [0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,0x6C,0xD8,0xAB,0x4D]

def _xtime(a): return ((a << 1) ^ 0x1B) & 0xFF if a & 0x80 else (a << 1)
def _mul(a,b):
    r = 0
    for _ in range(8):
        if b & 1: r ^= a
        hi = a & 0x80; a = (a << 1) & 0xFF
        if hi: a ^= 0x1B
        b >>= 1
    return r

def _key_expansion_256(key):
    assert len(key) == 32
    Nk, Nr = 8, 14
    w = [list(key[4*i:4*i+4]) for i in range(Nk)]
    for i in range(Nk, 4*(Nr+1)):
        t = list(w[i-1])
        if i % Nk == 0:
            t = t[1:]+t[:1]
            t = [_SBOX[b] for b in t]
            t[0] ^= _RCON[i//Nk - 1]
        elif i % Nk == 4:
            t = [_SBOX[b] for b in t]
        w.append([w[i-Nk][j] ^ t[j] for j in range(4)])
    return w

def _add_round_key(s, w, rnd):
    for c in range(4):
        for r in range(4):
            s[r][c] ^= w[rnd*4+c][r]

def aes256_encrypt_block(key, block):
    assert len(block) == 16
    w = _key_expansion_256(key)
    s = [[block[r+4*c] for c in range(4)] for r in range(4)]
    _add_round_key(s, w, 0)
    for rnd in range(1, 14):
        for r in range(4):
            for c in range(4):
                s[r][c] = _SBOX[s[r][c]]
        s = [s[0], s[1][1:]+s[1][:1], s[2][2:]+s[2][:2], s[3][3:]+s[3][:3]]
        ns = [[0]*4 for _ in range(4)]
        for c in range(4):
            col = [s[r][c] for r in range(4)]
            ns[0][c] = _mul(col[0],2)^_mul(col[1],3)^col[2]^col[3]
            ns[1][c] = col[0]^_mul(col[1],2)^_mul(col[2],3)^col[3]
            ns[2][c] = col[0]^col[1]^_mul(col[2],2)^_mul(col[3],3)
            ns[3][c] = _mul(col[0],3)^col[1]^col[2]^_mul(col[3],2)
        s = ns
        _add_round_key(s, w, rnd)
    for r in range(4):
        for c in range(4):
            s[r][c] = _SBOX[s[r][c]]
    s = [s[0], s[1][1:]+s[1][:1], s[2][2:]+s[2][:2], s[3][3:]+s[3][:3]]
    _add_round_key(s, w, 14)
    return bytes(s[r][c] for c in range(4) for r in range(4))

# ---------------------------------------------------------------- scheme logic
def reconstruct_key(key4way: bytes) -> bytes:
    """key4way -> 32-byte AES-256 key.
    Each AES key word = XOR of 4 consecutive 32-bit shares (little-endian words).
    FIB workaround: words 0..15 give key[0..3], words 16..31 are dummy (skipped),
    words 32..47 give key[4..7]. So key4way must be >= 192 bytes."""
    import struct
    w = list(struct.unpack('<48I', key4way[:192]))
    aes_key_words = []
    for i in range(8):
        skip = (i // 4) * 16
        base = i*4 + skip
        aes_key_words.append(w[base] ^ w[base+1] ^ w[base+2] ^ w[base+3])
    return struct.pack('<8I', *aes_key_words)

def make_iv0(iv_public: bytes, iv_otp_salt: bytes) -> bytes:
    assert len(iv_public) == 16 and len(iv_otp_salt) == 16
    return bytes(a ^ b for a, b in zip(iv_public, iv_otp_salt))

def ctr_xor_decrypt(aes_key: bytes, iv0: bytes, ciphertext: bytes) -> bytes:
    """AES-256-CTR with the picotool/mbedtls 'XOR counter into last 4 bytes' nonce rule."""
    out = bytearray()
    nblk = (len(ciphertext) + 15) // 16
    for k in range(nblk):
        nonce = bytearray(iv0)
        # XOR 32-bit big-endian counter k into bytes 12..15 (byte 15 = LSB)
        nonce[12] ^= (k >> 24) & 0xFF
        nonce[13] ^= (k >> 16) & 0xFF
        nonce[14] ^= (k >>  8) & 0xFF
        nonce[15] ^= (k      ) & 0xFF
        ks = aes256_encrypt_block(aes_key, bytes(nonce))
        chunk = ciphertext[k*16:k*16+16]
        out += bytes(c ^ s for c, s in zip(chunk, ks))
    return bytes(out)

def decrypt_image(key4way: bytes, iv_otp_salt: bytes, iv_public: bytes, ciphertext: bytes) -> bytes:
    return ctr_xor_decrypt(reconstruct_key(key4way), make_iv0(iv_public, iv_otp_salt), ciphertext)

# Convenience: decrypt with a direct 32-byte key (e.g. upstream privateaes.bin)
def decrypt_with_raw_key(aes_key32: bytes, ivsalt16: bytes, iv_public16: bytes, ciphertext: bytes) -> bytes:
    return ctr_xor_decrypt(aes_key32, make_iv0(iv_public16, ivsalt16), ciphertext)

# ---------------------------------------------------------------- self-test
if __name__ == "__main__":
    # NIST FIPS-197 / SP800-38A AES-256 ECB known-answer vector
    k = bytes.fromhex("603deb1015ca71be2b73aef0857d7781"
                      "1f352c073b6108d72d9810a30914dff4")
    pt = bytes.fromhex("6bc1bee22e409f96e93d7e117393172a")
    ct = bytes.fromhex("f3eed1bdb5d2a03c064b5a7e3db181f8")
    got = aes256_encrypt_block(k, pt)
    assert got == ct, (got.hex(), "AES-256 core FAILED")
    print("AES-256 core OK:", got.hex())

    # key4way reconstruction round-trip test
    import os, struct
    target_key = os.urandom(32)
    tk = list(struct.unpack('<8I', target_key))
    shares = [0]*48
    for i in range(8):
        skip = (i//4)*16
        base = i*4 + skip
        s0,s1,s2 = (struct.unpack('<I', os.urandom(4))[0] for _ in range(3))
        s3 = tk[i] ^ s0 ^ s1 ^ s2
        shares[base], shares[base+1], shares[base+2], shares[base+3] = s0,s1,s2,s3
    key4way = struct.pack('<48I', *shares)
    assert reconstruct_key(key4way) == target_key
    print("key4way reconstruction OK")

    # CTR-XOR self-consistency: our encrypt==decrypt (XOR stream) round-trip
    iv_pub = os.urandom(16); salt = os.urandom(16); data = os.urandom(64+5)
    data += b"\x00"*((16 - len(data)%16)%16)   # pad to block
    enc = decrypt_image(key4way, salt, iv_pub, data)   # "encrypt"
    dec = decrypt_image(key4way, salt, iv_pub, enc)    # decrypt
    assert dec == data
    print("CTR-XOR round-trip OK")
    print("ALL SELF-TESTS PASSED")
