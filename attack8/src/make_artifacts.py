#!/usr/bin/env python3
"""Generate attack8 artifacts from the live UART boot-log capture.

Single source of truth: the captured enc_bootloader UART log. This script
parses the leaked "Pre/Post decryption image begins with" words, validates the
pre-decryption words against the real firmware dump (proving the live device
runs the image we analysed), and writes the binary artifacts into out/.

Usage (from repo root):
    python3 attack8/src/make_artifacts.py [path/to/boot_capture.txt]
If no path is given, reads attack8/out/boot_capture.txt (the committed copy).
"""
import sys, re, os, struct, shutil

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
OUT = os.path.join(REPO, "attack8", "out")
DUMP = os.path.join(REPO, "firmware_dump.bin")
CT_OFF = 0x80024  # ciphertext start of partition B (verified)

def words_after(log, tag):
    i = log.find(tag)
    if i < 0:
        return None
    return [int(x, 16) for x in re.findall(r'[0-9a-fA-F]{8}', log[i + len(tag):])[:4]]

def pack_le(words):
    return b"".join(struct.pack("<I", w) for w in words)

def main():
    src = sys.argv[1] if len(sys.argv) > 1 else os.path.join(OUT, "boot_capture.txt")
    log = open(src, errors="replace").read()

    pre = words_after(log, "Pre decryption image begins with")
    post = words_after(log, "Post decryption image begins with")
    if not pre or not post:
        sys.exit("ERROR: could not find pre/post decryption words in log")

    ct_head = pack_le(pre)    # first 16 bytes of ciphertext (from UART)
    pt_head = pack_le(post)   # first 16 bytes of DECRYPTED image (the leak)

    # Validate against the real dump: UART pre-decryption must equal the dump ciphertext.
    with open(DUMP, "rb") as f:
        f.seek(CT_OFF)
        dump_ct = f.read(16)
    ok = (dump_ct == ct_head)
    print(f"ciphertext head (UART) : {ct_head.hex()}")
    print(f"ciphertext head (dump) : {dump_ct.hex()}")
    print(f"match with dump @0x{CT_OFF:x}: {'YES' if ok else 'NO'}")
    if not ok:
        sys.exit("ERROR: UART ciphertext does not match dump - aborting")

    print(f"decrypted image head   : {pt_head.hex()}")
    sp, reset = post[0], post[1]
    print(f"  initial SP   = 0x{sp:08x}")
    print(f"  reset vector = 0x{reset:08x}")

    os.makedirs(OUT, exist_ok=True)
    # keep a committed copy of the raw log next to the artifacts
    dst_log = os.path.join(OUT, "boot_capture.txt")
    if os.path.abspath(src) != os.path.abspath(dst_log):
        shutil.copyfile(src, dst_log)
    open(os.path.join(OUT, "decrypted_app_head.bin"), "wb").write(pt_head)
    open(os.path.join(OUT, "ciphertext_head.bin"), "wb").write(ct_head)
    print(f"wrote: {OUT}/decrypted_app_head.bin (16 B), ciphertext_head.bin (16 B), boot_capture.txt")

if __name__ == "__main__":
    main()
