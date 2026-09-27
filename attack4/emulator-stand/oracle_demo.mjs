// Timing-oracle demonstration on the REAL firmware in rp2040js (CWE-208).
// On the 4th button press the firmware calls sleep_ms(50) once per matching
// leading PIN digit, freezing all outputs. We dial a code, then instrument the
// sleep_ms entry (0x1000239c) and count calls whose argument is exactly 50 (the
// oracle delay; the poll tick uses 2 and the reject animation uses 200) -> the
// count is the matched-prefix length k, and 50*k ms is the observable think-time.
//
//   node oracle_demo.mjs <flash_dump.bin> <code>
import { readFileSync } from 'fs';
import { Simulator } from 'rp2040js';
import { bootromB1 } from './bootrom.mjs';

const dump = readFileSync(process.argv[2]);
const code = String(process.argv[3] || '0952').padStart(4, '0').slice(0, 4);
const REF = '3952';
const SILENT = { debug() {}, info() {}, warn() {}, error() {} };
const CYCLE_NS = 1e9 / 125e6, EDGE_MS = 30, A = 29, B = 27, SW = 28;
const SLEEP_MS = 0x1000239c, ORACLE_ARG = 50;

const sim = new Simulator(); const mcu = sim.rp2040;
mcu.logger = SILENT; mcu.loadBootrom(bootromB1); mcu.logger = SILENT;
mcu.flash.set(new Uint8Array(dump.buffer, dump.byteOffset, dump.length), 0);
mcu.core.PC = 0x10000000; sim.stopped = false;
for (const p of [A, B, SW]) mcu.gpio[p].setInputValue(true);

const runUntil = (t) => {
  while (sim.clock.nanos < t) {
    if (mcu.core.waiting) {
      const n = sim.clock.nanosToNextAlarm;
      const step = (!isFinite(n) || n < 0) ? t - sim.clock.nanos : Math.min(n, t - sim.clock.nanos);
      if (inSleep50) frozenNs += step;
      sim.clock.tick(step);
    } else {
      if (mcu.core.PC === SLEEP_MS) {
        if (mcu.core.registers[0] === ORACLE_ARG) { oracleCalls++; inSleep50 = true; sleepRet = mcu.core.registers[14] & ~1; }
        else inSleep50 = false;
      }
      if (inSleep50 && mcu.core.PC === sleepRet) inSleep50 = false;
      sim.clock.tick(mcu.core.executeInstruction() * CYCLE_NS);
    }
  }
};
const runMs = (ms) => runUntil(sim.clock.nanos + ms * 1e6);
let oracleCalls = 0, frozenNs = 0, inSleep50 = false, sleepRet = 0;

const segPtr = mcu.flashView.getUint32(0xdf0, true);
const segTab = [...Array(10)].map((_, d) => mcu.flash[(segPtr - 0x10000000) + d]);
const p2d = {}; segTab.forEach((pin, d) => (p2d[pin] = d));
const shown = () => { for (let p = 0; p <= 9; p++) { const g = mcu.gpio[p]; if (g.outputEnable && g.outputValue && p in p2d) return p2d[p]; } return -1; };
const GA = [1, 0, 0, 1], GB = [1, 1, 0, 0]; let q = 0;
const edge = (dr) => { q = (q + (dr > 0 ? 1 : 3)) & 3; mcu.gpio[A].setInputValue(!!GA[q]); mcu.gpio[B].setInputValue(!!GB[q]); runMs(EDGE_MS); };
const dial = (t) => { let g = 0; while (shown() !== t && g++ < 120) edge(+1); };

const k = [...code].findIndex((c, i) => c !== REF[i]) === -1 ? 4 : [...code].findIndex((c, i) => c !== REF[i]);
runMs(700);
const d = code.split('').map(Number);
for (let i = 0; i < 3; i++) { dial(d[i]); mcu.gpio[SW].setInputValue(false); runMs(40); mcu.gpio[SW].setInputValue(true); runMs(80); }
dial(d[3]);
oracleCalls = 0; frozenNs = 0;             // count only the 4th-press compare
mcu.gpio[SW].setInputValue(false); runMs(40);
mcu.gpio[SW].setInputValue(true); runMs(400);
console.log(`code ${code} vs ${REF}: true k=${k} | observed sleep_ms(50) calls=${oracleCalls} | think-time frozen=${(frozenNs / 1e6).toFixed(1)}ms (=50*${oracleCalls})`);
process.exit(0);
