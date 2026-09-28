# RP2040 program extraction and decompilation

This report records how the team isolated program bytes from the [full flash dump](../artifacts/backup_full.bin). [Attack 0](../attack0/README.md) records how the team read the flash from the board

## Flash layout

The raw image spans two MiB. Ghidra maps it at `0x10000000` as `ARM:LE:32:Cortex`. [`memmap.txt`](memmap.txt) records the memory export

| Flash offset | Content |
| --- | --- |
| `0x000000–0x0000FF` | boot2 flash loader |
| `0x000100–0x006FFF` | vector table, application code, and constants |
| `0x007000–0x0FFFFF` | erased flash |
| `0x100000–0x1AF5FF` | encrypted storage data |
| `0x1AF600–0x1FFFFF` | erased flash |

The vector table holds SP `0x20042000` and reset vector `0x100001F7`. The low bit of the reset vector selects Thumb mode

The boot2 CRC at `0xFC` is `0x7A4EB274`. CRC-32/MPEG-2 over bytes `0x00–0xFB` gives the same value. `zlib.crc32` uses different parameters and gives `0xD4A3FD6A`

## Program artifacts

| File | Content |
| --- | --- |
| [`program.bin`](program.bin) | first `0x7000` flash bytes, with boot2, application code, and constants |
| [`app.bin`](app.bin) | application bytes from offset `0x100`, with load base `0x10000100` |
| [`program_decompiled.c`](program_decompiled.c) | decompiled program functions |
| [`program_summary.txt`](program_summary.txt) | function and string inventory |

[`scripts/DumpMemoryMap.java`](scripts/DumpMemoryMap.java) exports initialized Ghidra memory. [`scripts/DecompileAll.java`](scripts/DecompileAll.java) imports the program range and decompiles its functions

## Findings and limits

The firmware uses Pico SDK and TinyUSB. USB MSC exposes the encrypted storage volume after PIN entry. It does not expose the firmware area as a writable MSC volume

The USB string `123456` is a serial number, not a PIN. [Attack 8](../attack8/README.md) identifies the PIN comparison bytes. [Attack 1](../attack1/README.md) verifies the storage cipher and FAT12 output

The absence of a firmware signature check matters to [attack 6](../attack6/README.md). BOOTSEL provides a separate flash write path, and the attack 6 emulator stand tests modified program behavior
