#!/usr/bin/env python3
"""Unwrap and rewrap the prize matryoshka.

The prize is layers.zip -> layer_1.zip -> layer_2.zip -> ... -> prize.html, where
each layer_N.zip is an AES-256 zip whose password is the number N (the readme in
the volume states this). This module peels the layers down to the final payload
and rebuilds them with the SAME passwords, so an evil-maid can swap the innermost
file (see evil_maid.py) and leave every layer looking authentic to the victim.

7z is the only common tool that reads and writes AES-256 zips; Info-ZIP unzip
cannot, which is why the original prize is unpacked with `7z x -pN`.
"""
import os
import subprocess

SEVENZIP = "7z"


def _run(args):
    r = subprocess.run(args, capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError(f"{' '.join(args)} -> rc {r.returncode}\n{r.stderr[-400:]}")


def password_for(layer_number):
    """Password of layer_N.zip is the number N as a string (the readme's rule)."""
    return str(layer_number)


def unwrap(work_dir, layers_zip):
    """Peel layers.zip down to the final payload inside work_dir.

    Returns (final_path, depth). The final payload is whatever the deepest layer
    holds that is not another layer_*.zip.
    """
    _run([SEVENZIP, "x", "-y", f"-o{work_dir}", layers_zip])   # layers.zip is Store, no password
    depth = 0
    while True:
        n = depth + 1
        cur = os.path.join(work_dir, f"layer_{n}.zip")
        if not os.path.exists(cur):
            break
        _run([SEVENZIP, "x", f"-p{password_for(n)}", "-y", f"-o{work_dir}", cur])
        os.remove(cur)
        depth = n
    skip = os.path.basename(layers_zip)                        # ignore the input archive if it sits here
    finals = [f for f in os.listdir(work_dir)
              if not f.startswith("layer_") and f != skip]
    if len(finals) != 1:
        raise RuntimeError(f"expected one final payload, found {finals}")
    return os.path.join(work_dir, finals[0]), depth


def rewrap(work_dir, final_path, depth):
    """Rebuild depth AES-256 layers around final_path, innermost password = depth.

    Produces work_dir/layers.zip (a Store container over layer_1.zip), matching
    the original structure. Returns its path.
    """
    inner = final_path
    for n in range(depth, 0, -1):
        out = os.path.join(work_dir, f"layer_{n}.zip")
        if os.path.exists(out):
            os.remove(out)
        # pack `inner` (prize.html for n==depth, else layer_{n+1}.zip) into layer_n.zip
        _run([SEVENZIP, "a", "-tzip", "-mem=AES256", f"-p{password_for(n)}",
              out, inner])
        if inner != final_path:
            os.remove(inner)
        inner = out
    layers = os.path.join(work_dir, "layers.zip")
    if os.path.exists(layers):
        os.remove(layers)
    _run([SEVENZIP, "a", "-tzip", layers, inner])              # outer container: no password
    os.remove(inner)
    return layers
