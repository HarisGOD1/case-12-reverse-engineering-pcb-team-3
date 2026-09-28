import tempfile
import unittest
from pathlib import Path

from decrypt_storage import STORAGE_LEN, STORAGE_OFF, decrypt


SOURCE = Path(__file__).resolve().parents[1] / "artifacts" / "backup_full.bin"


class DecryptStorageTests(unittest.TestCase):
    def test_real_dump_yields_fat12_boot_sector(self):
        volume = decrypt(SOURCE)
        self.assertEqual(len(volume), STORAGE_LEN)
        self.assertEqual(volume[0:3], b"\xeb\x3c\x90")
        self.assertEqual(volume[11:13], b"\x00\x02")
        self.assertEqual(volume[510:512], b"\x55\xaa")

    def test_partial_dump_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            partial = Path(directory) / "partial.bin"
            with SOURCE.open("rb") as source, partial.open("wb") as target:
                target.write(source.read(STORAGE_OFF + 512))
            with self.assertRaises(ValueError):
                decrypt(partial)


if __name__ == "__main__":
    unittest.main()
