import unittest
from unittest.mock import patch

from browser_demo import bounded_page


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


if __name__ == "__main__":
    unittest.main()
