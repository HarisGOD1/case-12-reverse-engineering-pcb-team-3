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
import zipfile
from pathlib import Path

import bomb_html


def bounded_page(original=None, copies=2):
    if not 1 <= copies <= 16:
        raise ValueError("The copy limit must be between 1 and 16")
    page = bomb_html.make_bomb_html() if original is None else bomb_html.inject_bomb(original)
    if page.count(b"while (true)") != 1 or page.count(b"hold.push(unit.slice());") != 1:
        raise ValueError("The payload shape changed; the bounded browser stand needs review")
    page = page.replace(b"while (true)", f"while (hold.length < {copies})".encode(), 1)
    return page.replace(
        b"hold.push(unit.slice());",
        b"hold.push(unit.slice()); document.documentElement.dataset.copies = String(hold.length);",
        1,
    )


def export_bounded(source, output, copies):
    archive = output.with_suffix(".zip")
    if source.resolve() in (output.resolve(), archive.resolve()):
        raise ValueError("Output paths must differ from the source page")
    for target in (output, archive):
        if target.exists() or target.is_symlink():
            raise FileExistsError(f"Output already exists: {target}")
    page = bounded_page(source.read_bytes(), copies)
    with zipfile.ZipFile(archive, "x", compression=zipfile.ZIP_DEFLATED) as bundle:
        bundle.writestr("prize.html", page)
    with output.open("xb") as target:
        target.write(page)
    return archive


def demonstrate(chrome, source=None, copies=2):
    if shutil.which(chrome) is None:
        raise ValueError(f"Chromium executable is unavailable: {chrome}")
    with tempfile.TemporaryDirectory(dir=Path(__file__).resolve().parent) as directory:
        scratch = Path(directory)
        page = scratch / "bounded.html"
        page.write_bytes(bounded_page(source.read_bytes() if source else None, copies))
        result = subprocess.run(
            [chrome, "--headless", "--no-sandbox", "--disable-gpu", "--disable-dev-shm-usage",
             "--disable-background-networking", f"--user-data-dir={scratch / 'profile'}",
             "--virtual-time-budget=15000", "--dump-dom", page.as_uri()],
            capture_output=True, text=True, timeout=60,
        )
        if result.returncode or f'data-copies="{copies}"' not in result.stdout or (
            source is None and "<h1>Congratulations!</h1>" not in result.stdout
        ):
            raise ValueError(f"Browser did not finish {copies} bounded copies: exit {result.returncode}")
    print(f"PASS: Chromium inflated the gzip seed and held {copies} bounded copies")
    print("The endless allocation loop and tab OOM were not run")


def main():
    parser = argparse.ArgumentParser(description="Run a bounded browser check of the attack2 HTML payload")
    parser.add_argument("--chromium", default="chromium", help="Chromium executable")
    parser.add_argument("--source", type=Path, help="source HTML page")
    parser.add_argument("--copies", type=int, choices=range(1, 17), default=2, help="retained 16 MiB copies (1..16)")
    parser.add_argument("--export", type=Path, help="write HTML here and a ZIP beside it")
    args = parser.parse_args()
    if args.export and not args.source:
        parser.error("--export requires --source")
    try:
        if args.export:
            archive = export_bounded(args.source, args.export, args.copies)
            print(f"Saved bounded page: {args.export}")
            print(f"Saved ZIP with prize.html: {archive}")
        else:
            demonstrate(args.chromium, args.source, args.copies)
    except (OSError, ValueError, subprocess.TimeoutExpired, zipfile.BadZipFile) as error:
        print(f"Browser stand failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
