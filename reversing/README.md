# Реверс прошивки — выделение программы из дампа и декомпиляция

Дополняет отчёты [`attack1/`](../attack1/README.md), [`attack2/`](../attack2/README.md),
[`attack3/`](../attack3/README.md). Здесь — как из полного дампа флеша
(`backup_full.bin`, 2 МиБ) выделен **код самой программы** и как он
**декомпилирован в Ghidra**, плюс независимая проверка цепочки атаки.

## Источник

- Ghidra Server: `ghidra://157.228.183.38:13100`, репозиторий **`pcb-team-3`**
- Файл: `backup_full.bin` (`ARM:LE:32:Cortex`, база `0x10000000`), версия 3
  (переименования функций от `rokokol`, ReVa)

## Память → программа

Экспорт всей инициализированной памяти (`scripts/DumpMemoryMap.java`) даёт образ
2 МиБ. Раскладка:

```
0x000000 - 0x0000FF  boot2 (RP2040 stage2 flash loader)
0x000100 - 0x0001FF  таблица векторов + резерв
0x000200 - 0x006FFF  .text / .rodata / данные          <- ПРОГРАММА
0x007000 - 0x0FFFFF  стёрто (0xFF)
0x100000 - 0x1AF5FF  шифрованное хранилище (FAT12)     <- данные (см. attack1)
0x1AF600 - 0x1FFFFF  стёрто (0xFF)
```

Таблица векторов: SP=`0x20042000`, Reset_Handler=`0x100001F7` (Thumb, LSB=1).

**Артефакты:**

| Файл | Диапазон | Размер | Что это |
|---|---|---|---|
| `program.bin` | `0x0000–0x6FFF` | 28 КиБ | программа целиком (boot2 + приложение + rodata) |
| `app.bin` | `0x0100–0x6FFF` | 28 416 Б | только приложение (база `0x10000100`) |
| `program_decompiled.c` | — | 154 КБ | декомпилированный C (155 функций) |

**Замечание:** CRC boot2 не сходится — в `0xFC` лежит `0x7A4EB274`, а расчёт по
байтам `0x00..0xFB` даёт `0xD4A3FD6A`. То есть boot2 **модифицирован** (штатный
W25Q080-загрузчик CRC бы прошёл).

## Декомпиляция

`scripts/DecompileAll.java` импортирует `program.bin` в новый локальный проект
(`-processor ARM:LE:32:Cortex -loader-baseAddr 0x10000000`) и декомпилирует всё:

- **155 / 155 функций** успешно → `program_decompiled.c` (5690 строк)
- инвентарь функций и строк → `program_summary.txt`

Идентификация (скиллы `armv8-reversing`, `firmware-analysis`): это прошивка на
**Pico SDK + TinyUSB + FatFs**, USB Mass Storage («Flash Disk»). `main` =
`0x1000089c` (цикл `tud_task` + `app_led_task`).

Подтверждение переименований из общего проекта (`scripts/DumpNamed.java`):

```
100002e8 tud_msc_inquiry_cb
10000354 msc_read10_decrypt_storage
1000053c msc_write10_encrypt_storage
1000089c main
100008d4 tud_descriptor_string_cb
100009a4 app_led_task
10000ab0 board_gpio_init
10003858 tud_task_ext
10004e64 tud_init
100050e0 runtime_init
```

## Проверка цепочки атаки (воспроизведено)

1. `attack1/decrypt_storage.py backup_full.bin storage_decrypted.img` → том FAT12,
   OEM `MSDOS5.0`, подпись `55 aa` — совпадает с отчётом.
2. В корне тома — `your_prize.zip` (691 868 Б).
3. `your_prize.zip` → `layers.zip` → **1337 вложенных zip с AES** (метод 99),
   пароль слоя = его номер → `prize.html` («Ты открыл 1337 слоёв!» + GIF-рикролл).
   Все 1337 слоёв сняты `7z`.

## Воспроизведение

```bash
# 1. экспорт памяти из общего проекта (только чтение)
GHIDRA_JAVA_OPTIONS="-Duser.name=core_gemeni" analyzeHeadless \
  ghidra://157.228.183.38:13100/pcb-team-3 -connect core_gemeni -p -readOnly \
  -noanalysis -scriptPath scripts -postScript DumpMemoryMap.java \
  -process backup_full.bin

# 2. выделить program.bin (0x0000-0x6FFF), импортировать и декомпилировать
analyzeHeadless /tmp/ghproj prog -import program.bin \
  -processor ARM:LE:32:Cortex -loader-baseAddr 0x10000000 \
  -scriptPath scripts -postScript DecompileAll.java
```

Англоязычная версия отчёта — [`EXTRACTION_REPORT.en.md`](EXTRACTION_REPORT.en.md).
