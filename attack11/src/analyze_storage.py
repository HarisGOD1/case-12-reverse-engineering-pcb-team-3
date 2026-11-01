#!/usr/bin/env python3
"""Print the FAT12 layout of the storage image that the app serves over USB MSC.

The image is the decrypted view of the on-disk volume, so the fields here describe
what the device hides behind its sector cipher.
"""

import argparse
import struct
from pathlib import Path


def fat12(fat, cluster):
    value = struct.unpack_from("<H", fat, (cluster * 3) // 2)[0]
    return (value >> 4) if cluster & 1 else (value & 0xFFF)


def entries(img, offset, count):
    out = []
    for i in range(count):
        entry = img[offset + i * 32: offset + (i + 1) * 32]
        if entry[0] == 0x00:
            break
        if entry[0] == 0xE5:
            out.append(("deleted", entry[1:11]))
        elif entry[11] & 0x0F != 0x0F:
            size = struct.unpack_from("<I", entry, 28)[0]
            out.append(("file" if not entry[11] & 0x10 else "dir", entry[0:11], size))
    return out


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", help="raw 1 MiB storage image")
    args = parser.parse_args()

    img = Path(args.image).read_bytes()
    bs = img[:512]
    bps = struct.unpack_from("<H", bs, 11)[0]
    spc = bs[13]
    reserved = struct.unpack_from("<H", bs, 14)[0]
    fats = bs[16]
    root_count = struct.unpack_from("<H", bs, 17)[0]
    total = struct.unpack_from("<H", bs, 19)[0] or struct.unpack_from("<I", bs, 32)[0]
    spf = struct.unpack_from("<H", bs, 22)[0]
    serial = struct.unpack_from("<I", bs, 39)[0]

    fat_off = reserved * bps
    root_off = (reserved + fats * spf) * bps
    data_off = root_off + root_count * 32
    cluster_bytes = bps * spc
    clusters = (len(img) - data_off) // cluster_bytes
    fat = img[fat_off: fat_off + spf * bps]

    print(f"size            : {len(img)} bytes ({len(img)//bps} sectors)")
    print(f"oem             : {bs[3:11].decode('ascii', 'replace')}")
    print(f"fs type         : {bs[54:62].decode('ascii', 'replace').strip()}")
    print(f"label           : {bs[43:54].decode('ascii', 'replace').strip() or '(none)'}")
    print(f"serial          : 0x{serial:08X} -> {serial & 0xFFFF:04X}-{serial >> 16:04X}")
    print(f"geometry        : {bps} B/sector, {spc} sector/cluster, {reserved} reserved, {fats} FATs, {spf} sectors/FAT, {root_count} root entries")
    print(f"layout          : FAT 0x{fat_off:x}, root 0x{root_off:x}, data 0x{data_off:x}, {clusters} clusters")
    print(f"root            : {entries(img, root_off, root_count)}")

    used = [c for c in range(2, clusters + 2) if fat12(fat, c) != 0]
    print(f"allocated       : {len(used)} clusters {[(c, hex(fat12(fat, c))) for c in used[:8]]}")

    svi = data_off + (2 - 2) * cluster_bytes
    print(f"cluster 2       : {entries(img, svi, 16)}")

    zero = sum(1 for b in img if b == 0)
    erased = sum(1 for b in img if b == 0xFF)
    print(f"byte census     : zero {zero}, 0xFF {erased}, other {len(img) - zero - erased}")
    print("note            : free clusters hold the sector cipher applied to erased 0xFF, so they look random while the FAT marks them free")


if __name__ == "__main__":
    main()
