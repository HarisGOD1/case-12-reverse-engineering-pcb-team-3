import sys
import tempfile
import unittest
import zipfile
from pathlib import Path

import matryoshka
from browser_demo import bounded_page
from evil_maid import build_evil_prize

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "artifacts"))
from unpack_layers import unpack


class BoundedPrizeArchiveTests(unittest.TestCase):
    def test_bounded_page_survives_original_zip_layers(self):
        original = b"<html><body><h1>Original prize</h1></body></html>"
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            layers_dir = root / "layers"
            layers_dir.mkdir()
            (layers_dir / "prize.html").write_bytes(original)
            layers = matryoshka.rewrap(str(layers_dir), str(layers_dir / "prize.html"), 3)
            source = root / "your_prize.zip"
            with zipfile.ZipFile(source, "w") as outer:
                outer.write(layers, "layers.zip")
                outer.writestr("readme.txt", "Layer password equals its number")
            work = root / "build"
            work.mkdir()
            archive, depth, _, _ = build_evil_prize(str(work), str(source), bounded_copies=16)
            extracted = root / "result.html"
            self.assertEqual(depth, 3)
            self.assertEqual(unpack(Path(archive), extracted), 3)
            self.assertEqual(extracted.read_bytes(), bounded_page(original, 16))


if __name__ == "__main__":
    unittest.main()
