// Count only 50 ms sleep calls. Polling uses 2 ms; rejection uses 200 ms
// The observed call count must match the PIN prefix read from flash
import { readFileSync } from 'fs';
import { Simulator } from 'rp2040js';
import { bootromB1 } from './bootrom.mjs';

const dump = readFileSync(process.argv[2]);
const code = String(process.argv[3] || '0952').padStart(4, '0').slice(0, 4);
const refOffset = dump.readUInt32LE(0xe10) - 0x10000000;
const REF = [...dump.subarray(refOffset + 1, refOffset + 5)].join('');
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
oracleCalls = 0; frozenNs = 0;             // earlier button presses do not compare the PIN
mcu.gpio[SW].setInputValue(false); runMs(40);
mcu.gpio[SW].setInputValue(true); runMs(400);
const flag = mcu.readUint8(0x20002ed2);
const passed = oracleCalls === k && (flag === 1) === (k === 4);
console.log(`code ${code} vs ${REF}: expected prefix=${k} | observed sleep_ms(50) calls=${oracleCalls} | frozen=${(frozenNs / 1e6).toFixed(1)}ms | unlock flag=${flag}`);
console.log(`[${passed ? 'PASS' : 'FAIL'}] firmware response matches the PIN reference`);
process.exit(passed ? 0 : 1);
