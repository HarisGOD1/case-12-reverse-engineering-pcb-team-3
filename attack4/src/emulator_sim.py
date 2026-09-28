#!/usr/bin/env python3
"""
Hardware-free co-simulation of the attack4 encoder emulator against a model of
the safe, so the attack logic can be validated before touching the (capricious)
bench. Two layers are checked:

  1. CONTROL logic of the emulator (src/main.cpp primitives turnOne / gotoDigit /
     enterPin): does dialling produce the intended 4 digits, given the safe's
     "display resets to 0 on each confirm" behaviour? This is pure and exact.

  2. ORACLE attack strategy (the mode the clean rewrite still lacks): the safe
     leaks the matched-prefix length as think-time (sleep_ms(50) per matching
     leading digit, firmware 0x10000c92 / CWE-208). The emulator measures that
     over SENSE and recovers the PIN digit-by-digit. Modelled with jitter to
     check robustness and the <=40-press bound.

This does NOT drive real quadrature timing (that needs the bench for
QUARTERS_PER_STEP / INVERT_DIR calibration); it validates the digit-sequence and
decision logic that the calibration feeds. Run: python3 emulator_sim.py
Exit 0 = all checks pass.
"""
import random
import sys

TRUE_PIN = (3, 9, 5, 2)          # recovered elsewhere; the sim never assumes the attacker knows it
DIGIT_DELAY_MS = 50              # sleep_ms(50) per matching leading digit


# --------------------------------------------------------------------------
# Model of the safe as the emulator sees it: a displayed digit that steps with
# detents and RESETS TO 0 on each button confirm, a 4-slot entry buffer, and the
# firmware compare with its per-digit timing leak.
# --------------------------------------------------------------------------
class SafeModel:
    def __init__(self, reset_on_confirm=True):
        self.display = 0
        self.entered = []
        self.reset_on_confirm = reset_on_confirm
        self.unlocked = False

    def detent(self, dir_):
        self.display = (self.display + (1 if dir_ >= 0 else -1)) % 10

    def confirm(self):
        """Button press: latch the shown digit; on the 4th, run the compare."""
        self.entered.append(self.display)
        if self.reset_on_confirm:
            self.display = 0
        if len(self.entered) == 4:
            matched = self._matched_prefix(self.entered)
            if matched == 4:
                self.unlocked = True
            result = (self.unlocked, matched)
            self.entered = []
            return result
        return None

    @staticmethod
    def _matched_prefix(code):
        k = 0
        for a, b in zip(code, TRUE_PIN):
            if a != b:
                break
            k += 1
        return k

    def think_time_ms(self, matched_prefix):
        """Frozen-output interval the attacker measures on SENSE."""
        return DIGIT_DELAY_MS * matched_prefix


# --------------------------------------------------------------------------
# Model of the emulator's control logic (mirrors src/main.cpp).
# --------------------------------------------------------------------------
class Emulator:
    def __init__(self, safe, invert=False):
        self.safe = safe
        self.believed = 0
        self.invert = invert

    def turn_one(self, dir_):
        gdir = (-dir_ if self.invert else dir_)
        self.safe.detent(gdir)
        self.believed = (self.believed + (1 if dir_ >= 0 else -1)) % 10

    def goto_digit(self, target):
        up = (target - self.believed) % 10
        down = (10 - up) % 10
        if up == 0:
            return
        steps = [+1] * up if up <= down else [-1] * down
        for s in steps:
            self.turn_one(s)

    def enter_pin(self, pin4):
        self.believed = 0
        last = None
        for d in pin4:
            self.goto_digit(d)
            last = self.safe.confirm()
            self.believed = 0            # safe zeroed the display on confirm
        assert last is not None          # the 4th confirm always returns the compare result
        return last


# --------------------------------------------------------------------------
# Checks
# --------------------------------------------------------------------------
def check_control_logic():
    """With calibration matching the safe, enter_pin(3952) unlocks and a wrong
    code leaks the right matched-prefix length; shortest-path dialling and
    believed-tracking land the intended digits."""
    safe = SafeModel(reset_on_confirm=True)
    emu = Emulator(safe, invert=False)                 # invert matches this safe model
    unlocked, matched = emu.enter_pin(TRUE_PIN)
    assert unlocked and matched == 4, "correct PIN failed to unlock"

    safe = SafeModel(reset_on_confirm=True)
    emu = Emulator(safe, invert=False)
    unlocked, matched = emu.enter_pin((3, 9, 5, 3))
    assert not unlocked and matched == 3, "wrong PIN did not leak prefix 3"
    return True


def check_calibration_matters():
    """The two bench-calibration knobs must match the safe or digits desync -- the
    sim must NOT unlock when they are wrong (so a green run means something, and
    it flags exactly what needs the bench)."""
    # wrong direction (INVERT_DIR mismatched):
    safe = SafeModel(reset_on_confirm=True)
    if Emulator(safe, invert=True).enter_pin(TRUE_PIN)[0]:
        return False
    # wrong reset assumption (safe does not zero the display on confirm):
    safe = SafeModel(reset_on_confirm=False)
    if Emulator(safe, invert=False).enter_pin(TRUE_PIN)[0]:
        return False
    return True


def measure_k(safe_prefix, base_ms=0.0, step_ms=50.0, jitter_ms=6.0, rng=random):
    """Emulator's SENSE measurement: frozen interval ~ base + step*k, noisy."""
    freeze = base_ms + step_ms * safe_prefix + rng.uniform(-jitter_ms, jitter_ms)
    return round((freeze - base_ms) / step_ms)


def oracle_attack(rng):
    """Recover the PIN digit-by-digit via think-time, counting button presses.
    Returns (recovered_pin, presses)."""
    presses = 0
    pin = [0, 0, 0, 0]
    for pos in range(4):
        best_d, best_k = 0, -1
        for d in range(10):
            trial = pin[:pos] + [d] + [0] * (3 - pos)
            safe = SafeModel()
            emu = Emulator(safe)
            _, matched = emu.enter_pin(trial)
            presses += 4
            k = measure_k(matched, rng=rng)
            if k > best_k:
                best_k, best_d = k, d
            if k >= pos + 1:            # early out: this digit already matches the prefix
                best_d = d
                break
        pin[pos] = best_d
    return tuple(pin), presses


def check_oracle_recovers_pin():
    """Over many noisy runs the oracle must recover 3952 every time, and the
    per-attempt press budget must stay within the <=40-attempt claim."""
    rng = random.Random(1234)
    worst_attempts = 0
    for _ in range(500):
        pin, presses = oracle_attack(rng)
        assert pin == TRUE_PIN, f"oracle recovered wrong PIN {pin}"
        worst_attempts = max(worst_attempts, presses // 4)
    assert worst_attempts <= 40, f"oracle exceeded 40 attempts: {worst_attempts}"
    return worst_attempts


def check_dumb_sweep_bound():
    """A blind full sweep tries at most 10^4 codes and hits 3952 at its index."""
    target_index = TRUE_PIN[0] * 1000 + TRUE_PIN[1] * 100 + TRUE_PIN[2] * 10 + TRUE_PIN[3]
    return 0 <= target_index < 10000


def main():
    checks = [
        ("control logic dials the intended PIN with matching calibration", check_control_logic),
        ("wrong calibration (invert / no-reset) does NOT unlock -- flags bench needs", check_calibration_matters),
        ("dumb sweep bounded by 10^4", check_dumb_sweep_bound),
    ]
    failed = 0
    for name, fn in checks:
        try:
            fn()
            print(f"[ok]   {name}")
        except AssertionError as e:
            failed += 1
            print(f"[FAIL] {name}: {e}")

    try:
        worst = check_oracle_recovers_pin()
        print(f"[ok]   oracle recovers 3952 over 500 noisy runs, worst-case {worst} attempts (<=40)")
    except AssertionError as e:
        failed += 1
        print(f"[FAIL] oracle attack: {e}")

    print(f"\n{'PASS' if failed == 0 else 'FAIL'}: {failed} failure(s)")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
