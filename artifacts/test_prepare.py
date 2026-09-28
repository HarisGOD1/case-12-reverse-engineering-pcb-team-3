import tempfile
import unittest
from pathlib import Path

from prepare import ARTIFACTS, prepare


class PrepareTests(unittest.TestCase):
    def test_stored_dump_reproduces_stored_archive(self):
        prepare(ARTIFACTS / "backup_full.bin", verify=True)

    def test_modified_dump_fails_verification(self):
        with tempfile.TemporaryDirectory() as directory:
            changed = Path(directory) / "flash.bin"
            data = bytearray((ARTIFACTS / "backup_full.bin").read_bytes())
            data[0] ^= 1
            changed.write_bytes(data)
            with self.assertRaises(ValueError):
                prepare(changed, verify=True)

    def test_unrelated_reference_zip_fails_verification(self):
        with tempfile.TemporaryDirectory() as directory:
            reference = Path(directory) / "reference.zip"
            reference.write_bytes(b"unrelated")
            with self.assertRaises(ValueError):
                prepare(ARTIFACTS / "backup_full.bin", reference, verify=True)


if __name__ == "__main__":
    unittest.main()
