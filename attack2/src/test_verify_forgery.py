import tempfile
import unittest
from pathlib import Path

from verify_forgery import verify_forgery


ROOT = Path(__file__).resolve().parents[2]


class ForgeryVerificationTests(unittest.TestCase):
    def test_change_outside_storage_is_rejected(self):
        source = ROOT / "artifacts" / "backup_full.bin"
        with tempfile.TemporaryDirectory() as directory:
            forged = Path(directory) / "forged.bin"
            changed = bytearray(source.read_bytes())
            changed[0] ^= 1
            forged.write_bytes(changed)
            with self.assertRaises(ValueError):
                verify_forgery(source, forged)


if __name__ == "__main__":
    unittest.main()
