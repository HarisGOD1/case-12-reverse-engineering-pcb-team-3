#!/usr/bin/env python3
"""Tests for bomb_html.py: the injected page stays look-alike and the seed is real.

These are fast unit tests (no dump, no 7z). The end-to-end swap on a real dump is
exercised by evil_maid.py itself and recorded in demo_output.txt.
"""
import base64
import gzip
import re
import sys

import bomb_html

SAMPLE = b"<!DOCTYPE html><html><head><title>Rickrolled!</title></head>" \
         b"<body>\n<h1>PRIZE</h1>\n<img src=\"data:image/gif;base64,AAAA\">\n</body></html>"


def test_inject_keeps_original_markup():
    out = bomb_html.inject_bomb(SAMPLE)
    assert b"<h1>PRIZE</h1>" in out, "original heading lost"
    assert b"<img src=" in out, "original image lost"
    assert b"<title>Rickrolled!</title>" in out, "original title lost"


def test_script_injected_before_body_close_after_content():
    out = bomb_html.inject_bomb(SAMPLE)
    assert b"DecompressionStream" in out, "bomb script not injected"
    assert out.index(b"<script") < out.rindex(b"</body>"), "script must sit before </body>"
    assert out.index(b"<h1>PRIZE</h1>") < out.index(b"<script"), "visible content must come first"


def test_payload_is_silent():
    # look-alike requirement: the injected script writes nothing visible to the page
    out = bomb_html.inject_bomb(SAMPLE)
    for visible in (b"document.write", b"innerHTML", b"textContent", b"appendChild"):
        assert visible not in out, f"payload touches the DOM ({visible!r}) -- not look-alike"


def test_seed_inflates_to_declared_size():
    script = bomb_html.payload_script(unit_mib=16)
    m = re.search(r'SEED_B64 = "([^"]+)"', script)
    assert m, "SEED_B64 not found in payload script"
    seed = base64.b64decode(m.group(1))
    inflated = gzip.decompress(seed)
    assert len(inflated) == 16 * 1024 * 1024, f"seed inflates to {len(inflated)}, expected 16 MiB"
    assert inflated == b"\x00" * len(inflated), "seed is not the zero-fill bomb"
    assert len(seed) < len(inflated) // 100, "seed not compressed enough to be a bomb"


def test_no_anchor_still_injects():
    out = bomb_html.inject_bomb(b"<h1>bare</h1>")            # no </body> or </html>
    assert b"<h1>bare</h1>" in out and b"DecompressionStream" in out


def _run():
    tests = [v for k, v in sorted(globals().items()) if k.startswith("test_")]
    failed = 0
    for t in tests:
        try:
            t()
            print(f"[ok]   {t.__name__}")
        except Exception as e:                              # noqa: BLE001 (runner reports any failure)
            failed += 1
            print(f"[FAIL] {t.__name__}: {e}")
    print(f"\n{'PASS' if failed == 0 else 'FAIL'}: {len(tests)} unit tests, {failed} failure(s)")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(_run())
