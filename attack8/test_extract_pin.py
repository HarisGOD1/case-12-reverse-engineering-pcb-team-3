import tempfile
import unittest
from pathlib import Path

from extract_pin import extract_pin


DUMP = Path(__file__).resolve().parents[1] / "artifacts" / "backup_full.bin"


class ExtractPinTests(unittest.TestCase):
    def test_reads_reference_from_flash(self):
        self.assertEqual(extract_pin(DUMP), "3952")

    def test_changed_reference_changes_result(self):
        with tempfile.TemporaryDirectory() as directory:
            changed = Path(directory) / "changed.bin"
            image = bytearray(DUMP.read_bytes())
            image[0x5500] = 4
            changed.write_bytes(image)
            self.assertEqual(extract_pin(changed), "4952")

    def test_short_image_fails(self):
        with tempfile.TemporaryDirectory() as directory:
            short = Path(directory) / "short.bin"
            short.write_bytes(b"\x00" * 20)
            with self.assertRaises(ValueError):
                extract_pin(short)


if __name__ == "__main__":
    unittest.main()
