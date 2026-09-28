import contextlib
import io
import tempfile
import unittest
from pathlib import Path

import keystream_leak_demo as demo


DUMP = Path(__file__).resolve().parents[2] / "artifacts" / "backup_full.bin"


def observed_tail(data):
    leaked = {}
    for lba in range(demo.STORAGE_LEN // demo.SECTOR, demo.DISK_SECTORS):
        sector = data[demo.STORAGE_OFF + lba * demo.SECTOR:demo.STORAGE_OFF + (lba + 1) * demo.SECTOR]
        if sector == b"\xff" * demo.SECTOR:
            served = bytes(byte ^ key for byte, key in zip(sector, demo.ks_sector(lba)))
            leaked[lba] = bytes(0xff ^ byte for byte in served)
    return leaked


class LeakTests(unittest.TestCase):
    def test_tail_only_cli_exports_keystream_without_dump(self):
        start = demo.STORAGE_LEN // demo.SECTOR
        tail = b"".join(
            bytes(0xff ^ key for key in demo.ks_sector(lba))
            for lba in range(start, demo.DISK_SECTORS - 1)
        ) + b"\x00" * demo.SECTOR
        with tempfile.TemporaryDirectory() as directory:
            capture = Path(directory) / "captured-usb.bin"
            table = Path(directory) / "keystream.bin"
            capture.write_bytes(tail)
            with contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(demo.main(["demo", "--usb-tail-only", str(capture), str(table)]), 0)
            self.assertEqual(table.stat().st_size, demo.STORAGE_LEN)
            result = table.read_bytes()
            self.assertEqual(result[:demo.SECTOR], demo.ks_sector(0))
            self.assertEqual(result[-demo.SECTOR:], demo.ks_sector(start - 1))
            with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(demo.main(["demo", "--usb-tail-only", str(capture), str(table)]), 1)
            self.assertEqual(table.read_bytes(), result)

    def test_recorded_tail_mode_checks_source_image(self):
        data = DUMP.read_bytes()
        start = demo.STORAGE_LEN // demo.SECTOR
        tail = b"".join(
            bytes(byte ^ key for byte, key in zip(
                data[demo.STORAGE_OFF + lba * demo.SECTOR:demo.STORAGE_OFF + (lba + 1) * demo.SECTOR],
                demo.ks_sector(lba),
            )) for lba in range(start, demo.DISK_SECTORS)
        )
        with tempfile.TemporaryDirectory() as directory:
            capture = Path(directory) / "usb-tail.bin"
            capture.write_bytes(tail)
            with contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(demo.main(["demo", str(DUMP), "--usb-tail", str(capture)]), 0)
            changed = bytearray(tail)
            changed[0] ^= 1
            capture.write_bytes(changed)
            with contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(demo.main(["demo", str(DUMP), "--usb-tail", str(capture)]), 1)

    def test_tail_capture_parser_matches_erased_flash(self):
        tail = b"".join(
            bytes(0xff ^ key for key in demo.ks_sector(lba))
            for lba in range(demo.STORAGE_LEN // demo.SECTOR, demo.DISK_SECTORS - 1)
        ) + b"\x00" * demo.SECTOR
        self.assertEqual(demo.decode_tail_response(tail), observed_tail(DUMP.read_bytes()))
        with self.assertRaises(ValueError):
            demo.decode_tail_response(tail[:-1])

    def test_oracle_only_recovery_decrypts_held_out_dump(self):
        data = DUMP.read_bytes()
        leaked = observed_tail(data)
        self.assertEqual(len(leaked), 639)
        recovered = demo.recover_generator_from_leak(leaked)
        self.assertIsNotNone(recovered)
        from recover_keystream import decrypt_all, volume_has_valid_zip

        plaintext = decrypt_all(
            data[demo.STORAGE_OFF:demo.STORAGE_OFF + demo.STORAGE_LEN], *recovered
        )
        self.assertTrue(volume_has_valid_zip(plaintext))

    def test_conflicting_observation_fails(self):
        leaked = observed_tail(DUMP.read_bytes())
        first = min(leaked)
        changed = bytearray(leaked[first])
        changed[0] ^= 0x80
        leaked[first] = bytes(changed)
        self.assertIsNone(demo.recover_generator_from_leak(leaked))


if __name__ == "__main__":
    unittest.main()
