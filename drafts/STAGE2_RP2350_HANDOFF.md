# Этап 2 (RP2350) — сводка для передачи другой сессии

Рабочий документ для параллельной сессии. Фиксирует, что известно и какие догадки по новому этапу ЛЦТ 2026 (трек Positive Technologies). Общая память прошлого этапа — [`.claude/memory/lct-2026-usb-token-reverse.md`](../.claude/memory/lct-2026-usb-token-reverse.md), она про прошлогодний RP2040 и к новому железу напрямую не применима

## TL;DR

Новое устройство — RP2350, прошивка в дампе — это апстримный пример `enc_bootloader` из pico-examples, а не прошлогодний `usb_token`. Прошлогодний класс атак (оффлайн-дешифровка из флеш-дампа) закрыт системно: ключ AES-256 вынесен в OTP и залочен, добавлены secure boot и своя подпись. Из одного `firmware_dump.bin` образ приложения расшифровать нельзя — нужен ключ из OTP, а он достаётся только физическим глитчем. Дешифратор под ключ уже готов и проверен

## Устройство и образ

- Чип RP2350 (Cortex-M33, ARMv8-M), плата RP2350 Zero. Маркировка корпуса `RP2350A0A4` = корпус A (QFN-60), stepping **A4**. SRAM 520 КБ (`0x20000000..0x20082000`), XIP-флеш база `0x10000000`, 4 МиБ
- ВАЖНО: stepping A4 (не A2) закрыл challenge-глитчи E16/E20/E21/E24 (OTP wrapper + A4 boot ROM, верифицировано по Raspberry Pi PCN и WOOT'25). Глитч-путь к OTP-ключу недоступен, остаётся только FIB/PVC уровня IOActive
- Serial живого чипа `8295FF712987C1CA`, сейчас в BOOTSEL (`USB 2e8a:000f RP2350 Boot`)
- Прошивка — `enc_bootloader` (pico-examples, ветка master, mbedtls-вариант). SDK 2.3.1, собрано 9 окт 2026, build=Debug, автор по пути сборки — `C:/Users/dsokolov/...`
- Образ исполняется из SRAM: boot ROM копирует `0x10000000..0x10007530` в `0x20070000`, entry `0x20070197`, SP `0x20082000`
- Исходник апстрима (достать: `gh api repos/raspberrypi/pico-examples/contents/bootloaders/encrypted/<file>`): `enc_bootloader.c`, `mbedtls_aes.c`, `enc-pt.json`, `README.md`

## Карта дампа firmware_dump.bin (XIP база 0x10000000)

| Flash off | Что |
| --- | --- |
| `0x0..0x7530` | enc_bootloader (код + vector table + picobin метаблоки на 0x15c, 0x700, 0x7530, 0x75a4) |
| `0x5000..0x7200` | таблицы mbedtls AES (S-box @0x718c, inv-S-box @0x608c, T-table FT0 @0x6d8c) — НЕ ключ |
| `0x8200..0x7ffff` | 0xFF, включая partition A (пустая) |
| `0x80000..0x8cfff` | partition B — зашифрованный образ приложения |
| `0x8d000..0x2fffff` | 0xFF |
| `0x300000..0x3065ff` | непрозрачный блоб 26112 байт (догадка ниже) |
| `0x306600..0x3ffdff` | 0xFF |
| `0x3ffe00..0x3fffff` | ещё блоб 512 байт (догадка ниже) |

Партиции A/B из таблицы: A `0x10000..0x80000`, B `0x80000..0xf0000`. Активна B (A пустая)

## Крипто-схема (подтверждена по исходнику + самотестами)

- AES-256-CTR, 14 раундов, keystream = AES256-ECB-ENC(nonce)
- `iv0 = IV_public XOR IV_OTPsalt` (побайтовый XOR 16 байт)
- Инкремент нестандартный: для блока k `nonce = iv0`, затем `nonce[12..15] ^= big-endian u32(k)` (XOR, не сложение; старшие 12 байт неизменны)
- Ключ — 4-way XOR share в OTP page 29-30 (192 байта: слова 0..15 → key[0..3], слова 16..31 пустышка-дыра FIB, слова 32..47 → key[4..7])
- Соль IV — OTP page 31 (16 байт)
- Дешифратор: `scratchpad/decrypt_rp2350.py` (автономный, свой AES-256, все самотесты прошли). Функции `reconstruct_key`, `make_iv0`, `ctr_xor_decrypt`, `decrypt_image`, `decrypt_with_raw_key`

## Структура зашифрованного образа в partition B (разобрано побайтово)

- start marker `0xffffded3` @ `0x80000`, end marker `0xab123579` @ `0x80010`, слово перед end @ `0x8000c` = `0xc344` (link → last_mb_start)
- публичный IV (16 байт) @ `0x80014` = `921e94597201e138c2747ca1e1bd1b0b`
- шифртекст @ `0x80024`, размер `0xc320` (49952 байта, 3122 блока по 16, кратно 16)
- last block start @ `0x8c344` = `0xffffded3` (сходится)

## OTP (снято с живого устройства, picotool, read-only)

Залочены на чтение (и guarded, и raw → `XXXXXXXX`): page 29, 30 (ключ), 31 (соль), 48, 49 (BOOTKEY-хэши), 62, 63. Остальной OTP читается. Raw-режим лок не обходит

Secure-boot конфиг (CRIT1 = `0x7`):
- `SECURE_BOOT_ENABLE` = 1
- `DEBUG_DISABLE` = 1 (весь debug, включая SWD, закрыт)
- `SECURE_DEBUG_DISABLE` = 1
- `GLITCH_DETECTOR_ENABLE` = 0 ← детекторы глитчей НЕ включены (слабость конфигурации)

BOOT_FLAGS0 = `0x0`: PICOBOOT и MSD открыты, `DISABLE_WATCHDOG_SCRATCH` = 0 (паллиатив против E20 не выставлен). BOOT_FLAGS1 KEY_VALID = 1 (bootkey 0 валиден)

Подпись: ключ подписи заменён на собственный (не демо). Публичный ключ устройства `8B2316D5...`, демо-ключ примера `61826D31...` — разные. OTP_DATA_GUARDED_BASE = `0x40138000` (сверено по SDK; литералы в дизасме `0x40138e80`/`0x40138f80` = page29/31)

## Карта доступов

| Канал | Статус | Обход |
| --- | --- | --- |
| SWD / debug | закрыт (OTP DEBUG_DISABLE=1) | voltage glitch E16 → re-enable debug |
| PICOBOOT / MSD (BOOTSEL) | открыт (им читаем) | не нужен, но ключ залочен |
| OTP page 29/31 (ключ+соль) | закрыт (read-lock + sw_lock) | глитч → debug или ранний код до sw_lock |
| Secure boot | включён, подпись своя | laser E24 / reboot-glitch E20 |
| Glitch-детекторы | выключены | облегчает всё выше |

## Что проверено отрицательно

- Брутфорс ключа/соли — невозможен (AES-256 + 128-бит соль)
- Слабый ГСЧ ключа (`scratchpad/brute_oracle.py`): дефолтные ключи примера, нулевые, FF, паттерны — ни один не даёт правдоподобный RP2350-образ. Ключ — настоящий OTP-секрет
- Ключ/соль как функция публичного chip serial `8295FF712987C1CA` (`brute_serial.py`, сессия 2026-10-16): 16 KDF-кандидатов (sha256/sha512/md5 от serial в обоих порядках байт, с солью/без, с префиксами) — ни один не даёт валидный образ. Ключ не выводится из читаемых данных
- ШИРОКИЙ перебор простого/человеческого ключа (`scratchpad/brute_weak_key.py`, сессия 2026-10-16, точный оракул по keystream[0]): 23934 проверки = 3989 ключей × 6 солей. Классы: ~65 осмысленных строк (dsokolov, Positive Technologies/Labs, LCT2026, rp2350, enc_bootloader, usb_token, CyberSafe, build-путь, даты сборки) как raw/sha256/sha512/md5 и с солями; составные пары токенов с разделителями; байтовые паттерны и все 256 константных ключей; демо-ключ as-is/reversed/xor; оба chip-id. Соли: zero, 0xFF, демо `8cb6c678ecdb25dc...`, serial×2, IV_public. НИ ОДНА не совпала. Ключ — настоящий случайный OTP-секрет, не человеческий ввод
- Второй 64-бит идентификатор в OTP rows 0x04-0x07: `179764F532D29850` (вероятно ECC/второй chip-id; serial `8295FF712987C1CA` — rows 0x00-0x03). Как ключ/соль/материал KDF — не подходит
- Reuse keystream между областями НЕТ: `XOR(ct_partitionB, blob@0x300000)` даёт энтропию 7.96 (не низкую) — keystream не общий; `blob XOR keystream[0]` и `tail XOR keystream[0]` = мусор. Блоб `0x300000` и хвост `0x3ffe00` своего IV-заголовка не имеют, тем же keystream не шифрованы — либо другой ключ/IV, либо случайны; статистикой (энтропия ≈8, без ECB-повторов) не различить
- OTP page 29/30 (ключ) и 31 (соль) подтверждены сплошными `XXXXXXXX` и в guarded, и в raw дампе — секрета в снятом OTP нет, эмуляция загрузчика его не материализует
- Known-plaintext атака на CTR (сессия 2026-10-16): с живого устройства через CWE-209 сняты первые 16 байт РАСшифрованного образа = vector table приложения `SP=0x20082000, reset=0x20000197, NMI=0x2000011b, HF=0x2000011d` (база 0x20000000 = SRAM_BASE, offset'ы векторов идентичны загрузчику → тот же Pico SDK crt0). `keystream[0] = CT^PT = 4a5e4990 642bb182 72952f73 b9854d09 = AES_ENC(key, iv0)`. Расшифровку НЕ продвигает: в CTR каждый блок = AES(key, свой nonce), keystream блока 0 не даёт keystream блоков 1+ без ключа. Скрипт `scratchpad/brute_serial.py` и расчёт keystream — в scratchpad сессии. Ценность — доказательство CWE-209 на живом железе
- Подпись демо-ключом — нет, ключ заменён
- Чтение ключа через picotool raw/guarded — залочено
- Прошлогодний умный брутфорс (восстановление малого состояния LCG по known-plaintext) — закрыт переходом на AES

## Векторы (реальные остаточные)

1. Voltage glitch E16 (Cullen) — рекомендуется: детекторы выключены. Нужно: глитчер (ChipWhisperer или crowbar MOSFET+FPGA, <$1000), изоляция рельса `USB_OTP_VDD`, SWD-адаптер, осциллограф. Просадка ~50 мкс после PSM guard-read → re-enable debug → читать OTP page 29-31 → офлайн-дешифровка
2. Reboot-API glitch E20 (Muench) — свой код до sw_lock, payload = key-dumper OTP→UART. `DISABLE_WATCHDOG_SCRATCH`=0 → уязвимо
3. Laser FI E24 (Courdesses) — дорого, decap, не первый выбор
4. UART debug-утечка (CWE-209) — Debug-сборка печатает в UART GP0 первые 16 байт РАСШИФРОВАННОГО образа (`Post decryption image begins with`). Без ключа, но только 16 байт + known-plaintext
5. Конфиг-слабость — glitch-детекторы off, watchdog-scratch паллиатив не выставлен, PICOBOOT открыт: устройство оставлено максимально податливым к глитчам

Перепаять OTP НЕЛЬЗЯ — это antifuse внутри die RP2350, не отдельный чип (в отличие от прошлогодней внешней QSPI-флеш). Физический доступ к OTP — только decap+FIB/PVC (IOActive-уровень) или глитч по питанию

## Догадки (не подтверждены)

- Блок `0x300000` (26112 байт) — не образ (нет picobin-маркеров, не грузится загрузчиком), не копия B, не ECB, не reuse keystream с B. Догадка: зашифрованное хранилище данных приложения (аналог прошлогоднего скрытого хранилища usb_token) либо наживка. Без ключа неотличим от случайного
- Хвост `0x3ffe00` (512 байт, случайные) — догадка: сохранённое состояние (ключ/соль/счётчик/хеш). Проверить, читает ли код XIP-адрес `0x103ffe00`
- PIN/энкодер прошлого года — в коде загрузчика их НЕТ (ни IO_BANK0, ни чтения GPIO-входов, ни таблиц сегментов). Если PIN есть, он в зашифрованном приложении partition B, в крипту не входит
- Внутреннее приложение — вероятно тот же `usb_token` или демо `hello_serial_enc`; выяснится только после расшифровки

## Открытые вопросы для следующей сессии

- ~~Stepping A2 или A4~~ ПОДТВЕРЖДЁН A4 (с железа, сессия 2026-10-16): voltage/reboot/laser-глитчи E16/E20/E21/E24 закрыты фиксом boot ROM + OTP-враппера (по PCN + WOOT'25, первоисточники в этой сессии не перепроверялись). Путь к OTP-ключу — только инвазивный FIB/PVC уровня IOActive. Глитч-стенд на эмуляторе для этих векторов смысла не имеет
- ~~Что в блоке `0x300000` и хвосте `0x3ffe00`~~ ЗАКРЫТО Ghidra-разведкой: код загрузчика их не читает (см. раздел ниже)
- ~~Сверить дизасм загрузчика с чистым апстримом на предмет модификаций/бэкдора~~ ЗАКРЫТО: прикладной код побайтно = апстрим, бэкдора нет (см. ниже)
- Снять свежий `picotool save` и побайтово сверить с `firmware_dump.bin`

## Ghidra-разведка (сессия 2026-10-16)

Программа `firmware_dump.bin` в общем Ghidra-проекте (versioned), язык `ARM:LE:32:Cortex`, один блок `ram 0x10000000..0x103fffff`. Разметка — только метаданные, байты образа не тронуты

Адресация (важно для любого, кто откроет проект): образ слинкован под исполнение из SRAM `0x20070000` (reset `0x20070197`, SP `0x20082000`), а в дампе лежит по базе flash `0x10000000`. Смещение ровно `0x10070000`. Поэтому прямые `BL`-вызовы и чтения XIP-флеша `0x10xxxxxx` разрешаются корректно, а абсолютные литералы-указатели `0x2007xxxx` (vector table, адреса функций и строк) указывают на SRAM и в этой раскладке "висят". Авто-граф раскручен вручную: создана функция по reset `0x10000196` и три crt0-указателя из его литерального пула, затем авто-анализ (101 функция). PyGhidra недоступен (Ghidra не под PyGhidra), поэтому byte-mapped блок SRAM не создавался

Переименованы и прокомментированы (flash-адрес → имя):

| Flash | Имя | Что делает |
| --- | --- | --- |
| `0x10000196` | `reset_handler` | vector[1], Thumb entry |
| `0x1000046c` | `main` | суперлогика загрузчика, см. ниже |
| `0x100007ce` | `decrypt` | реконструкция ключа из OTP + запуск AES-CTR |
| `0x1000074c` | `mb_aes_crypt_ctr_xor` | AES-256-CTR c нестандартным инкрементом nonce |
| `0x100040f4` | `mbedtls_aes_crypt_ecb` | блочный AES-256 (генератор keystream) |
| `0x100037c8` | `mbedtls_aes_setkey_enc` | key schedule AES-256 |
| `0x1000045c` | `lock_key` | блокировка OTP-страниц ключа (sw_lock 29/30) |
| `0x10000214` | `rom_get_partition_table_info` | ROM API |
| `0x10000270` | `rom_load_partition_table` | ROM API |
| `0x10000304` | `rom_get_boot_info` | ROM API |
| `0x1000033c` | `rom_chain_image` | ROM API, передача управления образу |
| `0x10001b6c` | `rom_pick_ab_partition_during_update` | ROM API, выбор A/B |
| `0x10001b32` | `reset_usb_boot` | ROM API, сброс в BOOTSEL при ошибке |
| `0x1000452c` | `memcpy` | копирование шифртекста в SRAM |
| `0x10003438` / `0x100036dc` / `0x1000375c` | `stdio_init_all` / `stdio_uart_init` / `stdio_uart_deinit` | stdio |
| `0x10003442` / `0x100034a0` | `puts` / `printf` | отладочный вывод |

Главные выводы:

- **Прошивка — немодифицированный `enc_bootloader` из pico-examples (mbedtls-вариант).** `main`, `decrypt`, `mb_aes_crypt_ctr_xor`, `mbedtls_aes_crypt_ecb`, `lock_key` сверены с апстримным исходником по декомпиляции — логика совпадает побайтно. Авторских модификаций и бэкдора в коде загрузчика нет. Ключ реально читается из OTP guarded (`0x40138E80` page 29, `0x40138F80` page 31), в коде его нет
- **CWE-209 подтверждён прямо в `main`**: `puts("Pre decryption image begins with")` + 4× `printf("%08x")` печатает первые 16 байт ЗАшифрованного образа, а `puts("Post decryption image begins with")` + 4× `printf("%08x")` — первые 16 байт РАСшифрованного образа приложения, в UART. Это 16 байт открытого текста плюс known-plaintext (vector table приложения). Совпадает с вектором 4
- **Блоб `0x300000` и хвост `0x3ffe00` загрузчиком не используются**: в кодовом сегменте нет ни одного выровненного литерала `0x10300000` / `0x103ffe00`, а `main` читает только активную партицию B (`data_start` берётся из partition table через ROM API, не хардкодом). Единственное `#0x300000` в дизасме — арифметическая маска в `orr`, не адрес. Догадка "наживка либо данные приложения" подтверждается со стороны загрузчика
- `runtime_init_clocks` (переопределение weak SDK, настройка ROSC/PLL) присутствует, но декомпилятор местами спотыкается на висячих ссылках; детального разбора не требует для текущих векторов

Статус сохранения: имена и комментарии записаны в рабочую копию программы (save прошёл), но VCS check-in через MCP упал — "Not connected to repository server". Для передачи команде нужен Check In через Ghidra GUI при подключённом сервере (та же картина, что на прошлом этапе)

## Артефакты в scratchpad сессии

## Артефакты в scratchpad сессии

Путь: `/tmp/claude-1000/-home-rokokol-Projects-case-12-reverse-engineering-pcb-team-3/8be6f7fa-ebde-4316-bb1a-a1d27afe7097/scratchpad/`
- `decrypt_rp2350.py` — дешифратор AES-256-CTR + самотесты
- `brute_oracle.py` — оракул слабого ГСЧ (негативный результат)
- `default_privateaes.bin` / `default_ivsalt.bin` — дефолтные ключи примера (сырой git-blob, 32+16 байт)
- `live_otp_dump.txt` / `live_otp_raw.txt` — дампы OTP с живого устройства
- `master/` — исходники апстрима (ветка master, авторитет схемы)
- `upstream/` — исходники ветки enc-bootloader-docs (asm-вариант aes.S, не наш)
- `boot_disasm.txt` — дизасм загрузчика (objdump, база 0x20070000)

## Крипто-библиотека и вывод по side-channel (сессия slave2, 2026-10-16)

- **mbedtls 3.6.6** — pico-sdk 2.3.1 пинит сабмодуль `lib/mbedtls` точно на коммит `0bebf8b8c7f07abe3571ded48a11aa907a1ffb20` (2026-03-26), sha совпадает с тегом `v3.6.6` побайтно. Не промежуточный коммит
- Конфиг `bootloaders/encrypted/mbedtls_config.h`: `MBEDTLS_AES_C`, `MBEDTLS_AES_ROM_TABLES`, `MBEDTLS_CIPHER_MODE_CTR`, `MBEDTLS_HAVE_ASM`. НЕТ `MBEDTLS_AES_FEWER_TABLES`, `MBEDTLS_AES_USE_HARDWARE_ONLY`, ни одного `*_ALT`. Табличный программный AES без constant-time и без SCA-контрмер. Используется только encrypt-путь (`mbedtls_aes_crypt_ecb(..., MBEDTLS_AES_ENCRYPT, ...)`), decrypt mbedtls не зовётся
- CVE 3.6.6 по AES/setkey/side-channel: CVE-2025-52496, CVE-2025-59438, CVE-2025-54764 пофикшены ДО 3.6.6 и по сценарию неприменимы (нет x86/AESNI/многопоточности, нет padding, нет RSA). CVE-2025-66442 формально покрывает 3.6.6, но требует clang/LLVM с select-optimize (тулчейн — `arm-none-eabi-gcc`) И decrypt-путь CBC/ECB (не вызывается) — неприменима ни по тулчейну, ни по use-case
- **Программный side-channel (cache-timing, Bernstein) к ключу НЕ ведёт**: Cortex-M33 без data cache (нет hit/miss-канала), атакующий не исполняет свой код (SWD закрыт, A4 закрыл boot ROM), AES крутится при загрузке и не оракул для множества запросов. Это честный "нет" по ПРОГРАММНОМУ каналу
- **Физический CPA по питанию — НЕ закрыт этим выводом и остаётся главным крипто-путём**: табличный `mb_aes_crypt_ctr_xor` крутит 3122 блока одним ключом с варьирующимся счётчиком → материал для корреляции. A4 закрыл глитчи, но не power-SCA; `GLITCH_DETECTOR_ENABLE`=0 и отсутствие SCA-контрмер облегчают захват. CWE-1300. В отчёт включён как вектор В2
- Поправка к CWE anti-rollback: правильный класс — **CWE-1328** "Security Version Number Mutable to Older Versions" (не CWE-1329 = "Reliance on Component That is Not Updateable"). В отчёте — вектор В8
