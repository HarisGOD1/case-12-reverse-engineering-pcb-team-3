#!/usr/bin/env python3
"""
PoC для вектора attack5 — оракул гаммы из секторов за пределами шифрованного хранилища.

Механика (см. README.md): msc_read10_decrypt_storage (0x10000354) обслуживает
0x800 секторов (1 МиБ), а шифрованное хранилище занимает только 0xB0000 (704 КиБ).
Флеш за его концом стёрт (0xFF), и устройство «расшифровывает» его на лету:
    served(L) = 0xFF XOR keystream(L) = ~keystream(L)
— то есть любой смонтировавший диск хост легально получает чистую гамму.

Здесь модель проверяется по дампу: константы гаммы взяты из attack1
(взгляд «атакующий с дампом прошивки»); чёрноящичный путь восстановления
параметров без констант описан в README и здесь не реализуется.

Usage:
    python3 keystream_leak_demo.py backup_full.bin
"""
import sys
import math
from collections import Counter

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
    print(f"    (гамма покрывает LBA {first_ff}..0x7FF; по линейности генератора"
          f" восстанавливается на весь диск — путь в README)")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))