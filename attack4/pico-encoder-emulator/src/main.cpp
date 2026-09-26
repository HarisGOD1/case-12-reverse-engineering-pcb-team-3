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
// DRIVE MODEL: open-drain. We only pull a line LOW or release it to high-Z (the
// target's pull-up restores HIGH). We never source a HIGH.
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
// One place for every delay. "Grarmar" of the timing lives here so the readiness
// function (next step) can key off these. All milliseconds.
// TODO(next): replace the fixed step delay with an adaptive one that keys off the
// target's response on SENSE instead of a constant.
static uint32_t EDGE_MS = 10;	 // between quadrature edges
static uint32_t DETENT_MS = 6;	 // after each one-digit detent
static uint32_t HOLD_MS = 40;	 // button held low
static uint32_t RELEASE_MS = 60; // button released before the next action
static uint32_t DIGIT_GAP_MS = 80; // between confirmed digits

// ------------------------------- Sense (opt) -------------------------------
static bool SENSE_ENABLE = false;	  // watch PIN_SENSE
static int SENSE_ACTIVE_LEVEL = HIGH; // level that means "unlocked" (once wired right)

// ------------------------------- Run state ---------------------------------
static int g_believed = RESET_DIGIT; // our belief of the shown digit (0..9)
static uint8_t g_qidx = 0;			 // quadrature state index 0..3

// Gray-coded quadrature states for (A,B); consecutive states differ by 1 bit.
static const uint8_t GRAY_A[4] = {1, 0, 0, 1};
static const uint8_t GRAY_B[4] = {1, 1, 0, 0};

// ------------------------------ Low-level I/O ------------------------------
// Open-drain: level 1 = release to high-Z (target pull-up wins), 0 = drive low.
static inline void setLine(uint8_t pin, uint8_t level)
{
	if (level)
	{
		pinMode(pin, INPUT); // high-Z
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
	for (int k = 0; k < QUARTERS_PER_STEP; k++)
	{
		g_qidx = (uint8_t)((g_qidx + (gdir > 0 ? 1 : 3)) & 3);
		applyQuadState(g_qidx);
		delay(edgeDelayMs());
	}
	delay(DETENT_MS);
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

// -------------------------------- Console ----------------------------------
static void printStatus()
{
	Serial.printf("[status] believed=%d q/step=%d invert=%d sense=%s\r\n",
				  g_believed, QUARTERS_PER_STEP, INVERT_DIR, SENSE_ENABLE ? "ON" : "OFF");
	Serial.printf("[timing] edge=%lu detent=%lu hold=%lu release=%lu digitgap=%lu (ms)\r\n",
				  (unsigned long)EDGE_MS, (unsigned long)DETENT_MS, (unsigned long)HOLD_MS,
				  (unsigned long)RELEASE_MS, (unsigned long)DIGIT_GAP_MS);
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
	Serial.println(F("  e  toggle SENSE    m<sec> monitor SENSE edges (find the right LED)"));
	Serial.println(F("Misc:   ?  status    h  this help"));
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
	Serial.println(F("[ready] manual mode. '.'/','/'p' primitives, n<pin>/k to enter."));
}

// --------------------------------- Loop ------------------------------------
void loop()
{
	while (Serial.available())
		handleChar(Serial.read());
}
