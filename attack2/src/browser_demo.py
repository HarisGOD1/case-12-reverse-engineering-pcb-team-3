#!/usr/bin/env python3
"""Exercise the HTML payload in Chromium with a strict allocation limit

The stand changes only the endless loop and adds an observable counter
It verifies gzip inflation and the event loop without triggering browser OOM
"""
import argparse
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

import bomb_html


def bounded_page():
    page = bomb_html.make_bomb_html()
    if page.count(b"while (true)") != 1 or page.count(b"hold.push(unit.slice());") != 1:
        raise ValueError("The payload shape changed; the bounded browser stand needs review")
    page = page.replace(b"while (true)", b"while (hold.length < 2)", 1)
    return page.replace(
        b"hold.push(unit.slice());",
        b"hold.push(unit.slice()); document.documentElement.dataset.copies = String(hold.length);",
        1,
    )


def demonstrate(chrome):
    if shutil.which(chrome) is None:
        raise ValueError(f"Chromium executable is unavailable: {chrome}")
    with tempfile.TemporaryDirectory(dir=Path(__file__).resolve().parent) as directory:
        scratch = Path(directory)
        page = scratch / "bounded.html"
        page.write_bytes(bounded_page())
        result = subprocess.run(
            [chrome, "--headless", "--no-sandbox", "--disable-gpu", "--disable-dev-shm-usage",
             "--disable-background-networking", f"--user-data-dir={scratch / 'profile'}",
             "--virtual-time-budget=15000", "--dump-dom", page.as_uri()],
            capture_output=True, text=True, timeout=60,
        )
        if result.returncode or 'data-copies="2"' not in result.stdout or "<h1>Congratulations!</h1>" not in result.stdout:
            raise ValueError(f"Browser did not finish two bounded copies: exit {result.returncode}")
    print("PASS: Chromium inflated the gzip seed, held two copies, and kept the visible heading")
    print("The endless allocation loop and tab OOM were not run")


def main():
    parser = argparse.ArgumentParser(description="Run a bounded browser check of the attack2 HTML payload")
    parser.add_argument("--chromium", default="chromium", help="Chromium executable")
    args = parser.parse_args()
    try:
        demonstrate(args.chromium)
    except (OSError, ValueError, subprocess.TimeoutExpired) as error:
        print(f"Browser stand failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
