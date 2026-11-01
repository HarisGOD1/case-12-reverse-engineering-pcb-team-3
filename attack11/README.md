# attack11 — подмена корня доверия в OTP и чтение ключа AES-256 из Secure-домена

## Суть

Устройство на RP2350 запускает приложение `usb_token` через `enc_bootloader`, ключ AES-256 лежит в OTP pages 29–30, соль — в page 31. Вендор закрыл эти страницы так, что читать их может только Secure-домен (`0x3d00`: Secure read-only, Non-secure и bootloader inaccessible) — иначе их же загрузчик не расшифровал бы образ. При этом свободные слоты boot key в OTP остались не помечены `KEY_INVALID`, а PICOBOOT в `BOOT_FLAGS0` не выключен

Атакующий с доступом к USB в режиме BOOTSEL записывает отпечаток своего ключа в слот `BOOTKEY1`, выставляет бит `KEY_VALID`, загружает свой подписанный Secure-образ в SRAM через PICOBOOT и читает страницы 29–31 тем же правом, которым пользуется загрузчик. Флеш не меняется, а после сброса устройство снова грузит вендорский образ

## Почему это работает

| Факт | Значение | Источник |
| --- | --- | --- |
| `KEY_VALID` до опыта | `0x1`, слот 0 занят вендорским ключом | строка `0x4b` |
| `KEY_INVALID` | `0x0`, слоты 1–3 не запрещены | строка `0x4b` |
| Слоты `BOOTKEY1..3` | все нули | строки `0x90–0xbf` |
| Лок страниц 1 и 2 | `0x0400`, bootloader — read-write | строки `0xf83`, `0xf85` |
| `BOOT_FLAGS0` | `0x0`, PICOBOOT и MSD открыты | строка `0x48` |
| Гейт по secure boot в `otp_access` | отсутствует, проверяются только локи и запрет сброса битов | `pico-bootrom-rp2350/src/main/arm/varm_otp.c` |
| Лок страниц 29–31 | `0x3d00`, Secure read-only | снято дампером из Secure-домена |
| Чтение ключа загрузчиком | `OTP_DATA_GUARDED_BASE` из Secure-кода на каждом запуске | `enc_bootloader.c`, дизассемблер |

Датащит сам называет дыру: "mark any boot key slots you don't intend to use as KEY_INVALID, so that spurious keys can not be installed at a later time" (§10.5). Обе половины этого условия на устройстве были открыты

## Живое подтверждение

1. Запись отпечатка: `picotool otp load attack11/out/otp_enroll_slot1_full.json`, один файл с отпечатком и битом `KEY_VALID`. Диф с baseline — ровно 19 строк: `0x4b–0x4d` перешли из `0x000001` в `0x000003`, строки `0x90–0x9f` получили отпечаток, остальное не изменилось (`attack11/out/otp_before_enrollment.txt` против `attack11/out/otp_after_enrollment.txt`)
2. Загрузка своего образа: `picotool load -v -x` подписанного RAM-образа дампера. Bootrom проверил подпись по новому слоту и передал управление, флеш не тронут
3. Чтение OTP из Secure-домена: страницы 29–31 вернули данные, а не `0xffff` (`attack11/out/otp_dump_secure.txt`), локи подтвердились как `0x3d00`
4. Восстановление ключа и расшифровка партиции B: `attack11/src/recover_key.py`. Первые 16 байт открытого текста совпали с утечкой по UART из [attack8](../attack8/), а `picotool info -a` на расшифрованном образе показывает `hash: verified` и `signature: verified` — это независимая криптопроверка побайтовой точности

## Результат

| Что | Значение |
| --- | --- |
| AES-256 ключ | `26efb69e8f543a0ea4db0523616a163233d3c9b7a6b5ec3a5fabdd2229ab91f8` |
| Соль OTP (page 31) | `7a10f676118536a7207c1d5c226a15a0` |
| Публичный IV (flash `0x80014`) | `921e94597201e138c2747ca1e1bd1b0b` |
| Образ приложения | `usb_token` 0.1, ARM Secure, Debug, pico-sdk 2.3.1, сборка 9 окт 2026 |
| Строки приложения | `usb_token boot`, `PIN OK`, `starting USB MSC`, `PIN FAIL (%u)`, `auth: locked for %u ms` |

Расшифрованный образ — [`attack11/out/partition_b_decrypted.bin`](out/partition_b_decrypted.bin), материал ключа с раскладкой по строкам — [`attack11/out/key_material.txt`](out/key_material.txt)

## Что осталось

- Pages 48–49 читаются из Secure-домена и содержат 64 байта данных (приведены в `key_material.txt`). Назначение не установлено; кандидат — материал ключа для блоба `0x300000`, который раньше не поддался офлайн-атакам
- Приложение `usb_token` теперь доступно целиком: логика PIN, USB MSC и скрытое хранилище — следующий шаг разбора
- Флеш не менялся, изменён только OTP: слот `BOOTKEY1` и бит `KEY_VALID` заняты навсегда, откат невозможен. Боевое устройство после этого остаётся рабочим: вендорский образ по-прежнему сходится с ключом 0

## Класс угрозы и оценка

Основной класс — CWE-1299 "Missing Protection Mechanism for Alternate Hardware Interface": интерфейс BOOTSEL оставлен с правом записи в OTP после включения secure boot. Близкие — CWE-288 (аутентификация в обход, через дополнительный слот доверия) и CWE-321 (ключ, доступный коду устройства)

CVSS 3.1: `AV:P/AC:L/PR:N/UI:N/S:U/C:H/I:H/A:N`, базовый балл 6.1 (Medium)

## Артефакты

| Файл | Содержимое |
| --- | --- |
| [`src/otp_dump.c`](src/otp_dump.c) | дампер OTP для RP2350: ECC и raw алиасы, локи, CRIT, BOOT_FLAGS1 |
| [`src/CMakeLists.txt`](src/CMakeLists.txt) | сборка дампера, включая RAM-вариант `no_flash` |
| [`src/recover_key.py`](src/recover_key.py) | разбор лога дампера, восстановление ключа, расшифровка партиции B |
| [`src/capture.py`](src/capture.py) | чтение вывода дампера по USB CDC |
| [`out/otp_dump_secure.txt`](out/otp_dump_secure.txt) | полный лог дампера из Secure-домена |
| [`out/otp_before_enrollment.txt`](out/otp_before_enrollment.txt) | OTP до записи |
| [`out/otp_after_enrollment.txt`](out/otp_after_enrollment.txt) | OTP после записи |
| [`out/otp_enroll_slot1_full.json`](out/otp_enroll_slot1_full.json) | файл записи: отпечаток в слот 1 и бит `KEY_VALID` |
| [`out/partition_b_decrypted.bin`](out/partition_b_decrypted.bin) | расшифрованный образ приложения |
| [`out/key_material.txt`](out/key_material.txt) | ключ, соль, IV и раскладка по строкам OTP |

## Повторение

```sh
python3 attack11/src/recover_key.py attack11/out/otp_dump_secure.txt firmware_dump.bin -o /tmp/partition_b.bin
```

Сборка и заливка дампера: `pico-sdk` 2.3.1, `cmake`/`ninja`, затем `picotool seal --sign build/otp_dump_ram.uf2 signed.uf2 attack11/keys/private.pem` и `picotool load -v -x signed.uf2`. Ключ подписи — [`keys/private.pem`](keys/private.pem), его отпечаток и проверка описаны в [`keys/README.md`](keys/README.md)
