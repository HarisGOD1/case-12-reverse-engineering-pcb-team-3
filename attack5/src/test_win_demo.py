"""Core of win_demo: recovery and the two cross-checks from a modelled tail.

The Windows raw-disk read cannot run without the device, so it is not tested
here. The recovery core is: model the device's served bytes from the dump the
same way the hardware would (erased flash XOR keystream), hand only the tail to
recover_and_verify, and require it to rebuild the generator. Then check both
oracles: the live volume the firmware serves already-decrypted is valid FAT12
as-is, and the cold ciphertext snapshot decrypts to FAT12 with a valid ZIP. A
corrupted tail must break the cold decryption.
"""
import os
import sys
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import keystream_leak_demo as kld  # noqa: E402
import win_demo  # noqa: E402

DUMP = os.path.join(HERE, "..", "..", "artifacts", "backup_full.bin")


def model_tail(dump):
    """The bytes the safe serves for the tail: erased flash XOR the keystream."""
    tail = bytearray()
    for lba in range(win_demo.TAIL_FIRST, win_demo.DISK_SECTORS):
        base = kld.STORAGE_OFF + lba * win_demo.SECTOR
        flash = dump[base:base + win_demo.SECTOR]
        ks = kld.ks_sector(lba)
        tail += bytes(f ^ k for f, k in zip(flash, ks))
    return bytes(tail)


def cold_ciphertext(dump):
    return dump[kld.STORAGE_OFF:kld.STORAGE_OFF + kld.STORAGE_LEN]


@unittest.skipUnless(os.path.exists(DUMP), "needs artifacts/backup_full.bin")
class RecoverAndVerifyTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with open(DUMP, "rb") as handle:
            cls.dump = handle.read()
        cls.tail = model_tail(cls.dump)
        cls.cold = cold_ciphertext(cls.dump)
        # what the firmware serves over USB for the volume: already decrypted
        found = kld.recover_generator_from_leak(kld.decode_tail_response(cls.tail))
        assert found is not None, "the modelled tail must recover a generator"
        cls.params = found
        a, seed, b, w = found
        cls.live = kld.R.decrypt_all(cls.cold, a, seed, b, w)

    def test_recovers_generator_from_tail_alone(self):
        report = win_demo.recover_and_verify(self.tail)
        self.assertTrue(report["recovered"])
        self.assertEqual(report["params"]["B"], 0x41C64E6D)  # public build constant
        self.assertIsNone(report["cold_boot_ok"])            # nothing to check yet

    def test_cold_snapshot_decrypts_with_leaked_keystream(self):
        report = win_demo.recover_and_verify(self.tail, cold_ciphertext=self.cold)
        self.assertTrue(report["cold_boot_ok"])
        self.assertTrue(report["cold_zip_ok"])
        self.assertEqual(len(report["cold_plaintext"]), kld.STORAGE_LEN)

    def test_live_volume_is_checked_as_is_not_decrypted(self):
        # the firmware-decrypted live volume is already FAT12; the script must
        # not XOR it with the keystream again (that was the bug on real hardware)
        report = win_demo.recover_and_verify(self.tail, live_volume=self.live)
        self.assertTrue(report["live_boot_ok"])
        double = kld.R.decrypt_all(self.live, *self.params)
        self.assertFalse(kld.R.strict_bootsector(double))

    def test_corrupted_tail_breaks_the_cold_decryption(self):
        tail = bytearray(model_tail(self.dump))
        for i in range(0, len(tail), win_demo.SECTOR):   # flip one byte per sector
            tail[i] ^= 0xFF
        report = win_demo.recover_and_verify(bytes(tail), cold_ciphertext=self.cold)
        self.assertFalse(report["recovered"] and report["cold_zip_ok"])

    def test_short_tail_is_rejected(self):
        with self.assertRaises(ValueError):
            win_demo.recover_and_verify(b"\x00" * 100)


if __name__ == "__main__":
    unittest.main()
