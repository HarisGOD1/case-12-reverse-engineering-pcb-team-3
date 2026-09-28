#!/usr/bin/env python3
"""Tests for matryoshka.py: the layer rewrap/unwrap round-trip and its guards.

Uses a shallow 3-layer nest so it runs fast; the depth is a parameter, so the
same code drives the real 1337-layer prize. A falsification test proves the
suite would notice if the password scheme (password of layer N is N) broke.
"""
import os
import shutil
import subprocess
import sys
import tempfile

import matryoshka as M

FINAL = b"<html>PWNED PRIZE -- evil-maid payload</html>"
DEPTH = 3


def _fresh_nest(work):
    final = os.path.join(work, "prize.html")
    with open(final, "wb") as f:
        f.write(FINAL)
    return M.rewrap(work, final, DEPTH)


def test_passwords_are_literally_the_layer_numbers():
    # The contract the victim relies on: layer_N opens with the LITERAL number N.
    # Unwrap by hand with hard-coded "1".."DEPTH" (not M.password_for), so a broken
    # password scheme fails here even though rewrap and unwrap would still agree.
    with tempfile.TemporaryDirectory() as work:
        layers = _fresh_nest(work)
        u = os.path.join(work, "u")
        os.makedirs(u)
        subprocess.run(["7z", "x", "-y", f"-o{u}", layers],
                       capture_output=True, check=True)         # container: no password
        for n in range(1, DEPTH + 1):
            layer = os.path.join(u, f"layer_{n}.zip")
            r = subprocess.run(["7z", "x", f"-p{n}", "-y", f"-o{u}", layer],
                               capture_output=True, text=True)
            assert r.returncode == 0, f"layer_{n} did not open with literal password {n}"
            os.remove(layer)
        assert open(os.path.join(u, "prize.html"), "rb").read() == FINAL, "final payload not preserved"


def test_roundtrip_via_api_preserves_depth():
    with tempfile.TemporaryDirectory() as work:
        layers = _fresh_nest(work)
        u = os.path.join(work, "u")
        os.makedirs(u)
        shutil.copy(layers, os.path.join(u, "layers.zip"))
        final_path, depth = M.unwrap(u, os.path.join(u, "layers.zip"))
        assert depth == DEPTH, f"depth {depth} != {DEPTH}"
        assert open(final_path, "rb").read() == FINAL, "final payload not preserved"


def test_layers_are_really_encrypted():
    # layer_1 must reject the wrong password: proves rewrap wrote AES, not Store.
    with tempfile.TemporaryDirectory() as work:
        layers = _fresh_nest(work)
        u = os.path.join(work, "u")
        os.makedirs(u)
        subprocess.run(["7z", "x", "-y", f"-o{u}", layers],
                       capture_output=True, check=True)     # layers.zip: no password
        layer1 = os.path.join(u, "layer_1.zip")
        bad = subprocess.run(["7z", "x", "-pWRONG", "-y", f"-o{u}", layer1],
                             capture_output=True, text=True)
        assert bad.returncode != 0, "wrong password extracted layer_1 -- not encrypted"


def test_falsification_wrong_password_scheme_breaks_unwrap():
    # If layers were packed with a password OTHER than their number, unwrap (which
    # tries password = N) must fail. This is the guard that the scheme is honoured.
    with tempfile.TemporaryDirectory() as work:
        final = os.path.join(work, "prize.html")
        open(final, "wb").write(FINAL)
        # pack a single layer with the WRONG password (99 instead of 1)
        layer1 = os.path.join(work, "layer_1.zip")
        subprocess.run(["7z", "a", "-tzip", "-mem=AES256", "-p99", layer1, final],
                       capture_output=True, check=True)
        layers = os.path.join(work, "layers.zip")
        subprocess.run(["7z", "a", "-tzip", layers, layer1],
                       capture_output=True, check=True)
        u = os.path.join(work, "u")
        os.makedirs(u)
        shutil.copy(layers, os.path.join(u, "layers.zip"))
        try:
            M.unwrap(u, os.path.join(u, "layers.zip"))
        except RuntimeError:
            return                                          # expected: password 1 fails on a p99 layer
        raise AssertionError("unwrap accepted a layer packed with the wrong password")


def _run():
    tests = [v for k, v in sorted(globals().items()) if k.startswith("test_")]
    failed = 0
    for t in tests:
        try:
            t()
            print(f"[ok]   {t.__name__}")
        except Exception as e:                              # noqa: BLE001 (test runner reports any failure)
            failed += 1
            print(f"[FAIL] {t.__name__}: {e}")
    print(f"\n{'PASS' if failed == 0 else 'FAIL'}: {len(tests)} unit tests, {failed} failure(s)")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(_run())
