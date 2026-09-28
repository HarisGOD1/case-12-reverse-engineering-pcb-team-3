import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from capture import capture


class CaptureTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.output = Path(self.directory.name) / "firmware.bin"

    def test_matching_reads_publish_one_dump(self):
        calls = []

        def save(command, *, check):
            self.assertTrue(check)
            calls.append(command)
            Path(command[-1]).write_bytes(b"firmware")

        with patch("capture.subprocess.run", side_effect=save):
            capture(self.output)

        self.assertEqual(len(calls), 2)
        self.assertTrue(all(command[:3] == ["picotool", "save", "-a"] for command in calls))
        self.assertNotEqual(calls[0][-1], calls[1][-1])
        self.assertEqual(self.output.read_bytes(), b"firmware")

    def test_mismatch_does_not_publish(self):
        values = iter((b"first", b"other"))

        def save(command, *, check):
            Path(command[-1]).write_bytes(next(values))

        with patch("capture.subprocess.run", side_effect=save):
            with self.assertRaises(ValueError):
                capture(self.output)

        self.assertFalse(self.output.exists())

    def test_failed_read_does_not_publish(self):
        with patch("capture.subprocess.run", side_effect=subprocess.CalledProcessError(1, "picotool")):
            with self.assertRaises(subprocess.CalledProcessError):
                capture(self.output)

        self.assertFalse(self.output.exists())

    def test_existing_output_is_not_replaced(self):
        self.output.write_bytes(b"original")
        with patch("capture.subprocess.run") as save:
            with self.assertRaises(FileExistsError):
                capture(self.output)

        save.assert_not_called()
        self.assertEqual(self.output.read_bytes(), b"original")


if __name__ == "__main__":
    unittest.main()
