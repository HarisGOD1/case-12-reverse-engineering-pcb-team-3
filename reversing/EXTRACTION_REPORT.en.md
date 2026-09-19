# RP2040 Flash Dump → Program Extraction & Decompilation

## Source (Ghidra Server)

- **Server:** `ghidra://157.228.183.38:13100`
- **Repository:** `pcb-team-3`
- **File:** `backup_full.bin` (content type `Program`, version 2)
- **Program language:** `ARM:LE:32:Cortex`, mapped at base `0x10000000`
- Repo has exactly **one** program file (the full flash dump).

## Step 1 — Memory → Program (carving)

`DumpMemoryMap.java` exported the whole initialized memory of the imported program.

| Artifact | Offset range | Size | Meaning |
|---|---|---|---|
| `memory_full.bin` | — | **0x200000 (2 MiB)** | Complete flash image ("memory") |
| `program.bin` | `0x0000 – 0x6FFF` | **0x7000 (28 KiB)** | The flashed **program** (boot2 + app + rodata) |
| `app.bin` | `0x0100 – 0x6FFF` | 0x6F00 (28,416 B) | Application only (vector table onward, base `0x10000100`) |
| `flash_data_blob.bin` | `0x100000 – 0x1AF5FF` | 0xAF600 (718 KiB) | High-entropy data region (separate from code) |

### Flash map (from `memmap.txt` + analysis)

```
0x000000 - 0x0000FF  boot2 (RP2040 stage2 flash loader)      <- program
0x000100 - 0x0001FF  vector table + reserved                 <- program
0x000200 - 0x006FFF  .text / .rodata / data                  <- program
0x007000 - 0x0FFFFF  erased (0xFF)
0x100000 - 0x1AF5FF  high-entropy blob (NOT ARM code)        <- data
0x1AF600 - 0x1FFFFF  erased (0xFF)
```

**Carving rule used:** the program is the contiguous non-erased region at the
bottom of flash (`0x0000–0x6FFF`); everything after the first 0xFF gap is not
executable program code. The program's last code address is `0x10006708`; the
last used byte is at `0x6FFF` (page-aligned), then flash is erased.

### Vector table (RP2040, ARMv6-M)

```
Initial SP      = 0x20042000
Reset_Handler   = 0x100001F7  (Thumb, LSB=1)
NMI             = 0x100001CB
HardFault       = 0x100001CD
```

### boot2 integrity note

- Stored CRC32 @ `0xFC` = `0x7A4EB274`
- Computed CRC32 over `0x00..0xFB` = `0xD4A3FD6A` → **MISMATCH**
- The boot2 image has been **modified** (stock W25Q080 boot2 would validate).

## Step 2 — Decompile with Ghidra

`DecompileAll.java` imported `program.bin` into a fresh local project with
`-processor ARM:LE:32:Cortex -loader-baseAddr 0x10000000` and decompiled every
function.

- **155 / 155 functions decompiled successfully** → `program_decompiled.c` (5,690 lines)
- `program_summary.txt`: full function inventory + 16 defined strings

### Program identification (skills applied)

Using `armv8-reversing` + `firmware-analysis`:

- This is a **pico-SDK + TinyUSB + FatFs USB Mass Storage ("flash disk")** firmware.
- Strings: `POSILABS`, ` FLASH MSC `, `PositiveLabs`, `RP2040 Flash MSC`,
  `123456`, `Flash Disk`, `usb_token`, plus TinyUSB panic/endpoint messages.
- **`main` = `FUN_1000089c`** — calls GPIO init (`FUN_100050e0`), then loops
  `FUN_10003858` (TinyUSB device task) + `FUN_100009a4` (timed GPIO/LED work).
- `FUN_10000000` = pico-SDK reset/CRT runtime; `FUN_10000ab0` = GPIO function-select init.
- `FUN_10005104` = `memmove`, `FUN_100051c8` = `strlen`, `FUN_10004f60` = FatFs path/alloc helper.

### Security observations (firmware-update-security skill)

- No asymmetric signature / SHA verification code path was found in the
  application: **no authenticity or integrity gate** on the exposed flash.
- The device exposes its own flash over USB MSC ("Flash Disk"), so the flash
  contents are writable from the host with **no update-authentication check**.
- `123456` appears in the TinyUSB string-descriptor table (offset `0x6734`) —
  a weak/hardcoded identifier (default PIN/label).
- The 718 KiB high-entropy blob at `0x100000` is not ARM code; likely the
  filesystem/encrypted payload area and the next thing worth reversing.

## Reproduction

```bash
# 1. export memory from the server program (read-only)
GHIDRA_JAVA_OPTIONS="-Duser.name=core_gemeni" analyzeHeadless \
  ghidra://157.228.183.38:13100/pcb-team-3 -connect core_gemeni -p -readOnly \
  -noanalysis -scriptPath ghidra_scripts -postScript DumpMemoryMap.java \
  -process backup_full.bin

# 2. carve program (0x0000-0x6FFF) and import + decompile
analyzeHeadless /tmp/ghproj prog -import program.bin \
  -processor ARM:LE:32:Cortex -loader-baseAddr 0x10000000 \
  -scriptPath ghidra_scripts -postScript DecompileAll.java
```
