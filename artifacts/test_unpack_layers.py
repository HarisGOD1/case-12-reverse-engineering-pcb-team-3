import io
import subprocess
import tempfile
import unittest
import zipfile
from pathlib import Path

from unpack_layers import unpack


class UnpackTests(unittest.TestCase):
    def setUp(self):
        self.scratch = tempfile.TemporaryDirectory()
        self.addCleanup(self.scratch.cleanup)
        self.root = Path(self.scratch.name)
        self.archive = self.root / "your_prize.zip"
        self.result = self.root / "prize.html"

    def encrypted_zip(self, name, member, password):
        subprocess.run(
            ["7z", "a", "-tzip", "-mem=AES256", f"-p{password}", name, member],
            cwd=self.root,
            check=True,
            capture_output=True,
        )

    def build_archive(self):
        (self.root / "prize.html").write_bytes(b"<html>verified</html>")
        self.encrypted_zip("layer_2.zip", "prize.html", 2)
        self.result.unlink()
        self.encrypted_zip("layer_1.zip", "layer_2.zip", 1)
        with zipfile.ZipFile(self.root / "layers.zip", "w") as layers:
            layers.write(self.root / "layer_1.zip", "layer_1.zip")
        with zipfile.ZipFile(self.archive, "w") as outer:
            outer.write(self.root / "layers.zip", "layers.zip")
            outer.writestr("readme.txt", b"Passwords equal layer numbers")

    def test_password_chain_produces_final_html(self):
        self.build_archive()
        self.assertEqual(unpack(self.archive, self.result), 2)
        self.assertEqual(self.result.read_bytes(), b"<html>verified</html>")

    def test_bad_layer_does_not_publish(self):
        with zipfile.ZipFile(self.archive, "w") as outer:
            outer.writestr("layers.zip", self._bad_layers())
        with self.assertRaises(ValueError):
            unpack(self.archive, self.result)
        self.assertFalse(self.result.exists())

    def _bad_layers(self):
        stream = io.BytesIO()
        with zipfile.ZipFile(stream, "w") as layers:
            layers.writestr("unexpected.zip", b"invalid")
        return stream.getvalue()

    def test_existing_result_is_not_replaced(self):
        self.result.write_bytes(b"original")
        with self.assertRaises(FileExistsError):
            unpack(self.archive, self.result)
        self.assertEqual(self.result.read_bytes(), b"original")


if __name__ == "__main__":
    unittest.main()
