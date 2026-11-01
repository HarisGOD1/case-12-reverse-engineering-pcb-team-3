#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "hardware/regs/addressmap.h"
#include "pico/stdlib.h"

#ifndef PROBE_GUARDED
#define PROBE_GUARDED 0
#endif

#define OTP_ROW_COUNT 4096u
#define OTP_PAGE_COUNT 64u
#define OTP_ROWS_PER_PAGE 64u
#define OTP_LOCK_BASE 0x0f80u
#define OTP_CRIT0_BASE 0x0038u
#define OTP_CRIT1_BASE 0x0040u
#define OTP_BOOT_FLAGS1_BASE 0x004bu

static volatile const uint32_t *const otp_ecc = (volatile const uint32_t *)(uintptr_t)OTP_DATA_BASE;
static volatile const uint32_t *const otp_raw = (volatile const uint32_t *)(uintptr_t)OTP_DATA_RAW_BASE;

static uint16_t ecc_row(uint16_t row) {
    uint32_t pair = otp_ecc[row >> 1];
    return (uint16_t)(pair >> ((row & 1u) * 16u));
}

static uint32_t raw_row(uint16_t row) {
    return otp_raw[row] & 0x00ffffffu;
}

static uint8_t majority3_bytes(uint32_t raw) {
    uint8_t a = (uint8_t)raw;
    uint8_t b = (uint8_t)(raw >> 8);
    uint8_t c = (uint8_t)(raw >> 16);
    return (uint8_t)((a & b) | (a & c) | (b & c));
}

static bool triple_bytes_match(uint32_t raw) {
    uint8_t a = (uint8_t)raw;
    uint8_t b = (uint8_t)(raw >> 8);
    uint8_t c = (uint8_t)(raw >> 16);
    return a == b && b == c;
}

static uint32_t majority3_words(uint32_t a, uint32_t b, uint32_t c) {
    return (a & b) | (a & c) | (b & c);
}

static uint8_t count_set_votes(const uint32_t *rows, unsigned count, unsigned bit) {
    uint8_t votes = 0;
    for (unsigned i = 0; i < count; ++i) {
        votes += (uint8_t)((rows[i] >> bit) & 1u);
    }
    return votes;
}

static uint32_t majority8_words(const uint32_t rows[8]) {
    uint32_t result = 0;
    for (unsigned bit = 0; bit < 24; ++bit) {
        if (count_set_votes(rows, 8, bit) >= 3u) {
            result |= 1u << bit;
        }
    }
    return result;
}

static const char *lock_state(unsigned code) {
    switch (code) {
        case 0:
            return "rw";
        case 1:
            return "ro";
        case 2:
            return "inaccessible";
        default:
            return "invalid(3)";
    }
}

static const char *no_key_state(unsigned code) {
    return code == 0u ? "ro" : "inaccessible";
}

static void dump_page_ecc(void) {
    for (unsigned page = 0; page < OTP_PAGE_COUNT; ++page) {
        uint16_t values[OTP_ROWS_PER_PAGE];
        unsigned all_ones = 0;
        unsigned first_row = page * OTP_ROWS_PER_PAGE;

        for (unsigned i = 0; i < OTP_ROWS_PER_PAGE; ++i) {
            values[i] = ecc_row((uint16_t)(first_row + i));
            all_ones += values[i] == 0xffffu;
        }

        printf("page %u: ECC=", page);
        for (unsigned i = 0; i < OTP_ROWS_PER_PAGE; ++i) {
            printf("%s%04x", i == 0u ? "" : " ", (unsigned)values[i]);
        }
        printf(" all_ones=%u verdict=%s\n", all_ones,
               all_ones == OTP_ROWS_PER_PAGE ? "all-ones" : all_ones == 0u ? "data" : "mixed");
    }
}

static void dump_raw_rows(void) {
    puts("RAW OTP dump");
    for (unsigned row = 0; row < OTP_ROW_COUNT; ++row) {
        printf("row 0x%03x: 0x%06lx\n", row, (unsigned long)raw_row((uint16_t)row));
    }
}

static void dump_lock_words(void) {
    puts("OTP page locks (raw triple-byte majority)");
    for (unsigned page = 0; page < OTP_PAGE_COUNT; ++page) {
        uint16_t low_row = (uint16_t)(OTP_LOCK_BASE + 2u * page);
        uint16_t high_row = (uint16_t)(low_row + 1u);
        uint32_t low_raw = raw_row(low_row);
        uint32_t high_raw = raw_row(high_row);
        uint8_t low = majority3_bytes(low_raw);
        uint8_t high = majority3_bytes(high_raw);
        uint16_t lock = (uint16_t)(low | ((uint16_t)high << 8));
        bool replicas_match = triple_bytes_match(low_raw) && triple_bytes_match(high_raw);
        unsigned write_key = lock & 0x7u;
        unsigned read_key = (lock >> 3) & 0x7u;
        unsigned no_key = (lock >> 6) & 0x1u;
        unsigned secure = (lock >> 8) & 0x3u;
        unsigned nonsecure = (lock >> 10) & 0x3u;
        unsigned picoboot = (lock >> 12) & 0x3u;

        printf("page %u: SEC=%s NSEC=%s BL=%s write_key=%u read_key=%u no-key=%s triple_match=%s word=0x%04x\n",
               page, lock_state(secure), lock_state(nonsecure), lock_state(picoboot), write_key, read_key,
               no_key_state(no_key), replicas_match ? "yes" : "no", (unsigned)lock);

        if (low_raw == 0x00ffffffu) {
            printf("LOCK CONTRADICTION: page %u lock row 0x%03x reads 0xffffff; lock rows are specified as world-readable\n",
                   page, (unsigned)low_row);
        }
        if (high_raw == 0x00ffffffu) {
            printf("LOCK CONTRADICTION: page %u lock row 0x%03x reads 0xffffff; lock rows are specified as world-readable\n",
                   page, (unsigned)high_row);
        }
    }
}

static void print_crit_bit(const char *name, const uint32_t rows[8], uint32_t voted, unsigned bit) {
    printf(" %s=%u(%u/8)", name, (unsigned)((voted >> bit) & 1u), (unsigned)count_set_votes(rows, 8, bit));
}

static void dump_critical_flags(void) {
    uint32_t crit0_rows[8];
    uint32_t crit1_rows[8];
    uint32_t crit0;
    uint32_t crit1;

    for (unsigned i = 0; i < 8; ++i) {
        crit0_rows[i] = raw_row((uint16_t)(OTP_CRIT0_BASE + i));
        crit1_rows[i] = raw_row((uint16_t)(OTP_CRIT1_BASE + i));
    }
    crit0 = majority8_words(crit0_rows);
    crit1 = majority8_words(crit1_rows);

    printf("CRIT0 RBIT-8 voted=0x%06lx", (unsigned long)crit0);
    print_crit_bit("ARM_DISABLE", crit0_rows, crit0, 0);
    print_crit_bit("RISCV_DISABLE", crit0_rows, crit0, 1);
    putchar('\n');

    printf("CRIT1 RBIT-8 voted=0x%06lx", (unsigned long)crit1);
    print_crit_bit("SECURE_BOOT_ENABLE", crit1_rows, crit1, 0);
    print_crit_bit("SECURE_DEBUG_DISABLE", crit1_rows, crit1, 1);
    print_crit_bit("DEBUG_DISABLE", crit1_rows, crit1, 2);
    print_crit_bit("BOOT_ARCH", crit1_rows, crit1, 3);
    print_crit_bit("GLITCH_DETECTOR_ENABLE", crit1_rows, crit1, 4);
    printf(" GLITCH_DETECTOR_SENS=%u(bit5=%u/8,bit6=%u/8)\n",
           (unsigned)((crit1 >> 5) & 0x3u), (unsigned)count_set_votes(crit1_rows, 8, 5),
           (unsigned)count_set_votes(crit1_rows, 8, 6));
}

static void dump_boot_flags1(void) {
    uint32_t a = raw_row(OTP_BOOT_FLAGS1_BASE);
    uint32_t b = raw_row(OTP_BOOT_FLAGS1_BASE + 1u);
    uint32_t c = raw_row(OTP_BOOT_FLAGS1_BASE + 2u);
    uint32_t flags = majority3_words(a, b, c);
    unsigned key_valid = flags & 0x0fu;
    unsigned key_invalid = (flags >> 8) & 0x0fu;

    printf("BOOT_FLAGS1 RBIT-3 voted=0x%06lx KEY_VALID=0x%x KEY_INVALID=0x%x\n",
           (unsigned long)flags, key_valid, key_invalid);
    for (unsigned slot = 0; slot < 4; ++slot) {
        bool valid = (key_valid & (1u << slot)) != 0u;
        bool invalid = (key_invalid & (1u << slot)) != 0u;
        const char *state = invalid ? "forbidden" : valid ? "valid" : "not-valid";
        printf("boot key slot %u: KEY_VALID=%u KEY_INVALID=%u state=%s\n",
               slot, valid ? 1u : 0u, invalid ? 1u : 0u, state);
    }
}

static void dump_special_raw_range(const char *label, uint16_t first, uint16_t last) {
    printf("%s raw rows\n", label);
    for (uint16_t row = first; row <= last; ++row) {
        printf("row 0x%03x: 0x%06lx\n", (unsigned)row, (unsigned long)raw_row(row));
    }
}

static void report_guarded_probe(void) {
#if PROBE_GUARDED
    puts("probe skipped: PROBE_GUARDED requested but guarded-fault recovery is not implemented");
#else
    puts("probe skipped");
#endif
}

static void dump_all(void) {
    puts("=== SUMMARY ===");
    dump_critical_flags();
    dump_boot_flags1();
    dump_lock_words();
    dump_special_raw_range("pages 29-31 (AES key share + IV salt)", 0x0740u, 0x07ffu);
    dump_special_raw_range("pages 48-49 (locked user data)", 0x0c00u, 0x0c7fu);
    dump_special_raw_range("page 61 access keys and valid rows", 0x0f40u, 0x0f7fu);
    puts("=== FULL DUMP ===");
    dump_page_ecc();
    dump_raw_rows();
    report_guarded_probe();
    stdio_flush();
}

int main(void) {
    stdio_init_all();
    puts("RP2350 OTP read-only dumper (loop mode, output repeats every 5 s)");
    printf("ECC alias=0x%08lx raw alias=0x%08lx\n",
           (unsigned long)OTP_DATA_BASE, (unsigned long)OTP_DATA_RAW_BASE);

    while (true) {
        dump_all();
        sleep_ms(5000);
    }
}
