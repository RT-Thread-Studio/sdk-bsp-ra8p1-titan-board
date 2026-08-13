import sys
import tempfile
import unittest
from pathlib import Path


SOURCE_DIR = Path(__file__).resolve().parents[1]
if str(SOURCE_DIR) not in sys.path:
    sys.path.insert(0, str(SOURCE_DIR))

from build_manager import BuildManager


class BuildManagerLanguageRewriteTests(unittest.TestCase):
    def _rewrite(self, html: str, language: str) -> str:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            source = root / "source.html"
            target = root / "target.html"
            source.write_text(html, encoding="utf-8")
            BuildManager.__new__(BuildManager)._fix_html_language(
                source, target, language
            )
            return target.read_text(encoding="utf-8")

    def test_english_page_keeps_cross_language_markdown_link(self):
        html = (
            '<html lang="zh-CN"><body>'
            '<a href="guide/start_zh.html">Chinese</a>'
            '<!-- docs-cross-language-link -->'
            '<a href="guide/index_zh.html">Guide</a>'
            '</body></html>'
        )

        rewritten = self._rewrite(html, "en")

        self.assertIn('href="guide/start_zh.html"', rewritten)
        self.assertIn('href="guide/index.html"', rewritten)

    def test_chinese_page_keeps_cross_language_markdown_link(self):
        html = (
            '<html lang="en"><body>'
            '<a href="guide/start.html">English</a>'
            '<!-- docs-cross-language-link -->'
            '<a href="guide/index.html">Guide</a>'
            '</body></html>'
        )

        rewritten = self._rewrite(html, "zh")

        self.assertIn('href="guide/start.html"', rewritten)
        self.assertIn('href="guide/index_zh.html"', rewritten)


if __name__ == "__main__":
    unittest.main()
