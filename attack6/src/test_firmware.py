import unittest
from pathlib import Path

from demo import build_flash
from make_evil_firmware import PATCHES, make_uf2, patch_image


ROOT = Path(__file__).resolve().parents[2]
PROGRAM = (ROOT / "reversing" / "program.bin").read_bytes()


class FirmwareTests(unittest.TestCase):
    def test_each_patch_changes_only_declared_bytes(self):
        for name, patches in PATCHES.items():
            with self.subTest(name=name):
                changed = patch_image(PROGRAM, name)
                differences = {i for i in range(len(PROGRAM)) if changed[i] != PROGRAM[i]}
                expected = {i for off, old, new, _ in patches for i in range(off, off + len(new))}
                self.assertEqual(differences, expected)
                self.assertEqual(changed, (ROOT / "attack6" / "out" / f"evil_{name}.bin").read_bytes())
                self.assertEqual(make_uf2(changed), (ROOT / "attack6" / "out" / f"evil_{name}.uf2").read_bytes())

    def test_wrong_firmware_is_rejected(self):
        changed = bytearray(PROGRAM)
        changed[0xC8C] ^= 1
        with self.assertRaises(ValueError):
            patch_image(changed, "anypin")

    def test_emulator_image_preserves_flash_data(self):
        flash = (ROOT / "artifacts" / "backup_full.bin").read_bytes()
        changed = build_flash(flash, "anypin")
        self.assertEqual(changed[:len(PROGRAM)], patch_image(PROGRAM, "anypin"))
        self.assertEqual(changed[len(PROGRAM):], flash[len(PROGRAM):])


if __name__ == "__main__":
    unittest.main()
