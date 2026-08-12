import sys
import unittest
from pathlib import Path
from unittest.mock import patch


SOURCE_DIR = Path(__file__).resolve().parents[1]
if str(SOURCE_DIR) not in sys.path:
    sys.path.insert(0, str(SOURCE_DIR))

from utils.pdf_environment import configured_pdf_fonts, ensure_pdf_environment


class PdfEnvironmentTests(unittest.TestCase):
    def test_default_fonts_are_the_tex_live_font_set(self):
        self.assertEqual(
            configured_pdf_fonts(),
            {
                "latin": "TeX Gyre Termes",
                "cjk_body": "FandolSong-Regular.otf",
                "cjk_heading": "FandolHei-Regular.otf",
                "cjk_emphasis": "FandolKai-Regular.otf",
                "code": "Source Code Pro",
            },
        )

    def test_configured_font_name_is_used_exactly(self):
        config = {"generation": {"pdf_fonts": {"cjk_body": "Custom Song"}}}
        self.assertEqual(configured_pdf_fonts(config)["cjk_body"], "Custom Song")

    @patch("utils.pdf_environment._probe_font", return_value=False)
    @patch("utils.pdf_environment.find_xelatex", return_value="xelatex")
    @patch("builtins.print")
    def test_missing_font_fails_when_auto_install_is_disabled(
        self, _print, _find_xelatex, _probe_font
    ):
        self.assertFalse(ensure_pdf_environment(auto_install=False))


if __name__ == "__main__":
    unittest.main()
