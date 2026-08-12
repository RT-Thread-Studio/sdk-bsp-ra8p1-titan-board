import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch


SOURCE_DIR = Path(__file__).resolve().parents[1]
if str(SOURCE_DIR) not in sys.path:
    sys.path.insert(0, str(SOURCE_DIR))

from utils.html_builder import (
    _prepare_english_reserved_index,
    build_html_site,
    language_exclude_patterns,
    write_site_entry,
)


class HtmlBuilderTests(unittest.TestCase):
    def test_language_build_excludes_only_the_opposite_language(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            source = Path(temp_dir)
            guide = source / "guide"
            guide.mkdir()
            (source / "index_zh.rst").write_text("中文\n", encoding="utf-8")
            (source / "index.rst").write_text("English\n", encoding="utf-8")
            (guide / "README_zh.md").write_text("# 中文\n", encoding="utf-8")
            (guide / "README.md").write_text("# English\n", encoding="utf-8")
            stale = source / "_build" / "worktree"
            stale.mkdir(parents=True)
            (stale / "README.md").write_text("ignored\n", encoding="utf-8")

            self.assertEqual(
                language_exclude_patterns(source, "zh"),
                ["guide/README.md", "index.rst"],
            )
            self.assertEqual(
                language_exclude_patterns(source, "en"),
                ["guide/README_zh.md", "index_zh.rst"],
            )

    def test_english_index_is_reserved_and_internal_home_links_are_rewritten(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            output = Path(temp_dir)
            nested = output / "guide"
            nested.mkdir()
            (output / "index.html").write_text(
                '<a href="index.html">Home</a>', encoding="utf-8"
            )
            (nested / "start.html").write_text(
                '<link href="../_static/theme.css">'
                '<a href="../index.html#top">Home</a>'
                '<form action="../search.html"></form>',
                encoding="utf-8",
            )
            (output / "search.html").write_text(
                '<script>Search.loadIndex("searchindex.js")</script>',
                encoding="utf-8",
            )
            (output / "searchindex.js").write_text("index", encoding="utf-8")
            (output / "_static").mkdir()

            _prepare_english_reserved_index(output, "zh")

            self.assertFalse((output / "index.html").exists())
            self.assertTrue((output / "index_en.html").is_file())
            self.assertIn(
                "../index_en.html#top",
                (nested / "start.html").read_text(encoding="utf-8"),
            )
            self.assertIn(
                "../_static_en/theme.css",
                (nested / "start.html").read_text(encoding="utf-8"),
            )
            self.assertIn(
                "../search_en.html",
                (nested / "start.html").read_text(encoding="utf-8"),
            )
            self.assertTrue((output / "_static_en").is_dir())
            self.assertTrue((output / "search_en.html").is_file())
            self.assertTrue((output / "searchindex_en.js").is_file())
            self.assertIn(
                'Search.loadIndex("searchindex_en.js")',
                (output / "search_en.html").read_text(encoding="utf-8"),
            )

    def test_site_entry_redirects_to_the_configured_default_home(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            output = Path(temp_dir)
            created = write_site_entry(
                output, "index_zh", "SDK Docs", "zh"
            )

            self.assertTrue(created)
            self.assertIn(
                "url=./index_zh.html",
                (output / "index.html").read_text(encoding="utf-8"),
            )

    def test_html_only_rebuild_preserves_existing_pdf_outputs(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            source = root / "source"
            output = root / "html"
            source.mkdir()
            static = output / "_static"
            static.mkdir(parents=True)
            pdf_content = b"%PDF-1.7\nexisting-pdf"
            (static / "SDK_Docs_EN.pdf").write_bytes(pdf_content)
            (static / "project_info.json").write_text(
                '{"pdfFileName":"SDK_Docs_EN.pdf"}', encoding="utf-8"
            )

            def fake_build(_source, language_output, *_args):
                (language_output / "_static").mkdir(parents=True)
                (language_output / "_static" / "basic.css").write_text(
                    "body {}", encoding="utf-8"
                )
                (language_output / "index.html").write_text(
                    "<h1>Docs</h1>", encoding="utf-8"
                )

            with patch("utils.html_builder._build_one_language", fake_build):
                build_html_site(source, output, {"generation": {}}, ("en",), "en")

            self.assertEqual(
                (output / "_static" / "SDK_Docs_EN.pdf").read_bytes(),
                pdf_content,
            )
            self.assertTrue((output / "_static" / "project_info.json").is_file())
            self.assertTrue((output / "_static" / "basic.css").is_file())
            self.assertFalse((root / ".html_preserved").exists())


if __name__ == "__main__":
    unittest.main()
