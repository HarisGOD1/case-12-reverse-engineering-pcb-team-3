#!/usr/bin/env python3
"""attack5 live demo for Windows: leak the keystream from a physical safe over
USB, with no flash dump and no firmware constants, and prove it is the real key.

Run it from an elevated (Administrator) shell after the safe is unlocked (enter
the PIN on the encoder so the USB disk appears). The script finds the safe disk
by its 1 MiB size, reads the erased tail sectors beyond the storage boundary,
recovers the generator with the tested keystream_leak_demo code, and cross-checks
the result.

Two things the demo makes explicit, because they look confusing on live hardware:

  * The live volume (LBA 0..1407) read over USB is ALREADY plaintext. The
    firmware decrypts every READ(10), so the unlocked disk shows an open FAT12
    directly; the leaked keystream is not applied to it.
  * The value of the leak is a COLD snapshot: a raw image taken without the PIN
    (a BOOTSEL dump). The build's snapshot ships in the repo, so the demo
    decrypts it with the keystream leaked from the live device to prove the two
    share one fixed key (CWE-321).

    python attack5\\src\\win_demo.py                 # auto-detect the disk
    python attack5\\src\\win_demo.py --volume E      # force a drive letter
    python attack5\\src\\win_demo.py --tail-file t.bin  # offline, from a saved tail

The recovery is the same code attack5/README documents and test_keystream_leak_demo
covers. Only the raw Windows read is new here, and it needs the physical device.
"""
import argparse
import hashlib
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, HERE)
import keystream_leak_demo as kld  # noqa: E402  (reuse the tested recovery)

SECTOR = kld.SECTOR
STORAGE_SECTORS = kld.STORAGE_LEN // SECTOR          # 1408: encrypted volume
DISK_SECTORS = kld.DISK_SECTORS                      # 2048: whole MSC disk
TAIL_FIRST = STORAGE_SECTORS                         # first sector past storage
TAIL_COUNT = DISK_SECTORS - STORAGE_SECTORS          # 640 erased/tail sectors
SAFE_DISK_BYTES = DISK_SECTORS * SECTOR              # 1 MiB, the size signature


# --- pure core: tested on Linux against a modelled tail -----------------------

def recover_and_verify(tail, live_volume=None, cold_ciphertext=None):
    """Recover the generator from the USB tail and check it two ways.

    tail: raw bytes of sectors TAIL_FIRST..DISK_SECTORS-1 as the device served
    them. live_volume: sectors 0..STORAGE_SECTORS-1 read over USB from the
    unlocked device -- already plaintext, because the firmware decrypts READ(10),
    so it is checked as-is and never decrypted again. cold_ciphertext: a raw
    snapshot of the same build (a BOOTSEL dump), decrypted with the recovered
    keystream to prove the leak is the real key. Recovery needs neither.
    """
    report = {"recovered": False, "params": None, "keystream_sha256": None,
              "observed_sectors": 0, "live_boot_ok": None,
              "cold_boot_ok": None, "cold_zip_ok": None, "cold_plaintext": None}
    observations = kld.decode_tail_response(tail)
    report["observed_sectors"] = len(observations)
    found = kld.recover_generator_from_leak(observations)
    if found is None:
        return report
    a, seed, b, w = found
    report["recovered"] = True
    report["params"] = {"A": a, "seed": seed, "B": b, "W": w}
    args = kld.R.position_args(STORAGE_SECTORS, a, b, w)
    keystream = kld.R.keystream_from_args(args, seed).tobytes()
    report["keystream_sha256"] = hashlib.sha256(keystream).hexdigest()
    if live_volume is not None and len(live_volume) >= kld.STORAGE_LEN:
        # over USB the firmware already decrypted this; check it as-is
        report["live_boot_ok"] = kld.R.strict_bootsector(live_volume[:kld.STORAGE_LEN])
    if cold_ciphertext is not None and len(cold_ciphertext) >= kld.STORAGE_LEN:
        plaintext = kld.R.decrypt_all(cold_ciphertext[:kld.STORAGE_LEN], a, seed, b, w)
        report["cold_plaintext"] = plaintext
        report["cold_boot_ok"] = kld.R.strict_bootsector(plaintext)
        report["cold_zip_ok"] = kld.R.volume_has_valid_zip(plaintext)
    return report


def format_report(report):
    lines = [f"observed erased tail sectors : {report['observed_sectors']}"]
    if not report["recovered"]:
        lines.append("recovery                     : FAILED "
                     "(no unique generator matched the tail)")
        return "\n".join(lines)
    p = report["params"]
    lines.append("recovery                     : OK (no dump, no firmware constant)")
    lines.append(f"    B    = 0x{p['B']:08X}")
    lines.append(f"    W    = 0x{p['W']:08X}")
    lines.append(f"    A    = 0x{p['A']:08X}")
    lines.append(f"    seed = 0x{p['seed']:08X}")
    lines.append(f"full-volume keystream sha256 : {report['keystream_sha256']}")
    if report["live_boot_ok"] is not None:
        state = "valid (open FAT12)" if report["live_boot_ok"] else "not FAT12"
        lines.append(f"live USB volume, already decrypted by firmware : {state}")
    if report["cold_boot_ok"] is not None:
        boot = "valid" if report["cold_boot_ok"] else "INVALID"
        zipc = "valid" if report["cold_zip_ok"] else "INVALID"
        lines.append(f"cold snapshot decrypted with the leaked keystream : "
                     f"FAT12 {boot}, ZIP CRC {zipc}")
    return "\n".join(lines)


def load_cold_ciphertext():
    """The build's raw snapshot ships in the repo; use it to prove the key."""
    path = os.path.join(REPO_ROOT, "artifacts", "backup_full.bin")
    if not os.path.exists(path):
        return None, None
    with open(path, "rb") as handle:
        dump = handle.read()
    return dump[kld.STORAGE_OFF:kld.STORAGE_OFF + kld.STORAGE_LEN], path


# --- Windows raw disk access: cannot be tested without the device -------------

def _win():
    """Return (kernel32, ctypes, wintypes); import here so this loads off Windows."""
    import ctypes
    from ctypes import wintypes
    if not hasattr(ctypes, "windll"):
        raise OSError("Raw disk access here needs Windows (ctypes.windll)")
    return getattr(ctypes, "windll").kernel32, ctypes, wintypes


def is_admin():
    import ctypes
    try:
        return getattr(ctypes, "windll").shell32.IsUserAnAdmin() != 0
    except Exception:
        return False


def find_safe_volume():
    """Return the drive letter of the removable 1 MiB safe volume, or None."""
    kernel32, ctypes, _ = _win()
    drive_removable = 2
    mask = kernel32.GetLogicalDrives()
    matches = []
    for i in range(26):
        if not (mask >> i) & 1:
            continue
        letter = chr(ord("A") + i)
        root = f"{letter}:\\"
        if kernel32.GetDriveTypeW(root) != drive_removable:
            continue
        total = ctypes.c_ulonglong(0)
        if kernel32.GetDiskFreeSpaceExW(root, None, ctypes.byref(total), None):
            # the safe is a 2048-sector superfloppy; nothing else is this small
            if abs(total.value - SAFE_DISK_BYTES) <= 64 * 1024:
                matches.append(letter)
    if len(matches) == 1:
        return matches[0]
    return None


def read_raw_sectors(volume_letter, first_sector, count):
    r"""Read `count` sectors starting at `first_sector` from \\.\<letter>:.

    Locks the volume so Windows serves raw sectors of a mounted filesystem.
    Falls back to an unlocked read if the lock is refused. Returns the bytes.
    """
    kernel32, ctypes, wintypes = _win()
    generic_read = 0x80000000
    file_share = 0x00000001 | 0x00000002       # READ | WRITE
    open_existing = 3
    invalid_handle = ctypes.c_void_p(-1).value
    fsctl_lock_volume = 0x00090018
    fsctl_unlock_volume = 0x0009001C
    file_begin = 0

    path = f"\\\\.\\{volume_letter}:"
    kernel32.CreateFileW.restype = ctypes.c_void_p
    handle = kernel32.CreateFileW(path, generic_read, file_share, None,
                                  open_existing, 0, None)
    if handle == invalid_handle or handle is None:
        raise OSError(f"cannot open {path} (err {ctypes.get_last_error()}); "
                      "run as Administrator")
    try:
        returned = wintypes.DWORD(0)
        locked = kernel32.DeviceIoControl(ctypes.c_void_p(handle), fsctl_lock_volume,
                                          None, 0, None, 0,
                                          ctypes.byref(returned), None)
        try:
            new_pos = ctypes.c_longlong(0)
            offset = ctypes.c_longlong(first_sector * SECTOR)
            if not kernel32.SetFilePointerEx(ctypes.c_void_p(handle), offset,
                                             ctypes.byref(new_pos), file_begin):
                raise OSError(f"seek failed (err {ctypes.get_last_error()})")
            size = count * SECTOR
            buf = (ctypes.c_char * size)()
            read = wintypes.DWORD(0)
            got = 0
            while got < size:                  # some stacks refuse one large raw read
                step = min(65536, size - got)
                if not kernel32.ReadFile(ctypes.c_void_p(handle),
                                         ctypes.byref(buf, got), step,
                                         ctypes.byref(read), None):
                    raise OSError(f"read failed at +{got} (err {ctypes.get_last_error()})")
                if read.value == 0:
                    break
                got += read.value
            data = bytes(buf[:got])
            if len(data) != size:
                raise OSError(f"short read: {len(data)} of {size} bytes")
            return data
        finally:
            if locked:
                kernel32.DeviceIoControl(ctypes.c_void_p(handle), fsctl_unlock_volume,
                                         None, 0, None, 0,
                                         ctypes.byref(returned), None)
    finally:
        kernel32.CloseHandle(ctypes.c_void_p(handle))


def run_live(volume_letter, out_path):
    print(f"[*] reading erased tail (LBA {TAIL_FIRST}..{DISK_SECTORS - 1}) "
          f"from \\\\.\\{volume_letter}: ...")
    tail = read_raw_sectors(volume_letter, TAIL_FIRST, TAIL_COUNT)
    print(f"    got {len(tail)} bytes")
    print(f"[*] reading live volume (LBA 0..{STORAGE_SECTORS - 1}) over USB ...")
    live = read_raw_sectors(volume_letter, 0, STORAGE_SECTORS)
    print(f"    got {len(live)} bytes (the firmware already decrypted this)")
    cold, cold_path = load_cold_ciphertext()
    if cold_path:
        print(f"[*] cross-check: decrypt the build snapshot {os.path.relpath(cold_path, os.getcwd())}")
    else:
        print("[*] no build snapshot found; skipping the cold cross-check")
    report = recover_and_verify(tail, live_volume=live, cold_ciphertext=cold)
    print(format_report(report))
    if report["cold_plaintext"] is not None and report["cold_zip_ok"]:
        with open(out_path, "wb") as handle:
            handle.write(report["cold_plaintext"])
        print(f"[*] decrypted snapshot written: {out_path}")
        print("    open it with 7-Zip or `mdir` to reach your_prize.zip")
    if report["live_boot_ok"]:
        print(f"[*] the unlocked disk {volume_letter}: is already open FAT12 -- "
              "browse it directly for the files")
    return 0 if report["recovered"] else 1


def run_offline(tail_path, dump_path, out_path):
    with open(tail_path, "rb") as handle:
        tail = handle.read()
    cold = None
    if dump_path:
        with open(dump_path, "rb") as handle:
            dump = handle.read()
        cold = dump[kld.STORAGE_OFF:kld.STORAGE_OFF + kld.STORAGE_LEN]
    else:
        cold, _ = load_cold_ciphertext()
    report = recover_and_verify(tail, live_volume=None, cold_ciphertext=cold)
    print(format_report(report))
    if report["cold_plaintext"] is not None and report["cold_zip_ok"]:
        with open(out_path, "wb") as handle:
            handle.write(report["cold_plaintext"])
        print(f"[*] decrypted snapshot written: {out_path}")
    return 0 if report["recovered"] else 1


def main(argv=None):
    parser = argparse.ArgumentParser(description="attack5 live demo (Windows)")
    parser.add_argument("--volume", help="drive letter of the safe, e.g. E")
    parser.add_argument("--tail-file", help="offline: raw tail bytes instead of a disk")
    parser.add_argument("--dump", help="offline: full dump for the cold cross-check")
    parser.add_argument("--out", default="storage_decrypted.img",
                        help="where to write the decrypted cold snapshot")
    args = parser.parse_args(argv)

    if args.tail_file:
        return run_offline(args.tail_file, args.dump, args.out)

    if os.name != "nt":
        parser.error("live capture needs Windows; use --tail-file off Windows")
    if not is_admin():
        print("[!] not elevated. Raw disk access needs Administrator.\n"
              "    Open cmd/PowerShell as Administrator and run this again.",
              file=sys.stderr)
        return 2
    letter = args.volume or find_safe_volume()
    if not letter:
        print("[!] could not identify the safe disk (a removable 1 MiB volume).\n"
              "    Unlock the safe first (PIN on the encoder), then either let it\n"
              "    auto-detect or pass --volume <letter>.", file=sys.stderr)
        return 2
    return run_live(letter, args.out)


if __name__ == "__main__":
    sys.exit(main())
