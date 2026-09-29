import tempfile
import unittest
import zipfile
from pathlib import Path
from unittest.mock import patch

from browser_demo import bounded_page, export_bounded


class BrowserDemoTests(unittest.TestCase):
    def test_bounded_page_reuses_the_real_payload(self):
        page = bounded_page()
        self.assertIn(b"DecompressionStream", page)
        self.assertIn(b"document.documentElement.dataset.copies", page)
        self.assertIn(b"hold.length < 2", page)
        self.assertNotIn(b"while (true)", page)

    def test_unexpected_payload_shape_is_rejected(self):
        with patch("browser_demo.bomb_html.make_bomb_html", return_value=b"<html>plain</html>"):
            with self.assertRaises(ValueError):
                bounded_page()

    def test_export_uses_original_page_and_caps_copies(self):
        source_page = b"<html><body><h1>Original</h1></body></html>"
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "original.html"
            output = Path(directory) / "prize_bounded.html"
            source.write_bytes(source_page)
            archive = export_bounded(source, output, 16)
            page = output.read_bytes()
            self.assertIn(source_page.split(b"</body>")[0], page)
            self.assertIn(b"while (hold.length < 16)", page)
            self.assertNotIn(b"while (true)", page)
            with zipfile.ZipFile(archive) as bundled:
                self.assertEqual(bundled.namelist(), ["prize.html"])
                self.assertEqual(bundled.read("prize.html"), page)
            with self.assertRaises(FileExistsError):
                export_bounded(source, output, 16)
            self.assertEqual(output.read_bytes(), page)

    def test_copy_limit_rejects_unbounded_values(self):
        for copies in (0, 17):
            with self.subTest(copies=copies), self.assertRaises(ValueError):
                bounded_page(copies=copies)


if __name__ == "__main__":
    unittest.main()
