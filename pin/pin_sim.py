#!/usr/bin/env python3
"""
PIN simulation + brute-force for the LCT-2026 cyber-safe (RP2040, usb_token).

Reconstructed from the firmware (see reversing/README.md):
  * board_gpio_init @ 0x10000ab0 polls the rotary encoder, keeps 4 digits
    (raw values 0..9, wrap 9->0 / 0->9) in a stack buffer at sp+0x34.
  * On the 4th encoder-button press the firmware verifies:
        for idx in 1..4:
            if entered[sp+0x33+idx] != ref[0x100054FF+idx]:  FAIL (early exit)
            delay(50)                                        # feedback leak
        UNLOCK  (flag *0x20002ED2 = 1 -> USB MSC becomes visible)
  * ref bytes at 0x100054FF: 00 03 09 05 02  (ref[0] is a dummy byte)

This script reimplements the check faithfully and brute-forces the whole
10^4 space twice:
  1) "read the flag" attack      - what the firmware compares against.
  2) "black-box" beep/timing side channel - the delay-per-matching-digit
     leaks the longest matching prefix, so a real device yields the PIN
     in at most 4*10 = 40 dial attempts (no firmware knowledge needed).
"""
import itertools

# --- constants extracted from the firmware image -------------------------
REF_ADDR   = 0x100054FF          # pointer pool DAT_10000e10
REF_BYTES  = bytes([0x00, 0x03, 0x09, 0x05, 0x02])   # dump: 00 03 09 05 02
UNLOCK_PTR = 0x20002ED2          # pointer pool DAT_10000e14 (flag byte in SRAM)
DIGIT_DELAY_MS = 50              # FUN_1000239c(0x32) per matching digit


# --- faithful reimplementation of the verification loop ------------------
def verify(entered):
    """entered: 4 ints 0..9.  Returns (unlocked, matching_prefix_len)."""
    for idx in range(1, 5):                 # idx = 1..4
        if entered[idx - 1] != REF_BYTES[idx]:
            return (False, idx - 1)          # early exit at first mismatch
        # digit matched: device delays/beeps here (side-channel)
    return (True, 4)


def brute_force_full():
    """Option 2a: run all 10^4 codes through the simulated checker."""
    hits = []
    for pin in itertools.product(range(10), repeat=4):
        if verify(pin)[0]:
            hits.append(pin)
    return hits


def brute_force_side_channel():
    """Option 2b: black-box attack using the per-digit feedback delay.

    The device emits one 50 ms delay per leading matching digit, so the
    response time leaks how many digits are correct. Find each digit in
    isolation -> at most 40 attempts.
    """
    pin = []
    for pos in range(4):                    # digit position 0..3
        best, best_prefix = None, -1
        for d in range(10):                 # dial digit, press, observe time
            cand = pin + [d] + [0] * (4 - len(pin) - 1)
            prefix = verify(cand)[1]
            if prefix > best_prefix:
                best, best_prefix = d, prefix
        pin.append(best)
    return pin


def main():
    print(f"reference bytes @ {REF_ADDR:#x}: {REF_BYTES.hex(' ')}")
    print(f"(dummy byte, d1, d2, d3, d4)  -> PIN candidate: "
          f"{''.join(str(b) for b in REF_BYTES[1:])}")
    print()

    hits = brute_force_full()
    print(f"[sim] full 10^4 brute force: {len(hits)} accepted PIN(s): "
          + ", ".join("".join(map(str, h)) for h in hits))

    pin = brute_force_side_channel()
    print(f"[sim] black-box side-channel attack: {len(pin) and 40} dial "
          f"attempts worst-case -> PIN {''.join(map(str, pin))}")

    ok, prefix = verify(hits[0] if hits else [0, 0, 0, 0])
    print(f"[sim] verify({''.join(map(str, hits[0]))}) -> unlocked={ok}")
    print()
    print(f"unlock flag write: *(uint8_t*){UNLOCK_PTR:#x} = 1  "
          f"(gates USB MSC visibility to the host)")


if __name__ == "__main__":
    main()