#!/usr/bin/env python3
"""Model the PIN comparison and timing leak after reading the reference from flash

This model does not run the firmware; attack4/emulator-stand does
"""
import itertools
from pathlib import Path

from extract_pin import extract_pin


DEFAULT_DUMP = Path(__file__).resolve().parents[1] / "artifacts" / "backup_full.bin"
REF_BYTES = bytes([0]) + bytes(int(digit) for digit in extract_pin(DEFAULT_DUMP))


def verify(entered):
    """Return acceptance and the length of the matching leading sequence"""
    for idx in range(1, 5):
        if entered[idx - 1] != REF_BYTES[idx]:
            return (False, idx - 1)
    return (True, 4)


def brute_force_full():
    hits = []
    for pin in itertools.product(range(10), repeat=4):
        if verify(pin)[0]:
            hits.append(pin)
    return hits


def brute_force_side_channel():
    """Find each digit from the simulated matching-prefix response"""
    pin = []
    for pos in range(4):                    # digit position 0..3
        best, best_prefix = None, -1
        for d in range(10):
            cand = pin + [d] + [0] * (4 - len(pin) - 1)
            prefix = verify(cand)[1]
            if prefix > best_prefix:
                best, best_prefix = d, prefix
        pin.append(best)
    return pin


def main():
    print(f"reference bytes from flash: {REF_BYTES.hex(' ')}")
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


if __name__ == "__main__":
    main()
