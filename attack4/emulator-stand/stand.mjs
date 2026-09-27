// Hardware-free attack stand for vector 4: boot the REAL usb_token firmware in
// the rp2040js RP2040 emulator, impersonate the rotary encoder + button over
// GPIO, dial a PIN closed-loop (watching the displayed digit), and detect unlock
// via the firmware's own flag. No physical device, no micro-USB, no bench.
//
//   node stand.mjs <flash_dump.bin> [pin=3952]
//
// Needs rp2040js (npm i) and the RP2040 bootrom as ./bootrom.mjs
// (run ./fetch-bootrom.sh once). The dump is a raw 2 MiB flash image, passed as
// an argument; it is never bundled here.
import { readFileSync } from 'fs';
import { Simulator } from 'rp2040js';
import { bootromB1 } from './bootrom.mjs';

const dumpPath = process.argv[2];
if (!dumpPath) { console.error('usage: node stand.mjs <flash_dump.bin> [pin]'); process.exit(2); }
const code = String(process.argv[3] || '3952').padStart(4, '0').slice(0, 4);
const dump = readFileSync(dumpPath);

const SILENT = { debug() {}, info() {}, warn() {}, error() {} };
const CYCLE_NS = 1e9 / 125e6;
const EDGE_MS = 30;                 // calibrated in-emulator: clean, monotonic digit stepping
const A = 29, B = 27, SW = 28;      // target encoder pins: A / B / button
const UNLOCK_FLAG = 0x20002ED2;     // firmware unlock flag (set on correct PIN)
const PIN_LOOP = [0x10000ab0, 0x100010ef];

const sim = new Simulator();
const mcu = sim.rp2040;
mcu.logger = SILENT;
mcu.loadBootrom(bootromB1);
mcu.logger = SILENT;
mcu.flash.set(new Uint8Array(dump.buffer, dump.byteOffset, dump.length), 0);
mcu.core.PC = 0x10000000;            // run boot2 -> XIP -> app
sim.stopped = false;
for (const p of [A, B, SW]) mcu.gpio[p].setInputValue(true);   // idle high (pull-ups)

// Deterministic mirror of Simulator.execute(): advance the clock so the timer
// runs (sleep_ms works) and fast-forward through WFE low-power waits.
function runUntilNanos(target) {
  while (sim.clock.nanos < target) {
    if (mcu.core.waiting) {
      const n = sim.clock.nanosToNextAlarm;
      if (!isFinite(n) || n < 0) { sim.clock.tick(target - sim.clock.nanos); break; }
      sim.clock.tick(Math.min(n, target - sim.clock.nanos));
    } else {
      sim.clock.tick(mcu.core.executeInstruction() * CYCLE_NS);
    }
  }
}
const runMs = (ms) => runUntilNanos(sim.clock.nanos + ms * 1e6);

// displayed digit: the lit segment among GPIO 0..9 maps back through the
// firmware's digit->pin table (pointer at flash 0x10000df0).
const segPtr = mcu.flashView.getUint32(0xdf0, true);
const segTab = [...Array(10)].map((_, d) => mcu.flash[(segPtr - 0x10000000) + d]);
const pinToDigit = {}; segTab.forEach((pin, d) => (pinToDigit[pin] = d));
function shownDigit() {
  for (let p = 0; p <= 9; p++) {
    const g = mcu.gpio[p];
    if (g.outputEnable && g.outputValue && p in pinToDigit) return pinToDigit[p];
  }
  return -1;
}

// quadrature drive (Gray code, matching the encoder emulator firmware)
const GRAY_A = [1, 0, 0, 1], GRAY_B = [1, 1, 0, 0]; let qidx = 0;
function edge(dir) {
  qidx = (qidx + (dir > 0 ? 1 : 3)) & 3;
  mcu.gpio[A].setInputValue(!!GRAY_A[qidx]);
  mcu.gpio[B].setInputValue(!!GRAY_B[qidx]);
  runMs(EDGE_MS);
}
function dialTo(target) {            // closed-loop: turn until the display shows target
  let guard = 0;
  while (shownDigit() !== target && guard++ < 120) edge(+1);
  return shownDigit() === target;
}
function press() {
  mcu.gpio[SW].setInputValue(false); runMs(40);
  mcu.gpio[SW].setInputValue(true); runMs(80);
}

runMs(700);                         // boot to the encoder loop
const pc = mcu.core.PC;
const gated = pc >= PIN_LOOP[0] && pc <= PIN_LOOP[1] || (pc >= 0x1000239c && pc <= 0x10002cff);
console.log(`booted (${(sim.clock.nanos / 1e6).toFixed(0)} ms virtual); USB gated behind PIN loop: ${gated}`);
console.log(`entering PIN ${code} via emulated encoder...`);

for (const d of code.split('').map(Number)) {
  const ok = dialTo(d);
  console.log(`  dialled ${d} (ok=${ok})`);
  press();
}
runMs(300);

const flag = mcu.readUint8(UNLOCK_FLAG);
const unlocked = flag === 1;
console.log(`unlock flag *0x${UNLOCK_FLAG.toString(16)} = ${flag} -> ${unlocked ? 'UNLOCKED' : 'locked'}`);
console.log(`[${unlocked ? 'PASS' : 'INFO'}] PIN ${code}: ${unlocked
  ? 'real firmware opened in emulation, no hardware'
  : 'not opened (expected for a wrong PIN)'}`);
process.exit(unlocked ? 0 : 3);
