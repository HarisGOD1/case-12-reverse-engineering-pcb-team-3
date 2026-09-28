#!/usr/bin/env bash
# Fetch the RP2040 bootrom (revision B1) as an ES module for the emulator stand.
# The bootrom is Raspberry Pi's, redistributed by the rp2040js project under
# BSD-3-Clause; we do not vendor it here, we fetch it on demand.
set -euo pipefail

usage() { echo "usage: $0   # writes src/bootrom.mjs (RP2040 B1 bootrom as a Uint32Array)"; }
if [ "${1:-}" = "-h" ] || [ "${1:-}" = "--help" ]; then usage; exit 0; fi

URL="https://raw.githubusercontent.com/wokwi/rp2040js/main/demo/bootrom.ts"
OUT="$(dirname "$0")/bootrom.mjs"

echo "fetching RP2040 B1 bootrom from rp2040js..."
curl -fsSL --max-time 30 "$URL" -o "$OUT"
grep -q 'export const bootromB1' "$OUT" || { echo "error: unexpected content" >&2; rm -f "$OUT"; exit 1; }
echo "wrote $OUT ($(wc -c < "$OUT") bytes) -- valid ESM (export const bootromB1)"
